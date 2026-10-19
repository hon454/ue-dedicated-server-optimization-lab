# 작업 기록: 포스팅 14 레거시 리플리케이션을 Replication Graph로 바꾸기 (태스크 31)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-19 태스크 31: 레거시 리플리케이션을 Replication Graph로 바꾸기 (`act2-rg-base-nodedef1`, `act2-rg-base1`, `act2-rg-nodedef1`, `act2-rg1`, `act2-rg-trk-base1`, `act2-rg-trk1`, `act2-rg-notrk1`, `tsmall-rg-base2`, `tsmall-rg2`)

- **태스크 31(포스팅 14, `post-14-replication-graph`)이 끝났다(2026-10-19).** 다음 태스크를 시작할 때 이 줄의 경위를 `Docs/Worklog/14-replication-graph.md`로 옮긴다. [관찰 자료](../../Posts/14-replication-graph/candidates.md)를 지난 실행(`act2-net-base1-r3`, `act2-nodeuf-split1-r3`)과 엔진 소스로 썼다. 남은 것은 활성 목록을 따지는 두 단계(`Consider Actors Time` 0.959, `Prioritize Actors Time` 0.845ms, ④에서 잼)다. 그래프는 이 두 단계를 돌지 않고 연결마다 시점이 있는 격자 칸에서만 모은다(`NetDriver.cpp:6295-6298`). 그래프 묶음에서는 Consider List 길이 지표가 남지 않는다. 사용자가 B(동적 액터 버킷을 하나로 둔 그래프)와 2 × 2(레거시/그래프 × 자원 노드 빈도 기본값/2)를 골랐다(관찰 자료 7절). `ULabReplicationGraph`를 `-ReplicationGraph`로 구현했다. 클래스마다 보내는 간격을 레거시의 평균 고려 간격((30 ÷ 빈도) + 1프레임)에 맞췄고 디버그 액터를 끄며, 작은 규모에서 채널 수 58과 NPC 수신 간격 133.5 → 133.7ms가 레거시와 같았다(`tsmall-rg-base2-r1`, `tsmall-rg2-r1`). 확정 규모 2 × 2(아래 "측정 결과")에서 그래프는 `work_avg_ms`를 ⑧보다 1.225 줄였고, 자원 노드 빈도를 기본값으로 둬도 차이가 변동 폭 안이었다. 품질 지표는 네 묶음이 구별되지 않았다. 대신 송신량이 8.8% 늘었다. 그래프는 캐릭터를 매 프레임 보내서(레거시 평균 1.3프레임) 캐릭터 송신이 29% 늘었고, 포스팅 12의 캐릭터 송신량으로 셈한 증가 약 894바이트/초가 CSV의 889와 맞는다(관찰 자료 9.5절). 또 클라이언트가 가진 자원 노드가 176 → 380개였다(`slot=1` 자동 스크린샷, t=75s). 그래프의 Dormancy 노드가 거리에서 걸러진 액터를 한 프레임에 하나씩 거리 무시로 보내는 trickle 때문이다(`Net.RepGraph.TrickleDistCullOnDormancyNodes` 기본값 1, `ReplicationGraph.cpp:165, 4717-4721`). 사용자가 [블로그 글](https://hon454.github.io/posts/unreal-engine-replication-graph/)을 보고 trickle을 끈 묶음을 더 재고, 본문에서 trickle을 인용구로 설명하기로 했다(관찰 자료 7절). `-RepGraphNoTrickle`을 구현해 둘째 묶음(아래 "측정 결과")을 쟀다. trickle을 끄면 클라이언트의 자원 노드가 레거시와 같은 176개이고 송신량은 그대로이며, 그래프 자체 시간이 0.41ms 늘어 `work_avg_ms`가 7.362 → 7.763이다(레거시 8.555).

## 2026-10-19 태스크 32 준비: STATUS.md에서 옮긴 측정 행 (`act2-rg-base-nodedef1`, `act2-rg-base1`, `act2-rg-nodedef1`, `act2-rg1`, `act2-rg-trk-base1`, `act2-rg-trk1`, `act2-rg-notrk1`)

STATUS.md "측정 결과"에 있던 포스팅 14의 일곱 묶음이다. 포스팅 15(태스크 32)를 시작하면서 옮겼다.

포스팅 14의 네 묶음(2026-10-19 16:22\~16:54, 본체 화면, 연달아 잼, 12회 모두 종료 코드 0)은 중앙값(변동 폭)만 적는다. 모두 ⑧의 인자에 `-MotionLog`를 더했다. 실행별 행과 품질 지표는 [관찰 자료](../../Posts/14-replication-graph/candidates.md) 9절에 있다.

| 묶음 | 리플리케이션, 자원 노드 빈도 | work_avg_ms | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn | `GameNetDriver` 프레임당(중앙값 실행) |
| --- | --- | --- | --- | --- | --- | --- |
| `act2-rg-base-nodedef1` | 레거시, 100 | 17.506(2.748) | 25.237 | 12.963 | 10097(43) | 12.681ms(`r1`) |
| `act2-rg-base1` | 레거시, 2(⑧) | 8.662(0.082) | 12.811 | 4.209 | 10073(14) | 3.958ms(`r3`) |
| `act2-rg-nodedef1` | 그래프, 100 | 7.537(0.332) | 10.945 | 3.195 | 10954(16) | 2.946ms(`r3`) |
| `act2-rg1` | 그래프, 2 | 7.437(0.071) | 10.370 | 3.168 | 10962(47) | 2.923ms(`r1`) |

둘째 묶음(2026-10-19 17:30\~17:53, 같은 조건, 9회 모두 종료 코드 0. 모두 ⑧에 `-MotionLog`, 자원 노드 빈도 2). 첫 묶음과는 비교하지 않는다. 실행별 행은 관찰 자료 10절.

| 묶음 | 리플리케이션 | work_avg_ms | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn | `GameNetDriver` 프레임당(`r3`) |
| --- | --- | --- | --- | --- | --- | --- |
| `act2-rg-trk-base1` | 레거시 | 8.555(0.169) | 12.978 | 4.148 | 10080(32) | 3.889ms |
| `act2-rg-trk1` | 그래프 | 7.362(0.076) | 10.626 | 3.126 | 10969(21) | 2.880ms |
| `act2-rg-notrk1` | 그래프, trickle 끔 | 7.763(0.341) | 11.492 | 3.536 | 10964(30) | 3.292ms |
