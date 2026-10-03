# 2막 구현 계획

작성: 2026-10-03. 설계는 [2막 설계](2026-10-03-act-2-design.md), 결정은 [ADR-0014](../Decisions/0014-act-2-testbed-expansion.md)에 있다. 태스크 번호는 1막의 구현 계획(태스크 1\~14)에 이어 15부터 붙인다.

단계는 다섯이다: 기법 전환 인자, 테스트베드 확장, 새 기준선, 세 기법 다시 적용, 2막 포스팅 주기. 앞 단계가 끝나야 다음 단계를 시작한다. `[사람]`은 사용자가 하는 일이다.

모든 태스크에 공통이다.

- 빌드와 실행은 [STATUS.md](../STATUS.md) "명령"의 명령으로 한다. 확정 규모 측정은 main 체크아웃에서 하고, 측정 중에는 다른 작업을 하지 않는다.
- 코드 태스크는 작은 규모 확인(`-Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`)이 종료 코드 0으로 끝난 뒤에 커밋한다.
- 실행 인자를 더하면 `LabScenarioConfig.h`, `Scripts/run-scenario.ps1`(UTF-8 BOM), 루트 README의 코드 표를 함께 고친다.
- 엔진 동작에 기대는 값은 5.8.3 소스에서 확인하고 [engine-notes.md](../Reference/engine-notes.md)에 적는다.

## 단계 1. 기법 전환 인자

[ADR-0014](../Decisions/0014-act-2-testbed-expansion.md) 승인과 상관없이 시작할 수 있다. 기본 동작을 바꾸지 않는다.

### 태스크 15. 세 기법을 실행 인자로 켜고 끄기

- 15.1 서버 인자 세 개를 `FLabServerConfig`에 더한다. 인자를 주지 않으면 지금 구성(세 기법 모두 적용)이다.
  - `-LabAlwaysRelevant`: 자원 노드와 NPC에 `bAlwaysRelevant = true`를 준다([포스팅 2](../../Posts/02-relevancy/README.md) "적용"에서 지운 두 줄).
  - `-LabNoNodeDormancy`: 자원 노드를 Dormant 상태로 두지 않는다(`LabResourceNode.cpp:17`의 `NetDormancy = DORM_DormantAll`을 적용하지 않는다).
  - `-LabNpcUpdateFrequency=`: NPC의 `NetUpdateFrequency` 값(`LabNpc.cpp:17`, 기본 10). 엔진 기본값은 100이다(STATUS.md "확정할 값").
  - 값을 적용하는 자리(생성자, 스폰할 때)는 구현하면서 정한다. 세 값은 서버에서만 뜻이 있고, 생성자는 클라이언트에서도 돈다.
- 15.2 `run-scenario.ps1`에 매개변수 `-AlwaysRelevant`, `-NoNodeDormancy`, `-NpcUpdateFrequency <값>`을 더하고, 서버 로그에 적용된 구성을 한 줄로 남긴다.
- 15.2a 수치 CSV에 `config` 열을 더한다. 지금 열에는 구성이 없어서(`LabMetricsSubsystem.cpp:280`), 한 바이너리에서 구성을 바꿔 재면 행이 어느 구성인지 라벨에만 기댄다. 값은 기본값과 다른 인자를 `;`로 이은 문자열이고(예: `AlwaysRelevant;NoNodeDormancy;NpcUpdateFrequency=100`), 인자 없는 실행은 `default`다. 2막에서 더하는 요소와 기법의 인자도 이 열에 적는다. 열이 달라지므로 지금의 `Saved/LabMetrics/summary.csv`는 `summary-act1.csv`로 이름을 바꿔 남기고 새 파일로 시작한다. [measurement.md](../Guides/measurement.md)에 열의 뜻을 적는다.
- 15.3 작은 규모에서 네 구성을 한 번씩 실행해 종료 코드 0과 `config` 열을 확인한다.

| 구성 | 서버 인자 | `run-scenario.ps1` 매개변수 | 1막의 `open_actor_channels_per_conn` |
| --- | --- | --- | --- |
| 기준선 | `-LabAlwaysRelevant -LabNoNodeDormancy -LabNpcUpdateFrequency=100` | `-AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100` | 5,314(`baseline3`) |
| Relevancy | `-LabNoNodeDormancy -LabNpcUpdateFrequency=100` | `-NoNodeDormancy -NpcUpdateFrequency 100` | 118(`relevancy2`) |
| Dormancy | `-LabNpcUpdateFrequency=100` | `-NpcUpdateFrequency 100` | 20(`dormancy6`) |
| Net Update Frequency(기본) | 없음 | 없음 | 20(`update-frequency3`) |

### 태스크 16. 인자가 1막의 구성을 재현하는지 확인

- 16.0 확정 규모 측정을 시작하기 전에 사용자에게 알리고 답을 받는다. 측정하는 동안 이 PC에서 다른 작업을 할 수 없다(STATUS.md "명령"). 16.1과 16.2를 한 번에 알린다.
- 16.1 확정 규모에서 인자 없는 실행 3회(라벨 `toggle1`)를 재서 `timerfix`(STATUS.md "단계")와 비교한다. `work_avg_ms`, `netflush_avg_ms`의 중앙값 차이가 변동 폭 안이어야 한다. 벗어나면 인자를 넣기 전 커밋을 다시 빌드해 연달아 재서 가린다.
- 16.2 확정 규모에서 기준선, Relevancy, Dormancy 구성을 한 번씩 실행해(라벨 `toggle-baseline`, `toggle-relevancy`, `toggle-dormancy`) `open_actor_channels_per_conn`이 위 표의 1막 값과 같은지 확인한다. 이 실행의 시간 수치는 비교에 쓰지 않는다.
- 16.3 STATUS.md "명령"에 구성별 명령을 적는다.

## 단계 2. 테스트베드 확장

ADR-0014가 승인된 뒤에 시작한다. 요소마다 태스크 하나이고, 각 태스크는 같은 순서로 한다: 설계 3절의 "정할 것"을 선택지와 추천안으로 사용자에게 묻는다 → 엔진 소스 확인 → 구현(인자 기본값 끔) → 화면 글자에 수 표시 → 작은 규모 확인 → 자동 스크린샷을 열어 확인.

### 태스크 17. 플레이어가 모이는 배치

- 플레이어 자리 간격을 정하는 서버 인자. 클라이언트의 자동 이동 경로가 겹치는지 확인한다.
- 내려다보기 화면에 다른 플레이어의 파란 점이 보이는지 스크린샷으로 확인한다.

### 태스크 18. 상태 값과 인벤토리

- 18.1 NPC와 플레이어의 상태 값. 프로퍼티는 하위 클래스나 컴포넌트에 두고, 서버가 시드로 정한 간격에 일부 액터의 값만 바꾼다.
- 18.2 인벤토리. 일반 `TArray` 프로퍼티, 시드로 채우기, 채집 성공 때 한 칸 변경. 둘 액터는 "정할 것"에서 정한다(`PlayerState.cpp:26, 28`).

### 태스크 19. 건축물, 그리고 1막 시나리오의 재현 확인

- 19.1 건축물 액터와 플레이어 자리 주변의 시드 배치. 내려다보기 화면의 점 색을 정한다.
- 19.2 확정 규모에서 새 인자를 모두 끈 실행 3회를 태스크 16.1의 묶음과 연달아 재서 구별되지 않는지 확인한다. 구별되면 원인을 찾아 고친 뒤 다시 잰다.

## 단계 3. 새 기준선

### 태스크 20. 보정

- 20.1 출발값을 정한다. 1막의 측정에서 계산해 근거를 적는다(예: 채널이 열린 자원 노드 하나를 확인하는 비용은 `relevancy2-r1`의 `LabResourceNode` 프레임당 1.98ms ÷ (95.6개 × 연결 8)로 구한다).
- 20.2 보정 실행(라벨 `calib2-a`부터). 실행마다 서버 로그와 자동 스크린샷을 확인하고, 표를 [Worklog](../Worklog/)의 새 파일 `05-expanded-testbed.md`에 쌓는다.
- 20.3 기준선의 네 조건을 확인한다. 송신 한도에 포화되면 설계 5절의 계산으로 한도를 올린다. 준비 구간 안에 초기 전송이 끝나는지 본다.
- 20.4 세 기법을 적용한 구성에서 Insights를 열어 클래스 타이머와 비트 수를 읽고, 설계 6절의 후보마다 겨냥할 비용이 `work_avg_ms`의 변동 폭보다 큰지 표로 만든다.
- 20.5 `[사람]` 규모 확정. 에이전트가 20.3과 20.4의 표를 보고하고 사용자가 확정한다. 확정값은 STATUS.md "확정할 값"에 2막의 표로 적는다.

### 태스크 21. 포스팅 5: 테스트베드 확장과 새 기준선

- 21.1 확정 규모에서 기준선을 3회 잰다(라벨은 20.5 뒤에 정한다).
- 21.2 Insights 캡처와 자동 스크린샷, 관찰 자료(`Posts/05-expanded-testbed/candidates.md`)를 준비한다.
- 21.3 `[사람]` "관찰" 작성. 에이전트는 나머지 섹션의 초안을 쓴다. 이 포스팅에는 "선택"과 "적용"이 없으므로 [posting.md](../Guides/posting.md)의 틀에서 빼는 섹션을 먼저 사용자와 정한다.
- 21.4 태그 `post-05-expanded-testbed`.

## 단계 4. 세 기법 다시 적용

### 태스크 22. 포스팅 6: 세 기법 다시 적용

- 22.1 기준선, Relevancy, Dormancy, Net Update Frequency 네 구성을 3회씩 연달아 잰다. 기준선은 21.1과 화면 조건이 같으면 그 묶음을 쓴다.
- 22.2 구성마다 중앙값 실행의 Insights 값을 읽는다. 새 요소(건축물, 상태 값, 인벤토리, 다른 플레이어의 캐릭터)의 클래스 타이머를 함께 읽는다.
- 22.3 포스팅 초안. 네 구성의 표, 1막의 같은 표와 나란히 놓은 비교, 달라진 점(예: 건축물이 Net Cull Distance 안에 몰려 있을 때 Relevancy와 Dormancy의 몫). 원리 설명은 1막의 글로 링크한다.
- 22.4 루트 README에 2막의 포스팅 표와 누적 수치를 더한다. 1막의 표는 그대로 둔다.
- 22.5 태그 `post-06-three-techniques-again`.

### 태스크 23. 2막 포스팅 순서 확정

- 23.1 포스팅 6의 최종 구성에서 남은 비용을 큰 순서로 정리하고, 설계 6절의 후보마다 겨냥할 비용의 크기를 적는다.
- 23.2 `[사람]` 순서 확정. 에이전트는 추천 순서와 이유를 낸다. 확정된 순서는 [backlog.md](../backlog.md) "우선순위 순"에 적는다.

## 단계 5. 2막 포스팅 주기

포스팅 7부터는 포스팅마다 같은 주기를 돈다. 태스크 번호는 24부터 포스팅마다 하나씩 붙인다.

1. 직전 포스팅의 최종 구성을 기준 묶음으로 삼는다. 화면 조건이 달라졌으면 연달아 다시 잰다.
2. 관찰 자료를 준비한다: Insights에서 읽은 값, CSV와 대조한 표, 후보 기법과 엔진 소스 위치(`candidates.md`).
3. `[사람]` "관찰"과 "선택".
4. 기법을 실행 인자와 함께 구현한다. 기본값은 끔이고, 인자 없는 실행은 언제나 1막의 최종 구성이다(설계 4절).
5. 3회 측정, Insights 값, 시각 자료.
6. 초안(요약, 적용, 결과, 한계와 다음) → `[사람]` 확인 → 태그.
7. STATUS.md와 루트 README의 2막 표를 갱신하고, 경위를 Worklog로 옮긴다.

기법이 없는 진단 포스팅(자체 시간 나누기, 지연과 패킷 손실)은 3, 4를 건너뛰고 "선택"과 "적용" 대신 무엇을 어떻게 쟀는지를 적는다.

## 사람에게 넘기는 일 모음

| 시점 | 일 |
| --- | --- |
| 단계 2 시작 전 | ADR-0014 승인 |
| 태스크 17\~19 | 요소마다 "정할 것"의 선택 |
| 태스크 20.5 | 규모 확정 |
| 태스크 21.3 | 포스팅 5의 "관찰" |
| 태스크 23.2 | 포스팅 7부터의 순서 확정 |
| 포스팅마다 | "관찰"과 "선택", 시각 자료의 최종 선택, 화면 녹화 허가, 푸시 |
