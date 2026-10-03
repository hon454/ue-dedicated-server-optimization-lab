#include "LabStateComponent.h"

#include "Math/RandomStream.h"
#include "Net/UnrealNetwork.h"

ULabStateComponent::ULabStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULabStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULabStateComponent, Health);
	DOREPLIFETIME(ULabStateComponent, MaxHealth);
	DOREPLIFETIME(ULabStateComponent, Level);
	DOREPLIFETIME(ULabStateComponent, StatusFlags);
	DOREPLIFETIME(ULabStateComponent, Stamina);
	DOREPLIFETIME(ULabStateComponent, Shield);
	DOREPLIFETIME(ULabStateComponent, SpeedScale);
	DOREPLIFETIME(ULabStateComponent, Threat);
}

void ULabStateComponent::ChangeOne(FRandomStream& Rng)
{
	// 어느 경우든 값이 전과 달라지게 한다. 같은 값을 다시 쓰면 보낼 것이 없다.
	switch (Rng.RandHelper(NumValues))
	{
	case 0: Health = Health > 1 ? Health - 1 : MaxHealth; break;
	case 1: ++MaxHealth; break;
	case 2: ++Level; break;
	case 3: StatusFlags ^= 1 << Rng.RandHelper(8); break;
	case 4: Stamina = Stamina > 1.f ? Stamina - 1.f : 100.f; break;
	case 5: Shield += 1.f; break;
	case 6: SpeedScale = SpeedScale < 1.5f ? SpeedScale + 0.05f : 1.f; break;
	default: Threat += 1.f; break;
	}
}
