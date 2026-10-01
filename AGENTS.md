# UE Dedicated Server 포트폴리오

언리얼 엔진 5.8.3 Dedicated Server 최적화 과정을 기법 하나당 포스팅 하나로 보여주는 포트폴리오다. 포스팅 0~4를 완성하는 것이 단기 목표다.

## 세션을 시작하면

1. [Docs/STATUS.md](Docs/STATUS.md)를 읽는다. 현재 진행 중인 작업과 다음 할 일이 있다.
2. [구현 계획](Docs/Planning/2026-10-01-short-term-implementation-plan.md)의 체크리스트에서 체크되지 않은 첫 단계를 찾는다. 끝낸 단계는 그 파일에서 체크한다.
3. 그 단계에 필요한 문서만 아래 "문서 지도"에서 찾아 연다. 결정을 바꾸고 싶어지면 먼저 [ADR 목록](Docs/Decisions/README.md)을 본다.

## 문서 지도

| 알고 싶은 것 | 볼 곳 |
| --- | --- |
| 진행 상황, 다음 할 일, 빌드와 실행 명령, 확정값, 측정 결과 | [Docs/STATUS.md](Docs/STATUS.md) |
| 다음 작업과 그 절차 | [구현 계획](Docs/Planning/2026-10-01-short-term-implementation-plan.md)의 체크리스트 |
| 확정된 결정과 그 이유, 버린 대안 | [Docs/Decisions/](Docs/Decisions/README.md)(ADR), [설계 문서](Docs/Planning/2026-10-01-short-term-portfolio-design.md) 3절 |
| 측정 규약: 시나리오, 절차, 지표, 한계 | 설계 문서 8절 |
| 수치를 부르는 이름 | 구현 계획 "수치의 이름과 출처" |
| 시각 자료의 종류와 파일 이름 | 구현 계획 "시각 자료 규칙" |
| 포스팅 틀 | 설계 문서 7절. 포스팅 0은 구현 계획 태스크 9.2 |
| 기법별 코드 초안 | 구현 계획 태스크 11 "기법별 코드" |
| 엔진 기본값, 동작 순서, 소스 위치 | [engine-notes.md](Docs/Planning/engine-notes.md) |
| 일정이 넘칠 때 줄이는 순서 | 구현 계획 "일정이 넘칠 때" |
| 단기 범위 밖의 주제, 작업 중 떠오른 기법 | [backlog.md](Docs/Planning/backlog.md) |
| 측정 PC 사양 | [pc-specs.md](Docs/Planning/pc-specs.md) |
| 대상 채용공고 | [job-posting-ue5-dedicated-server.md](Docs/Planning/job-posting-ue5-dedicated-server.md) |
| 공개 소개, 코드 파일별 역할 | [README.md](README.md) |
| 포스팅 본문과 이미지 | `Posts/NN-이름/README.md`, `Posts/NN-이름/images/` |
| 실행 산출물 | 수치 CSV `Saved/LabMetrics/summary.csv`, 트레이스 `Saved/Traces/<라벨>-rN.utrace`, 스크린샷 `Saved/Screenshots/Lab/`, 로그 `Saved/Logs/` |

새 문서를 만들거나 옮기면 이 표를 함께 고친다.

## 규칙

- **개발 속도가 최우선이다.** 단위 테스트, CI, PR, 워크트리, 외부 이슈 트래커를 쓰지 않는다. main에 직접 커밋한다.
- **커밋 메시지는 [Conventional Commits](https://www.conventionalcommits.org/) 형식으로 쓴다.** `type(scope): summary` 한 줄이고, summary는 소문자로 시작하는 영어 명령형이다. type은 `feat`, `fix`, `docs`, `build`, `refactor`, `chore` 중에서 고르고 scope는 필요할 때만 붙인다.
- **확정된 결정을 다시 열지 않는다.** 설계 문서 3절과 승인된 ADR의 결정을 바꾸자고 제안하지 않는다. 사용자는 계획 단계로 되돌아가는 일을 반복해 왔다. 예외는 결정의 근거가 사실과 맞지 않음을 확인한 경우 하나다. 근거는 엔진 소스(파일과 줄)나 실행 결과(라벨)여야 한다. 이때는 대체 ADR을 "제안됨"으로 쓰고, 그 결정에 기대는 측정을 멈추고, 사용자에게 알린다. 더 나은 방법이 있다는 판단은 예외가 아니다. 그런 생각은 [backlog.md](Docs/Planning/backlog.md)에 적는다.
- **포스팅 하나에 기법 하나.** 여러 기법을 함께 넣으면 어느 것이 효과를 냈는지 알 수 없다. 단기에 구현하는 기법은 관련성, 자원 노드 휴면, NPC 업데이트 빈도 세 가지뿐이다. 사용자는 이 셋의 순서를 고른다. 사용자가 다른 기법을 원해도 단기에는 구현하지 않고 [backlog.md](Docs/Planning/backlog.md)의 "작업 중 떠오른 것"에 적는다.
- **패키징하지 않고 null RHI를 쓰지 않는다.** 에디터 빌드 실행 파일을 쿠킹 없이 실행한다([ADR-0002](Docs/Decisions/0002-editor-build-without-packaging.md)).
- **레거시 리플리케이션만 쓴다.** 게임 코드는 표준 `UPROPERTY` 리플리케이션과 RPC만 사용한다([ADR-0001](Docs/Decisions/0001-legacy-replication.md)).
- **에디터 작업을 만들지 않는다.** 에이전트는 에디터를 다룰 수 없어서, 에디터 작업이 끼면 사람을 기다려야 하고 결과를 스스로 검증하지 못한다. 액터는 C++로 작성하고, 자원 노드와 AI NPC는 실행 시 코드로 생성한다. 블루프린트와 맵 편집이 필요한 설계를 피한다.
- **수치를 정해진 이름으로만 부른다.** CSV 값과 Insights 값은 정의가 달라서, 이름을 섞으면 다른 수치를 같은 것처럼 비교하게 된다([ADR-0004](Docs/Decisions/0004-insights-and-csv-metrics.md)). 구현 계획의 "수치의 이름과 출처" 표를 따른다. CSV 값을 Insights 지표 이름으로 부르지 않고, 열린 액터 채널 수를 액터 수라고 부르지 않는다.
- **모든 수치와 설정값에 근거를 적는다.** 측정값(조건 명시), 엔진 소스 확인값(파일과 심볼 명시), 계산값(식 명시) 중 하나다. 기억에 의존한 엔진 기본값은 5.8.3 소스에서 확인한 뒤에 쓴다.
- **시각 자료를 적극적으로 모은다.** 측정할 때마다 자동 스크린샷을 직접 열어 보고 포스팅에 넣는다. 전후 비교는 이미지를 나란히 놓고, 수치는 Mermaid 차트로도 보여준다.
- **구조와 이름.** 언리얼 프로젝트는 레포 루트의 `DSOptLab.uproject`다(모듈 `DSOptLab`). 이 프로젝트에서 만드는 클래스, 실행 인자, 저장 폴더의 접두사는 `Lab`이다(`ALabResourceNode`, `-LabNodes=`, `Saved/LabMetrics/`). 폴더 이름은 대문자로 시작한다(`Docs/`, `Posts/`, `Scripts/`).
- **`Scripts/`의 PowerShell 스크립트는 UTF-8 BOM으로 저장한다.** Windows PowerShell 5.1이 BOM 없는 한글을 잘못 읽는다(engine-notes.md 라절).
- **포스팅은 한국어로 쓴다.** 틀은 설계 문서 7절을 따른다.
- **문서에 일정 표현을 쓰지 않는다.** README, 포스팅, `Docs/` 어디에도 "N일차"나 "N일 안에" 같은 표현을 쓰지 않는다. 단계는 구현 계획의 큰 제목(테스트베드 구축, 규모 확정과 포스팅 0, 기준선과 첫 번째 기법, 나머지 두 기법, 마무리)과 태스크 번호로 부른다. 레포가 공개다.

## 결정 기록(ADR)

- 대안을 비교해 내린 결정이 새로 생기면(측정 정의를 고침, 설정 방침을 정함, 구조를 바꿈) [Docs/Decisions/](Docs/Decisions/README.md)에 상태 "제안됨"으로 초안을 쓰고, [Docs/STATUS.md](Docs/STATUS.md)의 "사용자에게 요청한 일"에 승인을 요청한다. 승인은 사용자가 한다.
- 승인된 ADR의 본문은 고치지 않는다. 결정이 바뀌면 새 ADR로 대체한다. 쓰는 법은 [Docs/Decisions/README.md](Docs/Decisions/README.md)에 있다.
- 기법 선택과 포스팅 순서는 ADR로 쓰지 않는다. 사용자가 포스팅의 "선택" 섹션에 직접 쓴다.

## 완료 확인

작업 하나가 끝났다고 말하기 전에 다음을 확인한다.

1. 빌드가 성공한다.
2. 시나리오 스크립트가 종료 코드 0으로 끝나고 서버가 수치 CSV에 새 행을 남긴다.
3. CSV 수치를 이전 구성과 비교해 [Docs/STATUS.md](Docs/STATUS.md)에 적는다.

문서만 바꾼 작업은 1~3 대신 바꾼 문서의 상대 링크가 실제 파일을 가리키는지 확인한다. 실패한 실행의 수치는 쓰지 않는다. 같은 구성을 다시 측정할 때는 새 라벨을 쓴다.

빌드와 실행은 [Docs/STATUS.md](Docs/STATUS.md)의 "명령"에 적힌 명령으로만 한다.

## 사람에게 넘기는 일

다음은 직접 하지 않고, 필요한 시점에 무엇을 해야 하는지 구체적으로 적어 사용자에게 요청한다.

- **Insights 분석과 포스팅의 "관찰", "선택" 섹션 작성.** 사용자가 직접 분석하고 직접 쓴다. 에이전트는 CSV 수치 표와 후보 기법 목록(각 기법의 구현 비용과 관련 엔진 소스 위치)만 준비한다. 어떤 기법을 고를지 결론을 먼저 제시하지 않는다.
- Insights 스크린샷, 클라이언트 화면 영상, 모든 클라이언트 창이 보이는 전체 화면. 필요한 시점에 파일 이름까지 정해서 요청한다.
- 기준선 확정(구현 계획 태스크 8.6). 결과를 표로 보고하고 사용자가 확정한다. 기준선이 송신 한도에 포화되면 프로젝트 설정에서 한도를 올리는 것은 이미 정해진 방침이라 묻지 않고 진행한다([ADR-0007](Docs/Decisions/0007-raise-send-limit-once.md), 태스크 8.4a). 그 밖의 설정이나 시나리오 규모는 임의로 바꾸지 않는다.
- ADR 승인.
- 에디터에서만 가능한 작업.
- GitHub 푸시.

포스팅의 나머지 섹션(요약, 적용, Iris에서는, 결과, 한계와 다음)은 에이전트가 초안을 쓴다.

## 세션을 끝낼 때

1. [Docs/STATUS.md](Docs/STATUS.md)를 갱신한다: 끝낸 것, 다음 할 일, 막힌 것, 사용자에게 요청한 일.
2. 이 세션에 새 결정이 생겼으면 ADR 초안을 썼는지 확인한다.
3. 문서를 만들거나 옮겼으면 "문서 지도"를 고쳤는지 확인한다.
