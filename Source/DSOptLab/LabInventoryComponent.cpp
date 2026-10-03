#include "LabInventoryComponent.h"

#include "Net/UnrealNetwork.h"

ULabInventoryComponent::ULabInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULabInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULabInventoryComponent, Items);
}

FLabItem ULabInventoryComponent::MakeItem()
{
	FLabItem Item;
	Item.ItemId = Rng.RandRange(1, 1000);
	Item.Count = Rng.RandRange(1, 99);
	return Item;
}

void ULabInventoryComponent::Fill(int32 NumItems, int32 Seed)
{
	Rng.Initialize(Seed);
	Items.Reset(NumItems);
	for (int32 Index = 0; Index < NumItems; ++Index)
	{
		Items.Add(MakeItem());
	}
}

void ULabInventoryComponent::Churn()
{
	if (Items.IsEmpty())
	{
		return;
	}

	Items.RemoveAt(0);
	Items.Add(MakeItem());
}

void ULabInventoryComponent::AddHarvest()
{
	if (Items.IsEmpty())
	{
		return;
	}

	++Items[HarvestCursor % Items.Num()].Count;
	++HarvestCursor;
}
