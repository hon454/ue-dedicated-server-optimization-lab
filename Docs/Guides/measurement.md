# 측정 규약

모든 포스팅이 같은 시나리오, 같은 절차, 같은 지표로 잰다. 확정한 값과 그 근거는 [STATUS.md](../STATUS.md)의 "확정할 값"과 "기준선 조건"에, 실행 명령은 "명령"에 있다. 결정의 이유와 버린 대안은 [ADR](../Decisions/README.md)에 있다.

이 문서는 [단기 설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md) 8절과 [단기 구현 계획](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-implementation-plan.md)의 "수치의 이름과 출처"를 옮긴 것이다(2026-10-03).

## 시나리오

확정 규모는 클라이언트 8개, 자원 노드 5,000개와 검증용 1개, AI NPC 300명, 준비 구간 30초, 측정 구간 60초다(STATUS.md "확정할 값"). 1막(포스팅 0\~4)의 규모이고 1막 안에서는 바꾸지 않았다. 2막의 규모는 테스트베드를 확장한 뒤 따로 확정한다([2막 구현 계획](../Planning/2026-10-03-act-2-implementation-plan.md) 태스크 20, [ADR-0014](../Decisions/0014-act-2-testbed-expansion.md)). 아래 표는 1막의 값이다.

2막의 확장 요소는 서버 인자로 켠다. 인자를 주지 않은 실행은 1막의 시나리오다. 플레이어 자리 간격(`-PlayerSpacing`), 상태 값(`-StateInterval`), 인벤토리(`-InventoryItems`, `-InventoryChurn`), 건축물(`-Buildings`, `-BuildInterval`)이고, 뜻과 배치 방법은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 3절에 있다. 값은 태스크 20에서 확정해 STATUS.md "확정할 값"에 적는다. 이 요소들도 고정 시드와 공통 시작 신호를 따른다. 값을 바꾸는 타이머는 시작 신호에서 출발하고, 자원 노드와 NPC의 배치에 쓰는 난수와 따로 쓰는 시드로 고른다. 요소를 켠 실행에서는 클라이언트 화면 글자에 셋째 줄(`buildings=`, `states=`, `inventories=`, 자기 인벤토리의 칸 수와 맨 앞 칸의 번호)이 생긴다.

| 항목 | 값 | 근거 |
| --- | --- | --- |
| 서버 틱 | 30Hz (틱 예산 33.3ms) | 엔진 소스: `NetServerMaxTickRate` 기본값 30(`Engine/Config/BaseEngine.ini:1867`). 계산값: 1000ms ÷ 30 = 33.3ms |
| 맵 크기 | 바닥 2km × 2km, 배치 영역 1.9km × 1.9km | 계산값: 엔진 기본 컬 거리(150m)의 원 면적 π × 150² ≈ 70,700m²는 배치 영역 3,610,000m²의 약 2.0%다 |
| 자원 노드 | 5,000개 + 검증용 1개 | 계산값: 균등 배치에서 가장자리가 아닌 위치의 컬 거리 안 기대 개수는 5,000 × 70,700 ÷ 3,610,000 ≈ 98개다 |

기준선 조건. 규모를 확정할 때 다음을 모두 확인했다(결과는 STATUS.md "기준선 조건").

- **초기 전송 완료**: 측정 시작 전에 연결당 열린 액터 채널 수가 전체 액터 수 근처에서 멈춘다.
- **지속적인 예산 초과**: 측정 구간 프레임의 절반 이상이 틱 예산을 넘는다. P99만 넘는 것은 해당하지 않는다.
- **가장 큰 비용이 네트워크**: Insights에서 확인한다.
- **송신 한도에 포화되지 않는다**: 포화되면 기준선의 처리 비용이 실제보다 작게 나온다. 기준선이 포화되어 연결당 송신 한도를 350,000바이트/초로 올려 고정했다([ADR-0007](../Decisions/0007-raise-send-limit-once.md)). 기본 한도에서의 포화와 우선순위는 [backlog.md](../backlog.md)에 있다.

재현성([ADR-0008](../Decisions/0008-reproducible-runs.md)):

- 노드와 NPC 배치는 고정 시드로 생성한다.
- 클라이언트는 실행 인자로 자리 번호를 받는다. 접속 순서에 따라 위치가 달라지지 않는다.
- 모든 클라이언트가 준비되면 서버가 공통 시작 신호를 낸다. 클라이언트의 이동과 채집, NPC의 배회, 자동 스크린샷의 시계가 모두 이 신호에서 출발한다.
- 클라이언트는 고정 경유점을 따라 자동 이동한다.
- 클라이언트 하나는 제자리에서 검증용 노드를 반복 채집해 약 26초 주기의 상태 변화를 만든다(2초 간격 3회에 고갈, 20초 뒤 재생).
- 클라이언트는 저해상도 창과 30fps 제한으로 실행한다.
- 서버는 논리 프로세서 2\~7(마스크 252)에 고정하고, 클라이언트는 나머지를 쓴다([ADR-0009](../Decisions/0009-server-cores-without-dpc-load.md)).
- 서버 프로세스의 타이머 스로틀을 끈다([ADR-0012](../Decisions/0012-server-timer-resolution.md)).
- 스크립트 하나(`Scripts/run-scenario.ps1`)가 서버와 클라이언트 전부를 띄운다.

## 절차

| 항목 | 값 | 근거 |
| --- | --- | --- |
| 준비 구간 | 접속 완료 후 30초를 버림 | 접속 직후에는 모든 액터의 초기 스폰 데이터가 한꺼번에 전송되어 정상 상태와 다르다. 측정값: `calib-f-r1`에서 측정 시작 22초 전에 열린 액터 채널 수가 더 늘지 않았다 |
| 측정 구간 | 60초 | 서버가 30Hz를 유지하면 1,800프레임이다. 유지하지 못하면 그보다 적으므로 실제 프레임 수를 함께 적는다. P99는 99백분위 경계값이며 느린 1%의 평균이 아니다 |
| 반복 | 구성마다 3회, 중앙값 기록 | 한 번의 이상치를 걸러 낸다. 지속적인 경합이나 실행 순서의 영향까지 없애지는 못한다. 세 값과 변동 폭(최댓값 - 최솟값)을 모두 적는다 |
| 차이의 판단 | 중앙값의 변화가 두 구성의 변동 폭 중 큰 쪽보다 클 때만 차이가 있다고 적음 | 그보다 작으면 "구별하지 못했다"고 적고 두 구성을 한 번 더 측정한다 |
| 비교 조건 | 같은 화면 조건에서 연달아 잰 묶음끼리 비교 | 코드가 같은 `dormancy2`(원격 데스크톱 화면)와 `dormancy6`(본체 화면)의 `work_avg_ms` 중앙값이 1.066 달랐다 |
| 실패 처리 | 시작 신호 이후(준비 구간 포함) 연결 수가 바뀌면 그 실행은 버림 | 연결이 빠지면 송신 바이트 합계와 서버 부하가 함께 줄어 최적화 효과로 잘못 읽게 된다. 서버가 CSV를 쓰지 않고 종료 코드 1로 끝나고, 스크립트가 실패로 처리한다 |

서버 실행 시 CPU와 네트워크 트레이스 채널을 켜서 Unreal Insights로 수집한다. 트레이스 인자는 [engine-notes.md](../Reference/engine-notes.md) 바절 "트레이스 인자"에 있다.

## 수치의 이름과 출처

같은 이름으로 다른 값을 부르지 않는다([ADR-0004](../Decisions/0004-insights-and-csv-metrics.md)).

| 이름 | 정의 | 출처 | 쓰는 곳 |
| --- | --- | --- | --- |
| `work` | 월드 틱 시작부터 프레임 끝까지의 경과 시간. CPU 실행 시간이 아니라 경과 시간이라서 그 구간의 스레드 대기와 OS 스케줄링 지연이 들어간다 | CSV | `Docs/STATUS.md`, 에이전트의 전후 비교 |
| `netflush` | 액터 틱 종료부터 프레임 끝까지의 경과 시간. 리플리케이션 전용 시간이 아니다 | CSV | `Docs/STATUS.md`, 보조 지표 |
| 서버 프레임 시간 | Timing Insights의 프레임 시간에서 틱 속도 제한 대기(`FEngineLoop_UpdateTimeAndHandleMaxTickRate`)를 뺀 시간. 평균은 (측정 구간 길이 − 구간 안의 대기 Incl) ÷ `Frame` Count, P99는 프레임마다 (`Frame` − 대기)의 99백분위 경계값([ADR-0010](../Decisions/0010-frame-time-without-tick-wait.md)) | Insights | 포스팅과 README의 표 |
| 리플리케이션 시간 | Insights 타이머 `GameNetDriver`의 프레임당 Incl(선택 구간의 Incl ÷ `WorldTick`의 Count). 규모를 확정할 때 사용자가 골랐고(2026-10-01) 이후 바꾸지 않는다 | Insights | 포스팅과 README의 표 |
| 연결당 송신 대역폭 | 측정 구간의 연결당 초당 송신 바이트 | CSV와 Network Insights | 둘 다 |
| 연결당 열린 액터 채널 수 | 연결 하나에 열려 있는 액터 채널 수. Dormant 상태의 액터는 채널이 닫히므로 "클라이언트에 존재하는 액터 수"와 다르다 | CSV | 둘 다 |
| 클라이언트에 존재하는 액터 수 | 클라이언트 화면 위 글자의 노드 수와 NPC 수 | 스크린샷 | 포스팅 |
| `saturated_ratio` | 측정 구간에 모든 연결에서 송신 한도 때문에 중간에 끊긴 리플리케이션 횟수 ÷ 리플리케이션 시도 횟수. 엔진이 `ServerReplicateActors`에서 연결마다 남기는 기록(`UNetConnection::GetSaturationAnalytics`)의 차다([ADR-0006](../Decisions/0006-saturation-from-engine-analytics.md)). 그 전의 `smoke` 행은 프레임 끝의 `IsNetReady()`로 잰 값이다. 포화 판정 기준은 0.01 이상이다 | CSV | `Docs/STATUS.md`, 해석의 전제 |

수치 CSV의 `config` 열은 그 행을 잰 구성이다. 서버 인자 가운데 기본값과 다른 것을 `;`로 이은 문자열이고(예: `AlwaysRelevant;NoNodeDormancy;NpcUpdateFrequency=100`), 인자를 주지 않은 실행은 `default`다. 2막의 확장 요소도 이 열에 적힌다(플레이어 자리 간격은 `PlayerSpacing=3`). 서버 로그의 `lab_config=` 줄에 같은 값이 남는다. 이 열을 더하기 전(2026-10-03, 2막 구현 계획 태스크 15)의 행은 `Saved/LabMetrics/summary-act1.csv`에 있고, 그 행의 구성은 라벨과 그때의 빌드로 정해진다.

`Docs/STATUS.md`의 측정 결과 표에는 CSV 값을 CSV 열 이름 그대로 적는다. 포스팅과 README의 표에는 Insights에서 읽은 값을 쓴다. 서버는 측정 구간의 시작과 끝에 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 트레이스에 남기므로, Insights에서 이 두 북마크 사이를 본다. 읽는 순서는 [insights-reading.md](insights-reading.md)에 있다.

## 지표 표

모든 포스팅이 같은 표를 쓴다.

| 지표 | 정의와 출처 | 보는 이유 |
| --- | --- | --- |
| 서버 프레임 시간 (평균, P99) | Timing Insights. 위 "수치의 이름과 출처" | 틱 예산 33.3ms 대비 여유 |
| 리플리케이션 시간 (프레임당) | Timing Insights의 `GameNetDriver` | 프레임 시간 중 리플리케이션이 차지하는 몫 |
| 연결당 송신 대역폭 | Network Insights. CSV 값과 대조 | 클라이언트 회선 부담과 서버 송신 비용 |
| 연결당 열린 액터 채널 수 | CSV | 서버가 연결마다 유지하는 리플리케이션 대상의 수 |
| 클라이언트에 존재하는 액터 수 | 클라이언트 화면 위 글자 | 클라이언트가 실제로 가진 것 |

## 한계

테스트베드 포스팅과 README에 명시한다.

- 에디터 빌드 실행 파일을 쿠킹 없이 사용하므로 절대 수치는 출시 빌드와 다르다([ADR-0002](../Decisions/0002-editor-build-without-packaging.md)).
- 서버와 클라이언트가 같은 PC([pc-specs.md](../Reference/pc-specs.md))에서 돈다. 서로 다른 코어에 고정하지만 캐시와 메모리 대역폭 경합은 남는다. 네트워크는 루프백이다.
- 수치는 같은 조건의 전후 비교로만 해석한다. "서버 코드만의 CPU 개선률"이라고 쓰지 않는다.
- 기준선은 엔진의 기본 관련성 판정을 의도적으로 끄고([ADR-0003](../Decisions/0003-lawless-baseline.md)) 연결당 송신 한도를 올린 인위적인 출발점이다.
- AI NPC는 움직이는 리플리케이트 액터의 대역이다. 길 찾기나 행동 트리 같은 AI 비용은 측정하지 않는다.
- 채집 RPC는 자동 실험용이다. 서버가 대상 탐색과 거리 검사, 호출 간격 제한을 하지만 그 밖의 악의적 호출은 막지 않는다.
