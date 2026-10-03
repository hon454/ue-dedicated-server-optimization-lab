# 포스팅 5 관찰 자료 (`act2-baseline1`)

2막 구현 계획 태스크 21.2의 자료다. 에이전트가 2막 기준선 `act2-baseline1`의 트레이스를 창 없이 Unreal Insights 내보내기 명령으로 읽었다([insights-reading.md](../../Docs/Guides/insights-reading.md) "에이전트가 직접 열 때 알아 둘 것"). 이 포스팅은 기법을 고르지 않는다. "관찰"은 사용자가 판단하고(구현 계획 태스크 21.3), 에이전트는 그 판단을 옮겨 초안을 쓴다.

- 대상 실행: `act2-baseline1-r1`\~`r3`(2026-10-04 00:47\~00:55, 본체 화면, `0f689c2` 소스의 빌드). 확정 명령(클라이언트 8, 자원 노드 5,000, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 마스크 252, 송신 한도 350,000, 트레이스 켬)에 2막의 확정값 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`과 기준선 인자 `-AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100`을 더했다. CSV 수치와 기준선의 네 조건은 [STATUS.md](../../Docs/STATUS.md)의 "측정 결과"와 "기준선 조건"에 있다. `work_avg_ms`의 중앙값 실행은 `r2`(215.801)다.
- 1\~4절은 내보낸 값과 그 값으로 계산한 값이다. 계산에는 식을 적었다. 7절 "에이전트 의견"만 해석이다.
- 프레임당 값은 Incl ÷ `WorldTick` Count다. "한 번"은 Incl ÷ Count(연결 하나가 액터 하나를 한 프레임에 처리하는 것)다.

## 1. 측정 구간

북마크를 창에서 읽지 않고 서버 로그의 프레임 번호로 구했다. `Measuring 60s` 줄의 프레임이 측정을 시작한 프레임, CSV 행을 찍은 줄의 프레임이 끝난 프레임이다. `TimingInsights.ExportTimingEvents`로 받은 `Frame` 이벤트의 마지막이 로그의 마지막 프레임이다.

| 항목 | `r1` | `r2` | `r3` |
| --- | --- | --- | --- |
| 로그의 시작, 끝, 마지막 프레임(1,000으로 나눈 나머지) | 77, 356, 357 | 164, 442, 443 | 165, 437, 438 |
| `Frame` 이벤트 수 | 1,357 | 1,443 | 1,438 |
| `-startTime`(시작 프레임이 시작한 시각) | 93.1792초 | 92.2855초 | 90.7916초 |
| `-endTime`(끝 프레임이 시작한 시각) | 153.3610초 | 152.3626초 | 151.0451초 |
| 선택 구간 | 60.182초 | 60.077초 | 60.253초 |
| 선택 구간의 `WorldTick` Count | 279 | 278 | 272 |
| CSV `frames` | 279 | 278 | 272 |

세 실행 모두 `WorldTick` Count가 CSV `frames`와 같다. 북마크는 그 프레임 안의 어느 시점이라 구간이 한 프레임까지 어긋날 수 있다.

## 2. 서버 프레임과 CSV 대조 (Timing Insights)

| 값 | 식 | `r1` | `r2` | `r3` | 대조할 CSV 열(`r1` / `r2` / `r3`) | 차이 |
| --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 | 측정 구간의 프레임마다 (`Frame` − 그 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate`)의 평균([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)) | 215.669ms | 216.067ms | 221.482ms | `work_avg_ms` 215.238 / 215.801 / 220.521 | +0.20% / +0.12% / +0.44% |
| 서버 프레임 시간 P99 | 같은 값을 정렬한 ceil(N × 0.99)번째(N = 279 / 278 / 272) | 261.850ms | 271.026ms | 331.761ms | `work_p99_ms` 261.636 / 270.781 / 331.528 | +0.08% / +0.09% / +0.07% |
| 리플리케이션 시간 | `GameNetDriver` Incl ÷ `WorldTick` Count | 206.238ms | 206.441ms | 211.538ms | `netflush_avg_ms` 206.558 / 206.944 / 211.365 | -0.15% / -0.24% / +0.08% |
| `GameNetDriver`의 `WorldTick` 대비 | `GameNetDriver` Incl ÷ `WorldTick` Incl | 95.85% | 95.79% | 95.74% | `netflush_avg_ms` ÷ `work_avg_ms` 96.0% / 95.9% / 95.8% | |

틱 속도 제한 대기는 프레임당 0.015ms(`r2`, Incl 0.004초 ÷ 278)다. 서버가 틱 예산을 계속 넘어 기다리지 않는다.

## 3. `GameNetDriver` 안의 내역

`TimingInsights.ExportTimerCallees -timers=WorldTick -threads=GameThread`로 받은 값에서 `GameNetDriver` 아래의 클래스 타이머다.

| 타이머 | 프레임당 처리 횟수 | 처리 횟수의 식 | 프레임당 Incl(`r1` / `r2` / `r3`) | 한 번(`r2`) | `GameNetDriver` 대비(`r2`) |
| --- | --- | --- | --- | --- | --- |
| `GameNetDriver` | 1 | | 206.238 / 206.441 / 211.538ms | | 100% |
| `GameNetDriver` Excl | | | 70.722 / 71.695 / 73.004ms | | 34.7% |
| `LabResourceNode` | 40,008 | 5,001 × 8 | 94.330 / 93.156 / 95.479ms | 2.33µs | 45.1% |
| `LabNpc` | 2,800 | 350 × 8(맵 전체 300 + 주변 50) | 28.556 / 29.067 / 30.158ms | 10.38µs | 14.1% |
| `LabNpc` 아래 `LabStateComponent` | 2,800 | | 5.882 / 6.188 / 6.532ms | 2.21µs | (`LabNpc`에 포함) |
| `LabBuilding` | 4,000 | 500 × 8 | 9.865 / 9.769 / 9.981ms | 2.44µs | 4.7% |
| `LabCharacter` | 64 | 8 × 8 | 1.960 / 1.943 / 2.076ms | 30.36µs | 0.9% |
| `LabCharacter` 아래 `LabInventoryComponent` | 64 | | 0.630 / 0.626 / 0.672ms | 9.78µs | (`LabCharacter`에 포함) |
| `LabCharacter` 아래 `LabStateComponent` | 64 | | 0.178 / 0.178 / 0.192ms | 2.79µs | (`LabCharacter`에 포함) |
| 나머지 여섯(`ClientMoveResponsePacked`, `LabPlayerController`, `GameStateBase`, `GameplayDebuggerCategoryReplicator`, `PlayerState`, `WorldSettings`) | 8\~13 | | 합 0.811ms(`r2`) | | 0.4% |

- 처리 횟수가 식과 정확히 같다(`r2`: `LabResourceNode` 11,122,224 = 5,001 × 8 × 278, `LabNpc` 778,400 = 350 × 8 × 278, `LabBuilding` 1,112,000 = 500 × 8 × 278). Always Relevant라 모든 자원 노드, NPC, 건축물이 모든 연결에 대해 매 프레임 한 번씩 처리된다.
- `LabStateComponent`의 합은 프레임당 6.366ms(6.188 + 0.178, `r2`)다. 기준선에서는 NPC 350개가 매 프레임 처리되어 세 기법을 적용한 구성(0.264ms, `calib2-a-r1`)보다 크다.
- `GameNetDriver` 밖의 큰 타이머: `UNetConnection_ReceivedPacket` 프레임당 5.345 / 5.501 / 5.738ms, `TickCompletionEvents`(액터 틱) 2.388 / 2.391 / 2.442ms. `WorldTick` 안이다.

## 4. 1막 기준선과의 구성 비교 (`baseline3-r1`)

시각이 다른 묶음이라 절댓값의 차이는 비교에 쓰지 않는다(STATUS.md "명령"). 처리 횟수와 한 번의 시간, 비율만 본다. 1막의 값은 [기준선 관찰 자료](../01-baseline/candidates.md) 1절이다.

| 항목 | 1막 `baseline3-r1` | 2막 `act2-baseline1-r2` | 식 |
| --- | --- | --- | --- |
| `GameNetDriver` 프레임당 | 188.97ms | 206.441ms | 1막: 57.07초 ÷ 302 |
| `GameNetDriver` Excl 프레임당 | 69.90ms | 71.695ms | 1막: 21.11초 ÷ 302 |
| `LabResourceNode` 프레임당 / 한 번 / 비율 | 98.51ms / 2.46µs / 52.1% | 93.156ms / 2.33µs / 45.1% | 1막: 29.75초 ÷ 302, 29.75초 ÷ 12,082,416 |
| `LabNpc` 프레임당 / 한 번 / 비율 | 19.47ms / 8.11µs / 10.3% | 29.067ms / 10.38µs / 14.1% | 1막: 5.88초 ÷ 302, 5.88초 ÷ 724,800 |
| `LabBuilding` | 없음 | 9.769ms / 2.44µs / 4.7% | |
| `LabCharacter` | 1막에도 있음(나머지 일곱 개의 합이 0.5% 안팎) | 1.943ms / 30.36µs / 0.9% | |

- **설계의 예상과 맞는 것(계산값).** [2막 설계](../../Docs/Planning/2026-10-03-act-2-design.md) 5.1은 기준선에 건축물 9.85ms(500 × 8 × 2.46µs)와 NPC 3.3ms(50 × 8 × 8.13µs)가 더해진다고 예상했다. 건축물은 9.769ms다. `LabNpc`의 Excl(상태 값 컴포넌트를 뺀 몫)은 22.880ms로 1막의 `LabNpc` 19.47ms보다 3.41ms 크다.
- **NPC 한 번이 길어진 몫은 상태 값 컴포넌트와 같은 크기다(계산값).** 10.38 − 2.21 = 8.17µs로 1막의 8.11µs와 거의 같다. 1막에는 상태 값 컴포넌트가 없었다.
- 설계 5.1은 기준선 전체를 "210ms를 넘는다"고 예상했다. `work_avg_ms` 중앙값은 215.801이다.

## 5. 밀집과 분산 (20.4a의 묶음을 그대로 쓴다)

사용자 결정(2026-10-04)으로 분산 배치는 새로 재지 않았다. 같은 빌드로 세 기법을 적용한 구성을 번갈아 3회씩 잰 `layout-dense`, `layout-apart`를 포스팅 5의 비교에 쓴다. 표와 경위는 [Worklog/05-expanded-testbed.md](../../Docs/Worklog/05-expanded-testbed.md)의 "태스크 20.4a"에 있다.

| 배치 | `work_avg_ms` 중앙값(변동 폭) | `out_bytes_per_sec_per_conn` 중앙값(변동 폭) | `open_actor_channels_per_conn` | 화면 글자(1번 클라이언트, t=60초) |
| --- | --- | --- | --- | --- |
| 밀집 3m(`layout-dense1`, `2`, `4`) | 16.779(0.065) | 16,575(52) | 77 | `nodes=176 npcs=58 players=8 buildings=500`(`layout-dense4-r1`) |
| 분산 300m(`layout-apart1`, `7`, `8`) | 27.347(0.281) | 9,743(55) | 69 | `nodes=253 npcs=58 players=1 buildings=934`(`layout-apart8-r1`) |

- 분산 배치는 무리가 여덟 개라 월드의 주변 NPC가 400개, 건축물이 4,000개다. 연결 하나가 받는 수는 비슷해도 서버가 가진 수가 다르다. 어느 쪽이 얼마를 더했는지는 Insights에서 읽지 않았다.
- 분산 배치의 `buildings=934`는 시작 신호 전에 맵 가운데에서 받은 다른 무리의 건축물이 Dormant 상태로 남은 것이다(troubleshooting.md, 정상 동작).
- 기준선을 분산으로 재지 않은 이유: Always Relevant에서는 배치와 상관없이 모든 연결이 모든 액터를 받는다. 분산 기준선과 밀집 기준선의 차이는 월드의 액터 수에서만 나온다.

## 6. 연결 하나의 패킷 (Networking Insights)

**아직 읽지 않았다.** 비트 수는 Networking Insights 창을 열어야 하고, 컴퓨터 조작 권한(`G:\Epic Games\UE_Source\Engine\Binaries\Win64\UnrealInsights.exe`)이 필요하다. 읽을 것은 `act2-baseline1-r2`의 `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`에서 측정 구간(92.2855\~152.3626초)의 `Actor`, `LabResourceNode`, `LabNpc`, `LabBuilding`, `BP_LabCharacter_C`, `LabInventoryComponent`, `LabStateComponent`, `ReplicatedMovement`, `PacketHeaderAndInfo`의 Count와 Incl이다. 세 기법을 적용한 구성의 같은 표는 Worklog "태스크 20.4: `layout-dense1-r1`의 Networking Insights 값"에 있다.

## 7. 에이전트 의견

- **가장 큰 비용은 1막과 같이 네트워크이고, 그 안에서 자원 노드가 가장 크다.** `GameNetDriver`가 `WorldTick`의 95.8%이고, 그 가운데 `LabResourceNode`가 45.1%, Excl이 34.7%다. 1막(52.1%, 37%)과 순서가 같다. 새 요소 가운데 기준선에서 가장 큰 것은 상태 값 컴포넌트를 포함한 NPC(14.1%)이고, 건축물(4.7%)이 그다음이다.
- **2막의 새 요소는 기준선에서는 작고, 뒤의 구성에서 몫이 커진다.** 기준선에서 건축물과 주변 NPC를 합쳐도 `GameNetDriver`의 약 1할이다(9.769 + 상태 값을 뺀 NPC 증가분 3.41 = 13.2ms, 6.4%). 보정에서 Relevancy만 적용한 구성은 `work_avg_ms`가 35.717로 틱 예산을 넘었고(`calib2-c-r1`), Dormancy가 15.706을 줄였다(`calib2-b-r1`). 출발값의 예상으로는 그 가운데 건축물이 약 10.4ms다(설계 5.1, 그 트레이스는 읽지 않았다). 포스팅 5의 "관찰"은 기준선의 구성보다 "무엇을 넣었고 기준선에서 얼마인지"를 보이고, 각 요소가 겨냥하는 기법은 포스팅 6 이후로 넘기는 편이 맞아 보인다.
- **상태 값 컴포넌트는 기준선에서만 크다.** 6.366ms는 NPC 350개가 매 프레임 처리되기 때문이다. 세 기법을 적용한 구성에서는 0.264ms라 Push Model 글의 재료로는 여전히 작다(STATUS.md "확정할 값").
- **변동.** `r3`만 `work_p99_ms`가 331.528로 높고 클래스 타이머의 한 번도 2\~4% 길다(`LabResourceNode` 2.39µs와 `r2` 2.33µs, `LabNpc` 10.77µs와 10.38µs). 실행 전체가 조금 느렸던 것으로 보인다.

## 8. 확인하지 않은 것

- Networking Insights의 비트 수(6절).
- 분산 배치에서 서버의 추가 비용이 어디에서 나오는지(5절).
- 보정의 `calib2-d3-r1`(184.837)보다 `act2-baseline1`이 30.964 높은 이유. 약 1.5시간 떨어졌고 건축물 배치와 노드 하나를 고친 빌드다. `LabResourceNode` 한 번이 2.02µs에서 2.33µs로 1.15배라(같은 처리 횟수 40,008), 77분 사이에 모든 타이머가 약 1.2배 달라진 일(`calib2-e2-r1`)과 같은 종류로 보이지만 연달아 재서 가르지 않았다.

## 9. 시각 자료 후보

자동 스크린샷에서 골랐다. 포스팅에 넣을 것은 사용자가 고른다. 아직 `images/`에 복사하지 않았다.

| 후보 | 파일(`Saved/Screenshots/Lab/`) | 보여 주는 것 |
| --- | --- | --- |
| 기준선 내려다보기 | `act2-baseline1-r2-topdown-03.png` | t=60초. 밀집 배치의 무리 하나(건축물 노란 점, NPC 빨간 점, 플레이어 파란 점)와 맵 전체의 자원 노드(초록 점). 화면 글자 `nodes=5001 npcs=350 players=8 buildings=500 states=358 inventories=8`로 Always Relevant를 보여 준다 |
| 기준선 3인칭 | `act2-baseline1-r2-tpp-03.png` | 0번 자리(채집 담당)의 3인칭. 건축물(주황 상자)과 다른 플레이어가 보이고 가리는 것이 없다 |
| 세 기법 적용, 밀집 | `layout-dense4-r1-topdown-03.png` | 기준선 내려다보기와 나란히 놓으면 같은 장면에서 `nodes=5001`이 `nodes=176`이 된다 |
| 세 기법 적용, 분산 | `layout-apart8-r1-topdown-03.png` | 밀집과 나란히 놓으면 `players=8`이 `players=1`이 된다. 다른 플레이어의 파란 점이 없다 |

- Insights 캡처(Timers 패널, `GameNetDriver` 아래의 Callees, Networking의 Net Stats)는 창을 열어야 해서 찍지 않았다. 1막 기준선 글의 `insights-r1-timers.png`, `insights-r1-net-stats.png`에 해당하는 것이다.
- 영상은 찍지 않았다. 찍는다면 다음 라벨은 `visual13`이고, 찍기 전에 허가를 받는다.
