#include "LabGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformMisc.h"
#include "Math/RandomStream.h"
#include "DSOptLab.h"
#include "LabBuilding.h"
#include "LabCharacter.h"
#include "LabHUD.h"
#include "LabInventoryComponent.h"
#include "LabNpc.h"
#include "LabPlayerController.h"
#include "LabResourceNode.h"
#include "LabScenarioConfig.h"
#include "LabStateComponent.h"
#include "TimerManager.h"
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
		if (FLabServerConfig::Get().bMeasure)
		{
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}
	}

	SpawnWorld();

	// 측정 실행은 기대한 수의 클라이언트가 준비를 보고한 뒤에 시작한다(HandlePlayerReady). 그 밖의 실행은 바로 시작한다.
	if (!FLabServerConfig::Get().bMeasure)
	{
		StartScenario();
	}
}

FVector ALabGameMode::GetSlotLocation(int32 Slot)
{
	const FLabServerConfig& Config = FLabServerConfig::Get();
	if (Config.PlayerSpacingMeters > 0.f)
	{
		// 맵 가운데를 지나는 대각선 위에 같은 간격으로 놓는다. 자리마다 x와 y가 모두 달라서
		// 정사각형 경로의 변이 서로 겹치지 않는다(이웃한 변 사이는 간격 ÷ √2).
		const float Step = Config.PlayerSpacingMeters * 100.f / UE_SQRT_2;
		const float Offset = (static_cast<float>(Slot) - 0.5f * static_cast<float>(Config.ExpectedClients - 1)) * Step;
		return FVector(Offset, Offset, 200.f);
	}

	const float Angle = 2.f * PI * static_cast<float>(Slot) / static_cast<float>(FLabScenarioConfig::NumPlayerSlots);
	const float Radius = FLabScenarioConfig::PlayerRingRadius;
	return FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 200.f);
}

void ALabGameMode::SpawnWorld()
{
	const FLabServerConfig& Config = FLabServerConfig::Get();
	const float Extent = FLabScenarioConfig::WorldHalfExtent;

	UE_LOG(LogDSOptLab, Display, TEXT("lab_config=%s"), *Config.GetConfigName());

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
		ALabNpc* Npc = GetWorld()->SpawnActor<ALabNpc>(ALabNpc::StaticClass(), Location, FRotator::ZeroRotator, Params);
		if (Npc && Config.StateIntervalSeconds > 0.f)
		{
			AddStateComponent(*Npc);
		}
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
		if (ALabShowcaseNpc* Npc = GetWorld()->SpawnActor<ALabShowcaseNpc>(ALabShowcaseNpc::StaticClass(), PatrolStart, FRotator::ZeroRotator, Params))
		{
			Npc->SetPatrol(PatrolStart, PatrolEnd);
		}
	}

	if (Config.BuildingsPerCluster > 0)
	{
		SpawnBuildings();
	}
}

void ALabGameMode::SpawnBuildings()
{
	const FLabServerConfig& Config = FLabServerConfig::Get();

	// 자원 노드와 NPC의 난수와 따로 써서, 건축물을 켜도 그 배치가 달라지지 않는다.
	BuildingRng.Initialize(Config.Seed + 2);

	// 자리마다 경로(정사각형)의 중심을 구하고, 가까운 자리끼리 한 무리로 묶는다.
	// 밀집 배치에서는 여덟 자리가 한 무리가 되고, 분산 배치에서는 자리마다 무리가 하나씩 생긴다.
	// 어느 배치에서나 연결 하나가 받는 건축물의 수가 같게 하려는 것이다.
	const float Side = Config.PlayerSpacingMeters > 0.f ? ALabPlayerController::SpacedWaypointSide : ALabPlayerController::WaypointSide;
	TArray<FVector> CenterSums;
	for (int32 Slot = 0; Slot < Config.ExpectedClients; ++Slot)
	{
		FVector RouteCenter = GetSlotLocation(Slot) + FVector(0.5f * Side, 0.5f * Side, 0.f);
		RouteCenter.Z = 0.f;

		const int32 Found = BuildingClusters.IndexOfByPredicate([&RouteCenter](const FBuildingCluster& Cluster)
		{
			return FVector::DistSquared2D(Cluster.FirstRouteCenter, RouteCenter) < FMath::Square(BuildingClusterMergeDistance);
		});
		if (Found != INDEX_NONE)
		{
			CenterSums[Found] += RouteCenter;
			++BuildingClusters[Found].NumSlots;
		}
		else
		{
			FBuildingCluster& Cluster = BuildingClusters.AddDefaulted_GetRef();
			Cluster.FirstRouteCenter = RouteCenter;
			Cluster.NumSlots = 1;
			CenterSums.Add(RouteCenter);
		}
	}

	for (int32 Index = 0; Index < BuildingClusters.Num(); ++Index)
	{
		FBuildingCluster& Cluster = BuildingClusters[Index];
		Cluster.Center = CenterSums[Index] / static_cast<float>(Cluster.NumSlots);
		for (int32 Count = 0; Count < Config.BuildingsPerCluster; ++Count)
		{
			SpawnBuilding(Cluster);
		}
	}

	UE_LOG(LogDSOptLab, Display, TEXT("lab_buildings clusters=%d per_cluster=%d"), BuildingClusters.Num(), Config.BuildingsPerCluster);
}

void ALabGameMode::SpawnBuilding(FBuildingCluster& Cluster)
{
	// 원 안에 고르게 놓는다. 한 변 200cm 정육면체의 중심이 100cm에 오게 해 바닥에 세운다.
	const float Distance = BuildingClusterRadius * FMath::Sqrt(BuildingRng.FRand());
	const float Angle = 2.f * PI * BuildingRng.FRand();
	const FVector Location = Cluster.Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 100.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ALabBuilding* Building = GetWorld()->SpawnActor<ALabBuilding>(ALabBuilding::StaticClass(), Location, FRotator::ZeroRotator, Params))
	{
		Cluster.Buildings.Add(Building);
	}
}

void ALabGameMode::RebuildOne()
{
	// 무리를 돌아가며 가장 오래된 것을 허물고 새로 하나를 짓는다. 건축물의 수는 그대로다.
	FBuildingCluster& Cluster = BuildingClusters[NextRebuildCluster % BuildingClusters.Num()];
	++NextRebuildCluster;

	if (!Cluster.Buildings.IsEmpty())
	{
		if (ALabBuilding* Oldest = Cluster.Buildings[0].Get())
		{
			Oldest->Destroy();
		}
		Cluster.Buildings.RemoveAt(0);
	}
	SpawnBuilding(Cluster);
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

	TArray<ALabPlayerController*> PlacedPlayers;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALabPlayerController* Player = Cast<ALabPlayerController>(It->Get());
		if (Player && Player->IsReady())
		{
			PlaceAndStart(*Player);
			PlacedPlayers.Add(Player);
		}
	}

	// 접속한 순서는 실행마다 다를 수 있으므로 자리 번호 순서로 붙인다.
	PlacedPlayers.StableSort([](const ALabPlayerController& A, const ALabPlayerController& B) { return A.GetSlot() < B.GetSlot(); });
	for (ALabPlayerController* Player : PlacedPlayers)
	{
		AddPlayerElements(*Player);
	}
	StartElementTimers();

	OnScenarioStarted.Broadcast();
}

void ALabGameMode::AddStateComponent(AActor& Actor)
{
	ULabStateComponent* State = NewObject<ULabStateComponent>(&Actor);
	State->RegisterComponent();
	StateComponents.Add(State);
}

void ALabGameMode::AddPlayerElements(ALabPlayerController& Player)
{
	const FLabServerConfig& Config = FLabServerConfig::Get();
	APawn* Pawn = Player.GetPawn();
	if (!Pawn)
	{
		return;
	}

	if (Config.StateIntervalSeconds > 0.f && !Pawn->FindComponentByClass<ULabStateComponent>())
	{
		AddStateComponent(*Pawn);
	}

	if (Config.InventoryItems > 0 && !Pawn->FindComponentByClass<ULabInventoryComponent>())
	{
		ULabInventoryComponent* Inventory = NewObject<ULabInventoryComponent>(Pawn);
		Inventory->RegisterComponent();
		// 자리마다 다른 시드라서 인벤토리의 내용이 서로 다르고, 실행마다 같다.
		Inventory->Fill(Config.InventoryItems, Config.Seed + 1000 + Player.GetSlot());
		Inventories.Add(Inventory);
	}
}

void ALabGameMode::StartElementTimers()
{
	const FLabServerConfig& Config = FLabServerConfig::Get();

	// 타이머 하나가 전체를 돌아가며 바꾼다. 주기를 대상의 수로 나눠서, 대상 하나로 보면 인자로 준 간격이 된다.
	// 프레임이 주기보다 길면 타이머 매니저가 밀린 횟수만큼 한 프레임에 부르므로(TimerManager.cpp의 CallCount)
	// 서버가 느린 구성에서도 초당 바뀌는 횟수가 같다. 시작 신호 뒤에 들어온 플레이어는 주기에 반영하지 않는다.
	if (Config.StateIntervalSeconds > 0.f && StateComponents.Num() > 0)
	{
		StateRng.Initialize(Config.Seed + 1);
		GetWorldTimerManager().SetTimer(StateTimer, this, &ALabGameMode::ChangeOneState,
			Config.StateIntervalSeconds / StateComponents.Num(), true);
	}

	if (Config.InventoryChurnSeconds > 0.f && Inventories.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(ChurnTimer, this, &ALabGameMode::ChurnOneInventory,
			Config.InventoryChurnSeconds / Inventories.Num(), true);
	}

	if (Config.BuildIntervalSeconds > 0.f && BuildingClusters.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(RebuildTimer, this, &ALabGameMode::RebuildOne,
			Config.BuildIntervalSeconds / BuildingClusters.Num(), true);
	}
}

void ALabGameMode::ChangeOneState()
{
	// 난수는 대상이 사라졌어도 같은 횟수만큼 뽑아, 남은 대상의 순서가 달라지지 않게 한다.
	const int32 Index = StateRng.RandHelper(StateComponents.Num());
	if (ULabStateComponent* State = StateComponents[Index].Get())
	{
		State->ChangeOne(StateRng);
	}
}

void ALabGameMode::ChurnOneInventory()
{
	if (ULabInventoryComponent* Inventory = Inventories[NextChurnIndex % Inventories.Num()].Get())
	{
		Inventory->Churn();
	}
	++NextChurnIndex;
}

int32 ALabGameMode::CountReadyPlayers() const
{
	int32 NumReady = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const ALabPlayerController* Player = Cast<ALabPlayerController>(It->Get());
		if (Player && Player->IsReady())
		{
			++NumReady;
		}
	}
	return NumReady;
}

void ALabGameMode::HandlePlayerReady(ALabPlayerController& Player)
{
	if (bScenarioStarted)
	{
		PlaceAndStart(Player);
		AddPlayerElements(Player);
	}
	else if (CountReadyPlayers() >= FLabServerConfig::Get().ExpectedClients)
	{
		// 측정 실행의 시작 조건. 측정이 아닌 실행은 BeginPlay에서 이미 시작했으므로 여기에 오지 않는다.
		StartScenario();
	}
}

void ALabGameMode::PlaceAndStart(ALabPlayerController& Player)
{
	APawn* Pawn = Player.GetPawn();
	if (!Pawn)
	{
		return;
	}

	const int32 Slot = Player.GetSlot();

	// 1막에서는 모든 자리가 정사각형의 0번 꼭짓점에서 같은 방향으로 출발한다.
	float Side = ALabPlayerController::WaypointSide;
	FVector StartOffset = FVector::ZeroVector;
	int32 FirstWaypoint = 1;
	bool bReverse = false;

	// 간격을 준 배치에서는 자리 번호로 출발 위치와 방향을 정한다. 난수를 쓰지 않아 실행마다 같다.
	// 짝수 자리는 꼭짓점에서 1막의 방향으로, 홀수 자리는 변의 가운데에서 반대 방향으로 출발한다.
	// 반대 방향으로 도는 두 경로가 만나는 점에 두 캐릭터가 동시에 닿는 것은, 두 출발 위치를 둘레를 따라 잰 거리의 합이
	// 변의 두 배(둘레로 나눈 나머지)일 때뿐이다. 이 배정에서는 합이 변의 정수배가 아니라서 부딪히지 않는다.
	if (FLabServerConfig::Get().PlayerSpacingMeters > 0.f)
	{
		Side = ALabPlayerController::SpacedWaypointSide;
		const int32 Quarter = (Slot / 2) % 4;
		bReverse = Slot % 2 == 1;
		if (bReverse)
		{
			StartOffset = 0.5f * (ALabPlayerController::GetWaypointCorner(Quarter, Side) + ALabPlayerController::GetWaypointCorner(Quarter + 1, Side));
			FirstWaypoint = Quarter;
		}
		else
		{
			StartOffset = ALabPlayerController::GetWaypointCorner(Quarter, Side);
			FirstWaypoint = (Quarter + 1) % 4;
		}
	}

	int32& UseCount = SlotUseCount.FindOrAdd(Slot);
	const FVector RouteOrigin = GetSlotLocation(Slot) + FVector(0.f, 200.f * UseCount, 0.f);
	const FVector Location = RouteOrigin + StartOffset;
	++UseCount;

	Pawn->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	Player.ClientStartScenario(Location, RouteOrigin, Side, FirstWaypoint, bReverse);
}
