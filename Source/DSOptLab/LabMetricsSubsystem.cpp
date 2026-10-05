#include "LabMetricsSubsystem.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/NetworkObjectList.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CoreDelegates.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Net/NetAnalyticsTypes.h"
#include "Net/NetworkMetricsDatabase.h"
#include "Net/NetworkMetricsDefs.h"
#include "ProfilingDebugging/MiscTrace.h"
#include "LabGameMode.h"
#include "LabPlayerController.h"
#include "LabScenarioConfig.h"

DEFINE_LOG_CATEGORY_STATIC(LogLabMetrics, Log, All);

namespace
{
	double Average(const TArray<double>& Values)
	{
		if (Values.IsEmpty())
		{
			return 0.0;
		}

		double Sum = 0.0;
		for (const double Value : Values)
		{
			Sum += Value;
		}
		return Sum / Values.Num();
	}

	/** 99백분위 경계값. 정렬 후 ceil(N * 0.99)번째 값이다. 느린 1%의 평균이 아니다. */
	double Percentile99(TArray<double> Values)
	{
		if (Values.IsEmpty())
		{
			return 0.0;
		}

		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::CeilToInt32(Values.Num() * 0.99) - 1, 0, Values.Num() - 1);
		return Values[Index];
	}
}

bool ULabMetricsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && IsRunningDedicatedServer() && FLabServerConfig::Get().bMeasure;
}

bool ULabMetricsSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game;
}

void ULabMetricsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TickStartHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &ULabMetricsSubsystem::HandleWorldTickStart);
	PostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &ULabMetricsSubsystem::HandlePostActorTick);
	EndFrameHandle = FCoreDelegates::OnEndFrame.AddUObject(this, &ULabMetricsSubsystem::HandleEndFrame);

	// 시작 조건은 게임 모드가 판단한다. 여기서는 시작 신호를 듣고 준비 구간을 시작한다.
	// 게임 모드는 월드가 초기화될 때 만들어지므로 이 시점에 이미 있다(World.cpp의 UWorld::BeginPlay).
	ALabGameMode* GameMode = InWorld.GetAuthGameMode<ALabGameMode>();
	if (!GameMode)
	{
		Fail(TEXT("the game mode is not ALabGameMode"));
		return;
	}
	GameMode->OnScenarioStarted.AddUObject(this, &ULabMetricsSubsystem::HandleScenarioStarted);

	const FLabServerConfig& Config = FLabServerConfig::Get();
	UE_LOG(LogLabMetrics, Display, TEXT("Waiting for %d ready clients (label=%s nodes=%d npcs=%d)"),
		Config.ExpectedClients, *FLabScenarioConfig::Get().Label, Config.NumNodes, Config.NumNpcs);
}

void ULabMetricsSubsystem::HandleScenarioStarted()
{
	if (Phase != EPhase::WaitingForClients)
	{
		return;
	}

	Phase = EPhase::Warmup;
	PhaseStartTime = FPlatformTime::Seconds();
	ConnectionsAtStart = SampleConnections().NumConnections;
	UE_LOG(LogLabMetrics, Display, TEXT("All clients ready. Scenario started. Warmup %.0fs"), FLabServerConfig::Get().WarmupSeconds);
}

void ULabMetricsSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldTickStart.Remove(TickStartHandle);
	FWorldDelegates::OnWorldPostActorTick.Remove(PostActorTickHandle);
	FCoreDelegates::OnEndFrame.Remove(EndFrameHandle);

	Super::Deinitialize();
}

void ULabMetricsSubsystem::HandleWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld())
	{
		return;
	}

	FrameStartTime = FPlatformTime::Seconds();
	PostActorTickTime = FrameStartTime;
	bFrameOpen = true;
}

void ULabMetricsSubsystem::HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld())
	{
		return;
	}

	PostActorTickTime = FPlatformTime::Seconds();
}

ULabMetricsSubsystem::FConnectionSample ULabMetricsSubsystem::SampleConnections() const
{
	FConnectionSample Sample;

	const UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	if (!NetDriver)
	{
		return Sample;
	}

	for (UNetConnection* Connection : NetDriver->ClientConnections)
	{
		if (!Connection)
		{
			continue;
		}

		++Sample.NumConnections;
		Sample.TotalBytes += Connection->OutTotalBytes;
		Sample.OpenActorChannels += Connection->ActorChannelsNum();
		Sample.NetSpeed = Connection->CurrentNetSpeed;

		// 엔진이 ServerReplicateActors에서 연결마다 남기는 기록이다(NetDriver.cpp의 TrackReplicationForAnalytics).
		// 프레임 끝의 IsNetReady()는 그 프레임의 송신 예산을 뺀 뒤라서 지속적인 포화를 잡지 못한다.
		const FNetConnectionSaturationAnalytics& Saturation = Connection->GetSaturationAnalytics();
		Sample.Replications += Saturation.GetNumberOfReplications();
		Sample.SaturatedReplications += Saturation.GetNumberOfSaturatedReplications();

		const ALabPlayerController* Player = Cast<ALabPlayerController>(Connection->PlayerController);
		if (Player && Player->IsReady())
		{
			++Sample.NumReady;
		}
	}

	return Sample;
}

void ULabMetricsSubsystem::Fail(const TCHAR* Reason)
{
	UE_LOG(LogLabMetrics, Error, TEXT("Run failed: %s"), Reason);
	Phase = EPhase::Done;
	FPlatformMisc::RequestExitWithStatus(false, 1);
}

void ULabMetricsSubsystem::HandleEndFrame()
{
	if (!bFrameOpen || Phase == EPhase::Done)
	{
		return;
	}
	bFrameOpen = false;

	const double Now = FPlatformTime::Seconds();
	const FLabServerConfig& Config = FLabServerConfig::Get();
	const FConnectionSample Sample = SampleConnections();

	const double OpenChannelsPerConnection = Sample.NumConnections > 0
		? static_cast<double>(Sample.OpenActorChannels) / Sample.NumConnections : 0.0;

	// 준비 구간 길이와 초기 전송 완료 여부를 판단할 수 있게 5초마다 상태를 남긴다.
	if (Now - LastStatusLogTime >= 5.0)
	{
		LastStatusLogTime = Now;
		UE_LOG(LogLabMetrics, Display,
			TEXT("phase=%d connections=%d ready=%d open_actor_channels_per_conn=%.0f saturated_replications=%lld/%lld net_speed=%d out_total_bytes=%lld"),
			static_cast<int32>(Phase), Sample.NumConnections, Sample.NumReady, OpenChannelsPerConnection,
			Sample.SaturatedReplications, Sample.Replications, Sample.NetSpeed, Sample.TotalBytes);
	}

	if (Phase != EPhase::WaitingForClients && Sample.NumConnections != ConnectionsAtStart)
	{
		Fail(TEXT("connection count changed after start"));
		return;
	}

	switch (Phase)
	{
	case EPhase::Warmup:
		if (Now - PhaseStartTime >= Config.WarmupSeconds)
		{
			Phase = EPhase::Measuring;
			PhaseStartTime = Now;
			BytesAtMeasureStart = Sample.TotalBytes;
			ReplicationsAtMeasureStart = Sample.Replications;
			SaturatedReplicationsAtMeasureStart = Sample.SaturatedReplications;
			OpenChannelsPerConnectionSum = 0.0;
			ConsideredActorsSum = 0;
			WorkMs.Reset();
			NetFlushMs.Reset();

			// 틱 예산은 서버 틱 상한의 역수다. 엔진이 데디케이티드 서버의 틱을 이 값으로 제한한다(GameEngine.cpp의 UGameEngine::GetMaxTickRate).
			// 프로젝트는 엔진 기본값 30(BaseEngine.ini의 [/Script/OnlineSubsystemUtils.IpNetDriver] NetServerMaxTickRate)을 쓴다.
			{
				const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
				const int32 TickRate = NetDriver ? FMath::Clamp(NetDriver->GetNetServerMaxTickRate(), 1, 1000) : 30;
				TickBudgetMs = 1000.0 / TickRate;
				UE_LOG(LogLabMetrics, Display, TEXT("tick_rate=%d tick_budget_ms=%.3f"), TickRate, TickBudgetMs);
			}

			TRACE_BOOKMARK(TEXT("Lab_MeasureStart"));
			UE_LOG(LogLabMetrics, Display, TEXT("Measuring %.0fs"), Config.MeasureSeconds);
		}
		break;

	case EPhase::Measuring:
		WorkMs.Add((Now - FrameStartTime) * 1000.0);
		NetFlushMs.Add((Now - PostActorTickTime) * 1000.0);
		OpenChannelsPerConnectionSum += OpenChannelsPerConnection;
		ConsideredActorsSum += SampleConsideredActors();

		if (Now - PhaseStartTime >= Config.MeasureSeconds)
		{
			TRACE_BOOKMARK(TEXT("Lab_MeasureEnd"));

			if (!WriteSummary(Sample, Now - PhaseStartTime))
			{
				Fail(TEXT("could not write summary.csv"));
				return;
			}
			LogNetworkObjects();

			// 측정 구간의 프레임마다 Consider List에 든 액터 수의 평균. 연결과 상관없이 프레임에 한 번 만드는 목록이다.
			UE_LOG(LogLabMetrics, Display, TEXT("lab_consider_list avg_per_frame=%.1f frames=%d"),
				WorkMs.Num() > 0 ? static_cast<double>(ConsideredActorsSum) / WorkMs.Num() : 0.0, WorkMs.Num());

			Phase = EPhase::Done;
			FPlatformMisc::RequestExitWithStatus(false, 0);
		}
		break;

	default:
		break;
	}
}

int64 ULabMetricsSubsystem::SampleConsideredActors() const
{
	// 엔진이 Consider List를 만들 때마다 그 길이를 지표로 남긴다(NetDriver.cpp의 ServerReplicateActors_BuildConsiderList 끝,
	// UE::Net::Metric::NumConsideredActors). 프레임 끝에 읽으면 그 프레임의 값이다.
	UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	UNetworkMetricsDatabase* Metrics = NetDriver ? NetDriver->GetMetrics().Get() : nullptr;
	if (!Metrics || !Metrics->Contains(UE::Net::Metric::NumConsideredActors))
	{
		return 0;
	}
	return Metrics->GetInt(UE::Net::Metric::NumConsideredActors);
}

void ULabMetricsSubsystem::LogNetworkObjects() const
{
	// Consider List는 활성 목록만 돈다(NetDriver.cpp의 ServerReplicateActors_BuildConsiderList). 액터는 모든 연결에서 Dormant 상태여야
	// 활성 목록에서 빠진다(NetworkObjectList.cpp의 FNetworkObjectList::MarkDormant). 측정이 끝난 뒤 한 번만 세어 클래스별로 남긴다.
	// 값은 클래스마다 활성 목록의 수 / 그 가운데 일부 연결에서 Dormant 상태인 수 / 모든 연결에서 Dormant 상태라 빠진 수다.
	const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	if (!NetDriver)
	{
		return;
	}

	struct FClassCount
	{
		int32 Active = 0;
		int32 ActivePartlyDormant = 0;
		int32 DormantOnAll = 0;
	};
	TMap<FName, FClassCount> Counts;

	const FNetworkObjectList& List = NetDriver->GetNetworkObjectList();
	for (const TSharedPtr<FNetworkObjectInfo>& Info : List.GetActiveObjects())
	{
		if (const AActor* Actor = Info->WeakActor.Get())
		{
			FClassCount& Count = Counts.FindOrAdd(Actor->GetClass()->GetFName());
			++Count.Active;
			if (Info->DormantConnections.Num() > 0)
			{
				++Count.ActivePartlyDormant;
			}
		}
	}
	for (const TSharedPtr<FNetworkObjectInfo>& Info : List.GetDormantObjectsOnAllConnections())
	{
		if (const AActor* Actor = Info->WeakActor.Get())
		{
			++Counts.FindOrAdd(Actor->GetClass()->GetFName()).DormantOnAll;
		}
	}

	Counts.ValueSort([](const FClassCount& A, const FClassCount& B) { return A.Active > B.Active; });
	FString ByClass;
	for (const TPair<FName, FClassCount>& Pair : Counts)
	{
		ByClass += FString::Printf(TEXT("%s%s=%d/%d/%d"), ByClass.IsEmpty() ? TEXT("") : TEXT(","),
			*Pair.Key.ToString(), Pair.Value.Active, Pair.Value.ActivePartlyDormant, Pair.Value.DormantOnAll);
	}
	UE_LOG(LogLabMetrics, Display, TEXT("lab_network_objects active=%d dormant_on_all=%d by_class(active/partly_dormant/dormant_on_all)=%s"),
		List.GetActiveObjects().Num(), List.GetDormantObjectsOnAllConnections().Num(), *ByClass);
}

bool ULabMetricsSubsystem::WriteSummary(const FConnectionSample& Sample, double MeasuredSeconds) const
{
	const FLabServerConfig& Config = FLabServerConfig::Get();
	const int32 Frames = FMath::Max(1, WorkMs.Num());
	const int32 Connections = FMath::Max(1, ConnectionsAtStart);
	const double BytesPerSecPerConn = static_cast<double>(Sample.TotalBytes - BytesAtMeasureStart) / MeasuredSeconds / Connections;

	// 측정 구간에 모든 연결에서 포화로 끊긴 리플리케이션 횟수 / 시도 횟수.
	const int64 Replications = Sample.Replications - ReplicationsAtMeasureStart;
	const int64 SaturatedReplications = Sample.SaturatedReplications - SaturatedReplicationsAtMeasureStart;
	const double SaturatedRatio = Replications > 0 ? static_cast<double>(SaturatedReplications) / Replications : 0.0;

	int32 OverBudgetFrames = 0;
	for (const double Value : WorkMs)
	{
		if (Value > TickBudgetMs)
		{
			++OverBudgetFrames;
		}
	}

	const FString Header = TEXT("label,timestamp,clients,nodes,npcs,frames,work_avg_ms,work_p99_ms,over_budget_frames,netflush_avg_ms,netflush_p99_ms,out_bytes_per_sec_per_conn,open_actor_channels_per_conn,saturated_ratio,net_speed,config");
	const FString Row = FString::Printf(TEXT("%s,%s,%d,%d,%d,%d,%.3f,%.3f,%d,%.3f,%.3f,%.0f,%.0f,%.3f,%d,%s"),
		*FLabScenarioConfig::Get().Label,
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
		Sample.NumConnections,
		Config.NumNodes + 1, // 검증용 노드 포함
		Config.NumNpcs,
		WorkMs.Num(),
		Average(WorkMs),
		Percentile99(WorkMs),
		OverBudgetFrames,
		Average(NetFlushMs),
		Percentile99(NetFlushMs),
		BytesPerSecPerConn,
		OpenChannelsPerConnectionSum / Frames,
		SaturatedRatio,
		Sample.NetSpeed,
		*Config.GetConfigName());

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("LabMetrics");
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString FilePath = Directory / TEXT("summary.csv");

	FString Output;
	if (!FPaths::FileExists(FilePath))
	{
		Output += Header + LINE_TERMINATOR;
	}
	Output += Row + LINE_TERMINATOR;

	UE_LOG(LogLabMetrics, Display, TEXT("%s"), *Header);
	UE_LOG(LogLabMetrics, Display, TEXT("%s"), *Row);

	return FFileHelper::SaveStringToFile(Output, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(), FILEWRITE_Append);
}
