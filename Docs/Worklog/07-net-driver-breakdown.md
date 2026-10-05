# 작업 기록: 포스팅 7 네트워크 드라이버 자체 시간 나누기 (태스크 24)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-05 태스크 24: GameNetDriver 자체 시간 나누기 (`act2-split-base1`, `act2-split1`, `act2-split2`, `act2-split-nodes2500-1`)

- **태스크 24(포스팅 7, `GameNetDriver` 자체 시간 나누기, 진단).** 순서는 [backlog.md](../backlog.md) "우선순위 순"(태스크 23). [2막 구현 계획](../Planning/2026-10-03-act-2-implementation-plan.md) 단계 5의 주기에서 3, 4를 건너뛴다. 측정과 Insights 값까지 끝났다(2026-10-05): 기준 묶음 `act2-split-base1`(`work_avg_ms` 중앙값 17.358, 변동 폭 0.155)과 나누는 묶음 `act2-split1`(`-StatNamedEvents`, 17.703, 0.229). `GameNetDriver` Incl 가운데 `Prioritize Actors Time` 56.4%, `Consider Actors Time` 23.5%이고 활성 목록 5,272개 가운데 자원 노드가 4,887개다. 값과 의견은 [관찰 자료](../../Posts/07-net-driver-breakdown/candidates.md). 해석은 사용자가 승인했고(2026-10-05) 본문과 [측정 기록](../../Posts/07-net-driver-breakdown/measurements.md) 초안을 썼다. **[사람] 초안을 읽고 고칠 곳과 진단 포스팅의 틀(posting.md "틀")을 정한다.** 그 뒤 Insights 캡처 후보, 태그, Worklog 정리가 남았다. 대조 실행 `act2-split2`(18.472, 변동 폭 4.189)와 `act2-split-nodes2500-1`(`-Nodes 2500`, 12.479, 0.357)에서 따지는 두 단계가 실행마다의 빠르기를 지운 값으로 51.2% 남아 활성 목록 길이 비례(53.8%)와 가깝다(관찰 자료 6절).
- 2026-10-06: 사용자가 초안을 고칠 곳 없이 확정했고, 진단 포스팅의 틀을 초안의 구성(기법 글의 여섯 절에서 "적용" 자리에 "측정")으로 정했다([posting.md](../Guides/posting.md) "틀"). 분량 지침은 [ADR-0017](../Decisions/0017-post-length-range.md)로 바뀌었다.
- 2026-10-06: `act2-split1-r3`의 `GameNetDriver` Callees 캡처 두 장(잘라 낸 것과 창 전체)을 측정 기록 5절에 넣었다. 처음 조작 권한 요청은 거절되었고, 사용자가 다시 요청하라고 해서 허용받았다. 루트 README 표의 "(작성 중)"을 빼고 태그 `post-07-net-driver-breakdown`을 붙였다.
