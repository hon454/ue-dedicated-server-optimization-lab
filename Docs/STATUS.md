# 현재 상태

마지막 갱신: 2026-10-02

## 단계

태스크 1\~13을 끝냈고 포스팅 1\~4가 완료다(포스팅 4는 2026-10-02, 태그 `post-04-update-frequency`). 세 기법을 모두 적용했다. 빌드된 바이너리는 main의 소스(`SetNetUpdateFrequency(10.f)` 적용, 시연용 NPC 코드 포함)와 같다. 경위는 [Worklog/00-testbed.md](Worklog/00-testbed.md)(태스크 1\~9, 보정 실행 표), [Worklog/01-baseline.md](Worklog/01-baseline.md)(태스크 10), [Worklog/02-relevancy.md](Worklog/02-relevancy.md)(태스크 11), [Worklog/03-dormancy.md](Worklog/03-dormancy.md)(태스크 12), [Worklog/04-update-frequency.md](Worklog/04-update-frequency.md)(태스크 13)에 있다. 다음 작업에 영향을 주는 것만 여기에 남긴다.

- 내려다보기 화면의 파란 점(다른 플레이어)은 현재 시나리오에서 보이지 않는다. 플레이어 자리 간격 약 195m가 컬 거리 150m보다 크다(00-testbed.md "내려다보기 화면의 플레이어 점").

## 다음 할 일

1. **태스크 14(전체 다듬기).** 14.4는 끝났다(아래 "측정 결과"의 `verify-baseline-r1`). 14.2의 점검에서 나온 수정 가운데 포스팅 0, 2, 3의 네 곳은 고쳤고, README의 두 곳(168ms 옆에 198.43ms 문장, 실행 방법의 태그별 차이)은 14.3에서 고친다. 남은 순서: 아래 "사용자에게 요청한 일"의 표현 결정 → 그 결정을 반영하는 내용 커밋 → 14.2a(한다체 변환, 한 커밋. 포스팅 하나를 먼저 보여 주고 나머지를 바꾼다) → 14.3 → 14.6.
2. **태스크 9.4.** `all-clients.png`와 `clip.gif`를 넣었다(`visual11-r1`, 태그 `post-01-baseline`의 빌드). [사람] 넣은 자료를 고르고 테스트베드 포스팅 초안을 읽고 다듬는다.
3. 태스크 9.5(사용자가 초안을 승인하면 포스팅 진행표를 "완료"로, 커밋과 `post-00-testbed` 태그).
4. 푸시는 사용자가 정한 시점에 한다.

## 포스팅 진행

| 포스팅 | 상태 | 태그 |
| --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 초안(태스크 9.4, 9.5 남음) | |
| 1. 무법지대 측정 | 완료 | `post-01-baseline` |
| 2. 관련성과 컬 거리 | 완료 | `post-02-relevancy` |
| 3. 자원 노드 휴면 | 완료 | `post-03-dormancy` |
| 4. AI NPC 업데이트 빈도 | 완료 | `post-04-update-frequency` |

## 명령

태스크 8에서 확정했다(2026-10-01).

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (2026-10-01 성공 확인. 에디터가 열려 있으면 DLL 잠금으로 실패한다)
- 시나리오 실행(확정 규모, `-Label`과 `-Runs` 없이): `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Clients 8 -Nodes 5000 -Npcs 300 -Warmup 30 -Measure 60`. 측정할 때는 `-Label <새 라벨> -Runs 3`을 더한다. 서버 마스크는 스크립트 기본값 252(논리 프로세서 2\~7)이고 트레이스는 켜진다. `calib-f-r1`이 이 조건의 실행이다.
- 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 측정 중에는 클라이언트 창에 키 입력을 하지 않고, Insights 분석이나 빌드 같은 무거운 작업을 하지 않는다. 에이전트도 문서 편집을 포함해 다른 작업을 하지 않는다(`calib-g-r1` 실행 중의 문서 편집, [Worklog/00-testbed.md](Worklog/00-testbed.md) "태스크 9.1\~9.3"). 서버만 논리 프로세서 2\~7에 고정하므로 다른 프로그램은 그 코어를 쓸 수 있다. 에디터가 열려 있으면 스크립트가 실행을 거부한다.
- 구성 사이의 비교는 같은 화면 조건에서 연달아 잰 묶음끼리 한다. 코드가 같은 `dormancy2`(원격 데스크톱 화면)와 `dormancy6`(본체 화면)의 `work_avg_ms` 중앙값이 1.066 달랐다([Worklog/04-update-frequency.md](Worklog/04-update-frequency.md) "태스크 11.1\~11.4").
- Insights: `powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label <라벨>-rN`. 읽는 순서는 [insights-reading.md](Guides/insights-reading.md)에 있다.
- 두 클라이언트 영상(1번 내려다보기, 2번 3인칭 이동): 이 PC의 150% 배율에서 두 창은 959,-47과 1919,-47(각 962×588)이라 `capture-video.ps1 -Region "959,0,1922,541" -NoMouse`로 찍는다. `-Region`은 창을 맨 위로 올리지 않으므로, 찍기 전에 두 창(명령줄 `-LabSlot=1`, `-LabSlot=2`)을 `SetWindowPos`로 TOPMOST로 올리고 끝나면 되돌린다(`visual2`, `visual3`에서 이렇게 찍었다).
- NPC 하나의 움직임을 전후로 찍을 때: `run-scenario.ps1`에 `-ShowcaseNpc`를 더한다(`visualN` 라벨에서만 받는다. 0번 자리 앞 10m를 왕복하는 NPC 하나가 더 생긴다). 0번 창의 클라이언트 영역은 0,0 960×540이라 `capture-video.ps1 -Region "0,0,960,540" -Fps 60 -NoMouse -AllowMeasuring -Out <이름>.mp4`로 찍고, 자르기와 느린 재생은 ffmpeg로 따로 한다(`visual9`, `visual10`. 가공 값은 [candidates.md](../Posts/04-update-frequency/candidates.md) 6절).
- 수동 확인을 에이전트가 할 때: `run-manual.ps1`은 `Read-Host`로 기다리므로 `Start-Process powershell`로 새 창에 띄우고, 끝나면 `UnrealEditor`와 그 창을 종료한다(에이전트의 `Stop-Process`는 거부된다. troubleshooting.md). 컴퓨터 조작 권한은 `UnrealEditor.exe`의 전체 경로로 요청한다. 관찰자 창을 클릭하면 마우스가 카메라를 돌리므로 클릭한 뒤 커서를 옮겨 카메라를 맞춘다(커서의 창 안 x좌표에 따라 돌고, 조작 도구 좌표로 1px에 약 0.3°, 창 폭만큼만 돌릴 수 있다). 달리기(`shift+w`) 28초가 약 275m다(2026-10-02, 포스팅 3의 정확성 확인).
- 문서용 Insights 캡처: `powershell -ExecutionPolicy Bypass -File Scripts/capture-insights.ps1 -Label <라벨>-rN -Out <경로>.png`. 화면 복사 대신 창 내용만 찍는다.
- Insights 이미지에 번호 붙은 상자 그리기: `powershell -ExecutionPolicy Bypass -File Scripts/annotate-image.ps1 -In <원본>.png -Out <포스팅용 이름>.png -Boxes "x,y,w,h;x,y,w,h"`. 좌표는 원본 픽셀 기준, 상자는 최대 3개, 원본은 그대로 둔다.
- 빌드나 실행이 실패하면 [troubleshooting.md](Guides/troubleshooting.md)에서 증상을 찾는다.
- 수치 CSV 위치: `Saved/LabMetrics/summary.csv`
- 리플리케이션 시간으로 쓰는 Insights 타이머: `GameNetDriver`(프레임당 Incl = 선택 구간의 Incl ÷ `WorldTick`의 Count). 태스크 8.5에서 사용자가 확정했고 이후 바꾸지 않는다.

## 확정할 값

설계 문서 8절의 출발값을 엔진 소스 확인(태스크 2.5)과 실측(태스크 8)으로 확정해 여기에 적는다.

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 수 | 8 | 8 | 측정값: `calib-a-r1` 실행 중 UE 프로세스 9개의 메모리 합계 27.5GB, 사용 가능 메모리 최저 14.9GB로 줄일 필요가 없었다(태스크 8.2) |
| 자원 노드 수 | 5,000 | 5,000(와 검증용 1개, 합 5,001) | 측정값: 포화가 없는 실행에서 `over_budget_frames`가 `frames`와 같아(`calib-b-r1` 100/100, `calib-f-r1` 356/356) 늘리지 않았다(태스크 8.4) |
| AI NPC 수 | 300 | 300 | 출발값 그대로. 네 조건을 모두 만족해 바꾸지 않았다(태스크 8.6) |
| 준비 구간 | 30초 | 30초 | 측정값: `calib-f-r1`에서 측정 시작 22초 전에 `open_actor_channels_per_conn`이 5,314에 도달해 더 늘지 않았다(태스크 8.3) |
| 측정 구간 | 60초 | 60초 | 출발값 그대로. Insights에서 두 북마크 사이가 60.018초(`calib-f-r1`) |
| 서버 코어 | 논리 프로세서 0\~7 | 논리 프로세서 2\~7(마스크 252) | [ADR-0009](Decisions/0009-server-cores-without-dpc-load.md). 측정값: `diag-d-r1`, `calib-f-r1` |
| 리플리케이션 시간 타이머 | 미정 | `GameNetDriver` | 측정값: `calib-f-r1` 측정 구간에서 Incl 57.45초, `WorldTick`의 96.01%. 프레임당 161.4ms가 CSV `netflush_avg_ms` 161.744와 0.2% 차이(태스크 8.5, 사용자 확정) |
| `NetServerMaxTickRate` 기본값 | 30 (기억값) | 30 | 엔진 소스: `Engine/Config/BaseEngine.ini:1867`. 실행 중 적용값도 30(30초에 898프레임, `smoke2-r1`) |
| `NetCullDistanceSquared` 기본값 | 225,000,000 (기억값) | 225,000,000 (150m) | 엔진 소스: `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 | 100 (기억값) | 100 (`MinNetUpdateFrequency` 2) | 엔진 소스: `Actor.cpp:295-296` |
| 연결당 송신 한도(엔진 기본값) | 모름 | 100,000바이트/초 | 엔진 소스: `BaseEngine.ini:1839-1840, 1860-1861`. 올릴 때는 세 키를 함께 올린다(engine-notes.md 가절) |
| 연결당 송신 한도(이 프로젝트에서 고정한 값) | 기준선 실측 송신량을 30Hz로 환산한 값의 약 두 배(구현 계획 8.4a) | 350,000바이트/초 | 계산값: `calib-b-r1`의 `out_bytes_per_sec_per_conn` 9,591, 실제 틱 100 ÷ 60 = 1.67Hz, 환산 송신량 9,591 × 30 ÷ 1.67 = 172,638, 두 배 345,276을 올림. 바꾼 키: `Config/DefaultEngine.ini`의 `[/Script/Engine.Player] ConfiguredInternetSpeed`, `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`, `MaxInternetClientRate`. 서버 로그 `net_speed=350000`과 `saturated_ratio` 0.000 확인(`calib-e-r1`) |
| 리플리케이션 시스템 | 레거시여야 함 | 레거시(소스 기준) | 엔진 소스: `IrisConfig.cpp:15-16`의 `net.Iris.UseIrisReplication` 기본값 0. 서버 로그 `using replication model Generic` 확인(`smoke2-r1`) |

## 기준선 조건

구현 계획 태스크 8.4\~8.6에서 채운다.

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | 예 | `calib-f-r1` 서버 로그: 측정 시작(13:57:10 UTC) 22초 전의 줄(13:56:48)에서 `open_actor_channels_per_conn`이 이미 5,314이고 더 늘지 않음 |
| 지속적인 예산 초과 | 예 | `calib-f-r1`: `over_budget_frames` 356 = `frames` 356. `work_avg_ms` 168.309는 틱 예산 33.3ms(1 ÷ 30Hz)의 5.0배 |
| 가장 큰 비용이 네트워크 | 예(사용자 판단, 2026-10-01) | `calib-f-r1`의 Insights 측정 구간: `WorldTick` 59.84초 중 `GameNetDriver` 57.45초(96.01%). [insights-walkthrough-calib-f.md](Guides/insights-walkthrough-calib-f.md) 6단계 |
| 송신 한도에 포화되지 않음(포화되면 한도를 올린다. 2026-10-01 결정) | 예(한도를 350,000으로 올린 뒤) | 엔진 기본 한도에서는 `saturated_ratio` 1.000(`calib-a-r1`). 350,000에서 0.000(`calib-f-r1`), 측정 구간에 `saturated_replications`의 앞 숫자가 211에서 늘지 않음 |

## 측정 결과

CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `baseline3-r1` | 302 | 198.773 | 268.485 | 302 | 189.375 | 28896 | 5314 | 0.000 |
| `baseline3-r2` | 292 | 205.150 | 238.737 | 292 | 195.108 | 27997 | 5314 | 0.000 |
| `baseline3-r3` | 334 | 179.673 | 262.736 | 334 | 172.293 | 31990 | 5314 | 0.000 |
| **중앙값** | 302 | 198.773 | 262.736 | 302 | 189.375 | 28896 | 5314 | 0.000 |
| **변동 폭** | 42 | 25.477 | 29.748 | 42 | 22.815 | 3993 | 0 | 0.000 |
| `relevancy2-r1` | 1304 | 17.254 | 25.984 | 1 | 13.357 | 3871 | 118 | 0.000 |
| `relevancy2-r2` | 1488 | 17.447 | 29.711 | 4 | 13.293 | 4076 | 118 | 0.000 |
| `relevancy2-r3` | 1797 | 17.380 | 25.685 | 1 | 13.478 | 4475 | 118 | 0.000 |
| **중앙값** | 1488 | 17.380 | 25.984 | 1 | 13.357 | 4076 | 118 | 0.000 |
| **변동 폭** | 493 | 0.193 | 4.026 | 3 | 0.185 | 604 | 0 | 0.000 |
| **`baseline3` 대비** | +1186 | -181.393(-91.3%) | -236.752(-90.1%) | -301 | -176.018(-92.9%) | -24820(-85.9%) | -5196(-97.8%) | 0 |
| `dormancy2-r1` | 1288 | 16.205 | 24.410 | 1 | 11.774 | 3734 | 20 | 0.000 |
| `dormancy2-r2` | 1788 | 14.409 | 22.063 | 1 | 10.430 | 4316 | 20 | 0.000 |
| `dormancy2-r3` | 1789 | 14.173 | 21.116 | 1 | 10.298 | 4320 | 20 | 0.000 |
| **중앙값** | 1788 | 14.409 | 22.063 | 1 | 10.430 | 4316 | 20 | 0.000 |
| **변동 폭** | 501 | 2.032 | 3.294 | 0 | 1.476 | 586 | 0 | 0.000 |
| **`relevancy2` 대비** | +300 | -2.971(-17.1%) | -3.921(-15.1%) | 0 | -2.927(-21.9%) | +240(+5.9%) | -98(-83.1%) | 0 |
| `update-frequency2-r1` | 1792 | 12.305 | 18.928 | 1 | 8.674 | 2412 | 20 | 0.000 |
| `update-frequency2-r2` | 1792 | 12.645 | 19.175 | 1 | 8.915 | 2408 | 20 | 0.000 |
| `update-frequency2-r3` | 1798 | 12.595 | 19.045 | 1 | 8.783 | 2400 | 20 | 0.000 |
| **중앙값** | 1792 | 12.595 | 19.045 | 1 | 8.783 | 2408 | 20 | 0.000 |
| **변동 폭** | 6 | 0.340 | 0.247 | 0 | 0.241 | 12 | 0 | 0.000 |
| **`dormancy2` 대비** | +4 | -1.814(-12.6%) | -3.018(-13.7%) | 0 | -1.647(-15.8%) | -1908(-44.2%) | 0 | 0 |
| `dormancy6-r1` | 1792 | 13.343 | 20.604 | 1 | 9.600 | 4323 | 20 | 0.000 |
| `dormancy6-r2` | 1794 | 13.593 | 21.044 | 1 | 9.700 | 4317 | 20 | 0.000 |
| `dormancy6-r3` | 1797 | 13.080 | 20.109 | 1 | 9.212 | 4314 | 20 | 0.000 |
| **중앙값** | 1794 | 13.343 | 20.604 | 1 | 9.600 | 4317 | 20 | 0.000 |
| **변동 폭** | 5 | 0.513 | 0.935 | 0 | 0.488 | 9 | 0 | 0.000 |
| `update-frequency3-r1` | 1797 | 12.894 | 19.080 | 1 | 9.054 | 2402 | 20 | 0.000 |
| `update-frequency3-r2` | 1795 | 12.479 | 18.298 | 1 | 8.719 | 2400 | 20 | 0.000 |
| `update-frequency3-r3` | 1798 | 12.923 | 19.327 | 1 | 8.969 | 2400 | 20 | 0.000 |
| **중앙값** | 1797 | 12.894 | 19.080 | 1 | 8.969 | 2400 | 20 | 0.000 |
| **변동 폭** | 3 | 0.444 | 1.029 | 0 | 0.335 | 2 | 0 | 0.000 |
| **`dormancy6` 대비** | +3 | -0.449(-3.4%) | -1.524(-7.4%) | 0 | -0.631(-6.6%) | -1917(-44.4%) | 0 | 0 |

기준선 `baseline3`(2026-10-02, 태스크 10.1\~10.2)은 확정 명령에 `-Label baseline3 -Runs 3`을 더해 한 번에 실행했고 종료 코드 0, 세 실행 모두 측정 시작 뒤의 선호도 재설정이 없었다(마지막 재적용이 측정 시작 9\~39초 전). `.utrace` 세 개가 `Saved/Traces/`에 있다. 이전 라벨 `baseline`, `baseline2`는 실패해 수치를 쓰지 않는다. 준비 구간은 30초 그대로다(사용자 결정, 2026-10-02).

- **빌드에 들어 있는 것.** 메시 머테리얼(`98ecb8f`, `104c299`), 달리기, 내려다보기 플레이어 점이 모두 들어간 main 소스(`9833b92`)의 빌드다. 서버 수치는 머테리얼로 달라지지 않음을 대조 실행으로 확인했다([Worklog/00-testbed.md](Worklog/00-testbed.md)의 "보정 실행 표"). 달리기와 플레이어 점은 같은 방식으로 대조하지 않았다. 시나리오의 클라이언트는 달리지 않는다.
- **변동 폭이 크다.** `work_avg_ms`의 변동 폭 25.477은 중앙값의 12.8%다. `frames`는 292\~334로, 서버가 30Hz(60초에 1,800프레임)의 약 6분의 1\~5분의 1밖에 못 돈다. 30Hz 환산 송신량은 세 실행이 172,2\~172,9KB 근처로 같다(28,896 × 30 ÷ (302 ÷ 60) = 172,230바이트/초, 27,997 × 30 ÷ (292 ÷ 60) = 172,579, 31,990 × 30 ÷ (334 ÷ 60) = 172,401). 기법 효과를 읽을 때 이 폭보다 큰 차이여야 의미가 있다.
- 같은 확정 조건의 이전 성공 실행(`calib-f-r1` 168.309, `calib-g-r1` 188.412, `vis-g-r1` 170.188)보다 `baseline3`의 중앙값이 높다. 원인은 모른다. 태그 `post-01-baseline`을 다시 빌드해 1회 실행한 `verify-baseline-r1`(2026-10-02, 태스크 14.4, 종료 코드 0)도 `work_avg_ms` 161.649, `frames` 371, `open_actor_channels_per_conn` 5,314, `saturated_ratio` 0.000으로, `baseline3`의 최솟값 179.673보다 18.024 낮고 이전 성공 실행들에 가깝다. 30Hz 환산 송신량은 35,546 × 30 ÷ (371 ÷ 60) = 172,460바이트/초로 같다.

관련성 `relevancy2`(2026-10-02)는 `baseline3`와 구별되는 차이다. 틱 예산 안이라 `frames`(1,304\~1,797)와 초당 값 `out_bytes_per_sec_per_conn`이 틱 속도 제한 대기를 따라 흔들리므로, 다음 구성과의 비교는 `work_avg_ms`, `netflush_avg_ms`를 먼저 본다. Insights 값([ADR-0010](Decisions/0010-frame-time-without-tick-wait.md) 정의)은 [포스팅 2](../Posts/02-relevancy/README.md)에 있고, 경위는 [Worklog/02-relevancy.md](Worklog/02-relevancy.md)에 있다.

자원 노드 휴면 `dormancy2`(2026-10-02, 중앙값 실행 `r2`)는 `work_avg_ms`, `netflush_avg_ms`가 `relevancy2`와 구별되고, `work_p99_ms`와 `out_bytes_per_sec_per_conn`은 구별되지 않았다. `r1`만 느려서(`work_avg_ms` 16.205, `frames` 1,288) 변동 폭이 2.032로 크다(원인 모름). 다음 구성의 차이는 이 폭보다 커야 구별된다. Insights 값은 [포스팅 3](../Posts/03-dormancy/README.md)에 있고, 경위는 [Worklog/03-dormancy.md](Worklog/03-dormancy.md)에 있다.

NPC 업데이트 빈도 `update-frequency3`(2026-10-02, 중앙값 실행 `r1`)은 같은 화면 조건에서 연달아 잰 `dormancy6`(중앙값 실행 `r1`)과 비교한다. `netflush_avg_ms`, `work_p99_ms`, `out_bytes_per_sec_per_conn`은 구별되고 `work_avg_ms`만 구별되지 않았다. `update-frequency2`는 `dormancy2`와 비교한 처음 측정이다. Insights 값은 [포스팅 4](../Posts/04-update-frequency/README.md)에 있고, 경위는 [Worklog/04-update-frequency.md](Worklog/04-update-frequency.md)에 있다.

## 포스팅 주기 진행

구현 계획 태스크 11은 포스팅 2, 3, 4에 반복해서 쓴다. 현재 포스팅과 끝낸 단계를 여기에 적는다.

- 포스팅 2\~4가 끝났다(포스팅 4는 2026-10-02, 태그 `post-04-update-frequency`). 단기에 구현하는 기법은 더 없다. 다음 시각 자료 라벨은 `visual12`다.

## 막힌 것

- 없음.

## 사용자에게 요청한 일

- **포스팅 4 확인.** [포스팅 4](../Posts/04-update-frequency/README.md)의 "관찰"과 "선택"은 인터뷰 답(대역폭 중심, 포스팅 1의 이유와 전제가 채워졌는지 확인)을 에이전트가 문장으로 옮긴 것이다. 초안과 끊김 영상은 승인됐다(2026-10-02). 남은 것은 Insights 캡처 네 장과 `before-clip.gif`, `after-clip.gif`를 보고 다른 장면이 좋으면 바꾸는 것이다.
- **포스팅 3 확인.** [포스팅 3](../Posts/03-dormancy/README.md)의 "정확성 확인"에 넣은 전후 영상(`before-clip.gif`, `after-clip.gif`)과 세 항목의 표, 다시 찍은 `after-timing.png`, `after-network.png`를 보고 다른 장면이 좋으면 바꾼다. 요약의 내려다보기 화면은 순번 04(t=75s)다.
- **포스팅 1 확인.** [포스팅 1](../Posts/01-baseline/README.md)의 "선택"에서 순서의 이유 세 단락은 에이전트가 추천 근거를 옮긴 문장이다. 본인의 판단과 다르면 고친다. 연결당 송신 대역폭은 `r1` 값이다(`r2` 미확인). "관찰"의 서버 프레임 시간 문장은 ADR-0010에 맞춰 에이전트가 고쳤다(아직 확인하지 않음).
- **태스크 9.4, 9.5.** [포스팅 0](../Posts/00-testbed/README.md)에 넣은 `all-clients.png`(t=41s), `clip.gif`(t=50\~59s)와 `timing.png`, `network.png`를 보고 다른 장면이 좋으면 바꾼다. 초안을 읽고 승인하면 에이전트가 9.5(커밋과 태그)를 한다.
- **표현 결정(2026-10-02 사용자 제기, 에이전트 의견은 채팅에 냄).** 정해지면 내용 커밋으로 반영한다: ① "포스팅 N"을 글 제목으로 부르기(지시됨) ② 문단은 한다체, 표의 칸은 개조식 허용 ③ 기법 이름을 원문(Relevancy, Dormancy, Net Update Frequency)으로 쓸지와 글 제목 ④ 포스팅 파일 이름 `README.md`를 유지할지 ⑤ "Iris에서는" 섹션을 빼고 backlog로 옮길지(ADR-0001의 "결과"에 적힌 내용이라 새 ADR이 필요하다) ⑥ "관련성을 먼저 합니다"처럼 기법 이름을 행동으로 쓴 문장 고치기.
- 푸시는 사용자가 정한 시점에 한다.
