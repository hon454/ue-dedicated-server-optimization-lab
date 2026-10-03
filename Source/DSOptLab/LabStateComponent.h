#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LabStateComponent.generated.h"

struct FRandomStream;

/**
 * 드물게 바뀌는 상태 값 여덟 개. 2막의 확장 요소다(-LabStateInterval=).
 * 서버가 NPC와 플레이어 캐릭터에 실행 중에 붙인다. 인자를 주지 않으면 만들지 않으므로 1막의 액터는 그대로다.
 * 액터가 리플리케이션 대상으로 고려될 때마다 여덟 값을 비교하고, 바뀐 값만 보낸다.
 */
UCLASS()
class DSOPTLAB_API ULabStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULabStateComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버 전용. 여덟 값 가운데 하나를 골라 다른 값으로 바꾼다. */
	void ChangeOne(FRandomStream& Rng);

private:
	static constexpr int32 NumValues = 8;

	UPROPERTY(Replicated)
	int32 Health = 100;

	UPROPERTY(Replicated)
	int32 MaxHealth = 100;

	UPROPERTY(Replicated)
	int32 Level = 1;

	UPROPERTY(Replicated)
	int32 StatusFlags = 0;

	UPROPERTY(Replicated)
	float Stamina = 100.f;

	UPROPERTY(Replicated)
	float Shield = 0.f;

	UPROPERTY(Replicated)
	float SpeedScale = 1.f;

	UPROPERTY(Replicated)
	float Threat = 0.f;
};
