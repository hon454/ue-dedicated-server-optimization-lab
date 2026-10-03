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

	// 1막의 세 기법을 켜고 끈다. 인자를 주지 않으면 세 기법이 모두 적용된 구성이다.
	// 액터의 생성자가 읽는다. 생성자는 클라이언트에서도 돌지만 세 값은 서버에서만 뜻이 있다.
	bool bAlwaysRelevant = false;      // -LabAlwaysRelevant (자원 노드, NPC, 건축물을 거리와 상관없이 모든 연결에 보낸다)
	bool bNodeDormancy = true;         // -LabNoNodeDormancy로 끈다 (자원 노드와 건축물)
	float NpcUpdateFrequency = 10.f;   // -LabNpcUpdateFrequency= (엔진 기본값은 100)

	// 2막의 확장 요소. 인자를 주지 않으면 1막의 시나리오다.
	// 0보다 크면 플레이어를 맵 가운데를 지나는 대각선 위에 이 간격(m)으로 놓고, 자리마다 다른 경로를 돌게 한다.
	float PlayerSpacingMeters = 0.f;   // -LabPlayerSpacing=
	// 0보다 크면 NPC와 플레이어 캐릭터에 상태 값(ULabStateComponent)을 붙이고, 액터 하나의 값이 평균 이 간격(초)마다 하나씩 바뀐다.
	float StateIntervalSeconds = 0.f;  // -LabStateInterval=
	// 0보다 크면 플레이어 캐릭터에 이 칸 수의 인벤토리(ULabInventoryComponent)를 붙인다.
	int32 InventoryItems = 0;          // -LabInventoryItems=
	// 0보다 크면 플레이어마다 이 간격(초)으로 인벤토리의 맨 앞 칸을 지우고 맨 뒤에 새 칸을 더한다.
	float InventoryChurnSeconds = 0.f; // -LabInventoryChurn=
	// 0보다 크면 플레이어가 있는 곳마다 건축물(ALabBuilding)을 이 수만큼 모아 놓는다. 밀집 배치에서는 한 무리다.
	int32 BuildingsPerCluster = 0;     // -LabBuildings=
	// 0보다 크면 무리마다 이 간격(초)으로 가장 오래된 건축물 하나를 허물고 새로 하나를 짓는다.
	float BuildIntervalSeconds = 0.f;  // -LabBuildInterval=
	// 0보다 크면 플레이어가 있는 곳마다 NPC(ALabNpc)를 이 수만큼 더 놓는다. 맵 전체에 놓는 -LabNpcs=와 따로다.
	int32 NpcsPerCluster = 0;          // -LabNpcsNearPlayers=

	/** 수치 CSV의 config 열에 적는 값. 기본값과 다른 인자를 ';'로 잇고, 모두 기본값이면 "default"다. */
	FString GetConfigName() const;

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
