# 포스팅 9 관찰 자료 (`act2-nodeuf1-r3`, `act2-nodeuf-split1-r3`)

2막 구현 계획 태스크 26의 자료다. 기법을 고르기 전에 쓴 후보 비교이고, 폴더 이름(`09-inventory`)은 기법이 정해지면 다시 정하는 임시 이름이다.

- 새로 측정하지 않았다. 기준 구성의 실행이 이미 있어서 [포스팅 8](../08-node-update-frequency/candidates.md)의 트레이스를 다시 읽었다. 바이트는 `act2-nodeuf1-r3`(기본 트레이스)를 Networking Insights 창에서 읽었고(2026-10-06), CPU는 `act2-nodeuf-split1-r3`(`-StatNamedEvents`)의 `Saved/InsightsExport/act2-nodeuf-split1-r3/summary.txt`에서 읽었다.
- 엔진 소스는 5.8.3(`G:\Epic Games\UE_Source`)에서 읽었다. 경로는 `Engine/Source/Runtime/` 기준이다.
- 1\~3절은 사실과 계산이고, 5절의 "예상"은 계산이다. 6절 "에이전트 의견"만 해석이다.

## 1. 기준 구성

[2막 구현 계획](../../Docs/Planning/2026-10-03-act-2-implementation-plan.md) 단계 5의 1번에 따라 직전 포스팅의 최종 구성을 기준으로 삼는다.

- 확정 명령에 포스팅 6의 2막 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`과 포스팅 8의 `-NodeUpdateFrequency 2`를 더한 구성이다. CSV `config` 열에는 이 인자들이 적힌다.
- 이 구성의 묶음은 `act2-nodeuf1`이다(`work_avg_ms` 중앙값 8.615, 변동 폭 0.223, `out_bytes_per_sec_per_conn` 중앙값 16,570). 고른 기법과 비교할 때는 몇 시간 떨어진 이 묶음 대신 기준 묶음을 새 라벨로 연달아 다시 잰다(STATUS.md "명령").
- 새 기법은 2막 설계 4절에 따라 실행 인자로 켜고 끄며, 기본값은 끔이다.

## 2. 인벤토리의 몫

### 2.1 바이트 (Networking Insights, `act2-nodeuf1-r3`)

`Game Instance 0 [Server]`, `Connection 0`(0번 자리, 채집 담당), `Outgoing`이다. 측정 구간은 `export-insights.ps1`이 정한 91.818\~151.819초(`WorldTick` 1,795프레임)다. 툴팁의 Engine Frame Number로 첫 막대(Largest Packet 프레임 2,376, 1분 31.88초)와 끝 막대(4,170, 2분 31.85초)를 골랐고, 2,120패킷, 59.973초였다. 화면 한 픽셀에 패킷 네 개가 들어가 경계가 한 프레임까지 어긋날 수 있다. 창 이미지는 [images/act2-nodeuf1-r3-netstats-connection0.png](images/act2-nodeuf1-r3-netstats-connection0.png)다.

| Net Stats 줄 | Count | Incl(비트) | I.Avg(비트) |
| --- | ---: | ---: | ---: |
| `Actor` | 36,284 | 7,435,255 | 204 |
| `LabNpc` | 25,182 | 2,549,362 | 101 |
| `LabInventoryComponent` | 127 | 2,182,034 | 17,181 |
| 그 아래 `Items` | 127 | 2,173,256 | 17,112 |
| 그 아래 `ItemId` | 24,000 | 1,090,560 | 45 |
| 그 아래 `Count` | 23,737 | 1,079,648 | 45 |
| `BP_LabCharacter_C` | 9,853 | 1,488,450 | 151 |
| `LabStateComponent` | 750 | 64,524 | 86 |
| `LabBuilding` | 134 | 5,494 | 41 |
| `LabResourceNode` | 14 | 997 | 71 |
| `PacketHeaderAndInfo`(`Actor` 밖) | 2,120 | 191,454 | 90 |

| 계산한 값 | 값 | 식 |
| --- | ---: | --- |
| 연결당 송신량 | 15,896바이트/초 | (7,435,255 + 191,454) ÷ 8 ÷ 59.973 |
| CSV `out_bytes_per_sec_per_conn`(`r3`)와의 차이 | -4.1% | 15,896 ÷ 16,569 − 1. CSV는 여덟 연결의 평균이다 |
| 인벤토리 | 4,548바이트/초 | 2,182,034 ÷ 8 ÷ 59.973 |
| 인벤토리 ÷ `Actor` | 29.3% | 2,182,034 ÷ 7,435,255 |
| 인벤토리 ÷ 연결당 송신량 | 28.6% | 2,182,034 ÷ 7,626,709 |
| NPC ÷ `Actor` | 34.3% | 2,549,362 ÷ 7,435,255 |
| 플레이어 캐릭터 ÷ `Actor` | 20.0% | 1,488,450 ÷ 7,435,255 |
| 한 칸의 비트 | 90.4비트 | (1,090,560 + 1,079,648) ÷ 24,000 |

- 포스팅 6의 최종 구성(`act2-update-frequency1-r1`)에서는 인벤토리가 4,583바이트/초, `Actor`의 29.5%였다([포스팅 6 관찰 자료](../06-three-techniques-again/candidates.md) 6절). 포스팅 8은 자원 노드만 바꿔서 인벤토리의 몫이 그대로다.
- 이 구성에서 인벤토리는 NPC 다음으로 큰 몫이고, 연결당 송신량의 4분의 1이 넘는다.

### 2.2 CPU (`act2-nodeuf-split1-r3`, 프레임당 ms)

타이머를 더 켠 실행이라 비율만 본다([engine-notes.md](../../Docs/Reference/engine-notes.md) 차절). `LabInventoryComponent`는 `Replicate Actor Time` → `LabCharacter` 아래에 있다.

| 타이머 | 프레임당 횟수 | Incl | `GameNetDriver` Incl(4.081) 대비 |
| --- | ---: | ---: | ---: |
| `LabCharacter` | 49.8 | 0.595 | 14.6% |
| `LabInventoryComponent` | 49.8 | 0.145 | 3.6% |
| 그 아래 `Dynamic Property Rep Time` | 49.8 | 0.067 | 1.6% |
| 그 아래 `Dynamic Property Compare Time` | 6.2 | 0.024 | 0.6% |

- `LabInventoryComponent`의 Excl은 0.041ms다. 서버 프레임 시간 평균 9.250ms의 1.6%다(0.145 ÷ 9.250).
- 비교(`Dynamic Property Compare Time`)는 프레임에 6.2번이다. 비교는 객체마다 프레임에 한 번 하고 그 결과를 연결들이 함께 쓴다(`Engine/Private/RepLayout.cpp:1275-1331`, engine-notes.md 자절). 200칸을 비교하는 일은 한 번에 약 3.9µs다(0.024 ÷ 6.2, 계산).
- `export-insights.ps1`의 트리는 부모의 타이머 ID로 자식을 모은다. 그래서 `Dynamic Property Rep Time`처럼 여러 부모 아래 나오는 타이머의 자식 줄(요약 파일에서 `LabInventoryComponent` 아래 `Dynamic Property Send Time` 114.8번 등)은 여러 곳의 값이 합쳐져 있어 쓰지 않았다. 위 표는 부모가 하나뿐인 줄이다.

## 3. 인벤토리가 바뀌는 방식

- 플레이어 캐릭터 8개에 `ULabInventoryComponent`가 하나씩 붙고, 칸은 200개다(`Source/DSOptLab/LabGameMode.cpp:363-369`). 칸은 `ItemId`와 `Count` 두 `int32`다(`LabInventoryComponent.h`의 `FLabItem`).
- 서버 타이머가 4 ÷ 8 = 0.5초마다 인벤토리 하나를 돌아가며 바꾼다(`LabGameMode.cpp:387-391`, `ChurnOneInventory`). 인벤토리 하나는 4초마다, 60초에 15번, 여덟 개 합쳐 120번 바뀐다.
- 바꾸는 일은 맨 앞 칸을 지우고(`RemoveAt(0)`) 맨 뒤에 새 칸을 더하는 것이다(`LabInventoryComponent.cpp`의 `Churn`). 뒤의 칸이 모두 한 칸씩 당겨진다.
- 일반 `TArray`는 같은 자리(인덱스)끼리 비교한다(`RepLayout.cpp:1692-1775`, `CompareProperties_Array_r`). 당겨진 칸은 모두 바뀐 칸이 된다. 2.1절의 `ItemId` Count 24,000은 120번 × 200칸과 같다. 바뀔 때마다 200칸을 모두 다시 보낸다.
- 나머지 7번(127 − 120)은 채집이다. 채집은 한 칸의 `Count`만 늘린다(`AddHarvest`). 이때는 그 칸만 보낸다.
- 인벤토리는 소유 조건 없이 리플리케이트된다(`DOREPLIFETIME(ULabInventoryComponent, Items)`). 연결 하나가 여덟 플레이어의 인벤토리를 모두 받는다. 클라이언트에서 인벤토리를 쓰는 곳은 화면 글자의 자기 인벤토리 칸 수와 첫 칸의 `ItemId`, 인벤토리 컴포넌트의 개수뿐이다(`LabHUD.cpp:100-109, 134`).

## 4. 후보

### A. FastArray(`FFastArraySerializer`)로 바꾸기

backlog.md "우선순위 순" 5번의 제목이다.

- **원리**: 칸마다 서버가 정한 번호(`ReplicationID`)와 바뀐 횟수(`ReplicationKey`)를 둔다(`Net/Core/Classes/Net/Serialization/FastArraySerializer.h:298-332`). 연결마다 지난번에 보낸 번호와 키의 표를 기억하고, 번호로 칸을 찾아 키가 달라진 칸과 없어진 번호만 보낸다(`FastArraySerializer.h:896-975`, `BuildChangedAndDeletedBuffers`). 자리가 당겨져도 번호와 키는 그대로라 보내지 않는다(같은 함수의 "Stayed the same, it might have moved but we dont care"). 배열 전체의 키(`ArrayReplicationKey`)가 그대로면 칸을 보지 않고 끝낸다(`819-853`, `ConditionalCreateNewDeltaState`).
- **보내는 것**: 머리(배열 키, 기준 키, 지운 수, 바뀐 수. `int32` 넷, `FastArraySerializer.h:980-1007`, `WriteDeltaHeader`), 지운 번호, 바뀐 칸의 번호와 칸 전체다. 칸 안의 바뀐 프로퍼티만 보내는 기능은 따로 켜야 한다(`SetDeltaSerializationEnabled`, `549-566`. 생성자의 기본값은 끔, `Net/Core/Private/Net/Serialization/FastArraySerializer.cpp:33`). 칸이 `int32` 둘이라 여기서는 상관없다.
- **구현**: `FLabItem`이 `FFastArraySerializerItem`을 상속하고, 배열을 감싼 `FFastArraySerializer` 하위 구조체에 `NetDeltaSerialize`와 `TStructOpsTypeTraits`의 `WithNetDeltaSerializer`를 둔다. `Fill`, `Churn`, `AddHarvest`에서 `MarkItemDirty`나 `MarkArrayDirty`를 부른다(`FastArraySerializer.h:441-473`). 끈 구성이 지금 코드와 같도록 FastArray 쪽을 별도 컴포넌트 클래스로 두고, 서버가 인자(예: `run-scenario.ps1 -InventoryFastArray`, 서버 `-LabInventoryFastArray`)로 붙일 클래스를 고르게 한다. 수십\~100줄이다.
- **대가**: 클라이언트는 지운 칸을 `RemoveAtSwap`으로 지운다(`FastArraySerializer.h:1193`). 클라이언트의 칸 순서가 서버와 달라진다. 화면 글자의 "first id"는 서버의 첫 칸이 아니게 되므로, 바뀐 것을 화면에서 확인할 다른 값(예: 가장 최근에 더한 칸의 `ItemId`)으로 바꿔야 한다. 바뀔 때 연결마다 200칸의 번호 표를 새로 만든다(`BuildChangedAndDeletedBuffers`의 `NewIDToKeyMap`).
- **바꾸지 않는 것**: 누가 받는지(여덟 연결 모두), 바꾸는 방식(맨 앞 칸 지우기).

### B. 소유자에게만 보내기(`COND_OwnerOnly`)

- **원리**: 리플리케이션 조건을 `COND_OwnerOnly`로 두면 연결이 그 액터의 소유자일 때만 그 프로퍼티를 보낸다(`CoreUObject/Public/UObject/CoreNetTypes.h:20`, 조건 표는 `Engine/Public/Net/RepLayout.h:135-149`의 `BuildConditionMapFromRepFlags`). 비교는 지금처럼 객체마다 한 번 하고(`RepLayout.cpp:1275`), 연결마다 꺼진 조건의 프로퍼티를 바뀐 목록에서 뺀다(`RepLayout.cpp:2047, 2660-2680`, `FilterChangeListToActive`).
- **구현**: `DOREPLIFETIME_CONDITION(ULabInventoryComponent, Items, COND_OwnerOnly)` 한 줄과 전환 인자다. `GetLifetimeReplicatedProps`에서 실행 설정(`FLabScenarioConfig`)을 읽어 조건을 고르면 된다. 수십 줄 안이다.
- **대가**: 다른 플레이어의 인벤토리를 클라이언트가 받지 못한다. 지금 화면 글자는 자기 인벤토리만 쓰므로 표시는 그대로다(3절). 바꾸는 것은 게임 규칙(누가 무엇을 아는가)이다. 소유자는 여전히 바뀔 때마다 200칸을 받는다.

### 고르지 않는 쪽으로 본 것

- **Push Model**(`MARK_PROPERTY_DIRTY`). 바뀌지 않은 프레임의 비교를 건너뛴다. 이 구성에서 인벤토리 비교는 프레임당 0.024ms라(2.2절) 줄일 것이 거의 없고, 보내는 바이트는 그대로다. backlog의 Push Model 후보(상태 값)에서 다룬다.
- **지우는 방식을 바꾸기**(`RemoveAtSwap`이나 빈 칸 표시). 칸이 당겨지지 않아 일반 `TArray`로도 두 칸만 보낸다. 그러나 테스트베드가 정한 일(맨 앞 칸을 지우고 맨 뒤에 더한다. 2막 설계 3.3절, 2026-10-03 사용자 결정)을 바꾸는 것이라 전후가 같은 일을 하지 않는다.
- **A와 B를 함께**. 효과가 섞여 "포스팅 하나에 기법 하나"에 맞지 않는다. 하나를 고르면 다른 하나는 backlog.md에 적는다.

## 5. 예상 (계산)

연결 하나(`Connection 0`) 기준이다. CPU는 2.2절의 몫이 작아 서버 프레임 시간의 변화로는 보이지 않을 것으로 둔다.

| 후보 | 인벤토리(바이트/초) | 연결당 송신량(바이트/초) | 인벤토리 CPU |
| --- | ---: | ---: | --- |
| 지금 | 4,548 | 15,896 | 0.145ms/프레임(타이머를 더 켠 실행) |
| A(FastArray) | 약 80 | 약 11,430(-28.1%) | 비교 0.024ms가 빠지고 연결마다의 번호 표 비용이 생긴다. 재 봐야 안다 |
| B(소유자에게만) | 약 570 | 약 11,920(-25.0%) | 비교는 그대로, 연결 7개의 직렬화가 빠진다 |

- A: 한 번 바뀔 때 머리 128비트 + 지운 번호 32비트 + 바뀐 칸(번호 32비트 + 칸 약 64\~90비트)으로 약 300비트로 둔다. 127번 × 300 ÷ 8 ÷ 59.973 = 약 79바이트/초. 연결당 송신량은 (7,626,709 − 2,182,034 + 127 × 300) ÷ 8 ÷ 59.973 = 약 11,428. 프로퍼티 머리 같은 덧붙는 비트는 재지 않았다.
- B: 120번 가운데 자기 인벤토리는 15번이다. 2,182,034 × 15 ÷ 120 ÷ 8 ÷ 59.973 = 약 568바이트/초. 연결당 송신량은 (7,626,709 − 2,182,034 × 105 ÷ 120) ÷ 8 ÷ 59.973 = 약 11,917. 채집 7번의 몫은 무시했다.
- 서버 프레임 시간: 인벤토리의 CPU 몫이 프레임의 1.6%라, 어느 쪽이든 `work_avg_ms`의 변화가 `act2-nodeuf1`의 변동 폭 0.223ms보다 작을 수 있다(계산이 아니라 짐작이다).

## 6. 에이전트 의견

**A(FastArray)를 추천한다.** 앞 칸을 지울 때 200칸을 다시 보내는 원인은 일반 배열이 자리로 비교하는 데 있다. A는 그 원인을 바꾸고, 누가 무엇을 받는지는 그대로 둔다. 인벤토리 바이트가 약 98% 줄어 B(약 87.5%)보다 크고, backlog.md의 이 포스팅 제목과 같다.

A를 고를 때 잃는 것:

- 구현이 B보다 크다(구조체 둘, 표시 확인값 변경, 전환용 컴포넌트 클래스).
- 클라이언트의 칸 순서가 서버와 달라진다. 순서가 의미 있는 인벤토리에는 따로 정렬 키가 필요하다는 것을 "배운 것과 한계"에 적게 된다.
- 결과는 대역폭에서 보이고 서버 프레임 시간에서는 거의 보이지 않을 것이다. 지금까지의 글은 서버 프레임 시간이 주된 결과였다. 이 글의 "결과"는 연결당 송신 대역폭이 중심이 된다.

B를 고르면 한 줄로 연결당 송신량을 약 25% 줄이고, "다른 플레이어에게 보낼 필요가 없는 데이터"라는 설계 판단을 글로 쓸 수 있다. 대신 소유자는 여전히 바뀔 때마다 200칸을 받아, 자리로 비교하는 원인은 남는다. B 뒤에 A를 하면 남은 1/8에서만 줄고, A 뒤에 B를 하면 약 80바이트/초에서 더 줄 것이 거의 없다.

## 7. 확인하지 않은 것

- FastArray 한 번의 실제 비트 수(5절의 300비트는 머리와 칸의 합으로 둔 값이다).
- FastArray가 연결마다 번호 표를 만드는 CPU 비용.
- 다른 연결(`Connection 1`\~`7`)의 인벤토리 몫. 모든 연결이 여덟 인벤토리를 받으므로 같다고 보았다.
- 기본 트레이스(`act2-nodeuf1-r3`)에서 인벤토리의 CPU. 기본 트레이스에는 `LabInventoryComponent` 타이머가 없다(`LabCharacter` 0.548ms에 들어 있다).
