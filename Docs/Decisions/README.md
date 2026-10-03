# 결정 기록(ADR)

이 프로젝트에서 대안을 비교해 내린 결정과 그 이유를 남긴다. 기법 선택(포스팅의 "선택" 섹션)은 여기에 두지 않는다.

| ADR | 결정 | 상태 |
| --- | --- | --- |
| [0001](0001-legacy-replication.md) | 레거시 리플리케이션으로 시작하고 Iris와 Replication Graph는 미룬다 | 대체됨 → 0013 |
| [0002](0002-editor-build-without-packaging.md) | 패키징과 null RHI 없이 에디터 빌드로 측정한다 | 승인됨 |
| [0003](0003-lawless-baseline.md) | 기준선에서 엔진 기본 관련성 판정을 끈다("무법지대") | 승인됨 |
| [0004](0004-insights-and-csv-metrics.md) | 수치를 Insights와 CSV 두 갈래로 나눈다 | 승인됨 |
| [0005](0005-saturation-from-isnetready.md) | 송신 한도 포화를 프레임 끝의 `IsNetReady()`로 판정한다 | 대체됨 → 0006 |
| [0006](0006-saturation-from-engine-analytics.md) | 송신 한도 포화를 엔진의 연결별 포화 기록으로 판정한다 | 승인됨 |
| [0007](0007-raise-send-limit-once.md) | 기준선이 포화되면 송신 한도를 올려 한 번만 고정한다 | 승인됨 |
| [0008](0008-reproducible-runs.md) | 실행을 재현 가능하게 만들고, 3회 중앙값과 실패 규칙으로 판정한다 | 승인됨 |
| [0009](0009-server-cores-without-dpc-load.md) | 서버를 DPC 부하가 몰리는 코어를 뺀 논리 프로세서 2\~7에 고정한다(0008의 코어 고정 항목을 바꾼다) | 승인됨 |
| [0010](0010-frame-time-without-tick-wait.md) | 서버 프레임 시간에서 틱 속도 제한 대기를 뺀다 | 승인됨 |
| [0011](0011-no-iris-preview-section.md) | 포스팅에서 "Iris에서는" 섹션을 뺀다(0001의 "결과" 한 줄을 바꾼다) | 승인됨 |
| [0012](0012-server-timer-resolution.md) | 서버 프로세스가 타이머 해상도 요청을 무시당하지 않게 한다(틱이 약 21Hz로 도는 실행을 없앤다) | 승인됨 |
| [0013](0013-lift-legacy-only-rule.md) | 레거시 리플리케이션만 쓴다는 제약을 푼다(0001을 대체한다) | 승인됨 |
| [0014](0014-act-2-testbed-expansion.md) | 테스트베드를 한 번 확장하고(2막) 새 기준선에서 세 기법을 다시 적용한다 | 제안됨 |
| [0015](0015-post-body-and-measurement-record.md) | 포스팅 본문과 측정 기록을 나누고 틀을 바꾼다(0011의 "결과" 가운데 틀 한 줄을 바꾼다) | 승인됨 |

## ADR 이전의 결정

ADR을 쓰기 전에 [단기 설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md) 3절에서 확정한 결정 가운데 ADR이 없는 것이다(2026-10-03에 옮김). 승인된 ADR과 같이 다시 열지 않는다. Iris, Replication Graph, FastArray의 순서는 [backlog.md](../backlog.md)에 있다.

| 결정 | 내용 | 이유 |
| --- | --- | --- |
| 부하 원천 | 실제 클라이언트 + 다수의 리플리케이트 액터 혼합 | 연결당 비용과 액터 수 비용이 둘 다 드러난다 |
| 월드 구성 | 플레이어 캐릭터, 자원 노드, AI NPC 세 가지 | 세 요소의 비용 패턴이 겹치지 않는다. 플레이어는 수가 적고 연결마다 비용이 생기고, 자원 노드는 수가 많고 거의 안 변하고, AI NPC는 수가 많고 계속 움직인다 |
| 작업 환경 | Windows PC에서 Claude Code 실행 | 코드 작성, 빌드, 실행, 로그 확인을 한곳에서 한다 |
| 포스팅 언어 | 한국어 | 국내 공고 대상([job-posting-ue5-dedicated-server.md](../Reference/job-posting-ue5-dedicated-server.md)) |
| 에셋 | TPP 템플릿과 프리미티브 + 단색 머티리얼 | 에셋 작업 시간을 쓰지 않는다 |

## 쓰는 법

- 파일 이름은 `NNNN-영문-요약.md`다. 번호는 이어서 붙이고 다시 쓰지 않는다.
- 상태는 셋 중 하나다. **제안됨**(에이전트가 초안을 썼고 사용자 승인을 기다림), **승인됨**, **대체됨 → NNNN**.
- 승인된 ADR의 본문은 고치지 않는다. 결정이 바뀌면 새 ADR을 쓰고, 옛 ADR의 상태 줄만 "대체됨"으로 바꾼다. 오타와 깨진 링크는 고쳐도 된다.
- 틀: 머리(상태, 날짜, 출처) → 맥락 → 결정 → 고려한 대안 → 결과. 수치와 설정값에는 근거(측정 조건, 엔진 소스 파일과 줄, 계산식)를 적는다.
