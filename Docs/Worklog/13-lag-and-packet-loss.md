# 작업 기록: 포스팅 13 지연과 패킷 손실에서의 동기화 품질 (태스크 30)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-07 태스크 30: 지연과 패킷 손실에서의 동기화 품질 (`act2-net-base1`, `act2-net-avg1`, `act2-net-bad1`, `act2-net-lag1`, `act2-net-loss1`, `tsmall-net1`\~`net4`, `visual22`, `visual23`)

- **태스크 30(포스팅 13, `post-13-lag-and-packet-loss`, 진단)이 끝났다(2026-10-07).** 사용자 결정: 시뮬레이션은 클라이언트에 건다, 다섯 묶음(루프백, `Average`, `Bad`, `LabLagOnly`, `LabLossOnly`)을 잰다, 재는 것은 NPC뿐이다, 표시 지연의 탐색 범위를 600ms로 넓힌다(ADR 없이 measurement.md에 적음). 구현은 `run-scenario.ps1 -NetEmulationProfile`, `Config/DefaultEngine.ini`의 두 프로필, 서버의 기록용 `-LabNetEmulationProfile=`이다("명령"). 작은 규모 확인(`tsmall-net1`\~`net4`)에서 손실만은 예상대로 수신 간격 P99가 267ms가 되어 NPC가 118ms씩 멈췄고, 지연만은 예상과 달리 갱신마다 한 프레임 멈추고 두 걸음을 갔다(표시 속도 오차 12.7 → 70.2cm/s). 보간 버퍼 150ms가 갱신 간격 134ms에 16ms 여유뿐이라서다. 값과 확인 과정은 [관찰 자료](../../Posts/13-lag-and-packet-loss/candidates.md) 3\~4절, 엔진 소스는 [engine-notes.md](../Reference/engine-notes.md) 14절에 있다. 확정 규모의 다섯 묶음(`act2-net-base1`, `act2-net-avg1`, `act2-net-bad1`, `act2-net-lag1`, `act2-net-loss1`)을 2026-10-07 17:49\~18:30에 연달아 쟀고 15회 모두 종료 코드 0이었다(아래 "측정 결과"). 표시 지연은 루프백 154 → 지연만 286ms, 표시 속도 오차 평균은 15.1 → 지연만 112, 손실만 38.4, 나쁨 134cm/s였다. 지연을 건 묶음에서 송신 대역폭이 9% 줄고 `work_avg_ms`가 0.4 줄었는데 리플리케이션 시간은 거의 같았다(원인 미확인, [측정 기록](../../Posts/13-lag-and-packet-loss/measurements.md) 7절). [본문](../../Posts/13-lag-and-packet-loss/README.md) 초안(3,695자)과 측정 기록을 썼고, 위치 그래프 세 장은 `Scripts/make-net-trace.ps1`로 그렸다. 사용자가 "문제"와 "원리"를 승인하고 영상을 찍기로 해, 시연용 NPC를 루프백(`visual22`)과 `Bad`(`visual23`)에서 찍어 요약의 GIF(862KB)로 붙이고 태그를 붙였다. 다음 태스크를 시작할 때 이 줄의 경위를 `Docs/Worklog/13-lag-and-packet-loss.md`로 옮긴다.

## 2026-10-19 태스크 31 준비: STATUS.md에서 옮긴 측정 행 (`act2-net-base1`, `act2-net-avg1`, `act2-net-bad1`, `act2-net-lag1`, `act2-net-loss1`)

STATUS.md "측정 결과"에 있던 포스팅 13의 다섯 묶음이다. 포스팅 14(태스크 31)를 시작하면서 옮겼다.

포스팅 13의 다섯 묶음(2026-10-07 17:49\~18:30, 본체 화면, 연달아 잼)은 중앙값(변동 폭)만 적는다. 실행별 행과 품질 지표는 [포스팅 13 측정 기록](../../Posts/13-lag-and-packet-loss/measurements.md) 3절, 4절에 있다. 서버는 다섯 묶음이 같고 클라이언트의 패킷 시뮬레이션 프로필만 다르다.

| 묶음 | 프로필 | work_avg_ms | out_bytes_per_sec_per_conn | 표시 지연(ms) | 표시 속도 오차 평균(cm/s) | 수신 간격 P99(ms) |
| --- | --- | --- | --- | --- | --- | --- |
| `act2-net-base1` | 없음 | 8.759(0.058) | 10084(22) | 154(2) | 15.1(0.2) | 166.8(0.2) |
| `act2-net-avg1` | `Average` | 8.803(0.578) | 9760(62) | 196(4) | 41.1(5.8) | 233.3(13.5) |
| `act2-net-bad1` | `Bad` | 8.285(0.024) | 9179(17) | 287(1) | 134.1(4.4) | 304.8(5.9) |
| `act2-net-lag1` | `LabLagOnly` | 8.358(0.057) | 9183(10) | 286(0) | 112.1(3.2) | 220.9(2.9) |
| `act2-net-loss1` | `LabLossOnly` | 8.659(0.123) | 10003(7) | 154(0) | 38.4(1.3) | 282.1(3.6) |
