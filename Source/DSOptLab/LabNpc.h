#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "LabNpc.generated.h"

class UStaticMeshComponent;

/**
 * NPC 한 번의 이동(포스팅 12, -LabNpcCompactMove). 엔진의 FRepMovement 대신 평면 이동에 필요한 것만 담는다.
 * FRepMovement는 핸들을 빼고 33 + 3N비트(N은 위치 성분당 비트 수)를 쓰고, 그중 Z, 속도, 위치 머리, 늘 같은 플래그는 NPC에게 필요 없다
 * (ReplicatedState.cpp:67-152, engine-notes.md 13절). 이 구조체는 X, Y 13비트씩, Yaw 8비트, 서버 프레임 번호 8비트로 42비트다.
 */
USTRUCT()
struct FLabNpcMove
{
	GENERATED_BODY()

	/** 기준점(ALabNpc::MoveOrigin)에서 잰 평면 위치(cm). 서버는 반올림하기 전의 값을 넣고, 직렬화할 때 1cm로 반올림한다. */
	UPROPERTY()
	FVector2D Offset = FVector2D::ZeroVector;

	/** 엔진의 ByteComponents와 같은 1바이트(약 1.41°)로 보낸다. NPC는 평면에서만 돌아 Pitch와 Roll이 늘 0이다. */
	UPROPERTY()
	float Yaw = 0.f;

	/** 이 위치를 정한 서버 프레임 번호의 아래 8비트. 포스팅 11의 보간이 쓴다(ALabNpc::ServerFrame과 같은 값). */
	UPROPERTY()
	uint8 ServerFrame = 0;

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FLabNpcMove> : public TStructOpsTypeTraitsBase2<FLabNpcMove>
{
	enum
	{
		WithNetSerializer = true,
		// 프레임마다 한 번 직렬화한 결과를 모든 연결이 함께 쓴다(RepLayout.cpp:5555-5557). FRepMovement와 같다.
		WithNetSharedSerialization = true,
	};
};

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

	/**
	 * 평면 이동(포스팅 12). -LabNpcCompactMove일 때만 ReplicatedMovement와 ServerFrame 대신 보낸다.
	 * 서버가 프레임 번호를 넣으므로 NPC가 리플리케이트될 때마다 간다.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Move)
	FLabNpcMove Move;

	/**
	 * Move.Offset의 기준점. 서버의 Home을 cm 정수로 반올림한 값이라 FVector_NetQuantize로 잃는 것이 없다.
	 * 채널이 열릴 때 한 번만 보낸다(COND_InitialOnly). Home 자체를 반올림하면 난수 시드(GetTypeHash(Home))가 바뀌어 경로가 기준 구성과 달라진다.
	 */
	UPROPERTY(Replicated)
	FVector_NetQuantize MoveOrigin = FVector_NetQuantize::ZeroVector;

	UFUNCTION()
	void OnRep_Move();

	void PickTarget();
	void TickInterpolation();
	/** 클라이언트 전용. 받은 위치를 보간 버퍼에 넣는다. 처음 받은 위치라서 바로 놓아야 하면 true다. */
	bool AddSnapshot(const FVector& Location, const FQuat& Rotation, uint8 Frame);
	void RecordReceive(uint8 Frame);
	static bool IsInterpolating();
	static bool IsCompactMove();

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
