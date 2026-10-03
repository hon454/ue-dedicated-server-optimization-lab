# Relevancy와 Net Cull Distance: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값은 [후보 기법 자료](candidates.md)에 있다.

## 1. 실행 조건

- 적용 전은 `baseline3`, 적용 후는 `relevancy2`다. 각각 같은 명령으로 세 번 실행했다(`r1`\~`r3`).
- 조건: 클라이언트 8, 자원 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 여섯 실행 모두 종료 코드 0으로 끝났고 `saturated_ratio`는 0.000이다.
- Insights 값은 두 북마크(`Lab_MeasureStart`, `Lab_MeasureEnd`) 사이의 측정 구간에서 읽었다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 198ms → 17.6ms(-91%) | 198.43ms → 17.64ms(-91.1%) | Timing Insights, 세 실행의 중앙값(`baseline3-r1`, `relevancy2-r3`). 3절 |
| 리플리케이션 시간 189ms → 13.1ms(-93%) | 188.97ms → 13.14ms(-93.0%) | Timing Insights `GameNetDriver`, 세 실행의 중앙값(둘 다 `r1`). 3절 |
| 연결당 송신 대역폭 28,000 → 2,900바이트/초(-90%) | 28,048 → 2,897바이트/초(-89.7%) | Network Insights `Connection 0`(`baseline3-r1`, `relevancy2-r3`). 3절 |
| 8개 연결의 평균으로는 86% 감소 | 28,896 → 4,076바이트/초(-85.9%) | CSV `out_bytes_per_sec_per_conn`의 중앙값. 4절 |
| 클라이언트에 존재하는 자원 노드 5,001 → 114개 | 같음 | 1번 클라이언트의 내려다보기 화면 글자(`baseline3-r1`, `relevancy2-r3`, t=45s) |
| 한 프레임의 처리 횟수 42,408번 | (5,001 + 300) × 8 | `baseline3-r1`의 타이머 Count. `LabResourceNode` 12,082,416 = 5,001 × 8 × `WorldTick` 302, `LabNpc` 724,800 = 300 × 8 × 302 |
| 리플리케이션이 차지하는 비율 95% | 95.17% | `baseline3-r1`의 `GameNetDriver` `% Root`(`WorldTick` 대비). 세 실행에서 95.00\~95.80% |
| 틱 예산의 약 6배 | 198.43 ÷ 33.3 = 5.96 | 틱 예산은 1000 ÷ `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`) |
| 원의 넓이는 배치 영역의 약 2% | π × 150² ÷ 1,900² = 1.96% | 배치 영역 1.9km × 1.9km(`Source/DSOptLab/LabScenarioConfig.h`의 `WorldHalfExtent` 95,000cm) |
| 예상: 자원 노드 약 98개, NPC 약 6명 | 5,000 × 0.0196 = 97.9, 300 × 0.0196 = 5.9 | 균등 배치를 가정한 계산값 |
| 틱 예산을 넘은 프레임 1\~4개 | 1 / 4 / 1 | CSV `over_budget_frames`(`relevancy2-r1`\~`r3`). 적용 전은 `frames`와 같다(302 / 292 / 334) |
| 내역: 액터 채널의 처리 118ms → 2.4ms | 98.5 + 19.5 = 118.0ms → 1.98 + 0.42 = 2.40ms | `LabResourceNode`와 `LabNpc` 타이머의 합. 5절 |
| 내역: 네트워크 드라이버 자체 시간 69.9ms → 10.4ms, 79% | 69.9ms → 10.43ms, 10.43 ÷ 13.14 = 79.4% | `GameNetDriver` Exclusive. 5절 |

## 3. 세 실행의 Insights 값

| 지표 | `r1` | `r2` | `r3` | 중앙값 | 변동 폭 | 기준선 중앙값(변동 폭) |
| --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 17.60 | 17.76 | 17.64 | 17.64 | 0.16 | 198.43(25.36) |
| 서버 프레임 시간 P99(ms) | 26.64 | 30.18 | 26.12 | 26.64 | 4.06 | 262.96(29.66) |
| 리플리케이션 시간(ms/프레임) | 13.14 | 13.06 | 13.24 | 13.14 | 0.18 | 188.97(22.80) |
| 연결당 송신 대역폭(바이트/초, `Connection 0`) | 읽지 않음 | 읽지 않음 | 2,897 | | | 28,048(`r1`) |
| 연결당 열린 액터 채널 수(CSV) | 118 | 118 | 118 | 118 | 0 | 5,314(0) |

- 세 시간 지표 모두 중앙값의 변화(180.79ms, 236.32ms, 175.83ms)가 두 구성의 변동 폭 중 큰 쪽(25.36ms, 29.66ms, 22.80ms)보다 크다. 구별되는 차이다.
- 서버 프레임 시간은 프레임 시간에서 틱 속도 제한 대기(`FEngineLoop_UpdateTimeAndHandleMaxTickRate`)를 뺀 시간이다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)). 평균은 (측정 구간 − 대기) ÷ `Frame` Count다(`r3`: (60.019초 − 28.297초) ÷ 1,798).
- P99는 프레임마다 대기를 뺀 길이를 `TimingInsights.ExportTimingEvents`로 내보내 정렬한 ceil(N × 0.99)번째 값이다.
- 리플리케이션 시간은 `GameNetDriver` Incl ÷ `WorldTick` Count다(`r1`: 17.13초 ÷ 1,304).
- 연결당 송신 대역폭은 (`Actor` Incl 1,221,591 + `PacketHeaderAndInfo` Incl 164,864)비트 ÷ 8 ÷ 59.817초다(`r3` `Connection 0` `Outgoing`, 1,792패킷).

## 4. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `over_budget_frames` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `relevancy2-r1` | 1,304 | 17.254 | 25.984 | 1 | 13.357 | 3,871 | 118 | 0.000 |
| `relevancy2-r2` | 1,488 | 17.447 | 29.711 | 4 | 13.293 | 4,076 | 118 | 0.000 |
| `relevancy2-r3` | 1,797 | 17.380 | 25.685 | 1 | 13.478 | 4,475 | 118 | 0.000 |
| 중앙값 | 1,488 | 17.380 | 25.984 | 1 | 13.357 | 4,076 | 118 | 0.000 |
| 변동 폭 | 493 | 0.193 | 4.026 | 3 | 0.185 | 604 | 0 | 0.000 |
| 기준선 중앙값 | 302 | 198.773 | 262.736 | 302 | 189.375 | 28,896 | 5,314 | 0.000 |

- Insights 값과의 차이: 서버 프레임 시간 평균과 `work_avg_ms`가 +1.5\~+2.0%, P99와 `work_p99_ms`가 +1.6\~+2.5%, 리플리케이션 시간과 `netflush_avg_ms`가 -1.6\~-1.8%.
- `frames`가 실행마다 다르다(1,304\~1,797). 일한 시간은 거의 같고(`work_avg_ms` 변동 폭 0.193) 틱 속도 제한 대기가 실행마다 다르다(프레임당 15.75\~28.41ms). 대기가 왜 다른지는 이 측정에서 확인하지 않았다. 대기를 빼지 않으면 서버 프레임 시간이 33.38\~45.99ms로 흔들린다.
- 초당 값인 `out_bytes_per_sec_per_conn`은 `frames`를 따라 커진다(1,304프레임 3,871, 1,488프레임 4,076, 1,797프레임 4,475).
- `Connection 0`의 2,897바이트/초는 CSV의 8개 연결 평균 4,475(`r3`)보다 35% 작다. CSV는 패킷마다 IP와 UDP 헤더 28바이트를 더하는데(`Engine/Source/Runtime/Engine/Private/NetConnection.cpp:2562-2586`), 헤더를 더해도 3,736바이트/초로 평균보다 작다. 연결마다 받는 액터가 위치에 따라 달라서, 제자리에서 채집하는 `Connection 0`이 평균보다 적게 받는 것으로 보인다. 나머지 연결과 `r1`, `r2`는 읽지 않았다. 기준선은 모든 연결이 같은 액터를 받아 차이가 3%였다.

## 5. 리플리케이션 시간의 내역

| 타이머(프레임당) | 기준선 `r1` | Relevancy `r1` | 변화 |
| --- | --- | --- | --- |
| `LabResourceNode` | 98.5ms (52.1%) | 1.98ms (15.1%) | -98.0% |
| `LabNpc` | 19.5ms (10.3%) | 0.42ms (3.2%) | -97.8% |
| `GameNetDriver` Exclusive | 69.9ms (37.0%) | 10.43ms (79.4%) | -85.1% |
| 나머지 | 1.1ms | 0.31ms | |
| 합계(리플리케이션 시간) | 188.97ms | 13.14ms | -93.0% |

- 각 값은 타이머 Incl(Exclusive는 Excl) ÷ `WorldTick` Count이고, 괄호는 리플리케이션 시간 대비 비율이다. 나머지는 합계에서 위 세 줄을 뺀 값이다.
- 본문의 "액터 채널의 처리"는 `LabResourceNode`와 `LabNpc`의 합이다. 이 타이머는 `UActorChannel::ReplicateActor`가 C++ 부모 클래스 이름으로 남긴다(`Engine/Source/Runtime/Engine/Private/DataChannel.cpp:3622-3625`의 `SCOPE_CYCLE_UOBJECT`).
- `GameNetDriver` Exclusive는 세 실행에서 프레임당 10.43\~10.80ms이고 리플리케이션 시간의 79\~82%다. Consider List 만들기, Net Cull Distance 검사, 우선순위 정렬, 송신이 섞여 있어 이 트레이스로는 따로 볼 수 없다.
- 송신 비트의 68.7%가 여전히 NPC 이동이다(적용 전 76.0%). `Connection 0`의 `Actor` 1,221,591비트 중 `LabNpc` 839,204비트다. `LabResourceNode`는 27번, 1,314비트다.

## 6. 예상과 실제

| 항목 | 예상(계산) | 실제 | 근거 |
| --- | --- | --- | --- |
| 클라이언트에 존재하는 자원 노드 수 | 약 98 | 114(1번, t=45s), 80(0번, t=75s) | `relevancy2-r3` 자동 스크린샷의 화면 글자 |
| 클라이언트에 존재하는 NPC 수 | 약 6 | 7(1번), 5(0번) | 같음 |
| 연결 하나가 프레임마다 처리하는 자원 노드 | 약 98 | 75.4\~95.6번 | `LabResourceNode` Count ÷ `WorldTick` Count ÷ 8(`r3` 1,083,238 ÷ 1,797 ÷ 8, `r1` 997,693 ÷ 1,304 ÷ 8) |
| 연결 하나가 프레임마다 처리하는 NPC | 약 6 | 5.5\~7.0번 | `LabNpc` 같은 식 |

- 다른 플레이어는 보이지 않는다(`players=1`). 플레이어의 시작 자리 간격(약 195m)이 Net Cull Distance 150m보다 크다.
- 적용 후 3인칭 화면에 캐릭터 앞의 검증용 노드가 없는 것은 그 시점에 채집으로 고갈되어 있기 때문이다.

## 7. Insights 화면

적용 전(`baseline3-r1`)과 적용 후(`relevancy2-r3`)의 측정 구간이다.

| 적용 전 | 적용 후 |
| --- | --- |
| ![적용 전 Timers와 WorldTick Callees](images/before-timing.png) | ![적용 후 Timers와 WorldTick Callees](images/after-timing.png) |
| ![적용 전 Connection 0 Outgoing의 Net Stats](images/before-network.png) | ![적용 후 Connection 0 Outgoing의 Net Stats](images/after-network.png) |

- Timing Insights: ① `GameNetDriver`(`% Root` 95.17% → 76.90%), ② `LabResourceNode`와 `LabNpc`(`% Parent` 52.14%, 10.31% → 13.15%, 2.95%), ③ 두 북마크.
- Network Insights: ① `Actor`와 `LabNpc`의 비트(13,269,082, 10,090,719 → 1,221,591, 839,204), ② `LabResourceNode`의 비트(9번 576 → 27번 1,314), ③ 고른 측정 구간(1,801패킷 59.800초 → 1,792패킷 59.817초).

## 8. 엔진 소스 위치

| 사실 | 위치 |
| --- | --- |
| Net Cull Distance 기본값 150m(`NetCullDistanceSquared` 225,000,000) | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `bAlwaysRelevant`면 참, 아니면 거리로 판정 | `AActor::IsNetRelevantFor`, `Engine/Source/Runtime/Engine/Private/ActorReplication.cpp:388-419` |
| 서버는 채널이 없는 액터마다 연결별로 Relevancy를 검사한다 | `ServerReplicateActors_PrioritizeActors`, `Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5580-5593` |
| Relevancy를 잃은 액터의 채널을 닫는다 | `NetDriver.cpp:5877-5889` |
| 판정의 기준 위치는 서버가 계산한 3인칭 카메라 위치다 | [engine-notes.md](../../Docs/Reference/engine-notes.md) "관련성 판정의 기준 위치" |

본문의 흐름도는 줄인 그림이다. 실제로는 채널이 이미 열린 액터의 Relevancy를 처리 단계에서 다시 확인하고, 일정 시간 Relevancy를 잃은 채널을 닫는다.

## 9. 빌드와 시각 자료의 사정

- 수치를 잰 빌드는 커밋 `b438852`다. 태그 `post-02-relevancy`에는 그 뒤에 넣은 화면 표시 변경(`8075998`, 채집 중인 노드의 높이를 체력에 비례해 줄이고 내려다보기 화면에서 노란 점으로 그림)이 함께 들어 있다. 이미 리플리케이트하던 체력을 화면에 그리는 변경이라 보내는 프로퍼티는 같다. 서버 수치로 대조하지는 않았다.
- 영상 `after-clip.gif`는 수치를 쓰지 않는 시각 자료 전용 실행 `visual2`의 측정 구간에서 찍었다. 1번(내려다보기)과 2번(3인칭) 클라이언트가 정해진 정사각형 경로를 걷는 10초다. 1번 클라이언트는 x 516m에서 466m로 걸었고 화면 글자의 노드 수는 108\~115였다. 적용 전 영상 `before-clip.gif`는 두 줄을 잠시 되돌려 빌드한 `visual3`에서 찍었고 본문에는 넣지 않았다.
- 3인칭 화면은 0번 클라이언트의 t=75s다(`nodes=5001` → `nodes=80`).
- 개념도 `relevancy-map.svg`는 `Scripts/make-relevancy-map.ps1`이 만든다. 맵 크기, 플레이어 자리와 경로, 150m 원은 소스의 값이고, 점의 위치는 그 스크립트의 고정 시드로 뿌린 예시다. 8배속이다.

## 10. 확인하지 않은 것

- `GameNetDriver` Exclusive의 내역. 거리 검사가 이 시간에 들어 있다는 것은 추정이다.
- `Connection 1`\~`7`과 `r1`, `r2`의 연결당 송신 대역폭.
- 틱 속도 제한 대기가 실행마다 다른 이유.
