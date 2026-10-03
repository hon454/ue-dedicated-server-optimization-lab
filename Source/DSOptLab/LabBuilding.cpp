#include "LabBuilding.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "LabScenarioConfig.h"
#include "LabVisual.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ALabBuilding::ALabBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 자원 노드와 같은 인자를 따른다. 기준선에서는 Always Relevant이고 Dormant 상태가 아니다.
	const FLabServerConfig& Config = FLabServerConfig::Get();
	bAlwaysRelevant = Config.bAlwaysRelevant;
	if (Config.bNodeDormancy)
	{
		NetDormancy = DORM_DormantAll;
	}

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	// 플레이어의 자동 이동 경로를 막지 않게 충돌을 끈다.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
}

void ALabBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Material = LabVisual::GetSharedMaterial(LabVisual::BuildingColor))
	{
		Mesh->SetMaterial(0, Material);
	}
}

void ALabBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALabBuilding, Health);
	DOREPLIFETIME(ALabBuilding, Tier);
}
