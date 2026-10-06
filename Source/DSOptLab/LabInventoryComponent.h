#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/RandomStream.h"
#include "LabInventoryComponent.generated.h"

/** 인벤토리의 한 칸. */
USTRUCT()
struct FLabItem
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemId = 0;

	UPROPERTY()
	int32 Count = 0;

	bool operator==(const FLabItem& Other) const { return ItemId == Other.ItemId && Count == Other.Count; }
};

/**
 * 플레이어 캐릭터의 인벤토리. 2막의 확장 요소다(-LabInventoryItems=).
 * 서버가 캐릭터에 실행 중에 붙인다. 인자를 주지 않으면 만들지 않으므로 1막의 캐릭터는 그대로다.
 * 일반 TArray 프로퍼티이고 캐릭터가 고려될 때마다 모든 칸을 비교한다.
 * 기본은 조건 없이 모든 연결에 보내고, -LabInventoryOwnerOnly를 주면 소유자의 연결에만 보낸다(포스팅 9).
 */
UCLASS()
class DSOPTLAB_API ULabInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULabInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버 전용. 시드로 정한 아이템을 NumItems칸 채운다. */
	void Fill(int32 NumItems, int32 Seed);

	/** 서버 전용. 맨 앞 칸을 지우고 맨 뒤에 새 칸을 더한다. 뒤의 칸이 모두 한 칸씩 당겨진다. */
	void Churn();

	/** 서버 전용. 채집에 성공했을 때 한 칸의 수량을 하나 늘린다. */
	void AddHarvest();

	int32 GetNumItems() const { return Items.Num(); }
	const TArray<FLabItem>& GetItems() const { return Items; }
	int32 GetFirstItemId() const { return Items.IsEmpty() ? 0 : Items[0].ItemId; }

private:
	FLabItem MakeItem();

	UPROPERTY(Replicated)
	TArray<FLabItem> Items;

	// 서버에서 쓰는 상태
	FRandomStream Rng;
	int32 HarvestCursor = 0;
};
