#include "LabGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformMisc.h"
#include "Math/RandomStream.h"
#include "DSOptLab.h"
#include "LabCharacter.h"
#include "LabHUD.h"
#include "LabNpc.h"
#include "LabPlayerController.h"
#include "LabResourceNode.h"
#include "LabScenarioConfig.h"
#include "UObject/ConstructorHelpers.h"

ALabGameMode::ALabGameMode()
{
	PlayerControllerClass = ALabPlayerController::StaticClass();
	HUDClass = ALabHUD::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnFinder(TEXT("/Game/Blueprints/BP_LabCharacter"));
	if (PawnFinder.Succeeded())
	{
		DefaultPawnClass = PawnFinder.Class;
	}
}

void ALabGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트 폰을 읽지 못하면 엔진 기본 폰으로 조용히 실행되어, 다른 폰으로 잰 수치가 CSV에 남는다.
	if (!DefaultPawnClass || !DefaultPawnClass->IsChildOf(ALabCharacter::StaticClass()))
	{
		UE_LOG(LogDSOptLab, Error, TEXT("DefaultPawnClass is '%s', not a child of ALabCharacter. The player pawn blueprint failed to load."), *GetNameSafe(DefaultPawnClass));
		if (FLabScenarioConfig::Get().bMeasure)
		{
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}
	}

	SpawnWorld();

	// 측정 실행에서는 측정 서브시스템이 모든 클라이언트를 확인한 뒤 시작 신호를 낸다.
	if (!FLabScenarioConfig::Get().bMeasure)
	{
		StartScenario();
	}
}

FVector ALabGameMode::GetSlotLocation(int32 Slot)
{
	const float Angle = 2.f * PI * static_cast<float>(Slot) / static_cast<float>(FLabScenarioConfig::NumPlayerSlots);
	const float Radius = FLabScenarioConfig::PlayerRingRadius;
	return FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 200.f);
}

void ALabGameMode::SpawnWorld()
{
	const FLabScenarioConfig& Config = FLabScenarioConfig::Get();
	const float Extent = FLabScenarioConfig::WorldHalfExtent;

	// 고정 시드라서 실행마다 같은 배치가 나온다.
	FRandomStream Rng(Config.Seed);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Index = 0; Index < Config.NumNodes; ++Index)
	{
		// 실린더 높이 300cm의 중심이 150cm에 오게 해 바닥에 세운다.
		const FVector Location(Rng.FRandRange(-Extent, Extent), Rng.FRandRange(-Extent, Extent), 150.f);
		GetWorld()->SpawnActor<ALabResourceNode>(ALabResourceNode::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	for (int32 Index = 0; Index < Config.NumNpcs; ++Index)
	{
		const FVector Location(Rng.FRandRange(-Extent, Extent), Rng.FRandRange(-Extent, Extent), 50.f);
		GetWorld()->SpawnActor<ALabNpc>(ALabNpc::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	// 검증용 노드. 0번 자리에서 3m 떨어진 곳에 항상 있다. 채집 담당이 이 노드를 고갈시킨다.
	FVector VerificationNodeLocation = GetSlotLocation(0) + FVector(300.f, 0.f, 0.f);
	VerificationNodeLocation.Z = 150.f;
	GetWorld()->SpawnActor<ALabResourceNode>(ALabResourceNode::StaticClass(), VerificationNodeLocation, FRotator::ZeroRotator, Params);

	// 영상용 NPC. 0번 자리의 3인칭 화면 앞 10m에서 화면 오른쪽 절반을 가로지르는 10m 직선을 왕복한다.
	// 캐릭터와 검증용 노드에 가리지 않게 화면 가운데에서 2.5m 띄웠다. 측정 실행에는 없다.
	// 난수를 쓰지 않으므로 다른 액터의 배치는 그대로다.
	if (Config.bShowcaseNpc)
	{
		FVector PatrolStart = GetSlotLocation(0) + FVector(1000.f, 250.f, 0.f);
		FVector PatrolEnd = GetSlotLocation(0) + FVector(1000.f, 1250.f, 0.f);
		PatrolStart.Z = PatrolEnd.Z = 50.f;
		if (ALabNpc* Npc = GetWorld()->SpawnActor<ALabNpc>(ALabNpc::StaticClass(), PatrolStart, FRotator::ZeroRotator, Params))
		{
			Npc->SetPatrol(PatrolStart, PatrolEnd);
		}
	}
}

void ALabGameMode::StartScenario()
{
	if (bScenarioStarted)
	{
		return;
	}
	bScenarioStarted = true;

	for (TActorIterator<ALabNpc> It(GetWorld()); It; ++It)
	{
		It->StartWandering();
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALabPlayerController* Player = Cast<ALabPlayerController>(It->Get());
		if (Player && Player->IsReady())
		{
			PlaceAndStart(*Player);
		}
	}
}

void ALabGameMode::HandlePlayerReady(ALabPlayerController& Player)
{
	if (bScenarioStarted)
	{
		PlaceAndStart(Player);
	}
}

void ALabGameMode::PlaceAndStart(ALabPlayerController& Player)
{
	APawn* Pawn = Player.GetPawn();
	if (!Pawn)
	{
		return;
	}

	int32& UseCount = SlotUseCount.FindOrAdd(Player.GetSlot());
	const FVector Location = GetSlotLocation(Player.GetSlot()) + FVector(0.f, 200.f * UseCount, 0.f);
	++UseCount;

	Pawn->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	Player.ClientStartScenario(Location);
}
