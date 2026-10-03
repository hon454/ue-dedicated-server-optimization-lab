# 포스팅 4 후보 기법 자료 (`dormancy6`, `update-frequency3`)

구현 계획 태스크 11.7의 자료다. 단기에 구현하는 기법은 이 포스팅이 마지막이라 남은 기법 표는 없다. Insights에서 읽은 값, 에이전트 의견, 확인하지 않은 것만 적는다.

- 대상 실행: `dormancy6-r1`\~`r3`(직전 구성을 다시 잰 것. `SetNetUpdateFrequency` 한 줄을 되돌린 빌드), `update-frequency3-r1`\~`r3`. 조건은 확정 명령과 같고, 본체 화면(3840×2160)에서 연달아 쟀다. CSV 수치는 [STATUS.md](../../Docs/STATUS.md)의 "측정 결과"에 있다. `work_avg_ms`의 중앙값 실행은 두 구성 모두 `r1`이다.
- 읽은 방법: 여섯 실행의 `Lab_MeasureStart`, `Lab_MeasureEnd` 시각은 Insights 창의 Log View에서 읽었다. 1절의 Count, Incl, Excl은 그 시각을 `-startTime`, `-endTime`으로 준 Insights 내보내기(`TimingInsights.ExportTimerStatistics`, `TimingInsights.ExportTimerCallees -timers=WorldTick`, `-threads=GameThread`)의 값이다. 두 `r1`은 창의 Timers와 Callees 패널에서도 읽어 같은 값임을 확인했다(`dormancy6-r1`: `GameNetDriver` 16.76초 / 15.55초, `LabNpc` 78,876 / 671.55ms. `update-frequency3-r1`: 15.84초 / 15.02초, 25,580 / 279.12ms). P99는 `TimingInsights.ExportTimingEvents`로 계산했다([insights-reading.md](../../Docs/Guides/insights-reading.md) "P99 읽기"). 2절은 두 `r1`의 Networking Insights 창(3000×2080)에서 읽었다.
- 1\~3절은 읽은 값과 그 값으로 계산한 값이다. 4절만 해석이다.

## 1. 서버 프레임 (Timing Insights)

| 항목 | `dormancy6-r1` | `r2` | `r3` | `update-frequency3-r1` | `r2` | `r3` |
| --- | --- | --- | --- | --- | --- | --- |
| `Lab_MeasureStart` | 1분 31.436789초 | 1분 30.484993초 | 1분 30.496327초 | 1분 31.881453초 | 1분 30.729072초 | 1분 31.104750초 |
| `Lab_MeasureEnd` | 2분 31.464644초 | 2분 30.515251초 | 2분 30.506096초 | 2분 31.884040초 | 2분 30.735122초 | 2분 31.122883초 |
| `Frame` Count | 1,793 | 1,795 | 1,798 | 1,798 | 1,796 | 1,799 |
| `WorldTick` Count / Incl | 1,792 / 23.60초 | 1,794 / 24.08초 | 1,797 / 23.21초 | 1,797 / 22.87초 | 1,795 / 22.10초 | 1,798 / 22.93초 |
| `FEngineLoop_UpdateTimeAndHandleMaxTickRate` Incl(틱 속도 제한 대기) | 35.635초 | 35.143초 | 36.015초 | 36.337초 | 37.124초 | 36.292초 |
| `GameNetDriver` Incl / Excl | 16.76초 / 15.55초 | 16.97초 / 15.75초 | 16.14초 / 14.97초 | 15.84초 / 15.02초 | 15.23초 / 14.44초 | 15.69초 / 14.91초 |
| `GameNetDriver`의 `% Root`(`WorldTick` 대비) | 71.02% | 70.46% | 69.51% | 69.28% | 68.90% | 68.43% |
| `LabNpc` Count / Incl / `% Parent`(`GameNetDriver` 아래) | 78,876 / 671.55ms / 4.01% | 78,951 / 679.80ms / 4.01% | 78,984 / 651.37ms / 4.04% | 25,580 / 279.12ms / 1.76% | 25,637 / 272.40ms / 1.79% | 25,654 / 264.61ms / 1.69% |
| `LabResourceNode` Count / Incl | 608 / 11.40ms | 595 / 10.91ms | 573 / 10.50ms | 581 / 11.75ms | 580 / 10.42ms | 605 / 10.95ms |

| 프레임당 값 | 식 | `dormancy6`(`r1` / `r2` / `r3`) | `update-frequency3`(`r1` / `r2` / `r3`) | 대조할 CSV |
| --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)) | (구간 − 대기) ÷ `Frame` Count | 13.60 / 13.86 / 13.35ms | 13.16 / 12.74 / 13.19ms | `work_avg_ms` 13.343 / 13.593 / 13.080, 12.894 / 12.479 / 12.923(+1.9\~+2.1%) |
| 서버 프레임 시간 P99 | 프레임마다 (`Frame` − 대기)의 ceil(N × 0.99)번째 | 20.84 / 21.47 / 20.50ms | 19.50 / 18.63 / 19.59ms | `work_p99_ms` 20.604 / 21.044 / 20.109, 19.080 / 18.298 / 19.327(+1.1\~+2.2%) |
| 리플리케이션 시간 | `GameNetDriver` Incl ÷ `WorldTick` Count | 9.35 / 9.46 / 8.98ms | 8.82 / 8.48 / 8.73ms | `netflush_avg_ms` 9.600 / 9.700 / 9.212, 9.054 / 8.719 / 8.969(-2.5\~-2.7%) |
| 그중 `LabNpc` | Incl ÷ `WorldTick` Count | 0.375 / 0.379 / 0.362ms | 0.155 / 0.152 / 0.147ms | |
| 그중 `GameNetDriver` Excl | Excl ÷ `WorldTick` Count | 8.68 / 8.78 / 8.33ms | 8.36 / 8.05 / 8.29ms | |
| 그중 나머지 | Incl − Excl − `LabNpc` − `LabResourceNode` | 0.30 / 0.29 / 0.28ms | 0.30 / 0.28 / 0.28ms | |
| 연결 하나당 프레임당 `LabNpc` 호출 | Count ÷ `WorldTick` Count ÷ 8 | 5.50 / 5.50 / 5.49 | 1.78 / 1.79 / 1.78 | |

- 중앙값(변동 폭): 서버 프레임 시간 평균 13.60(0.51) → 13.16(0.45), P99 20.84(0.97) → 19.50(0.96), 리플리케이션 시간 9.35(0.48) → 8.73(0.34), `LabNpc` 0.375 → 0.152, Excl 8.68(0.45) → 8.29(0.31).
- 판단 규칙(중앙값의 변화가 두 변동 폭 중 큰 쪽보다 큰가): 서버 프레임 시간 평균 0.44 < 0.51로 구별되지 않음, P99 1.34 > 0.97로 구별됨, 리플리케이션 시간 0.62 > 0.48로 구별됨, Excl 0.39 < 0.45로 구별되지 않음.
- `LabNpc` 호출 비율 1.78 ÷ 5.50 = 0.324.

## 2. 연결의 패킷 (Network Insights, `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`)

| 항목 | `dormancy6-r1` | `update-frequency3-r1` |
| --- | --- | --- |
| 선택 범위 | 1,795패킷, 60.095초(시작 묶음의 가장 큰 패킷 17,791번 1분 31.34초, 끝 묶음 19,585번 2분 31.43초) | 1,797패킷, 60.075초(시작 묶음 5,717번 1분 31.82초, 끝 묶음 7,513번 2분 31.89초) |
| `Actor` Count / Incl(비트) | 8,934 / 1,177,510 | 3,693 / 460,936 |
| `LabNpc` Count / Incl / I.Avg | 7,622 / 846,108 / 111 | 2,472 / 274,436 / 111 |
| `ReplicatedMovement` Count / Incl | 7,622 / 701,224 | 2,472 / 227,424 |
| `BunchHeader` Count / Incl | 8,934 / 241,678 | 3,695 / 100,279 |
| `PacketHeaderAndInfo` Count / Incl | 1,795 / 165,140 | 1,797 / 165,324 |
| `LabResourceNode` Count / Incl | 9 / 779 | 9 / 810 |
| `NewActor` Count / Incl | 10 / 1,657 | 10 / 1,831 |
| `PlayerState` Count / Incl | 300 / 10,500 | 224 / 7,840 |

| Insights에서 읽은 값 | 식 | `dormancy6-r1` | `update-frequency3-r1` |
| --- | --- | --- | --- |
| 연결당 송신 대역폭(내용 기준) | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위 시간 | 2,793바이트/초 | 1,303바이트/초 |
| `LabNpc` ÷ `Actor` | | 71.9% | 59.5% |
| 패킷당 `LabNpc` 갱신 | `LabNpc` Count ÷ 패킷 수 | 4.25 | 1.38 |
| `LabNpc` Count 비율 | | | 0.324 |

- 선택 범위의 시작은 화면 한 픽셀에 든 패킷 3개 묶음이라 북마크 시각과 0.06\~0.1초 어긋난다.
- **NPC 하나의 갱신 간격(`update-frequency3-r1`).** 패킷 6,329번(1분 52.26초)부터 Find Packet의 다음 단추로 20패킷을 하나씩 넘기며 패킷 내용의 `LabNpc (NetId …)`를 읽었다. NetId 1306과 1308은 6,329, 6,333, 6,337, 6,341, 6,345, 6,349번에, 1292는 6,330, 6,334, 6,338, 6,342, 6,346번에, 1300은 6,332, 6,336, 6,340, 6,344, 6,348번에 실렸다. 모두 4패킷 간격이다. 패킷 간격 60.075초 ÷ 1,797 = 33.4ms, 4패킷은 약 134ms다. 6,331, 6,335, 6,339, 6,343, 6,347번에는 NPC가 없었다(`GameStateBase`, `PlayerState` 등). `By NetId`와 Find Event의 다음 단추는 한 패킷 안에서만 움직여 패킷을 건너 찾지 못했다.
- `dormancy6-r1`에서는 NPC 하나의 간격을 읽지 않았다. 시작 묶음의 패킷 17,791번에는 `LabNpc` 6개가 실려 있었다.
- CSV `out_bytes_per_sec_per_conn`(4,323, 2,402)은 8개 연결 평균이고 `Connection 0`보다 크다. 포스팅 2, 3과 같은 모양이다. `Connection 1`\~`7`은 읽지 않았다.
- `PlayerState`가 300번에서 224번으로 줄었다. 이유는 확인하지 않았다.

## 3. 클라이언트에 존재하는 액터 수 (자동 스크린샷의 화면 글자)

| 화면 | `dormancy6-r1` | `update-frequency3-r1` |
| --- | --- | --- |
| 1번(이동, 내려다보기) t=45s(순번 02) | `nodes=301 npcs=7` | `nodes=301 npcs=7` |
| 1번 t=75s(순번 04) | `nodes=313 npcs=4` | `nodes=313 npcs=5` |
| 0번(채집, 3인칭) t=75s(순번 04) | `nodes=195 npcs=5` | `nodes=195 npcs=5` |

- CSV `open_actor_channels_per_conn`은 두 구성 모두 20이다.
- 1번 t=75s의 NPC 수가 4에서 5가 됐다. `update-frequency2-r3`도 5, `dormancy2-r2`는 4였다. 화면의 빨간 점은 두 화면 모두 네 개이고 위치가 같다. 다섯 번째가 화면 밖에 있는지, 왜 남았는지는 확인하지 않았다.

## 4. 에이전트 의견

화면에서 읽은 사실이 아니라 해석이다.

- **소스의 계산과 맞는다.** 기본값 100에서는 다음 고려 시각이 10\~43.3ms 뒤라 평균 1.3프레임에 한 번, 10에서는 100\~133.3ms 뒤라 4프레임에 한 번이다(`NetDriver.cpp:5319-5323, 5420-5425`). 비율 0.325가 `LabNpc` 호출 비율 0.324, `Connection 0`의 `LabNpc` Count 비율 0.324와 맞고, NPC 하나의 간격 4패킷과도 맞는다.
- **대역폭이 주된 효과다.** `Connection 0`에서 -53.3%, CSV에서 -44.4%로 변동 폭(9, 2)보다 훨씬 크다. 시간 지표는 리플리케이션 시간과 P99가 구별되지만 작고(-6.6%, -6.4%), 서버 프레임 시간 평균은 구별되지 않는다. 포스팅 3의 예고와 같은 방향이다.
- **리플리케이션 시간이 NPC 타이머보다 많이 줄었다.** 0.62ms 가운데 NPC 타이머는 0.23ms이고 0.39ms는 Excl이다. 고려 목록에서 빠진 NPC는 연결마다의 관련성 검사와 우선순위 정렬, 채널 처리에도 들어가지 않으므로(`NetDriver.cpp:5319-5323`에서 건너뛰면 고려 목록에 없다) Excl이 함께 주는 것이 자연스럽다. 다만 Excl의 차이는 변동 폭 안이라 확정할 수 없다. 확인하지 않은 것: Excl 안의 내역.
- **처음 비교(`dormancy2` 대비 `work_avg_ms` -1.814)는 기법의 효과가 아니었다.** 같은 코드의 `dormancy6`이 `dormancy2`보다 1.066 낮다. 연달아 잰 묶음끼리의 차이는 0.449다. 구성 사이의 비교는 같은 조건에서 연달아 잰 묶음으로 해야 한다는 근거가 된다. 원인은 확인하지 않았다.
- **끊김은 화면에서도 계산대로 나왔다.** 시연용 NPC의 60fps 영상(6절)에서 위치가 바뀐 간격의 평균이 46.4ms에서 130.6ms가 됐다. 계산값 43ms, 133ms와 맞는다. 사용자가 4배 느린 GIF와 실제 속도 원본 모두에서 끊김이 보인다고 확인했다(2026-10-02). 확인하지 않은 것: 다른 거리와 이동 방향, 30fps가 아닌 클라이언트, 클라이언트가 위치를 적용한 시각의 직접 기록.

## 5. 확인하지 않은 것

- `GameNetDriver` Excl(프레임당 8.05\~8.78ms)의 내역과, 0.39ms 감소가 실제인지.
- `dormancy2`와 `dormancy6`이 다른 이유.
- 측정 중 선호도 재설정이 이날 저녁에 잦았던 이유(`update-frequency`, `dormancy3`, `dormancy4`, `dormancy5`가 실패). 녹화한 `visual6-r1`은 녹화 시작(창을 맨 위로 올림) 몇 초 뒤에 재설정됐다.
- `Connection 1`\~`7`과 `r2`, `r3`의 Network Insights 값. 적용 전 NPC 하나의 갱신 간격.
- 1번 클라이언트 t=75s의 NPC 수가 4에서 5가 된 이유. `PlayerState` 전송이 300번에서 224번으로 준 이유.
- 끊김이 시연용 NPC가 아닌 조건(다른 거리와 이동 방향, 30fps가 아닌 클라이언트)에서 어떻게 보이는지. `before-clip.gif`, `after-clip.gif`(8fps)에서는 사용자가 차이를 구별하지 못했다(2026-10-02). 처음 찍은 `run-manual.ps1`의 관찰자 영상(24fps, 폭 480px, 전후가 같은 NPC와 같은 각도가 아님)에서도 구별하지 못해 시연용 NPC 영상으로 바꿨다.
- 송신 한도에 포화된 조건에서의 효과.

## 6. 스크린샷과 영상 후보

`Scripts/capture-insights.ps1`로 창 내용만 찍었다(3000×2080 창, Networking은 위쪽 1,000픽셀). 포스팅에 넣을 것은 사용자가 고른다.

- `insights-before-r1-timers.png`, `insights-after-r1-timers.png`: 측정 구간의 Timers와 `WorldTick` Callees. 상자를 그린 것이 `before-timing.png`, `after-timing.png`다(① `GameNetDriver`, ② `LabNpc`, ③ 두 북마크).
- `insights-before-r1-net-stats.png`, `insights-after-r1-net-stats.png`: `Connection 0` `Outgoing` 측정 구간의 Net Stats. 상자를 그린 것이 `before-network.png`, `after-network.png`다(① `Actor`, ② `LabNpc`, ③ 고른 범위).
- `before-npc.gif`(`visual9-r1`, 한 줄을 되돌린 빌드), `after-npc.gif`(`visual10-r1`): 확정 규모에 `-ShowcaseNpc`를 더한 실행의 0번 클라이언트 창(화면 영역 0,0 960×540, 실행 중에 창의 클라이언트 영역에서 읽음)을 `capture-video.ps1 -Region "0,0,960,540" -Fps 60 -NoMouse -AllowMeasuring`으로 8초 찍었다. 녹화는 시작 신호(서버 로그 `Scenario started`) 뒤 48.5초에 시작시켰고 끝난 시각은 59.80초와 59.22초다. 두 실행 모두 종료 코드 0이다. 원본은 `Saved/Screenshots/Lab/visual9-npc.mp4`, `visual10-npc.mp4`다. GIF는 ffmpeg로 따로 만들었다: 60fps로 맞춘 뒤 NPC가 같은 위치에 오도록 적용 후를 27프레임 자르고(왼쪽 끝에 닿은 프레임이 140번과 167번), 454프레임을 4배 느리게(15fps), 영역 520,220 440×120을 줄이지 않고 잘라 64색으로 줄였다.
- `npc-position.png`: 같은 두 MP4에서 영역 480,262 480×48을 프레임마다 뽑아, 빨간색(R>170, G<125, B<115, R-G>80)인 픽셀이 가장 많이 이어진 열 묶음의 x 중심을 읽었다(481프레임, `visual9`에서 2프레임은 찾지 못해 앞 프레임 값을 썼다). 그림은 맞춘 뒤의 150번 프레임부터 2초다. x가 1px 넘게 바뀐 프레임을 위치가 바뀐 것으로 셌다.

| 항목 | `visual9-r1` | `visual10-r1` |
| --- | --- | --- |
| 위치가 바뀐 횟수 | 172 | 61 |
| 간격(프레임 수: 횟수) | 1: 13, 2: 78, 3: 33, 4: 34, 5: 9, 6: 1, 7: 3 | 2: 1, 3: 1, 4: 1, 5: 2, 7: 12, 8: 26, 9: 13, 10: 2, 11: 2 |
| 간격 평균 / 중앙값 / 최댓값 | 46.4 / 33.3 / 116.7ms | 130.6 / 133.3 / 183.3ms |
| 한 번에 움직인 픽셀: 평균 / 중앙값 / 최솟값 / 최댓값 | 4.78 / 3.68 / 2.19 / 10.98 | 13.66 / 14.10 / 1.03 / 15.15 |
| x의 최솟값 / 최댓값(왕복 구간 10m) | 559.2 / 904.4(345px) | 556.8 / 907.8(351px) |

- 1프레임(16.7ms) 간격 13번은 30fps 클라이언트 화면을 60fps로 받을 때의 어긋남으로 보인다(확인하지 않음).
- `after-clip.gif`: `visual6-r1`의 1번, 2번 클라이언트 창(영역 959,0 1922×541), t=51\~60s. `before-clip.gif`는 포스팅 3의 `after-clip.gif`(`visual5-r1`, t=50\~59s)를 복사했다.

![적용 전 r1 측정 구간의 Timers와 WorldTick Callees](images/insights-before-r1-timers.png)

![적용 후 r1 측정 구간의 Timers와 WorldTick Callees](images/insights-after-r1-timers.png)

![적용 전 r1 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-before-r1-net-stats.png)

![적용 후 r1 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-after-r1-net-stats.png)
