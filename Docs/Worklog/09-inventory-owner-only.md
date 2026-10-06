# 작업 기록: 포스팅 9 인벤토리를 소유자에게만 보내기 (태스크 26)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-06 태스크 26: 인벤토리를 소유자에게만 보내기 (`act2-invown-base1`, `act2-invown1`, `visual14`, `visual15`, `tsmall-invown-*`, `tsmall-invpanel-*`)

- **태스크 26(포스팅 9, 인벤토리를 소유자에게만 보내기).** [backlog.md](../backlog.md) "우선순위 순" 5번. [2막 구현 계획](../Planning/2026-10-03-act-2-implementation-plan.md) 단계 5의 주기를 돈다. 기준 구성은 포스팅 6의 최종 구성(2막 요소 인자)에 `-NodeUpdateFrequency 2`를 더한 것이다. [관찰 자료](../../Posts/09-inventory-owner-only/candidates.md)(2026-10-06, 새 측정 없이 `act2-nodeuf1-r3`, `act2-nodeuf-split1-r3`을 다시 읽음): 인벤토리는 `Connection 0` 송신량의 28.6%(4,548바이트/초)이고, CPU는 `GameNetDriver`의 3.6%다. 바뀔 때마다 200칸을 모두 다시 보낸다. 사용자가 두 후보를 나누어 포스팅 9는 `COND_OwnerOnly`, 포스팅 10은 FastArray로 정했다(2026-10-06. 반대 순서면 뒤의 효과가 CSV 변동 폭 안이다, 관찰 자료 6절). 전환 인자 `-InventoryOwnerOnly`를 구현했다(2026-10-06, 인자는 "명령"). 조건은 `GetLifetimeReplicatedProps`가 서버 인자를 읽어 고르고, 클라이언트는 `COND_None`으로 둔 채 받는다. 작은 규모(클라이언트 2, 인벤토리 200칸, 4초)에서 화면 글자 `other items`가 200 → 0이고 `out_bytes_per_sec_per_conn`이 2,950 → 2,237, 2,265였다(`tsmall-invown-off1-r1`, `tsmall-invown-on1-r1`, `tsmall-invown-on2-r1`, 실행 한 번씩이라 방향만 본다). 오버레이 글자만으로는 독자가 알아보기 어렵다는 사용자 의견(2026-10-06)으로 인벤토리 패널을 만들었다(사용자가 2번 창에 띄우기로 정함, 인자는 "명령"). 클라이언트 8개의 작은 규모에서 0번 창에 띄워 확인했다: 끈 구성은 다른 플레이어 7명의 격자가 차 있고 하나가 통째로 번쩍였으며, 켠 구성은 7개 모두 빈 격자였다(`tsmall-invpanel-off1-r1`, `tsmall-invpanel-on1-r1`의 `tpp-01`. `out_bytes_per_sec_per_conn` 9,439 → 5,221). 사용자가 측정과 녹화 계획을 승인했다(2026-10-06). 측정이 끝났다: `act2-invown-base1`(포스팅 8의 최종 구성)과 `act2-invown1`(`-InventoryOwnerOnly`)을 12:05\~12:21에 연달아 3회씩, 본체 화면, 트레이스 켬. `out_bytes_per_sec_per_conn` 중앙값 16,635 → 12,421(-25.3%, 관찰 자료 5절의 예상 -25.0%), `work_avg_ms` 8.560 → 8.493(-0.067로 변동 폭 0.153보다 작아 구별하지 못함). 아래 "측정 결과". 패널을 `visual14`(기준), `visual15`(`-InventoryOwnerOnly`)에서 2번 창 영역 `1920,0,960,540`으로 10초씩 찍었다(`Posts/09-inventory-owner-only/images/inventory-panel-before.gif`, `-after.gif`, 각 약 5.9MB, 5.7MB. 영역이 맞음을 미리보기로 확인). Networking Insights(중앙값 실행 `r2`끼리, `Connection 0`)에서 연결당 송신량 15,864 → 11,832(-25.4%), 인벤토리 4,545 → 571바이트/초, 초당 패킷 35.5 → 30.5였다. 줄어든 몫의 98.6%가 인벤토리다. 값과 캡처 네 장은 [관찰 자료](../../Posts/09-inventory-owner-only/candidates.md) 8\~14절. 사용자가 패널 GIF와 패킷 그래프 캡처를 골랐다(2026-10-06). 본문(2,900자 안팎)과 측정 기록 초안을 썼다. GIF는 본문 요약에, 패킷 그래프는 ADR-0015에 따라 측정 기록 4절에 넣었다. 사용자가 초안을 승인했고(2026-10-06) 루트 README 표의 "(작성 중)"을 빼고 태그 `post-09-inventory-owner-only`를 붙였다.

## 2026-10-06 태스크 27 준비: STATUS.md에서 옮긴 측정 행 (`act2-invown-base1`, `act2-invown1`)

STATUS.md "측정 결과"에 있던 포스팅 9의 두 묶음이다. 포스팅 10의 기준 묶음(`act2-fastarr-base1`)을 새로 재서 옮겼다. 2026-10-06에 연달아 쟀고, Insights 값은 [포스팅 9 관찰 자료](../../Posts/09-inventory-owner-only/candidates.md) 8절부터에 있다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-invown-base1-r1` | 1788 | 8.494 | 14.090 | 0 | 4.200 | 16637 | 77 | 0.000 |
| `act2-invown-base1-r2` | 1796 | 8.560 | 13.187 | 1 | 4.157 | 16550 | 77 | 0.000 |
| `act2-invown-base1-r3` | 1789 | 8.588 | 13.161 | 0 | 4.278 | 16635 | 77 | 0.000 |
| **중앙값** | 1789 | 8.560 | 13.187 | 0 | 4.200 | 16635 | 77 | 0.000 |
| **변동 폭** | 8 | 0.094 | 0.929 | 1 | 0.121 | 87 | 0 | 0.000 |
| `act2-invown1-r1` | 1787 | 8.551 | 13.236 | 1 | 4.183 | 12421 | 77 | 0.000 |
| `act2-invown1-r2` | 1797 | 8.493 | 12.756 | 0 | 4.133 | 12441 | 77 | 0.000 |
| `act2-invown1-r3` | 1798 | 8.398 | 13.451 | 1 | 4.090 | 12408 | 77 | 0.000 |
| **중앙값** | 1797 | 8.493 | 13.236 | 1 | 4.133 | 12421 | 77 | 0.000 |
| **변동 폭** | 11 | 0.153 | 0.695 | 1 | 0.093 | 33 | 0 | 0.000 |
| **`act2-invown-base1` 대비** | +8 | -0.067(-0.8%) | +0.049(+0.4%) | +1 | -0.067(-1.6%) | -4,214(-25.3%) | 0 | 0 |
