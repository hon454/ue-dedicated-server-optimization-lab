#include "LabResourceNode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ALabResourceNode::ALabResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 기준선: 거리와 무관하게 모든 연결에 보낸다.
	bAlwaysRelevant = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(1.f, 1.f, 3.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
}

void ALabResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALabResourceNode, Health);
	DOREPLIFETIME(ALabResourceNode, bDepleted);
}

void ALabResourceNode::Harvest()
{
	if (!HasAuthority() || bDepleted)
	{
		return;
	}

	--Health;
	if (Health <= 0)
	{
		bDepleted = true;
		OnRep_Depleted();
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ALabResourceNode::Respawn, RespawnSeconds, false);
	}
}

void ALabResourceNode::Respawn()
{
	Health = MaxHealth;
	bDepleted = false;
	OnRep_Depleted();
}

void ALabResourceNode::OnRep_Depleted()
{
	Mesh->SetVisibility(!bDepleted);
}
