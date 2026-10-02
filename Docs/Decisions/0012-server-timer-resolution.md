# ADR-0012: 서버 프로세스가 타이머 해상도 요청을 무시당하지 않게 한다

- 상태: 승인됨
- 날짜: 2026-10-03 (사용자 승인)
- 출처: [engine-notes.md](../Reference/engine-notes.md) 아절, [ADR-0008](0008-reproducible-runs.md)

## 맥락

틱 예산 안의 구성에서 서버가 30Hz가 아니라 약 21Hz로 도는 실행이 섞여 나온다. 60초 `frames`가 약 1,790이 아니라 약 1,280이다(`relevancy2-r1` 1,304, `dormancy2-r1` 1,288, `refactor-after2-r1` 1,271, `refactor-after2-r3` 1,287). 이런 실행은 `frames`와 `out_bytes_per_sec_per_conn`이 약 0.71배가 되고, `work_avg_ms`도 높게 나온 적이 있어(`dormancy2-r1` 16.205) 3회 묶음의 변동 폭을 키운다.

원인은 Windows 11이 창이 최소화되거나 완전히 가려진 프로세스의 타이머 해상도 요청을 무시하는 것이다(engine-notes.md 아절). 엔진의 틱 속도 제한은 `::Sleep`에 기대므로(`UnrealEngine.cpp:3116`) 해상도가 15.625ms로 떨어지면 프레임 주기가 46.875ms가 된다. 서버 콘솔 창을 최소화하면 `frames`가 644, 645(30초)이고(`timerdiag-min-r1`, `timerdiag-min2-r1`), 같은 상태에서 이 동작을 끄면 901, 902다(`timerdiag-optout-r1`, `timerdiag-optout2-r1`).

서버 창이 보이는지는 창의 겹침 순서와 그때 사용자가 쓰던 창에 달려 있어, 지금은 실행마다 우연히 정해진다.

## 결정

`Scripts/run-scenario.ps1`이 서버 프로세스를 띄운 직후 `SetProcessInformation(ProcessPowerThrottling)`으로 `PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION`을 끈다. 게임 코드와 엔진 설정은 바꾸지 않는다. `run-manual.ps1`의 서버에도 같이 적용한다.

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| 그대로 두고 3회 중앙값에 맡기기 | 느린 틱이 3회 중 두 번 나온 묶음이 있다(`refactor-after2`). 중앙값도 느린 쪽이 될 수 있다 |
| 스크립트가 서버 콘솔 창을 맨 위에 두기 | 화면 한 구석을 계속 가리고, 가려짐만으로 재현하는 실행을 하지 않아 충분한지 확인하지 못했다 |
| 서버에서 `-log`를 빼 창을 없애기 | 창이 없는 프로세스의 동작을 확인하지 않았고, 실행 중 서버 로그를 눈으로 볼 수 없게 된다 |
| 게임 모듈 시작 코드에서 같은 API를 부르기 | 측정 환경의 문제를 게임 코드로 푼다. 스크립트에서 풀면 태그의 빌드에도 그대로 적용된다 |
| 실행 뒤 `frames`가 낮으면 실패로 처리하기 | 원인을 없애지 않고 재실행 횟수만 늘린다 |

## 결과

- 지금까지의 수치는 바꾸지 않는다. `update-frequency3`, `dormancy6`, `refactor-before`는 세 실행 모두 `frames`가 1,787 이상이라 영향이 없다. `relevancy2`, `dormancy2`의 느린 실행은 STATUS.md "측정 결과"에 변동 폭으로 적혀 있다.
- 승인되면 스크립트를 고친 뒤 확정 규모로 3회를 재서 세 실행의 `frames`가 모두 약 1,790인지 확인한다.
- 클라이언트에는 적용하지 않는다. 클라이언트 창은 격자로 놓여 완전히 가려지지 않고, 클라이언트의 틱 속도는 측정값이 아니다.
- 확인하지 않은 것: 창이 최소화가 아니라 가려지기만 했을 때 같은 일이 생기는지. 결정은 두 경우를 모두 막는다.
