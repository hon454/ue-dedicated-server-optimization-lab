# 인벤토리를 소유자에게만 보내기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 실행별 CSV, Insights 값, 서버 로그는 [관찰 자료](candidates.md) 8\~12절에 있다.

## 1. 실행 조건

- 구성: [포스팅 8](../08-node-update-frequency/measurements.md)의 최종 구성. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`과 `-NodeUpdateFrequency 2`. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초, 트레이스 켬.
- 두 묶음을 세 번씩 연달아 쟀다. 2026-10-06 12:05\~12:21, 본체 화면, 실행 중 PC 조작 없음. 6회 모두 종료 코드 0이었다.

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-invown-base1` | 적용 전 | 없음 | `r2` |
| `act2-invown1` | 적용 후 | `-InventoryOwnerOnly` | `r2` |

- 빌드는 커밋 `92dc93e`의 소스다. 인자 `-LabInventoryOwnerOnly`는 커밋 `730f247`이 더했고, `92dc93e`는 클라이언트의 인벤토리 패널(`-LabInventoryPanel`)을 더했다. 패널은 측정 실행에서 그리지 않는다.
- 본문의 Networking Insights 값은 `Game Instance 0 [Server]`, `Connection 0`(0번 자리, 채집 담당), `Outgoing`에서 측정 구간의 패킷을 골라 읽었다. 고른 범위는 적용 전 2,130패킷, 60.015초, 적용 후 1,824패킷, 59.886초다(관찰 자료 10절).
- 바이트/초는 Net Stats의 Incl(비트) ÷ 8 ÷ 고른 범위의 시간이다. 연결당 송신량은 `Actor`와 `PacketHeaderAndInfo`의 Incl을 더한 값으로 계산했다.
- Timing 값은 `Scripts/export-insights.ps1`로 내보냈다. 프레임 수는 `WorldTick` Count가 CSV `frames`와 같았다(1,796, 1,797).

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 연결 하나가 받는 데이터의 28.6%는 인벤토리 | 2,182,034 ÷ (7,424,325 + 192,286) = 28.65%(`act2-invown-base1-r2`) | 관찰 자료 10절 |
| 연결당 송신 대역폭 15,900 → 11,800바이트/초, -25.4%, "25% 줄었다" | (7,424,325 + 192,286) ÷ 8 ÷ 60.015 = 15,864.0, (5,501,213 + 167,478) ÷ 8 ÷ 59.886 = 11,832.3, 11,832.3 ÷ 15,864.0 − 1 = -25.41%. CSV `out_bytes_per_sec_per_conn` 중앙값은 16,635 → 12,421(-25.3%) | 관찰 자료 9절, 10절 |
| 인벤토리 4,540 → 571바이트/초, -87.4% | `LabInventoryComponent` 2,182,034 ÷ 8 ÷ 60.015 = 4,544.8, 273,676 ÷ 8 ÷ 59.886 = 571.2, 571.2 ÷ 4,544.8 − 1 = -87.43% | 관찰 자료 10절 |
| 초당 패킷 수 35.5 → 30.5개, -14.2% | 2,130 ÷ 60.015 = 35.49, 1,824 ÷ 59.886 = 30.46, 30.46 ÷ 35.49 − 1 = -14.18% | 관찰 자료 10절 |
| 서버 프레임 시간 평균 8.84 → 8.77ms, 구별되지 않음 | Timing Insights 8.835 → 8.774(`r2`끼리). CSV `work_avg_ms` 중앙값 8.560 → 8.493(-0.067)이 두 묶음의 변동 폭 0.094, 0.153보다 작다 | 관찰 자료 9절, 11절 |
| 플레이어 캐릭터 8개, 인벤토리 200칸, 칸 하나는 아이템 번호와 수량 | `-Clients 8 -InventoryItems 200`, `FLabItem`의 `ItemId`, `Count`(`Source/DSOptLab/LabInventoryComponent.h`) | 실행 조건, 프로젝트 소스 |
| 0.5초마다 하나를 돌아가며 바꾼다, 하나는 4초마다 | 타이머 간격 `InventoryChurnSeconds ÷ Inventories.Num()` = 4 ÷ 8 = 0.5초(`Source/DSOptLab/LabGameMode.cpp`의 `ChurnTimer`, `ChurnOneInventory`) | 프로젝트 소스, 계산 |
| 맨 앞 칸을 지우고 맨 뒤에 새 칸을 더한다 | `Items.RemoveAt(0); Items.Add(MakeItem());`(`LabInventoryComponent.cpp`의 `Churn`) | 프로젝트 소스 |
| 여덟 명은 서로 150m 안 | 밀집 배치의 모든 경로가 한 변 84.85m 상자 안이고 대각선이 120.0m다. Net Cull Distance는 150m(`Engine/Source/Runtime/Engine/Private/Actor.cpp:312`) | [2막 설계](../../Docs/Planning/2026-10-03-act-2-design.md) 3.1절 |
| 조건 없이 리플리케이트했다 | `DOREPLIFETIME(ULabInventoryComponent, Items)`(커밋 `730f247` 전) | 프로젝트 소스 |
| 문제의 표: NPC 5,310(33.5%), 인벤토리 4,540(28.6%), 플레이어 캐릭터 3,080(19.4%), 그 밖 2,930(18.5%) | `LabNpc` 2,550,189 ÷ 8 ÷ 60.015 = 5,311.6, `BP_LabCharacter_C` 1,478,785 ÷ 8 ÷ 60.015 = 3,080.0, 그 밖 = 15,864.0 − 4,544.8 − 5,311.6 − 3,080.0 = 2,927.6. 비율은 15,864.0에 대한 값 | 관찰 자료 10절 |
| 같은 자리끼리 비교해 당겨진 칸을 모두 바뀐 칸으로 본다 | `CompareProperties_Array_r`가 원소를 인덱스마다 비교한다(`Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1692-1775`) | 엔진 소스, [engine-notes.md](../../Docs/Reference/engine-notes.md) 8절 |
| 바뀔 때마다 200칸을 다시 보낸다 | 적용 전 `ItemId` Count 24,000 = 120번 × 200칸. 120번은 60초 ÷ 0.5초. 패킷 16,384 하나의 내용에 `ItemId` 200개, `Count` 198개 | 관찰 자료 10절, 10.1절 |
| 한 번에 약 18,200비트, 적용 후에도 같다 | `LabInventoryComponent` I.Max 18,190비트(적용 전, 적용 후) | 관찰 자료 10절 캡처 |
| 클라이언트는 자기 인벤토리만 쓴다 | 클라이언트에서 인벤토리를 읽는 곳은 `LabHUD.cpp`의 화면 글자(자기 인벤토리의 칸 수와 첫 칸)와 인벤토리 패널뿐이다 | 프로젝트 소스 |
| `COND_OwnerOnly`는 소유자 연결에만 보낸다 | `COND_OwnerOnly = 2`, "This property will only send to the actor's owner"(`Engine/Source/Runtime/CoreUObject/Public/UObject/CoreNetTypes.h:20`). 조건 표 `ConditionMap[COND_OwnerOnly] = bIsOwner`(`Engine/Source/Runtime/Engine/Public/Net/RepLayout.h:135-149`) | 엔진 소스 |
| 캐릭터의 소유자 연결은 조종하는 플레이어의 연결 | `APawn::GetNetConnection`이 컨트롤러의 연결을 돌려준다(`Pawn.cpp:753-760`). 연결마다 `RepFlags.bNetOwner = (OwningConnection == Connection ...)`(`DataChannel.cpp:3809-3812`) | 엔진 소스 |
| 비교는 객체마다 프레임에 한 번, 결과를 모든 연결이 함께 쓴다 | `FRepLayout::UpdateChangelistMgr`가 `LastReplicationFrame`이 이번 프레임이면 비교를 건너뛴다(`RepLayout.cpp:1275-1331`) | 엔진 소스, engine-notes.md 8절 |
| 조건은 연결마다 확인해 바뀐 목록에서 뺀다 | `FRepLayout::ReplicateProperties`에서 `FilterChangeListToActive(RepState->LifetimeChangelist, RepState->InactiveParents, ...)`(`RepLayout.cpp:2047`, 함수 `2660-2680`) | 엔진 소스 |
| 예상: 약 570바이트/초, 8분의 1 | 4,548 × 15 ÷ 120 = 568(포스팅 8 최종 구성의 `act2-nodeuf1-r3`에서 계산) | 관찰 자료 5절 |
| 예상: 연결당 송신 대역폭 약 25%, 약 11,900 | (7,626,709 − 2,182,034 × 105 ÷ 120) ÷ 8 ÷ 59.973 = 11,917, -25.0% | 관찰 자료 5절 |
| 인벤토리를 리플리케이트하는 시간이 프레임의 2%가 되지 않는다 | `LabInventoryComponent` 프레임당 0.145ms ÷ 서버 프레임 시간 9.250ms = 1.6%(`act2-nodeuf-split1-r3`, 타이머를 더 켠 실행이라 비율만 본다) | 관찰 자료 2.2절 |
| 결과 표: 받은 횟수 127번 → 22번, 예상 22번 | `LabInventoryComponent` Count 127, 22. 예상은 자기 인벤토리 15번(60초 ÷ 4초) + 채집 7번(적용 전 127 − 120) | 관찰 자료 10절 |
| 자기 인벤토리 15번, 채집 7번 | 적용 후 `ItemId` Count 3,000 = 15번 × 200칸, 22 − 15 = 7 | 관찰 자료 10절 |
| 차트의 값 | 인벤토리 4,544.8, 571.2, NPC 5,311.6, 5,317.0(2,547,334 ÷ 8 ÷ 59.886), 플레이어 캐릭터 3,080.0, 3,083.3(1,477,192 ÷ 8 ÷ 59.886) | 관찰 자료 10절 |
| 줄어든 송신량의 99%가 인벤토리 | (4,544.8 − 571.2) ÷ (15,864.0 − 11,832.3) = 3,973.6 ÷ 4,031.7 = 98.6% | 계산 |
| 패킷이 사라진 것으로 추정 | 인벤토리 한 번(18,190비트)이 `Actor` 줄 최대 7,630비트의 패킷 여러 개에 `PartialInitial` 묶음으로 나뉘어 실린다(패킷 16,384). 패킷을 하나씩 세지는 않았다 | 관찰 자료 10.1절 |
| 리플리케이션 시간 3.91 → 3.89ms | `GameNetDriver` 프레임당 Incl 3.908 → 3.885(`r2`끼리) | 관찰 자료 11절 |
| 플레이어 캐릭터를 직렬화하는 시간 0.537 → 0.480ms | `LabCharacter` 프레임당 Incl 0.537 → 0.480(`r2`끼리, 인벤토리 컴포넌트를 포함한 캐릭터 액터의 리플리케이션) | 관찰 자료 11절 |
| 적용 후 영상에서도 자기 격자는 통째로 흰색 | `images/inventory-panel-after.gif`(`visual15`) | 4절 |
| "0 of 7 received", 다른 플레이어 칸 수 1,400 → 0 | 패널 글자(`visual15`), 화면 글자 `other items=1400`(`act2-invown-base1-r2-tpp-01`, t=30초), `other items=0`(`act2-invown1-r2-tpp-01`) | 자동 스크린샷, 4절 |
| 열린 액터 채널 수 77 | CSV `open_actor_channels_per_conn` 여섯 실행 모두 77 | 관찰 자료 9절 |
| 흩어진 플레이어의 캐릭터는 150m 밖 | 분산 배치는 자리 간격 300m(`-PlayerSpacing 300`, 2막 설계 3.1절)로 Net Cull Distance 150m보다 멀다 | 2막 설계, 엔진 소스 |
| 다음 글: 4초마다 200칸 | 위 "바뀔 때마다 200칸", "하나는 4초마다" 줄 | |

## 3. 서버가 남긴 CSV

실행별 값은 [관찰 자료](candidates.md) 9절에 있다.

| 묶음 | `work_avg_ms` 중앙값 | 변동 폭 | `netflush_avg_ms` 중앙값 | `out_bytes_per_sec_per_conn` 중앙값 | 변동 폭 | `open_actor_channels_per_conn` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-invown-base1` | 8.560 | 0.094 | 4.200 | 16,635 | 87 | 77 |
| `act2-invown1` | 8.493 | 0.153 | 4.133 | 12,421 | 33 | 77 |

- `out_bytes_per_sec_per_conn`은 4,214(25.3%) 줄었다. 변동 폭(87, 33)보다 훨씬 크다.
- `work_avg_ms`는 0.067 줄었다. 변동 폭 가운데 큰 쪽(0.153)보다 작아 구별하지 못했다(measurement.md "차이의 판단").
- `act2-invown-base1`은 포스팅 8의 `act2-nodeuf1`(8.615)과 구성이 같다. 다른 시각의 묶음이라 비교하지 않았다.

## 4. 시각 자료

- 요약의 두 GIF는 인벤토리 패널을 2번 클라이언트(3인칭 이동)에 띄운 시각 자료 전용 실행 `visual14`(적용 전 구성)과 `visual15`(적용 후 구성)에서 찍었다. 측정 구간에 `Scripts/capture-video.ps1 -Region "1920,0,960,540" -RaiseSlots "2" -NoMouse -AllowMeasuring -Seconds 10`으로 10초씩 찍었다(폭 960px, 8fps, 48색). 두 실행의 수치는 쓰지 않는다.
- 패널은 `LabHUD.cpp`의 `DrawInventoryPanel`이 그린다. 칸 색은 아이템 번호(`ItemId × 47 mod 256`을 색상으로), 클라이언트가 지난 틱에 본 값과 같은 자리의 값이 다르면 0.4초 흰색, 받은 칸이 없으면 회색이다. 다른 플레이어의 격자는 PlayerId 순서다.
- 원리의 흐름도는 Mermaid로 그렸다. "비교"는 `UpdateChangelistMgr`, "조건 확인"은 `FilterChangeListToActive`에 대응한다.
- 결과의 차트 값은 2절 "차트의 값" 줄이다.

### 패킷 그래프 (Networking Insights, `Connection 0`, `Outgoing`)

창을 3000×2080으로 맞추고, 측정 구간을 고른 뒤 그래프를 확대해 약 120패킷(약 3.5초)을 보였다. 큰 패킷 하나를 골라 아래에 그 내용을 띄웠다. 2026-10-06에 `Scripts/capture-insights.ps1`로 찍었다.

![적용 전 act2-invown-base1-r2. 약 0.5초마다 큰 패킷이 나오고, 고른 패킷 16,384에 다른 플레이어의 LabInventoryComponent 18,190비트가 실려 있다](images/act2-invown-base1-r2-packets-zoom.png)

적용 전(`act2-invown-base1-r2`, 패킷 2,736\~2,858): 약 7,600비트짜리 패킷이 약 18패킷(약 0.5초)마다 몰려 나온다. 고른 패킷 16,384의 `Actor ChannelId:18 | PartialInitial` 안에 다른 플레이어 캐릭터(NetId 1510)의 `LabInventoryComponent` 18,190비트가 있고, `ItemId` 200개와 `Count` 198개가 보인다.

![적용 후 act2-invown1-r2. 같은 폭에서 큰 패킷이 한 번 나오고, 고른 패킷 17,626은 자기 캐릭터의 채널이다](images/act2-invown1-r2-packets-zoom.png)

적용 후(`act2-invown1-r2`, 패킷 2,032\~2,156): 같은 폭에서 큰 패킷이 한 번 나온다. 고른 패킷 17,626은 자기 캐릭터의 채널(`ChannelId:4`)이고 같은 18,190비트다.

Net Stats 창 전체(측정 구간): [적용 전](images/act2-invown-base1-r2-netstats-connection0.png), [적용 후](images/act2-invown1-r2-netstats-connection0.png).

## 5. 확인하지 않은 것

- 초당 패킷 수가 준 원인. 인벤토리의 큰 묶음이 나뉘어 실리던 패킷이 사라졌다고 추정했지만 패킷을 하나씩 세지 않았다.
- 0번이 아닌 연결의 Networking Insights 값. 모든 연결이 같은 일을 받으므로 같다고 보았다(CSV는 여덟 연결의 평균이다).
- `LabCharacter` 시간 차이(0.057ms)의 변동 폭. 중앙값 실행 하나끼리의 값이다.
- 분산 배치(`-PlayerSpacing 300`)에서의 효과.
- 클라이언트가 조건 없이 등록한 채 받는 것은 실행으로만 확인했다(화면 글자와 패널). 받는 쪽이 조건을 쓰지 않는다는 것을 엔진 소스에서 끝까지 따라가지는 않았다.
