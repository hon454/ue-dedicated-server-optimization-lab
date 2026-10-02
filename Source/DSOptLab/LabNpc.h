#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "LabNpc.generated.h"

class UStaticMeshComponent;

/** 서버에서 시작 위치 주변을 배회하는 NPC. 이동만 리플리케이트한다. */
UCLASS()
class DSOPTLAB_API ALabNpc : public AActor
{
	GENERATED_BODY()

public:
	ALabNpc();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** 서버 전용. 공통 시작 신호에서 호출한다. 그 전에는 움직이지 않는다. */
	void StartWandering();

	/** 서버 전용. 배회하지 않고 두 점 사이의 직선을 MoveSpeed로 왕복하게 한다. 시작 신호 전에 호출한다. */
	void SetPatrol(const FVector& Start, const FVector& End);

private:
	static constexpr float WanderRadius = 3000.f;
	static constexpr float MoveSpeed = 300.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	void PickTarget();

	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FRandomStream Rng;

	bool bPatrol = false;
	FVector PatrolStart = FVector::ZeroVector;
	FVector PatrolEnd = FVector::ZeroVector;
	float PatrolTime = 0.f;
};
