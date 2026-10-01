#include "LabPlayerController.h"

#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "Misc/Paths.h"
#include "LabGameMode.h"
#include "LabNpc.h"
#include "LabResourceNode.h"
#include "LabScenarioConfig.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealClient.h"

ALabPlayerController::ALabPlayerController()
{
	// 템플릿이 블루프린트 컨트롤러에서 지정하던 입력 매핑을 코드에서 넣는다. 수동 조작(run-manual.ps1의 관찰자)에 쓴다.
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultContext(TEXT("/Game/Input/IMC_Default.IMC_Default"));
	if (DefaultContext.Succeeded())
	{
		MappingContexts.Add(DefaultContext.Object);
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookContext(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	if (MouseLookContext.Succeeded())
	{
		MappingContexts.Add(MouseLookContext.Object);
	}
}

void ALabPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Context : MappingContexts)
		{
			Subsystem->AddMappingContext(Context, 0);
		}
	}
}

void ALabPlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	// 서버가 클라이언트의 카메라 위치가 아니라 폰 쪽 시점을 기준으로 관련성을 판정하게 한다.
	// 서버와 클라이언트 양쪽에서 꺼야 한다(Docs/Planning/engine-notes.md의 "관련성 판정의 기준 위치").
	if (PlayerCameraManager)
	{
		PlayerCameraManager->bUseClientSideCameraUpdates = false;
	}
}

void ALabPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FLabScenarioConfig& Config = FLabScenarioConfig::Get();

	if (!bReportedReady)
	{
		bReportedReady = true;
		ServerReportReady(Config.ClientSlot);
	}

	if (Config.bTopDown)
	{
		TickTopDown(*ControlledPawn);
	}

	TickOverlay();

	// 이동, 채집, 스크린샷은 공통 시작 신호 이후에만 한다.
	if (!bScenarioStarted)
	{
		return;
	}

	if (Config.bAutoHarvest)
	{
		// 채집 담당은 검증용 노드 옆에 서서 채집만 한다.
		HarvestAccumulator += DeltaTime;
		if (HarvestAccumulator >= HarvestInterval)
		{
			HarvestAccumulator = 0.f;
			ServerHarvest();
		}
	}
	else if (Config.bAutoMove)
	{
		TickAutoMove(*ControlledPawn);
	}

	if (Config.bAutoScreenshot)
	{
		TickAutoScreenshot(DeltaTime);
	}
}

void ALabPlayerController::ServerReportReady_Implementation(int32 InSlot)
{
	Slot = FMath::Clamp(InSlot, 0, FLabScenarioConfig::NumPlayerSlots - 1);

	if (ALabGameMode* GameMode = GetWorld()->GetAuthGameMode<ALabGameMode>())
	{
		GameMode->HandlePlayerReady(*this);
	}
}

void ALabPlayerController::ClientStartScenario_Implementation(FVector StartLocation)
{
	// 폰의 현재 위치는 아직 서버의 이동을 반영하지 않았을 수 있으므로 서버가 준 위치를 기준으로 삼는다.
	Home = StartLocation;
	WaypointIndex = 0;
	HarvestAccumulator = 0.f;
	ScreenshotAccumulator = 0.f;
	ScreenshotIndex = 0;
	bScenarioStarted = true;
}

void ALabPlayerController::TickAutoMove(APawn& ControlledPawn)
{
	// 시작 위치를 한 꼭짓점으로 하는 한 변 100m 정사각형을 돈다.
	const FVector Offsets[4] = {
		FVector(WaypointSide, 0.f, 0.f),
		FVector(WaypointSide, WaypointSide, 0.f),
		FVector(0.f, WaypointSide, 0.f),
		FVector::ZeroVector,
	};

	FVector ToWaypoint = Home + Offsets[WaypointIndex] - ControlledPawn.GetActorLocation();
	ToWaypoint.Z = 0.f;

	if (ToWaypoint.Size() < WaypointReachDistance)
	{
		WaypointIndex = (WaypointIndex + 1) % 4;
		return;
	}

	ControlledPawn.AddMovementInput(ToWaypoint.GetSafeNormal());
}

void ALabPlayerController::TickOverlay()
{
	const FLabScenarioConfig& Config = FLabScenarioConfig::Get();
	UWorld* World = GetWorld();

	// 메시에 가려지지 않도록 메시 위쪽에, 깊이 검사를 받지 않는 그룹으로 그린다.
	const FVector PointOffset(0.f, 0.f, 400.f);

	// 클라이언트에 존재하는 액터만 센다. 서버가 보내지 않은 액터는 여기에 없다.
	int32 NumNodes = 0;
	for (TActorIterator<ALabResourceNode> It(World); It; ++It)
	{
		++NumNodes;
		if (Config.bTopDown)
		{
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, 5.f,
				It->IsDepleted() ? FColor::Black : FColor::Green, false, -1.f, SDPG_Foreground);
		}
	}

	int32 NumNpcs = 0;
	for (TActorIterator<ALabNpc> It(World); It; ++It)
	{
		++NumNpcs;
		if (Config.bTopDown)
		{
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, 7.f, FColor::Red, false, -1.f, SDPG_Foreground);
		}
	}

	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("%s | on this client: nodes=%d npcs=%d"), *Config.Label, NumNodes, NumNpcs);
		GEngine->AddOnScreenDebugMessage(1001, 0.f, FColor::Yellow, Message);
	}
}

void ALabPlayerController::TickTopDown(const APawn& ControlledPawn)
{
	if (!TopDownCamera)
	{
		// 클라이언트에서만 뷰 타깃을 바꾼다. 서버의 뷰 타깃은 폰으로 남는다.
		TopDownCamera = GetWorld()->SpawnActor<ACameraActor>();

		// 서버가 ClientSetViewTarget으로 뷰 타깃을 폰으로 되돌리지 못하게 한다(PlayerController.cpp의 ClientSetViewTarget_Implementation).
		bAutoManageActiveCameraTarget = false;
		if (PlayerCameraManager)
		{
			PlayerCameraManager->bClientSimulatingViewTarget = true;
		}
	}

	if (GetViewTarget() != TopDownCamera)
	{
		SetViewTarget(TopDownCamera);
	}

	// 수평 시야각 90도에서 높이 350m면 좌우 350m, 위아래 약 197m가 보인다.
	TopDownCamera->SetActorLocationAndRotation(
		ControlledPawn.GetActorLocation() + FVector(0.f, 0.f, TopDownHeight),
		FRotator(-90.f, 0.f, 0.f));
}

void ALabPlayerController::TickAutoScreenshot(float DeltaTime)
{
	ScreenshotAccumulator += DeltaTime;
	if (ScreenshotAccumulator < ScreenshotInterval)
	{
		return;
	}
	ScreenshotAccumulator = 0.f;

	const FLabScenarioConfig& Config = FLabScenarioConfig::Get();
	const TCHAR* View = Config.bTopDown ? TEXT("topdown") : TEXT("tpp");
	const FString Path = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("Lab")
		/ FString::Printf(TEXT("%s-%s-%02d.png"), *Config.Label, View, ScreenshotIndex++);

	FScreenshotRequest::RequestScreenshot(Path, true, false);
}

void ALabPlayerController::ServerHarvest_Implementation()
{
	// 자동 실험용 RPC다. 호출 간격만 제한하고, 그 밖의 악의적 호출은 막지 않는다.
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHarvestTime < MinServerHarvestInterval)
	{
		return;
	}
	LastHarvestTime = Now;

	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FVector Location = ControlledPawn->GetActorLocation();
	ALabResourceNode* Nearest = nullptr;
	float NearestDistSq = FMath::Square(HarvestRange);

	for (TActorIterator<ALabResourceNode> It(GetWorld()); It; ++It)
	{
		if (It->IsDepleted())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared2D(It->GetActorLocation(), Location);
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = *It;
		}
	}

	if (Nearest)
	{
		Nearest->Harvest();
	}
}
