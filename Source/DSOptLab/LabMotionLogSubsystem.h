#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "LabMotionLogSubsystem.generated.h"

class ALabNpc;

/**
 * NPC 움직임의 품질 지표를 위한 위치 기록(ADR-0020). -LabMotionLog일 때만 생긴다.
 * 서버는 프레임마다 NPC의 위치를, 클라이언트는 프레임마다 그려지는 NPC의 위치와 이동 갱신을 받은 시각을 메모리에 모은다.
 * 시각은 FPlatformTime::Cycles64()(QueryPerformanceCounter)라서 같은 PC의 두 프로세스가 같은 시각축을 쓴다.
 * NPC는 서버와 클라이언트에서 같은 NetGUID로 짝짓는다. 파일은 Saved/LabMotion/<라벨>/에 쓰고 Scripts/analyze-motion.ps1이 읽는다.
 */
UCLASS()
class DSOPTLAB_API ULabMotionLogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** 서버 전용. ULabMetricsSubsystem이 측정 구간의 시작과 끝(두 북마크와 같은 순간)에 부른다. 끝에서 파일을 쓴다. */
	void MarkMeasureStart();
	void MarkMeasureEndAndWrite();

	/** 클라이언트 전용. ALabNpc가 이동 갱신을 받을 때 부른다. ServerFrame은 보간을 끈 실행에서는 0이다. */
	void RecordReceive(const ALabNpc& Npc, uint8 ServerFrame);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	struct FPositionSample
	{
		uint64 Cycles = 0;
		uint64 Guid = 0;
		FVector3f Location = FVector3f::ZeroVector;
	};

	struct FReceiveSample
	{
		uint64 Cycles = 0;
		uint64 Guid = 0;
		uint8 ServerFrame = 0;
	};

	void HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void HandleScenarioStarted();
	void RecordServerFrame(uint64 Now);
	void RecordClientFrame(uint64 Now);
	uint64 GetGuid(const AActor& Actor) const;
	bool WriteFile();

	bool bServer = false;
	bool bRecording = false;
	bool bWritten = false;

	uint64 StartSignalCycles = 0;
	uint64 MeasureStartCycles = 0;
	uint64 MeasureEndCycles = 0;

	/** 서버 전용. 시작 신호 때 모은 NPC와 그 NetGUID(처음 리플리케이트될 때 정해지므로 0이면 다시 찾는다). */
	TArray<TWeakObjectPtr<ALabNpc>> ServerNpcs;
	TArray<uint64> ServerNpcGuids;

	/** 서버 전용. 측정 구간에 기록하느라 쓴 시간(사이클)과 프레임 수. 기록 비용이 작은지 보려고 남긴다(ADR-0020 "고려한 대안"). */
	uint64 MeasureRecordCycles = 0;
	int32 MeasureRecordFrames = 0;
	int32 ServerFrameIndex = 0;

	TArray<FPositionSample> Positions;
	TArray<FReceiveSample> Receives;

	FDelegateHandle PostActorTickHandle;
};
