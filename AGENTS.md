# UE Dedicated Server 포트폴리오

언리얼 엔진 5.8.3 Dedicated Server 최적화 과정을 기법 하나당 포스팅 하나로 보여주는 포트폴리오다. 포스팅 0\~4가 1막이고 끝났다(2026-10-02). 포스팅 5부터가 2막이다. 2막은 테스트베드를 한 번 확장해 새 기준선을 잡고, 1막의 세 기법을 약식으로 다시 적용한 뒤 새 기법을 하나씩 다룬다([2막 설계](Docs/Planning/2026-10-03-act-2-design.md), [2막 구현 계획](Docs/Planning/2026-10-03-act-2-implementation-plan.md)). 그 뒤의 순서는 [backlog.md](Docs/backlog.md)의 우선순위 순이다.

## 세션을 시작하면

1. [Docs/STATUS.md](Docs/STATUS.md)를 읽는다. 현재 진행 중인 작업과 다음 할 일이 있다.
2. STATUS.md의 "단계", "막힌 것", "사용자에게 요청한 일"에 이미 끝나고 커밋된 태스크의 기록이 남아 있으면, 다음 태스크를 시작하기 전에 정리안을 채팅에 보여 주고 승인을 받는다. 정리안은 옮길 항목, 옮길 파일, STATUS.md에 남길 한 줄, 지침으로 올릴 문장과 그 위치의 목록이다. 승인되면 정리하고 따로 커밋한 뒤 태스크를 시작한다. 기준은 아래 규칙 "STATUS.md에는 지금 유효한 것만 둔다"에 있다.
3. STATUS.md의 "다음 할 일"에서 첫 항목을 고른다. 비어 있으면 [backlog.md](Docs/backlog.md)의 "우선순위 순"에서 다음 주제를 사용자에게 묻는다.
4. 그 단계에 필요한 문서만 아래 "문서 지도"에서 찾아 연다. 결정을 바꾸고 싶어지면 먼저 [ADR 목록](Docs/Decisions/README.md)을 본다.

## 문서 지도

| 알고 싶은 것 | 볼 곳 |
| --- | --- |
| 진행 상황, 다음 할 일, 빌드와 실행 명령, 확정값, 측정 결과 | [Docs/STATUS.md](Docs/STATUS.md) |
| 끝낸 작업의 경위, 실패한 실행, 보정 실행 수치 | `Docs/Worklog/NN-이름.md`(포스팅별, `Posts/`의 폴더 이름과 같다). 세션을 시작할 때 읽지 않고 라벨이나 날짜로 검색한다 |
| 다음 작업 | STATUS.md "다음 할 일", 그다음 [backlog.md](Docs/backlog.md) "우선순위 순" |
| 확정된 결정과 그 이유, 버린 대안 | [Docs/Decisions/](Docs/Decisions/README.md)(ADR), ADR이 없는 초기 결정은 그 README의 "ADR 이전의 결정" |
| 측정 규약: 시나리오, 절차, 지표, 한계 | [measurement.md](Docs/Guides/measurement.md) |
| 수치를 부르는 이름 | measurement.md "수치의 이름과 출처" |
| 시각 자료의 종류와 파일 이름 | [posting.md](Docs/Guides/posting.md) "시각 자료" |
| 포스팅의 독자와 분량, 틀, 문장 규칙, 본문과 측정 기록의 구분 | posting.md "독자와 분량", "틀", "문장 규칙", "본문과 측정 기록". 따라 쓸 본보기는 [Relevancy 글](Posts/02-relevancy/README.md)과 그 [측정 기록](Posts/02-relevancy/measurements.md) |
| 엔진 기본값, 동작 순서, 소스 위치, 실행에서 확인한 사실 | [engine-notes.md](Docs/Reference/engine-notes.md) |
| 빌드나 실행이 실패했을 때의 증상별 대처 | [troubleshooting.md](Docs/Guides/troubleshooting.md). 문제가 생겼을 때만 연다 |
| Insights로 트레이스를 여는 법과 읽는 순서, 역할 분담 | [insights-reading.md](Docs/Guides/insights-reading.md). 여는 명령은 `Scripts/open-insights.ps1 -Label <라벨>-rN`, 창 없이 Timing 값을 내보내는 것은 `Scripts/export-insights.ps1 -Label <라벨>-rN`, 문서용 캡처는 `Scripts/capture-insights.ps1`, 캡처에 번호 붙은 상자를 그리는 것은 `Scripts/annotate-image.ps1` |
| 클라이언트 화면 영상과 전체 화면 캡처 | `Scripts/capture-video.ps1`(사용법은 파일 머리 주석). 찍기 전 허가와 측정 분리는 아래 규칙 "화면을 찍기 전에 허가를 받는다" |
| Insights 화면을 단계별로 읽은 예(캡처와 행동마다의 이유) | [insights-walkthrough-calib-f.md](Docs/Guides/insights-walkthrough-calib-f.md) |
| 다음 주제의 우선순위, 작업 중 떠오른 기법 | [backlog.md](Docs/backlog.md) |
| 측정 PC 사양 | [pc-specs.md](Docs/Reference/pc-specs.md) |
| 대상 채용공고 | [job-posting-ue5-dedicated-server.md](Docs/Reference/job-posting-ue5-dedicated-server.md) |
| 2막의 목적, 확장 요소, 포스팅 계획과 태스크 15부터의 순서 | [2막 설계](Docs/Planning/2026-10-03-act-2-design.md), [2막 구현 계획](Docs/Planning/2026-10-03-act-2-implementation-plan.md) |
| 끝난 단기(1막) 설계 문서와 구현 계획(태스크 1\~14가 가리키는 곳) | 태그 `post-04-update-frequency`의 [설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md), [구현 계획](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-implementation-plan.md). 2026-10-03에 레포에서 지웠다 |
| 공개 소개, 코드 파일별 역할 | [README.md](README.md) |
| 포스팅 본문과 이미지 | `Posts/NN-이름/README.md`, `Posts/NN-이름/images/` |
| 포스팅 수치의 근거(정밀한 값, 실행 라벨, 계산식, 세 실행과 CSV 표, Insights 캡처, 엔진 소스 위치) | `Posts/NN-이름/measurements.md` |
| 루트 README의 누적 수치의 근거(구성별 중앙값, 기법별 변화, 차트) | [Posts/measurements.md](Posts/measurements.md). 루트 README도 포스팅과 같은 문장 규칙으로 쓰고 실행 라벨과 계산식을 여기에 둔다 |
| 후보 기법 자료(Insights에서 읽은 값, 엔진 소스 위치, 에이전트 의견) | `Posts/NN-이름/candidates.md` |
| 개념도 SVG를 만드는 스크립트 | `Scripts/make-<이름>.ps1`(예: `Scripts/make-relevancy-map.ps1`) |
| 시리즈 웹 페이지(UE Dedicated Server, 단계별로 최적화해 보기)와 그 수치의 출처, 배포 | `Site/index.html`(의존성 없는 한 파일), [Site/README.md](Site/README.md), `.github/workflows/pages.yml`. main의 `Site/`가 바뀐 채로 푸시되면 `https://hon454.github.io/ue-dedicated-server-optimization-lab/`에 공개된다 |
| 실행 산출물 | 수치 CSV `Saved/LabMetrics/summary.csv`(`config` 열을 더하기 전인 1막의 행은 `summary-act1.csv`), 트레이스 `Saved/Traces/<라벨>-rN.utrace`, 스크린샷 `Saved/Screenshots/Lab/`, 로그 `Saved/Logs/` |

새 문서를 만들거나 옮기면 이 표를 함께 고친다.

## 규칙

- **개발 속도가 최우선이다.** 단위 테스트, CI, PR, 외부 이슈 트래커를 쓰지 않는다. main에 직접 커밋하거나, 워크트리 브랜치에서 작업하고 main에 병합한다. 예외는 GitHub Pages 배포 워크플로 하나다. 테스트나 빌드를 돌리지 않고 `Site/` 폴더를 올리기만 한다(저장소의 Pages Source를 "GitHub Actions"로 정했다. 2026-10-03 사용자 결정).
- **워크트리는 여러 세션이 동시에 코드나 문서를 고칠 때 쓴다.** 모든 체크아웃이 측정 PC 한 대의 CPU를 나눠 쓰므로 다음을 지킨다.
  - 포스팅과 STATUS.md의 비교에 쓰는 측정은 main 체크아웃에서 한다. `Saved/`는 체크아웃마다 따로라서, 워크트리에서 측정한 행은 main의 `Saved/LabMetrics/summary.csv`에 남지 않는다. 워크트리에서는 작은 규모로 동작만 확인한다.
  - 빌드와 실행은 다른 체크아웃의 실행이 떠 있으면 하지 않고, 끝날 때까지 기다린다. `build.ps1`과 `run-scenario.ps1`은 `UnrealEditor`가 떠 있으면 거부한다. 반대로 실행 중에 시작되는 다른 체크아웃의 빌드는 막지 못하므로, 측정 중에는 다른 세션에서 빌드하지 않는다.
  - 병합 때 STATUS.md가 충돌하면 양쪽 내용을 모두 남긴다. 병합한 뒤에는 워크트리를 등록 해제하고 브랜치를 지운다.
- **STATUS.md에는 지금 유효한 것만 두고 150줄을 넘기지 않는다.** 세션마다 통째로 읽는 파일이라 길어지면 다음 할 일이 묻힌다. 끝낸 태스크의 경위(무엇을 시도했고 어떤 실행이 실패했는지, 그 수치)는 다음 태스크를 시작할 때 `Docs/Worklog/<포스팅 폴더 이름>.md`의 끝에 `## 날짜 태스크 번호: 한 일 (실행 라벨)` 제목으로 원문 그대로 옮긴다. 포스팅에 속하지 않는 작업은 그때 진행 중이던 포스팅의 파일에 적는다. STATUS.md에는 다음 작업에 영향을 주는 결과만 한두 줄로 남기고 옮긴 제목으로 링크한다. 실패한 시도는 다음 작업을 제약하는 동안만 한 줄로 남긴다. 정리할 때 남길 줄마다 성격을 가린다. 포스팅과 상관없이 계속 지켜야 하는 것은 지침으로 올린다: 빌드와 측정의 절차는 STATUS.md의 "명령"에, 그 밖의 규칙은 이 파일의 "규칙"에 넣는다. 다시 나올 수 있는 문제는 증상과 대처를 [troubleshooting.md](Docs/Guides/troubleshooting.md)에 적는다. 엔진 소스나 실행에서 확인한 사실은 engine-notes.md에 적고 troubleshooting.md에서 그 절을 가리킨다. 다음 작업에만 영향을 주는 것만 "단계"에 남긴다. `Docs/Worklog/`의 지난 항목은 고치지 않고, 세션을 시작할 때 읽지 않는다.
- **커밋 메시지는 [Conventional Commits](https://www.conventionalcommits.org/) 형식으로 쓴다.** 제목은 `type(scope): summary` 한 줄이고, summary는 소문자로 시작하는 영어 명령형이다. type은 `feat`, `fix`, `docs`, `build`, `refactor`, `chore` 중에서 고르고 scope는 필요할 때만 붙인다. 제목만으로 무엇을 왜 바꿨는지 분명하면 제목 한 줄로 끝낸다. 그렇지 않으면 빈 줄 하나 뒤에 영어 본문을 붙여 제목에 담지 못한 것을 적는다: 바꾼 이유, 달라진 동작, 근거가 된 실행 라벨이나 엔진 소스 위치. 측정 조건이나 설정값을 바꾼 커밋, 실패 원인을 기록하는 커밋은 본문을 쓴다.
- **확정된 결정을 다시 열지 않는다.** [ADR 목록](Docs/Decisions/README.md)의 "ADR 이전의 결정"과 승인된 ADR의 결정을 바꾸자고 제안하지 않는다. 사용자는 계획 단계로 되돌아가는 일을 반복해 왔다. 예외는 결정의 근거가 사실과 맞지 않음을 확인한 경우 하나다. 근거는 엔진 소스(파일과 줄)나 실행 결과(라벨)여야 한다. 이때는 대체 ADR을 "제안됨"으로 쓰고, 그 결정에 기대는 측정을 멈추고, 사용자에게 알린다. 더 나은 방법이 있다는 판단은 예외가 아니다. 그런 생각은 [backlog.md](Docs/backlog.md)에 적는다.
- **질문에는 답과 수정안만 낸다.** 사용자가 "\~하는 게 낫지 않아?", "\~필요하지 않을까?"처럼 질문형으로 물으면, 의견과 수정안(바꿀 파일과 문장)을 채팅에 보여 주고 편집과 커밋은 하지 않는다. "고쳐", "반영해" 같은 지시를 받거나 수정안이 승인된 뒤에 구현한다. 근거를 찾으려고 파일을 읽거나 검색하는 것은 해도 된다. 사용자는 질문으로 방향을 먼저 맞춰 본 뒤에 결정한다.
- **선택지를 낼 때는 추천안을 함께 낸다.** 어느 선택지를 추천하는지 먼저 밝히고, 이유를 이해하기 쉬운 한두 문장으로 쓴다. 이유는 이 프로젝트의 사실(측정값, 엔진 소스, 확정된 결정, 작업량)에 기댄다. 다른 선택지를 고르면 무엇을 얻고 무엇을 잃는지도 적는다. 선택은 사용자가 한다.
- **포스팅 하나에 기법 하나.** 여러 기법을 함께 넣으면 어느 것이 효과를 냈는지 알 수 없다. 1막에서는 관련성, 자원 노드 휴면, NPC 업데이트 빈도 세 기법을 구현했다. 기법 없이 측정만 하는 포스팅(환경을 설명하는 포스팅, 이미 다룬 기법을 새 기준선에 다시 적용하는 포스팅, 진단 포스팅)을 둘 수 있다. 다음 기법은 [backlog.md](Docs/backlog.md)에서 사용자가 고르고, 작업 중 떠오른 다른 기법은 진행 중인 포스팅에 넣지 않고 backlog.md의 "작업 중 떠오른 것"에 적는다.
- **포스팅 폴더에 본문을 처음 커밋할 때 루트 README의 "포스팅" 표에 그 글의 행을 더한다.** 태그를 붙이기 전에는 제목 뒤에 "(작성 중)"을 적고, 태그를 붙이는 커밋에서 뺀다. README의 표를 2막 계획의 한 태스크로 미뤄 두었다가, 태그를 붙인 포스팅 5가 README에 보이지 않았다(2026-10-05 사용자 지적). 결과 표와 차트처럼 측정이 끝나야 고칠 수 있는 부분은 따로 갱신해도 된다.
- **패키징하지 않고 null RHI를 쓰지 않는다.** 에디터 빌드 실행 파일을 쿠킹 없이 실행한다([ADR-0002](Docs/Decisions/0002-editor-build-without-packaging.md)).
- **에디터 작업을 만들지 않는다.** 에이전트는 에디터를 다룰 수 없어서, 에디터 작업이 끼면 사람을 기다려야 하고 결과를 스스로 검증하지 못한다. 액터는 C++로 작성하고, 자원 노드와 AI NPC는 실행 시 코드로 생성한다. 블루프린트와 맵 편집이 필요한 설계를 피한다.
- **수치를 정해진 이름으로만 부른다.** CSV 값과 Insights 값은 정의가 달라서, 이름을 섞으면 다른 수치를 같은 것처럼 비교하게 된다([ADR-0004](Docs/Decisions/0004-insights-and-csv-metrics.md)). [measurement.md](Docs/Guides/measurement.md)의 "수치의 이름과 출처" 표를 따른다. CSV 값을 Insights 지표 이름으로 부르지 않고, 열린 액터 채널 수를 액터 수라고 부르지 않는다.
- **모든 수치와 설정값에 근거를 적는다.** 측정값(조건 명시), 엔진 소스 확인값(파일과 심볼 명시), 계산값(식 명시) 중 하나다. 기억에 의존한 엔진 기본값은 5.8.3 소스에서 확인한 뒤에 쓴다. 포스팅에서는 근거를 본문 문장에 넣지 않고 같은 폴더의 `measurements.md`에 둔다. 본문의 수치는 모두 그 파일의 "본문의 수치와 출처" 표에 있어야 한다([ADR-0015](Docs/Decisions/0015-post-body-and-measurement-record.md)).
- **포스팅은 독자가 3\~5분에 읽고 원리를 이해하게 쓴다.** 독자는 UE 네트워킹의 초급에서 중급 사이인 개발자다. 본문은 표와 코드를 뺀 글로 6,000\~8,000자이고([ADR-0017](Docs/Decisions/0017-post-length-range.md)), 직전 구성의 문제, 기법이 그것을 줄이는 원리(도식), 예상, 결과의 순서로 쓴다. 문장은 [posting.md](Docs/Guides/posting.md)의 "문장 규칙"을 따른다: 한 문장에 사실 하나, 80자 이하, 숫자 둘과 괄호 하나까지, 새 용어는 처음 나올 때 정의한다. 사용자가 포스팅 0\~4를 읽고 글이 길고 수치만 나열되어 이해하기 어렵다고 했다(2026-10-03).
- **시각 자료를 적극적으로 모은다.** 측정할 때마다 자동 스크린샷을 직접 열어 보고 포스팅에 쓸 것을 고른다. 본문에는 주장 하나에 시각 자료 하나를 넣고, 원리는 흐름도나 개념도로 그린다(posting.md "시각 자료"). 전후 비교는 이미지를 나란히 놓고, 수치는 Mermaid 차트로도 보여준다. 클라이언트 영상은 에이전트가 `Scripts/capture-video.ps1`로 찍고, 찍은 뒤 미리보기(`Saved/Screenshots/Lab/<이름>-preview.png`)를 열어 확인한다. 영상은 GitHub 마크다운 본문에서 이미지처럼 바로 보이는 GIF(폭 960px, 8fps, 48색. `capture-video.ps1`의 기본값)로 넣는다. 15fps, 256색은 10초에 9\~13MB라 줄였다. 시각 자료는 글이 설명하는 구성의 빌드에서 찍는다. 지난 구성의 화면이 필요하면 그 태그를 빌드해서 찍고, 지금 빌드의 화면으로 대신하지 않는다(테스트베드 포스팅의 `visual11`은 `post-01-baseline`을 빌드해 찍었다).
- **영상은 수치를 쓰는 측정 실행에서 찍지 않는다.** 녹화와 인코딩이 측정 PC의 CPU를 쓰기 때문이다. `Scripts/run-manual.ps1`의 실행이나, 수치를 쓰지 않는 시각 자료 전용 라벨(`visualN`)의 `run-scenario.ps1` 실행에서 찍는다. `capture-video.ps1`은 측정 중인 서버가 있으면 거부하고, `visualN` 실행에서만 `-AllowMeasuring`으로 넘긴다. `visualN`의 수치는 포스팅과 STATUS.md의 비교에 쓰지 않는다. 전후 영상은 자동 이동 클라이언트(정해진 정사각형 경로)를 찍어 같은 경로로 비교한다. 특정 장면을 위해 관찰자를 조작해야 할 때만 그 조작을 사용자에게 요청한다.
- **화면을 찍기 전에 허가를 받는다.** `capture-video.ps1`은 화면을 그대로 받아서 사용자의 다른 창과 개인 정보가 찍힐 수 있다. 실행하기 전에 무엇을(창 제목 패턴이나 영역), 어느 실행에서, 몇 초, 어느 파일 이름으로 찍는지 채팅에 적고 사용자의 승인을 받는다. 여러 개를 찍을 때는 목록을 한 번에 보여 주고 승인받아도 된다. 승인은 그 목록에만 유효하고 다음 녹화로 이어지지 않는다. 창 하나의 내용만 받는 `capture-insights.ps1`은 해당하지 않는다.
- **구조와 이름.** 언리얼 프로젝트는 레포 루트의 `DSOptLab.uproject`다(모듈 `DSOptLab`). 이 프로젝트에서 만드는 클래스, 실행 인자, 저장 폴더의 접두사는 `Lab`이다(`ALabResourceNode`, `-LabNodes=`, `Saved/LabMetrics/`). 폴더 이름은 대문자로 시작한다(`Docs/`, `Posts/`, `Scripts/`). 런처 설치본(`G:\Epic Games\UE_5.8`)으로 프로젝트를 열거나 빌드하지 않는다. 소스 빌드와 같은 `Binaries/`에 번갈아 빌드하게 된다(engine-notes.md 0절).
- **`Scripts/`의 PowerShell 스크립트는 UTF-8 BOM으로 저장한다.** Windows PowerShell 5.1이 BOM 없는 한글을 잘못 읽는다(engine-notes.md 라절).
- **문서에서 `~`는 `\~`로 쓴다.** GFM은 한 문단에 `~`가 둘 이상이면 그 사이를 취소선으로 렌더링한다(`2\~7, 8\~31`). 코드 스팬 안에서는 이스케이프하지 않는다. `~~` 취소선은 쓰지 않는다.
- **포스팅과 루트 README의 용어와 부르는 법(2026-10-02 사용자 결정).** 엔진의 기법 이름은 번역하지 않고 원문으로 쓴다: Relevancy, Dormancy, Net Update Frequency, Net Cull Distance. 엔진 소스에 이름이 있는 단계, 자료구조, 기능도 원문으로 쓴다: Consider List(`ServerReplicateActors_BuildConsiderList`), Push Model, Adaptive Net Update Frequency. 동사로 쓰는 "리플리케이션 대상으로 고려한다"와, 직렬화, 송신, 우선순위 정렬, 거리 검사, 활성 목록처럼 한국어로 자연스러운 용어는 그대로 둔다. 상태는 "Dormant 상태"로 쓴다. 기법 이름은 엔진의 개념을 정의할 때만 쓰고, 한 일은 서버에서 실제로 바꾼 것으로 쓴다. "Relevancy를 적용했다"가 아니라 "`bAlwaysRelevant`를 끄고 150m 거리 판정을 쓰게 했다"로, "Dormancy를 적용했다"가 아니라 "자원 노드를 Dormant 상태로 두었다"로, "Net Update Frequency를 적용했다"가 아니라 "NPC의 Net Update Frequency를 10으로 낮췄다"로 쓴다. 구성과 단계도 바꾼 것으로 부른다(포스팅 6의 "① 거리 판정", "② Dormant 상태", "③ NPC 업데이트 빈도"). 개념 이름을 한 일처럼 쓰면 독자가 무엇을 바꿨는지 알 수 없다(2026-10-05 사용자 지적). 측정 기록과 관찰 자료의 표는 실행 라벨을 따라 기법 이름으로 불러도 된다. 리플리케이션, 대역폭, 액터 채널 같은 일반 용어와 "수치의 이름과 출처"의 지표 이름은 그대로 둔다. 다른 글은 "포스팅 1"이 아니라 제목 링크로 부른다(`[Always Relevant 기준선](../01-baseline/README.md)`). 포스팅 파일 이름은 `README.md`로 둔다(GitHub가 폴더를 열면 바로 그려 준다). `Docs/`의 작업 문서는 지금 용어(관련성, 휴면, 업데이트 빈도, 포스팅 N)를 그대로 쓴다. 자원 노드는 Consider List, 활성 목록, 액터 채널, 거리 검사처럼 엔진 내부 처리를 말하는 문장에서 "노드"로 줄이지 않는다. 화면의 점이나 화면 글자의 개수처럼 자원 노드만 가리키는 것이 분명한 곳에서는 줄여도 된다. Replication Graph의 노드는 "그래프 노드"로 부른다.
- **포스팅과 루트 README는 한국어 한다체(-다)로 쓴다.** 문단과 목록의 문장은 한다체("확인했다")다. 표의 칸은 명사나 "-음"으로 끝나는 개조식("확인", "더 늘지 않음")을 써도 되고, 문단에는 개조식을 쓰지 않는다. 틀은 [posting.md](Docs/Guides/posting.md)를 따른다. 태그 `post-01-baseline`\~`post-04-update-frequency` 시점의 글은 합니다체로 남아 있다(2026-10-02에 한 커밋으로 바꿨다).
- **문서에 일정 표현을 쓰지 않는다.** README, 포스팅, `Docs/` 어디에도 "N일차"나 "N일 안에" 같은 표현을 쓰지 않는다. 단계는 단기 구현 계획의 큰 제목(테스트베드 구축, 규모 확정과 테스트베드 포스팅, 기준선과 첫 번째 기법, 나머지 두 기법, 마무리)과 태스크 번호로 부른다. 태스크 1\~14는 태그 `post-04-update-frequency`의 구현 계획을 가리킨다. 태스크 15부터는 [2막 구현 계획](Docs/Planning/2026-10-03-act-2-implementation-plan.md)을 가리키고, 단계는 그 문서의 큰 제목(기법 전환 인자, 테스트베드 확장, 새 기준선, 세 기법 다시 적용, 2막 포스팅 주기)으로 부른다. 레포가 공개다.

## 결정 기록(ADR)

- 대안을 비교해 내린 결정이 새로 생기면(측정 정의를 고침, 설정 방침을 정함, 구조를 바꿈) [Docs/Decisions/](Docs/Decisions/README.md)에 상태 "제안됨"으로 초안을 쓰고, [Docs/STATUS.md](Docs/STATUS.md)의 "사용자에게 요청한 일"에 승인을 요청한다. 승인은 사용자가 한다.
- 승인된 ADR의 본문은 고치지 않는다. 결정이 바뀌면 새 ADR로 대체한다. 쓰는 법은 [Docs/Decisions/README.md](Docs/Decisions/README.md)에 있다.
- 기법 선택과 포스팅 순서는 ADR로 쓰지 않는다. 사용자가 정하고, 진단하는 포스팅(기준선 글)의 "선택" 섹션에 적는다.

## 완료 확인

작업 하나가 끝났다고 말하기 전에 다음을 확인한다.

1. 빌드가 성공한다.
2. 시나리오 스크립트가 종료 코드 0으로 끝나고 서버가 수치 CSV에 새 행을 남긴다.
3. CSV 수치를 이전 구성과 비교해 [Docs/STATUS.md](Docs/STATUS.md)에 적는다.

문서만 바꾼 작업은 1\~3 대신 바꾼 문서의 상대 링크가 실제 파일을 가리키는지 확인한다. 실패한 실행의 수치는 쓰지 않는다. 같은 구성을 다시 측정할 때는 새 라벨을 쓴다.

빌드와 실행은 [Docs/STATUS.md](Docs/STATUS.md)의 "명령"에 적힌 명령으로만 한다.

## 사람에게 넘기는 일

다음은 직접 하지 않고, 필요한 시점에 무엇을 해야 하는지 구체적으로 적어 사용자에게 요청한다.

- **Insights 분석의 판단과 기법의 선택.** 가장 큰 비용이 무엇인지, 어떤 기법을 어떤 순서로 적용할지는 사용자가 판단한다. 포스팅의 "문제"와 "원리"(진단하는 글의 "선택" 포함)는 에이전트가 그 판단을 옮겨 초안을 쓰고 사용자가 승인한다(2026-10-03 사용자 결정, [ADR-0015](Docs/Decisions/0015-post-body-and-measurement-record.md). 그 전에는 "관찰"과 "선택"을 사용자가 직접 쓰는 규칙이었다). 에이전트는 Insights를 직접 열어 [insights-reading.md](Docs/Guides/insights-reading.md)의 순서로 값을 읽고, CSV와 대조한 표와 후보 기법 목록(각 기법의 구현 비용과 관련 엔진 소스 위치)을 준비한다. 판단에 도움이 되는 의견도 낸다(2026-10-01 사용자 결정). 의견은 화면에서 읽은 사실과 구분해 "에이전트 의견"으로 적고, 근거가 된 수치와 확인하지 않은 것을 함께 적는다. 기법과 순서를 정하는 것은 사용자다.
- 포스팅에 넣을 시각 자료의 최종 선택. 클라이언트 영상과 전체 화면(`Scripts/capture-video.ps1`, 찍기 전에 허가를 받는다), Insights 스크린샷(`Scripts/capture-insights.ps1`)은 에이전트가 찍어 후보로 주고, 포스팅에 넣을 것은 사용자가 고르거나 직접 찍는다. 특정 장면을 위해 관찰자를 조작해야 하면 그 조작만 사용자에게 요청한다.
- 기준선 확정(단기 구현 계획 태스크 8.6, 2막 구현 계획 태스크 20.5). 결과를 표로 보고하고 사용자가 확정한다. 기준선이 송신 한도에 포화되면 프로젝트 설정에서 한도를 올리는 것은 이미 정해진 방침이라 묻지 않고 진행한다([ADR-0007](Docs/Decisions/0007-raise-send-limit-once.md), 태스크 8.4a). 그 밖의 설정이나 시나리오 규모는 임의로 바꾸지 않는다.
- ADR 승인.
- 에디터에서만 가능한 작업.
- GitHub 푸시.

포스팅의 나머지 섹션(요약, 적용, 결과, 배운 것과 한계)과 측정 기록(`measurements.md`)은 에이전트가 쓴다.

## 세션을 끝낼 때

1. [Docs/STATUS.md](Docs/STATUS.md)를 갱신한다: 끝낸 것, 다음 할 일, 막힌 것, 사용자에게 요청한 일.
2. 이 세션에 새 결정이 생겼으면 ADR 초안을 썼는지 확인한다.
3. 문서를 만들거나 옮겼으면 "문서 지도"를 고쳤는지 확인한다.
