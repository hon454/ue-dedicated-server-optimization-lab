# 작업 기록: 포스팅 10 인벤토리를 FastArray로 보내기 (태스크 27)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-06 태스크 27: 인벤토리를 FastArray로 보내기 (`act2-fastarr-base1`, `act2-fastarr1`, `visual18`, `visual19`)

- **태스크 27의 경위.** [backlog.md](../backlog.md) "우선순위 순" 6번. 초안은 [본문](../../Posts/10-inventory-fastarray/README.md)과 [측정 기록](../../Posts/10-inventory-fastarray/measurements.md)이다(2026-10-06). FastArray의 추가, 삭제, 변경은 사용자가 고른 대로 도식 하나(`Scripts/make-fastarray-cases.ps1`)와, 경우마다 클라이언트가 부르는 함수와 칸 하나에 가는 비트의 표로 보였다(2026-10-06 사용자 요청). 칸 안 델타 직렬화가 기본으로 켜져 있다는 것을 확인해 engine-notes.md 10절의 틀린 행을 바로잡았다. "문제", "원리"는 에이전트 초안이라 사용자가 승인한다. 사용자가 승인했다(2026-10-06). 루트 README 행의 "(작성 중)"을 빼고 태그를 붙였다. 루트 README의 "결과 한눈에 보기"와 [Posts/measurements.md](../../Posts/measurements.md)는 포스팅 6의 세 단계만 다루고 포스팅 8, 9도 고치지 않아 그대로 두었다. 구현, 측정, Insights 값, 패널 영상이 끝났다(2026-10-06). 연결당 송신량은 12,387 → 11,807(-4.7%)이고, `Connection 0`의 인벤토리는 한 번 바뀔 때 최대 18,190 → 345비트, 60초에 273,676 → 7,086비트다. `work_avg_ms`는 구별되지 않았고(8.852 → 8.586, 변동 폭 0.289), 인벤토리의 CPU는 줄지 않았다(프레임당 0.104 → 0.117ms, 실행 하나씩). 값과 계산은 관찰 자료 10절부터에 있다. 본문의 "문제", "원리"는 에이전트 초안을 사용자가 승인한다(AGENTS.md "사람에게 넘기는 일").

## 2026-10-06 README와 시각화 페이지 갱신: 2막 여섯 단계 다시 재기 (`act2-all-*1`, `act2-all-check*`, `act2-bisect-*`, `act2-split-nofa`, `act2-split-netcore`, `act2-split-main`)

포스팅에 속하지 않는 작업이라 진행 중이던 포스팅 10의 파일에 적는다. 사용자가 루트 README와 시리즈 웹 페이지를 지금 시점으로 갱신하자고 했다(2026-10-06). README는 일곱 구성을 연달아 다시 재기로, 웹 페이지는 1막과 2막을 고르는 전환을 더하기로 했다(사용자 선택). 연결당 송신 대역폭은 CSV 값으로 바꾸기로 했다(사용자 승인).

- **일곱 구성 측정.** 2026-10-06 19:56\~20:52, 본체 화면, 소스 `243be3d`. 구성과 값은 [누적 수치의 측정 기록](../../Posts/measurements.md) "2막: 여섯 단계"에 있다. 모든 실행이 종료 코드 0이고 서버 로그의 구성 줄이 맞았다.
- **느린 값.** 기준선과 ①\~③이 포스팅 6의 묶음보다 15\~54% 컸다(① 54.860 대 35.674, 변동 폭 12.654 대 1.467). Epic Games Launcher, Riot Client, Notion, Chrome, Unreal Insights 창 둘이 떠 있었다. 사용자가 Epic Games Launcher를 뺀 나머지를 닫은 뒤 ①을 한 번 다시 쟀다(`act2-all-check1`, 50.669). 돌아오지 않았다.
- **소스 비교.** STATUS.md "명령"의 절차로 포스팅 6 태그의 소스를 빌드해 ①을 쟀다(`act2-all-check-old1`, 40.422). 작업 트리를 덮는 `git restore`는 권한 분류기가 한 번 거부했고, 사용자가 허용했다. 세 번씩 연달아 다시 쟀다. main `act2-all-check2` 49.510, 49.854, 50.061, 포스팅 6 태그 `act2-all-check-old2` 39.477, 39.188, 37.320이다.
- **커밋 나누기.** 포스팅 6 뒤 소스를 바꾼 커밋마다 ①을 한 번씩 쟀다. `act2-bisect-3a82d7b` 38.926, `act2-bisect-a397f55` 38.564, `act2-bisect-730f247` 37.288, `act2-bisect-92dc93e` 38.111이다. `243be3d`만 느렸다. `243be3d`를 나눠 쟀다. FastArray 컴포넌트와 `NetCore` 의존을 뺀 main `act2-split-nofa` 41.361, `92dc93e`에 `NetCore`만 더한 `act2-split-netcore` 39.219, main `act2-split-main` 49.920이다.
- **원인.** `act2-all-check2-r2`와 `act2-bisect-92dc93e-r1`을 `export-insights.ps1`로 비교했다. 액터 한 번의 비용은 같고 Consider List가 5,073.9 → 5,875.4로 길었다. 틱 예산을 조금 넘는 구성에서 프레임 시간이 Consider List를 늘리는 되먹임이다([engine-notes.md](../Reference/engine-notes.md) 12절). `243be3d`가 ① 구성에서 하는 새 일은 없다. 코드는 고치지 않았다.
- **결정.** 사용자가 `act2-all-*1`을 README에 쓰기로 했다. ①이 흔들린다는 것을 README의 주의에, 근거를 누적 측정 기록 5절에 적었다. 진단 실행의 값은 README와 포스팅에 쓰지 않는다. 진단 실행 라벨 가운데 `act2-split-*`은 포스팅 7의 `act2-split-base1` 등과 접두사가 같지만 다른 실행이다.
