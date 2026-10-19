# 레거시 리플리케이션을 Replication Graph로 바꾸기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 실행별 CSV, Insights 값, 서버 로그, 품질 지표는 [관찰 자료](candidates.md) 9\~11절에 있다. 아래에서 `RG.cpp`는 `Engine/Plugins/Runtime/ReplicationGraph/Source/Private/ReplicationGraph.cpp`, `RG.h`는 같은 플러그인의 `Source/Public/ReplicationGraph.h`, `ND.cpp`는 `Engine/Source/Runtime/Engine/Private/NetDriver.cpp`다(언리얼 엔진 5.8.3).

## 1. 실행 조건

- 구성: [포스팅 12](../12-npc-move-netserialize/measurements.md)의 최종 구성(⑧)에 `-MotionLog`. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, 기법 `-InventoryOwnerOnly -InventoryFastArray -NpcInterpDelay 150 -NpcCompactMove`. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 두 묶음을 쟀다. 둘 다 본체 화면, 실행 중 PC 조작 없음, 모든 실행이 종료 코드 0이다. 묶음끼리는 비교하지 않는다.

| 묶음 | 라벨 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- | --- |
| 첫째(2026-10-19 16:22\~16:54) | `act2-rg-base-nodedef1` | 레거시, 빈도 기본값 | 없음 | `r1` |
| | `act2-rg-base1` | 레거시, 빈도 2(지금 구성) | `-NodeUpdateFrequency 2` | `r3` |
| | `act2-rg-nodedef1` | 그래프, 빈도 기본값 | `-ReplicationGraph` | `r3` |
| | `act2-rg1` | 그래프, 빈도 2 | `-ReplicationGraph -NodeUpdateFrequency 2` | `r1` |
| 둘째(2026-10-19 17:30\~17:53) | `act2-rg-trk-base1` | 함께 잰 지금 구성 | `-NodeUpdateFrequency 2` | `r3` |
| | `act2-rg-trk1` | 그래프 | `-ReplicationGraph -NodeUpdateFrequency 2` | `r3` |
| | `act2-rg-notrk1` | 그래프, trickle 끔 | `-ReplicationGraph -RepGraphNoTrickle -NodeUpdateFrequency 2` | `r3` |

- 빌드: 첫째 묶음은 커밋 `507e0e0`, 둘째 묶음은 커밋 `45af534`의 소스다. `45af534`는 `-RepGraphNoTrickle`을 더했고 그 인자가 없을 때의 동작은 같다.
- 서버 로그로 구성을 확인했다: 리플리케이션 모델 `Generic` 또는 `LabReplicationGraph`(`ND.cpp:1939`의 `InitBase` 줄), 그래프 실행의 `lab_repgraph_class`(자원 노드 간격 1 또는 16프레임, NPC 4프레임, 캐릭터 1프레임)와 `lab_repgraph ... trickle=`(1 또는 0).
- 서버 프레임 시간과 리플리케이션 시간은 `Scripts/export-insights.ps1`로 중앙값 실행의 트레이스에서 읽었다. 그래프 실행의 클래스 타이머는 `UReplicationGraph_ServerReplicateActors` 아래에 있다(`RG.cpp:1144`의 `SCOPED_NAMED_EVENT`). 이 글에서 스크립트가 그 아래도 적게 고쳤다.
- 연결당 송신 대역폭은 CSV `out_bytes_per_sec_per_conn`의 중앙값이다. 구별의 기준은 [measurement.md](../../Docs/Guides/measurement.md)대로 CSV 중앙값의 변화가 큰 쪽 변동 폭보다 큰지다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 연결당 송신 대역폭 +8.8%, 10,100 → 11,000바이트/초 | CSV 중앙값 10,073 → 10,962(변동 폭 14, 47), 10,962 ÷ 10,073 − 1 = +8.8% | 관찰 자료 9.1절(`act2-rg-base1`, `act2-rg1`) |
| 서버 프레임 시간 평균 8.94 → 7.71ms, -13.8% | Timing Insights 8.944(`act2-rg-base1-r3`) → 7.710(`act2-rg1-r1`), 7.710 ÷ 8.944 − 1 = -13.8% | 관찰 자료 9.2절 |
| 리플리케이션 시간 3.96 → 2.92ms, -26.1% | `GameNetDriver` 프레임당 Incl 3.958 → 2.923, 2.923 ÷ 3.958 − 1 = -26.1% | 관찰 자료 9.2절 |
| 1번 클라이언트에 존재하는 자원 노드 176 → 380개 | 화면 글자 `nodes=176`(`act2-rg-base1-r2-topdown-04`), `nodes=380`(`act2-rg1-r2-topdown-04`), t=75초 | 자동 스크린샷 |
| 개념도의 칸 100m, 150m, 칸 경계, 400m | `CellSize` 10000(`Source/DSOptLab/LabReplicationGraph.cpp`), `NetCullDistanceSquared` 225,000,000(`Engine/Source/Runtime/Engine/Private/Actor.cpp:312`). 칸 번호 = (위치 − `SpatialBias`) ÷ `CellSize`(`RG.cpp:5737-5741, 6268-6275`), `SpatialBias` = −`UE_OLD_WORLD_MAX` = −2,097,152cm(`Engine/Source/Runtime/Engine/Public/EngineDefines.h:37`). 2,097,152 = 209 × 10,000 + 7,152이므로 경계는 x, y = 28.48m + 100m × n. 시점 칸의 목록에는 칸을 150m 넓힌 100 + 2 × 150 = 400m 사각형 안의 액터가 든다(`RG.cpp:5688-5743`) | 엔진 소스, 계산, `Scripts/make-repgraph-cell.ps1` |
| 개념도의 시점, 점의 밀도 | 시점 (−5m, 60m)(`act2-rg-trk1-r2-topdown-04`의 화면 글자 `pos`), 밀도 5,001 ÷ 1,900² = 0.00139개/m²(`LabScenarioConfig.h`의 `WorldHalfExtent` 95,000cm). 점의 위치는 스크립트의 고정 시드로 뿌린 예시다 | 자동 스크린샷, 계산 |
| 모든 연결에서 Dormant 상태가 되어야 활성 목록에서 빠진다 | `FNetworkObjectList::MarkDormant`가 모든 연결에서 Dormant일 때만 옮긴다(`Engine/Source/Runtime/Engine/Private/NetworkObjectList.cpp:348-376`) | 엔진 소스, [포스팅 7](../07-net-driver-breakdown/measurements.md) |
| 16프레임에 한 번 | 빈도 2: 500\~533ms 뒤 | [포스팅 8 측정 기록](../08-node-update-frequency/measurements.md) 2절 |
| 문제의 표: ① 0.959ms, ② 0.845ms, ③ 1.87ms | `Consider Actors Time` 0.959, `Prioritize Actors Time` 0.845, `Process Prioritized Actors Time` 1.868(`act2-nodeuf-split1-r3`, 포스팅 8의 적용 후. ⑤\~⑧ 전의 구성이고 이 글의 묶음에서는 재지 않았다) | 포스팅 8 측정 기록 2절 |
| ①의 약 3분의 2는 건너뛰는 자원 노드 | 4,857.2 × 0.128µs = 0.622ms, 0.622 ÷ 0.959 = 64.8% | 포스팅 8 측정 기록 2절 |
| 레거시와 같은 함수가 보낸다 | 그래프의 `ReplicateSingleActor`가 `UActorChannel::ReplicateActor`를 부른다(`RG.cpp:2151, 2157`). 레거시도 같은 함수다(`ND.cpp:5830`) | 엔진 소스 |
| 레거시가 만드는 목록을 대신한다 | 드라이버가 있으면 `ServerReplicateActors`가 바로 넘기고 Consider List를 만들지 않는다(`ND.cpp:6295-6298`) | 엔진 소스 |
| 액터는 150m가 걸치는 모든 칸에 미리 들어간다, 시점이 있는 칸 하나의 목록만 모은다 | `GetCellInfoForActor`(`RG.cpp:5688-5743`), `GatherActorListsForConnection`(`RG.cpp:6238-6293`) | 엔진 소스 |
| 모은 목록은 150m 거리 검사를 거친다 | `ReplicateActorListsForConnections_Default`의 거리 검사(`RG.cpp:1559-1589`) | 엔진 소스 |
| Dormant 상태의 액터는 칸마다 연결별 목록에 있고, Dormant 상태가 되면 지운다 | `UReplicationGraphNode_DormancyNode`가 연결별 `UReplicationGraphNode_ConnectionDormancyNode`를 만들고(`RG.cpp:5016-5067`), gather 때 `bDormantOnConnection`인 액터를 지운다(`RG.cpp:4694-4716`) | 엔진 소스 |
| 예제는 움직이는 액터를 세 묶음으로 나눠 3프레임에 한 번 모은다 | `UReplicationGraphNode_ActorListFrequencyBuckets::FSettings::NumBuckets` 기본값 3(`RG.h:247`), 프레임마다 한 묶음만 돌려준다(`RG.cpp:3915-3920`). 엔진의 예제 그래프 `UBasicReplicationGraph`는 모든 액터를 `AddActor_Dormancy`로 넣는다(`BasicReplicationGraph.cpp:91-116`) | 엔진 소스 |
| 레거시는 다음 고려 시각에 0\~1프레임의 난수를 더한다 | `NextUpdateTime = TimeSeconds + RandDelay + NextUpdateDelta`, `RandDelay = FRand() * ServerTickTime`(`ND.cpp:5421-5425`). 그 시각을 넘긴 첫 프레임에 고려한다(`ND.cpp:5318-5319`) | 엔진 소스 |
| NPC 4프레임, 자원 노드 16프레임, 캐릭터 평균 1.3프레임 | 평균 간격 = (30 ÷ 빈도) + 1프레임. 빈도 10 → 4, 2 → 16, 100 → 1.3. 그래프는 `RoundToInt(30 ÷ 빈도 + 1)` → 4, 16, 1(서버 로그 `lab_repgraph_class`). 빈도 10이 4프레임인 것은 1막에서 실행으로 확인했다 | 계산, [engine-notes.md](../../Docs/Reference/engine-notes.md) 2절, 관찰 자료 7절 |
| 예상: 활성 목록 5,272개 | 서버 로그 `lab_network_objects active=5272`(`act2-rg-base1-r3`) | 서버 로그 |
| 예상: 한 연결이 모으는 액터 약 75개 | 칸 목록의 자원 노드 약 222개(400m × 400m × 0.00139)는 자리를 잡으면 Dormant 상태로 빠진다. NPC 약 63개(무리 주변 50 + 맵 300 × 160,000 ÷ 3,610,000), 캐릭터 8, 소유자 액터와 모든 연결의 액터 몇 개 | 관찰 자료 4절, 계산 |
| 예상: ①과 ②의 1.80ms | 0.959 + 0.845 = 1.804 | 계산 |
| 예상: 캐릭터를 보내는 횟수 약 1.3배 | 1.3프레임 → 1프레임, 1.3 ÷ 1 | 계산 |
| 결과 표: 서버 프레임 시간 평균 17.8, 8.94, 7.81, 7.71ms | Timing Insights 17.788(`act2-rg-base-nodedef1-r1`), 8.944(`act2-rg-base1-r3`), 7.809(`act2-rg-nodedef1-r3`), 7.710(`act2-rg1-r1`) | 관찰 자료 9.2절 |
| 결과 표: 리플리케이션 시간 12.7, 3.96, 2.95, 2.92ms | `GameNetDriver` 프레임당 Incl 12.681, 3.958, 2.946, 2.923(같은 실행) | 관찰 자료 9.2절 |
| 차트의 값 | 17.788, 8.944, 7.809, 7.710 | 관찰 자료 9.2절 |
| 빈도를 낮추는 방법 -49.7%, 그래프 -56.1% | 8.944 ÷ 17.788 − 1 = -49.7%, 7.809 ÷ 17.788 − 1 = -56.1% | 계산 |
| 그래프에서 빈도 2의 차이는 구별되지 않았다 | CSV `work_avg_ms` 중앙값 7.537 → 7.437, 차이 0.100이 변동 폭 0.332(빈도 기본값), 0.071(빈도 2)의 큰 쪽보다 작다 | 관찰 자료 9.1절 |
| 보낼지 따지는 일 2.45 → 1.14ms | 레거시: `GameNetDriver` 프레임당 Excl 2.448(`act2-rg-base1-r3`). 그래프: `GameNetDriver` Excl 0.362 + `UReplicationGraph_ServerReplicateActors` Excl 0.774 = 1.136(`act2-rg1-r1`). 두 값 모두 클래스 타이머 밖의 시간이다 | 관찰 자료 9.2절 |
| 실제로 보내는 일 1.51 → 1.79ms | 클래스 타이머와 RPC 타이머의 합. 레거시: `GameNetDriver` 바로 아래 줄의 합 1.511. 그래프: `UReplicationGraph_ServerReplicateActors` Incl 2.561 − Excl 0.774 = 1.787(바로 아래 줄의 합 1.786과 같다) | 관찰 자료 9.2절, `Saved/InsightsExport/act2-rg1-r1/summary.txt` |
| 캐릭터를 더 자주 보낸 몫 | `LabCharacter` 호출 프레임당 49.6 → 64.0(`act2-rg-trk-base1-r3` → `act2-rg-trk1-r3`는 49.4 → 64.0) | 관찰 자료 9.5절, 10.2절 |
| 한 연결이 보낸 캐릭터 데이터 3,080 → 3,880바이트/초 | Networking Insights `Connection 0`, `Outgoing`의 `BP_LabCharacter_C` 1,435,500비트 ÷ 58.339초 ÷ 8 = 3,075.8, 1,819,718비트 ÷ 58.584초 ÷ 8 = 3,882.7(`act2-rg-trk-base1-r3`, `act2-rg-trk1-r3`) | 관찰 자료 11절 |
| 늘어난 액터 데이터의 77% | `Actor` 9,224.0 → 10,266.4바이트/초(+1,042.4), 캐릭터 +806.9, 806.9 ÷ 1,042.4 = 77.4% | 관찰 자료 11절 |
| 나머지는 늘어난 캐릭터 Bunch의 헤더 | `BunchHeader` 2,219.3 → 2,486.9바이트/초(+267.6), Bunch 수 34,949 → 37,301. 캐릭터 Count 9,502 → 11,986 | 관찰 자료 11절 |
| 자원 노드가 보낸 데이터는 초당 2바이트로 같았다 | `LabResourceNode` 866비트 ÷ 58.339초 ÷ 8 = 1.86, 907 ÷ 58.584 ÷ 8 = 1.94바이트/초 | 관찰 자료 11절 |
| trickle: 176 → 380개, 끄면 176개 | 화면 글자 `nodes=`(`act2-rg-trk-base1-r2-topdown-04` 176, `act2-rg-trk1-r2-topdown-04` 380, `act2-rg-notrk1-r2-topdown-04` 176), t=75초. 서버 로그의 모든 연결에서 Dormant 상태인 자원 노드 114, 241, 114 | 자동 스크린샷, 관찰 자료 10.3절 |
| 한 프레임에 하나씩 거리와 상관없이 보낸다, 받은 액터는 목록에서 빠진다 | gather에서 지울 Dormant 액터가 없으면 남은 액터 하나에 `bForceCullDistanceToZero`를 켠다(`RG.cpp:4717-4721`). 컬 거리 0은 거리 검사를 하지 않는다(`RG.cpp:1571`). Dormant 상태가 되면 그 플래그를 끄고 목록에서 지운다(`RG.cpp:4702-4708`) | 엔진 소스 |
| 콘솔 변수 `Net.RepGraph.TrickleDistCullOnDormancyNodes` | 기본값 1(`RG.cpp:165`). 이 글은 `-LabRepGraphNoTrickle`일 때 그래프가 초기화하며 0으로 둔다 | 엔진 소스, `Source/DSOptLab/LabReplicationGraph.cpp` |
| trickle을 끄면 서버 프레임 시간 7.64 → 8.04ms | Timing Insights 7.640(`act2-rg-trk1-r3`) → 8.039(`act2-rg-notrk1-r3`). CSV `work_avg_ms` 중앙값 7.362 → 7.763, 차이 0.401이 변동 폭 0.341보다 크다. 그래프 자체 시간(`UReplicationGraph_ServerReplicateActors` Excl) 0.771 → 1.183 | 관찰 자료 10.1절, 10.2절 |
| 함께 잰 지금 구성 8.83ms | Timing Insights 8.832(`act2-rg-trk-base1-r3`) | 관찰 자료 10.2절 |
| trickle을 꺼도 연결당 송신 대역폭은 같았다 | CSV 중앙값 10,969 → 10,964, 차이 5가 변동 폭 21, 30보다 작다 | 관찰 자료 10.1절 |
| 그래프는 보낼지 확인하는 방법만 바꾸므로 보내는 액터와 횟수가 같으면 송신량도 같다 | [Unreal Engine - Replication Graph](https://hon454.github.io/posts/unreal-engine-replication-graph/) TL;DR의 "어느 연결에 무엇을 보낼지가 같다면 송신량도 같다"를 풀어 썼다. 그래프도 레거시와 같은 `UActorChannel::ReplicateActor`로 보낸다(`RG.cpp:2151, 2157`) | 외부 글, 엔진 소스 |
| NPC의 표시 위치 오차와 표시 지연은 네 구성에서 구별되지 않았다 | 표시 위치 오차 평균 45.29\~46.60cm(빈도 기본값 레거시 48.08\~48.73cm는 서버 프레임이 길어 표시 지연이 약 8ms 길다), 표시 지연 152\~155ms. NPC 수신 간격 평균 레거시 133.4\~133.7ms, 그래프 133.7\~134.5ms | 관찰 자료 9.3절, 10.5절 |
| 배운 것: 서버 프레임 시간 13.8% | 위 요약 표의 줄 | 관찰 자료 9.2절 |

## 3. 서버가 남긴 CSV

실행별 행은 [관찰 자료](candidates.md) 9.1절(첫째 묶음)과 10.1절(둘째 묶음)에 있다. 중앙값(변동 폭)은 아래와 같다.

| 묶음 | work_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- |
| `act2-rg-base-nodedef1` | 17.506(2.748) | 10,097(43) | 77 | 0.000 |
| `act2-rg-base1` | 8.662(0.082) | 10,073(14) | 77 | 0.000 |
| `act2-rg-nodedef1` | 7.537(0.332) | 10,954(16) | 77 | 0.000 |
| `act2-rg1` | 7.437(0.071) | 10,962(47) | 77 | 0.000 |
| `act2-rg-trk-base1` | 8.555(0.169) | 10,080(32) | 77 | 0.000 |
| `act2-rg-trk1` | 7.362(0.076) | 10,969(21) | 77 | 0.000 |
| `act2-rg-notrk1` | 7.763(0.341) | 10,964(30) | 77 | 0.000 |

- `act2-rg-base-nodedef1`은 변동 폭이 커서(2.748) 중앙값 실행 `r1`의 Insights 값을 썼다. 다른 묶음과의 차이는 그보다 훨씬 커서 결론은 같다.
- 그래프 묶음에서는 Consider List를 만들지 않으므로 서버 로그 `lab_consider_list`가 남지 않는다.

## 4. 시각 자료

- 개념도: `Scripts/make-repgraph-cell.ps1`이 만든 [images/repgraph-cell.svg](images/repgraph-cell.svg). 무엇이 실제 값이고 무엇이 예시인지는 스크립트 머리 주석에 있다.
- 내려다보기 화면: 둘째 묶음 `r2`의 1번 클라이언트 자동 스크린샷(t=75초)을 그대로 복사했다. [images/topdown-legacy.png](images/topdown-legacy.png)(`act2-rg-trk-base1-r2-topdown-04`), [images/topdown-graph.png](images/topdown-graph.png)(`act2-rg-trk1-r2-topdown-04`), [images/topdown-graph-notrickle.png](images/topdown-graph-notrickle.png)(`act2-rg-notrk1-r2-topdown-04`).
- Networking Insights 캡처: 둘째 묶음 `r3`, `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`. 패킷 약 1,830번부터 끝까지를 클릭과 Shift-클릭으로 골랐다(레거시 1,748패킷 58.339초, 그래프 1,746패킷 58.584초). 창을 최대화하고 `Scripts/capture-insights.ps1`로 찍었다(3862×2110).

![레거시의 Net Stats](images/insights-netstats-legacy.png)

![그래프의 Net Stats](images/insights-netstats-graph.png)

## 5. 확인하지 않은 것

- ⑧에서의 ①, ②의 크기. 문제의 표는 포스팅 8의 적용 후(④)에서 읽었다.
- 그래프의 따지는 일(1.14ms)의 내역. `-StatNamedEvents` 실행을 하지 않아 gather, 연결별 정렬, 움직이는 액터의 칸 옮기기를 나누지 않았다.
- 보내는 일이 늘어난 몫 가운데 캐릭터를 더 보낸 몫과 그래프가 액터마다 더 하는 일의 비율.
- Networking Insights의 패킷 범위는 측정 구간보다 약 50패킷 짧다. 초당 값으로 비교했다.
- 0번 연결의 송신량 증가(+10.9%)가 CSV 연결 평균(+8.8%)보다 큰 이유. 0번이 제자리에서 채집하는 연결이라서로 본다.
- 플레이어가 흩어진 배치(`-PlayerSpacing 300`)와 다른 칸 크기에서의 결과.
- 클라이언트에 남는 Dormant 자원 노드를 지우는 동작(`Net.RepGraph.DormantDynamicActorsDestruction` 기본값 0, `RG.cpp:125-127`).
