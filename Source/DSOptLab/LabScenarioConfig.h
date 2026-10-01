#pragma once

#include "CoreMinimal.h"

/** 실행 인자로 받는 시나리오 값. 프로세스당 한 번만 읽는다. */
struct FLabScenarioConfig
{
	/** 노드와 NPC를 배치하는 영역의 반폭(cm). 바닥 반폭 100,000보다 안쪽이다. */
	static constexpr float WorldHalfExtent = 95000.f;

	/** 플레이어를 배치하는 원의 반지름(cm)과 원 위의 자리 수. */
	static constexpr float PlayerRingRadius = 50000.f;
	static constexpr int32 NumPlayerSlots = 16;

	int32 NumNodes = 5000;        // -LabNodes=
	int32 NumNpcs = 300;          // -LabNpcs=
	int32 Seed = 20261001;        // -LabSeed=
	int32 ExpectedClients = 8;    // 서버: -LabExpectedClients=
	float WarmupSeconds = 30.f;   // 서버: -LabWarmup=
	float MeasureSeconds = 60.f;  // 서버: -LabMeasureSeconds=
	FString Label = TEXT("unlabeled"); // -LabLabel=

	bool bMeasure = false;        // 서버: -LabMeasure

	int32 ClientSlot = 0;         // 클라이언트: -LabSlot=
	bool bAutoMove = false;       // 클라이언트: -LabAutoMove
	bool bAutoHarvest = false;    // 클라이언트: -LabAutoHarvest (제자리에서 채집만 한다)
	bool bTopDown = false;        // 클라이언트: -LabTopDown
	bool bAutoScreenshot = false; // 클라이언트: -LabAutoScreenshot

	static const FLabScenarioConfig& Get();
};
