# Always Relevant 기준선: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값과 후보 기법의 엔진 소스 위치는 [후보 기법 자료](candidates.md)에 있다.

## 1. 실행 조건

- 라벨은 `baseline3`이고 같은 명령으로 세 번 실행했다(`r1`\~`r3`). 중앙값 실행은 `r1`이다.
- 조건: 클라이언트 8, 자원 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 세 실행 모두 종료 코드 0으로 끝났다.
- Insights 값은 두 북마크(`Lab_MeasureStart`, `Lab_MeasureEnd`) 사이의 측정 구간에서 읽었다(`r1` 60.128초).

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 198ms | 198.43ms | Timing Insights, 세 실행의 중앙값(`r1`). (60.128초 − 틱 속도 제한 대기 0.004초) ÷ `Frame` Count 303 |
| 틱 예산 33.3ms, 약 6배 | 1000 ÷ 30 = 33.3ms, 198.43 ÷ 33.3 = 5.96 | `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`) |
| 리플리케이션이 95% | 95.17% | `r1`의 `GameNetDriver` `% Root`(`WorldTick` 대비). 세 실행에서 95.00\~95.80% |
| 리플리케이션 시간 189ms | 188.97ms | `GameNetDriver` Incl 57.07초 ÷ `WorldTick` Count 302(`r1`), 세 실행의 중앙값 |
| 연결당 송신 대역폭 28,000바이트/초 | 28,048바이트/초 | Network Insights `Connection 0` `Outgoing`(`r1`). (`Actor` Incl 13,269,082 + `PacketHeaderAndInfo` Incl 149,181)비트 ÷ 8 ÷ 59.800초 |
| 클라이언트에 존재하는 자원 노드 5,001개, NPC 300명 | 같음 | `r1` 자동 스크린샷의 화면 글자(`nodes=5001 npcs=300`) |
| 엔진은 150m보다 먼 액터를 보내지 않는다 | `NetCullDistanceSquared` 225,000,000 | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| 엔진 기본 간격 0.01초 | 1 ÷ `NetUpdateFrequency` 100 | `Actor.cpp:295-296`. 다음 고려 시각은 지금 + 0\~1/30초 + 0.01초(`NetDriver.cpp:5420-5425`, `6341-6348`)라 최대 0.043초다 |
| 프레임 간격 약 0.2초 | 0.179\~0.205초 | 세 실행의 서버 프레임 시간 평균 |
| 프레임당 42,408번 | (5,001 + 300) × 8 | `r1`의 타이머 Count. `LabResourceNode` 12,082,416 = 5,001 × 8 × `WorldTick` 302, `LabNpc` 724,800 = 300 × 8 × 302 |
| 자원 노드 98.5ms(52%), NPC 19.5ms(10%), 드라이버 자체 69.9ms(37%) | 52.14%, 10.31%, 37.0% | `r1`의 타이머 Incl(자체 시간은 Excl) ÷ `WorldTick` Count. 자원 노드 29.75초, NPC 5.88초, `GameNetDriver` Exclusive 21.11초 |
| 송신 비트의 76%가 NPC | 76.0% | `r1` `Connection 0`: `LabNpc` 10,090,719 ÷ `Actor` 13,269,082비트. 그 가운데 `ReplicatedMovement`가 8,379,693비트 |
| 자원 노드의 송신 비트 0.004% | 576 ÷ 13,269,082 = 0.0043% | `r1` `Connection 0`: `LabResourceNode` 9번, 576비트 |
| 클라이언트 하나에 약 151만 번 처리, 9번 송신 | 12,082,416 ÷ 8 = 1,510,302 | 위의 타이머 Count와 Network Insights |
| 처리 한 번 2.5마이크로초 | 2.46μs | 29.75초 ÷ 12,082,416 |
| 프레임마다 40,008번 | 5,001 × 8 | 위의 타이머 Count |
| Net Update Frequency 10의 고려 간격 0.1\~0.133초 | 0.1초 + 0\~1/30초 | `NetDriver.cpp:5420-5425`, `6341-6348` |
| 세 실행의 서버 프레임 시간 평균 179\~205ms, 25ms | 179.45 / 198.43 / 204.81, 변동 폭 25.36 | 3절 |

## 3. 세 실행의 Insights 값

| 지표 | `r1` | `r2` | `r3` | 중앙값 | 변동 폭 |
| --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 198.43 | 204.81 | 179.45 | 198.43 | 25.36 |
| 서버 프레임 시간 P99(ms) | 268.69 | 239.03 | 262.96 | 262.96 | 29.66 |
| 리플리케이션 시간(ms/프레임) | 188.97 | 194.69 | 171.89 | 188.97 | 22.80 |
| 연결당 송신 대역폭(바이트/초, `Connection 0`) | 28,048 | 읽지 않음 | 31,012 | | |
| 연결당 열린 액터 채널 수(CSV) | 5,314 | 5,314 | 5,314 | 5,314 | 0 |

- 서버 프레임 시간은 프레임 시간에서 틱 속도 제한 대기(`FEngineLoop_UpdateTimeAndHandleMaxTickRate`)를 뺀 시간이다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)). 기준선은 틱 예산을 늘 넘어서 이 대기가 프레임당 0.01\~0.02ms뿐이다.
- P99는 측정 구간에 걸친 GameThread `Frame` 이벤트(`r1` 303개)마다 대기를 뺀 길이를 `TimingInsights.ExportTimingEvents`로 내보내, 정렬한 뒤 ceil(N × 0.99)번째 값을 읽은 것이다. 99백분위 경계값이며 느린 1%의 평균이 아니다.
- 변동 폭은 중앙값의 12.8%다(`work_avg_ms` 기준). 세 실행에서 하는 일의 양(타이머 호출 횟수)과 구성 비율(`LabResourceNode` 52.0\~52.6%, `GameNetDriver` Exclusive 37.0\~37.1%)은 같고, 느린 실행은 모든 하위 타이머가 1.12\~1.18배 느렸다([후보 기법 자료](candidates.md) 4절). 같은 일을 CPU가 더 느리게 처리한 것으로 보이며 원인은 확인하지 않았다.
- 같은 조건의 다른 성공 실행(`calib-f-r1` 168.309, `verify-baseline-r1` 161.649, CSV `work_avg_ms`)은 `baseline3`보다 낮았다. 원인은 모른다([STATUS.md](../../Docs/STATUS.md) "측정 결과").

## 4. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `baseline3-r1` | 302 | 198.773 | 268.485 | 189.375 | 28,896 | 5,314 | 0.000 |
| `baseline3-r2` | 292 | 205.150 | 238.737 | 195.108 | 27,997 | 5,314 | 0.000 |
| `baseline3-r3` | 334 | 179.673 | 262.736 | 172.293 | 31,990 | 5,314 | 0.000 |
| 중앙값 | 302 | 198.773 | 262.736 | 189.375 | 28,896 | 5,314 | 0.000 |
| 변동 폭 | 42 | 25.477 | 29.748 | 22.815 | 3,993 | 0 | 0.000 |

- Insights 값과의 차이: 서버 프레임 시간 평균과 `work_avg_ms`가 -0.12\~-0.17%, P99와 `work_p99_ms`가 +0.08\~+0.12%, 리플리케이션 시간과 `netflush_avg_ms`가 -0.21\~-0.24%.
- 연결당 송신 대역폭은 CSV가 Insights보다 3.0%(`r1`), 3.2%(`r3`) 크다. 이유는 확인하지 않았다.
- 서버는 30Hz(60초에 1,800프레임)의 약 6분의 1만 돈다. 30Hz로 환산한 송신량은 세 실행이 약 172,000바이트/초로 같다(`r1`: 28,896 × 30 ÷ (302 ÷ 60) = 172,230).

## 5. 기준선 조건

| 조건 | 결과 | 근거(`r1` / `r2` / `r3`) |
| --- | --- | --- |
| 초기 전송 완료 | 예 | `open_actor_channels_per_conn` 5,314 / 5,314 / 5,314. 같은 규모의 `calib-f-r1`에서 측정 시작 22초 전에 이 값에 도달하고 더 늘지 않았다 |
| 지속적인 예산 초과 | 예 | `over_budget_frames`가 `frames`와 같다(302 / 292 / 334) |
| 송신 한도에 포화되지 않음 | 예 | `saturated_ratio` 0.000 / 0.000 / 0.000 |
| 가장 큰 비용이 네트워크 | 예 | `GameNetDriver`가 `WorldTick`의 95.17% / 95.00% / 95.80% |

## 6. Insights 화면

| Timing Insights | Network Insights |
| --- | --- |
| ![r1 측정 구간의 Timers와 WorldTick Callees](images/timing.png) | ![r1 Connection 0 Outgoing 측정 구간의 Net Stats](images/network.png) |

- Timing Insights: ① `GameNetDriver`(`% Root` 95.17%), ② `LabResourceNode`와 `LabNpc`(`% Parent` 52.14%, 10.31%), ③ 두 북마크.
- Network Insights: ① `Actor`와 `LabNpc`의 비트, ② `LabResourceNode`의 비트(9번, 576), ③ 고른 측정 구간(1,801패킷, 59.800초).

![r1 패킷 3,710의 내용](images/packet.png)

측정 구간 가운데의 패킷 하나(Sequence 3,710, 1,018바이트)다. 액터 번치 55개가 모두 `LabNpc`이고, 번치마다 `ReplicatedMovement`가 평균 92비트다.

## 7. 본문 흐름도의 엔진 소스 위치

본문의 흐름도는 레거시 리플리케이션의 `UNetDriver::ServerReplicateActors`를 줄인 그림이다. 서버 로그의 `using replication model Generic`이 레거시를 뜻한다([engine-notes.md](../../Docs/Reference/engine-notes.md) "실제로 쓰는 리플리케이션 시스템").

| 흐름도의 단계 | 엔진 소스 |
| --- | --- |
| 활성 목록에서 Consider List 만들기(프레임당 한 번) | `ServerReplicateActors_BuildConsiderList`, `Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5303`, 활성 목록만 돈다(`5315`) |
| ① 고려할 시각 | `World->TimeSeconds <= ActorInfo->NextUpdateTime`이면 건너뜀(`NetDriver.cpp:5319-5323`), 다음 시각 계산(`5420-5425`) |
| ② Relevancy | `ServerReplicateActors_PrioritizeActors`가 채널이 없는 액터에 검사(`NetDriver.cpp:5580-5593`), `AActor::IsNetRelevantFor`(`ActorReplication.cpp:388-419`) |
| ③ Dormancy | 이 연결에서 Dormant 상태면 건너뜀(`NetDriver.cpp:5618-5624`) |
| 우선순위 정렬 | `NetDriver.cpp:5669` |
| ④ 프로퍼티 비교와 직렬화 | `UActorChannel::ReplicateActor`. Insights의 클래스 이름 타이머가 이 함수다(`DataChannel.cpp:3622-3625`) |
| 패킷 송신 | 송신 버퍼가 차면 그 자리에서, 나머지는 프레임 끝의 `Connection->Tick`(`NetDriver.cpp:1304-1307`) |

- 줄인 것: 흐름도에는 우선순위 정렬(③과 ④ 사이)과 건너뛰는 가지를 그리지 않았다. 세로로 길어 한 화면에 들어오지 않아서 두 줄로 줄였다(2026-10-03). 채널이 이미 열린 액터의 Relevancy는 ④ 직전에 다시 확인하고, 일정 시간 Relevancy를 잃은 채널을 닫는다(`NetDriver.cpp:5877-5889`). 모든 연결에서 Dormant 상태가 된 액터는 활성 목록에서 빠진다(`NetworkObjectList.cpp:348-376`).
- 본문의 "네트워크 드라이버 자체 시간"은 `GameNetDriver`의 Exclusive다. Consider List 만들기, 연결마다의 우선순위 정렬(②와 ③ 포함), 프레임 끝의 송신이 섞여 있다. 이들을 재는 `STAT_NetConsiderActorsTime` 같은 stat은 기본 트레이스(`-trace=default,net`)에 남지 않는다. 프레임 하나를 확대해 어림한 내용은 [후보 기법 자료](candidates.md) 2절에 있다.

## 8. "선택"의 근거

- 기법과 순서는 사용자가 정했다. 후보 여섯 개의 엔진 소스 위치와 에이전트 의견은 [후보 기법 자료](candidates.md) 5\~6절에 있다.
- 기준선이 엔진 기본 동작을 끈 결정은 [ADR-0003](../../Docs/Decisions/0003-lawless-baseline.md)이다.
- Relevancy의 기대 수: 배치 영역 1.9km × 1.9km에서 반경 150m 안은 노드 약 98개(5,000 × π × 150² ÷ 1,900²), NPC 약 6명이다.
- Dormancy의 처음 예상: 모든 연결에서 Dormant 상태가 된 자원 노드가 활성 목록에서 빠져 자원 노드 5,001 × 연결 8의 거리 검사가 줄어든다. Relevancy를 적용한 뒤에는 한 자원 노드에 채널을 여는 연결이 몇 개뿐이라 이 조건이 채워지지 않는다([자원 노드 Dormancy의 측정 기록](../03-dormancy/measurements.md)).
- Adaptive Net Update Frequency는 꺼져 있다(`net.UseAdaptiveNetUpdateFrequency` 기본값 0, `NetDriver.cpp:523-526`).

## 9. 확인하지 않은 것

- `GameNetDriver` Exclusive(37%)의 내역. 나누려면 `-statnamedevents`를 준 별도 실행이 필요하고, 그러면 측정 조건이 달라진다.
- 실행 사이 흔들림의 원인.
- `r2`의 Network Insights 값과 `Connection 1`\~`7`의 송신 대역폭.
- "선택"에 적은 기대 효과. 모두 계산이나 엔진 소스에서 읽은 추론이었고, 뒤의 세 글에서 측정했다.
