#include "LabCharacterMovement.h"

#include "GameFramework/Character.h"

namespace
{
	/** 달리기 여부를 함께 저장하는 이동. 클라이언트가 서버로 보내고, 보정을 받으면 다시 재생한다. */
	class FLabSavedMove : public FSavedMove_Character
	{
		using Super = FSavedMove_Character;

	public:
		bool bSavedWantsToSprint = false;

		virtual void Clear() override
		{
			Super::Clear();
			bSavedWantsToSprint = false;
		}

		virtual uint8 GetCompressedFlags() const override
		{
			uint8 Result = Super::GetCompressedFlags();
			if (bSavedWantsToSprint)
			{
				Result |= FLAG_Custom_0;
			}
			return Result;
		}

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
		{
			// 달리기 상태가 바뀌는 두 이동을 합치면 서버가 바뀐 시점을 알 수 없다.
			if (bSavedWantsToSprint != static_cast<const FLabSavedMove*>(NewMove.Get())->bSavedWantsToSprint)
			{
				return false;
			}
			return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
		}

		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override
		{
			Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
			if (const ULabCharacterMovement* Movement = Cast<ULabCharacterMovement>(C->GetCharacterMovement()))
			{
				bSavedWantsToSprint = Movement->bWantsToSprint;
			}
		}

		virtual void PrepMoveFor(ACharacter* C) override
		{
			Super::PrepMoveFor(C);
			if (ULabCharacterMovement* Movement = Cast<ULabCharacterMovement>(C->GetCharacterMovement()))
			{
				Movement->bWantsToSprint = bSavedWantsToSprint;
			}
		}
	};

	class FLabNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		explicit FLabNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
			: FNetworkPredictionData_Client_Character(ClientMovement)
		{
		}

		virtual FSavedMovePtr AllocateNewMove() override
		{
			return FSavedMovePtr(new FLabSavedMove());
		}
	};
}

float ULabCharacterMovement::GetMaxSpeed() const
{
	if (bWantsToSprint && IsMovingOnGround())
	{
		return SprintSpeed;
	}
	return Super::GetMaxSpeed();
}

void ULabCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* ULabCharacterMovement::GetPredictionData_Client() const
{
	// 엔진 구현(CharacterMovementComponent.cpp의 GetPredictionData_Client)과 같은 방식으로 처음 부를 때 만든다.
	if (ClientPredictionData == nullptr)
	{
		ULabCharacterMovement* MutableThis = const_cast<ULabCharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FLabNetworkPredictionData_Client(*this);
	}
	return ClientPredictionData;
}
