#include "LabInventoryFastArrayComponent.h"

#include "Net/UnrealNetwork.h"

void ULabInventoryFastArrayComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.Condition = GetItemsCondition();
	DOREPLIFETIME_WITH_PARAMS(ULabInventoryFastArrayComponent, Inventory, Params);
}

FLabFastItem ULabInventoryFastArrayComponent::MakeFastItem()
{
	const FLabItem Item = MakeItem();
	FLabFastItem FastItem;
	FastItem.ItemId = Item.ItemId;
	FastItem.Count = Item.Count;
	return FastItem;
}

void ULabInventoryFastArrayComponent::Fill(int32 NumItems, int32 Seed)
{
	Rng.Initialize(Seed);
	Inventory.Items.Reset(NumItems);
	for (int32 Index = 0; Index < NumItems; ++Index)
	{
		// 새 칸은 MarkItemDirty가 번호를 정하고 키를 올린다. 배열 키도 함께 오른다.
		Inventory.MarkItemDirty(Inventory.Items.Add_GetRef(MakeFastItem()));
	}
}

void ULabInventoryFastArrayComponent::Churn()
{
	if (Inventory.Items.IsEmpty())
	{
		return;
	}

	// 지우기만 한 것은 MarkArrayDirty로 알린다. 남은 칸은 자리가 당겨져도 번호와 키가 그대로라 보내지 않는다.
	Inventory.Items.RemoveAt(0);
	Inventory.MarkArrayDirty();
	Inventory.MarkItemDirty(Inventory.Items.Add_GetRef(MakeFastItem()));
}

void ULabInventoryFastArrayComponent::AddHarvest()
{
	if (Inventory.Items.IsEmpty())
	{
		return;
	}

	FLabFastItem& Item = Inventory.Items[HarvestCursor % Inventory.Items.Num()];
	++Item.Count;
	Inventory.MarkItemDirty(Item);
	++HarvestCursor;
}

int32 ULabInventoryFastArrayComponent::GetNewestItemId() const
{
	// 번호는 서버가 칸을 더할 때마다 하나씩 올려 정하고, 클라이언트도 받은 번호를 그대로 가진다.
	// 클라이언트의 칸 순서는 서버와 다르므로 맨 뒤 칸이 아니라 번호가 가장 큰 칸을 찾는다.
	const FLabFastItem* Newest = nullptr;
	for (const FLabFastItem& Item : Inventory.Items)
	{
		if (!Newest || Item.ReplicationID > Newest->ReplicationID)
		{
			Newest = &Item;
		}
	}
	return Newest ? Newest->ItemId : 0;
}

void ULabInventoryFastArrayComponent::CopyItems(TArray<FLabItem>& OutItems) const
{
	OutItems.Reset(Inventory.Items.Num());
	for (const FLabFastItem& Item : Inventory.Items)
	{
		FLabItem& Copy = OutItems.AddDefaulted_GetRef();
		Copy.ItemId = Item.ItemId;
		Copy.Count = Item.Count;
	}
}
