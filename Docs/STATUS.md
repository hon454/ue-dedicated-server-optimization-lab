# 현재 상태

마지막 갱신: 2026-10-20

## 단계

**1막(포스팅 0\~4)과 2막의 포스팅 5\~15가 끝났다.** 끝낸 태스크의 경위는 아래 "포스팅 진행" 표의 작업 기록에 있다. 여기에는 다음 작업을 제약하는 것만 둔다.

- **루트 README의 결과는 2막 열한 구성(기준선, ①\~⑨, Iris)을 연달아 잰 `act2-all-*4`다(2026-10-20).** 포스팅 14를 ⑨ Replication Graph 단계로, 포스팅 15를 리플리케이션 시스템 갈래 표(레거시 ⑧, ⑨, Iris)로 더하면서 모든 구성을 다시 쟀다(01:09\~02:38, 본체 화면, 사용자가 무거운 프로그램을 닫은 뒤, `-MotionLog` 없음, 33회 모두 종료 코드 0). 값은 [누적 수치의 측정 기록](../Posts/measurements.md) "2막: 아홉 단계와 리플리케이션 시스템 갈래"에 있다. Insights 서버 프레임 시간 평균은 ⑧ 8.672, ⑨ 7.546(-13.0%), Iris 7.019(⑧보다 -19.1%, ⑨보다 -7.0%)ms이고, `out_bytes_per_sec_per_conn` 중앙값은 10,072, 10,947, 10,442다. 지난 묶음에서 변동 폭을 겨우 넘었던 ⑥의 `work_avg_ms` 증가는 이번에 구별되지 않았다. 기준선 `r1`이 또 다른 두 실행보다 7% 느렸다. 이 묶음은 포스팅 15의 묶음(10-19 밤)보다 7.7\~13.7% 느렸고, Iris와 ⑨의 차이가 11.2%에서 7.0%로 줄었다. 각 글의 값과 섞지 않는다. 지난 묶음 `act2-all-*3`의 경위는 [Worklog/12-npc-move-netserialize.md](Worklog/12-npc-move-netserialize.md)의 "README와 시각화 페이지 갱신"에 있다.
- **시리즈 웹 페이지** [UE Dedicated Server, 단계별로 최적화해 보기](https://hon454.github.io/ue-dedicated-server-optimization-lab/)는 [Site/index.html](../Site/index.html) 한 파일이고, 1막의 네 단계와 2막의 열 단계(포스팅 14까지)를 고를 수 있다. 2막에는 리플리케이션 시스템만 바꾼 갈래 카드(포스팅 15의 세 묶음, Iris는 측정값만)가 있다(2026-10-20). main의 `Site/`가 바뀐 채로 푸시되면 워크플로 `Deploy Pages`가 다시 올린다. 수치의 출처와 시뮬레이션의 한계는 [Site/README.md](../Site/README.md)에 있다. 단계를 더하는 기준은 [AGENTS.md](../AGENTS.md) "규칙"에 있다.
- 다음 시각 자료 라벨은 `visual26`이다(`visual24`, `visual25`는 포스팅 15의 Iris 기본 우선순위기와 우선순위 1.0, 2026-10-20).

## 다음 할 일

1. 다음 작업을 사용자에게 묻는다(2막에서 계획한 포스팅은 끝났다).

## 포스팅 진행

| 포스팅 | 상태 | 태그 | 작업 기록 |
| --- | --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 완료 | `post-00-testbed` | [00-testbed](Worklog/00-testbed.md)(태스크 1\~9, 14) |
| 1. Always Relevant 기준선 | 완료 | `post-01-baseline` | [01-baseline](Worklog/01-baseline.md) |
| 2. Relevancy와 Net Cull Distance(관련성) | 완료 | `post-02-relevancy` | [02-relevancy](Worklog/02-relevancy.md) |
| 3. 자원 노드 Dormancy(휴면) | 완료 | `post-03-dormancy` | [03-dormancy](Worklog/03-dormancy.md) |
| 4. AI NPC Net Update Frequency(업데이트 빈도) | 완료 | `post-04-update-frequency` | [04-update-frequency](Worklog/04-update-frequency.md) |
| 5. 테스트베드 확장과 새 기준선 | 완료 | `post-05-expanded-testbed` | [05-expanded-testbed](Worklog/05-expanded-testbed.md)(태스크 15\~21) |
| 6. 1막의 세 최적화를 2막에 다시 적용 | 완료 | `post-06-three-techniques-again` | [06-three-techniques-again](Worklog/06-three-techniques-again.md) |
| 7. 네트워크 드라이버 자체 시간 나누기 | 완료 | `post-07-net-driver-breakdown` | [07-net-driver-breakdown](Worklog/07-net-driver-breakdown.md) |
| 8. 자원 노드의 Net Update Frequency 낮추기 | 완료 | `post-08-node-update-frequency` | [08-node-update-frequency](Worklog/08-node-update-frequency.md) |
| 9. 인벤토리를 소유자에게만 보내기 | 완료 | `post-09-inventory-owner-only` | [09-inventory-owner-only](Worklog/09-inventory-owner-only.md) |
| 10. 인벤토리를 FastArray로 보내기 | 완료 | `post-10-inventory-fastarray` | [10-inventory-fastarray](Worklog/10-inventory-fastarray.md) |
| 11. 클라이언트에서 NPC 위치를 보간하기 | 완료 | `post-11-npc-interpolation` | [11-npc-interpolation](Worklog/11-npc-interpolation.md) |
| 12. NPC 이동을 필요한 비트만 담은 구조체로 보내기 | 완료 | `post-12-npc-move-netserialize` | [12-npc-move-netserialize](Worklog/12-npc-move-netserialize.md) |
| 13. 지연과 패킷 손실에서의 동기화 품질(진단) | 완료 | `post-13-lag-and-packet-loss` | [13-lag-and-packet-loss](Worklog/13-lag-and-packet-loss.md) |
| 14. 레거시 리플리케이션을 Replication Graph로 바꾸기 | 완료 | `post-14-replication-graph` | [14-replication-graph](Worklog/14-replication-graph.md) |
| 15. 레거시 리플리케이션을 Iris로 바꾸기 | 완료 | `post-15-iris` | [15-iris](Worklog/15-iris.md) |

## 명령

매번 쓰는 명령과 측정 중에 지킬 것만 둔다. 구성별 인자, 측정 절차(묶음끼리 비교하는 규칙, 리플리케이션 시간 타이머, 구성별 로그 확인), Insights, 영상과 시각 자료, 수동 확인은 [commands.md](Guides/commands.md)에 있다. 확정값과 기준선 조건은 [measurement.md](Guides/measurement.md)의 "확정값과 기준선 조건"에 있다.

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (`UnrealEditor`가 떠 있으면 스크립트가 빌드를 거부한다. 에디터나 다른 체크아웃의 실행이 끝난 뒤에 한다)
- 시나리오 실행(확정 규모, `-Label`과 `-Runs` 없이): `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Clients 8 -Nodes 5000 -Npcs 300 -Warmup 30 -Measure 60`. 측정할 때는 `-Label <새 라벨> -Runs 3`을 더한다. 서버 마스크는 스크립트 기본값 252(논리 프로세서 2\~7)이고 트레이스는 켜진다. `calib-f-r1`이 이 조건의 실행이다.
- 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 확정 규모 측정은 시작하기 전에 잴 구성, 라벨, 걸리는 시간, 화면 조건을 알리고 답을 받는다. 실행마다 서버 로그(`lab_config`, `lab_buildings`, `lab_npcs_near_players`, `lab_nodes_moved_from_harvest_spot`, 워밍업 구간의 `open_actor_channels_per_conn`)와 자동 스크린샷을 확인한다. 구성별로 확인할 로그는 commands.md "측정 절차"에 있다.
- 측정 중에는 클라이언트 창에 키 입력을 하지 않고, Insights 분석이나 빌드 같은 무거운 작업을 하지 않는다. 에이전트도 문서 편집을 포함해 다른 작업을 하지 않는다(`calib-g-r1` 실행 중의 문서 편집, [Worklog/00-testbed.md](Worklog/00-testbed.md) "태스크 9.1\~9.3"). 서버만 논리 프로세서 2\~7에 고정하므로 다른 프로그램은 그 코어를 쓸 수 있다. 에디터가 열려 있으면 스크립트가 실행을 거부한다.
- 빌드나 실행이 실패하면 [troubleshooting.md](Guides/troubleshooting.md)에서 증상을 찾는다.

## 측정 결과

진행 중인 태스크의 묶음만 둔다. CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다. 끝난 태스크의 묶음은 다음 태스크를 시작할 때 그 포스팅의 작업 기록으로 옮긴다.

진행 중인 묶음이 없다.

## 막힌 것

- 없음.

## 사용자에게 요청한 일

- 없음.
