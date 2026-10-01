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

	/** NPC의 배회를 시작하고, 준비된 플레이어를 자리로 옮겨 출발시킨다. 한 번만 동작한다. */
	void StartScenario();

	/** 플레이어가 준비를 보고했을 때 호출된다. 이미 시작한 뒤라면 그 플레이어를 바로 출발시킨다. */
	void HandlePlayerReady(ALabPlayerController& Player);

	static FVector GetSlotLocation(int32 Slot);

private:
	void SpawnWorld();
	void PlaceAndStart(ALabPlayerController& Player);

	bool bScenarioStarted = false;

	/** 같은 자리를 여러 플레이어가 쓸 때 겹치지 않게 옆으로 밀기 위한 사용 횟수. */
	TMap<int32, int32> SlotUseCount;
};
