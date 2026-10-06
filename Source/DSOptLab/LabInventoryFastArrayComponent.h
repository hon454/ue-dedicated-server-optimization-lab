#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LabInventoryComponent.h"
#include "LabInventoryFastArrayComponent.generated.h"

/** FastArray 인벤토리의 한 칸. 부모가 칸 번호(ReplicationID)와 바뀐 횟수(ReplicationKey)를 가진다. 둘은 리플리케이트하지 않는다. */
USTRUCT()
struct FLabFastItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemId = 0;

	UPROPERTY()
	int32 Count = 0;
};

/**
 * 칸 배열과 FastArray의 상태(배열 키, 번호 표). 연결마다 지난번에 보낸 번호와 키를 기억하고,
 * 키가 바뀐 칸과 새 칸, 없어진 번호만 보낸다(FastArraySerializer.h의 BuildChangedAndDeletedBuffers).
 */
USTRUCT()
struct FLabFastItemArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLabFastItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FLabFastItem, FLabFastItemArray>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FLabFastItemArray> : public TStructOpsTypeTraitsBase2<FLabFastItemArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 * FastArray로 칸을 보내는 인벤토리. 인자 -LabInventoryFastArray를 주면 ULabInventoryComponent 대신 붙는다(포스팅 10).
 * 하는 일(같은 시드의 같은 아이템, 맨 앞 칸 지우기, 채집)은 ULabInventoryComponent와 같다.
 * 클라이언트는 새 칸을 맨 뒤에 더하고 지운 칸을 RemoveAtSwap으로 지우므로 칸 순서가 서버와 달라진다.
 */
UCLASS()
class DSOPTLAB_API ULabInventoryFastArrayComponent : public ULabInventoryBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Fill(int32 NumItems, int32 Seed) override;
	virtual void Churn() override;
	virtual void AddHarvest() override;

	virtual int32 GetNumItems() const override { return Inventory.Items.Num(); }
	virtual int32 GetNewestItemId() const override;
	virtual void CopyItems(TArray<FLabItem>& OutItems) const override;

private:
	FLabFastItem MakeFastItem();

	UPROPERTY(Replicated)
	FLabFastItemArray Inventory;
};
