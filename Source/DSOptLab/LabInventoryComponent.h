#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
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
 * 칸을 담는 방식이 다른 두 클래스(일반 TArray, FastArray)의 공통 부모이고, 리플리케이트 프로퍼티는 없다.
 * 게임 모드, 채집, 화면은 이 클래스로 두 클래스를 같은 방식으로 부른다.
 */
UCLASS(Abstract)
class DSOPTLAB_API ULabInventoryBase : public UActorComponent
{
	GENERATED_BODY()

public:
	ULabInventoryBase();

	/** 서버 전용. 시드로 정한 아이템을 NumItems칸 채운다. */
	virtual void Fill(int32 NumItems, int32 Seed) {}

	/** 서버 전용. 맨 앞 칸을 지우고 맨 뒤에 새 칸을 더한다. */
	virtual void Churn() {}

	/** 서버 전용. 채집에 성공했을 때 한 칸의 수량을 하나 늘린다. */
	virtual void AddHarvest() {}

	virtual int32 GetNumItems() const { return 0; }

	/** 가장 최근에 더한 칸의 아이템 번호. 서버와 클라이언트에서 같은 값이다. */
	virtual int32 GetNewestItemId() const { return 0; }

	/** 칸을 이 컴포넌트가 가진 순서대로 복사한다. 클라이언트의 순서는 서버와 다를 수 있다(FastArray). */
	virtual void CopyItems(TArray<FLabItem>& OutItems) const { OutItems.Reset(); }

protected:
	/** 칸 프로퍼티의 리플리케이션 조건. -LabInventoryOwnerOnly를 주면 소유자의 연결에만 보낸다(포스팅 9). */
	static ELifetimeCondition GetItemsCondition();

	/** 시드로 정한 난수에서 새 칸을 만든다. 두 클래스가 같은 순서로 같은 아이템을 만든다. */
	FLabItem MakeItem();

	// 서버에서 쓰는 상태
	FRandomStream Rng;
	int32 HarvestCursor = 0;
};

/**
 * 일반 TArray 프로퍼티로 칸을 보내는 인벤토리. 인자 -LabInventoryFastArray가 없을 때 붙는다.
 * 캐릭터가 고려될 때마다 모든 칸을 같은 자리끼리 비교한다. 앞 칸을 지우면 뒤 칸이 당겨져 모두 바뀐 칸이 된다.
 * 기본은 조건 없이 모든 연결에 보내고, -LabInventoryOwnerOnly를 주면 소유자의 연결에만 보낸다(포스팅 9).
 */
UCLASS()
class DSOPTLAB_API ULabInventoryComponent : public ULabInventoryBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Fill(int32 NumItems, int32 Seed) override;
	virtual void Churn() override;
	virtual void AddHarvest() override;

	virtual int32 GetNumItems() const override { return Items.Num(); }
	virtual int32 GetNewestItemId() const override { return Items.IsEmpty() ? 0 : Items.Last().ItemId; }
	virtual void CopyItems(TArray<FLabItem>& OutItems) const override { OutItems = Items; }

private:
	UPROPERTY(Replicated)
	TArray<FLabItem> Items;
};
