# 작업 기록: 포스팅 11 클라이언트에서 NPC 위치를 보간하기 (태스크 28)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-07 태스크 28: 클라이언트에서 NPC 위치를 보간하기 (`act2-interp-base2`, `act2-interp2`, `act2-interp-base1`, `act2-interp1`, `tsmall-motion1`\~`tsmall-motion5`, `visual20`, `visual21`)

- **포스팅 11의 작은 규모 확인(2026-10-07).** 클라이언트 2개, 자원 노드 100개, NPC 10개와 플레이어 주변 50개(`-PlayerSpacing 3 -NpcsNearPlayers 50 -MotionLog`), 준비 20초, 측정 30초다. 보간 없음(`tsmall-motion1-r1`) → 150ms 보간(`tsmall-motion2-r1`)에서 표시 위치 오차 평균 21.19 → 49.03cm, 표시 속도 오차 평균 448.7 → 12.3cm/s, 표시 지연 70 → 165ms, 수신 간격 평균 133.4ms(그대로), `out_bytes_per_sec_per_conn` 7,677 → 8,421이다. 보간 없음의 값은 ADR-0020의 예상(약 25cm, 450cm/s)과 맞는다. 작은 규모라 포스팅의 비교에 쓰지 않는다.
- **첫 확정 규모 묶음은 쓰지 않는다(2026-10-07).** `act2-interp-base1`, `act2-interp1`(01:32\~01:50, 본체 화면)은 측정 시작 직후(01:33:12)부터 다른 세션이 남긴 `grep.exe`가 코어 하나를 계속 썼다. 기준 묶음의 `work_avg_ms`가 13.43\~14.16(전날 같은 구성 `act2-fastarr-base1` 8.852)이고, Consider List와 호출 횟수는 같은데 모든 타이머가 약 1.5배 느렸다(`GameNetDriver` 4.112 → 6.468ms, `act2-fastarr-base1-r3`과 `act2-interp-base1-r3`의 Insights 내보내기). 이 묶음에서 보간 시계의 결함도 찾았다. 서버가 1분에 1,764\~1,779프레임만 돌자 프레임 길이를 33.3ms로 가정한 시계가 밀려 표시 지연이 139\~144ms(150ms보다 짧음)였다. 클라이언트가 받은 시각에 직선을 맞춰 프레임 길이를 추정하게 고쳤고, 서버를 28Hz로 돌린 작은 규모 확인에서 표시 지연 151ms, 표시 속도 오차 평균 12.9cm/s였다(`tsmall-motion5-r1`, `run-scenario.ps1 -ServerTickRate 28`, 진단용). 같은 두 구성을 `act2-interp-base2`, `act2-interp2`로 다시 쟀다(아래 "측정 결과").
