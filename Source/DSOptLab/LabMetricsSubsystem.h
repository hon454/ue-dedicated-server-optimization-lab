#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "LabMetricsSubsystem.generated.h"

/** 서버 측정. 준비 구간과 측정 구간을 관리하고 요약을 CSV로 남긴 뒤 서버를 종료한다. */
UCLASS()
class DSOPTLAB_API ULabMetricsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	enum class EPhase : uint8
	{
		WaitingForClients,
		Warmup,
		Measuring,
		Done,
	};

	/** 프레임 끝에 모든 클라이언트 연결에서 읽은 값. */
	struct FConnectionSample
	{
		int32 NumConnections = 0;
		int32 NumReady = 0;
		int64 TotalBytes = 0;
		int32 OpenActorChannels = 0;
		/** 연결이 생긴 뒤의 누계. 엔진이 ServerReplicateActors에서 연결마다 기록한다. */
		int64 Replications = 0;
		int64 SaturatedReplications = 0;
		int32 NetSpeed = 0;
	};

	void HandleWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void HandleEndFrame();

	FConnectionSample SampleConnections() const;
	bool WriteSummary(const FConnectionSample& Sample, double MeasuredSeconds) const;
	void Fail(const TCHAR* Reason);

	EPhase Phase = EPhase::WaitingForClients;
	bool bFrameOpen = false;

	double FrameStartTime = 0.0;
	double PostActorTickTime = 0.0;
	double PhaseStartTime = 0.0;
	double LastStatusLogTime = 0.0;

	int32 ConnectionsAtStart = 0;
	int64 BytesAtMeasureStart = 0;
	int64 ReplicationsAtMeasureStart = 0;
	int64 SaturatedReplicationsAtMeasureStart = 0;
	double OpenChannelsPerConnectionSum = 0.0;

	TArray<double> WorkMs;
	TArray<double> NetFlushMs;

	FDelegateHandle TickStartHandle;
	FDelegateHandle PostActorTickHandle;
	FDelegateHandle EndFrameHandle;
};
