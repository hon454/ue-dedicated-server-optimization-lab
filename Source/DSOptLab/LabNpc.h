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

protected:
	static constexpr float MoveSpeed = 300.f;

private:
	static constexpr float WanderRadius = 3000.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	void PickTarget();

	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FRandomStream Rng;
};

/**
 * 영상용 NPC. 배회하지 않고 두 점 사이의 직선을 MoveSpeed로 왕복한다.
 * 리플리케이션 설정은 ALabNpc에서 물려받아 같다. 수치를 쓰지 않는 visualN 실행(-LabShowcaseNpc)에서만 스폰한다.
 */
UCLASS()
class DSOPTLAB_API ALabShowcaseNpc : public ALabNpc
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	/** 서버 전용. 왕복할 두 점을 정한다. 시작 신호 전에 호출한다. */
	void SetPatrol(const FVector& Start, const FVector& End);

private:
	FVector PatrolStart = FVector::ZeroVector;
	FVector PatrolEnd = FVector::ZeroVector;
	float PatrolTime = 0.f;
};
