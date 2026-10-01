# 포스팅 1 후보 기법 자료 (`baseline3`)

구현 계획 태스크 10.3의 자료다. 에이전트가 `baseline3`의 트레이스를 Unreal Insights로 열어 [insights-reading.md](../../Docs/Guides/insights-reading.md)의 순서로 값을 읽고, 후보 기법 여섯 개의 엔진 소스 위치를 UE 5.8.3 소스(`G:\Epic Games\UE_Source`, 태그 `5.8.3-release`)에서 찾아 정리했다. 기법과 순서는 사용자가 고른다. 포스팅의 "관찰"과 "선택" 섹션(태스크 10.4)은 사용자가 쓴다.

- 대상 실행: `baseline3-r1`\~`r3`(클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 마스크 252, 송신 한도 350,000, 트레이스 켬). CSV 수치는 [STATUS.md](../../Docs/STATUS.md)의 "측정 결과"에 있다. 중앙값 실행은 `r1`이다(`work_avg_ms` 198.773).
- 1\~4절은 Insights 화면에서 읽은 값과 그 값으로 계산한 값이다. 계산에는 식을 적었다. 5절 "에이전트 의견"만 해석이다.
- 엔진 소스 경로는 엔진 루트 기준이고 줄 번호는 `5.8.3-release` 태그의 것이다. `NetDriver.cpp`는 `Engine/Source/Runtime/Engine/Private/NetDriver.cpp`를 줄인 것이다.

## 1. 서버 프레임 (Timing Insights)

세 실행 모두 Log View에서 `Lab_MeasureStart` 줄을 클릭하고 `Lab_MeasureEnd` 줄을 Shift-클릭해 측정 구간을 골랐다.

| 항목 | `r1` | `r2` | `r3` |
| --- | --- | --- | --- |
| `Lab_MeasureStart` | 1분 33.757853초 | 1분 33.237855초 | 1분 33.676777초 |
| `Lab_MeasureEnd` | 2분 33.885690초 | 2분 33.250394초 | 2분 33.798124초 |
| 선택 구간 | 60.127837초 | 60.012539초 | 60.121347초 |
| `Frame` Count | 303 | 293 | 335 |
| `WorldTick` Count | 302 | 292 | 334 |
| `GameNetDriver` Incl / Excl | 57.07초 / 21.11초 | 56.85초 / 21.08초 | 57.41초 / 21.22초 |
| `GameNetDriver`의 `WorldTick` 대비(`% Root`) | 95.17% | 95.00% | 95.80% |
| `LabResourceNode` Count / Incl (`GameNetDriver` 아래) | 12,082,416 / 29.75초 | 11,682,336 / 29.59초 | 13,362,672 / 30.2초 |
| `LabResourceNode`의 `% Parent` | 52.14% | 52.04% | 52.59% |
| `LabNpc` Count / Incl (`GameNetDriver` 아래) | 724,800 / 5.88초 | 700,800 / 5.86초 | 801,600 / 5.67초 |
| `LabNpc`의 `% Parent` | 10.31% | 10.31% | 9.87% |
| 나머지 액터 클래스 일곱 개의 `% Parent` 합 | 약 0.5% | 약 0.5% | 약 0.6% |
| `UNetConnection_ReceivedPacket`의 `% Parent`(패킷 수신) | 3.11% | 3.28% | 2.57% |
| `TickCompletionEvents`의 `% Parent`(액터 틱) | 1.20% | 1.19% | 1.15% |

- **Count가 실행마다 같은 식을 따른다.** `LabResourceNode`의 Count는 5,001(노드) × 8(연결) × `WorldTick` Count와 정확히 같다(`r1`: 5,001 × 8 × 302 = 12,082,416). `LabNpc`는 300 × 8 × `WorldTick` Count다(`r1`: 724,800). 모든 노드와 NPC가 모든 연결에 대해 매 프레임 한 번씩 처리된다.
- **나머지 일곱 개**는 `LabCharacter`, `ClientMoveResponsePacked`, `GameStateBase`, `LabPlayerController`, `GameplayDebuggerCategoryReplicator`, `PlayerState`, `WorldSettings`다. `r1`에서 각각 84.92ms 이하다.
- **타임라인을 확대하면 모든 프레임이 같은 모양이다.** `r1`의 측정 구간 가운데(1분 48초 무렵)에서 연속한 11프레임의 `Frame` 막대가 183.5\~214.35ms였고, 그 안의 일곱 줄(`FEngineLoop::Tick`부터 `GameNetDriver`까지)의 길이가 거의 같았다. 마우스를 올린 `GameNetDriver` 하나의 툴팁은 Inclusive 205.26ms, Exclusive 76.64ms(37.34%), `% of Root` 95.76%였다.
- **Timers 패널의 다른 큰 타이머.** `FWindowsPlatformFile_IterateDirectoryCommon_WithCallback`이 `r1` 16.61초, `r2` 15.26초, `r3` 21.4초다. `WorldTick` 아래에 없어서 게임 스레드의 프레임 시간에는 들어 있지 않다(`calib-f-r1`과 같다).

### CSV와 대조

| Insights에서 읽은 값 | 식 | `r1` | `r2` | `r3` | 대조할 CSV 열(`r1` / `r2` / `r3`) | 차이 |
| --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 | 선택 구간 ÷ `Frame` Count | 198.44ms | 204.82ms | 179.47ms | `work_avg_ms` 198.773 / 205.150 / 179.673 | -0.17% / -0.16% / -0.11% |
| 리플리케이션 시간 | `GameNetDriver` Incl ÷ `WorldTick` Count | 188.97ms | 194.69ms | 171.89ms | `netflush_avg_ms` 189.375 / 195.108 / 172.293 | -0.21% / -0.21% / -0.24% |

CSV `frames`(302, 292, 334)는 세 실행 모두 `WorldTick` Count와 같다. `calib-f-r1`에서 본 차이(0.1%, 0.2%)와 같은 수준이다.

## 2. `GameNetDriver` 안의 내역: 고려, 직렬화, 송신으로 나눌 수 있는가

**이 트레이스로는 일부만 나눌 수 있다.** 기본 트레이스(`-trace=default,net`)에서 `GameNetDriver` 아래에 보이는 타이머는 액터 클래스 이름의 타이머뿐이고, 나머지는 모두 `GameNetDriver`의 Excl에 들어간다. 엔진 소스에서 확인한 내용은 다음과 같다.

| 구간 | 엔진 소스 | Insights에서 보이는 곳 |
| --- | --- | --- |
| `GameNetDriver` 타이머 자체 | `UNetDriver::TickFlush`의 `TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*NetDriverDefinition.ToString())` (`NetDriver.cpp:1174`). 이 범위가 `ServerReplicateActors`, 연결마다의 `Connection->Tick`(`NetDriver.cpp:1304-1307`), `UpdateUnmappedObjects`를 감싼다 | `GameNetDriver` |
| 고려 목록 만들기 (프레임당 한 번, 모든 연결 공용) | `ServerReplicateActors_BuildConsiderList` (`NetDriver.cpp:5303`), 호출은 `NetDriver.cpp:6372` | Excl. `STAT_NetConsiderActorsTime`(`NetDriver.cpp:5305`)은 stat이라 기본 트레이스에 남지 않는다 |
| 연결마다의 우선순위 정렬 (관련성, 휴면 판정 포함) | `ServerReplicateActors_ForConnection` (`NetDriver.cpp:5938`)이 `SendClientAdjustment`(`NetDriver.cpp:5981`) 뒤에 `ServerReplicateActors_PrioritizeActors`(`NetDriver.cpp:6007`)를 부른다. 고려 목록 전체를 돈다(`NetDriver.cpp:5562`). 정렬은 `NetDriver.cpp:5669` | Excl. `STAT_NetPrioritizeActorsTime`(`NetDriver.cpp:5530`)은 stat이다 |
| 액터 하나를 연결 하나에 보내기 (프로퍼티 비교, 쓰기, 번치 전송) | `ServerReplicateActors_ProcessPrioritizedActorsRange`(`NetDriver.cpp:5830`)가 부르는 `UActorChannel::ReplicateActor`. 그 안의 `SCOPE_CYCLE_UOBJECT(ParentNativeClass, ...)`(`Engine/Source/Runtime/Engine/Private/DataChannel.cpp:3622-3625`)가 C++ 부모 클래스 이름으로 타이머를 남긴다 | `LabResourceNode`, `LabNpc` 등 클래스 이름 타이머 |
| 패킷 송신 | 연결의 송신 버퍼가 차면 `SendBunch` 안에서 `FlushNet`이 불려 클래스 타이머에 들어가고, 프레임 끝의 나머지는 `Connection->Tick`(`NetDriver.cpp:1306`)에서 나간다 | 클래스 타이머와 Excl에 나뉜다 |

**타임라인에서 본 모양(`r1`, 프레임 하나).** `GameNetDriver` 막대(205.26ms) 아래의 자식 타이머가 덩어리 8개로 뭉쳐 있고, 덩어리 사이마다 자식 타이머가 없는 빈 구간이 있다. 덩어리 경계에 `ClientMoveResponsePacked`(25.6μs)가 있다. 소스에서 `SendClientAdjustment`가 연결마다 `PrioritizeActors`보다 먼저 불리므로(`NetDriver.cpp:5981`, `6007`), 덩어리 하나가 연결 하나이고 그 앞의 빈 구간이 그 연결의 우선순위 정렬로 보인다. 같은 프레임에서 눈금으로 어림한 빈 구간은 3.6\~5.2ms, 덩어리는 16\~23ms였다. 덩어리 안에도 노드 호출(약 2.5μs) 사이마다 짧은 빈틈이 있다. 빈 구간을 8개 합쳐도 이 프레임의 Excl 76.64ms의 절반쯤이다(계산값: 3.6\~5.2ms × 8 = 29\~42ms). 나머지가 덩어리 안의 빈틈인지, 고려 목록 만들기나 프레임 끝의 `Connection->Tick`인지는 이 화면으로 구분하지 못했다.

**정리.**

- 직렬화(액터별 비교, 쓰기, 번치)는 클래스 타이머로 보인다: `LabResourceNode` 52%, `LabNpc` 10%.
- 고려(고려 목록, 관련성, 우선순위 정렬)와 송신(`Connection->Tick`)은 `GameNetDriver`의 Excl 37%에 섞여 있어 이 트레이스로는 나눌 수 없다.
- 나누려면 `-statnamedevents`를 더한 별도 실행이 필요하다(`Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp:1759`). 그러면 위 stat들이 Insights 타이머로 남는다. 다만 이 인자를 주면 클래스 타이머가 stat 경로로 바뀌어 이름이 달라질 수 있다(소스에서 읽은 추론이고 실행으로 확인하지 않았다). 측정 조건이 달라지므로 이번에는 실행하지 않았고, [backlog.md](../../Docs/Planning/backlog.md)의 "작업 중 떠오른 것"에 적었다.

## 3. 연결 하나의 패킷 (Network Insights, `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`)

패킷 막대를 클릭해 툴팁의 Engine Frame Number가 측정 시작 직후 프레임인 패킷을 찾고, 마지막 막대를 Shift-클릭했다.

| 항목 | `r1` | `r3` |
| --- | --- | --- |
| 첫 패킷 | Sequence 2,963 (툴팁 묶음의 가장 큰 패킷 2,965가 1분 33.80초, Engine Frame 989) | Sequence 14,739 (묶음의 가장 큰 패킷 14,742가 1분 33.71초, Engine Frame 1,220) |
| 끝 패킷 | Sequence 4,763, 2분 33.59초, Engine Frame 1,289 | Sequence 16,740, 2분 33.64초, Engine Frame 1,553 |
| 선택 범위 | 1,801패킷, 59.800초 | 2,002패킷, 60.100초 |
| `Actor` Count / Incl(비트) | 90,914 / 13,269,082 | 101,034 / 14,744,529 |
| `LabNpc` Count / Incl | 90,054 / 10,090,719 | 100,090 / 11,215,508 |
| `ReplicatedMovement` Incl | 8,379,693 | 9,313,798 |
| `BunchHeader` Incl | 3,121,902 | 3,466,662 |
| `ContentBlockHeader` Incl | 909,140 | 1,010,340 |
| `PacketHeaderAndInfo` Count / Incl | 1,801 / 149,181 | 2,002 / 165,836 |
| `GameStateBase` Count / Incl | 300 / 27,300 | 334 / 30,394 |
| `BP_ThirdPersonCharacter_C` Count / Incl | 300 / 19,800 | 334 / 22,044 |
| `ClientMoveResponsePacked` Count / Incl | 300 / 16,800 | 334 / 18,704 |
| `PlayerState` Count / Incl | 251 / 8,785 | 267 / 9,345 |
| `LabResourceNode` Count / Incl | 9 / 576 | 9 / 576 |

| Insights에서 읽은 값 | 식 | `r1` | `r3` | CSV `out_bytes_per_sec_per_conn` | CSV와의 차이 |
| --- | --- | --- | --- | --- | --- |
| 연결당 송신 대역폭(내용 기준) | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위 시간 | 28,048바이트/초 | 31,012바이트/초 | `r1` 28,896, `r3` 31,990 | CSV가 3.0%, 3.2% 크다 |
| 연결당 송신 대역폭(패킷 크기 기준 상한) | 패킷 수 × 1,023 ÷ 선택 범위 시간 | 30,810바이트/초 | 34,077바이트/초 | 같음 | CSV가 6.2%, 6.1% 작다 |

- **비트의 76%가 NPC 이동이다.** `LabNpc` ÷ `Actor` = 76.0%(`r1`), 76.1%(`r3`). `LabNpc` Count 90,054는 선택 범위 프레임 수(Engine Frame 989\~1,289, 301프레임)의 약 300배다. 프레임마다 NPC 300명분이 연결마다 나간다.
- **자원 노드는 60초에 9번, 576비트만 나간다.** 체력이 바뀐 검증용 노드다.
- **패킷 하나의 내용.** `r1` 측정 구간 가운데의 패킷 3,710(1분 58.37초, 1,018바이트)은 액터 번치 55개가 모두 `LabNpc`이고, Net Stats의 `ReplicatedMovement`가 번치당 평균 92비트(최대 95)다. `Connection 7`의 패킷 7,098(2분 26.27초, 1,011바이트)도 액터 번치 55개가 모두 `LabNpc`였다.
- `calib-f-r1`과 같은 모양이다(NPC 76.0%, 노드 9번 576비트).

## 4. 세 실행의 구성 비교

세 실행 사이에 `work_avg_ms`가 179.673\~205.150으로 흔들린다(변동 폭 25.477, 중앙값의 12.8%). 프레임당 값으로 나눠 보면 다음과 같다. 모두 Incl ÷ `WorldTick` Count다.

| 프레임당 | `r1` | `r2` | `r3` | `r2` ÷ `r3` |
| --- | --- | --- | --- | --- |
| 리플리케이션 시간(`GameNetDriver`) | 188.97ms | 194.69ms | 171.89ms | 1.133 |
| 그중 `LabResourceNode` | 98.5ms | 101.3ms | 90.4ms | 1.121 |
| 그중 `LabNpc` | 19.5ms | 20.1ms | 17.0ms | 1.182 |
| 그중 `GameNetDriver` Excl | 69.9ms | 72.2ms | 63.5ms | 1.136 |
| `GameNetDriver` Excl의 비율 | 37.0% | 37.1% | 37.0% | |
| `LabResourceNode` 호출 한 번 | 2.462μs | 2.533μs | 2.260μs | 1.121 |
| `LabNpc` 호출 한 번 | 8.113μs | 8.362μs | 7.073μs | 1.182 |
| 연결당 프레임당 송신(내용 기준) | 5,572바이트 | 읽지 않음 | 5,580바이트 | |

연결당 프레임당 송신은 (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위의 Engine Frame 수다(`r1` 13,418,263 ÷ 8 ÷ 301, `r3` 14,910,365 ÷ 8 ÷ 334).

**읽은 사실.** 하는 일의 양(호출 횟수 = 액터 수 × 연결 수 × 프레임 수, 프레임당 송신 비트)과 구성 비율(`LabResourceNode` 52.0\~52.6%, Excl 37.0\~37.1%)이 세 실행에서 같다. 느린 실행은 모든 항목이 비슷한 비율(1.12\~1.18배)로 느리다.

## 5. 후보 기법

단기에 구현하는 기법은 앞의 세 개뿐이다. 사용자는 이 세 개의 순서를 고른다. 나머지 세 개는 "관찰"에서 언급할 수 있는 참고 자료이고, 고르더라도 단기에는 구현하지 않고 [backlog.md](../../Docs/Planning/backlog.md)에 적는다. 마지막 열은 그 기법이 겨냥하는 비용의 `r1` 값이다(1\~3절).

| 기법 | 단기 구현 | 무엇을 줄이는가 | 구현 비용(바꿀 코드) | 관련 엔진 소스 | 클라이언트에 보이는 영향 | 기준선에서 겨냥하는 값(`r1`) |
| --- | --- | --- | --- | --- | --- | --- |
| 관련성(기본 컬 거리 복원) | 예 | 연결당 고려 대상과 열린 채널 수 | `LabResourceNode.cpp`, `LabNpc.cpp` 생성자에서 `bAlwaysRelevant = true;` 두 줄 제거 | `AActor::IsNetRelevantFor` (`Engine/Source/Runtime/Engine/Private/ActorReplication.cpp:388-419`), `AActor::IsWithinNetRelevancyDistance` (같은 파일 383\~386줄), `bAlwaysRelevant` (`Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:333`), 컬 거리 기본값 `SetNetCullDistanceSquared(225000000.0f)` (`Engine/Source/Runtime/Engine/Private/Actor.cpp:312`), 채널이 없는 액터만 관련성 검사 (`NetDriver.cpp:5580-5593`), 관련성을 잃은 채널 닫기 (`NetDriver.cpp:5877-5889`, `RelevantTimeout=5.0`은 `Engine/Config/BaseEngine.ini:1864`) | 150m 밖의 노드와 NPC가 클라이언트에서 사라진다(실행 중 스폰한 액터라 채널이 닫히면 클라이언트에서 지워진다, `NetDriver.cpp:5884`) | 노드 직렬화 98.5ms/프레임, NPC 직렬화 19.5ms/프레임, NPC 비트 76.0%, `open_actor_channels_per_conn` 5,314 |
| 자원 노드 휴면 | 예 | 변하지 않는 액터의 프레임당 고려 | `LabResourceNode.cpp`: 생성자에 `NetDormancy = DORM_DormantAll;`, `Harvest()`와 `Respawn()`에서 상태를 바꾸기 전에 `FlushNetDormancy()` | `ENetDormancy` (`Engine/Source/Runtime/Engine/Classes/Engine/EngineTypes.h:3597`), `AActor::NetDormancy` (`Actor.h:869`, 기본값 `DORM_Awake`는 `Actor.cpp:314`), `AActor::SetNetDormancy` (`Actor.cpp:3051`), `AActor::FlushNetDormancy` (`Actor.cpp:3102`), 모든 연결에서 휴면이면 활성 목록에서 뺌 `FNetworkObjectList::MarkDormant` (`Engine/Source/Runtime/Engine/Private/NetworkObjectList.cpp:290`, 348\~376줄), 고려 목록은 활성 목록만 돈다 (`NetDriver.cpp:5315`), 연결별 휴면 건너뛰기 `IsActorDormant` (`NetDriver.cpp:5499`, 5618\~5624줄) | 깨우기를 빠뜨리면 변경이 전달되지 않는다. 휴면 액터는 멀어져도 클라이언트에 남는다(engine-notes.md 나절 "휴면 액터와 관련성") | 노드 직렬화 98.5ms/프레임(노드 비트는 60초에 576비트) |
| NPC 업데이트 빈도 | 예 | 액터당 초당 고려 횟수 | `LabNpc.cpp` 생성자에 `SetNetUpdateFrequency(10.f);` | `AActor::SetNetUpdateFrequency` (`Actor.h:4624`, 기본값 100은 `Actor.cpp:295`), 다음 고려 시각 `NextUpdateTime = TimeSeconds + RandDelay + 1 / NetUpdateFrequency` (`NetDriver.cpp:5420-5425`), 그 시각 전이면 건너뜀 (`NetDriver.cpp:5319-5323`), `RandDelay`의 상한 `ServerTickTime = 1 / 30` (`NetDriver.cpp:6341-6348`) | 움직임이 끊겨 보일 수 있다 | NPC 직렬화 19.5ms/프레임, NPC 비트 76.0% |
| 적응형 업데이트 빈도 | 아니오 | 변하지 않는 액터의 고려 횟수 | 콘솔 변수 `net.UseAdaptiveNetUpdateFrequency=1` | `net.UseAdaptiveNetUpdateFrequency` 기본값 0 (`NetDriver.cpp:523-526`), 보낼 것이 없을 때 간격을 `1 / MinNetUpdateFrequency`(기본 2, `Actor.cpp:296`)까지 늘림 (`NetDriver.cpp:5384-5410`), 보낸 뒤 `OptimalNetUpdateDelta` 갱신 (`NetDriver.cpp:5849-5856`), 이 값은 콘솔 변수가 켜졌을 때만 쓴다 (`NetDriver.cpp:5420`) | 없음 | 노드 직렬화 98.5ms/프레임 |
| 푸시 모델 | 아니오 | 프로퍼티 비교 비용 | 프로퍼티 등록을 `DOREPLIFETIME_WITH_PARAMS_FAST`와 `bIsPushBased = true`로 바꾸고, 값을 바꿀 때 `MARK_PROPERTY_DIRTY_FROM_NAME`, 실행 시 `Net.IsPushModelEnabled=1` | `MARK_PROPERTY_DIRTY_FROM_NAME` (`Engine/Source/Runtime/Net/Core/Public/Net/Core/PushModel/PushModel.h:454`), `FDoRepLifetimeParams::bIsPushBased` (`Engine/Source/Runtime/Engine/Public/Net/UnrealNetwork.h:146`), `Net.IsPushModelEnabled` 기본값 false (`Engine/Source/Runtime/Net/Core/Private/Net/Core/PushModel/PushModel.cpp:434-439`), 푸시 프로퍼티는 더티일 때만 비교 (`Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1498-1510`), `WITH_PUSH_MODEL`은 Editor 타깃에서만 기본으로 켜진다 (`Engine/Source/Programs/UnrealBuildTool/Configuration/Rules/TargetRules.cs:1522-1527`). 이 프로젝트는 에디터 빌드로 실행하므로 켜져 있다 | 표시를 빠뜨리면 변경이 전달되지 않음 | 노드와 NPC 직렬화 중 프로퍼티 비교 부분(이 트레이스로는 나뉘지 않음) |
| 기본 송신 한도에서의 포화와 우선순위 | 아니오 | 한도 안에서 무엇을 먼저 보낼지 | 설정 파일의 송신 한도 세 키, 클래스의 `NetPriority` | 송신 한도 `ConfiguredInternetSpeed`, `MaxClientRate`, `MaxInternetClientRate` 기본값 100,000 (`Engine/Config/BaseEngine.ini:1839-1840`, `1860-1861`), `AActor::NetPriority` (`Actor.h:914`, 기본값 1.0은 `Actor.cpp:294`), `AActor::GetNetPriority` (`ActorReplication.cpp:48-92`), 우선순위 정렬 (`NetDriver.cpp:5669`), `UNetConnection::IsNetReady` (`Engine/Source/Runtime/Engine/Private/NetConnection.cpp:2731`), 포화되면 그 연결의 리플리케이션을 멈춤 (`NetDriver.cpp:5695`, `5868`) | 포화 시 갱신 지연 | 해당 없음(기준선은 한도 350,000에서 `saturated_ratio` 0.000) |

## 6. 에이전트 의견

화면에서 읽은 사실(1\~4절)에서 에이전트가 끌어낸 해석이다. 기법과 순서를 정하는 데 참고할 재료로 쓰고, 결론은 사용자가 낸다. 기대 효과는 모두 계산값이나 소스에서 읽은 추론이고 실행으로 확인하지 않았다.

### 가장 큰 비용

- **CPU를 가장 많이 쓰는 것은 변하지 않는 자원 노드를 연결마다 매 프레임 확인하는 일로 보인다.** 근거: `LabResourceNode`가 리플리케이션 시간의 52.1%(프레임당 98.5ms, `r1`)이고, 60초 동안 실제로 나간 노드 비트는 576비트(9번)다. 호출 한 번은 2.5μs로 작지만 프레임당 40,008번(5,001 × 8)이다.
- **그다음은 `GameNetDriver` 자체 시간 37%(프레임당 69.9ms)다.** 타임라인 모양과 소스로 보면 상당 부분이 연결마다 고려 목록 전체(노드와 NPC 5,301개 이상)를 돌며 우선순위를 매기는 일로 보인다(2절). 송신(`Connection->Tick`)이 얼마인지는 이 트레이스로 알 수 없다.
- **대역폭을 쓰는 것은 NPC다.** 비트의 76%가 NPC 이동이고, NPC 직렬화는 리플리케이션 시간의 10.3%다. CPU의 큰 비용(노드)과 대역폭의 큰 비용(NPC)이 다른 대상이다. `calib-f-r1`에서 본 것과 같다.

### 세 기법이 이 기준선에서 겨냥하는 것 (계산값, 미검증)

- **관련성.** 균등 배치에서 반경 150m 안의 기대 수는 노드 약 98개, NPC 약 6명이다(구현 계획 "기법별 코드" 가의 계산). 연결당 `ReplicateActor` 호출이 노드 5,001 → 약 98, NPC 300 → 약 6으로 줄면 클래스 타이머 몫(프레임당 약 118ms)과 NPC 비트의 대부분을 겨냥한다. 다만 우선순위 단계는 채널이 없는 액터에도 관련성 거리 검사를 하므로(`NetDriver.cpp:5580-5593`) 연결마다 고려 목록 전체를 도는 일은 남는다. Excl이 얼마나 줄지는 소스만으로 말할 수 없다.
- **자원 노드 휴면.** 모든 연결에서 휴면이 된 노드는 활성 목록에서 빠지고(`NetworkObjectList.cpp:348-376`), 고려 목록은 활성 목록만 돈다(`NetDriver.cpp:5315`). 그러면 노드의 클래스 타이머 몫(프레임당 98.5ms)과 우선순위 단계에서 노드가 차지하던 몫이 함께 빠진다. 노드는 원래 거의 보내지 않으므로 송신 대역폭은 거의 그대로일 것이다. 고려 목록에서 빠지는 액터 수로는 세 기법 중 가장 크다(5,000개).
- **NPC 업데이트 빈도 10.** 현재 서버 속도에서는 효과가 거의 없을 것으로 계산된다. 다음 고려 시각은 지금 + `RandDelay`(0\~1/30초) + 0.1초인데(`NetDriver.cpp:5420-5425`, `6341-6348`), 기준선의 프레임 간격이 약 0.18\~0.2초라서 다음 프레임에는 항상 그 시각이 지나 있다. 프레임 간격이 0.1초보다 짧아져야 건너뛰는 프레임이 생기고, 0.133초보다 길면 매 프레임 고려된다. 겨냥하는 비용도 리플리케이션 시간의 10.3%(프레임당 19.5ms)다. 대역폭(NPC 비트 76%)은 서버가 빨라진 뒤에 줄어들 여지가 있다.

### 순서를 고를 때 볼 점

- 관련성과 휴면은 둘 다 노드 비용을 겨냥한다. 먼저 적용한 쪽이 노드 비용 대부분을 가져가므로 뒤에 적용한 쪽은 남은 몫(관련성 뒤의 휴면은 연결당 약 98개, 휴면 뒤의 관련성은 NPC와 우선순위 단계)만 줄인다. 어느 순서든 포스팅마다 보이는 효과의 크기가 달라진다.
- NPC 업데이트 빈도는 위 계산대로라면 서버 프레임 간격이 0.1초 안쪽으로 들어온 뒤에 효과가 보인다. 이 기법을 먼저 적용하면 "이 측정으로는 차이를 구별하지 못했다"가 결과가 될 수 있다. 효과가 없었다는 결과도 포스팅 내용이라는 것이 구현 계획 11.4의 방침이다.
- 관련성은 기준선이 일부러 끈 엔진 기본 동작의 복원이고([ADR-0003](../../Docs/Decisions/0003-lawless-baseline.md)), 나머지 둘은 기본 동작 위의 개선이다. 이 구분은 태스크 10.6에서 포스팅에 적는다.
- 서버 프레임 시간이 줄면 초당 프레임 수가 늘어 초당 송신량은 오히려 늘 수 있다. 기준선의 연결당 프레임당 송신은 약 5,570\~5,580바이트이고 30Hz로 돌면 초당 약 172,000바이트다(STATUS.md의 30Hz 환산값). 한도 350,000 안이지만, 전후 비교에서 `out_bytes_per_sec_per_conn`은 `frames`와 함께 읽어야 한다.

### 실행 사이의 흔들림

- 4절에서 느린 실행은 일의 양과 구성 비율이 같고 모든 항목이 1.12\~1.18배 느렸다. 일이 늘어서가 아니라 같은 일을 CPU가 더 느리게 처리한 것으로 보인다(CPU 클럭, 같은 코어를 쓰는 다른 작업 같은 서버 밖의 요인). 원인은 확인하지 않았다.
- 기법 효과를 읽을 때 이 폭(중앙값의 12.8%)보다 큰 차이여야 한다. 위 계산대로라면 관련성과 휴면이 겨냥하는 몫은 리플리케이션 시간의 절반을 넘으므로 이 폭에 묻힐 가능성은 낮고, NPC 업데이트 빈도를 기준선 상태에 바로 적용하면 이 폭 안에 들어갈 수 있다.

## 7. 확인하지 않은 것

- `GameNetDriver` Excl의 내역. 빈 구간의 길이는 `r1`의 프레임 하나를 눈금으로 어림한 값이고, 빈 구간이 우선순위 정렬이라는 것은 `ClientMoveResponsePacked`의 위치와 소스 순서로 추정했다. 고려 목록 만들기와 `Connection->Tick`의 몫은 보지 않았다.
- `-statnamedevents`를 주면 클래스 타이머 이름이 바뀌는지(소스에서 읽은 추론).
- `r2`의 Network Insights 값.
- `Connection 1`\~`6`의 패킷 내용. `Connection 7`은 패킷 하나만 봤다.
- 패킷 범위의 시작. 툴팁은 화면 한 픽셀에 든 패킷 묶음(3\~4개) 중 가장 큰 것만 보여 주므로, 묶음의 첫 패킷(`r1` 2,963, `r3` 14,739)이 측정 시작 뒤 프레임인지는 확인하지 못했다. 앞뒤로 두세 패킷(선택 범위의 0.2% 이하) 어긋날 수 있다. 끝 패킷이 `Lab_MeasureEnd`보다 약 0.3초(`r1`)와 0.16초(`r3`) 앞이라 선택 범위가 Timing의 측정 구간보다 짧다.
- Insights 송신 대역폭과 CSV `out_bytes_per_sec_per_conn`이 3\~6% 다른 이유.
- `FWindowsPlatformFile_IterateDirectoryCommon_WithCallback`이 어느 스레드에서 무엇 때문에 도는지. `r3`(가장 빠른 실행)에서 가장 컸다(21.4초).
- 5절의 기대 효과. 모두 계산이나 소스에서 읽은 추론이다.

## 8. 스크린샷 후보

에이전트가 Insights 창(3000×2080)을 그대로 캡처했다. 포스팅에 넣을 것은 사용자가 고르거나 직접 찍는다. 모두 `images/`에 있다.

### `insights-r1-timers.png`

`r1` 측정 구간 선택, Timers 패널, `WorldTick`의 Callees(`GameNetDriver` 95.17%, `LabResourceNode` 52.14%). 1절의 근거다.

![r1 측정 구간의 Timers와 WorldTick Callees](images/insights-r1-timers.png)

### `insights-r1-frames.png`

`r1` 타임라인을 프레임 11개가 보이게 확대한 화면. 모든 프레임이 같은 모양이고, `GameNetDriver` 툴팁(Exclusive 37.34%)이 떠 있다. 1절의 근거다.

![r1 타임라인을 프레임 11개가 보이게 확대한 화면](images/insights-r1-frames.png)

### `insights-r1-one-frame.png`

`r1` 프레임 하나를 확대한 화면. `GameNetDriver` 아래 덩어리 8개와 그 사이의 빈 구간이 보인다. 2절의 근거다.

![r1 프레임 하나의 GameNetDriver 아래 덩어리 8개](images/insights-r1-one-frame.png)

### `insights-r1-net-stats.png`

`r1` `Connection 0` `Outgoing`에서 측정 구간 1,801패킷을 선택한 화면과 Net Stats(`LabNpc` 10,090,719비트). 3절의 근거다.

![r1 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-r1-net-stats.png)

### `insights-r1-packet.png`

`r1` 패킷 3,710 하나의 내용. 액터 번치 55개가 모두 `LabNpc`다. 3절의 근거다.

![r1 패킷 3,710의 내용](images/insights-r1-packet.png)

### `insights-r2-timers.png`

`r2` 측정 구간의 Timers와 Callees. 1절과 4절의 근거다.

![r2 측정 구간의 Timers와 WorldTick Callees](images/insights-r2-timers.png)

### `insights-r3-timers.png`

`r3` 측정 구간의 Timers와 Callees. 1절과 4절의 근거다.

![r3 측정 구간의 Timers와 WorldTick Callees](images/insights-r3-timers.png)

### `insights-r3-net-stats.png`

`r3` `Connection 0` `Outgoing`에서 측정 구간 2,002패킷을 선택한 화면과 Net Stats. 3절의 근거다.

![r3 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-r3-net-stats.png)

구현 계획 "시각 자료 규칙"의 포스팅용 이름(`timing.png`, `network.png`)으로는 아직 복사하지 않았다. 사용자가 고른 뒤에 정한다.
