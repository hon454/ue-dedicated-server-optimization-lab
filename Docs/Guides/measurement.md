# 측정 규약

모든 포스팅이 같은 시나리오, 같은 절차, 같은 지표로 잰다. 확정한 값과 그 근거는 아래 "확정값과 기준선 조건"에, 실행 명령은 [STATUS.md](../STATUS.md)의 "명령"과 [commands.md](commands.md)에 있다. 결정의 이유와 버린 대안은 [ADR](../Decisions/README.md)에 있다.

이 문서는 [단기 설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md) 8절과 [단기 구현 계획](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-implementation-plan.md)의 "수치의 이름과 출처"를 옮긴 것이다(2026-10-03).

## 시나리오

확정 규모는 클라이언트 8개, 자원 노드 5,000개와 검증용 1개, AI NPC 300명, 워밍업 구간 30초, 측정 구간 60초다(아래 "확정값"). 1막(포스팅 0\~4)의 규모이고 1막 안에서는 바꾸지 않았다. 2막의 규모는 테스트베드를 확장한 뒤 따로 확정한다([2막 구현 계획](../Planning/2026-10-03-act-2-implementation-plan.md) 태스크 20, [ADR-0014](../Decisions/0014-act-2-testbed-expansion.md)). 아래 표는 1막의 값이다.

2막의 확장 요소는 서버 인자로 켠다. 인자를 주지 않은 실행은 1막의 시나리오다. 플레이어 자리 간격(`-PlayerSpacing`), 상태 값(`-StateInterval`), 인벤토리(`-InventoryItems`, `-InventoryChurn`), 건축물(`-Buildings`, `-BuildInterval`), 플레이어 주변의 NPC(`-NpcsNearPlayers`)이고, 뜻과 배치 방법은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 3절에 있다. 값은 태스크 20에서 확정해 아래 "확정값"에 적었다. 이 요소들도 고정 시드와 공통 시작 신호를 따른다. 값을 바꾸는 타이머는 시작 신호에서 출발하고, 자원 노드와 NPC의 배치에 쓰는 난수와 따로 쓰는 시드로 고른다. 요소를 켠 실행에서는 클라이언트 화면 글자에 셋째 줄(`buildings=`, `states=`, `inventories=`, 자기 인벤토리의 칸 수와 맨 앞 칸의 번호)이 생긴다.

| 항목 | 값 | 근거 |
| --- | --- | --- |
| 서버 틱 | 30Hz (틱 예산 33.3ms) | 엔진 소스: `NetServerMaxTickRate` 기본값 30(`Engine/Config/BaseEngine.ini:1867`). 계산값: 1000ms ÷ 30 = 33.3ms |
| 맵 크기 | 바닥 2km × 2km, 배치 영역 1.9km × 1.9km | 계산값: 엔진 기본 컬 거리(150m)의 원 면적 π × 150² ≈ 70,700m²는 배치 영역 3,610,000m²의 약 2.0%다 |
| 자원 노드 | 5,000개 + 검증용 1개 | 계산값: 균등 배치에서 가장자리가 아닌 위치의 컬 거리 안 기대 개수는 5,000 × 70,700 ÷ 3,610,000 ≈ 98개다 |

기준선 조건. 규모를 확정할 때 다음을 모두 확인했다(결과는 아래 "기준선 조건").

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
| 워밍업 구간 | 접속 완료 후 30초를 버림 | 접속 직후에는 모든 액터의 초기 스폰 데이터가 한꺼번에 전송되어 정상 상태와 다르다. 측정값: `calib-f-r1`에서 측정 시작 22초 전에 열린 액터 채널 수가 더 늘지 않았다 |
| 측정 구간 | 60초 | 서버가 30Hz를 유지하면 1,800프레임이다. 유지하지 못하면 그보다 적으므로 실제 프레임 수를 함께 적는다. P99는 99백분위 경계값이며 느린 1%의 평균이 아니다 |
| 반복 | 구성마다 3회, 중앙값 기록 | 한 번의 이상치를 걸러 낸다. 지속적인 경합이나 실행 순서의 영향까지 없애지는 못한다. 세 값과 변동 폭(최댓값 - 최솟값)을 모두 적는다 |
| 차이의 판단 | 중앙값의 변화가 두 구성의 변동 폭 중 큰 쪽보다 클 때만 차이가 있다고 적음 | 그보다 작으면 "구별하지 못했다"고 적고 두 구성을 한 번 더 측정한다 |
| 비교 조건 | 같은 화면 조건에서 연달아 잰 묶음끼리 비교 | 코드가 같은 `dormancy2`(원격 데스크톱 화면)와 `dormancy6`(본체 화면)의 `work_avg_ms` 중앙값이 1.066 달랐다 |
| 실패 처리 | 시작 신호 이후(워밍업 구간 포함) 연결 수가 바뀌면 그 실행은 버림 | 연결이 빠지면 송신 바이트 합계와 서버 부하가 함께 줄어 최적화 효과로 잘못 읽게 된다. 서버가 CSV를 쓰지 않고 종료 코드 1로 끝나고, 스크립트가 실패로 처리한다 |

서버 실행 시 CPU와 네트워크 트레이스 채널을 켜서 Unreal Insights로 수집한다. 트레이스 인자는 [engine-notes.md](../Reference/engine-notes.md) 5절 "트레이스 인자"에 있다.

## 수치의 이름과 출처

같은 이름으로 다른 값을 부르지 않는다([ADR-0004](../Decisions/0004-insights-and-csv-metrics.md)). "모션 기록"은 `-MotionLog`를 켠 실행이 `Saved/LabMotion/<라벨>-rN/`에 남기는 서버와 클라이언트의 NPC 위치 기록이고, 값은 `Scripts/analyze-motion.ps1`이 계산한다([ADR-0020](../Decisions/0020-npc-motion-quality-metrics.md)).

| 이름 | 정의 | 출처 | 쓰는 곳 |
| --- | --- | --- | --- |
| `work` | 월드 틱 시작부터 프레임 끝까지의 경과 시간. CPU 실행 시간이 아니라 경과 시간이라서 그 구간의 스레드 대기와 OS 스케줄링 지연이 들어간다 | CSV | `Docs/STATUS.md`, 에이전트의 전후 비교 |
| `netflush` | 액터 틱 종료부터 프레임 끝까지의 경과 시간. 리플리케이션 전용 시간이 아니다 | CSV | `Docs/STATUS.md`, 보조 지표 |
| 서버 프레임 시간 | Timing Insights의 프레임 시간에서 틱 속도 제한 대기(`FEngineLoop_UpdateTimeAndHandleMaxTickRate`)를 뺀 시간. 평균은 (측정 구간 길이 − 구간 안의 대기 Incl) ÷ `Frame` Count, P99는 프레임마다 (`Frame` − 대기)의 99백분위 경계값([ADR-0010](../Decisions/0010-frame-time-without-tick-wait.md)) | Insights | 포스팅과 README의 표 |
| 리플리케이션 시간 | Insights 타이머 `GameNetDriver`의 프레임당 Incl(선택 구간의 Incl ÷ `WorldTick`의 Count). 규모를 확정할 때 사용자가 골랐고(2026-10-01) 이후 바꾸지 않는다 | Insights | 포스팅과 README의 표 |
| 연결당 송신 대역폭 | 측정 구간의 연결당 초당 송신 바이트 | CSV와 Network Insights | 둘 다 |
| 연결당 열린 액터 채널 수 | 연결 하나에 열려 있는 액터 채널 수. Dormant 상태의 액터는 채널이 닫히므로 "클라이언트에 존재하는 액터 수"와 다르다 | CSV | 둘 다 |
| 클라이언트에 존재하는 액터 수 | 클라이언트 화면 위 글자의 노드 수와 NPC 수 | 스크린샷 | 포스팅 |
| Consider List 길이(프레임당) | 측정 구간의 프레임마다 Consider List에 든 액터 수(엔진 지표 `NumConsideredActors`, `NetDriver.cpp:5454`)의 평균. 서버 로그 `lab_consider_list avg_per_frame=`. 레거시 경로에서만 남고 Replication Graph 실행에는 없다([engine-notes.md](../Reference/engine-notes.md) 15절) | 서버 로그 | `Docs/STATUS.md`, 포스팅 |
| `saturated_ratio` | 측정 구간에 모든 연결에서 송신 한도 때문에 중간에 끊긴 리플리케이션 횟수 ÷ 리플리케이션 시도 횟수. 엔진이 `ServerReplicateActors`에서 연결마다 남기는 기록(`UNetConnection::GetSaturationAnalytics`)의 차다([ADR-0006](../Decisions/0006-saturation-from-engine-analytics.md)). 그 전의 `smoke` 행은 프레임 끝의 `IsNetReady()`로 잰 값이다. 포화 판정 기준은 0.01 이상이다 | CSV | `Docs/STATUS.md`, 해석의 전제 |
| 표시 위치 오차 | 측정 구간의 (클라이언트, NPC, 클라이언트 프레임)마다 클라이언트에 그려지는 NPC 메시의 위치와 같은 시각의 서버 NPC 위치(서버 프레임 기록 사이를 선형 보간)의 거리(cm). 평균과 P99. NPC가 그 클라이언트에 생긴 뒤 1초 안의 표본은 뺀다([ADR-0020](../Decisions/0020-npc-motion-quality-metrics.md)) | 모션 기록 | 둘 다(포스팅 11\~13) |
| 표시 속도 오차 | 같은 표본에서 클라이언트 프레임 한 칸 동안의 표시 이동과 서버 이동의 차 ÷ 그 시간(cm/s). 평균과 P99(ADR-0020) | 모션 기록 | 둘 다(포스팅 11\~13) |
| 표시 위치가 바뀐 간격 | NPC마다 표시 위치가 1cm 넘게 바뀐 클라이언트 프레임 사이의 시간(ms). 평균과 P99. 포스팅 4의 영상에서 읽은 "위치가 바뀐 간격"을 기록으로 잰 것이다(ADR-0020) | 모션 기록 | 보조 |
| 수신 간격 | NPC마다 클라이언트가 이동 갱신을 받은 시각의 차(ms). 평균과 P99(ADR-0020) | 모션 기록 | 보조 |
| 표시 지연 | 서버 경로를 τ만큼 늦췄을 때 평균 표시 위치 오차가 가장 작은 τ(0\~600ms, 1ms 간격). 클라이언트 기록 시각이 게임 스레드 프레임의 끝이라 화면에 나오기까지의 시간은 들어가지 않는다(ADR-0020). ADR은 탐색 범위를 0\~300ms로 적었고, 포스팅 13의 지연 100\~200ms가 보간 지연 150ms에 더해지면 300ms를 넘으므로 범위만 600ms로 넓혔다(2026-10-07 사용자 결정. 정의는 같다) | 모션 기록 | 보조 |

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
| 표시 위치 오차, 표시 속도 오차 (평균, P99) | 모션 기록. 위 "수치의 이름과 출처". 포스팅 11부터 | 클라이언트가 보는 NPC 움직임의 정확도와 부드러움 |

## 한계

테스트베드 포스팅과 README에 명시한다.

- 에디터 빌드 실행 파일을 쿠킹 없이 사용하므로 절대 수치는 Shipping 빌드와 다르다([ADR-0002](../Decisions/0002-editor-build-without-packaging.md)).
- 서버와 클라이언트가 같은 PC([pc-specs.md](../Reference/pc-specs.md))에서 돈다. 서로 다른 코어에 고정하지만 캐시와 메모리 대역폭 경합은 남는다. 네트워크는 루프백이다.
- 수치는 같은 조건의 전후 비교로만 해석한다. "서버 코드만의 CPU 개선률"이라고 쓰지 않는다.
- 기준선은 엔진의 기본 관련성 판정을 의도적으로 끄고([ADR-0003](../Decisions/0003-lawless-baseline.md)) 연결당 송신 한도를 올린 인위적인 출발점이다.
- AI NPC는 움직이는 리플리케이트 액터를 대신한다. 길 찾기나 비헤이비어 트리 같은 AI 비용은 측정하지 않는다.
- 채집 RPC는 자동 실험용이다. 서버가 대상 탐색과 거리 검사, 호출 간격 제한을 하지만 그 밖의 악의적 호출은 막지 않는다.

## 확정값과 기준선 조건

2026-10-19에 STATUS.md의 "확정할 값"과 "기준선 조건"을 옮겼다. 값을 새로 확정하면 여기에 적는다.

### 확정값

단기 설계 문서 8절의 출발값을 엔진 소스 확인(태스크 2.5)과 실측(태스크 8)으로 확정해 여기에 적는다.

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 수 | 8 | 8 | 측정값: `calib-a-r1` 실행 중 UE 프로세스 9개의 메모리 합계 27.5GB, 사용 가능 메모리 최저 14.9GB로 줄일 필요가 없었다(태스크 8.2) |
| 자원 노드 수 | 5,000 | 5,000(와 검증용 1개, 합 5,001) | 측정값: 포화가 없는 실행에서 `over_budget_frames`가 `frames`와 같아(`calib-b-r1` 100/100, `calib-f-r1` 356/356) 늘리지 않았다(태스크 8.4) |
| AI NPC 수 | 300 | 300 | 출발값 그대로. 네 조건을 모두 만족해 바꾸지 않았다(태스크 8.6) |
| 워밍업 구간 | 30초 | 30초 | 측정값: `calib-f-r1`에서 측정 시작 22초 전에 `open_actor_channels_per_conn`이 5,314에 도달해 더 늘지 않았다(태스크 8.3) |
| 측정 구간 | 60초 | 60초 | 출발값 그대로. Insights에서 두 북마크 사이가 60.018초(`calib-f-r1`) |
| 서버 코어 | 논리 프로세서 0\~7 | 논리 프로세서 2\~7(마스크 252) | [ADR-0009](../Decisions/0009-server-cores-without-dpc-load.md). 측정값: `diag-d-r1`, `calib-f-r1` |
| 리플리케이션 시간 타이머 | 미정 | `GameNetDriver` | 측정값: `calib-f-r1` 측정 구간에서 Incl 57.45초, `WorldTick`의 96.01%. 프레임당 161.4ms가 CSV `netflush_avg_ms` 161.744와 0.2% 차이(태스크 8.5, 사용자 확정) |
| `NetServerMaxTickRate` 기본값 | 30 (기억값) | 30 | 엔진 소스: `Engine/Config/BaseEngine.ini:1867`. 실행 중 적용값도 30(30초에 898프레임, `smoke2-r1`) |
| `NetCullDistanceSquared` 기본값 | 225,000,000 (기억값) | 225,000,000 (150m) | 엔진 소스: `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 | 100 (기억값) | 100 (`MinNetUpdateFrequency` 2) | 엔진 소스: `Actor.cpp:295-296` |
| 연결당 송신 한도(엔진 기본값) | 모름 | 100,000바이트/초 | 엔진 소스: `BaseEngine.ini:1839-1840, 1860-1861`. 올릴 때는 세 키를 함께 올린다(engine-notes.md 1절) |
| 연결당 송신 한도(이 프로젝트에서 고정한 값) | 기준선 실측 송신량을 30Hz로 환산한 값의 약 두 배(구현 계획 8.4a) | 350,000바이트/초 | 계산값: `calib-b-r1`의 `out_bytes_per_sec_per_conn` 9,591, 실제 틱 100 ÷ 60 = 1.67Hz, 환산 송신량 9,591 × 30 ÷ 1.67 = 172,638, 두 배 345,276을 올림. 바꾼 키: `Config/DefaultEngine.ini`의 `[/Script/Engine.Player] ConfiguredInternetSpeed`, `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`, `MaxInternetClientRate`. 서버 로그 `net_speed=350000`과 `saturated_ratio` 0.000 확인(`calib-e-r1`) |
| 리플리케이션 시스템 | 레거시여야 함 | 레거시(소스 기준) | 엔진 소스: `IrisConfig.cpp:15-16`의 `net.Iris.UseIrisReplication` 기본값 0. 서버 로그 `using replication model Generic` 확인(`smoke2-r1`) |
| 2막: 플레이어 배치와 주변 NPC | 밀집 3m, 무리 하나 50개 | 밀집 3m(`-PlayerSpacing 3`), 주변 NPC 50개(`-NpcsNearPlayers 50`). 분산 300m는 비교 구성 | 사용자 확정(2026-10-04). 기준선 `calib2-d3-r1`: `over_budget_frames` 325 = `frames`, 측정 시작 21초 전에 채널 5,871, `saturated_ratio` 0.000, `GameNetDriver`가 `WorldTick`의 96.1%. Net Update Frequency가 `work_avg_ms`를 5.329 줄인다(`calib2-b-r1` 20.011, `calib2-a-r1` 14.682). 밀집과 분산은 구별된다(16.779, 27.347). 계산은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 5.1, 실행은 [Worklog/05-expanded-testbed.md](../Worklog/05-expanded-testbed.md) |
| 2막: 건축물 | 무리 하나 500개, 짓는 간격 1초 | 500개, 1초(`-Buildings 500 -BuildInterval 1`) | 사용자 확정(2026-10-04). Relevancy만 적용한 구성이 틱 예산을 넘고(`calib2-c-r1` `work_avg_ms` 35.717) Dormancy가 15.706을 줄인다(`calib2-b-r1` 20.011). 기준선에서 `LabBuilding`은 프레임당 8.42ms(`calib2-d3-r1`) |
| 2막: 상태 값, 인벤토리 | 상태 값 간격 5초, 인벤토리 200칸에 4초 | 상태 값 프로퍼티 8개에 간격 5초(`-StateInterval 5`), 인벤토리 200칸에 앞 칸을 지우는 간격 4초(`-InventoryItems 200 -InventoryChurn 4`) | 사용자 확정(2026-10-04). `LabStateComponent` 프레임당 0.264ms(`calib2-a-r1`)는 변동 폭보다 작고 프로퍼티 64개에서도 0.327ms라(`calib2-e2-r1`) Push Model만 조건의 예외로 둔다. `LabInventoryComponent`는 `Actor` 비트의 29.4%(`layout-dense1-r1`) |

### 기준선 조건

1막은 단기 구현 계획 태스크 8.4\~8.6, 2막은 2막 구현 계획 태스크 21.1(`act2-baseline1`)에서 채웠다.

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | 예 | `calib-f-r1` 서버 로그: 측정 시작(13:57:10 UTC) 22초 전의 줄(13:56:48)에서 `open_actor_channels_per_conn`이 이미 5,314이고 더 늘지 않음 |
| 지속적인 예산 초과 | 예 | `calib-f-r1`: `over_budget_frames` 356 = `frames` 356. `work_avg_ms` 168.309는 틱 예산 33.3ms(1 ÷ 30Hz)의 5.0배 |
| 가장 큰 비용이 네트워크 | 예(사용자 판단, 2026-10-01) | `calib-f-r1`의 Insights 측정 구간: `WorldTick` 59.84초 중 `GameNetDriver` 57.45초(96.01%). [insights-walkthrough-calib-f.md](insights-walkthrough-calib-f.md) 6단계 |
| 송신 한도에 포화되지 않음(포화되면 한도를 올린다. 2026-10-01 결정) | 예(한도를 350,000으로 올린 뒤) | 엔진 기본 한도에서는 `saturated_ratio` 1.000(`calib-a-r1`). 350,000에서 0.000(`calib-f-r1`), 측정 구간에 `saturated_replications`의 앞 숫자가 211에서 늘지 않음 |
| 2막: 초기 전송 완료 | 예 | 서버 로그: `open_actor_channels_per_conn`이 측정 시작 19초, 16초, 20초 전(`r1`, `r2`, `r3`)에 5,871에 도달해 더 늘지 않음 |
| 2막: 지속적인 예산 초과 | 예 | 세 실행 모두 `over_budget_frames` = `frames`(279, 278, 272). `work_avg_ms` 중앙값 215.801은 틱 예산 33.3ms의 6.47배 |
| 2막: 가장 큰 비용이 네트워크 | 예(사용자 판단, 2026-10-04) | 세 실행의 Insights 측정 구간에서 `GameNetDriver`가 `WorldTick`의 95.74\~95.85%. CSV로는 206.944 ÷ 215.801 = 95.9%(중앙값). [candidates.md](../../Posts/05-expanded-testbed/candidates.md) 2절 |
| 2막: 송신 한도에 포화되지 않음 | 예 | 세 실행 모두 `saturated_ratio` 0.000. 측정 구간에 `saturated_replications`의 앞 숫자가 259, 258, 261에서 늘지 않음. 30Hz 환산 송신량은 36,994 × 30 ÷ (278 ÷ 60) = 239,529바이트/초로 한도 350,000의 68%(`r2`) |
