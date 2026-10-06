# 포스팅 7 관찰 자료 (`act2-split-base1`, `act2-split1`, `act2-split2`, `act2-split-nodes2500-1`)

2막 구현 계획 태스크 24의 자료다. 기법 없이 진단만 하는 포스팅이라 후보 기법 목록 대신 `GameNetDriver` 자체 시간(Excl)의 내역을 적는다. 에이전트가 트레이스를 창 없이 `Scripts/export-insights.ps1`로 읽었다.

- 구성: 포스팅 6의 최종 구성(`config` 열 `default`, 2막 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`), 확정 규모(클라이언트 8, 자원 노드 5,001, 맵 전체의 NPC 300, 준비 30초, 측정 60초).
- 실행: 기준 묶음 `act2-split-base1-r1`\~`r3`(인자 없음), 나누는 묶음 `act2-split1-r1`\~`r3`(`run-scenario.ps1 -StatNamedEvents`, 서버에 `-statnamedevents`). 2026-10-05 22:16\~22:31에 연달아 잼, 본체 화면, 실행 중 PC 조작 없음. 빌드는 커밋 `3a82d7b`의 소스다. 6회 모두 종료 코드 0이고 선호도 재설정이 없었다.
- `act2-split1`은 이벤트를 더 기록하므로 수치를 다른 실행과 비교하지 않고, `GameNetDriver` 안의 비율만 본다(2절 끝의 비용은 참고값이다).
- 중앙값 실행(`work_avg_ms` 기준): `act2-split-base1-r2`, `act2-split1-r3`.
- 프레임당 값은 Incl ÷ 프레임 수다. 프레임 수는 기준 묶음에서 `WorldTick` Count, 나누는 묶음에서 `GameNetDriver` Count다(이 트레이스에서는 `WorldTick`이 프레임을 감싸지 않는다. [engine-notes.md](../../Docs/Reference/engine-notes.md) 9절). 둘 다 CSV `frames`와 같았다.
- 1\~6절은 내보낸 값과 그 값으로 계산한 값이다. 7절 "에이전트 의견"만 해석이다. 6절은 자원 노드 수를 바꾼 대조 실행(`act2-split2`, `act2-split-nodes2500-1`)이다.

## 1. CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-split-base1-r1` | 1788 | 17.238 | 25.645 | 1 | 12.961 | 16601 | 77 | 0.000 |
| `act2-split-base1-r2` | 1788 | 17.358 | 26.568 | 1 | 13.024 | 16622 | 77 | 0.000 |
| `act2-split-base1-r3` | 1797 | 17.393 | 25.857 | 1 | 13.016 | 16626 | 77 | 0.000 |
| **중앙값** | 1788 | 17.358 | 25.857 | 1 | 13.016 | 16622 | 77 | 0.000 |
| **변동 폭** | 9 | 0.155 | 0.923 | 0 | 0.063 | 25 | 0 | 0.000 |
| `act2-split1-r1` | 1787 | 17.679 | 26.013 | 1 | 13.053 | 16614 | 77 | 0.000 |
| `act2-split1-r2` | 1788 | 17.908 | 26.202 | 1 | 13.223 | 16592 | 77 | 0.000 |
| `act2-split1-r3` | 1787 | 17.703 | 25.928 | 1 | 13.061 | 16636 | 77 | 0.000 |
| **중앙값** | 1787 | 17.703 | 26.013 | 1 | 13.061 | 16614 | 77 | 0.000 |
| **변동 폭** | 1 | 0.229 | 0.274 | 0 | 0.170 | 44 | 0 | 0.000 |

- 채널 수는 측정 시작 전에 77로 안정됐다(서버 로그의 준비 구간 마지막 세 줄). `act2-split-base1-r1`만 마지막 줄(측정 시작 3초 전)이 79였고 CSV의 측정 구간 평균은 77이다.
- 서버 로그의 `lab_config`, `lab_buildings clusters=1 per_cluster=500`, `lab_npcs_near_players clusters=1 per_cluster=50`, `lab_nodes_moved_from_harvest_spot=1`이 여섯 실행 모두 같았다. 자동 스크린샷도 정상이다(내려다보기 화면에서 자원 노드 169개, `act2-split1-r2-topdown-02.png`).

## 2. 기본 트레이스의 `GameNetDriver` (기준 묶음, 프레임당 ms)

| 실행 | Incl | Excl | Excl ÷ Incl |
| --- | ---: | ---: | ---: |
| `act2-split-base1-r1` | 12.691 | 10.855 | 85.5% |
| `act2-split-base1-r2` | 12.755 | 10.930 | 85.7% |
| `act2-split-base1-r3` | 12.736 | 10.921 | 85.7% |

`GameNetDriver` 바로 아래 클래스 타이머는 `LabNpc` 0.959, `LabCharacter` 0.622이고 나머지는 모두 0.07 이하다(`r2`). 포스팅 6의 `act2-update-frequency1-r1`(Incl 11.967, Excl 10.141, 84.7%)과 비율이 같다. 몇 시간 떨어진 묶음이라 값은 비교하지 않는다.

named events의 비용(참고): `work_avg_ms` 중앙값이 17.358에서 17.703으로 0.345(2.0%) 늘었다. `GameNetDriver` Incl은 중앙값 실행끼리 12.755와 12.787로 0.3% 차이다.

## 3. `GameNetDriver` 안의 내역 (나누는 묶음, 프레임당 ms)

비율은 `GameNetDriver` Incl에 대한 값이다. 트리는 `GameNetDriver` → `ServerReplicateActors Time` → 아래 단계 순서다(`NetDriver.cpp:6279`). 연결 틱은 `STAT_NetDriver_TickClientConnections` 아래에 있다(`NetDriver.cpp:1269`).

| 타이머(stat, 소스 위치) | 프레임당 Count | `r1` | `r2` | `r3`(중앙값) | `r3` 비율 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `GameNetDriver` Incl | 1.0 | 12.776 | 12.944 | 12.787 | 100% |
| `Prioritize Actors Time`(`STAT_NetPrioritizeActorsTime`, `NetDriver.cpp:5530`) | 8.0 | 7.166 | 7.343 | 7.208 | 56.4% |
| `Consider Actors Time`(`STAT_NetConsiderActorsTime`, `NetDriver.cpp:5305`) | 1.0 | 3.031 | 3.003 | 3.003 | 23.5% |
| `Process Prioritized Actors Time`(`NetDriver.cpp:5689`) Incl | 8.0 | 2.110 | 2.125 | 2.114 | 16.5% |
| 그 가운데 `Replicate Actor Time`(`DataChannel.cpp:3608`) Incl | 184 | 1.934 | 1.955 | 1.943 | 15.2% |
| `NetConnection Tick`(`Stat_NetConnectionTick`, `NetConnection.cpp:4783`) Incl | 8.0 | 0.273 | 0.278 | 0.269 | 2.1% |
| 그 가운데 `IpConnection Socket SendTo` | 8.0 | 0.158 | 0.162 | 0.154 | 1.2% |
| `ServerReplicateActors Time` Excl | 1.0 | 0.095 | 0.094 | 0.093 | 0.7% |
| `GameNetDriver` Excl | 1.0 | 0.044 | 0.044 | 0.043 | 0.3% |

- `Prioritize Actors Time`과 `Consider Actors Time`의 합은 10.211ms, `GameNetDriver` Incl의 79.9%다(`r3`). 둘 다 Excl이 Incl과 같아 아래에 더 나뉘는 타이머가 없다.
- 클래스 타이머는 `Replicate Actor Time` 아래에 있다. `LabNpc` 프레임당 114.9번 1.030ms, `LabCharacter` 49.7번 0.666ms, `GameplayDebuggerCategoryReplicator` 6.2번 0.067ms(`r3`). 횟수는 기본 트레이스(`LabNpc` 114.9번, `LabCharacter` 49.4번, `act2-split-base1-r2`)와 같다.
- 프레임당 Count 8은 연결 8개에 한 번씩이다. `Consider Actors Time`은 프레임에 한 번 활성 목록 전체를 돈다(`ServerReplicateActors_BuildConsiderList`, `NetDriver.cpp:5303-5455`). `Prioritize Actors Time`은 연결마다 Consider List 전체를 돈다(`ServerReplicateActors_PrioritizeActors`, `NetDriver.cpp:5528-5679`).

## 4. 활성 목록의 액터 수 (서버 로그 `lab_network_objects`)

측정이 끝난 뒤 서버가 한 번 센 값이다(`LabMetricsSubsystem::LogNetworkObjects`). 칸은 활성 목록의 수 / 그 가운데 일부 연결에서 Dormant 상태인 수 / 모든 연결에서 Dormant 상태라 활성 목록에서 빠진 수다.

| 클래스 | `act2-split-base1-r2`, `act2-split1-r1`, `r3` | `act2-split-base1-r1`, `act2-split1-r2` |
| --- | --- | --- |
| 합계(활성 / 빠짐) | 5,272 / 613 | 5,271 / 614 |
| `LabResourceNode` | 4,887 / 76 / 114 | 4,886 / 75 / 115 |
| `LabNpc` | 350 / 0 / 0 | 350 / 0 / 0 |
| `LabBuilding` | 1 / 0 / 499 | 1 / 0 / 499 |
| 플레이어마다 하나인 것(`BP_LabCharacter_C`, `LabPlayerController`, `PlayerState`, `GameplayDebuggerCategoryReplicator`) | 각 8 / 0 / 0 | 같음 |
| `WorldSettings`, `GameStateBase` | 각 1 / 0 / 0 | 같음 |

- `act2-split-base1-r3`도 5,272 / 613이고 `LabResourceNode` 4,887 / 76 / 114다. `act2-split1-r1`의 `LabBuilding`은 1 / 1 / 499다.
- 자원 노드 5,001개 가운데 4,887개(97.7%)가 활성 목록에 남았고, 활성 목록 5,272개의 92.7%가 자원 노드다.
- 자원 노드는 `NetUpdateFrequency`를 바꾸지 않아 엔진 기본값 100이다(`Source/DSOptLab/LabResourceNode.cpp`에 설정 없음, `Actor.cpp:295`). 1 ÷ 100초는 프레임 간격(1 ÷ 30초)보다 짧아서 활성 목록에 있는 자원 노드는 매 프레임 Consider List에 들어간다(`NetDriver.cpp:5319`의 `NextUpdateTime` 검사). NPC는 `NetUpdateFrequency` 10이라 매 프레임 들어가지 않는다.

계산(근사): 활성 목록 하나에 `Consider Actors Time` 3.003 ÷ 5,272 = 0.570µs, 활성 목록 하나와 연결 하나에 `Prioritize Actors Time` 7.208 ÷ (5,272 × 8) = 0.171µs. Consider List의 실제 길이는 세지 않았다. NPC 일부가 빠지므로 활성 목록보다 조금 짧다.

## 5. `GameNetDriver` 밖의 `TickCompletionEvents` (프레임당 ms)

포스팅 6에서 정체를 모른다고 적은 타이머다(2.462ms, `act2-update-frequency1-r1`). 기본 트레이스에서는 아래의 `ProcessUntilTasksComplete` Excl이 Incl의 39.0%라 나뉘지 않는다(`act2-split-base1-r2`, 1.643 ÷ 4.208초). 나누는 묶음에서는 5.3%만 남는다.

| 타이머 | 프레임당 Count | 프레임당 ms(`act2-split1-r3`) | `TickCompletionEvents` Incl에 대한 비율 |
| --- | ---: | ---: | ---: |
| `TickCompletionEvents` Incl | 4.0 | 2.575 | 100% |
| `FTickFunctionTask` Incl | 402 | 2.418 | 93.9% |
| NPC의 액터 틱(`LabNpc`) | 350 | 0.830 | 32.2% |
| 그 가운데 `MoveComponent(Primitive) Time` | 350 | 0.604 | 23.4% |
| 물리 시작(`FStartPhysicsTickFunction_ExecuteTick`) | 1.0 | 0.747 | 29.0% |
| 플레이어 캐릭터의 메시(`CharacterMesh0`, 애니메이션 갱신) | 8.0 | 0.371 | 14.4% |
| 물리 끝(`FEndPhysicsTickFunction_ExecuteTick`) | 1.0 | 0.209 | 8.1% |
| 플레이어 캐릭터의 카메라 붐(`CameraBoom`) | 8.0 | 0.093 | 3.6% |

기본 트레이스의 `TickCompletionEvents`는 프레임당 2.376ms다(`act2-split-base1-r2`, 4.248 ÷ 1,788초 × 1,000).

## 6. 자원 노드 수를 절반으로 줄인 대조 실행 (`act2-split2`, `act2-split-nodes2500-1`)

5절까지의 가정(자원 노드 하나를 따지는 시간이 다른 액터와 같다)을 확인하려고 잰 묶음이다. 둘 다 `-StatNamedEvents`이고, 2026-10-05 23:26\~23:42에 연달아 잼, 본체 화면, 실행 중 PC 조작 없음. `act2-split2`는 3\~5절과 같은 구성이고 `act2-split-nodes2500-1`은 `-Nodes 2500`만 다르다. 6회 모두 종료 코드 0이고 선호도 재설정이 없었다.

CSV:

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-split2-r1` | 1795 | 14.853 | 22.131 | 1 | 10.767 | 16235 | 77 |
| `act2-split2-r2` | 1787 | 18.472 | 27.725 | 1 | 13.682 | 16628 | 77 |
| `act2-split2-r3` | 1787 | 19.042 | 28.229 | 4 | 14.202 | 16584 | 77 |
| **중앙값** | 1787 | 18.472 | 27.725 | 1 | 13.682 | 16584 | 77 |
| **변동 폭** | 8 | 4.189 | 6.098 | 3 | 3.435 | 393 | 0 |
| `act2-split-nodes2500-1-r1` | 1785 | 12.824 | 18.901 | 1 | 8.126 | 16629 | 78 |
| `act2-split-nodes2500-1-r2` | 1790 | 12.467 | 18.172 | 1 | 7.777 | 16698 | 78 |
| `act2-split-nodes2500-1-r3` | 1796 | 12.479 | 17.946 | 1 | 7.776 | 16656 | 78 |
| **중앙값** | 1790 | 12.479 | 18.172 | 1 | 7.777 | 16656 | 78 |
| **변동 폭** | 11 | 0.357 | 0.955 | 0 | 0.350 | 69 | 0 |

활성 목록(서버 로그 `lab_network_objects`): `act2-split2`는 5,272개(`r3`만 5,271)이고 그 가운데 자원 노드 4,887개(`r3` 4,886)다. `act2-split-nodes2500-1`은 세 실행 모두 2,835개이고 자원 노드 2,450개, 모든 연결에서 휴면이라 빠진 자원 노드 51개다. 활성 목록이 2,437개(46.2%) 줄었다.

`GameNetDriver` 안(프레임당 ms). "따지는 두 단계"는 `Prioritize Actors Time` + `Consider Actors Time`이다.

| 실행 | `GameNetDriver` Incl | `Prioritize Actors Time` | `Consider Actors Time` | 따지는 두 단계 | `Process Prioritized Actors Time` | 두 단계 ÷ `Process Prioritized` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-split2-r1` | 10.504 | 5.720 | 2.562 | 8.282 | 1.820 | 4.55 |
| `act2-split2-r2` | 13.402 | 7.565 | 3.215 | 10.780 | 2.145 | 5.03 |
| `act2-split2-r3` | 13.899 | 7.668 | 3.488 | 11.156 | 2.233 | 5.00 |
| `act2-split-nodes2500-1-r1` | 7.841 | 3.419 | 1.888 | 5.307 | 2.075 | 2.56 |
| `act2-split-nodes2500-1-r2` | 7.513 | 3.290 | 1.829 | 5.119 | 1.971 | 2.60 |
| `act2-split-nodes2500-1-r3` | 7.504 | 3.378 | 1.663 | 5.041 | 2.029 | 2.48 |

- 중앙값 실행끼리(`act2-split2-r2`, `act2-split-nodes2500-1-r3`) 따지는 두 단계가 10.780에서 5.041로 53.2% 줄었다. 활성 목록은 46.2% 줄었다.
- `act2-split2-r1`은 모든 타이머가 함께 빠르다. 처리 횟수는 같고(`LabNpc` 114.5번, `r2` 114.9번) `Process Prioritized Actors Time`도 1.820으로 `r2`의 0.85배다. 같은 일을 빠르게 한 실행이다(포스팅 6 관찰 자료 5절과 같은 모양).
- 그래서 실행마다의 빠르기를 지우려고, 자원 노드 수와 상관없는 `Process Prioritized Actors Time`으로 나눈 값도 적었다. 중앙값은 `act2-split2` 5.00, `act2-split-nodes2500-1` 2.56이고, 줄어든 뒤의 비율은 2.56 ÷ 5.00 = 51.2%다. 활성 목록 길이에 비례한다면 2,835 ÷ 5,272 = 53.8%다.
- NPC 배치가 달라진 영향은 작다. 맵 전체의 NPC는 자원 노드와 같은 난수에서 위치를 뽑으므로(`Source/DSOptLab/LabGameMode.cpp:82-122`) 자원 노드 수를 바꾸면 NPC 배치도 바뀐다. 열린 채널은 77개와 78개, `LabNpc` 처리 횟수는 프레임당 114.9번과 116.0번(`r2`, `r3`), `Process Prioritized Actors Time`은 2.145와 2.029다.
- 계산(근사): 정규화한 값의 차이 5.00 − 2.56 = 2.44가 자원 노드 2,437개의 몫이다. 같은 비율이면 자원 노드 4,887개의 몫은 2.44 × 4,887 ÷ 2,437 = 4.89, 따지는 두 단계(5.00)의 98%다. 3\~5절과 같은 묶음 `act2-split1`의 정규화 값 4.83(`r3`, 10.211 ÷ 2.114)을 쓰면 94%다.
- 서버 프레임 시간은 18.771ms에서 12.765ms로 32.0% 줄었다(Timing Insights, 중앙값 실행).

## 7. 에이전트 의견

- **가설은 맞는 쪽이다.** 세 기법 뒤의 리플리케이션 시간에서 가장 큰 두 단계가 Consider List를 만드는 일(23.5%)과 연결마다 우선순위를 매기는 일(56.4%)이고, 둘의 합이 79.9%다(3절). 두 단계는 활성 목록의 길이에 비례해 돌고, 활성 목록의 92.7%가 자원 노드다(4절). 자원 노드 5,001개 가운데 모든 연결에서 Dormant 상태가 된 것은 114개뿐이다. 무리 밖의 자원 노드는 어느 연결에서도 채널이 열리지 않아 Dormant 상태가 되지 못한다는 것과 맞는다.
- **시간으로도 자원 노드의 몫이다.** 두 단계의 시간을 클래스별로 나누는 타이머는 없어서 자원 노드 수만 절반으로 줄여 쟀다(6절). 실행마다의 빠르기를 지운 값으로 두 단계가 51.2% 남았고, 활성 목록 길이에 비례할 때의 53.8%와 가깝다. 자원 노드의 몫은 두 단계의 94\~98%로 계산된다. 자원 노드는 채널이 없어서 연결마다 레벨 확인과 거리 검사를 거치고(`NetDriver.cpp:5580-5593`), 채널이 있는 NPC와 플레이어는 대신 우선순위 계산을 거친다. 이 차이가 하나의 비용을 크게 바꾸지는 않는 것으로 보인다. 다만 `act2-split2`의 변동 폭(`work_avg_ms` 4.189)이 커서, 정규화하지 않은 중앙값으로는 53.2% 줄었다는 것까지만 말할 수 있다.
- **실제로 보내는 일은 작다.** 액터를 직렬화해 보내는 `Process Prioritized Actors Time`이 16.5%, 소켓 송신까지 포함한 연결 틱이 2.1%다. 포스팅 8(액터를 Consider List에서 빼기)이 줄일 수 있는 상한은 두 단계의 79.9%다. 자원 노드를 모두 빼도 NPC와 플레이어 액터의 몫은 남는다.
- **`TickCompletionEvents`는 리플리케이션이 아니다.** 액터와 컴포넌트의 틱이다. NPC 이동이 32%, 물리가 37%(시작 29.0%와 끝 8.1%), 서버에서 도는 플레이어 캐릭터의 애니메이션이 14%다(5절). 데디케이티드 서버에서 메시 애니메이션을 줄이는 설정(`EVisibilityBasedAnimTickOption`, `SkinnedMeshComponent.h:96`)은 backlog.md "작업 중 떠오른 것"에 적었고 이 포스팅에는 넣지 않는다.

## 8. 확인하지 않은 것

- named events 트레이스에서 `WorldTick` 타이머가 프레임을 감싸지 않는 원인.
- Consider List의 프레임당 길이(활성 목록의 수만 셌다).
- 4절의 수는 측정이 끝난 순간 한 번 센 값이다. 측정 구간 동안 같았는지는 세지 않았다. 채널 수가 측정 구간 내내 77이라 크게 다르지 않았을 것으로 본다.
