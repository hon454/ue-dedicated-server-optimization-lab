# 테스트베드 확장과 새 기준선: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값은 [관찰 자료](candidates.md)에 있다.

## 1. 실행 조건

- 새 기준선은 `act2-baseline1`이다. 같은 명령으로 세 번 연달아 실행했다(`r1`\~`r3`, 2026-10-04 00:47\~00:55, 본체 화면). 빌드는 `0f689c2`의 소스다.
- 조건: 클라이언트 8, 자원 노드 5,001, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 2막의 요소: `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`. 기준선 인자: `-AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100`.
- 세 실행 모두 종료 코드 0으로 끝났고 측정 시작 뒤의 선호도 재설정이 없었다. `saturated_ratio`는 0.000이다.
- 밀집과 분산의 비교는 세 기법을 적용한 구성(인자 없음 + 2막의 요소)을 번갈아 잰 `layout-dense1`, `2`, `4`와 `layout-apart1`, `7`, `8`이다(2026-10-03 23:50\~2026-10-04 00:21, 같은 빌드). 라벨마다 실행이 하나다.
- Insights 값은 측정 구간에서 읽었다. Timing 값은 서버 로그의 프레임 번호로 구간을 정해 창 없이 내보냈고, 북마크로 고른 창의 값과 같았다([관찰 자료](candidates.md) 1절, 9절).

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 216ms | 216.067ms | Timing Insights, 세 실행(215.669 / 216.067 / 221.482)의 중앙값 `r2`. 3절 |
| 틱 예산의 6.5배 | 216.067 ÷ 33.333 = 6.48 | 틱 예산은 1000 ÷ `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`) |
| 리플리케이션 시간 206ms, 서버 프레임 시간의 약 96% | 206.441ms. 206.441 ÷ 216.067 = 95.5%, `WorldTick` 대비 95.79%(`% Root`) | Timing Insights `GameNetDriver`, 세 실행의 중앙값 `r2`. 3절 |
| 연결당 송신 대역폭 36,000바이트/초 | 36,038바이트/초 | Network Insights `Connection 0`(`r2`). 6절 |
| 클라이언트에 존재하는 액터: 자원 노드 5,001개, NPC 350명, 건축물 500개, 플레이어 8명 | 같음 | 1번 클라이언트의 화면 글자(`r2`, t=60초) `nodes=5001 npcs=350 players=8 buildings=500` |
| 1막 마지막 글의 서버 프레임 시간 평균 13.2ms, 틱 예산의 40% | 13.16ms, 13.16 ÷ 33.3 = 0.40 | [1막의 누적 수치](../measurements.md) 1절(`update-frequency3`) |
| 플레이어가 서로 150m 넘게 떨어져 있었다 | 자리 간격 약 196m | [2막 설계](../../Docs/Planning/2026-10-03-act-2-design.md) 2절(2π × 500m ÷ 16) |
| 모이는 배치: 3m 간격, 한 변 70m의 정사각형 | `-LabPlayerSpacing=3`, `SpacedWaypointSide` 70m | 2막 설계 3.1(`LabPlayerController.h`) |
| 주변 NPC: 무리 둘레 40m 안에 50명, 맵 전체 300명 | `-LabNpcsNearPlayers=50`, 무리 반지름 40m | 2막 설계 3.5 |
| 건축물: 무리 둘레 80m 안에 500개, 1초마다 하나를 허물고 지음 | `-LabBuildings=500`, `-LabBuildInterval=1`, 무리 반지름 80m | 2막 설계 3.4 |
| 상태 값 8개, 액터마다 평균 5초에 하나 | 정수 넷과 실수 넷, `-LabStateInterval=5` | 2막 설계 3.2 |
| 인벤토리 200칸, 4초마다 맨 앞 칸을 지움 | `-LabInventoryItems=200`, `-LabInventoryChurn=4` | 2막 설계 3.3 |
| 가장 먼 두 플레이어도 120m 안 | 120.0m(84.85m 상자의 대각선) | 2막 설계 3.1 |
| Net Cull Distance 기본값 150m | `NetCullDistanceSquared` 225,000,000 | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| NPC는 초당 100번까지 고려한다 | `NetUpdateFrequency` 기본값 100 | `Actor.cpp:295-296`. 기준선 인자 `-NpcUpdateFrequency 100` |
| 한 프레임의 처리 횟수 46,872번, 액터 5,859개 × 연결 8개 | (5,001 + 350 + 500 + 8) × 8 | `r2`의 Count ÷ `WorldTick` 278: `LabResourceNode` 40,008, `LabNpc` 2,800, `LabBuilding` 4,000, `LabCharacter` 64. 4절 |
| 차트: 자원 노드 93.2, 드라이버 자체 시간 71.7, NPC 29.1, 건축물 9.77, 캐릭터 1.94ms | 93.156 / 71.695 / 29.067 / 9.769 / 1.943ms | `r2`의 프레임당 Incl(드라이버 자체 시간은 `GameNetDriver` Excl). 4절 |
| 자원 노드 45%, 드라이버 자체 시간 35% | 45.1%, 34.7% | `GameNetDriver` Incl 206.441ms 대비. 4절 |
| 1막의 기준선에서는 52%와 37% | 52.14%, 37.0% | [기준선의 관찰 자료](../01-baseline/candidates.md) 1절(`baseline3-r1`: `LabResourceNode` `% Parent`, 21.11초 ÷ 57.07초) |
| 받은 비트의 비율과 횟수(NPC 62.7%, 96,954번 등) | 6절의 표 | Network Insights `Connection 0`, 2,300패킷, 59.855초(`r2`) |
| 1막의 기준선에서도 NPC가 받은 비트의 76% | 76.0% | [Relevancy의 측정 기록](../02-relevancy/measurements.md) 5절(`baseline3-r1`) |
| 건축물 9.77ms, 미리 계산한 값 9.85ms | 9.769ms, 98.51ms × 500 ÷ 5,001 = 9.85ms | 계산은 2막 설계 5.1(`baseline3-r1`의 `LabResourceNode` 프레임당 98.51ms를 건축물 500개 몫으로 나눔. 한 번 2.46µs × 500 × 8과 같은 식) |
| 상태 값 6.37ms | 6.188 + 0.178 = 6.366ms | `LabNpc`와 `LabCharacter` 아래의 `LabStateComponent`(`r2`). 4절 |
| 인벤토리 0.626ms | 0.626ms | `LabCharacter` 아래의 `LabInventoryComponent`(`r2`) |
| 대역폭 0.03%, 2.1%, 12.8% | 6절 | |
| 세 기법을 적용한 구성에서 인벤토리는 받은 비트의 29.4% | 29.4% | `layout-dense1-r1` `Connection 0`: `LabInventoryComponent` 2,200,264 ÷ `Actor` 7,473,142비트([Worklog](../../Docs/Worklog/05-expanded-testbed.md) "태스크 20.4: `layout-dense1-r1`의 Networking Insights 값") |
| 분산 300m, 무리 여덟 개 | `-LabPlayerSpacing=300`, 서버 로그 `clusters=8` | 2막 설계 3.1, 3.4 |
| 서버 프레임 시간 평균 17.1ms와 27.6ms | 17.061ms, 27.603ms | Timing Insights, 중앙값 실행 `layout-dense4-r1`, `layout-apart8-r1`. 7절 |
| 연결당 송신 대역폭 16,600과 9,740바이트/초 | 16,575, 9,743 | CSV `out_bytes_per_sec_per_conn`의 중앙값(8개 연결의 평균). 7절 |
| 플레이어 8명과 1명 | `players=8`, `players=1` | 화면 글자(`layout-dense4-r1`, `layout-apart8-r1`, t=60초) |
| 월드의 건축물 500개와 4,000개 | 500 × 1, 500 × 8 | 무리 수 × 무리 하나의 수 |
| 62% 길다 | 27.603 ÷ 17.061 = 1.618 | 7절 |
| 드라이버 자체 시간 10.3ms에서 19.7ms | 10.295ms, 19.749ms | `GameNetDriver` Excl ÷ `WorldTick` Count. 7절 |
| 41% 작다 | 9,743 ÷ 16,575 = 0.588 | CSV |
| 상태 값 0.264ms | 0.187 + 0.077 | `calib2-a-r1`의 `LabStateComponent`(Worklog "태스크 20.4: `calib2-a-r1`의 Timing Insights 값") |
| 같은 소스를 두 시간 간격으로 잰 두 묶음이 5% 달랐다 | `toggle1` 13.581 → `abcheck-old2` 12.953, (13.581 − 12.953) ÷ 13.581 = 4.6% | CSV `work_avg_ms` 중앙값(Worklog "태스크 19.2") |

## 3. 세 실행의 Insights 값

| 지표 | `r1` | `r2` | `r3` | 중앙값 | 변동 폭 |
| --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 215.669 | 216.067 | 221.482 | 216.067 | 5.813 |
| 서버 프레임 시간 P99(ms) | 261.850 | 271.026 | 331.761 | 271.026 | 69.911 |
| 리플리케이션 시간(ms/프레임) | 206.238 | 206.441 | 211.538 | 206.441 | 5.300 |
| `GameNetDriver`의 `WorldTick` 대비 | 95.85% | 95.79% | 95.74% | 95.79% | |
| 연결당 송신 대역폭(바이트/초, `Connection 0`) | 읽지 않음 | 36,038 | 읽지 않음 | | |
| 연결당 열린 액터 채널 수(CSV) | 5,871 | 5,871 | 5,871 | 5,871 | 0 |

- 서버 프레임 시간은 측정 구간의 프레임마다 (`Frame` − 그 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate`)의 평균이다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)). 틱 예산을 계속 넘어 대기는 프레임당 0.015ms다.
- 구간과 CSV 대조는 [관찰 자료](candidates.md) 1절, 2절에 있다. CSV와의 차이는 0.5% 안이다.
- `r3`만 P99가 높다. 클래스 타이머의 한 번도 `r2`보다 2\~4% 길어 실행 전체가 조금 느렸다(관찰 자료 7절).

## 4. 리플리케이션 시간의 내역 (`r2`)

| 타이머(프레임당) | 프레임당 처리 횟수 | Incl | 한 번 | `GameNetDriver` 대비 |
| --- | --- | --- | --- | --- |
| `LabResourceNode` | 40,008 | 93.156ms | 2.33µs | 45.1% |
| `GameNetDriver` Exclusive | | 71.695ms | | 34.7% |
| `LabNpc`(아래 `LabStateComponent` 6.188ms 포함) | 2,800 | 29.067ms | 10.38µs | 14.1% |
| `LabBuilding` | 4,000 | 9.769ms | 2.44µs | 4.7% |
| `LabCharacter`(아래 `LabInventoryComponent` 0.626ms, `LabStateComponent` 0.178ms 포함) | 64 | 1.943ms | 30.36µs | 0.9% |
| 나머지 여섯 | | 0.811ms | | 0.4% |
| 합계(리플리케이션 시간) | | 206.441ms | | 100% |

- 각 값은 타이머 Incl(Exclusive는 Excl) ÷ `WorldTick` Count 278이다. 클래스 타이머는 `UActorChannel::ReplicateActor`가 C++ 부모 클래스 이름으로 남긴다(`Engine/Source/Runtime/Engine/Private/DataChannel.cpp:3622-3625`).
- 처리 횟수가 식과 정확히 같다: `LabResourceNode` 11,122,224 = 5,001 × 8 × 278, `LabNpc` 778,400 = 350 × 8 × 278, `LabBuilding` 1,112,000 = 500 × 8 × 278.
- 세 실행의 값은 관찰 자료 3절에 있다. 1막의 기준선과 비교한 표는 관찰 자료 4절에 있다. 시각이 다른 묶음이라 절댓값은 비교하지 않고 비율만 본문에 썼다.

## 5. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `over_budget_frames` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-baseline1-r1` | 279 | 215.238 | 261.636 | 279 | 206.558 | 37,065 | 5,871 | 0.000 |
| `act2-baseline1-r2` | 278 | 215.801 | 270.781 | 278 | 206.944 | 36,994 | 5,871 | 0.000 |
| `act2-baseline1-r3` | 272 | 220.521 | 331.528 | 272 | 211.365 | 36,324 | 5,871 | 0.000 |
| 중앙값 | 278 | 215.801 | 270.781 | 278 | 206.944 | 36,994 | 5,871 | 0.000 |
| 변동 폭 | 7 | 5.283 | 69.892 | 7 | 4.807 | 741 | 0 | 0.000 |

- 기준선의 네 조건: 초기 전송 완료(채널이 측정 시작 16\~20초 전에 5,871에 도달), 지속적인 예산 초과(`over_budget_frames` = `frames`), 가장 큰 비용이 네트워크(사용자 판단, 2026-10-04), 송신 한도에 포화되지 않음(30Hz 환산 239,529바이트/초로 한도의 68%). 근거는 [STATUS.md](../../Docs/STATUS.md) "기준선 조건"에 있다.
- `Connection 0`의 36,038바이트/초는 CSV의 8개 연결 평균 36,994보다 2.6% 작다. 기준선은 모든 연결이 같은 액터를 받는다.

## 6. 연결 하나가 받은 데이터 (`r2`, `Connection 0`, `Outgoing`)

| Net Stats 줄 | 본문의 이름 | Count | Incl(비트) | `Actor` 대비 |
| --- | --- | --- | --- | --- |
| `Actor` | | 100,132 | 17,066,950 | 100% |
| `LabNpc` | NPC | 96,954 | 10,708,961 | 62.7% |
| `LabInventoryComponent` | 인벤토리 | 127 | 2,182,034 | 12.8% |
| `LabStateComponent` | 상태 값 | 4,206 | 362,812 | 2.1% |
| `BP_LabCharacter_C` | 플레이어 캐릭터 | 2,216 | 320,433 | 1.9% |
| `LabBuilding` | 건축물 | 120 | 4,920 | 0.03% |
| `LabResourceNode` | 자원 노드 | 9 | 576 | 0.003% |
| `PacketHeaderAndInfo` | | 2,300 | 189,347 | (`Actor` 밖) |

- 측정 구간의 첫 프레임 1,164와 마지막 프레임 1,441이 든 패킷 막대를 골랐다(2,300패킷, 59.855초). 화면 한 픽셀에 패킷 네 개가 들어가 경계가 한 프레임까지 어긋날 수 있다.
- 연결당 송신 대역폭은 (17,066,950 + 189,347) ÷ 8 ÷ 59.855 = 36,038바이트/초다.
- `LabNpc`는 초당 1,619.8번으로, NPC 350명이 매 프레임 보내진 수(350 × 278 ÷ 60 = 1,622)와 같다.
- 본문의 "받은 횟수"는 Net Stats의 Count다.

## 7. 밀집과 분산 (세 기법을 적용한 구성)

| 지표 | 밀집 `layout-dense4-r1` | 분산 `layout-apart8-r1` | 차이 |
| --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 17.061 | 27.603 | +10.542 |
| 서버 프레임 시간 P99(ms) | 24.819 | 41.614 | |
| `WorldTick`(ms/프레임) | 16.590 | 27.162 | +10.572 |
| 리플리케이션 시간(ms/프레임) | 12.141 | 21.766 | +9.625 |
| `GameNetDriver` Exclusive(ms/프레임) | 10.295 | 19.749 | +9.454 |
| 클래스 타이머의 합(ms/프레임) | 1.846 | 2.017 | +0.171 |
| `TickCompletionEvents`(액터 틱, ms/프레임) | 2.379 | 3.193 | +0.814 |
| `LabNpc` 처리 횟수와 Incl(프레임당) | 115.1번, 0.974ms | 113.6번, 1.515ms | |
| `LabCharacter` 처리 횟수와 Incl(프레임당) | 49.6번, 0.626ms | 6.2번, 0.225ms | |

| CSV | 밀집 중앙값(변동 폭) | 분산 중앙값(변동 폭) |
| --- | --- | --- |
| `work_avg_ms` | 16.779(0.065) | 27.347(0.281) |
| `out_bytes_per_sec_per_conn` | 16,575(52) | 9,743(55) |
| `open_actor_channels_per_conn` | 77 | 69 |

- 두 실행은 `work_avg_ms`의 중앙값 실행이다. 측정 구간은 서버 로그의 프레임 번호로 구했다(밀집 89.6516\~149.6897초, 1,784프레임. 분산 90.0668\~150.0985초, 1,773프레임). 서버 프레임 시간 평균과 CSV `work_avg_ms`의 차이는 +1.7%, +0.9%다.
- 늘어난 `WorldTick` 10.572ms 가운데 9.454ms(89%)가 `GameNetDriver` Exclusive다. 클래스 타이머의 합은 거의 같다.
- CSV 표와 경위는 [Worklog](../../Docs/Worklog/05-expanded-testbed.md) "태스크 20.4a"에 있다. 실행 12회 가운데 6회가 선호도 재설정으로 실패해 수치를 쓰지 않았다.

## 8. 규모를 정한 과정

- 클라이언트 8, 자원 노드 5,000, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 송신 한도 350,000은 1막 그대로다.
- 요소의 값은 1막의 측정에서 계산한 출발값이다([2막 설계](../../Docs/Planning/2026-10-03-act-2-design.md) 5.1). 기준은 세 기법을 적용한 구성에서 뒤의 기법마다 겨냥할 비용이 `work_avg_ms`의 변동 폭(0.5\~0.7ms)보다 큰 것이다. 목표는 2ms다.
- 보정 실행으로 네 구성을 한 번씩 쟀다: 기준선 184.837(`calib2-d3-r1`), Relevancy만 35.717(`calib2-c-r1`), Dormancy까지 20.011(`calib2-b-r1`), 세 기법 모두 14.682(`calib2-a-r1`)다(CSV `work_avg_ms`). 건축물 배치와 0번 자리 옆 노드를 고치기 전의 실행이다.
- 상태 값의 비용은 프레임당 0.264ms로 변동 폭보다 작다. 프로퍼티를 64개로 늘려도 0.327ms였다(`calib2-e2-r1`). 그래서 값을 늘리지 않고 Push Model만 기준의 예외로 두었다.
- 사용자가 출발값을 그대로 확정했다(2026-10-04). 보정의 실행 표와 경위는 [Worklog](../../Docs/Worklog/05-expanded-testbed.md)의 "태스크 20.2"부터 "태스크 20: 보정의 경위"까지에 있다.
- `calib2-d3-r1`(184.837)과 새 기준선(215.801)은 같은 구성이다. 약 1.5시간 떨어졌고 건축물 배치와 노드 하나를 고친 빌드라 비교하지 않는다.

## 9. Insights 화면 (`r2`)

![Timers와 WorldTick Callees](images/timing.png)

① `GameNetDriver`(`% Root` 95.79%), ② `LabResourceNode`, `LabNpc`, `LabBuilding`(`% Parent` 45.13%, 14.08%, 4.73%), ③ 두 북마크(1분 32.712558초, 2분 32.800668초).

![Connection 0 Outgoing의 Net Stats](images/network.png)

① `Actor`부터 `LabInventoryComponent`까지(`LabNpc` 10,708,961비트), ② `LabResourceNode`(9번, 576비트), ③ 고른 측정 구간(2,300패킷, 59.855초).

상자를 그리기 전의 원본은 `images/insights-r2-timers.png`, `images/insights-r2-net-stats.png`다.

## 10. 엔진 소스 위치

| 사실 | 위치 |
| --- | --- |
| 틱 예산 33.3ms(`NetServerMaxTickRate` 30) | `Engine/Config/BaseEngine.ini:1867` |
| Net Cull Distance 기본값 150m | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 100 | `Actor.cpp:295-296` |
| 프로퍼티 비교는 객체마다 프레임에 한 번이고, 바뀐 것만 보낸다 | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1275-1331`([engine-notes.md](../../Docs/Reference/engine-notes.md) 자절) |
| 클래스 이름 타이머 | `DataChannel.cpp:3622-3625`의 `SCOPE_CYCLE_UOBJECT` |

## 11. 시각 자료의 사정

- `topdown.png`, `tpp.png`는 `act2-baseline1-r2`의 자동 스크린샷 03번(t=60초)이다.
- `dense-topdown.png`, `apart-topdown.png`는 세 기법을 적용한 구성의 `layout-dense4-r1`, `layout-apart8-r1` 03번이다. 분산 배치의 화면 글자 `buildings=934`는 시작 신호 전에 맵 가운데에서 받은 다른 무리의 건축물이 Dormant 상태로 남은 것이다(2막 설계 3.4).
- 영상은 찍지 않았다.

## 12. 확인하지 않은 것

- 분산 배치에서 늘어난 `GameNetDriver` Exclusive의 내역.
- `Connection 1`\~`7`과 `r1`, `r3`의 연결당 송신 대역폭.
- `r3`만 느렸던 이유, 새 기준선이 보정의 `calib2-d3-r1`보다 높은 이유.
