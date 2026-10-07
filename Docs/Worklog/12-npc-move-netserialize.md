# 작업 기록: 포스팅 12 NPC 이동을 필요한 비트만 담은 구조체로 보내기 (태스크 29)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-07 태스크 29: NPC 이동을 필요한 비트만 담은 구조체로 보내기 (`act2-npcmove-base1`, `act2-npcmove1`, `tsmall-move1`, `tsmall-move2`)

- **태스크 29(포스팅 12)를 시작했다(2026-10-07).** 지금 NPC 갱신 한 번의 이동 데이터는 핸들을 포함해 41 + 3N비트(N은 위치 성분당 비트 수, `act2-interp2-r1`에서 평균 13.81)로 평균 82.43비트이고, `ServerFrame` 16비트를 더해 약 98.4비트다. 그중 Z, 속도, 위치 머리, 플래그, 회전의 "0이 아님" 비트 39비트(위치 성분이 14비트일 때)는 NPC에게 필요 없다([관찰 자료](../../Posts/12-npc-move-netserialize/candidates.md) 1절, 식은 [engine-notes.md](../Reference/engine-notes.md) 13절). 사용자가 B(집 기준 상대 좌표, 1cm, 프레임 번호를 구조체에 넣음)를 골랐고(5절) `-NpcCompactMove`로 구현했다. 작은 규모 확인(클라이언트 2, 자원 노드 100, NPC 10과 플레이어 주변 50, `-PlayerSpacing 3 -NpcsNearPlayers 50 -MotionLog -NpcInterpDelay 150`, `-NoTrace`)에서 `out_bytes_per_sec_per_conn` 8,436 → 6,183이었다(`tsmall-move1-r1`, `tsmall-move2-r1`). 클라이언트 하나가 1초에 받은 NPC 갱신 약 375번으로 나누면 갱신 한 번에 약 48비트가 줄어 예상(48.4비트)과 맞는다. 표시 위치 오차 평균 50.18 → 45.88cm, 표시 지연 169 → 153ms, 그 지연에서의 위치 오차 2.22 → 2.15cm, 표시 속도 오차 평균 12.8 → 13.0cm/s, `lab_npc_move_clamped`는 없었다. 작은 규모라 포스팅의 비교에 쓰지 않는다.
- **[사람] 포스팅 12(태스크 29)의 초안을 확인한다.** [본문](../../Posts/12-npc-move-netserialize/README.md)과 [측정 기록](../../Posts/12-npc-move-netserialize/measurements.md)의 초안이다(본문 3,126자). "문제"와 "원리"는 승인이 필요한 에이전트 초안이다. 시각 자료는 사용자가 추천안대로 골랐다(비트 구성 도식 `Scripts/make-move-bits.ps1`, 흐름도, 송신 대역폭 차트, 측정 기록의 Net Stats 캡처 두 장, 영상 없음). Networking Insights(`Connection 0`, `Outgoing`)에서 `LabNpc` 갱신 한 번이 117.24 → 69.04비트, `Move`는 25,189번 모두 50비트였다(`act2-npcmove-base1-r2`, `act2-npcmove1-r2`). `Shared`의 합으로 `Move`가 공유 직렬화로 갔음을 확인했다(engine-notes.md 13절). 승인되면 태그 `post-12-npc-move-netserialize`를 붙이고, 루트 README의 "결과 한눈에 보기"와 시리즈 웹 페이지에 단계를 더한다(AGENTS.md "규칙").

## 2026-10-07 태스크 30 준비: STATUS.md에서 옮긴 측정 행 (`act2-npcmove-base1`, `act2-npcmove1`)

STATUS.md "측정 결과"에 있던 포스팅 12의 두 묶음과 품질 지표다. 포스팅 13(태스크 30)을 시작하면서 옮겼다. `act2-npcmove-base1`(포스팅 11의 최종 구성에 `-MotionLog`)과 `act2-npcmove1`(`-NpcCompactMove`)은 2026-10-07 12:07\~12:23에 본체 화면에서 연달아 쟀다. 6회 모두 종료 코드 0이고 `lab_npc_move_clamped`는 없었다. [measurement.md](../Guides/measurement.md)의 기준(중앙값의 변화가 큰 쪽 변동 폭보다 큼)으로 `out_bytes_per_sec_per_conn`(-2,584, 예상 -2,589), `work_p99_ms`, `netflush_avg_ms`는 구별되고, `work_avg_ms`는 변화 0.197이 변동 폭 0.190을 겨우 넘는다. 서버가 모션 기록에 쓴 시간은 프레임당 0.0687\~0.0721ms다(서버 로그). 중앙값 실행(두 묶음 모두 `r2`)의 Insights 값은 서버 프레임 시간 평균 8.957 → 8.760ms, `GameNetDriver` 프레임당 3.993 → 3.824ms(Excl 2.449 → 2.337ms), 그중 `LabNpc` 0.884 → 0.842ms다(`export-insights.ps1`).

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-npcmove-base1-r1` | 1797 | 8.606 | 12.897 | 0 | 4.198 | 12674 | 77 | 0.000 |
| `act2-npcmove-base1-r2` | 1788 | 8.681 | 13.237 | 1 | 4.246 | 12672 | 77 | 0.000 |
| `act2-npcmove-base1-r3` | 1797 | 8.691 | 13.387 | 1 | 4.253 | 12662 | 77 | 0.000 |
| **중앙값** | 1797 | 8.681 | 13.237 | 1 | 4.246 | 12672 | 77 | 0.000 |
| **변동 폭** | 9 | 0.085 | 0.490 | 1 | 0.055 | 12 | 0 | 0.000 |
| `act2-npcmove1-r1` | 1796 | 8.673 | 12.446 | 0 | 4.195 | 10083 | 77 | 0.000 |
| `act2-npcmove1-r2` | 1790 | 8.484 | 12.246 | 1 | 4.075 | 10088 | 77 | 0.000 |
| `act2-npcmove1-r3` | 1788 | 8.483 | 12.279 | 0 | 4.059 | 10104 | 77 | 0.000 |
| **중앙값** | 1790 | 8.484 | 12.279 | 0 | 4.075 | 10088 | 77 | 0.000 |
| **변동 폭** | 8 | 0.190 | 0.200 | 1 | 0.136 | 21 | 0 | 0.000 |
| **`act2-npcmove-base1` 대비** | -7 | -0.197(-2.3%) | -0.958(-7.2%) | -1 | -0.171(-4.0%) | -2,584(-20.4%) | 0 | 0 |

품질 지표([ADR-0020](../Decisions/0020-npc-motion-quality-metrics.md), `analyze-motion.ps1`, 클라이언트 8개, 측정 구간 60초, NPC 66개). 세 실행의 중앙값(변동 폭)이다. 두 묶음은 모든 지표에서 구별되지 않는다.

| 지표 | `act2-npcmove-base1` | `act2-npcmove1` |
| --- | --- | --- |
| 표시 위치 오차 평균 / P99(cm) | 46.56(1.01) / 52.93(3.90) | 45.87(0.63) / 49.70(4.08) |
| 표시 속도 오차 평균 / P99(cm/s) | 14.9(0.3) / 337.8(6.3) | 15.0(0.4) / 328.1(6.7) |
| 표시 지연(ms) / 그 지연에서의 위치 오차 평균(cm) | 155 / 1.37(0.24) | 154 / 1.22(0.26) |
| 수신 간격 평균(ms) | 133.4 | 133.5 |
| 클라이언트 프레임 간격 평균(ms) | 35.42 | 35.25 |
