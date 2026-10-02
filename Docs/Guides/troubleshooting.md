# 문제 해결

빌드나 실행에서 이미 겪은 문제를 증상으로 찾는 문서다. 문제가 생겼을 때만 연다. 증상과 대처만 적고, 원인과 근거는 가리키는 문서에 있다. 새 문제를 겪으면 같은 형식으로 한 줄을 더한다.

| 증상 | 확인할 것과 대처 | 근거 |
| --- | --- | --- |
| `Scripts/common.ps1`이 엔진을 찾지 못한다 | 그 스크립트를 실행하는 프로세스에서 `reg query "HKCU\Software\Epic Games\Unreal Engine\Builds"`로 `UE_DSOptLab`이 보이는지 확인한다. Claude 앱 안의 에이전트 프로세스에서는 레지스트리 편집기와 다른 값이 보인 적이 있다. 없으면 사용자에게 알리고 승인을 받아 그 프로세스에서 등록한다 | [Worklog/01-baseline.md](../Worklog/01-baseline.md) "풀린 문제: 에이전트 프로세스에서 엔진 등록이 다르게 보임". 원인은 확인하지 않았다 |
| 빌드가 DLL 잠금으로 실패한다 | 에디터가 열려 있는지 확인하고 닫는다 | [STATUS.md](../STATUS.md) "명령" |
| `run-scenario.ps1`이 `FAIL: processor affinity was re-applied`로 끝난다 | 그 실행의 수치를 쓰지 않고 라벨을 바꿔 다시 실행한다. `-Warmup`은 바꾸지 않는다 | [Worklog/01-baseline.md](../Worklog/01-baseline.md) "태스크 10.1". 원인은 모른다 |
| 녹화한 `visualN` 실행이 `FAIL: processor affinity was re-applied`(종료 코드 1)로 끝난다 | `visualN`은 수치를 쓰지 않으므로 미리보기에서 원하는 장면이 담겼으면 다시 실행하지 않는다 | `visual2-r1`(측정 시작 38초 뒤), `visual3-r1`(15초 뒤). 두 번 모두 측정 구간에 녹화했다. 녹화와 관련이 있는지는 확인하지 않았다 |
| 녹화나 화면 조작이 되지 않고 화면 크기가 1728×1084로 잡힌다(두 클라이언트 영상의 2번 창이 화면 밖으로 잘린다) | `qwinsta`로 세션 상태를 본다. 원격 데스크톱으로 접속 중이면 화면이 원격 창 크기가 되고, 원격 창을 닫기만 하면 세션이 `Disc`가 되어 화면이 그려지지 않는다. 사용자에게 본체에 로그인해 달라고 요청한 뒤(3840×2160, 150%) 진행한다 | [Worklog/03-dormancy.md](../Worklog/03-dormancy.md) "태스크 11.5\~11.10" |
| 에이전트의 `Stop-Process`(`UnrealEditor` 종료)가 권한 분류기에서 거부된다 | 실행은 스스로 정리하는 `run-scenario.ps1`로만 한다. `run-manual.ps1`의 실행이나 남은 프로세스는 사용자에게 종료를 요청한다(수동 실행은 그 창에서 Enter) | [Worklog/04-update-frequency.md](../Worklog/04-update-frequency.md) "태스크 11.5\~11.10" |
| 같은 라벨로 다시 실행하면 스크립트가 거부한다 | 정상 동작이다. 실패한 실행의 로그와 트레이스가 남아 있어서다. 새 라벨을 쓴다 | [engine-notes.md](../Planning/engine-notes.md) 바절 "트레이스 인자" |
| 클라이언트가 시작 직후 `Assertion failed: RefCount.load(...)`로 죽는다 | 시작 신호 전이면 스크립트가 다시 띄운다(실행당 최대 3번, 출력의 `RESTART:` 줄). 따로 할 일은 없다 | engine-notes.md 마절 "실행하면서 고친 것" |
| 서버가 `Run failed: connection count changed after start`로 끝난다 | 측정 중에 클라이언트 창이 닫힌 것이다(`smoke6-r1`은 Alt-F4). 수치를 쓰지 않고 새 라벨로 다시 실행한다 | [Worklog/00-testbed.md](../Worklog/00-testbed.md) "실행 인자 `-DisablePython`" |
| 같은 구성인데 서버가 몇 배 느리다(`frames`가 100 안팎) | 서버 마스크가 252(논리 프로세서 2\~7)인지 확인한다. 1번 프로세서는 DPC에 CPU를 빼앗긴다 | engine-notes.md 사절, [ADR-0009](../Decisions/0009-server-cores-without-dpc-load.md) |
| 측정 수치가 같은 구성의 다른 실행보다 나쁘다 | 측정 구간에 다른 체크아웃의 빌드나 실행이 겹쳤는지 시각으로 확인한다. 겹쳤으면 수치를 쓰지 않는다 | Worklog/00-testbed.md "수동 조작에 달리기"(`vis-e-r2`) |
| 실행 로그에 `LogPython: Error`가 나온다 | 실행 인자에 `-DisablePython`이 있는지 확인한다. `run-scenario.ps1`과 `run-manual.ps1`에는 들어 있다 | engine-notes.md 마절 |
| PowerShell 스크립트의 한글이 깨지거나 구문 오류가 난다 | 스크립트를 UTF-8 BOM으로 저장한다 | engine-notes.md 라절 |
