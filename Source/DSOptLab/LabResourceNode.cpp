#include "LabResourceNode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "LabScenarioConfig.h"
#include "LabVisual.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ALabResourceNode::ALabResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	const FLabServerConfig& Config = FLabServerConfig::Get();

	// 켜면 거리와 무관하게 모든 연결에 보낸다(1막의 기준선).
	bAlwaysRelevant = Config.bAlwaysRelevant;

	// 상태가 바뀔 때만 깨워서 보낸다.
	if (Config.bNodeDormancy)
	{
		NetDormancy = DORM_DormantAll;
	}

	// 낮추면 활성 목록에 남은 노드가 Consider List에 드는 간격이 길어진다(NetDriver.cpp의 NextUpdateTime 검사).
	SetNetUpdateFrequency(Config.NodeUpdateFrequency);

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

void ALabResourceNode::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Material = LabVisual::GetSharedMaterial(LabVisual::NodeColor))
	{
		Mesh->SetMaterial(0, Material);
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

	WakeForChange();

	--Health;
	OnRep_Health();
	if (Health <= 0)
	{
		bDepleted = true;
		OnRep_Depleted();
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ALabResourceNode::Respawn, RespawnSeconds, false);
	}
}

void ALabResourceNode::WakeForChange()
{
	// Dormant 상태면 깨워서 아래 변경이 전송되게 한다.
	// NetUpdateFrequency를 낮춘 구성에서는 다음 고려 시각도 앞당긴다. 그러지 않으면 변경이 최대 1 ÷ 빈도초 늦는다.
	// ForceNetUpdate는 NextUpdateTime을 지금으로 당기고(NetDriver.cpp의 UNetDriver::ForceNetUpdate), Dormant 상태면 FlushNetDormancy도 부른다(Actor.cpp).
	if (FLabServerConfig::Get().NodeUpdateFrequency < FLabServerConfig().NodeUpdateFrequency)
	{
		ForceNetUpdate();
	}
	else
	{
		FlushNetDormancy();
	}
}

void ALabResourceNode::Respawn()
{
	WakeForChange();

	Health = MaxHealth;
	OnRep_Health();
	bDepleted = false;
	OnRep_Depleted();
}

void ALabResourceNode::OnRep_Health()
{
	// 채집이 눈에 보이도록 체력에 비례해 노드 높이를 줄인다(체력 3일 때 원래 높이 3배).
	const float HeightScale = 3.f * FMath::Max(Health, 1) / MaxHealth;
	Mesh->SetRelativeScale3D(FVector(1.f, 1.f, HeightScale));
}

void ALabResourceNode::OnRep_Depleted()
{
	Mesh->SetVisibility(!bDepleted);
}
