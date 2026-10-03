#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabBuilding.generated.h"

class UStaticMeshComponent;

/**
 * 플레이어 주변에 몰려 있는 건축물. 2막의 확장 요소다(-LabBuildings=).
 * 움직이지 않고 체력과 단계만 리플리케이트한다. 자원 노드와 다른 점은 밀도다.
 * Net Cull Distance 안에 몰려 있어서 Relevancy만으로는 줄지 않는다.
 */
UCLASS()
class DSOPTLAB_API ALabBuilding : public AActor
{
	GENERATED_BODY()

public:
	ALabBuilding();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(Replicated)
	int32 Health = 100;

	UPROPERTY(Replicated)
	int32 Tier = 1;
};
