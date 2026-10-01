# 현재 상태

마지막 갱신: 2026-10-02

## 단계

테스트베드 구축 단계(태스크 1\~7)를 끝냈다. 사용자가 7.6(화면)과 7.8(수동 조작)을 확인했다(2026-10-01). 작은 규모 실행(클라이언트 2, 노드 101, NPC 10)이 종료 코드 0으로 끝나고 CSV 행과 자동 스크린샷(글자와 점)이 남는 것을 `smoke2`\~`smoke5`로 확인했다(`smoke5`는 템플릿 정리 후). 확인 내용은 [engine-notes.md](Planning/engine-notes.md) 마절에 있다.

실행 로그마다 남던 `LogPython: Error` 58줄(`AllToolsets` 플러그인의 Python 시작 스크립트가 `-server`, `-game`에서 실패)을 실행 스크립트에 `-DisablePython`을 넣어 없앴다. 사용자가 이 플러그인을 에디터에서 쓰므로 `.uproject`는 그대로 둔다(2026-10-01 사용자 확인). Python의 매 프레임 티커가 빠지므로 측정 조건이 바뀐 것이고, 기준선 전이라 비교에는 문제가 없지만 앞으로 측정은 `run-scenario.ps1`로만 실행한다(engine-notes.md 마절). `smoke7-r1`이 종료 코드 0으로 끝나고 CSV 행이 남았으며, 세 로그 모두 `LogPython: Error`가 0줄이다. `smoke7-r1` 수치는 `smoke2`\~`smoke5`와 같은 범위다(`frames` 898, `work_avg_ms` 2.741, `out_bytes_per_sec_per_conn` 5446, `open_actor_channels_per_conn` 118). `smoke6-r1`은 측정 구간 29.5초 시점에 1번 클라이언트 창이 Alt-F4로 닫혀(`client1-smoke6-r1.log`의 `Alt-F4 pressed!`, 11:24:34 UTC) 서버가 연결 수 변화를 검출하고 종료 상태 1로 끝났다(`server-smoke6-r1.log`의 `Run failed: connection count changed after start`). CSV 행이 없고 수치는 쓰지 않는다. 실패 처리가 실제로 동작한 사례다.

규모 확정 단계(태스크 8)를 시작하기 전에 점검을 하고 다음을 고쳤다(2026-10-01). 근거는 engine-notes.md 바절에 있다.

- **`saturated_ratio`의 정의를 바꿨다.** 프레임 끝의 `IsNetReady()`는 그 프레임의 송신 예산을 뺀 뒤라 지속적인 포화를 잡지 못한다. 엔진이 `ServerReplicateActors`에서 연결마다 남기는 기록으로 바꿨다(포화로 끊긴 리플리케이션 횟수 ÷ 시도 횟수). `smoke8-r1`의 서버 로그에서 접속 직후 `saturated_replications=14/184`가 찍혔고 측정 구간 값은 0.000이다. `smoke7`까지의 행은 옛 정의다.
- **태스크 8.4와 8.4a의 순서를 고쳤다.** 포화를 먼저 없앤 뒤 예산 초과를 판단하고, 한도는 규모가 정해진 뒤 30Hz 환산 송신량의 약 두 배로 한 번만 고정한다. 포화 판정 기준은 `saturated_ratio` 0.01 이상이다.
- **`run-scenario.ps1`의 실패 처리를 보강했다.** 시작 신호 전에 죽은 클라이언트는 다시 띄운다(실행당 최대 3번, `RESTART:` 줄). 서버 종료 시점의 클라이언트 생존, `.utrace` 존재, 측정 시작 뒤의 선호도 재설정, 라벨의 로그와 트레이스 재사용, 이미 떠 있는 `UnrealEditor`를 검사한다. 클라이언트에는 `-traceautostart=0`을 준다. 다시 띄우기와 각 실패 경로는 아직 실행으로 확인하지 않았다.
- **트레이스 인자가 동작한다.** `smoke8-r1`(클라이언트 2, 노드 101, NPC 10, 준비 20초, 측정 30초, 트레이스 켬)이 종료 코드 0으로 끝나고 `Saved/Traces/smoke8-r1.utrace`(5.5MB)가 생겼다. 수치는 이전 `smoke` 실행과 같은 범위다(`frames` 898, `work_avg_ms` 2.212, `out_bytes_per_sec_per_conn` 5417, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000). 사용자가 Insights로 열어 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 확인했다(2026-10-01). Networking 창에서 패킷 막대와 패킷 내용의 액터 이름도 확인했다. `-trace=default,net -NetTrace=1 -tracefile=` 인자를 그대로 쓴다.
- **`-traceautostart=0`을 넣은 뒤에도 실행이 정상이다.** `smoke9-r1`(같은 규모, `-NoTrace`)이 종료 코드 0으로 끝났다(`frames` 897, `work_avg_ms` 1.996, `out_bytes_per_sec_per_conn` 5430, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000). 같은 라벨로 다시 실행하면 스크립트가 거부하는 것도 확인했다. Insights가 떠 있을 때 자동 연결이 실제로 막히는지는 확인하지 않았다.
- **Unreal Insights를 소스 빌드에서 빌드했다.** `G:\Epic Games\UE_Source\Engine\Binaries\Win64\UnrealInsights.exe`.
- **README.md를 만들었다.** 규모는 보정 전의 출발값으로 적었고, 스크린샷 두 장은 `smoke8-r1`의 것이다(`Posts/00-testbed/images/`). 태스크 8 뒤에 확정값과 확정 규모의 이미지로 바꾼다.

결정 기록과 문서 지도를 만들었다(2026-10-01). 설계 문서와 구현 계획, engine-notes.md에 흩어져 있던 결정 8개를 [Docs/Decisions/](Decisions/README.md)에 ADR로 옮겼다(ADR-0005는 0006으로 대체된 포화 판정의 옛 정의). [AGENTS.md](../AGENTS.md)에 "문서 지도"와 ADR 규칙을 넣고, 에이전트 메모리에만 있던 일정 표현 금지 규칙과 스크립트의 UTF-8 BOM 규칙을 옮겼다. 결정 자체는 바꾸지 않았다. 이어서 AGENTS.md를 프롬프트 감사로 점검해 반영했다(곧 낡을 사실 두 개를 고치고, 이유가 없던 규칙 셋에 이유를 붙임). 커밋 메시지를 Conventional Commits로 정하고, 구현 계획의 커밋 예시와 기존 커밋 메시지를 모두 이 형식으로 바꿨다(사용자 지시로 히스토리를 재작성하고 포스 푸시함). AGENTS.md의 "확정된 결정을 다시 열지 않는다"에 예외 하나를 넣었다. 결정의 근거가 엔진 소스나 실행 결과와 맞지 않으면 대체 ADR을 "제안됨"으로 쓰고 그 결정에 기대는 측정을 멈춘다(사용자 결정).

엔진은 소스 빌드로 통일했다. `DSOptLab.uproject`의 `EngineAssociation`이 `UE_DSOptLab`이고, 스크립트는 `Scripts/common.ps1`에서 같은 값을 레지스트리로 찾는다. 런처 설치본(`G:\Epic Games\UE_5.8`)으로 프로젝트를 열면 같은 `Binaries/`에 다시 빌드되므로 열지 않는다.

규모 확정 단계의 태스크 8.1\~8.4a를 끝냈다(2026-10-01). 출발값 규모(클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 트레이스 켬)로 다섯 번 실행했다. 수치는 아래 "보정 실행" 표에 있다.

- **엔진 기본 한도에서는 완전히 포화된다.** `calib-a-r1`의 `saturated_ratio`가 1.000이다. ADR-0007에 따라 `Config/DefaultEngine.ini`에서 세 키를 10,000,000으로 올렸고, `calib-b-r1`에서 `net_speed` 10000000과 `saturated_ratio` 0.000을 확인했다.
- **출발값 규모에서 예산 초과가 지속적이다.** 포화가 없는 `calib-b-r1`에서 `over_budget_frames`가 `frames`와 같다(100/100). 노드를 늘리지 않았다.
- **한도를 350,000으로 고정했다.** `calib-b-r1`의 환산 송신량 = 9,591 × 30 ÷ (100 ÷ 60) = 172,638바이트/초이고, 그 두 배(345,276)를 올림했다. `calib-e-r1`에서 `saturated_ratio` 0.000을 확인했고, 그 실행의 환산 송신량도 28,121 × 30 ÷ (293 ÷ 60) = 172,756으로 같다. 사전 추정(135\~180KB/s) 안이다.
- **같은 구성의 실행 사이에 서버 속도가 약 세 배 차이 난다.** 포화가 없는 두 성공 실행에서 `frames`가 100(`calib-b-r1`)과 293(`calib-e-r1`), `work_avg_ms`가 602.178과 204.726이다. 프레임당 송신량은 같다(환산 송신량 172,638과 172,756).
- **원인을 확인했다(사용자 요청, 2026-10-01).** 서버 마스크 `0xFF`에 들어 있는 논리 프로세서 1번이 시나리오 실행 중 DPC를 초당 약 15,000개 처리한다(DPC 시간 32\~65%). 서버 게임 스레드가 1번에 올라가 있는 동안 CPU를 절반 넘게 빼앗긴다. 서버를 1번에만 고정하면 내내 느리고(`diag-c-r1`: `frames` 90, `work_avg_ms` 670.713), 2\~7번에 고정하면 내내 빠르다(`diag-d-r1`: `frames` 340, `work_avg_ms` 176.630). 근거와 확인하지 않은 것은 engine-notes.md 사절에 있다. 서버를 2\~7번에 고정하자는 [ADR-0009](Decisions/0009-server-cores-without-dpc-load.md)를 "제안됨"으로 썼다. ADR-0008의 코어 고정 항목에 기대는 측정(기준선)은 승인 전까지 시작하지 않는다. 진단을 위해 `run-scenario.ps1`에 `-ServerMask` 인자를 넣었다(기본값 255로 동작은 그대로).
- **측정 시작 뒤에 선호도가 다시 설정되어 두 번 실패했다.** `calib-c-r1`(측정 시작 22초 뒤)과 `calib-d-r1`(서버 종료 시각)이다. 두 실행의 CSV 행은 쓰지 않는다. 스크립트가 어느 프로세스인지 남기지 않아 원인을 알 수 없었으므로 `run-scenario.ps1`에 `AFFINITY:` 줄과 `pids:` 줄을 넣었고, 종료 중이라 선호도를 읽을 수 없는 프로세스는 다시 설정하지 않게 했다(`calib-d-r1`의 실패는 이 경우로 추정하며 확인하지 않았다). `calib-e-r1`에서는 클라이언트의 선호도가 시작 신호 15초 뒤(22:24:08, 측정 시작 15초 전)까지 전체 코어로 되돌아갔다.
- **PC 자원(8.2, `calib-a-r1` 실행 중 11초 간격 기록).** UE 프로세스 9개의 메모리 합계 27.5GB(서버 1.7GB, 클라이언트 3.0\~3.4GB), 사용 가능 메모리 최저 14.9GB, 커밋 80.9GB, VRAM 18.9GB/24.6GB, GPU 사용률 94%. 서버 코어는 논리 프로세서 1번이 77\~95%이고 나머지 일곱 개는 대체로 10\~50%다. 실행 전 사용 가능 메모리는 34.3GB였다.
- **초기 전송(8.3).** 성공한 세 실행 모두 측정 시작 12\~21초 전에 `open_actor_channels_per_conn`이 5,314에 도달해 더 늘지 않았다.
- **자동 스크린샷.** `calib-a-r1`의 3인칭 화면과 내려다보기 화면, `calib-b-r1`의 내려다보기 화면에 `nodes=5001 npcs=300` 글자와 초록 점, 빨간 점이 찍혔다.

이 세션은 앱이 만든 워크트리에서 열려 main 체크아웃의 파일 편집이 훅으로 막혔다. 워크트리의 `claude/task8-calibration` 브랜치에서 고치고 main에 fast-forward로 반영했다. 빌드와 실행은 main 체크아웃에서 했다(사용자 결정).

클라이언트 실행 인자에서 `-log`를 뺐다(2026-10-01, `run-scenario.ps1`과 `run-manual.ps1`). 클라이언트마다 뜨던 로그 콘솔 창이 없어지고 로그 파일은 그대로 남는다(`nolog1-r1`에서 확인). 측정 조건이 바뀐 것이고, 클라이언트에 `-log`가 없는 실행은 `diag-d-r1`부터다. 근거는 [engine-notes.md](Planning/engine-notes.md) 마절에 있다.

ADR-0009를 사용자가 승인했다(2026-10-01). `run-scenario.ps1`의 `-ServerMask` 기본값을 252(논리 프로세서 2\~7)로 바꿨다. 이 조건(클라이언트 `-log` 없음, 송신 한도 350,000)으로 확정 규모를 다시 실행한 `calib-f-r1`이 종료 코드 0으로 끝났다: `frames` 356, `work_avg_ms` 168.309, `work_p99_ms` 245.826, `saturated_ratio` 0.000. 5초 간격 틱이 준비 구간과 측정 구간 내내 4.8\~6.3Hz였고, 측정 시작 뒤의 선호도 재설정이 없었다. 실행 중 5초 간격으로 잰 DPC 시간은 1번이 36\~50%, 0번과 2\~7번이 0\~1%다. 30Hz 환산 송신량은 34,164 × 30 ÷ (356 ÷ 60) = 172,739로 이전과 같다. 3인칭 화면과 내려다보기 화면의 자동 스크린샷에 `nodes=5001 npcs=300` 글자와 점이 찍혔다. 서버를 1번에만 고정한 대조 실행 `diag-e-r1`(클라이언트 `-log` 없음)은 `frames` 153, `work_avg_ms` 392.972였다.

클라이언트 화면 글자를 바꿨다(2026-10-01, 사용자 요청). 엔진 화면 메시지(`AddOnScreenDebugMessage`, 노란색) 대신 새 클래스 `ALabHUD`가 반투명 검은 상자 위에 하늘색 글자를 1.4배 크기로 그린다. 노란색은 사진에서 경고처럼 보였고, 흰색은 밝은 하늘과 바닥 위에서 눈에 덜 띄었다(색 후보 비교 이미지로 사용자가 고름). 엔진 화면 메시지를 더 쓰지 않으므로 그 아래에 찍히던 엔진의 회색 안내 문구도 없어졌다. 첫 줄은 `<라벨> | slot=N <harvest|move|idle> <tpp|topdown> | t=Ns`(시작 신호 후 경과 시간, 신호 전에는 `t=waiting`), 둘째 줄은 `on this client: nodes=N npcs=N | pos x=Nm y=Nm`다. 클라이언트 화면만 바뀌고 서버 코드는 그대로다. `overlay2-r1`(클라이언트 2, 노드 101, NPC 10, 준비 20초, 측정 30초, 트레이스 끔)이 종료 코드 0으로 끝났고, 자동 스크린샷(3인칭, 내려다보기)에 두 줄이 찍혔다. 수치는 `frames` 680, `work_avg_ms` 1.961, `out_bytes_per_sec_per_conn` 4812, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000이다. 같은 규모의 `smoke9-r1`(`frames` 897, `work_avg_ms` 1.996, `out_bytes_per_sec_per_conn` 5430)과 비교하면 `frames`가 적다. `smoke9-r1`은 서버 마스크 255, 클라이언트 `-log`였으므로 조건이 다르고 원인은 확인하지 않았다. 앞선 `overlay1-r1`(흰 글자 화면 메시지)은 측정 끝 시각에 클라이언트 선호도가 다시 설정되어 실패 처리됐으므로(`FAIL: processor affinity was re-applied at 23:01:27`, `calib-d-r1`과 같은 유형) 수치를 쓰지 않는다. 내려다보기 화면에서는 상자가 왼쪽 위(640×360 창에서 약 470×68픽셀)를 가려 그 안의 점이 흐리게 보인다. README의 스크린샷 두 장을 `overlay2-r1`의 것(3인칭 순번 01, 내려다보기 순번 00)으로 바꾸고 화면 글자 설명을 고쳤다(사용자 요청). 내려다보기 순번 00은 고갈된 노드의 검은 점이 함께 보여서 골랐다. 태스크 9.3에서 확정 규모 이미지로 다시 바꾼다.

태스크 8.5\~8.7을 끝냈다(2026-10-01). 에이전트가 Insights로 `calib-f-r1.utrace`를 직접 열어 `Lab_MeasureStart`\~`Lab_MeasureEnd` 구간의 값을 읽었고, 사용자가 판단했다. 화면 캡처와 단계별 근거는 [insights-walkthrough-calib-f.md](Guides/insights-walkthrough-calib-f.md)에 있다.

- **가장 큰 비용은 네트워크다(사용자 판단).** `WorldTick` 59.84초 중 `GameNetDriver`가 57.45초(96.01%)다. 패킷 수신(`UNetConnection_ReceivedPacket`)은 2.40%, 액터 틱(`TickCompletionEvents`)은 1.13%다.
- **리플리케이션 시간 타이머는 `GameNetDriver`다(사용자 확정).** 프레임당 57.45초 ÷ 356 = 161.4ms로 CSV `netflush_avg_ms` 161.744와 0.2% 차이다. 그 안에서 `LabResourceNode`가 30.05초(52.30%, 14,242,848회), `LabNpc`가 5.58초(9.72%, 854,400회), `GameNetDriver` 자체(Excl)가 21.51초(37.4%)다.
- **서버 프레임 시간.** 선택 구간 60.018초 ÷ `Frame` 357 = 168.1ms로 CSV `work_avg_ms` 168.309와 0.1% 차이다.
- **연결당 송신량(서버, Connection 0, Outgoing, 2,132패킷, 59.870초).** 내용 기준 (`Actor` 15,713,136비트 + `PacketHeaderAndInfo` 176,608비트) ÷ 8 ÷ 59.870 = 33,176바이트/초, 패킷 크기 기준 상한 2,132 × 1,023 ÷ 59.870 = 36,430바이트/초다. CSV `out_bytes_per_sec_per_conn` 34,164가 그 사이에 있다. 비트의 76.0%가 `LabNpc`(11,945,853비트)이고 `LabResourceNode`는 576비트(9회)다.
- **기준선을 출발값 규모 그대로 확정했다.** 네 조건이 모두 예다("기준선 조건" 표). 구현 계획 8.6의 표에서 이 경우의 선택지는 "이 규모로 확정한다" 하나다.
- **Insights를 여는 방법과 읽는 순서를 지침으로 만들었다(사용자 요청).** `Scripts/open-insights.ps1`, [insights-reading.md](Guides/insights-reading.md). AGENTS.md의 역할 분담을 고쳤다: 에이전트가 Insights를 열어 값을 읽고 판단에 도움이 되는 의견을 내며, 판단과 "관찰", "선택" 섹션은 사용자가 한다.
- **확인하지 않은 것.** `FWindowsPlatformFile_IterateDirectoryCommon_WithCallback`이 측정 구간에 15초(16,216회) 찍혔다. `WorldTick` 아래에는 없어서 게임 스레드의 프레임 시간에는 들어 있지 않다. 어느 스레드인지, 무엇 때문인지는 보지 않았다. `GameNetDriver`의 Excl 21.51초의 내용도 이 트레이스로는 알 수 없다.

태스크 9.1\~9.3을 끝냈다(2026-10-01). README를 확정값으로 고치고 [테스트베드 포스팅 초안](../Posts/00-testbed/README.md)을 썼다. 화면 글자를 바꾼 뒤(`ALabHUD`)의 확정 규모 스크린샷이 없어서 확정 명령으로 `calib-g-r1`을 한 번 실행했다. 종료 코드 0, `frames` 318, `work_avg_ms` 188.412, `saturated_ratio` 0.000이고 측정 시작 뒤의 선호도 재설정이 없었다. `Posts/00-testbed/images/`의 `tpp.png`, `topdown.png`를 이 실행의 순번 02로 바꿨다(두 장 모두 `nodes=5001 npcs=300`, 내려다보기 화면은 가장자리까지 점이 참). `timing.png`, `network.png`는 에이전트가 찍은 `calib-f-r1`의 Insights 캡처다.

- **`calib-g-r1`은 `calib-f-r1`보다 12% 느리다**(`work_avg_ms` 188.412와 168.309, `frames` 318과 356). 30Hz 환산 송신량은 30,488 × 30 ÷ (318 ÷ 60) = 172,574로 같다. 달라진 조건은 클라이언트의 화면 글자(`ALabHUD`)와, 실행 중에 에이전트가 문서를 편집하고 있었다는 것이다. 어느 쪽 때문인지, 실행 사이의 흔들림인지는 한 번의 실행으로 알 수 없다. 기준선 3회 측정의 변동 폭으로 본다. 기준선 3회 측정 중에는 에이전트가 다른 작업을 하지 않는다.
- 테스트베드 포스팅은 초안이다. 태스크 9.4의 8개 창 전체 화면과 걷는 영상이 남았고, 9.5의 태그는 그 뒤에 만든다.

수동 조작에 달리기를 넣었다(2026-10-02, 사용자 요청). 왼쪽 Shift를 누르는 동안 걷기(500cm/s)의 두 배인 1,000cm/s로 달린다. 새 클래스 `ULabCharacterMovement`가 달리기 여부를 저장된 이동의 압축 플래그 한 비트로 서버에 보내므로 송신량과 리플리케이트되는 속성은 그대로다. 입력은 에디터 에셋 없이 코드에서 만든다. 근거는 [engine-notes.md](Planning/engine-notes.md) 마절에 있다. 사용자 지시로 워크트리(`feat/sprint` 브랜치)에서 작업하고 빌드한 뒤 main에 병합했다. 측정 경로가 바뀌지 않았는지 작은 규모로 `sprint-a-r1`(클라이언트 2, 노드 101, NPC 10, 준비 20초, 측정 30초, 트레이스 끔)을 실행했다. 종료 코드 0이고 `frames` 792, `work_avg_ms` 1.936, `out_bytes_per_sec_per_conn` 5143, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000이다. 같은 규모의 직전 실행 `vis-b-r1`(`frames` 897, `work_avg_ms` 2.113, `out_bytes_per_sec_per_conn` 5396, `open_actor_channels_per_conn` 118)과 비교하면 `frames`가 적고, 이 규모의 실행 사이에서 이미 680\~898로 흔들린 범위(`overlay2-r1` 680, `smoke7-r1` 898) 안이다. 로그의 경고 종류는 `vis-b-r1`과 같다. 사용자가 워크트리 빌드의 `run-manual.ps1`로 Shift 달리기가 잘 되는 것을 확인했다(2026-10-02). 그 뒤 워크트리를 등록 해제하고 `feat/sprint` 브랜치를 지웠다. 이 작업의 빌드(00:11:26\~00:12:20)가 같은 시각에 main에서 돌던 `vis-e-r2`의 측정 구간(00:11:11\~00:12:11)과 약 45초 겹쳤다. `vis-e-r2`의 `frames` 259는 `vis-e-r1`의 328보다 낮고, 이 실행의 수치는 쓰지 않는다. main의 `Binaries/`는 아직 달리기를 넣기 전의 빌드다.

내려다보기 화면에 플레이어 점을 넣었다(2026-10-02, 사용자 요청). 노드, NPC와 같은 `DrawDebugPoint`로 자기 폰은 흰색, 다른 플레이어는 파란색 9픽셀 점을 그리고, 둘째 줄 화면 글자에 `players=N`(이 클라이언트에 존재하는 `ALabCharacter` 수)을 더했다. 클라이언트 화면만 바뀌고 서버 코드는 그대로다. 모양을 구분하려고 `ALabHUD`에 원, 사각형, 삼각형을 그리는 방식을 먼저 시도했으나, 사용자가 화면에서 흔들려 보인다고 해서 폐기했다. 그 빌드로 돈 `vis-f-r1`의 수치는 쓰지 않는다. 폐기 후 빌드(달리기가 들어간 main 소스)로 확정 규모를 트레이스 없이 실행한 `vis-g-r1`이 종료 코드 0으로 끝났다: `frames` 352, `work_avg_ms` 170.188, `out_bytes_per_sec_per_conn` 33816, `open_actor_channels_per_conn` 5314, `saturated_ratio` 0.000. 같은 조건의 `vis-e-r1`(`frames` 328, `work_avg_ms` 183.283)보다 `frames`가 많고, 이 규모의 실행 사이에서 흔들린 범위(`calib-g-r1` 318\~`vis-f-r1` 369) 안이다. 내려다보기 자동 스크린샷 다섯 장 모두 흰 점이 화면 중앙에 찍혔고 `players=1`이었다. 플레이어 자리는 반지름 500m 원 위의 16자리라 이웃 자리 간격이 2 × 500 × sin(π/16) ≈ 195m(계산값, `LabScenarioConfig.h`의 `PlayerRingRadius`, `NumPlayerSlots`)이고, 캐릭터는 `bAlwaysRelevant`가 아니어서 컬 거리 150m(`Actor.cpp:312`) 밖의 다른 플레이어는 이 클라이언트에 오지 않는다. 그래서 현재 시나리오에서는 파란 점이 보이지 않는다.

## 다음 할 일

1. **[사람] 태스크 9.4.** 8개 창 전체 화면과 걷는 영상을 넣고, 테스트베드 포스팅 초안을 읽고 다듬는다("사용자에게 요청한 일").
2. 태스크 9.5(포스팅 진행표를 "완료"로, 커밋과 `post-00-testbed` 태그).
3. 태스크 10.1(기준선 3회 측정): 선호도 재설정 실패로 멈춰 있다("막힌 것"). 사용자가 방침을 정하면 아래 "명령"의 시나리오 명령에 `-Label baseline3 -Runs 3`을 더해 실행한다.
4. 푸시는 사용자가 정한 시점에 한다.

## 포스팅 진행

| 포스팅 | 상태 | 태그 |
| --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 초안(태스크 9.4, 9.5 남음) | |
| 1. 무법지대 측정 | 시작 전 | |
| 2. 관련성과 컬 거리 | 시작 전 | |
| 3. 자원 노드 휴면 | 시작 전 | |
| 4. AI NPC 업데이트 빈도 | 시작 전 | |

## 명령

태스크 8에서 확정했다(2026-10-01).

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (2026-10-01 성공 확인. 에디터가 열려 있으면 DLL 잠금으로 실패한다)
- 시나리오 실행(확정 규모, `-Label`과 `-Runs` 없이): `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Clients 8 -Nodes 5000 -Npcs 300 -Warmup 30 -Measure 60`. 측정할 때는 `-Label <새 라벨> -Runs 3`을 더한다. 서버 마스크는 스크립트 기본값 252(논리 프로세서 2\~7)이고 트레이스는 켜진다. `calib-f-r1`이 이 조건의 실행이다.
- 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 측정 중에는 클라이언트 창에 키 입력을 하지 않고, Insights 분석이나 빌드 같은 무거운 작업을 하지 않는다. 서버만 논리 프로세서 2\~7에 고정하므로 다른 프로그램은 그 코어를 쓸 수 있다. 에디터가 열려 있으면 스크립트가 실행을 거부한다.
- Insights: `powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label <라벨>-rN`. 읽는 순서는 [insights-reading.md](Guides/insights-reading.md)에 있다.
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

### 보정 실행

모두 클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 트레이스 켬이다. 실행마다 한 번씩이라 중앙값이 없다. 실패한 실행의 수치는 적지 않는다. 서버 마스크는 따로 적지 않은 실행이 255(논리 프로세서 0\~7)이고, 클라이언트 `-log`는 `diag-d-r1`부터 없다. 확정 조건(서버 마스크 252, `-log` 없음, 한도 350,000)의 실행은 `diag-d-r1`과 `calib-f-r1`이다.

| 라벨 | net_speed | 결과 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `calib-a-r1` | 100000 | 성공 | 358 | 167.475 | 452.948 | 358 | 157.255 | 19854 | 5314 | 1.000 |
| `calib-b-r1` | 10000000 | 성공 | 100 | 602.178 | 818.538 | 100 | 535.312 | 9591 | 5314 | 0.000 |
| `calib-c-r1` | 350000 | 실패(측정 중 선호도 재설정) | | | | | | | | |
| `calib-d-r1` | 350000 | 실패(측정 중 선호도 재설정) | | | | | | | | |
| `calib-e-r1` | 350000 | 성공 | 293 | 204.726 | 462.207 | 293 | 195.441 | 28121 | 5314 | 0.000 |
| `diag-a-r1` | 350000 | 성공(진단, 서버 마스크 255) | 177 | 339.134 | 872.006 | 177 | 311.294 | 16957 | 5314 | 0.000 |
| `diag-b-r1` | 350000 | 성공(진단, 서버 마스크 255) | 366 | 163.988 | 279.922 | 366 | 157.555 | 35061 | 5314 | 0.000 |
| `diag-c-r1` | 350000 | 성공(진단, 서버 마스크 2) | 90 | 670.713 | 841.178 | 90 | 594.193 | 8589 | 5314 | 0.000 |
| `diag-d-r1` | 350000 | 성공(진단, 서버 마스크 252) | 340 | 176.630 | 216.841 | 340 | 169.013 | 32540 | 5314 | 0.000 |
| `diag-e-r1` | 350000 | 성공(진단, 서버 마스크 2) | 153 | 392.972 | 1088.979 | 153 | 360.362 | 14675 | 5314 | 0.000 |
| `calib-f-r1` | 350000 | 성공(서버 마스크 252) | 356 | 168.309 | 245.826 | 356 | 161.744 | 34164 | 5314 | 0.000 |
| `calib-g-r1` | 350000 | 성공(서버 마스크 252, 새 화면 글자, 확정 명령) | 318 | 188.412 | 251.103 | 318 | 179.494 | 30488 | 5314 | 0.000 |

## 측정 결과

CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |

## 포스팅 주기 진행

구현 계획 태스크 11은 포스팅 2, 3, 4에 반복해서 쓴다. 현재 포스팅과 끝낸 단계를 여기에 적는다.

- 현재 포스팅: 없음
- 끝낸 단계: 없음

## 막힌 것

- **태스크 10.1이 선호도 재설정 실패로 두 번 멈췄다.** 두 번 모두 클라이언트 프로세스의 선호도가 측정 시작 뒤에 전체 코어로 돌아가 있었다. 실패한 묶음의 수치는 쓰지 않는다. 라벨 `baseline`, `baseline2`는 이미 썼으므로 다음 측정은 `baseline3`으로 한다.
  - `baseline`: `baseline-r1` 성공, `baseline-r2` 실패(측정 시작 23:52:09, 재적용 23:52:40, 클라이언트 0), r3 실행 안 됨.
  - `baseline2`: `baseline2-r1` 실패(측정 시작 23:54:54, 재적용 23:55:55, 클라이언트 0). CSV 행의 시각이 23:55:54라서 재적용은 측정 구간이 끝난 직후다.
  - `-Warmup`을 늘릴지는 사용자가 정한다("사용자에게 요청한 일").

## 사용자에게 요청한 일

- **기준선 3회 측정에서 선호도 재설정 실패가 반복됐다(`baseline-r2`, `baseline2-r1`). `-Warmup`을 늘릴지 정해 준다.** 내용은 "막힌 것"에 있다. 기준선 확정은 사용자가 확인했다(출발값 규모 그대로).
- **태스크 9.4.** 확정 명령으로 실행하는 동안 8개 창이 모두 보이는 전체 화면을 찍어 `Posts/00-testbed/images/all-clients.png`로, 클라이언트가 걷는 10초 영상을 `Posts/00-testbed/images/clip.gif`(또는 `.mp4`)로 넣는다. 본문에 자리 표시 주석이 있다. `timing.png`, `network.png`는 에이전트가 찍은 것을 넣어 두었으니 다른 장면이 좋으면 바꾼다. 테스트베드 포스팅 초안을 읽고 다듬는다.
- **선호도 재설정 실패가 두 번 있었다(`calib-c-r1`, `calib-d-r1`).** 그 뒤 일곱 번의 실행(`calib-e-r1`, `diag-a-r1`\~`diag-e-r1`, `calib-f-r1`)에서는 다시 나오지 않았다. 기준선 3회 측정에서 다시 나오면 `-Warmup`을 늘릴지 정한다(구현 계획 7.3의 표).
- ADR-0001\~0008의 내용을 읽고 확인한다. 이미 확정된 결정을 옮긴 것이라 상태는 "승인됨"으로 적었다. 고칠 곳이 있으면 알려 준다.
- 푸시는 사용자가 정한 시점에 한다.
