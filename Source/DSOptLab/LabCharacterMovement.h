#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LabCharacterMovement.generated.h"

/**
 * 수동 조작용 달리기를 넣은 이동 컴포넌트.
 * 달리기 여부는 저장된 이동의 압축 플래그(FLAG_Custom_0) 한 비트로 서버에 보낸다.
 * 이미 보내는 플래그 바이트에 들어가므로 송신량이 늘지 않고, 리플리케이트되는 프로퍼티도 RPC도 없다.
 * 시나리오의 클라이언트는 달리지 않으므로 측정에서는 걷기 속도만 쓰인다.
 */
UCLASS()
class DSOPTLAB_API ULabCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	/** 달리기 속도. 걷기 속도(ALabCharacter의 MaxWalkSpeed 500)의 두 배다. */
	UPROPERTY(EditDefaultsOnly, Category="Lab")
	float SprintSpeed = 1000.f;

	/** 로컬 입력에서 켜고 끈다. 서버에서는 압축 플래그로 채워진다. */
	bool bWantsToSprint = false;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};
