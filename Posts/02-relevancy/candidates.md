# 포스팅 2 후보 기법 자료 (`relevancy2`)

구현 계획 태스크 11.7의 자료다. 관련성을 적용한 뒤의 트레이스(`relevancy2-r1`\~`r3`)를 에이전트가 Unreal Insights로 열어 [insights-reading.md](../../Docs/Guides/insights-reading.md)의 순서로 읽었다. 남은 기법의 엔진 소스 위치는 [포스팅 1의 후보 기법 자료](../01-baseline/candidates.md) 5절과 같다. 순서는 포스팅 1의 "선택"에서 사용자가 정했다(다음은 자원 노드 휴면).

- 대상 실행: `relevancy2-r1`\~`r3`(조건은 `baseline3`와 같고 코드만 다르다). CSV 수치는 [STATUS.md](../../Docs/STATUS.md)의 "측정 결과"에 있다. `work_avg_ms`의 중앙값 실행은 `r3`(17.380)다.
- 1\~3절은 Insights 화면과 Insights 내보내기(`TimingInsights.ExportTimingEvents`)에서 읽은 값과 그 값으로 계산한 값이다. 계산에는 식을 적었다.

## 1. 서버 프레임 (Timing Insights)

| 항목 | `r1` | `r2` | `r3` |
| --- | --- | --- | --- |
| `Lab_MeasureStart` | 1분 34.615724초 | 1분 30.652966초 | 1분 30.383798초 |
| `Lab_MeasureEnd` | 2분 34.633355초 | 2분 30.675018초 | 2분 30.403101초 |
| `Frame` Count | 1,305 | 1,489 | 1,798 |
| `WorldTick` Count / Incl | 1,304 / 22.31초 | 1,488 / 25.72초 | 1,797 / 30.93초 |
| `FEngineLoop_UpdateTimeAndHandleMaxTickRate` Incl(틱 속도 제한 대기, 내보내기로 구간에 맞춰 자른 값) | 37.048초 | 33.570초 | 28.297초 |
| `GameNetDriver` Incl / Excl | 17.13초 / 13.6초 | 19.44초 / 15.62초 | 23.78초 / 19.41초 |
| `GameNetDriver`의 `% Root`(`WorldTick` 대비) | 76.80% | 75.56% | 76.90% |
| `LabResourceNode` Count / Incl / `% Parent`(`GameNetDriver` 아래) | 997,693 / 2.58초 / 15.09% | 1,023,235 / 2.77초 / 14.27% | 1,083,238 / 3.13초 / 13.15% |
| `LabNpc` Count / Incl / `% Parent`(`GameNetDriver` 아래) | 72,660 / 546.78ms / 3.19% | 74,463 / 597.12ms / 3.07% | 79,221 / 701.11ms / 2.95% |

| 프레임당 값 | 식 | `r1` | `r2` | `r3` | 대조할 CSV(`r1` / `r2` / `r3`) |
| --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간(옛 정의) | 구간 ÷ `Frame` Count | 45.99ms | 40.31ms | 33.38ms | `work_avg_ms` 17.254 / 17.447 / 17.380 |
| 서버 프레임 시간(대기를 뺀 값, [ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)의 정의) | (구간 − 대기) ÷ `Frame` Count | 17.60ms | 17.76ms | 17.64ms | 같음(+1.5\~2.0%) |
| 서버 프레임 시간 P99(대기를 뺀 값) | 프레임마다 (`Frame` − 대기)의 ceil(N × 0.99)번째 | 26.64ms | 30.18ms | 26.12ms | `work_p99_ms` 25.984 / 29.711 / 25.685 |
| 리플리케이션 시간 | `GameNetDriver` Incl ÷ `WorldTick` Count | 13.14ms | 13.06ms | 13.24ms | `netflush_avg_ms` 13.357 / 13.293 / 13.478 |
| 그중 `LabResourceNode` | Incl ÷ `WorldTick` Count | 1.98ms | 1.86ms | 1.74ms | |
| 그중 `LabNpc` | Incl ÷ `WorldTick` Count | 0.42ms | 0.40ms | 0.39ms | |
| 그중 `GameNetDriver` Excl | Excl ÷ `WorldTick` Count | 10.43ms | 10.50ms | 10.80ms | |
| 연결 하나당 프레임당 `LabResourceNode` 호출 | Count ÷ `WorldTick` Count ÷ 8 | 95.6 | 86.0 | 75.4 | |
| 연결 하나당 프레임당 `LabNpc` 호출 | Count ÷ `WorldTick` Count ÷ 8 | 6.97 | 6.26 | 5.51 | |

- **틱 예산 안으로 들어왔다.** `WorldTick` Incl ÷ Count가 17.1\~17.3ms이고, 남는 시간은 틱 속도 제한 대기다. 옛 정의(구간 ÷ `Frame` Count)의 서버 프레임 시간이 33.38\~45.99ms로 흔들리는 것은 이 대기 때문이다(ADR-0010의 맥락. 포스팅에는 대기를 뺀 값을 쓴다).
- **`GameNetDriver`의 대부분이 Excl이다.** 리플리케이션 시간의 79\~82%(`r3`: 10.80 ÷ 13.24)가 자식 타이머가 없는 시간이다. 기준선에서는 37%였다. 클래스 타이머(직렬화)는 연결당 액터 수에 맞게 줄었지만, Excl은 프레임당 69.9ms에서 10.80ms로 줄어 차지하는 비율이 커졌다.
- **노드 호출 수는 연결 위치에 따라 다르다.** 연결 하나당 프레임당 75\~96번이다. 구현 계획의 기대값(약 98개, 균등 배치 계산)과 같은 크기다.

## 2. 연결의 패킷 (Network Insights, `Game Instance 0 [Server]`, `Outgoing`, `relevancy2-r3`)

| 항목 | `Connection 0` | `Connection 1` |
| --- | --- | --- |
| 선택 범위 | 1,792패킷, 59.817초(시작 묶음의 다음 묶음 3,181번\~, 끝 4,978번 2분 30.40초) | 1,797패킷, 59.986초(시작 묶음의 가장 큰 패킷 5,561번 1분 30.38초, 끝 7,357번 2분 30.37초) |
| `Actor` Count / Incl(비트) | 8,796 / 1,221,591 | 8,508 / 1,165,017 |
| `LabNpc` Count / Incl | 7,560 / 839,204 | 7,021 / 783,010 |
| `ReplicatedMovement` Incl | 695,520 | 649,391 |
| `PacketHeaderAndInfo` Count / Incl | 1,792 / 164,864 | 1,797 / 165,324 |
| `LabResourceNode` Count / Incl | 27 / 1,314 | 145 / 5,945 |
| `NewActor` Count / Incl | 1 / 123 | 113 / 32,637 |

| Insights에서 읽은 값 | 식 | `Connection 0` | `Connection 1` |
| --- | --- | --- | --- |
| 연결당 송신 대역폭(내용 기준) | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위 시간 | 2,897바이트/초 | 2,772바이트/초 |
| 같은 값에 IP와 UDP 헤더를 더한 값 | 위 값 + 28 × 패킷 수 ÷ 선택 범위 시간 | 3,736바이트/초 | 3,611바이트/초 |
| `LabNpc` ÷ `Actor` | | 68.7% | 67.2% |

- **CSV와의 차이.** CSV `out_bytes_per_sec_per_conn`(`r3` 4,475)은 8개 연결의 평균이고, 패킷마다 IP와 UDP 헤더 28바이트를 더한 값이다(`OutTotalBytes += SendBuffer.GetNumBytes() + PacketOverhead`, `Engine/Source/Runtime/Engine/Private/NetConnection.cpp:2562-2586`; `PacketOverhead`는 `UDP_HEADER_SIZE`, `Engine/Plugins/Online/OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Private/IpConnection.cpp:35, 116-122`). 헤더를 더해도 두 연결이 평균보다 작으므로 나머지 연결이 더 많이 받는 것으로 보인다. 나머지 여섯 연결은 읽지 않았다. 기준선은 모든 연결이 같은 액터를 받아서 차이가 3%였다.
- **`Connection 1`은 새 액터를 계속 받는다.** `NewActor` 113번, `LabResourceNode` 145번이다. 움직이는 클라이언트의 컬 거리 안으로 들어오는 액터의 채널이 열린다. `Connection 0`(제자리에서 채집)은 `NewActor` 1번이다. 어느 연결이 어느 클라이언트인지는 연결 순서로 추정했고 확인하지 않았다.
- 비트의 68\~69%가 NPC 이동이다(기준선 76.0%).

## 3. 남은 기법

| 기법 | 단기 구현 | 이 구성에서 겨냥하는 값(`r3`) | 바꿀 코드와 엔진 소스 |
| --- | --- | --- | --- |
| 자원 노드 휴면 | 예(포스팅 3) | 고려 목록의 노드 5,001개. 관련성 판정은 채널이 없는 액터마다 연결별로 하므로(`NetDriver.cpp:5580-5593`) 노드 수 × 연결 수의 판정이 Excl(프레임당 10.80ms)에 남아 있다고 본다. 노드 직렬화 프레임당 1.74ms | 포스팅 1 후보 기법 자료 5절의 "자원 노드 휴면" 줄 |
| NPC 업데이트 빈도 | 예(포스팅 4) | NPC 직렬화 프레임당 0.39ms, NPC 비트 68.7% | 포스팅 1 후보 기법 자료 5절의 "NPC 업데이트 빈도" 줄 |
| 적응형 업데이트 빈도, 푸시 모델, 송신 한도와 우선순위 | 아니오 | 포스팅 1과 같다. `saturated_ratio`는 세 실행 모두 0.000 | 포스팅 1 후보 기법 자료 5절 |

## 4. 확인하지 않은 것

- `GameNetDriver` Excl(프레임당 10.43\~10.80ms) 가운데 관련성 판정과 우선순위 정렬이 차지하는 몫. 기준선과 같은 이유로 이 트레이스로는 나뉘지 않는다.
- `Connection 2`\~`7`의 송신 대역폭과, 연결 번호와 클라이언트 자리의 대응.
- 틱 속도 제한 대기가 실행마다 다른 이유(`frames` 1,304\~1,797).
- `r1`, `r2`의 Network Insights 값.

## 5. 스크린샷 후보

`Scripts/capture-insights.ps1`로 창 내용만 찍었다. 포스팅에 넣을 것은 사용자가 고른다.

- `insights-r3-timers.png`: `r3` 측정 구간, Timers와 `WorldTick` Callees(`GameNetDriver` 76.90%, `LabResourceNode` 13.15%). 1절의 근거다.
- `insights-r3-net-stats.png`: `r3` `Connection 0` `Outgoing` 측정 구간 1,792패킷의 Net Stats(`LabNpc` 839,204비트, `LabResourceNode` 27번). 2절의 근거다.

![r3 측정 구간의 Timers와 WorldTick Callees](images/insights-r3-timers.png)

![r3 Connection 0 Outgoing 측정 구간의 Net Stats](images/insights-r3-net-stats.png)
