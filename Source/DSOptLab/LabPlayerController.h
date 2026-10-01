#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LabPlayerController.generated.h"

class ACameraActor;
class UInputMappingContext;

/** 준비 보고, 자동 이동과 자동 채집, 채집 RPC, 시각 자료용 화면 표시를 담당한다. */
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

	/** ALabHUD가 그리는 화면 글자. 로컬 컨트롤러에서 매 틱 갱신한다. */
	const TArray<FString>& GetOverlayLines() const { return OverlayLines; }

private:
	static constexpr float HarvestRange = 500.f;
	static constexpr float HarvestInterval = 2.f;
	static constexpr float MinServerHarvestInterval = 1.f;
	static constexpr float WaypointSide = 10000.f;
	static constexpr float WaypointReachDistance = 200.f;
	static constexpr float TopDownHeight = 35000.f;
	static constexpr float ScreenshotInterval = 15.f;

	void TickAutoMove(APawn& ControlledPawn);

	/** 라벨, 자리와 역할, 시작 신호 후 경과 시간, 이 클라이언트에 존재하는 노드와 NPC와 플레이어 수, 폰 위치로 화면 글자를 만들고,
	 *  내려다보기 화면에서는 노드와 NPC와 플레이어 위치에 점을 그린다. */
	void TickOverlay(const APawn& ControlledPawn);

	void TickTopDown(const APawn& ControlledPawn);
	void TickAutoScreenshot(float DeltaTime);

	/** 수동 조작용 입력 매핑. 로컬 플레이어에게만 등록한다. */
	UPROPERTY()
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	UPROPERTY()
	TObjectPtr<ACameraActor> TopDownCamera;

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
	float ScreenshotAccumulator = 0.f;
	int32 ScreenshotIndex = 0;
	TArray<FString> OverlayLines;
};
