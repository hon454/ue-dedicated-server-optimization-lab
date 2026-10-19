# 작업 기록: 포스팅 15 레거시 리플리케이션을 Iris로 바꾸기 (태스크 32)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-20 태스크 32: 레거시 리플리케이션을 Iris로 바꾸기 (`act2-iris-base1`, `act2-iris1`, `act2-iris-rg1`, `act2-iris-prio1`, `act2-iris-base2`, `act2-iris2`, `act2-iris-rg2`, `act2-iris-prio2`, `tsmall-iris*`)

- **태스크 32(포스팅 15, `post-15-iris`)가 끝났다(2026-10-20).** 다음 태스크를 시작할 때 이 줄과 "측정 결과"의 표를 `Docs/Worklog/15-iris.md`로 옮긴다. 같은 설정에서 Iris(우선순위 1.0)가 서버 프레임 시간 7.64 → 6.23ms로 레거시보다 18.4%, Replication Graph보다 11.2% 짧았다(`act2-iris-base2`, `act2-iris-rg2`, `act2-iris2`). 선택, 구현, 측정, 보간 수정(`67cceff`)의 경위는 [관찰 자료](../../Posts/15-iris/candidates.md) 10\~14절, 명령은 [commands.md](../Guides/commands.md) "구성별 인자"에 있다.

## 2026-10-20 README 갱신 준비: STATUS.md에서 옮긴 측정 행 (`act2-iris-base2`, `act2-iris-rg2`, `act2-iris2`, `act2-iris-prio2`)

STATUS.md "측정 결과"에 있던 포스팅 15의 네 묶음이다. README에 ⑨와 리플리케이션 시스템 갈래 표를 더하는 작업을 시작하면서 옮겼다.

포스팅 15의 둘째 네 묶음(2026-10-19 22:30\~23:02, 본체 화면, 연달아 잼, 12회 모두 종료 코드 0, 보간 수정 `67cceff` 뒤의 빌드)은 중앙값(변동 폭)만 적는다. 모두 ⑧의 인자에 `-MotionLog`를 더했다. 실행별 행, 품질 지표, 수정 전의 첫 묶음(`act2-iris-*1`)은 [관찰 자료](../../Posts/15-iris/candidates.md) 12\~13절에 있다. Iris 묶음의 `open_actor_channels_per_conn`과 `saturated_ratio`는 늘 0이다(포화는 `lab_saturated_frames`, 모두 0). 표시 속도 오차는 Iris 기본 우선순위기만 약 98cm/s이고 나머지 셋은 15.0\~15.4cm/s다.

| 묶음 | 리플리케이션 | work_avg_ms | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn | `GameNetDriver` 프레임당(중앙값 실행) |
| --- | --- | --- | --- | --- | --- | --- |
| `act2-iris-base2` | 레거시 | 7.388(0.100) | 12.405 | 3.566 | 9791(38) | 3.346ms(`r1`) |
| `act2-iris-rg2` | Replication Graph | 6.755(0.394) | 10.897 | 2.890 | 10452(11) | 2.654ms(`r1`) |
| `act2-iris2` | Iris, 우선순위 1.0 | 5.966(0.064) | 9.307 | 2.138 | 10109(6) | 1.888ms(`r2`) |
| `act2-iris-prio2` | Iris, 기본 우선순위기 | 6.159(0.231) | 9.827 | 2.205 | 8882(26) | 1.940ms(`r1`) |
