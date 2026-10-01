#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabResourceNode.generated.h"

class UStaticMeshComponent;

/** 채집할 수 있는 자원 노드. 체력과 고갈 여부만 리플리케이트한다. */
UCLASS()
class DSOPTLAB_API ALabResourceNode : public AActor
{
	GENERATED_BODY()

public:
	ALabResourceNode();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버 전용. 체력을 1 줄이고, 0이 되면 고갈시킨다. */
	void Harvest();

	bool IsDepleted() const { return bDepleted; }

private:
	static constexpr int32 MaxHealth = 3;
	static constexpr float RespawnSeconds = 20.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(Replicated)
	int32 Health = MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Depleted)
	bool bDepleted = false;

	UFUNCTION()
	void OnRep_Depleted();

	void Respawn();

	FTimerHandle RespawnTimer;
};
