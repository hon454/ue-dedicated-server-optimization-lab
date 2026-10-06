#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LabInventoryComponent.h"
#include "LabHUD.generated.h"

class ACameraActor;
class ALabPlayerController;

/**
 * 시각 자료용 화면 표시. 화면 글자, 내려다보기 화면의 점과 카메라, 자동 스크린샷을 담당한다.
 * HUD는 로컬 플레이어의 클라이언트에만 생기므로, 여기의 코드는 서버에서 돌지 않는다.
 */
UCLASS()
class DSOPTLAB_API ALabHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	/** Tick에서 만든 화면 글자를 반투명 상자 위에 그린다. 자동 스크린샷에도 찍힌다. */
	virtual void DrawHUD() override;

private:
	static constexpr float Margin = 8.f;
	static constexpr float Padding = 6.f;
	static constexpr float TextScale = 1.4f;
	static constexpr float LineSpacing = 1.15f;
	static constexpr float TopDownHeight = 35000.f;
	static constexpr float ScreenshotInterval = 15.f;

	/** 라벨, 자리와 역할, 시작 신호 후 경과 시간, 이 클라이언트에 존재하는 노드와 NPC와 플레이어 수, 폰 위치로 화면 글자를 만들고,
	 *  내려다보기 화면에서는 노드와 NPC와 플레이어 위치에 점을 그린다. */
	void UpdateOverlay(const ALabPlayerController& Controller, const APawn& ControlledPawn);

	void UpdateTopDown(ALabPlayerController& Controller, const APawn& ControlledPawn);
	void UpdateAutoScreenshot(float DeltaSeconds);

	/** 인벤토리 패널(-LabInventoryPanel). 이 클라이언트에 있는 플레이어 캐릭터의 인벤토리를 칸 격자로 그린다.
	 *  칸 색은 아이템 번호, 이 클라이언트에서 값이 바뀐 칸은 잠깐 흰색, 받지 못한 칸은 회색이다. */
	void UpdateInventoryPanel(const APawn& ControlledPawn);
	void DrawInventoryPanel();

	/** 인벤토리 하나를 클라이언트가 지난 틱에 본 값과 비교하려고 기억한다. */
	struct FInventoryView
	{
		TArray<FLabItem> Previous;
		TArray<double> FlashUntil;
		bool bSeen = false;
	};

	struct FPanelEntry
	{
		TWeakObjectPtr<const ULabInventoryComponent> Inventory;
		bool bOwn = false;
	};

	void DrawInventoryGrid(const FPanelEntry& Entry, float X, float Y, float Cell, int32 NumCells, double Now);

	static constexpr int32 PanelColumns = 20;
	static constexpr double FlashSeconds = 0.4;

	TMap<TWeakObjectPtr<const ULabInventoryComponent>, FInventoryView> InventoryViews;
	TArray<FPanelEntry> PanelEntries;

	UPROPERTY()
	TObjectPtr<ACameraActor> TopDownCamera;

	TArray<FString> OverlayLines;
	float ScreenshotAccumulator = 0.f;
	int32 ScreenshotIndex = 0;
};
