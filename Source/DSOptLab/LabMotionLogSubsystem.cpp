#include "LabMotionLogSubsystem.h"

#include "Engine/NetDriver.h"
#include "Engine/PackageMapClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"
#include "DSOptLab.h"
#include "LabGameMode.h"
#include "LabNpc.h"
#include "LabPlayerController.h"
#include "LabScenarioConfig.h"

namespace
{
	// 파일 머리. Scripts/analyze-motion.ps1이 같은 순서로 읽는다.
	constexpr uint32 MotionLogMagic = 0x4D42414C; // "LABM"
	constexpr int32 MotionLogVersion = 1;
}

bool ULabMotionLogSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer) || !FLabScenarioConfig::Get().bMotionLog)
	{
		return false;
	}
	// 서버는 측정하는 실행에서만, 클라이언트는 기록 시간을 받았을 때만 기록한다.
	return IsRunningDedicatedServer() ? FLabServerConfig::Get().bMeasure : FLabClientConfig::Get().MotionLogSeconds > 0.f;
}

bool ULabMotionLogSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game;
}

void ULabMotionLogSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	bServer = IsRunningDedicatedServer();
	PostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &ULabMotionLogSubsystem::HandlePostActorTick);

	if (bServer)
	{
		if (ALabGameMode* GameMode = InWorld.GetAuthGameMode<ALabGameMode>())
		{
			GameMode->OnScenarioStarted.AddUObject(this, &ULabMotionLogSubsystem::HandleScenarioStarted);
		}
	}
}

void ULabMotionLogSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(PostActorTickHandle);

	// 클라이언트는 서버가 끝나 연결이 끊기면 월드가 정리된다. 기록 시간이 다 차기 전이어도 모은 것을 남긴다.
	if (!bServer && bRecording && !bWritten)
	{
		WriteFile();
	}

	Super::Deinitialize();
}

void ULabMotionLogSubsystem::HandleScenarioStarted()
{
	// NPC는 모두 시작 신호 전에 스폰된다(ALabGameMode). 여기서 한 번 모아 프레임마다 액터를 찾지 않는다.
	for (TActorIterator<ALabNpc> It(GetWorld()); It; ++It)
	{
		ServerNpcs.Add(*It);
		ServerNpcGuids.Add(0);
	}
	Positions.Reserve(ServerNpcs.Num() * 30 * 120);
	StartSignalCycles = FPlatformTime::Cycles64();
	bRecording = true;
	UE_LOG(LogDSOptLab, Display, TEXT("lab_motion_log server recording npcs=%d"), ServerNpcs.Num());
}

void ULabMotionLogSubsystem::MarkMeasureStart()
{
	MeasureStartCycles = FPlatformTime::Cycles64();
}

void ULabMotionLogSubsystem::MarkMeasureEndAndWrite()
{
	MeasureEndCycles = FPlatformTime::Cycles64();
	UE_LOG(LogDSOptLab, Display, TEXT("lab_motion_log server record_ms_per_frame=%.4f frames=%d"),
		MeasureRecordFrames > 0 ? FPlatformTime::ToMilliseconds64(MeasureRecordCycles) / MeasureRecordFrames : 0.0, MeasureRecordFrames);
	// 측정 구간의 마지막 프레임까지 기록했다. 파일을 쓰는 시간은 측정 구간 밖이다.
	WriteFile();
	bRecording = false;
}

void ULabMotionLogSubsystem::HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld() || bWritten)
	{
		return;
	}

	const uint64 Now = FPlatformTime::Cycles64();
	if (bServer)
	{
		if (bRecording)
		{
			RecordServerFrame(Now);
			if (MeasureStartCycles != 0)
			{
				MeasureRecordCycles += FPlatformTime::Cycles64() - Now;
				++MeasureRecordFrames;
			}
		}
		return;
	}

	if (!bRecording)
	{
		const ALabPlayerController* Player = World->GetFirstPlayerController<ALabPlayerController>();
		if (!Player || !Player->IsScenarioStarted())
		{
			return;
		}
		StartSignalCycles = Now;
		bRecording = true;
		Positions.Reserve(100 * 30 * 120);
		UE_LOG(LogDSOptLab, Display, TEXT("lab_motion_log client recording seconds=%.0f"), FLabClientConfig::Get().MotionLogSeconds);
	}

	RecordClientFrame(Now);

	const double Elapsed = static_cast<double>(Now - StartSignalCycles) * FPlatformTime::GetSecondsPerCycle64();
	if (Elapsed >= FLabClientConfig::Get().MotionLogSeconds)
	{
		WriteFile();
		bRecording = false;
	}
}

uint64 ULabMotionLogSubsystem::GetGuid(const AActor& Actor) const
{
	const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	if (!NetDriver || !NetDriver->GetNetGuidCache().IsValid())
	{
		return 0;
	}
	return NetDriver->GetNetGuidCache()->GetNetGUID(&Actor).ObjectId;
}

void ULabMotionLogSubsystem::RecordServerFrame(uint64 Now)
{
	++ServerFrameIndex;
	// 액터 틱이 끝난 뒤라 이 프레임에 옮긴 위치다. 리플리케이션은 이 뒤의 넷 틱에서 같은 위치를 보낸다.
	for (int32 Index = 0; Index < ServerNpcs.Num(); ++Index)
	{
		const ALabNpc* Npc = ServerNpcs[Index].Get();
		if (!Npc)
		{
			continue;
		}
		// NetGUID는 처음 리플리케이트될 때 정해진다. 아직 어느 연결에도 보내지 않은 NPC는 클라이언트에도 없으므로 건너뛴다.
		// 찾는 비용(프레임당 NPC 수만큼의 맵 조회)을 줄이려고 NPC마다 30프레임(약 1초)에 한 번만 다시 찾는다.
		// 그만큼 늦게 기록을 시작해도, 클라이언트에 나타난 뒤 1초 안의 표본은 지표에서 빼므로(ADR-0020) 잃는 것이 없다.
		if (ServerNpcGuids[Index] == 0)
		{
			if ((ServerFrameIndex + Index) % 30 != 0)
			{
				continue;
			}
			ServerNpcGuids[Index] = GetGuid(*Npc);
			if (ServerNpcGuids[Index] == 0)
			{
				continue;
			}
		}
		Positions.Add({ Now, ServerNpcGuids[Index], FVector3f(Npc->GetActorLocation()) });
	}
}

void ULabMotionLogSubsystem::RecordClientFrame(uint64 Now)
{
	for (TActorIterator<ALabNpc> It(GetWorld()); It; ++It)
	{
		const uint64 Guid = GetGuid(**It);
		if (Guid != 0)
		{
			Positions.Add({ Now, Guid, FVector3f(It->GetVisualLocation()) });
		}
	}
}

void ULabMotionLogSubsystem::RecordReceive(const ALabNpc& Npc, uint8 ServerFrame)
{
	if (bServer || !bRecording || bWritten)
	{
		return;
	}
	const uint64 Guid = GetGuid(Npc);
	if (Guid != 0)
	{
		Receives.Add({ FPlatformTime::Cycles64(), Guid, ServerFrame });
	}
}

bool ULabMotionLogSubsystem::WriteFile()
{
	bWritten = true;

	const int32 Slot = bServer ? -1 : FLabClientConfig::Get().Slot;
	TArray<uint8> Bytes;
	Bytes.Reserve(64 + Positions.Num() * 28 + Receives.Num() * 17);
	FMemoryWriter Writer(Bytes);

	uint32 Magic = MotionLogMagic;
	int32 Version = MotionLogVersion;
	int32 Kind = bServer ? 0 : 1;
	int32 SlotValue = Slot;
	double SecondsPerCycle = FPlatformTime::GetSecondsPerCycle64();
	Writer << Magic << Version << Kind << SlotValue << SecondsPerCycle;
	Writer << StartSignalCycles << MeasureStartCycles << MeasureEndCycles;

	int32 NumPositions = Positions.Num();
	Writer << NumPositions;
	for (FPositionSample& Sample : Positions)
	{
		Writer << Sample.Cycles << Sample.Guid << Sample.Location.X << Sample.Location.Y << Sample.Location.Z;
	}

	int32 NumReceives = Receives.Num();
	Writer << NumReceives;
	for (FReceiveSample& Sample : Receives)
	{
		Writer << Sample.Cycles << Sample.Guid << Sample.ServerFrame;
	}

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("LabMotion") / FLabScenarioConfig::Get().Label;
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString Name = bServer ? TEXT("server") : FString::Printf(TEXT("client%d"), Slot);
	const FString TempPath = Directory / (Name + TEXT(".tmp"));
	const FString FinalPath = Directory / (Name + TEXT(".bin"));

	// 다 쓴 뒤에 이름을 바꾼다. run-scenario.ps1은 .bin이 생기면 클라이언트를 끝낸다.
	const bool bOk = FFileHelper::SaveArrayToFile(Bytes, *TempPath) && IFileManager::Get().Move(*FinalPath, *TempPath, true);
	UE_LOG(LogDSOptLab, Display, TEXT("lab_motion_log wrote=%s positions=%d receives=%d ok=%d"), *FinalPath, NumPositions, NumReceives, bOk ? 1 : 0);

	Positions.Empty();
	Receives.Empty();
	return bOk;
}
