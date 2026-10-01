# 현재 상태

마지막 갱신: 2026-10-01

## 단계

테스트베드 구축 단계(태스크 1~7)를 끝냈다. 사용자가 7.6(화면)과 7.8(수동 조작)을 확인했다(2026-10-01). 작은 규모 실행(클라이언트 2, 노드 101, NPC 10)이 종료 코드 0으로 끝나고 CSV 행과 자동 스크린샷(글자와 점)이 남는 것을 `smoke2`~`smoke5`로 확인했다(`smoke5`는 템플릿 정리 후). 확인 내용은 [engine-notes.md](Planning/engine-notes.md) 마절에 있다.

실행 로그마다 남던 `LogPython: Error` 58줄(`AllToolsets` 플러그인의 Python 시작 스크립트가 `-server`, `-game`에서 실패)을 실행 스크립트에 `-DisablePython`을 넣어 없앴다. 사용자가 이 플러그인을 에디터에서 쓰므로 `.uproject`는 그대로 둔다(2026-10-01 사용자 확인). Python의 매 프레임 티커가 빠지므로 측정 조건이 바뀐 것이고, 기준선 전이라 비교에는 문제가 없지만 앞으로 측정은 `run-scenario.ps1`로만 실행한다(engine-notes.md 마절). `smoke7-r1`이 종료 코드 0으로 끝나고 CSV 행이 남았으며, 세 로그 모두 `LogPython: Error`가 0줄이다. `smoke7-r1` 수치는 `smoke2`~`smoke5`와 같은 범위다(`frames` 898, `work_avg_ms` 2.741, `out_bytes_per_sec_per_conn` 5446, `open_actor_channels_per_conn` 118). `smoke6-r1`은 측정 구간 29.5초 시점에 1번 클라이언트 창이 Alt-F4로 닫혀(`client1-smoke6-r1.log`의 `Alt-F4 pressed!`, 11:24:34 UTC) 서버가 연결 수 변화를 검출하고 종료 상태 1로 끝났다(`server-smoke6-r1.log`의 `Run failed: connection count changed after start`). CSV 행이 없고 수치는 쓰지 않는다. 실패 처리가 실제로 동작한 사례다.

규모 확정 단계(태스크 8)를 시작하기 전에 점검을 하고 다음을 고쳤다(2026-10-01). 근거는 engine-notes.md 바절에 있다.

- **`saturated_ratio`의 정의를 바꿨다.** 프레임 끝의 `IsNetReady()`는 그 프레임의 송신 예산을 뺀 뒤라 지속적인 포화를 잡지 못한다. 엔진이 `ServerReplicateActors`에서 연결마다 남기는 기록으로 바꿨다(포화로 끊긴 리플리케이션 횟수 ÷ 시도 횟수). `smoke8-r1`의 서버 로그에서 접속 직후 `saturated_replications=14/184`가 찍혔고 측정 구간 값은 0.000이다. `smoke7`까지의 행은 옛 정의다.
- **태스크 8.4와 8.4a의 순서를 고쳤다.** 포화를 먼저 없앤 뒤 예산 초과를 판단하고, 한도는 규모가 정해진 뒤 30Hz 환산 송신량의 약 두 배로 한 번만 고정한다. 포화 판정 기준은 `saturated_ratio` 0.01 이상이다.
- **`run-scenario.ps1`의 실패 처리를 보강했다.** 시작 신호 전에 죽은 클라이언트는 다시 띄운다(실행당 최대 3번, `RESTART:` 줄). 서버 종료 시점의 클라이언트 생존, `.utrace` 존재, 측정 시작 뒤의 선호도 재설정, 라벨의 로그와 트레이스 재사용, 이미 떠 있는 `UnrealEditor`를 검사한다. 클라이언트에는 `-traceautostart=0`을 준다. 다시 띄우기와 각 실패 경로는 아직 실행으로 확인하지 않았다.
- **트레이스 인자가 동작한다.** `smoke8-r1`(클라이언트 2, 노드 101, NPC 10, 준비 20초, 측정 30초, 트레이스 켬)이 종료 코드 0으로 끝나고 `Saved/Traces/smoke8-r1.utrace`(5.5MB)가 생겼다. 수치는 이전 `smoke` 실행과 같은 범위다(`frames` 898, `work_avg_ms` 2.212, `out_bytes_per_sec_per_conn` 5417, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000). 사용자가 Insights로 열어 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 확인했다(2026-10-01). Networking 창에서 패킷 막대와 패킷 내용의 액터 이름도 확인했다. `-trace=default,net -NetTrace=1 -tracefile=` 인자를 그대로 쓴다.
- **`-traceautostart=0`을 넣은 뒤에도 실행이 정상이다.** `smoke9-r1`(같은 규모, `-NoTrace`)이 종료 코드 0으로 끝났다(`frames` 897, `work_avg_ms` 1.996, `out_bytes_per_sec_per_conn` 5430, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000). 같은 라벨로 다시 실행하면 스크립트가 거부하는 것도 확인했다. Insights가 떠 있을 때 자동 연결이 실제로 막히는지는 확인하지 않았다.
- **Unreal Insights를 소스 빌드에서 빌드했다.** `G:\Epic Games\UE_Source\Engine\Binaries\Win64\UnrealInsights.exe`.
- **README.md를 만들었다.** 규모는 보정 전의 출발값으로 적었고, 스크린샷 두 장은 `smoke8-r1`의 것이다(`Posts/00-testbed/images/`). 태스크 8 뒤에 확정값과 확정 규모의 이미지로 바꾼다.

결정 기록과 문서 지도를 만들었다(2026-10-01). 설계 문서와 구현 계획, engine-notes.md에 흩어져 있던 결정 8개를 [Docs/Decisions/](Decisions/README.md)에 ADR로 옮겼다(ADR-0005는 0006으로 대체된 포화 판정의 옛 정의). [AGENTS.md](../AGENTS.md)에 "문서 지도"와 ADR 규칙을 넣고, 에이전트 메모리에만 있던 일정 표현 금지 규칙과 스크립트의 UTF-8 BOM 규칙을 옮겼다. 결정 자체는 바꾸지 않았다.

엔진은 소스 빌드로 통일했다. `DSOptLab.uproject`의 `EngineAssociation`이 `UE_DSOptLab`이고, 스크립트는 `Scripts/common.ps1`에서 같은 값을 레지스트리로 찾는다. 런처 설치본(`G:\Epic Games\UE_5.8`)으로 프로젝트를 열면 같은 `Binaries/`에 다시 빌드되므로 열지 않는다.

## 다음 할 일

1. 푸시는 사용자가 정한 시점에 한다. 원격 저장소(`origin`)는 이미 있고, 2026-10-01의 태스크 8 준비 커밋까지 올렸다.
2. **태스크 8.1 전에 메모리를 확보한다.** 2026-10-01 점검 때 UE 프로세스 없이 사용 가능 메모리가 23.7GB, 커밋이 56.0GB였다(Rider, Chrome 등). 클라이언트 하나가 맵 로드 전에 이미 2.5GB를 쓰므로(`client0-smoke1-r1.log`) 9개 프로세스에는 모자랄 수 있다. 다른 프로그램을 닫고 사용 가능 메모리를 확인한 뒤 실행한다.
3. 규모 확정 단계: 태스크 8(출발값으로 트레이스와 함께 실행, 기준선 조건 확인). 엔진 기본 송신 한도가 100,000바이트/초라 8.4a(한도 올리기)가 필요할 가능성이 높다. 태스크 8.1은 새 세션에서 시작한다(2026-10-01 사용자 결정). 트레이스 확인은 끝났으므로 2번의 메모리 확인만 하고 바로 실행한다.

## 포스팅 진행

| 포스팅 | 상태 | 태그 |
| --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 시작 전 | |
| 1. 무법지대 측정 | 시작 전 | |
| 2. 관련성과 컬 거리 | 시작 전 | |
| 3. 자원 노드 휴면 | 시작 전 | |
| 4. AI NPC 업데이트 빈도 | 시작 전 | |

## 명령

구현 계획의 태스크 8에서 확정한다.

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (2026-10-01 성공 확인. 에디터가 열려 있으면 DLL 잠금으로 실패한다)
- 시나리오 실행(`-Label`과 `-Runs` 없이): 미정(태스크 8에서 확정). 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 측정 중에는 클라이언트 창에 키 입력을 하지 않고, Insights 분석이나 빌드 같은 무거운 작업을 하지 않는다. 서버만 논리 프로세서 0~7에 고정하므로 다른 프로그램은 그 코어를 쓸 수 있다. 에디터가 열려 있으면 스크립트가 실행을 거부한다.
- Insights: `G:\Epic Games\UE_Source\Engine\Binaries\Win64\UnrealInsights.exe`로 `Saved/Traces/<라벨>-rN.utrace`를 연다.
- 수치 CSV 위치: `Saved/LabMetrics/summary.csv`
- 리플리케이션 시간으로 쓰는 Insights 타이머: 미정

## 확정할 값

설계 문서 8절의 출발값을 엔진 소스 확인(태스크 2.5)과 실측(태스크 8)으로 확정해 여기에 적는다.

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 수 | 8 | | |
| 자원 노드 수 | 5,000 | | |
| AI NPC 수 | 300 | | |
| 준비 구간 | 30초 | | |
| `NetServerMaxTickRate` 기본값 | 30 (기억값) | 30 | 엔진 소스: `Engine/Config/BaseEngine.ini:1867`. 실행 중 적용값도 30(30초에 898프레임, `smoke2-r1`) |
| `NetCullDistanceSquared` 기본값 | 225,000,000 (기억값) | 225,000,000 (150m) | 엔진 소스: `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 | 100 (기억값) | 100 (`MinNetUpdateFrequency` 2) | 엔진 소스: `Actor.cpp:295-296` |
| 연결당 송신 한도(엔진 기본값) | 모름 | 100,000바이트/초 | 엔진 소스: `BaseEngine.ini:1839-1840, 1860-1861`. 올릴 때는 세 키를 함께 올린다(engine-notes.md 가절) |
| 연결당 송신 한도(이 프로젝트에서 고정한 값) | 기준선 실측 송신량을 30Hz로 환산한 값의 약 두 배(구현 계획 8.4a) | | |
| 리플리케이션 시스템 | 레거시여야 함 | 레거시(소스 기준) | 엔진 소스: `IrisConfig.cpp:15-16`의 `net.Iris.UseIrisReplication` 기본값 0. 서버 로그 `using replication model Generic` 확인(`smoke2-r1`) |

## 기준선 조건

구현 계획 태스크 8.4~8.6에서 채운다.

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | | |
| 지속적인 예산 초과 | | |
| 가장 큰 비용이 네트워크 | | |
| 송신 한도에 포화되지 않음(포화되면 한도를 올린다. 2026-10-01 결정) | | |

## 측정 결과

CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |

## 포스팅 주기 진행

구현 계획 태스크 11은 포스팅 2, 3, 4에 반복해서 쓴다. 현재 포스팅과 끝낸 단계를 여기에 적는다.

- 현재 포스팅: 없음
- 끝낸 단계: 없음

## 막힌 것

없음.

## 사용자에게 요청한 일

- ADR-0001~0008의 내용을 읽고 확인한다. 이미 확정된 결정을 옮긴 것이라 상태는 "승인됨"으로 적었다. 고칠 곳이 있으면 알려 준다.
- 푸시는 사용자가 정한 시점에 한다.
