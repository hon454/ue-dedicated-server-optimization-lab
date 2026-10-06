# 포스팅 10 관찰 자료 (`act2-invown1-r2`)

2막 구현 계획 태스크 27의 자료다. 기법은 이미 정해져 있다. 사용자가 인벤토리의 두 기법을 두 포스팅으로 나누어 포스팅 9에서 소유자 조건(`COND_OwnerOnly`), 포스팅 10에서 FastArray를 다루기로 정했다(2026-10-06, [포스팅 9 관찰 자료](../09-inventory-owner-only/candidates.md) 6절). 출발점은 그 관찰 자료 4절 A와 engine-notes.md 10절이다.

- 새로 측정하지 않았다. 기준 구성의 트레이스(`act2-invown1-r2`)가 이미 있어서 포스팅 9 관찰 자료 10, 11절의 값을 다시 썼다.
- 엔진 소스는 5.8.3(`G:\Epic Games\UE_Source`)에서 읽었다. 경로는 `Engine/Source/Runtime/` 기준이다.
- 1\~3절은 사실과 계산이다. 4절 구현 방식은 사용자가 정한다. 5절 "예상"은 계산이고, 6절 "에이전트 의견"만 해석이다.

## 1. 기준 구성

[2막 구현 계획](../../Docs/Planning/2026-10-03-act-2-implementation-plan.md) 단계 5의 1번에 따라 직전 포스팅의 최종 구성을 기준으로 삼는다.

- 확정 명령에 2막 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, 포스팅 8의 `-NodeUpdateFrequency 2`, 포스팅 9의 `-InventoryOwnerOnly`를 더한 구성이다.
- 이 구성의 묶음은 `act2-invown1`이다(`work_avg_ms` 중앙값 8.493, 변동 폭 0.153, `out_bytes_per_sec_per_conn` 중앙값 12,421, 변동 폭 33). 비교할 때는 이 묶음 대신 기준 묶음을 새 라벨로 연달아 다시 잰다(STATUS.md "명령").
- FastArray는 [2막 설계](../../Docs/Planning/2026-10-03-act-2-design.md) 4절에 따라 실행 인자로 켜고 끄며, 기본값은 끔이다.

## 2. 인벤토리의 몫 (`act2-invown1-r2`, `Connection 0`, `Outgoing`)

포스팅 9 관찰 자료 10절의 값이다. 고른 범위는 1,824패킷, 59.886초다.

| 항목 | 값 |
| --- | ---: |
| `LabInventoryComponent`(Count / Incl 비트) | 22 / 273,676 |
| 그 아래 `ItemId`(Count / Incl 비트) | 3,000 / 136,320 |
| 인벤토리(바이트/초) | 571 |
| 연결당 송신량(바이트/초, Insights) | 11,832 |
| 인벤토리 ÷ 연결당 송신량 | 4.8% |
| 앞 칸을 지운 한 번(패킷 17,626) | 18,190비트 |

- 22번은 자기 인벤토리가 바뀐 15번과 채집 7번이다. `ItemId` 3,000은 15번 × 200칸이다.
- 채집 한 번은 약 118비트다. (273,676 − 15 × 18,190) ÷ 7 = 118(계산). 한 칸의 `Count`만 보낸다.
- CPU는 포스팅 9 관찰 자료 2.2절(`act2-nodeuf-split1-r3`, `-StatNamedEvents`)의 값이 있다. 인벤토리 비교(`Dynamic Property Compare Time`)는 프레임당 6.2번, 0.024ms다. 한 번에 약 3.9µs다. 소유자 조건은 비교를 줄이지 않으므로(engine-notes.md 10절) 기준 구성에서도 같다고 본다.

## 3. FastArray가 하는 일

### 3.1 서버

- 칸마다 서버가 정한 번호(`ReplicationID`)와 바뀐 횟수(`ReplicationKey`)를 둔다. `MarkItemDirty`가 번호를 처음 정하고 키를 올린다. `MarkItemDirty`와 `MarkArrayDirty`는 배열 전체의 키(`ArrayReplicationKey`)도 올린다(`Net/Core/Classes/Net/Serialization/FastArraySerializer.h:441-466`).
- 연결마다 지난번에 보낸 번호와 키의 표를 기억한다. 배열 키가 표의 키와 같으면 칸을 보지 않고 끝낸다(`FastArraySerializer.h:1420-1431`, `819-853`).
- 다르면 칸을 모두 돌며 새 표(`NewIDToKeyMap`)를 만들고, 키가 바뀐 칸과 새 칸, 표에서 사라진 번호를 고른다. 자리가 당겨진 칸은 번호와 키가 그대로라 보내지 않는다(`FastArraySerializer.h:896-975`).
- 보내는 것은 머리(배열 키, 기준 키, 지운 수, 바뀐 수의 `int32` 넷), 지운 번호(`int32`), 바뀐 칸마다 번호(`uint32`)와 칸 전체다(`FastArraySerializer.h:979-1008`, `1466-1485`).
- 칸 전체는 칸의 프로퍼티를 차례로 직렬화한다(`Engine/Private/RepLayout.cpp:7170-7185`, `SerializePropertiesForStruct`). 칸 안의 바뀐 프로퍼티만 보내는 기능은 생성자 기본값이 꺼짐이다(`Net/Core/Private/Net/Serialization/FastArraySerializer.cpp:33`). 칸이 `int32` 둘이라 켜지 않는다.

### 3.2 일반 배열과 다른 처리

- FastArray 구조체는 사용자 정의 델타 프로퍼티(`IsCustomDelta`)로 등록된다(`RepLayout.cpp:5848-5855`). 이런 프로퍼티는 `IsLifetime` 표시를 받지 않아(`RepLayout.cpp:6318-6321`) 객체마다 하는 비교에서 빠진다(`RepLayout.cpp:1424-1431`, `CompareParentProperty`). 2절의 비교 0.024ms가 없어진다.
- 대신 연결마다 그 객체를 리플리케이트할 때 `ReplicateCustomDeltaProperties`가 돈다(`Engine/Private/DataReplication.cpp:1646`). 임시 버퍼(1,024비트)를 만들고(`1691`), 조건이 꺼진 연결은 건너뛴다(`1719-1724`). 소유자 조건은 FastArray에서도 그대로 쓰인다.

### 3.3 클라이언트

- 받은 칸 가운데 처음 보는 번호는 배열 맨 뒤에 더한다(`FastArraySerializer.h:1524`, `AddDefaulted_GetRef`).
- 지운 번호의 칸은 마지막에 `RemoveAtSwap`으로 지운다(`FastArraySerializer.h:1186-1197`). 맨 뒤의 칸이 지운 자리로 옮겨 온다.
- 이 테스트베드는 맨 앞 칸을 지우고 맨 뒤에 더한다. 클라이언트에서는 새 칸이 맨 뒤에 붙었다가 지운 칸의 자리로 옮겨 온다. 처음 바뀔 때는 0번 칸, 다음에는 1번 칸이다. 새 칸이 앞에서부터 차례로 들어간다(계산).
- 그래서 클라이언트의 칸 순서가 서버와 달라진다. 화면 글자의 "first id"(클라이언트 배열의 0번 칸)는 첫 번째 바뀜 뒤로는 더 바뀌지 않는다. 바뀐 것을 화면에서 확인하려면 다른 값을 써야 한다(4절).

## 4. 구현 방식

사용자가 추천안(공통 부모)을 골랐다(2026-10-06). 구현과 확인은 8절에 있다.

### 고른 안: 별도 컴포넌트 클래스와 공통 부모

- 새 클래스 `ULabInventoryFastArrayComponent`를 둔다. 칸은 `FLabFastItem`(`FFastArraySerializerItem`을 상속, `ItemId`와 `Count`)이다. 배열은 `FLabFastItemArray`(`FFastArraySerializer`를 상속)이고 `NetDeltaSerialize`와 `WithNetDeltaSerializer`를 둔다.
- `Fill`, `Churn`, `AddHarvest`는 지금 클래스와 같은 일을 한다. 같은 시드, 같은 난수 순서로 같은 아이템을 만든다. `Churn`은 `RemoveAt(0)` 뒤 `MarkArrayDirty`, 새 칸에 `MarkItemDirty`를 부른다. `AddHarvest`는 그 칸에 `MarkItemDirty`를 부른다.
- 조건은 지금과 같이 서버 인자 `-LabInventoryOwnerOnly`를 읽어 고른다.
- 서버 인자 `-LabInventoryFastArray`(스크립트 `run-scenario.ps1 -InventoryFastArray`)가 있으면 게임 모드가 새 클래스를 붙인다. CSV `config` 열에 `InventoryFastArray`가 더해진다.
- 게임 모드, 채집, 화면이 두 클래스를 같은 방식으로 부르도록 리플리케이트 프로퍼티가 없는 추상 부모 `ULabInventoryBase`를 둔다(`Fill`, `Churn`, `AddHarvest`, 칸 수, 칸 복사, 가장 최근에 더한 칸의 번호).
- 끈 구성의 `ULabInventoryComponent`는 부모만 바뀐다. 리플리케이트 프로퍼티(`Items`), 조건, `Churn`의 동작, 클래스 이름(Insights 타이머 이름)은 그대로다.
- 화면 글자의 "first id"를 "newest id"(가장 최근에 더한 칸의 `ItemId`)로 바꾼다. 일반 배열은 맨 뒤 칸, FastArray는 `ReplicationID`가 가장 큰 칸이다. 서버와 클라이언트에서 같은 값이고, 바뀔 때마다 바뀐다. 클라이언트 화면만 바뀌고 서버가 보내는 것은 바뀌지 않는다.
- 인벤토리 패널은 칸을 복사해 지금처럼 같은 자리끼리 비교한다.

### 고르지 않은 안: 부모 없이 분기

`ULabInventoryComponent`를 한 줄도 바꾸지 않고, 게임 모드, 채집, 화면의 세 곳에서 두 클래스를 따로 찾는다. 끈 구성의 코드가 그대로라는 것을 글에서 가장 짧게 말할 수 있다. 대신 같은 분기가 세 곳에 생긴다.

## 5. 예상 (계산)

### 5.1 바이트 (`Connection 0`)

| 한 번에 보내는 것 | 일반 배열(지금) | FastArray |
| --- | ---: | ---: |
| 앞 칸을 지우고 새 칸을 더함 | 18,190비트(측정) | 약 340비트 |
| 채집(한 칸의 수량) | 약 118비트(계산) | 약 300비트 |

- FastArray의 앞 칸 지우기: 머리 128 + 지운 번호 32 + 새 칸(번호 32 + `ItemId` 32 + `Count` 32) = 256비트다. 채집은 머리 128 + 바뀐 칸 96 = 224비트다.
- 둘 다 프로퍼티 머리 같은 덧붙는 비트를 약 80비트로 더했다. 일반 배열의 채집 118비트에서 `Count` 32비트를 뺀 86비트를 기준으로 잡은 짐작이다.
- 채집은 FastArray가 오히려 크다. 머리 128비트가 붙기 때문이다. 60초에 7번이라 몫은 작다.

| 계산한 값 | 기준(`act2-invown1-r2`) | FastArray 예상 | 변화 |
| --- | ---: | ---: | --- |
| 인벤토리(비트, 60초) | 273,676 | 약 7,200 | |
| 인벤토리(바이트/초) | 571 | 약 15 | 약 -556 |
| 연결당 송신량(바이트/초, Insights) | 11,832 | 약 11,276 | 약 -4.7% |
| CSV `out_bytes_per_sec_per_conn` | 12,421 | 약 11,870 | 약 -4.5% |

- 인벤토리: 15 × 340 + 7 × 300 = 7,200비트. 7,200 ÷ 8 ÷ 59.886 = 약 15바이트/초.
- CSV는 여덟 연결의 평균이다. 연결마다 자기 인벤토리가 60초에 15번 바뀌므로 모든 연결이 비슷하게 준다. 15 × (18,190 − 340) ÷ 8 ÷ 60 = 약 558바이트/초. 12,421 − 558 = 약 11,863.
- 기준 묶음의 CSV 변동 폭은 33이다(`act2-invown1`). 약 550의 차이는 이보다 크다.

### 5.2 CPU

| 바뀌는 것 | 크기 | 근거 |
| --- | --- | --- |
| 객체마다의 200칸 비교가 없어짐 | 프레임당 약 -0.024ms | 2절, 3.2절 |
| 소유자에게 200칸을 직렬화하던 일이 없어짐 | 60초에 120번(여덟 인벤토리 합) | 2절 |
| 연결마다 리플리케이트할 때 임시 버퍼와 조건 확인 | 프레임당 약 49번(`LabCharacter` 리플리케이트 49.3번, 포스팅 9 관찰 자료 11절) | `DataReplication.cpp:1691, 1719` |
| 소유자 연결에서 배열 키 비교 | 같은 횟수, 바뀌지 않았으면 키 하나 비교로 끝 | `FastArraySerializer.h:819-853` |
| 바뀔 때 소유자 연결의 새 번호 표(200칸) | 60초에 120번 + 채집 7번 | `FastArraySerializer.h:896-975` |

- 번호 표는 바뀔 때만, 소유자 연결 하나에서만 만든다. 소유자 조건이 없으면 받는 연결 여덟이 모두 만든다.
- 합은 프레임당 0.01\~0.02ms 줄어드는 정도로 본다(짐작이다. 임시 버퍼와 번호 표의 비용은 재지 않았다). `work_avg_ms`의 변동 폭 0.094\~0.153보다 작아 서버 프레임 시간으로는 구별하지 못할 것이다.

### 5.3 인벤토리 패널

- 기준: 자기 격자가 4초마다 200칸 통째로 번쩍인다(포스팅 9의 `visual15`).
- FastArray: 4초마다 한 칸만 번쩍인다. 번쩍이는 칸은 바뀔 때마다 0번, 1번, 2번 칸으로 앞에서부터 옮겨 간다(3.3절). 채집하는 클라이언트라면 채집할 때 한 칸이 더 번쩍인다.

## 6. 에이전트 의견

- **4절은 추천안(공통 부모)을 권한다.** 같은 분기를 세 곳에 두지 않아 코드가 짧다. 끈 구성에서 바뀌는 것은 부모 클래스뿐이고, 리플리케이트 프로퍼티와 비교 대상이 같아 보내는 비트와 비교 비용이 같다. 끈 구성을 연달아 다시 재므로(1절) 차이가 있으면 기준 묶음에 드러난다.
- **결과는 포스팅 9처럼 연결당 송신 대역폭이 중심이다.** 줄어드는 몫(약 -4.5%)은 포스팅 9(-25.3%)보다 작다. 글의 중심은 "한 번 바뀔 때 18,190비트가 약 340비트로"이고, Networking Insights의 한 번 값과 인벤토리 패널의 한 칸 번쩍임으로 보이는 것이 맞다고 본다.
- **대가는 "배운 것과 한계"에 쓴다.** 클라이언트의 칸 순서가 서버와 달라진다. 순서가 의미 있는 인벤토리에는 따로 정렬 키가 필요하다. 채집 같은 작은 바뀜은 머리 128비트 때문에 오히려 커진다.

## 7. 확인하지 않은 것

- FastArray 한 번의 실제 비트 수(5.1절의 340비트와 300비트는 계산과 짐작이다).
- 프로퍼티 머리(`WritePropertyHeaderAndPayload`)의 비트 수.
- 연결마다의 임시 버퍼와 번호 표의 CPU 비용. 측정 전에는 `Custom Delta Property Rep Time`(`DataReplication.cpp:35`, `1661`의 `CONDITIONAL_SCOPE_CYCLE_COUNTER`)이 기록되지 않는다고 적었으나 틀렸다. `-StatNamedEvents` 실행에서 기록되었다(12절).
- 클라이언트가 칸을 앞에서부터 채운다는 3.3절의 순서. 8.2절의 스크린샷에서 맨 윗줄의 한 칸만 번쩍인 것까지 보았고, 칸 번호를 하나씩 따라가지는 않았다.

## 8. 구현과 작은 규모 확인 (2026-10-06)

### 8.1 구현

- 실행 인자 `run-scenario.ps1 -InventoryFastArray`(서버 `-LabInventoryFastArray`). 게임 모드가 `ULabInventoryComponent` 대신 `ULabInventoryFastArrayComponent`를 붙인다. CSV `config` 열에 `InventoryFastArray`가 더해진다.
- 두 클래스의 부모는 `ULabInventoryBase`(리플리케이트 프로퍼티 없음)다. 소유자 조건은 두 클래스가 같은 함수(`GetItemsCondition`)로 고른다.
- 모듈 의존성에 `NetCore`를 더했다(`FFastArraySerializer`가 그 모듈에 있다. 더하지 않으면 링크 오류).
- 화면 글자의 "first id"를 "newest id"로 바꿨다(4절).

### 8.2 작은 규모 확인

모두 클라이언트 8, 자원 노드 100과 검증용 1, NPC 10, 2막 요소를 줄인 값(`-PlayerSpacing 3 -NpcsNearPlayers 5 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 20 -BuildInterval 1`), `-NodeUpdateFrequency 2`, 준비 20초, 측정 30초, 트레이스 끔이다. 실행 한 번씩이라 방향만 본다.

| 라벨 | 더한 인자 | `out_bytes_per_sec_per_conn` | 확인한 것 |
| --- | --- | ---: | --- |
| `tsmall-fastarr-all1-r1` | `-InventoryFastArray -InventoryPanelSlot 0` | 5,397 | 0번 창(`tpp-01`, `tpp-02`): 다른 플레이어 일곱의 격자가 모두 차 있고, 격자 하나에서 맨 윗줄의 한 칸만 흰색이다. "newest id"가 867(t=30초) → 318(t=45초) |
| `tsmall-fastarr-on1-r1` | 위 + `-InventoryOwnerOnly` | 5,360 | 자기 인벤토리 200칸, "other players: 0 of 7 received", "other items=0" |
| `tsmall-fastarr-off2-r1` | `-InventoryOwnerOnly` | 5,907 | 바로 다음 실행과 비교하는 기준 |
| `tsmall-fastarr-on2-r1` | `-InventoryOwnerOnly -InventoryFastArray`, 서버 로그 `LogNetFastTArray Log` | 5,300 | 아래 표. 로그 때문에 서버가 느려 CPU 값은 쓰지 않는다 |

- 연달아 잰 `off2` → `on2`에서 `out_bytes_per_sec_per_conn`가 607 줄었다. 5.1절의 예상(약 -558)과 방향과 크기가 맞는다.
- `on1`(5,360)은 포스팅 9의 같은 규모 실행 `tsmall-invpanel-on1-r1`(5,221, 일반 배열 + 소유자 조건)보다 크다. 몇 시간 떨어진 실행이고, 같은 구성의 `off2`(5,907)와도 686 차이가 나서 작은 규모 실행 사이의 변동으로 보았다.
- 로그 줄은 `run-scenario.ps1`에 서버 인자 `-LogCmds="LogNetFastTArray Log"`를 임시로 더해 얻었고, 확인한 뒤 되돌렸다.

`tsmall-fastarr-on2-r1` 서버 로그의 `Writing Bunch` 줄(실행 전체, `FastArraySerializer.h:991-993`):

| 줄 | 횟수 | 뜻 |
| --- | ---: | --- |
| `NumChange: 200. NumDel: 0` | 8 | 소유자 연결에 처음 보낸 200칸 |
| `NumChange: 1. NumDel: 1` | 100 | 앞 칸 지우기(새 칸 하나, 지운 번호 하나). 인벤토리 여덟이 0.5초마다 돌아가며 바뀐다 |
| `NumChange: 1. NumDel: 0` | 6 | 채집 |
| `NumChange: 0. NumDel: 0 [0/-1]` | 64 | 8 × 8. 기본값에서 시작하는 기준 상태를 만든 것으로 보인다(확인하지 않았다) |

- 바뀌지 않았을 때 쓴 줄은 없다. 3.1절의 "배열 키가 같으면 칸을 보지 않고 끝낸다"와 맞는다.

## 9. 사용자와 논의할 것

- **FastArray의 추가, 삭제, 변경에서 일어나는 일을 글에서 자세히 보이기**(사용자 의견, 2026-10-06. 논의한 뒤 적용한다). 정해지면 여기에 적는다.
- 에이전트 안(채팅으로 냈다): 본문은 2,000\~4,000자라 글로 풀지 않고 도식 하나와 작은 표로 보인다. 도식은 `Scripts/make-<이름>.ps1`로 만드는 SVG이고, 세 줄에 서버 배열, 실제로 가는 것, 클라이언트 배열을 그린다. 경우마다의 비트는 측정 기록에 둔다.

| 경우 | 서버가 보내는 것 | 클라이언트가 하는 일 | 이 테스트베드에서 |
| --- | --- | --- | --- |
| 추가 | 머리 + 새 칸의 번호와 내용 | 배열 맨 뒤에 붙임(`PostReplicatedAdd`) | 앞 칸 지우기의 절반 |
| 삭제 | 머리 + 지운 번호 | 마지막에 `RemoveAtSwap`(`PreReplicatedRemove`) | 앞 칸 지우기의 나머지 절반 |
| 변경 | 머리 + 바뀐 칸의 번호와 내용 | 같은 칸을 덮어씀(`PostReplicatedChange`) | 채집 |

## 10. 측정 조건

사용자가 측정 계획(두 묶음 3회씩, `-StatNamedEvents` 실행 둘, 패널 녹화)을 승인한 뒤의 측정이다(2026-10-06).

- 구성: 1절의 기준 구성. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초, 트레이스 켬.
- 2026-10-06 17:31\~17:52, 본체 화면, 실행 중 PC 조작 없음. 8회 모두 종료 코드 0이고 시작 신호 전 클라이언트 재시작이 없었다.

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-fastarr-base1` | 적용 전 | 없음 | `r3` |
| `act2-fastarr1` | 적용 후 | `-InventoryFastArray` | `r2` |
| `act2-fastarr-base-split1` | (CPU 나누기) | `-StatNamedEvents` | 한 번 |
| `act2-fastarr-split1` | (CPU 나누기) | `-InventoryFastArray -StatNamedEvents` | 한 번 |

- 서버 로그는 여덟 실행이 같았다: `lab_buildings clusters=1 per_cluster=500`, `lab_npcs_near_players clusters=1 per_cluster=50`, `lab_nodes_moved_from_harvest_spot=1`, 준비 구간 끝의 `open_actor_channels_per_conn` 77\~79, `lab_consider_list avg_per_frame` 412.5\~415.9, 경고 없음.
- 자동 스크린샷(`act2-fastarr1-r2-tpp-01`): "own items=200 newest id=867 | other items=0".

## 11. CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-fastarr-base1-r1` | 1798 | 8.929 | 14.228 | 1 | 4.419 | 12364 |
| `act2-fastarr-base1-r2` | 1786 | 8.748 | 12.680 | 1 | 4.346 | 12390 |
| `act2-fastarr-base1-r3` | 1787 | 8.852 | 13.237 | 1 | 4.370 | 12387 |
| `act2-fastarr1-r1` | 1789 | 8.650 | 12.887 | 1 | 4.240 | 11807 |
| `act2-fastarr1-r2` | 1797 | 8.586 | 13.355 | 1 | 4.181 | 11799 |
| `act2-fastarr1-r3` | 1789 | 8.361 | 12.044 | 0 | 4.068 | 11828 |
| `act2-fastarr-base-split1-r1` | 1796 | 9.446 | 14.388 | 0 | 4.524 | 12382 |
| `act2-fastarr-split1-r1` | 1785 | 9.878 | 14.593 | 1 | 4.838 | 11769 |

- `out_bytes_per_sec_per_conn` 중앙값 12,387 → 11,807(-580, -4.7%). 변동 폭은 26과 29다. 5.1절의 예상은 약 11,870(약 -4.5%)이었다.
- `work_avg_ms` 중앙값 8.852 → 8.586(-0.266). 변동 폭 0.181, 0.289 가운데 큰 쪽보다 작아 구별하지 못했다(measurement.md "차이의 판단").
- `netflush_avg_ms` 중앙값 4.370 → 4.181(-0.189)은 변동 폭 0.073, 0.172보다 조금 크다. 같은 묶음의 `work_avg_ms`가 구별되지 않아 따로 주장하지 않는다.
- `-StatNamedEvents` 두 실행의 수치는 다른 실행과 비교하지 않는다(STATUS.md "명령"). 바이트는 위와 같은 방향이다(12,382 → 11,769).

## 12. Networking Insights (`Connection 0`, `Outgoing`, 중앙값 실행끼리)

포스팅 9 관찰 자료 10절과 같은 방법이다. 창을 최대화하고(2912 폭 화면), 툴팁의 Engine Frame Number로 측정 구간 안의 첫 막대와 끝 막대를 골랐다. 창 이미지는 [images/act2-fastarr-base1-r3-netstats-connection0.png](images/act2-fastarr-base1-r3-netstats-connection0.png), [images/act2-fastarr1-r2-netstats-connection0.png](images/act2-fastarr1-r2-netstats-connection0.png)다(측정 기록용).

| 항목 | 적용 전(`act2-fastarr-base1-r3`) | 적용 후(`act2-fastarr1-r2`) |
| --- | --- | --- |
| 측정 구간(`export-insights.ps1`) | 92.054\~152.074초, 프레임 2,381\~4,168 | 91.478\~151.510초, 프레임 2,369\~4,166 |
| 첫 막대 / 끝 막대(Engine Frame Number) | 2,382 / 4,165 | 2,369 / 4,164 |
| 고른 범위 | 1,815패킷, 59.884초 | 1,796패킷, 59.964초 |

| Net Stats 줄(Count / Incl 비트, I.Max) | 적용 전 | 적용 후 |
| --- | ---: | ---: |
| `Actor` | 35,954 / 5,499,105, 최대 7,630 | 35,900 / 5,227,069, 최대 669 |
| 인벤토리 컴포넌트 | `LabInventoryComponent` 22 / 273,676, 최대 18,190, 평균 12,439 | `LabInventoryFastArrayComponent` 22 / 7,086, 최대 345, 평균 322 |
| 그 아래 배열 프로퍼티 | `Items` 22 / 272,248 | `Inventory` 22 / 6,096, 최대 300 |
| 칸 | `ItemId` 3,000 / 136,320, `Count` 2,977 / 135,400 | `ChangedElement` 22 / 2,382, `ItemId` 15 / 480, `Count` 22 / 704 |
| `LabNpc` | 25,124 / 2,543,522 | 25,205 / 2,551,698 |
| `BP_LabCharacter_C` | 9,785 / 1,476,353 | 9,706 / 1,466,105 |
| `LabStateComponent` | 744 / 64,136 | 746 / 62,836 |
| `LabBuilding` | 135 / 5,535 | 134 / 5,494 |
| `PacketHeaderAndInfo`(`Actor` 밖) | 1,815 / 166,639 | 1,796 / 165,232 |

| 계산한 값 | 적용 전 | 적용 후 | 변화 |
| --- | ---: | ---: | --- |
| 연결당 송신량(바이트/초) | 11,826 | 11,241 | -585(-4.9%) |
| CSV(중앙값 실행)와의 차이 | -4.5%(12,387) | -4.7%(11,799) | |
| 인벤토리(바이트/초) | 571 | 15 | -556(-97.4%) |
| 인벤토리 ÷ 연결당 송신량 | 4.8% | 0.1% | |
| NPC(바이트/초) | 5,309 | 5,319 | 그대로 |
| 플레이어 캐릭터(바이트/초) | 3,082 | 3,056 | 그대로 |
| 초당 패킷 수 | 30.31 | 29.95 | -1.2% |

- 식: 연결당 송신량 = (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 고른 시간. 인벤토리 = 컴포넌트 Incl ÷ 8 ÷ 고른 시간.
- 적용 전의 인벤토리 값(22번, 273,676비트)은 포스팅 9의 `act2-invown1-r2`와 같다.
- 연결당 송신량이 준 몫의 95%가 인벤토리다(556 ÷ 585).
- 한 번 바뀔 때의 최대는 18,190 → 345비트(약 53분의 1)다. 5.1절의 예상(앞 칸 지우기 약 340비트)과 맞는다. 60초 합 7,086비트도 예상(약 7,200)과 맞는다.
- `Actor` 줄의 최대가 7,630 → 669비트다. 한 패킷에 다 들어가지 않아 여러 패킷으로 나뉘던 큰 묶음이 없어졌다. 그래프에서도 약 4초마다 솟던 막대가 사라졌다(창 이미지).
- 적용 후 `ItemId`가 15번, `Count`가 22번이다. 앞 칸 지우기 15번은 새 칸의 두 값을, 채집 7번은 `Count`만 보낸 것으로 보인다. 칸 안의 바뀐 프로퍼티만 보내는 기능은 꺼져 있으므로(3.1절) 이렇게 나뉘는 까닭은 확인하지 않았다.

## 13. Timing Insights (프레임당 ms)

중앙값 실행은 `Saved/InsightsExport/act2-fastarr-base1-r3/summary.txt`, `Saved/InsightsExport/act2-fastarr1-r2/summary.txt`다. 프레임 수는 `WorldTick` Count가 CSV `frames`와 같았다.

| 타이머 | 적용 전 | 적용 후 |
| --- | ---: | ---: |
| 서버 프레임 시간 평균 / P99 | 9.133 / 13.589 | 8.866 / 13.740 |
| `GameNetDriver` Incl / Excl | 4.112 / 2.536 | 3.933 / 2.406 |
| `LabCharacter`(횟수, Incl) | 49.6번, 0.491 | 49.1번, 0.481 |
| `LabNpc`(횟수, Incl) | 114.9번, 0.892 | 114.5번, 0.868 |

`-StatNamedEvents` 실행(`Saved/InsightsExport/act2-fastarr-base-split1-r1/summary.txt`, `act2-fastarr-split1-r1/summary.txt`)의 `Replicate Actor Time` → `LabCharacter` 아래:

| 타이머(횟수, Incl, Excl) | 적용 전 | 적용 후 |
| --- | --- | --- |
| 서버 프레임 시간 평균(참고) | 9.738 | 10.180 |
| `LabCharacter` | 49.2번, 0.526, 0.193 | 49.7번, 0.556, 0.209 |
| 인벤토리 컴포넌트 | `LabInventoryComponent` 49.2번, 0.104, 0.038 | `LabInventoryFastArrayComponent` 49.7번, 0.117, 0.040 |
| 그 아래 `Dynamic Property Rep Time` | 49.2번, 0.032 | 없음 |
| 그 아래 `Dynamic Property Compare Time` | 6.2번, 0.024 | 없음 |
| 그 아래 `Custom Delta Property Rep Time` | 없음 | 49.7번, 0.055, 0.033 |
| 참고: `LabNpc` | 114.4번, 0.942 | 115.0번, 0.989 |
| 참고: `Prioritize Actors Time` | 0.999 | 1.082 |

- 인벤토리 컴포넌트의 몫은 0.104 → 0.117ms로 줄지 않았다. 5.2절의 예상(0.01\~0.02ms 감소)은 틀렸다.
- 비교(0.024ms)와 일반 프로퍼티 처리(0.032ms)는 없어졌다. 대신 연결마다 리플리케이트할 때(프레임당 49.7번, 한 번 약 1.1µs) 사용자 정의 델타 처리가 0.055ms 생겼다(3.2절).
- 실행 하나씩이고, 적용 후 실행은 서버 전체가 느렸다(서버 프레임 시간 +4.5%, `LabNpc` +5.0%, `Prioritize Actors Time` +8.3%). 인벤토리의 +12.5%에서 이만큼을 빼면 "줄지 않았고 비슷하거나 조금 늘었다"까지가 말할 수 있는 범위다.

## 14. 시각 자료

- 인벤토리 패널(`-InventoryPanelSlot 2`). `visual18`(적용 전 구성)과 `visual19`(적용 후)의 측정 구간에서 2번 창 영역 `1920,0,960,540`을 10초씩 MP4로 찍었다(`Saved/Screenshots/Lab/visual18-panel.mp4`, `visual19-panel.mp4`). 두 실행의 수치는 쓰지 않는다.
- GIF는 패널만 잘라 만들었다([images/inventory-panel-before.gif](images/inventory-panel-before.gif), [images/inventory-panel-after.gif](images/inventory-panel-after.gif), 본문 후보). 명령은 STATUS.md "명령"의 인벤토리 패널 줄에 있다. AGENTS.md의 GIF 기본값(폭 960px, 48색)과 다르다. 폭 915px, 64색이다.
- 처음 찍은 `visual16`, `visual17`의 48색 GIF에서는 적용 후의 한 칸 번쩍임이 한 프레임도 남지 않았다. 원본 MP4는 스크립트가 지워 다시 변환할 수 없었고, 사용자가 다시 찍기를 승인했다(2026-10-06).
- 확인: 적용 전 GIF는 자기 격자 200칸이 약 4초 간격으로 세 번 흰색이 된다(80프레임 가운데 16\~18, 48\~50, 80번째). 적용 후 GIF는 한 칸(약 81픽셀)만 9\~11, 41\~43, 73\~75번째 프레임에서 흰색이고, 칸이 맨 윗줄에서 한 칸씩 오른쪽으로 옮겨 간다. MP4에서는 그 칸이 12프레임(0.4초) 흰색이고 간격은 121프레임(4.03초)이다. 3.3절의 "새 칸이 앞에서부터 차례로 들어간다"를 화면으로 확인했다.

## 15. 에이전트 의견

- **대역폭은 예상대로 줄었다.** 한 번 바뀔 때 18,190 → 345비트이고, 연결당 송신량은 4.7\~4.9% 줄었다. 포스팅 9(-25.3%)보다 작은 것은 소유자 조건 뒤에 남은 인벤토리가 한 연결의 4.8%뿐이었기 때문이다.
- **글의 중심은 "한 번에 보내는 양"으로 잡는 것을 권한다.** 연결당 송신량의 변화는 작지만, 한 번의 크기(약 53분의 1)와 패킷 그래프에서 솟던 막대가 사라진 것, 패널에서 200칸이 한 칸으로 줄어든 것이 원리를 바로 보여 준다.
- **CPU는 대가로 쓴다.** 비교는 없어졌지만 연결마다의 처리가 생겨 인벤토리의 CPU는 줄지 않았다. 바뀌지 않은 프레임에도 연결마다 배열 키를 확인하기 때문이다. 이 테스트베드처럼 칸이 드물게 바뀌고 받는 연결이 적으면 CPU 이득이 없다는 것을 "배운 것과 한계"에 적을 수 있다.
- **다른 대가도 그대로 남는다.** 클라이언트의 칸 순서가 서버와 다르고(14절에서 화면으로 확인), 채집 같은 작은 바뀜에도 머리가 붙는다.

## 16. 확인하지 않은 것 (측정 뒤)

- 0번이 아닌 연결의 값.
- 적용 후 `ItemId`가 15번만 나온 까닭(12절).
- CPU 차이의 변동 폭(`-StatNamedEvents` 실행을 한 번씩만 쟀다).
- 채집 한 번의 FastArray 비트(최대 345와 평균 322만 보았고, 앞 칸 지우기와 채집을 따로 고르지 않았다).
