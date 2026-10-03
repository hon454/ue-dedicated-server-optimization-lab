# 작업 기록: 포스팅 5 테스트베드 확장과 새 기준선 (태스크 15\~21)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다. 지금의 상태는 [STATUS.md](../STATUS.md)에 있다.

## 2026-10-03 태스크 15: 세 기법을 실행 인자로 켜고 끄기 (`tsmall-baseline`, `tsmall-relevancy`, `tsmall-dormancy`, `tsmall-default`)

**2막 태스크 15 완료(2026-10-03).** 1막의 세 기법을 서버 인자로 켜고 끈다(`-LabAlwaysRelevant`, `-LabNoNodeDormancy`, `-LabNpcUpdateFrequency=`. 인자가 없으면 세 기법이 모두 적용된 지금 구성). 수치 CSV에 `config` 열을 더했고, 그 전의 행은 `Saved/LabMetrics/summary-act1.csv`로 옮겼다. 작은 규모에서 네 구성이 종료 코드 0으로 끝났다(`tsmall-baseline`, `tsmall-relevancy`, `tsmall-dormancy`, `tsmall-default`의 `open_actor_channels_per_conn` 118, 10, 7, 7).

## 2026-10-03 태스크 16: 인자가 1막의 구성을 재현하는지 확인 (`toggle1`, `toggle-dormancy`, `toggle-relevancy`, `toggle-baseline`, `toggle-baseline2`)

**2막 태스크 16 완료(2026-10-03). 인자가 1막의 구성을 재현한다.** 인자 없는 확정 규모 `toggle1` 3회: `frames` 1,786\~1,796, `work_avg_ms` 13.757 / 13.565 / 13.581(중앙값 13.581, 변동 폭 0.192), `netflush_avg_ms` 중앙값 9.646(변동 폭 0.205), `out_bytes_per_sec_per_conn` 2,399\~2,402, `config` 열 `default`. `timerfix`와의 중앙값 차이는 `work_avg_ms` 0.097, `netflush_avg_ms` 0.087(9.646 - 9.559)로 `timerfix`의 변동 폭 1.036, 0.800 안이라 구별되지 않는다. 구성별 1회 실행의 `open_actor_channels_per_conn`은 `toggle-dormancy-r1` 20, `toggle-relevancy-r1` 118, `toggle-baseline2-r1` 5,314로 1막(`dormancy6`, `relevancy2`, `baseline3`)과 같다. 이 세 실행의 시간 수치는 비교에 쓰지 않는다. `toggle-baseline-r1`은 측정 시작 2초 뒤에 선호도가 재설정되어 실패했고 수치를 쓰지 않는다. 지금 빌드의 기준 묶음은 `toggle1`이다.

## 2026-10-03 태스크 17: 플레이어 자리 간격 인자 (`tsmall-gather`\~`tsmall-gather8b`, `tsmall-apart`\~`tsmall-apart3`, `tsmall-noarg`\~`tsmall-noarg3`)

**2막 태스크 17 완료(2026-10-03). 플레이어 자리 간격 인자.** 서버 인자 `-LabPlayerSpacing=<m>`(`run-scenario.ps1`의 `-PlayerSpacing`)을 주면 자리를 맵 가운데의 대각선 위에 그 간격으로 놓고, 자리마다 다른 출발 위치와 방향으로 한 변 70m 정사각형을 돈다. 밀집은 3, 분산은 300이고, 인자가 없으면 1막의 배치와 경로다. 값과 계산은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 3.1에 있다. 작은 규모 확인(모두 종료 코드 0): `tsmall-gather8b-r1`(클라이언트 8, 간격 3)은 두 클라이언트의 화면 글자가 t=15, 30, 45초에 `players=8`, `tsmall-apart3-r1`(클라이언트 2, 간격 300)은 `players=1`, `tsmall-noarg3-r1`(인자 없음)은 t=45초의 위치가 바꾸기 전(`tsmall-default2-r1`)과 같은 x=535m, y=290m. 확정 규모에서 인자 없는 실행이 `toggle1`과 구별되지 않는지는 태스크 19.2에서 잰다. `tsmall-gather`, `tsmall-gather2`, `tsmall-gather8`은 고치기 전의 경로이고 `tsmall-apart-r1`은 실패한 실행이다([troubleshooting.md](../Guides/troubleshooting.md)의 "`nodes=5001 npcs=300`" 줄).
