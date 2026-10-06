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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void PostNetReceiveLocationAndRotation() override;

	/** 서버 전용. 공통 시작 신호에서 호출한다. 그 전에는 움직이지 않는다. */
	void StartWandering();

	/** 화면에 그려지는 위치(메시의 월드 위치). 품질 지표가 기록하는 위치다(ADR-0020). */
	FVector GetVisualLocation() const;

protected:
	static constexpr float MoveSpeed = 300.f;

	/** 서버 전용. 프레임마다 위치를 옮긴다. */
	virtual void TickMovement(float DeltaSeconds);

private:
	static constexpr float WanderRadius = 3000.f;

	/** 클라이언트에서 받은 위치 하나. Frame은 펼친 서버 프레임 번호다. 보간은 이 번호를 시각축으로 쓴다. */
	struct FSnapshot
	{
		double Frame = 0.0;
		FVector Location = FVector::ZeroVector;
		FQuat Rotation = FQuat::Identity;
	};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	/**
	 * 이 위치를 정한 서버 프레임 번호의 아래 8비트(30Hz에서 8.5초마다 한 바퀴).
	 * 서버가 프레임마다 바꾸므로 NPC가 리플리케이트될 때마다 ReplicatedMovement와 함께 간다.
	 * 클라이언트가 받은 위치를 서버 시각축에 놓는 데 쓴다. 보간을 끈 실행에서는 보내지 않는다(COND_Never).
	 */
	UPROPERTY(Replicated)
	uint8 ServerFrame = 0;

	void PickTarget();
	void TickInterpolation();
	static bool IsInterpolating();

	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FRandomStream Rng;

	// 클라이언트에서 쓰는 상태(보간). 서버 시각 순서로 쌓인다.
	TArray<FSnapshot> Snapshots;
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
	/** 서버 전용. 왕복할 두 점을 정한다. 시작 신호 전에 호출한다. */
	void SetPatrol(const FVector& Start, const FVector& End);

protected:
	virtual void TickMovement(float DeltaSeconds) override;

private:
	FVector PatrolStart = FVector::ZeroVector;
	FVector PatrolEnd = FVector::ZeroVector;
	float PatrolTime = 0.f;
};
