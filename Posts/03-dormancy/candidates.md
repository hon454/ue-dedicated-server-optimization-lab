# 포스팅 3 후보 기법 자료 (`dormancy2`)

구현 계획 태스크 11.7의 자료다. 자원 노드 휴면을 적용한 뒤의 트레이스(`dormancy2-r1`\~`r3`)를 에이전트가 Unreal Insights로 읽었다. 남은 기법의 엔진 소스 위치는 [포스팅 1의 후보 기법 자료](../01-baseline/candidates.md) 5절과 같다. 순서는 포스팅 1의 "선택"에서 사용자가 정했다(다음은 NPC 업데이트 빈도).

- 대상 실행: `dormancy2-r1`\~`r3`(조건은 `relevancy2`와 같고 코드만 다르다). CSV 수치는 [STATUS.md](../../Docs/STATUS.md)의 "측정 결과"에 있다. `work_avg_ms`의 중앙값 실행은 `r2`(14.409)다.
- 읽은 방법: 세 실행의 `Lab_MeasureStart`, `Lab_MeasureEnd` 시각은 Insights 창의 Log View에서 읽었다. 1절의 Count, Incl, Excl은 그 시각을 `-startTime`, `-endTime`으로 준 Insights 내보내기(`TimingInsights.ExportTimerStatistics`, `TimingInsights.ExportTimerCallees -timers=WorldTick`, `-threads=GameThread`)의 값이다. `r2`는 창의 Timers와 Callees 패널에서도 읽어 내보내기와 같은 값임을 확인했다(`GameNetDriver` 18.18초 / 16.86초, `LabResourceNode` 609 / 12.52ms, `LabNpc` 78,801 / 729.23ms). P99는 `TimingInsights.ExportTimingEvents`로 계산했다([insights-reading.md](../../Docs/Guides/insights-reading.md) "P99 읽기"). 2절은 `r2`의 Networking Insights 창에서 읽었다.
- 1\~3절은 읽은 값과 그 값으로 계산한 값이다. 4절만 해석이다.

## 1. 서버 프레임 (Timing Insights)

| 항목 | `r1` | `r2` | `r3` |
| --- | --- | --- | --- |
| `Lab_MeasureStart` | 1분 31.005473초 | 1분 31.409156초 | 1분 30.672656초 |
| `Lab_MeasureEnd` | 2분 31.032874초 | 2분 31.424215초 | 2분 30.676739초 |
| `Frame` Count | 1,289 | 1,789 | 1,790 |
| `WorldTick` Count / Incl | 1,288 / 20.63초 | 1,788 / 25.43초 | 1,789 / 25.03초 |
| `FEngineLoop_UpdateTimeAndHandleMaxTickRate` Incl(틱 속도 제한 대기) | 38.660초 | 33.732초 | 34.130초 |
| `GameNetDriver` Incl / Excl | 14.81초 / 13.66초 | 18.18초 / 16.86초 | 17.96초 / 16.67초 |
| `GameNetDriver`의 `% Root`(`WorldTick` 대비) | 71.78% | 71.48% | 71.77% |
| `LabResourceNode` Count / Incl / `% Parent`(`GameNetDriver` 아래) | 546 / 11.22ms / 0.08% | 609 / 12.52ms / 0.07% | 607 / 11.89ms / 0.07% |
| `LabNpc` Count / Incl / `% Parent`(`GameNetDriver` 아래) | 73,128 / 636.82ms / 4.30% | 78,801 / 729.23ms / 4.01% | 78,924 / 709.71ms / 3.95% |

| 프레임당 값 | 식 | `r1` | `r2` | `r3` | `relevancy2`(`r1` / `r2` / `r3`) | 대조할 CSV(`r1` / `r2` / `r3`) |
| --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)) | (구간 − 대기) ÷ `Frame` Count | 16.58ms | 14.69ms | 14.45ms | 17.60 / 17.76 / 17.64 | `work_avg_ms` 16.205 / 14.409 / 14.173(+1.9\~+2.3%) |
| 서버 프레임 시간 P99 | 프레임마다 (`Frame` − 대기)의 ceil(N × 0.99)번째 | 25.02ms | 22.53ms | 21.34ms | 26.64 / 30.18 / 26.12 | `work_p99_ms` 24.410 / 22.063 / 21.116(+1.1\~+2.5%) |
| 리플리케이션 시간 | `GameNetDriver` Incl ÷ `WorldTick` Count | 11.50ms | 10.17ms | 10.04ms | 13.14 / 13.06 / 13.24 | `netflush_avg_ms` 11.774 / 10.430 / 10.298(-2.3\~-2.5%) |
| 그중 `LabResourceNode` | Incl ÷ `WorldTick` Count | 0.009ms | 0.007ms | 0.007ms | 1.98 / 1.86 / 1.74 | |
| 그중 `LabNpc` | Incl ÷ `WorldTick` Count | 0.49ms | 0.41ms | 0.40ms | 0.42 / 0.40 / 0.39 | |
| 그중 `GameNetDriver` Excl | Excl ÷ `WorldTick` Count | 10.60ms | 9.43ms | 9.32ms | 10.43 / 10.50 / 10.80 | |
| 연결 하나당 프레임당 `LabResourceNode` 호출 | Count ÷ `WorldTick` Count ÷ 8 | 0.053 | 0.043 | 0.042 | 95.6 / 86.0 / 75.4 | |
| 연결 하나당 프레임당 `LabNpc` 호출 | Count ÷ `WorldTick` Count ÷ 8 | 7.10 | 5.51 | 5.51 | 6.97 / 6.26 / 5.51 | |

- **노드 직렬화가 사라졌다.** `LabResourceNode` 호출이 60초에 546\~609번이다(적용 전 997,693\~1,083,238번). 프레임당 1.74\~1.98ms가 0.007\~0.009ms가 됐다.
- **`GameNetDriver` Excl은 대부분 남았다.** 중앙값 10.50ms에서 9.43ms로 1.07ms(10.2%) 줄었다. `dormancy2`의 변동 폭이 1.28ms(10.60 − 9.32)라서 이 차이는 변동 폭 안이다. `r1`(10.60)은 적용 전 범위(10.43\~10.80) 안에 있다.
- **`r1`만 느리다.** `r1`은 서버 프레임 시간, 리플리케이션 시간, Excl, `LabNpc`가 모두 `r2`, `r3`보다 1.13\~1.24배 크고 `Frame` Count가 1,289로 적다. 호출 횟수(연결당 프레임당)는 `LabNpc` 7.10으로 다르지만 NPC 배회 위치에 따라 달라지는 값이다. 원인은 모른다.
- 중앙값: 서버 프레임 시간 평균 14.69ms(`r2`, 변동 폭 2.13), P99 22.53ms(`r2`, 변동 폭 3.68), 리플리케이션 시간 10.17ms(`r2`, 변동 폭 1.46).

## 2. 연결의 패킷 (Network Insights, `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`, `dormancy2-r2`)

| 항목 | `dormancy2-r2` | `relevancy2-r3`(포스팅 2) |
| --- | --- | --- |
| 선택 범위 | 1,788패킷, 59.985초(시작 묶음의 가장 큰 패킷 4,470번 1분 31.37초, 끝 묶음 6,257번 2분 31.36초) | 1,792패킷, 59.817초 |
| `Actor` Count / Incl(비트) | 8,820 / 1,165,673 | 8,796 / 1,221,591 |
| `LabNpc` Count / Incl | 7,553 / 838,449 | 7,560 / 839,204 |
| `ReplicatedMovement` Incl | 694,876 | 695,520 |
| `PacketHeaderAndInfo` Count / Incl | 1,788 / 164,496 | 1,792 / 164,864 |
| `LabResourceNode` Count / Incl | 9 / 810 | 27 / 1,314 |
| `Health` Count / Incl, `bDepleted` Count / Incl | 9 / 360, 9 / 81 | 읽지 않음 |
| `NewActor` Count / Incl | 11 / 1,954 | 1 / 123 |

| Insights에서 읽은 값 | 식 | `dormancy2-r2` | `relevancy2-r3` |
| --- | --- | --- | --- |
| 연결당 송신 대역폭(내용 기준) | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위 시간 | 2,772바이트/초 | 2,897바이트/초 |
| 같은 값에 IP와 UDP 헤더를 더한 값 | 위 값 + 28 × 패킷 수 ÷ 선택 범위 시간 | 3,607바이트/초 | 3,736바이트/초 |
| `LabNpc` ÷ `Actor` | | 71.9% | 68.7% |

- 선택 범위의 시작은 화면 한 픽셀에 든 패킷 4개 묶음이라, 북마크 시각(1분 31.409초)과 최대 0.1초쯤 어긋난다.
- **송신 비트는 거의 그대로다.** 노드는 적용 전에도 60초에 1,314비트뿐이었다. 연결당 송신 대역폭 차이(-4.3%)는 `Actor` 비트 55,918의 차이인데, 노드 줄은 504비트만 줄었고 나머지는 읽은 줄로는 설명되지 않는다(`GameStateBase`, `PlayerState` 같은 줄을 적용 전에는 읽지 않았다).
- **`NewActor`가 11번이다(적용 전 1번).** 채널이 닫힌 휴면 노드를 깨우면 채널을 다시 열어 보낸다. `LabResourceNode` 9번, 평균 90비트다.
- CSV `out_bytes_per_sec_per_conn`(`r2` 4,316)은 8개 연결 평균이고 헤더를 더한 값(3,607)보다 크다. 포스팅 2와 같은 모양이다. `Connection 1`\~`7`은 읽지 않았다.

## 3. 클라이언트에 존재하는 액터 수 (자동 스크린샷의 화면 글자)

| 화면 | `relevancy2-r3` | `dormancy2-r2` |
| --- | --- | --- |
| 1번(이동, 내려다보기) t=15s | 읽지 않음 | `nodes=249 npcs=5` |
| 1번 t=45s(순번 02) | `nodes=114 npcs=7` | `nodes=301 npcs=7` |
| 1번 t=75s(순번 04) | `nodes=111 npcs=4` | `nodes=313 npcs=4` |
| 0번(채집, 3인칭) t=75s(순번 04) | `nodes=80 npcs=5` | `nodes=195 npcs=5` |

- CSV `open_actor_channels_per_conn`은 118에서 20으로 줄었다. 열린 채널 수와 클라이언트에 존재하는 액터 수가 반대로 움직인다.
- 1번 클라이언트는 t=15s에 이미 249개다. 시작 신호 전에 받은 노드가 남아 있는 것으로 보인다(확인하지 않음).
- 제자리에서 채집하는 0번 클라이언트도 195개다(적용 전 80개). 이동하지 않는데 늘어난 이유는 확인하지 않았다.

## 4. 에이전트 의견

화면에서 읽은 사실이 아니라 해석이다.

- **소스에서 예상한 대로다.** 줄어든 것은 채널이 열려 있던 노드의 직렬화(프레임당 약 1.86ms)이고, `GameNetDriver` Excl의 약 90%(9.43 ÷ 10.50)가 남았다. 노드가 활성 목록에서 빠지려면 모든 연결에서 휴면이어야 하는데(`NetworkObjectList.cpp:348-376`), 한 노드에 채널을 연 연결이 몇 개뿐이라 그렇게 되지 않는다([engine-notes.md](../../Docs/Planning/engine-notes.md) "휴면 액터와 관련성"). 근거 수치: `LabResourceNode` 호출 1,023,235 → 609, Excl 10.50 → 9.43ms(중앙값).
- **Excl이 1ms쯤 준 것은 연결마다의 우선순위 목록이 짧아졌기 때문으로 본다.** 휴면인 노드는 거리 검사 뒤에 건너뛰어(`NetDriver.cpp:5618-5624`) 우선순위 목록에 들어가지 않는다. 적용 전에는 연결당 약 98개의 노드가 목록에 들어가 정렬되고 채널마다 처리됐다. 다만 이 차이는 `dormancy2`의 변동 폭 안이라 확정할 수 없다. 확인하지 않은 것: Excl 안의 내역(거리 검사, 정렬, 송신).
- **남은 9.4ms의 대부분은 노드 5,001 × 연결 8의 거리 검사와 고려 목록 순회로 본다.** 이것을 줄이려면 노드가 고려 목록에 들어오지 않게 해야 하고(맵에 놓인 `DORM_Initial`, 구역 매니저, Replication Graph, Iris의 격자 필터), 단기의 세 기법 밖이다. backlog.md에 적었다.
- **NPC 업데이트 빈도가 겨냥하는 CPU 비용은 프레임당 0.41ms다(리플리케이션 시간의 4.0%).** 서버 프레임 간격이 이제 33.3ms(틱 속도 제한)라서, 빈도 10이면 다음 고려 시각이 0.1\~0.133초 뒤가 되어 3\~4프레임에 한 번 고려된다(`NetDriver.cpp:5420-5425`). 직렬화는 그만큼 줄겠지만 절대값이 작아 서버 프레임 시간에서는 변동 폭(2.13ms) 안에 들어갈 가능성이 높다. 대역폭은 송신 비트의 71.9%가 NPC라서 구별되는 차이가 날 수 있다. 확인하지 않은 것: 고려되지 않은 프레임에도 Excl의 NPC 몫이 남는지.

## 5. 남은 기법

| 기법 | 단기 구현 | 이 구성에서 겨냥하는 값(`r2`) | 바꿀 코드와 엔진 소스 |
| --- | --- | --- | --- |
| NPC 업데이트 빈도 | 예(포스팅 4) | NPC 직렬화 프레임당 0.41ms, NPC 비트 71.9%(`Connection 0`) | 포스팅 1 후보 기법 자료 5절의 "NPC 업데이트 빈도" 줄 |
| 적응형 업데이트 빈도, 푸시 모델, 송신 한도와 우선순위 | 아니오 | 포스팅 1과 같다. `saturated_ratio`는 세 실행 모두 0.000 | 포스팅 1 후보 기법 자료 5절 |

## 6. 확인하지 않은 것

- `GameNetDriver` Excl(프레임당 9.32\~10.60ms)의 내역.
- `r1`이 느린 이유와, 틱 속도 제한 대기가 실행마다 다른 이유(`frames` 1,288\~1,789).
- `Connection 1`\~`7`과 `r1`, `r3`의 Network Insights 값.
- 측정한 원격 데스크톱 세션의 화면 크기가 `relevancy2` 때와 달랐던 것(15:39에 다시 연결됨, 1728×1084)이 서버 수치에 영향을 줬는지.
- 멀리 있는 동안 바뀐 노드 상태가 채널이 다시 열릴 때 전달되는지. 정확성 확인(2026-10-02, `run-manual.ps1`, 에이전트가 관찰자를 조작하고 사용자가 화면을 보고 승인)에서는 돌아온 뒤 두 창이 어긋나지 않는 것까지만 봤다. 관찰자의 노드 수가 제자리에서 300에서 301로 는 것을 한 번 봤고 이유는 확인하지 않았다.

## 7. 스크린샷 후보

`Scripts/capture-insights.ps1`로 창 내용만 찍었다. 처음에는 창 크기가 1728×1044였고, 화면이 3840×2160으로 돌아온 뒤 포스팅 2와 같은 3000×2080으로 다시 찍었다(2026-10-02, 사용자 결정). 다시 찍을 때 Networking 화면의 한 픽셀에 드는 패킷 수가 달라져 고른 범위가 1,785패킷, 59.883초가 됐다(2절은 1,788패킷, 59.985초). 이 범위의 값은 `Actor` 8,805 / 1,163,648, `LabNpc` 7,539 / 836,895, `PacketHeaderAndInfo` 1,785 / 164,220, `LabResourceNode` 9 / 810이고, 연결당 송신 대역폭은 (1,163,648 + 164,220) ÷ 8 ÷ 59.883 = 2,772바이트/초로 2절과 같다. 포스팅 본문은 이미지에 맞춰 이 범위의 값을 쓴다. 포스팅에 넣을 것은 사용자가 고른다.

- `insights-r2-timers.png`: `r2` 측정 구간, Timers와 `WorldTick` Callees. 상자를 그린 것이 `after-timing.png`다(① `GameNetDriver`, ② `LabResourceNode`, ③ 두 북마크).
- `insights-r2-net-stats.png`: `r2` `Connection 0` `Outgoing` 1,788패킷의 Net Stats. 상자를 그린 것이 `after-network.png`다(① `Actor`와 `LabNpc`, ② `LabResourceNode`, ③ 고른 범위).
- `before-topdown-t75.png`, `after-topdown-t75.png`: 순번 04(t=75s)의 내려다보기 화면. 순번 02보다 원 밖에 남은 점이 잘 보인다.

![r2 측정 구간의 Timers와 WorldTick Callees](images/insights-r2-timers.png)

![r2 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-r2-net-stats.png)
