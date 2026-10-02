#include "LabNpc.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "LabVisual.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ALabNpc::ALabNpc()
{
	PrimaryActorTick.bCanEverTick = true;
	// 시작 신호 전에는 틱하지 않는다. 실행마다 같은 시점에 같은 위치에 있게 하기 위해서다.
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	SetReplicatingMovement(true);

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
}

void ALabNpc::StartWandering()
{
	if (HasAuthority())
	{
		SetActorTickEnabled(true);
	}
}

void ALabNpc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
