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

	/** 서버의 공통 시작 신호. 서버가 옮겨 놓은 시작 위치를 함께 받는다. */
	UFUNCTION(Client, Reliable)
	void ClientStartScenario(FVector StartLocation);

	/** 폰 주변에서 가장 가까운 노드를 채집한다. */
	UFUNCTION(Server, Reliable)
	void ServerHarvest();

	bool IsReady() const { return Slot != INDEX_NONE; }
	int32 GetSlot() const { return Slot; }

	/** 클라이언트에서 공통 시작 신호를 받았는지와 받은 시각(월드 시간). ALabHUD가 읽는다. */
	bool IsScenarioStarted() const { return bScenarioStarted; }
	double GetScenarioStartTime() const { return ScenarioStartTime; }

private:
	static constexpr float HarvestRange = 500.f;
	static constexpr float HarvestInterval = 2.f;
	static constexpr float MinServerHarvestInterval = 1.f;
	static constexpr float WaypointSide = 10000.f;
	static constexpr float WaypointReachDistance = 200.f;

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
	FVector Home = FVector::ZeroVector;
	int32 WaypointIndex = 0;
	float HarvestAccumulator = 0.f;
};
