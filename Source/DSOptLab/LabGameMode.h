#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Math/RandomStream.h"
#include "LabGameMode.generated.h"

class ALabBuilding;
class ALabPlayerController;
class ULabInventoryBase;
class ULabStateComponent;

/** 서버 시작 시 월드를 생성하고, 공통 시작 신호로 NPC와 플레이어를 출발시킨다. */
UCLASS()
class DSOPTLAB_API ALabGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALabGameMode();

	virtual void BeginPlay() override;

	/** 플레이어가 준비를 보고했을 때 호출된다. 시작 조건이 채워지면 시작하고, 이미 시작한 뒤라면 그 플레이어를 바로 출발시킨다. */
	void HandlePlayerReady(ALabPlayerController& Player);

	/** 공통 시작 신호를 낸 직후에 한 번 불린다. 측정 서브시스템이 준비 구간을 여기서 시작한다. */
	FSimpleMulticastDelegate OnScenarioStarted;

	static FVector GetSlotLocation(int32 Slot);

private:
	void SpawnWorld();

	/** NPC의 배회를 시작하고, 준비된 플레이어를 자리로 옮겨 출발시킨다. 한 번만 동작한다. */
	void StartScenario();

	void PlaceAndStart(ALabPlayerController& Player);
	int32 CountReadyPlayers() const;

	// 2막의 확장 요소(상태 값, 인벤토리, 건축물, 플레이어 주변의 NPC). 인자를 주지 않으면 아무것도 만들지 않는다.
	void AddStateComponent(AActor& Actor);
	void AddPlayerElements(ALabPlayerController& Player);
	void StartElementTimers();
	void ChangeOneState();
	void ChurnOneInventory();

	/** 플레이어 주변에 놓는 것(건축물, NPC)의 한 무리. 경로가 서로 Net Cull Distance 안에 있는 자리들이 한 무리를 함께 쓴다. */
	struct FPlayerCluster
	{
		FVector FirstRouteCenter = FVector::ZeroVector;
		FVector Center = FVector::ZeroVector;
		int32 NumSlots = 0;
		/** 이 무리를 쓰는 자리들의 경로(정사각형)의 0번 꼭짓점. 건축물을 경로에서 띄워 놓는 데 쓴다. */
		TArray<FVector> RouteOrigins;
		/** 지은 순서. 맨 앞이 가장 오래된 것이다. */
		TArray<TWeakObjectPtr<ALabBuilding>> Buildings;
	};

	void BuildPlayerClusters();
	void SpawnBuildings();
	void SpawnBuilding(FPlayerCluster& Cluster);
	void RebuildOne();
	void SpawnClusterNpcs();

	/** 건축물 무리의 반지름(cm). 밀집 배치의 경로 상자는 무리 중심에서 60m 안이라, 무리의 건축물은 모든 플레이어에게서 140m 안에 있다. */
	static constexpr float BuildingClusterRadius = 8000.f;
	/**
	 * 건축물의 중심을 경로의 변에서 띄우는 거리(cm). 3인칭 카메라는 캐릭터에서 4m 떨어져 있고(LabCharacter.cpp의 TargetArmLength),
	 * 한 변 2m 정육면체의 중심에서 모서리까지가 1.41m다. 이보다 가까우면 건축물이 화면을 가린다.
	 */
	static constexpr float BuildingRouteClearance = 600.f;
	/**
	 * NPC 무리의 반지름(cm). NPC는 시작 위치에서 대각선으로 42.4m(30m × √2)까지 움직이므로,
	 * 밀집 배치에서 모든 플레이어에게서 142.4m(40 + 42.4 + 60) 안에 있다.
	 */
	static constexpr float NpcClusterRadius = 4000.f;
	/** 경로 중심이 이 거리(Net Cull Distance, cm) 안인 자리들은 한 무리를 쓴다. */
	static constexpr float ClusterMergeDistance = 15000.f;

	TArray<FPlayerCluster> PlayerClusters;
	FRandomStream BuildingRng;
	int32 NextRebuildCluster = 0;
	FTimerHandle RebuildTimer;

	bool bScenarioStarted = false;

	/** 상태 값을 붙인 순서. NPC는 스폰한 순서, 플레이어는 자리 번호 순서라서 실행마다 같다. */
	TArray<TWeakObjectPtr<ULabStateComponent>> StateComponents;

	/** 인벤토리를 붙인 순서(자리 번호 순서). */
	TArray<TWeakObjectPtr<ULabInventoryBase>> Inventories;

	FRandomStream StateRng;
	int32 NextChurnIndex = 0;
	FTimerHandle StateTimer;
	FTimerHandle ChurnTimer;

	/** 같은 자리를 여러 플레이어가 쓸 때 겹치지 않게 옆으로 밀기 위한 사용 횟수. */
	TMap<int32, int32> SlotUseCount;
};
