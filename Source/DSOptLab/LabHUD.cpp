#include "LabHUD.h"

#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Misc/Paths.h"
#include "LabBuilding.h"
#include "LabCharacter.h"
#include "LabInventoryComponent.h"
#include "LabNpc.h"
#include "LabPlayerController.h"
#include "LabResourceNode.h"
#include "LabScenarioConfig.h"
#include "LabStateComponent.h"
#include "UnrealClient.h"

void ALabHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ALabPlayerController* Controller = Cast<ALabPlayerController>(PlayerOwner);
	const APawn* ControlledPawn = Controller ? Controller->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		return;
	}

	const FLabClientConfig& Config = FLabClientConfig::Get();

	if (Config.bTopDown)
	{
		UpdateTopDown(*Controller, *ControlledPawn);
	}

	UpdateOverlay(*Controller, *ControlledPawn);

	// 스크린샷은 공통 시작 신호 이후에만 찍는다.
	if (Config.bAutoScreenshot && Controller->IsScenarioStarted())
	{
		UpdateAutoScreenshot(DeltaSeconds);
	}
}

void ALabHUD::UpdateOverlay(const ALabPlayerController& Controller, const APawn& ControlledPawn)
{
	const FLabClientConfig& Config = FLabClientConfig::Get();
	UWorld* World = GetWorld();

	// 메시에 가려지지 않도록 메시 위쪽에, 깊이 검사를 받지 않는 그룹으로 그린다.
	const FVector PointOffset(0.f, 0.f, 400.f);

	// 클라이언트에 존재하는 액터만 센다. 서버가 보내지 않은 액터는 여기에 없다.
	int32 NumNodes = 0;
	for (TActorIterator<ALabResourceNode> It(World); It; ++It)
	{
		++NumNodes;
		const bool bDamaged = It->IsDamaged();
		if (Config.bTopDown)
		{
			// 체력이 깎인 노드는 노란색으로 보여 채집이 진행 중임을 알린다.
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, bDamaged ? 12.f : 5.f,
				It->IsDepleted() ? FColor::Black : bDamaged ? FColor::Yellow : FColor::Green, false, -1.f, SDPG_Foreground);
		}
	}

	// 2막의 확장 요소. 서버가 붙여서 보낸 컴포넌트만 센다. 1막의 실행에서는 모두 0이다.
	int32 NumStates = 0;
	int32 NumInventories = 0;
	int32 NumOwnItems = 0;
	int32 OwnFirstItemId = 0;

	int32 NumBuildings = 0;
	for (TActorIterator<ALabBuilding> It(World); It; ++It)
	{
		++NumBuildings;
		if (Config.bTopDown)
		{
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, 5.f, FColor(255, 170, 0), false, -1.f, SDPG_Foreground);
		}
	}

	int32 NumNpcs = 0;
	for (TActorIterator<ALabNpc> It(World); It; ++It)
	{
		++NumNpcs;
		NumStates += It->FindComponentByClass<ULabStateComponent>() ? 1 : 0;
		if (Config.bTopDown)
		{
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, 7.f, FColor::Red, false, -1.f, SDPG_Foreground);
		}
	}

	// 플레이어는 노드와 NPC보다 크게, 자기 폰은 흰색, 다른 플레이어는 파란색으로 그린다.
	int32 NumPlayers = 0;
	for (TActorIterator<ALabCharacter> It(World); It; ++It)
	{
		++NumPlayers;
		NumStates += It->FindComponentByClass<ULabStateComponent>() ? 1 : 0;
		if (const ULabInventoryComponent* Inventory = It->FindComponentByClass<ULabInventoryComponent>())
		{
			++NumInventories;
			if (*It == &ControlledPawn)
			{
				NumOwnItems = Inventory->GetNumItems();
				OwnFirstItemId = Inventory->GetFirstItemId();
			}
		}
		if (Config.bTopDown)
		{
			DrawDebugPoint(World, It->GetActorLocation() + PointOffset, 9.f,
				*It == &ControlledPawn ? FColor::White : FColor(40, 140, 255), false, -1.f, SDPG_Foreground);
		}
	}

	// 채집 담당은 이동하지 않으므로 채집을 먼저 본다(ALabPlayerController::PlayerTick과 같은 순서).
	const TCHAR* Duty = Config.bAutoHarvest ? TEXT("harvest") : Config.bAutoMove ? TEXT("move") : TEXT("idle");
	const TCHAR* View = Config.bTopDown ? TEXT("topdown") : TEXT("tpp");
	const FString Elapsed = Controller.IsScenarioStarted()
		? FString::Printf(TEXT("t=%.0fs"), World->GetTimeSeconds() - Controller.GetScenarioStartTime())
		: FString(TEXT("t=waiting"));
	const FVector Location = ControlledPawn.GetActorLocation() / 100.f;

	OverlayLines = {
		FString::Printf(TEXT("%s | slot=%d %s %s | %s"), *FLabScenarioConfig::Get().Label, Config.Slot, Duty, View, *Elapsed),
		FString::Printf(TEXT("on this client: nodes=%d npcs=%d players=%d | pos x=%.0fm y=%.0fm"), NumNodes, NumNpcs, NumPlayers, Location.X, Location.Y),
	};

	// 맨 앞 칸의 아이템 번호는 인벤토리의 앞 칸이 지워질 때마다 바뀐다.
	if (NumStates > 0 || NumInventories > 0 || NumBuildings > 0)
	{
		OverlayLines.Add(FString::Printf(TEXT("buildings=%d states=%d inventories=%d | own items=%d first id=%d"), NumBuildings, NumStates, NumInventories, NumOwnItems, OwnFirstItemId));
	}
}

void ALabHUD::UpdateTopDown(ALabPlayerController& Controller, const APawn& ControlledPawn)
{
	if (!TopDownCamera)
	{
		// 클라이언트에서만 뷰 타깃을 바꾼다. 서버의 뷰 타깃은 폰으로 남는다.
		TopDownCamera = GetWorld()->SpawnActor<ACameraActor>();

		// 서버가 ClientSetViewTarget으로 뷰 타깃을 폰으로 되돌리지 못하게 한다(PlayerController.cpp의 ClientSetViewTarget_Implementation).
		Controller.bAutoManageActiveCameraTarget = false;
		if (Controller.PlayerCameraManager)
		{
			Controller.PlayerCameraManager->bClientSimulatingViewTarget = true;
		}
	}

	if (Controller.GetViewTarget() != TopDownCamera)
	{
		Controller.SetViewTarget(TopDownCamera);
	}

	// 수평 시야각 90도에서 높이 350m면 좌우 350m, 위아래 약 197m가 보인다.
	TopDownCamera->SetActorLocationAndRotation(
		ControlledPawn.GetActorLocation() + FVector(0.f, 0.f, TopDownHeight),
		FRotator(-90.f, 0.f, 0.f));
}

void ALabHUD::UpdateAutoScreenshot(float DeltaSeconds)
{
	ScreenshotAccumulator += DeltaSeconds;
	if (ScreenshotAccumulator < ScreenshotInterval)
	{
		return;
	}
	ScreenshotAccumulator = 0.f;

	const FLabClientConfig& Config = FLabClientConfig::Get();
	const TCHAR* View = Config.bTopDown ? TEXT("topdown") : TEXT("tpp");
	const FString Path = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("Lab")
		/ FString::Printf(TEXT("%s-%s-%02d.png"), *FLabScenarioConfig::Get().Label, View, ScreenshotIndex++);

	FScreenshotRequest::RequestScreenshot(Path, true, false);
}

void ALabHUD::DrawHUD()
{
	Super::DrawHUD();

	if (OverlayLines.IsEmpty() || !GEngine)
	{
		return;
	}

	// 엔진 화면 메시지와 같은 글꼴을 쓴다(UnrealEngine.cpp의 DrawOnscreenDebugMessages).
	UFont* Font = GEngine->GetSmallFont();

	float BoxWidth = 0.f;
	float LineHeight = 0.f;
	for (const FString& Line : OverlayLines)
	{
		float Width = 0.f;
		float Height = 0.f;
		GetTextSize(Line, Width, Height, Font, TextScale);
		BoxWidth = FMath::Max(BoxWidth, Width);
		LineHeight = FMath::Max(LineHeight, Height);
	}
	LineHeight *= LineSpacing;

	// 밝은 하늘과 바닥 위에서도 대비가 일정하도록 반투명 검은 상자를 먼저 깐다.
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), Margin, Margin, BoxWidth + Padding * 2.f, LineHeight * OverlayLines.Num() + Padding * 2.f);

	// 노란색은 경고처럼 보이고 흰색은 눈에 덜 띄어서 하늘색을 쓴다.
	const FLinearColor TextColor(FColor::Cyan);
	float Y = Margin + Padding;
	for (const FString& Line : OverlayLines)
	{
		DrawText(Line, FLinearColor::Black, Margin + Padding + 1.f, Y + 1.f, Font, TextScale);
		DrawText(Line, TextColor, Margin + Padding, Y, Font, TextScale);
		Y += LineHeight;
	}
}
