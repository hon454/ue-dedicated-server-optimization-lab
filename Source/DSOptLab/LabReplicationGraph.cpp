#include "LabReplicationGraph.h"

#include "DSOptLab.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "HAL/IConsoleManager.h"
#include "LabBuilding.h"
#include "LabCharacter.h"
#include "LabNpc.h"
#include "LabResourceNode.h"
#include "LabScenarioConfig.h"
#include "UObject/UObjectIterator.h"

uint16 ULabReplicationGraph::GetLegacyPeriodFrame(float NetUpdateFrequency) const
{
	// 레거시는 액터를 고려한 뒤 다음 고려 시각을 1/빈도 + 0\~1프레임의 난수 뒤로 잡고(NetDriver.cpp의
	// ServerReplicateActors_BuildConsiderList, NextUpdateTime = TimeSeconds + RandDelay + NextUpdateDelta),
	// 그 시각이 지난 첫 프레임에 다시 고려한다. 그래서 고려 간격의 평균은 (틱 ÷ 빈도) + 1프레임이다.
	// 빈도 10은 4프레임, 2는 16프레임, 100은 1.3프레임이다(engine-notes.md 2절에서 빈도 10이 4프레임임을 실행으로 확인).
	// 그래프는 정수 프레임 간격이라 이 평균에 가장 가까운 정수를 쓴다. 엔진의 GetReplicationPeriodFrameForFrequency는
	// 틱 ÷ 빈도를 반올림해서 NPC를 3프레임마다 보내게 되고, 레거시보다 NPC 갱신이 3분의 1 늘어난다.
	check(NetDriver);
	check(NetUpdateFrequency > 0.f);
	const float TickRate = static_cast<float>(NetDriver->GetNetServerMaxTickRate());
	const int32 Frames = FMath::RoundToInt(TickRate / NetUpdateFrequency + 1.f);
	return static_cast<uint16>(FMath::Clamp(Frames, 1, static_cast<int32>(MAX_uint16)));
}

void ULabReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();

	// UBasicReplicationGraph::InitGlobalActorClassSettings와 같다. 간격만 GetLegacyPeriodFrame으로 바꿨다.
	// 클래스 기본 객체의 값을 쓴다. 이 프로젝트의 액터는 생성자에서 명령줄 설정(FLabServerConfig)을 읽으므로 기본 객체도 같은 값이다.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}
		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		FClassReplicationInfo ClassInfo;
		ClassInfo.ReplicationPeriodFrame = GetLegacyPeriodFrame(ActorCDO->GetNetUpdateFrequency());
		if (ActorCDO->bAlwaysRelevant || ActorCDO->bOnlyRelevantToOwner)
		{
			ClassInfo.SetCullDistanceSquared(0.f);
		}
		else
		{
			ClassInfo.SetCullDistanceSquared(ActorCDO->GetNetCullDistanceSquared());
		}
		GlobalActorReplicationInfoMap.SetClassInfo(Class, ClassInfo);
	}

	// 이 테스트베드의 클래스가 받은 값을 남긴다.
	const UClass* LoggedClasses[] = { ALabResourceNode::StaticClass(), ALabBuilding::StaticClass(), ALabNpc::StaticClass(), ALabCharacter::StaticClass() };
	for (const UClass* Class : LoggedClasses)
	{
		const AActor* ActorCDO = GetDefault<AActor>(const_cast<UClass*>(Class));
		const FClassReplicationInfo& Info = GlobalActorReplicationInfoMap.GetClassInfo(const_cast<UClass*>(Class));
		UE_LOG(LogDSOptLab, Display, TEXT("lab_repgraph_class class=%s net_update_frequency=%g period_frames=%d cull_distance_cm=%.0f dormant=%d"),
			*Class->GetName(), ActorCDO->GetNetUpdateFrequency(), Info.ReplicationPeriodFrame,
			FMath::Sqrt(Info.GetCullDistanceSquared()), ActorCDO->NetDormancy > DORM_Awake ? 1 : 0);
	}
}

void ULabReplicationGraph::InitGlobalGraphNodes()
{
	// 칸의 동적 액터 목록(UReplicationGraphNode_ActorListFrequencyBuckets)은 기본으로 버킷 3개 가운데 하나만 한 프레임에
	// 돌려준다(ReplicationGraph.cpp의 GatherActorListsForConnection). 버킷을 하나로 두어 동적 액터를 매 프레임 모으고,
	// 보내는 간격은 클래스마다 정한 ReplicationPeriodFrame이 정하게 한다. 칸 노드가 생길 때 이 값을 읽으므로 격자를 만들기 전에 둔다.
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.NumBuckets = 1;

	// UBasicReplicationGraph와 같은 격자다. 칸 100m, 맵 원점과 상관없이 음수 좌표까지 덮는 기준점.
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = 10000.f;
	GridNode->SpatialBias = FVector2D(-UE_OLD_WORLD_MAX, -UE_OLD_WORLD_MAX);
	AddGlobalGraphNode(GridNode);

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);

	// 연결별 Dormancy 노드는 Dormant 상태가 된 액터를 목록에서 지우다가, 한 번의 gather에서 지울 것이 없으면 남은 액터 하나의
	// 컬 거리를 0으로 만들어 150m 밖이어도 보낸다(ReplicationGraph.cpp의 UReplicationGraphNode_ConnectionDormancyNode::
	// ConditionalGatherDormantActorsForConnection). -LabRepGraphNoTrickle이면 이 동작을 끈다. 이 값은 gather 때마다 읽힌다.
	IConsoleVariable* Trickle = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.TrickleDistCullOnDormancyNodes"));
	if (Trickle && FLabServerConfig::Get().bRepGraphNoTrickle)
	{
		Trickle->Set(0, ECVF_SetByCode);
	}

	UE_LOG(LogDSOptLab, Display, TEXT("lab_repgraph cell_size_cm=%.0f buckets=%d trickle=%d"),
		GridNode->CellSize, UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.NumBuckets,
		Trickle ? Trickle->GetInt() : -1);
}

void ULabReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
	Super::InitConnectionGraphNodes(RepGraphConnection);

	// 연결마다 그 연결의 PlayerController와 시점 액터(자기 캐릭터), 소유자에게만 보내는 액터를 담는다.
	UReplicationGraphNode_AlwaysRelevant_ForConnection* Node = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
	AddConnectionGraphNode(Node, RepGraphConnection);
	AlwaysRelevantForConnection.Add(RepGraphConnection->NetConnection, Node);
}

void ULabReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	AActor* Actor = ActorInfo.Actor;
	if (Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
	}
	else if (Actor->bOnlyRelevantToOwner)
	{
		// PlayerController, GameplayDebuggerCategoryReplicator. 연결은 다음 ServerReplicateActors에서 찾는다.
		ActorsWithoutNetConnection.Add(Actor);
	}
	else
	{
		// Dormant 상태를 원하면 정적 액터로, 아니면 동적 액터로 넣고, Dormant 상태가 바뀌면 옮긴다(AddActor_Dormancy).
		// 자원 노드와 건축물은 정적 액터, NPC와 캐릭터는 동적 액터가 된다.
		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
	}
}

void ULabReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	AActor* Actor = ActorInfo.Actor;
	if (Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
		SetActorDestructionInfoToIgnoreDistanceCulling(Actor);
	}
	else if (Actor->bOnlyRelevantToOwner)
	{
		ActorsWithoutNetConnection.Remove(Actor);
		if (UReplicationGraphNode* Node = GetAlwaysRelevantNodeForConnection(Actor->GetNetConnection()))
		{
			Node->NotifyRemoveNetworkActor(ActorInfo);
		}
	}
	else
	{
		GridNode->RemoveActor_Dormancy(ActorInfo);
	}
}

void ULabReplicationGraph::RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo)
{
	AActor* Actor = ActorInfo.NewActorInfo.Actor;
	if (Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyActorRenamed(ActorInfo);
	}
	else if (Actor->bOnlyRelevantToOwner)
	{
		if (UReplicationGraphNode* Node = GetAlwaysRelevantNodeForConnection(Actor->GetNetConnection()))
		{
			Node->NotifyActorRenamed(ActorInfo);
		}
	}
	else
	{
		GridNode->RenameActor_Dormancy(ActorInfo);
	}
}

#if DO_ENABLE_REPGRAPH_DEBUG_ACTOR
AReplicationGraphDebugActor* ULabReplicationGraph::CreateDebugActor() const
{
	// Shipping이 아닌 빌드에서 그래프는 연결마다 디버그 액터를 하나 스폰해 그 연결에 보낸다(ReplicationGraph.cpp의
	// UNetReplicationGraphConnection::InitForGraph, ServerReplicateActors). 레거시에는 없는 액터라 만들지 않는다.
	return nullptr;
}
#endif

UReplicationGraphNode_AlwaysRelevant_ForConnection* ULabReplicationGraph::GetAlwaysRelevantNodeForConnection(UNetConnection* Connection)
{
	if (!Connection)
	{
		return nullptr;
	}
	const TObjectPtr<UReplicationGraphNode_AlwaysRelevant_ForConnection>* Node = AlwaysRelevantForConnection.Find(Connection);
	return Node ? Node->Get() : nullptr;
}

int32 ULabReplicationGraph::ServerReplicateActors(float DeltaSeconds)
{
	// 스폰할 때 연결이 없던 소유자 전용 액터를 연결이 생기면 그 연결의 노드로 옮긴다(UBasicReplicationGraph::ServerReplicateActors와 같다).
	for (int32 Index = ActorsWithoutNetConnection.Num() - 1; Index >= 0; --Index)
	{
		AActor* Actor = ActorsWithoutNetConnection[Index];
		if (!Actor)
		{
			ActorsWithoutNetConnection.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}
		if (UReplicationGraphNode_AlwaysRelevant_ForConnection* Node = GetAlwaysRelevantNodeForConnection(Actor->GetNetConnection()))
		{
			Node->NotifyAddNetworkActor(FNewReplicatedActorInfo(Actor));
			ActorsWithoutNetConnection.RemoveAtSwap(Index, EAllowShrinking::No);
		}
	}

	return Super::ServerReplicateActors(DeltaSeconds);
}
