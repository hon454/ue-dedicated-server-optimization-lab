#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "LabReplicationGraph.generated.h"

class UReplicationGraphNode_AlwaysRelevant_ForConnection;
class UReplicationGraphNode_GridSpatialization2D;

/**
 * 포스팅 14의 Replication Graph. -LabReplicationGraph일 때만 서버가 쓴다(DSOptLab.cpp).
 *
 * 엔진의 UBasicReplicationGraph(플러그인 BasicReplicationGraph.cpp)와 같은 경로로 액터를 나눈다.
 * bAlwaysRelevant는 모든 연결, bOnlyRelevantToOwner는 소유자 연결, 나머지는 공간 격자의 AddActor_Dormancy다.
 * UBasicReplicationGraph는 모듈 밖으로 내보내지 않은 클래스라 상속하지 못해서 같은 코드를 여기에 두었다.
 * 레거시와 다른 점을 격자로 모으는 것과 우선순위 매기는 법만 남기려고 두 가지를 바꿨다.
 * - 칸의 동적 액터 버킷을 3개에서 1개로 둔다. 3개면 동적 액터(NPC, 다른 플레이어의 캐릭터)를 3프레임에 한 번만 모은다.
 * - 클래스마다 보내는 간격(프레임)을 레거시가 실제로 고려하는 간격에 맞춘다(GetLegacyPeriodFrame).
 */
UCLASS(transient)
class ULabReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;

protected:
	virtual void RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo) override;
#if DO_ENABLE_REPGRAPH_DEBUG_ACTOR
	virtual class AReplicationGraphDebugActor* CreateDebugActor() const override;
#endif

private:
	/** 레거시가 이 빈도의 액터를 고려하는 간격(프레임). 자세한 것은 cpp. */
	uint16 GetLegacyPeriodFrame(float NetUpdateFrequency) const;

	UReplicationGraphNode_AlwaysRelevant_ForConnection* GetAlwaysRelevantNodeForConnection(UNetConnection* Connection);

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	UPROPERTY()
	TMap<TObjectPtr<UNetConnection>, TObjectPtr<UReplicationGraphNode_AlwaysRelevant_ForConnection>> AlwaysRelevantForConnection;

	/** 소유자 연결에만 보내는 액터 가운데 스폰할 때 연결이 없던 것. 연결이 생기면 그 연결의 노드로 옮긴다. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsWithoutNetConnection;
};
