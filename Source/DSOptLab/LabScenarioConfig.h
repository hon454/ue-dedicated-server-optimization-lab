#pragma once

#include "CoreMinimal.h"

// 실행 인자로 받는 시나리오 값. 프로세스당 한 번만 읽는다.
// 서버와 클라이언트가 함께 쓰는 값, 서버만 쓰는 값, 클라이언트만 쓰는 값으로 나눈다.

/** 서버와 클라이언트가 함께 쓰는 값과 월드의 고정 치수. */
struct FLabScenarioConfig
{
	/** 노드와 NPC를 배치하는 영역의 반폭(cm). 바닥 반폭 100,000보다 안쪽이다. */
	static constexpr float WorldHalfExtent = 95000.f;

	/** 플레이어를 배치하는 원의 반지름(cm)과 원 위의 자리 수. */
	static constexpr float PlayerRingRadius = 50000.f;
	static constexpr int32 NumPlayerSlots = 16;

	FString Label = TEXT("unlabeled"); // -LabLabel=

	static const FLabScenarioConfig& Get();
};

/** 서버만 쓰는 값. */
struct FLabServerConfig
{
	int32 NumNodes = 5000;        // -LabNodes=
	int32 NumNpcs = 300;          // -LabNpcs=
	int32 Seed = 20261001;        // -LabSeed=
	int32 ExpectedClients = 8;    // -LabExpectedClients=
	float WarmupSeconds = 30.f;   // -LabWarmup=
	float MeasureSeconds = 60.f;  // -LabMeasureSeconds=

	bool bMeasure = false;        // -LabMeasure
	bool bShowcaseNpc = false;    // -LabShowcaseNpc (0번 자리 앞을 왕복하는 영상용 NPC 하나를 더 스폰한다)

	static const FLabServerConfig& Get();
};

/** 클라이언트만 쓰는 값. */
struct FLabClientConfig
{
	int32 Slot = 0;               // -LabSlot=
	bool bAutoMove = false;       // -LabAutoMove
	bool bAutoHarvest = false;    // -LabAutoHarvest (제자리에서 채집만 한다)
	bool bTopDown = false;        // -LabTopDown
	bool bAutoScreenshot = false; // -LabAutoScreenshot

	static const FLabClientConfig& Get();
};
