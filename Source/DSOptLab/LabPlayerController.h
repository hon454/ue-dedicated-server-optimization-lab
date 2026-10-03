#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LabPlayerController.generated.h"

class UInputMappingContext;

/** 준비 보고, 자동 이동과 자동 채집, 채집 RPC를 담당한다. 화면 표시는 ALabHUD가 한다. */
UCLASS()
class DSOPTLAB_API ALabPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALabPlayerController();

	virtual void PlayerTick(float DeltaTime) override;
	virtual void SpawnPlayerCameraManager() override;

protected:
	virtual void SetupInputComponent() override;

public:

	/** 클라이언트가 폰을 얻은 뒤 자기 자리 번호를 알린다. */
	UFUNCTION(Server, Reliable)
	void ServerReportReady(int32 InSlot);

	/** 자동 이동 경로인 정사각형의 한 변(cm). 1막의 값과, 플레이어가 모이는 배치(-LabPlayerSpacing=)의 값. */
	static constexpr float WaypointSide = 10000.f;
	static constexpr float SpacedWaypointSide = 7000.f;

	/** 정사각형의 꼭짓점 번호(0~3)를 첫 꼭짓점에서 잰 위치로 바꾼다. 번호가 커지는 쪽이 1막의 도는 방향이다. */
	static FVector GetWaypointCorner(int32 Corner, float Side);

	/**
	 * 서버의 공통 시작 신호. 서버가 옮겨 놓은 시작 위치와 돌 경로를 함께 받는다.
	 * 경로는 InRouteOrigin을 0번 꼭짓점으로 하는 정사각형이다. FirstWaypoint번 꼭짓점으로 먼저 가고, bReverse면 1막과 반대 방향으로 돈다.
	 */
	UFUNCTION(Client, Reliable)
	void ClientStartScenario(FVector StartLocation, FVector InRouteOrigin, float Side, int32 FirstWaypoint, bool bReverse);

	/** 폰 주변에서 가장 가까운 노드를 채집한다. */
	UFUNCTION(Server, Reliable)
	void ServerHarvest();

	/** 채집할 수 있는 거리(cm). 폰과 노드의 수평 거리로 잰다. */
	static constexpr float HarvestRange = 500.f;

	bool IsReady() const { return Slot != INDEX_NONE; }
	int32 GetSlot() const { return Slot; }

	/** 클라이언트에서 공통 시작 신호를 받았는지와 받은 시각(월드 시간). ALabHUD가 읽는다. */
	bool IsScenarioStarted() const { return bScenarioStarted; }
	double GetScenarioStartTime() const { return ScenarioStartTime; }

private:
	static constexpr float HarvestInterval = 2.f;
	static constexpr float MinServerHarvestInterval = 1.f;
	static constexpr float WaypointReachDistance = 200.f;
	static constexpr float RouteEnterDistance = 500.f;

	void TickAutoMove(APawn& ControlledPawn);

	/** 수동 조작용 입력 매핑. 로컬 플레이어에게만 등록한다. */
	UPROPERTY()
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	// 서버에서 쓰는 상태
	int32 Slot = INDEX_NONE;
	double LastHarvestTime = -1000.0;

	// 클라이언트에서 쓰는 상태
	bool bReportedReady = false;
	bool bScenarioStarted = false;
	double ScenarioStartTime = 0.0;
	FVector RouteStart = FVector::ZeroVector;
	bool bRouteEntered = false;
	FVector RouteOrigin = FVector::ZeroVector;
	float RouteSide = WaypointSide;
	bool bRouteReverse = false;
	int32 WaypointIndex = 0;
	float HarvestAccumulator = 0.f;
};
