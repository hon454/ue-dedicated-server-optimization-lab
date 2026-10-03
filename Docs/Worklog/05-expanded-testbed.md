# 작업 기록: 포스팅 5 테스트베드 확장과 새 기준선 (태스크 15\~21)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다. 지금의 상태는 [STATUS.md](../STATUS.md)에 있다.

## 2026-10-03 태스크 15: 세 기법을 실행 인자로 켜고 끄기 (`tsmall-baseline`, `tsmall-relevancy`, `tsmall-dormancy`, `tsmall-default`)

**2막 태스크 15 완료(2026-10-03).** 1막의 세 기법을 서버 인자로 켜고 끈다(`-LabAlwaysRelevant`, `-LabNoNodeDormancy`, `-LabNpcUpdateFrequency=`. 인자가 없으면 세 기법이 모두 적용된 지금 구성). 수치 CSV에 `config` 열을 더했고, 그 전의 행은 `Saved/LabMetrics/summary-act1.csv`로 옮겼다. 작은 규모에서 네 구성이 종료 코드 0으로 끝났다(`tsmall-baseline`, `tsmall-relevancy`, `tsmall-dormancy`, `tsmall-default`의 `open_actor_channels_per_conn` 118, 10, 7, 7).

## 2026-10-03 태스크 16: 인자가 1막의 구성을 재현하는지 확인 (`toggle1`, `toggle-dormancy`, `toggle-relevancy`, `toggle-baseline`, `toggle-baseline2`)

**2막 태스크 16 완료(2026-10-03). 인자가 1막의 구성을 재현한다.** 인자 없는 확정 규모 `toggle1` 3회: `frames` 1,786\~1,796, `work_avg_ms` 13.757 / 13.565 / 13.581(중앙값 13.581, 변동 폭 0.192), `netflush_avg_ms` 중앙값 9.646(변동 폭 0.205), `out_bytes_per_sec_per_conn` 2,399\~2,402, `config` 열 `default`. `timerfix`와의 중앙값 차이는 `work_avg_ms` 0.097, `netflush_avg_ms` 0.087(9.646 - 9.559)로 `timerfix`의 변동 폭 1.036, 0.800 안이라 구별되지 않는다. 구성별 1회 실행의 `open_actor_channels_per_conn`은 `toggle-dormancy-r1` 20, `toggle-relevancy-r1` 118, `toggle-baseline2-r1` 5,314로 1막(`dormancy6`, `relevancy2`, `baseline3`)과 같다. 이 세 실행의 시간 수치는 비교에 쓰지 않는다. `toggle-baseline-r1`은 측정 시작 2초 뒤에 선호도가 재설정되어 실패했고 수치를 쓰지 않는다. 지금 빌드의 기준 묶음은 `toggle1`이다.

## 2026-10-03 태스크 17: 플레이어 자리 간격 인자 (`tsmall-gather`\~`tsmall-gather8b`, `tsmall-apart`\~`tsmall-apart3`, `tsmall-noarg`\~`tsmall-noarg3`)

**2막 태스크 17 완료(2026-10-03). 플레이어 자리 간격 인자.** 서버 인자 `-LabPlayerSpacing=<m>`(`run-scenario.ps1`의 `-PlayerSpacing`)을 주면 자리를 맵 가운데의 대각선 위에 그 간격으로 놓고, 자리마다 다른 출발 위치와 방향으로 한 변 70m 정사각형을 돈다. 밀집은 3, 분산은 300이고, 인자가 없으면 1막의 배치와 경로다. 값과 계산은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 3.1에 있다. 작은 규모 확인(모두 종료 코드 0): `tsmall-gather8b-r1`(클라이언트 8, 간격 3)은 두 클라이언트의 화면 글자가 t=15, 30, 45초에 `players=8`, `tsmall-apart3-r1`(클라이언트 2, 간격 300)은 `players=1`, `tsmall-noarg3-r1`(인자 없음)은 t=45초의 위치가 바꾸기 전(`tsmall-default2-r1`)과 같은 x=535m, y=290m. 확정 규모에서 인자 없는 실행이 `toggle1`과 구별되지 않는지는 태스크 19.2에서 잰다. `tsmall-gather`, `tsmall-gather2`, `tsmall-gather8`은 고치기 전의 경로이고 `tsmall-apart-r1`은 실패한 실행이다([troubleshooting.md](../Guides/troubleshooting.md)의 "`nodes=5001 npcs=300`" 줄).

## 2026-10-03 태스크 18: 상태 값과 인벤토리 (`tsmall-elem`, `tsmall-noarg4`)

**2막 태스크 18 완료(2026-10-03). 상태 값과 인벤토리.** `-LabStateInterval=<초>`(`-StateInterval`)는 NPC와 플레이어 캐릭터에 프로퍼티 여덟 개의 `ULabStateComponent`를 붙이고, 액터 하나의 값이 평균 그 간격마다 하나씩 바뀐다. `-LabInventoryItems=<칸 수>`(`-InventoryItems`)는 캐릭터에 `ULabInventoryComponent`를 붙여 시드로 채우고, `-LabInventoryChurn=<초>`(`-InventoryChurn`)는 플레이어마다 그 간격으로 맨 앞 칸을 지우고 맨 뒤에 더한다. 인자가 없으면 만들지 않는다. 출발값은 인벤토리 200칸, 4초이고 상태 값의 간격은 정하지 않았다. 보정(태스크 20)에서 정한다([2막 설계](../Planning/2026-10-03-act-2-design.md) 3.2, 3.3). 작은 규모 확인(종료 코드 0): `tsmall-elem-r1`(클라이언트 2, 간격 3m, 상태 2초, 50칸, 4초)의 화면 글자가 `states=2 inventories=2 | own items=50`이고 맨 앞 칸의 번호가 t=15초 851에서 t=45초 446으로 바뀌었다. `tsmall-noarg4-r1`(인자 없음)은 화면 글자가 두 줄 그대로이고 t=45초의 위치가 x=535m, y=290m로 같다.

## 2026-10-03 태스크 19.1: 건축물 (`tsmall-build`, `tsmall-build-apart`, `tsmall-noarg5`)

**2막 태스크 19.1 완료(2026-10-03). 건축물.** `-LabBuildings=<무리 하나의 수>`(`-Buildings`)는 플레이어가 있는 곳마다 `ALabBuilding`을 반지름 80m 원 안에 모아 놓는다(밀집 배치에서는 한 무리, 분산 배치에서는 자리마다 한 무리). `-LabBuildInterval=<초>`(`-BuildInterval`)는 무리마다 그 간격으로 가장 오래된 것을 허물고 새로 짓는다. 건축물은 자원 노드와 같은 기법 인자를 따른다. 값은 보정에서 정한다([2막 설계](../Planning/2026-10-03-act-2-design.md) 3.4). 작은 규모 확인(종료 코드 0): `tsmall-build-r1`(간격 3m, 50개, 2초)은 서버 로그 `clusters=1`, 화면 글자가 t=15, 30, 45초에 `buildings=50`. `tsmall-build-apart-r1`(간격 300m)은 `clusters=2`. `tsmall-noarg5-r1`(인자 없음)은 화면 글자 두 줄, t=45초 위치 x=535m, y=290m로 전과 같다. 이것으로 네 요소의 구현이 끝났다.

## 2026-10-03 태스크 19.2: 인자 없는 실행의 재현 확인 (`reproduce1`, `reproduce2`, `abcheck-old`, `abcheck-old2`, `abcheck-new`, `abcheck-new2`)

**2막 태스크 19.2 완료(2026-10-03). 인자 없는 실행은 지금 빌드에서도 1막의 시나리오다.** 확정 규모에서 `toggle1`을 잰 소스(`4c65b33`)를 다시 빌드한 `abcheck-old2` 3회와, 지금 빌드의 성공한 3회(`abcheck-new-r1`, `abcheck-new-r2`, `abcheck-new2-r1`)를 20:43\~21:02에 연달아 쟀다. `work_avg_ms`는 옛 빌드 13.333 / 12.784 / 12.953(중앙값 12.953, 변동 폭 0.549), 지금 빌드 12.642 / 12.536 / 13.245(중앙값 12.642, 변동 폭 0.709)다. 차이 0.311이 변동 폭보다 작아 구별되지 않는다. `netflush_avg_ms`도 중앙값 9.092와 8.776(변동 폭 0.517, 0.576)으로 구별되지 않고, `open_actor_channels_per_conn`은 모두 20이다. 앞서 잰 지금 빌드의 `reproduce2` 3회(20:25\~20:33, `work_avg_ms` 중앙값 12.459, 변동 폭 0.502)가 `toggle1`(18:28\~18:35, 13.581)과 구별된 것은 코드가 아니라 시간대 차이다. 같은 옛 소스가 두 시간 뒤에는 12.953이었다. 이 저녁의 확정 규모 14회 가운데 4회(`reproduce1-r1`, `abcheck-old-r2`, `abcheck-new-r3`, `abcheck-new2-r2`)가 `FAIL: processor affinity was re-applied`로 끝나 수치를 쓰지 않았다. 묶음은 실패한 실행에서 멈추므로 지금 빌드의 3회는 두 라벨에 걸쳐 있다.

## 2026-10-03 태스크 20.2: 2막 보정 실행 표 (`calib2-a`\~)

보정 실행마다 한 줄씩 더한다. 모두 확정 규모(클라이언트 8, 자원 노드 5,000, 맵 전체의 NPC 300, 준비 구간 30초, 측정 구간 60초)이고 한 번씩 잰 값이라 방향만 본다. 출발값의 계산은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 5.1에 있다.

| 라벨 | 구성 | 요소의 값 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio | 확인한 것 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `calib2-a-r1` | 세 기법 적용, 밀집 3m | 주변 NPC 50, 상태 값 5초, 인벤토리 200칸에 4초, 건축물 500개에 1초 | 1794 | 14.682 | 24.624 | 1 | 10.678 | 16325 | 77 | 0.000 | 종료 코드 0(22:30). 서버 로그 `lab_buildings clusters=1 per_cluster=500`, `lab_npcs_near_players clusters=1 per_cluster=50`. 준비 구간의 `open_actor_channels_per_conn`이 측정 시작 31초 전(13:29:22 UTC)에 78이고 측정 구간에 76\~78. 화면 글자는 t=60초에 `npcs=57 players=8 buildings=500 states=65 inventories=8`, 자기 인벤토리 200칸(1번, 0번 자리). 0번 자리의 3인칭 화면은 건축물에 가렸다(`calib2-a-r1-tpp-03.png`) |
| `calib2-b-r1` | Dormancy까지 적용(NPC 100Hz), 밀집 3m | 같다. 건축물을 경로에서 6m 띄운 배치(`180a75d`)부터다 | 1795 | 20.011 | 28.090 | 1 | 15.580 | 31338 | 77 | 0.000 | 종료 코드 0(23:09). 측정 시작 25초 전부터 채널 77\~78. 화면 글자 t=60초 `npcs=58 players=8 buildings=500 states=66` |
| `calib2-c-r1` | Relevancy만 적용, 밀집 3m | 같다 | 1641 | 35.717 | 53.377 | 968 | 31.261 | 31188 | 695 | 0.000 | 종료 코드 0(23:12). 측정 시작 25초 전부터 채널 693\~697. 화면 글자 `nodes=112 npcs=56 players=8 buildings=500`. 3인칭 화면이 가리지 않는다(`calib2-c-r1-tpp-03.png`) |
| `calib2-d3-r1` | 기준선(Always Relevant, Dormancy 없음, NPC 100Hz), 밀집 3m | 같다 | 325 | 184.837 | 225.137 | 325 | 177.836 | 42269 | 5871 | 0.000 | 종료 코드 0(23:20). 측정 시작 21초 전(14:19:17 UTC)에 채널 5,871에 도달해 더 늘지 않음. `saturated_replications`의 앞 숫자가 측정 구간에 250에서 늘지 않음. 화면 글자 `nodes=5001 npcs=350 players=8 buildings=500 states=358 inventories=8` |

## 2026-10-03 태스크 20.4: `calib2-a-r1`의 Timing Insights 값 (세 기법을 적용한 구성, 출발값)

창을 열지 않고 Insights 내보내기로 읽었다([insights-reading.md](../Guides/insights-reading.md) "에이전트가 직접 열 때 알아 둘 것"). 북마크 시각은 창에서 읽지 않고 프레임 번호로 구했다. 서버 로그에서 `Measuring 60s`가 프레임 `[339]`, CSV 행이 `[133]`, 마지막 줄이 `[134]`이고, `TimingInsights.ExportTimingEvents`로 받은 `Frame` 이벤트가 4,134개라 끝에서 둘째 이벤트가 측정이 끝난 프레임이다. 그 1,794프레임 앞의 이벤트가 시작한 시각 91.5701초와 끝 프레임이 시작한 시각 151.6059초를 `-startTime`, `-endTime`으로 줬다(60.036초). 선택 구간의 `WorldTick` Count가 1,794로 CSV의 `frames`와 같다. 북마크는 그 프레임 안의 어느 시점이라 구간이 한 프레임까지 어긋날 수 있다.

| 타이머 | Count | Incl | 프레임당(Incl ÷ 1,794) | 비고 |
| --- | --- | --- | --- | --- |
| `Frame` | 1,795 | 59.998초 | | |
| `FEngineLoop_UpdateTimeAndHandleMaxTickRate` | 1,794 | 33.181초 | 18.50ms | 틱 속도 제한 대기 |
| `WorldTick` | 1,794 | 26.017초 | 14.50ms | CSV `work_avg_ms` 14.682 |
| `GameNetDriver` | 1,794 | 18.693초 | 10.42ms | CSV `netflush_avg_ms` 10.678(2.4% 차이). Excl 15.717초, 프레임당 8.76ms(84.1%) |
| `LabNpc` | 205,556 | 1.597초 | 0.890ms | 프레임당 114.6번, 한 번 7.77µs |
| `LabNpc` 아래 `LabStateComponent` | 205,556 | 0.335초 | 0.187ms | 한 번 1.63µs |
| `LabCharacter` | 88,552 | 1.000초 | 0.557ms | 프레임당 49.4번, 한 번 11.29µs |
| `LabCharacter` 아래 `LabInventoryComponent` | 88,552 | 0.232초 | 0.129ms | 한 번 2.62µs |
| `LabCharacter` 아래 `LabStateComponent` | 88,552 | 0.138초 | 0.077ms | 한 번 1.56µs |
| `LabBuilding` | 1,698 | 0.019초 | 0.011ms | Dormant 상태. 짓고 허물 때만 처리 |
| `LabResourceNode` | 791 | 0.014초 | 0.008ms | |
| `TickCompletionEvents` | 7,176 | 3.965초 | 2.21ms | 액터 틱 |
| `UNetConnection_ReceivedPacket` | 11,658 | 2.298초 | 1.28ms | 클라이언트가 보낸 패킷 처리 |

- 액터 클래스 타이머는 `WorldTick`의 Callees에서 `GameNetDriver` 아래의 값이다(`TimingInsights.ExportTimerCallees -timers=WorldTick -threads=GameThread`).
- `LabStateComponent`의 합은 프레임당 0.264ms(0.187 + 0.077)다. 오늘 잰 `work_avg_ms`의 변동 폭 0.5\~0.7ms보다 작다. 출발값의 예상(많아도 0.55ms, 설계 5.1)대로 Push Model의 재료로는 모자란다.
- `LabNpc`는 예상 1.61ms보다 작은 0.890ms다. 처리 횟수가 프레임당 114.6번으로 예상 148번(55.5 × 8 ÷ 3)보다 적고, 한 번의 시간도 1막의 10.91µs(`update-frequency3-r3`)보다 짧다. 이유는 확인하지 않았다.
- 비트 수(Networking Insights)는 창을 열어야 해서 아직 읽지 않았다.

## 2026-10-03 태스크 20.2\~20.3: 네 구성의 보정 실행과 기준선의 네 조건 (`calib2-a`\~`calib2-d3`)

- 실패한 실행: 기준선 `calib2-d-r1`(23:14, 측정 시작 5초 뒤)과 `calib2-d2-r1`(23:17, 8초 뒤)이 `FAIL: processor affinity was re-applied`로 끝나 수치를 쓰지 않는다. 세 번째 `calib2-d3-r1`이 성공했다. 작은 규모의 `tsmall-calib-r1`, `tsmall-clear-r1`도 같은 실패였다(동작 확인용이라 화면만 썼다).
- 구성 사이의 차이(한 번씩 잰 값이고 `calib2-a-r1`은 40분 전에 건축물을 고치기 전의 배치로 쟀다. 방향만 본다): 기준선 184.837 → Relevancy 35.717(-149.120) → Dormancy 20.011(-15.706) → Net Update Frequency 14.682(-5.329). 오늘 잰 `work_avg_ms`의 변동 폭은 0.5\~0.7ms다.
- Relevancy만 적용한 구성은 틱 예산을 넘는다(`over_budget_frames` 968 ÷ `frames` 1,641 = 59%). 출발값의 예상은 건축물 10.4ms였고(설계 5.1), Dormancy가 줄인 15.7ms에는 자원 노드의 몫(1막에서 약 2ms)이 함께 들어 있다.
- 송신량: Net Update Frequency가 31,338에서 16,325로 줄였다(-47.9%). 기준선의 30Hz 환산 송신량은 42,269 × 30 ÷ (325 ÷ 60) = 234,105바이트/초로 지금 한도 350,000의 67%다.

기준선의 네 조건(`calib2-d3-r1`, 출발값):

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | 예 | 측정 시작(14:19:38 UTC) 21초 전의 줄(14:19:17)에서 `open_actor_channels_per_conn`이 5,871이고 더 늘지 않음 |
| 지속적인 예산 초과 | 예 | `over_budget_frames` 325 = `frames` 325. `work_avg_ms` 184.837은 틱 예산 33.3ms의 5.5배 |
| 가장 큰 비용이 네트워크 | CSV로는 예. Insights는 아직 읽지 않았다 | `netflush_avg_ms` 177.836 ÷ `work_avg_ms` 184.837 = 96.2%. 판단은 사용자가 한다 |
| 송신 한도에 포화되지 않음 | 예 | `saturated_ratio` 0.000. 측정 구간에 `saturated_replications`의 앞 숫자가 250에서 늘지 않음 |
