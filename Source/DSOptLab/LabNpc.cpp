#include "LabNpc.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/NetDriver.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "LabMotionLogSubsystem.h"
#include "LabScenarioConfig.h"
#include "LabVisual.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/**
	 * 클라이언트 전용. NPC 전체가 함께 쓰는 서버 시계(포스팅 11).
	 * 받은 8비트 프레임 번호를 지금까지 본 가장 큰 번호에 가장 가까운 값으로 펼친다. 모든 NPC의 갱신이 몇 프레임 안에 오므로 ±128프레임이면 충분하다.
	 * 받은 시각 - 서버 시각의 최솟값(최근 2초)을 두 시계의 차로 삼는다. 가장 빨리 도착한 갱신이 전달 지연이 가장 짧은 갱신이다.
	 */
	struct FLabServerClock
	{
		static constexpr double BucketSeconds = 0.5;
		static constexpr int32 NumBuckets = 4;

		int64 LatestFrame = 0;
		bool bHasFrame = false;
		double BucketMin[NumBuckets] = { DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX };
		double BucketStart = -1.0;
		int32 Bucket = 0;

		int64 Unwrap(uint8 Raw)
		{
			if (!bHasFrame)
			{
				bHasFrame = true;
				LatestFrame = Raw;
				return LatestFrame;
			}
			int32 Delta = (static_cast<int32>(Raw) - static_cast<int32>(LatestFrame & 0xFF)) & 0xFF;
			if (Delta >= 128)
			{
				Delta -= 256;
			}
			const int64 Frame = LatestFrame + Delta;
			LatestFrame = FMath::Max(LatestFrame, Frame);
			return Frame;
		}

		void AddSample(double ReceiveTime, double ServerTime)
		{
			if (BucketStart < 0.0)
			{
				BucketStart = ReceiveTime;
			}
			// 0.5초가 지날 때마다 가장 오래된 칸을 비운다. 2초 넘게 받지 못했으면 모두 비운다.
			const int32 Steps = FMath::Min(NumBuckets, static_cast<int32>((ReceiveTime - BucketStart) / BucketSeconds));
			for (int32 Step = 0; Step < Steps; ++Step)
			{
				Bucket = (Bucket + 1) % NumBuckets;
				BucketMin[Bucket] = DBL_MAX;
			}
			if (Steps > 0)
			{
				BucketStart = ReceiveTime;
			}
			BucketMin[Bucket] = FMath::Min(BucketMin[Bucket], ReceiveTime - ServerTime);
		}

		double GetOffset() const
		{
			double Offset = DBL_MAX;
			for (const double Value : BucketMin)
			{
				Offset = FMath::Min(Offset, Value);
			}
			return Offset;
		}
	};

	FLabServerClock& GetServerClock()
	{
		static FLabServerClock Clock;
		return Clock;
	}
}

ALabNpc::ALabNpc()
{
	PrimaryActorTick.bCanEverTick = true;
	// 시작 신호 전에는 틱하지 않는다. 실행마다 같은 시점에 같은 위치에 있게 하기 위해서다.
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	const FLabServerConfig& Config = FLabServerConfig::Get();

	// 켜면 거리와 무관하게 모든 연결에 보낸다(1막의 기준선).
	bAlwaysRelevant = Config.bAlwaysRelevant;

	// 기본값에서는 초당 10회만 리플리케이션 대상으로 고려한다.
	SetNetUpdateFrequency(Config.NpcUpdateFrequency);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
}

bool ALabNpc::IsInterpolating()
{
	return FLabScenarioConfig::Get().NpcInterpDelayMs > 0.f;
}

void ALabNpc::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 조건은 보내는 쪽(서버)이 거르는 데만 쓴다(LabInventoryComponent.cpp의 GetItemsCondition과 같다).
	// 보간을 끈 실행에서는 보내지 않아 기준 구성의 대역폭이 바뀌지 않는다.
	FDoRepLifetimeParams Params;
	Params.Condition = IsInterpolating() ? COND_None : COND_Never;
	DOREPLIFETIME_WITH_PARAMS(ALabNpc, ServerFrame, Params);
}

void ALabNpc::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	// 리플리케이트하기 직전, 같은 프레임에 모으는 위치(AActor::PreReplication의 GatherCurrentMovement)와 짝이 되는 프레임 번호다.
	if (IsInterpolating())
	{
		ServerFrame = static_cast<uint8>(GFrameCounter & 0xFF);
	}
	Super::PreReplication(ChangedPropertyTracker);
}

void ALabNpc::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Material = LabVisual::GetSharedMaterial(LabVisual::NpcColor))
	{
		Mesh->SetMaterial(0, Material);
	}

	if (HasAuthority())
	{
		Home = GetActorLocation();
		// 시작 위치로 시드를 정해, 같은 배치에서는 같은 경로로 움직이게 한다.
		Rng.Initialize(static_cast<int32>(GetTypeHash(Home)));
		PickTarget();
	}
	else if (IsInterpolating())
	{
		// 클라이언트는 받은 위치 사이를 프레임마다 보간한다.
		SetActorTickEnabled(true);
	}
}

void ALabNpc::StartWandering()
{
	if (HasAuthority())
	{
		SetActorTickEnabled(true);
	}
}

FVector ALabNpc::GetVisualLocation() const
{
	return Mesh->GetComponentLocation();
}

void ALabNpc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		TickMovement(DeltaSeconds);
	}
	else
	{
		TickInterpolation();
	}
}

void ALabNpc::TickMovement(float DeltaSeconds)
{
	const FVector Location = GetActorLocation();
	FVector ToTarget = Target - Location;
	ToTarget.Z = 0.f;

	const float Distance = ToTarget.Size();
	const float Step = MoveSpeed * DeltaSeconds;

	if (Distance <= Step)
	{
		SetActorLocation(Target);
		PickTarget();
		return;
	}

	const FVector Direction = ToTarget / Distance;
	SetActorLocationAndRotation(Location + Direction * Step, Direction.Rotation());
}

void ALabNpc::PickTarget()
{
	Target = Home + FVector(
		Rng.FRandRange(-WanderRadius, WanderRadius),
		Rng.FRandRange(-WanderRadius, WanderRadius),
		0.f);
}

void ALabNpc::PostNetReceiveLocationAndRotation()
{
	if (ULabMotionLogSubsystem* MotionLog = GetWorld()->GetSubsystem<ULabMotionLogSubsystem>())
	{
		MotionLog->RecordReceive(*this, ServerFrame);
	}

	// 보간을 끄면 엔진 기본 동작대로 받은 위치로 바로 옮긴다(ActorReplication.cpp의 AActor::PostNetReceiveLocationAndRotation).
	if (!IsInterpolating())
	{
		Super::PostNetReceiveLocationAndRotation();
		return;
	}

	// 같은 묶음의 프로퍼티는 모두 받은 뒤에 RepNotify가 불리므로 ServerFrame은 이 위치와 같은 프레임의 값이다.
	const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	const double FramePeriod = 1.0 / FMath::Clamp(NetDriver ? NetDriver->GetNetServerMaxTickRate() : 30, 1, 1000);
	FLabServerClock& Clock = GetServerClock();
	const double ServerTime = static_cast<double>(Clock.Unwrap(ServerFrame)) * FramePeriod;
	Clock.AddSample(FPlatformTime::Seconds(), ServerTime);

	const FRepMovement& Rep = GetReplicatedMovement();
	FSnapshot Snapshot;
	Snapshot.ServerTime = ServerTime;
	Snapshot.Location = FRepMovement::RebaseOntoLocalOrigin(Rep.Location, this);
	Snapshot.Rotation = Rep.Rotation.Quaternion();

	// 처음 받은 위치는 그대로 놓는다. 보간할 두 번째 위치가 아직 없다.
	if (Snapshots.IsEmpty())
	{
		Super::PostNetReceiveLocationAndRotation();
		Snapshots.Add(Snapshot);
		return;
	}
	// 같거나 이전 프레임의 위치는 버린다.
	if (ServerTime <= Snapshots.Last().ServerTime)
	{
		return;
	}
	if (Snapshots.Num() >= 32)
	{
		Snapshots.RemoveAt(0);
	}
	Snapshots.Add(Snapshot);
}

void ALabNpc::TickInterpolation()
{
	if (Snapshots.IsEmpty())
	{
		return;
	}

	// 서버 시각으로 지금보다 보간 지연만큼 앞선 순간을 그린다.
	const double RenderTime = FPlatformTime::Seconds() - GetServerClock().GetOffset() - FLabScenarioConfig::Get().NpcInterpDelayMs / 1000.0;

	// RenderTime보다 앞선 위치는 하나만 남긴다.
	while (Snapshots.Num() >= 2 && Snapshots[1].ServerTime <= RenderTime)
	{
		Snapshots.RemoveAt(0);
	}

	FVector Location = Snapshots[0].Location;
	FQuat Rotation = Snapshots[0].Rotation;
	// 두 위치 사이면 보간한다. 다음 위치가 아직 오지 않았으면(버퍼가 빔) 마지막 위치에 멈춘다.
	if (Snapshots.Num() >= 2 && RenderTime > Snapshots[0].ServerTime)
	{
		const FSnapshot& From = Snapshots[0];
		const FSnapshot& To = Snapshots[1];
		const double Alpha = (RenderTime - From.ServerTime) / (To.ServerTime - From.ServerTime);
		Location = FMath::Lerp(From.Location, To.Location, Alpha);
		Rotation = FQuat::Slerp(From.Rotation, To.Rotation, static_cast<float>(Alpha));
	}
	SetActorLocationAndRotation(Location, Rotation);
}

void ALabShowcaseNpc::SetPatrol(const FVector& Start, const FVector& End)
{
	PatrolStart = Start;
	PatrolEnd = End;
	PatrolTime = 0.f;
}

void ALabShowcaseNpc::TickMovement(float DeltaSeconds)
{
	// 배회하지 않는다. 위치를 시작 신호 뒤의 경과 시간만으로 정해, 실행마다 같은 시각에 같은 위치에 있게 한다.
	PatrolTime += DeltaSeconds;
	const float Length = FVector::Dist(PatrolStart, PatrolEnd);
	if (Length <= 0.f)
	{
		return;
	}
	const float Phase = FMath::Fmod(MoveSpeed * PatrolTime, 2.f * Length) / Length;
	const bool bForward = Phase <= 1.f;
	const FVector Direction = (bForward ? PatrolEnd - PatrolStart : PatrolStart - PatrolEnd) / Length;
	SetActorLocationAndRotation(FMath::Lerp(PatrolStart, PatrolEnd, bForward ? Phase : 2.f - Phase), Direction.Rotation());
}
