#include "LabInventoryComponent.h"

#include "LabScenarioConfig.h"
#include "Net/UnrealNetwork.h"

ULabInventoryComponent::ULabInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULabInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 클래스마다 한 번 불리고 서버와 클라이언트에서 모두 돈다. 인자는 서버에만 주므로 클라이언트에서는 COND_None이다.
	// 조건은 보내는 쪽(서버)이 연결마다 바뀐 목록을 거르는 데만 쓴다(RepLayout.cpp의 FilterChangeListToActive).
	FDoRepLifetimeParams Params;
	Params.Condition = FLabServerConfig::Get().bInventoryOwnerOnly ? COND_OwnerOnly : COND_None;
	DOREPLIFETIME_WITH_PARAMS(ULabInventoryComponent, Items, Params);
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
