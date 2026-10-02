#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LabGameMode.generated.h"

class ALabPlayerController;

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

	bool bScenarioStarted = false;

	/** 같은 자리를 여러 플레이어가 쓸 때 겹치지 않게 옆으로 밀기 위한 사용 횟수. */
	TMap<int32, int32> SlotUseCount;
};
