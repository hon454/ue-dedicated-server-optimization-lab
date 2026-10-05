# 포스팅 8 후보 기법 (액터를 Consider List에서 빼기)

포스팅 8의 기법을 고르기 위한 자료다. 기법은 사용자가 고른다. 사용자가 A(자원 노드의 Net Update Frequency를 2로 낮추기)를 골랐다(2026-10-06).

- 출발점: [포스팅 7 관찰 자료](../07-net-driver-breakdown/candidates.md)(`act2-split1-r3`, `act2-split2`, `act2-split-nodes2500-1`). 이 파일은 새로 측정하지 않았다. 수치는 포스팅 7의 값과 그 값으로 계산한 값이다.
- 엔진 소스는 5.8.3(`G:\Epic Games\UE_Source`)에서 읽었다. 경로는 `Engine/Source/Runtime/Engine/Private/` 기준이다(따로 적은 것 제외).
- 1\~2절은 사실과 계산이고, 3절의 "예상"은 계산이다. 4절 "에이전트 의견"만 해석이다.

## 1. 겨냥할 비용

| 항목 | 값 | 출처 |
| --- | --- | --- |
| `GameNetDriver` Incl | 12.787ms/프레임 | `act2-split1-r3` |
| ① `Consider Actors Time`(활성 목록 전체를 프레임에 한 번 돎) | 3.003ms, 23.5% | 같음 |
| ② `Prioritize Actors Time`(Consider List 전체를 연결마다 돎) | 7.208ms, 56.4% | 같음 |
| 활성 목록 / 그 가운데 자원 노드 | 5,272 / 4,887(92.7%) | 서버 로그 `lab_network_objects` |
| ①+② 가운데 자원 노드의 몫 | 94\~98% | 대조 실행의 비례 계산(관찰 자료 6절) |
| 자원 노드의 Net Update Frequency | 100(엔진 기본값) | `Actor.cpp:295`, `Source/DSOptLab/LabResourceNode.cpp`에 설정 없음 |

①과 ②는 비용이 붙는 목록이 다르다.

- ①은 **활성 목록** 전체를 돈다(`NetDriver.cpp:5315`). 다만 `NextUpdateTime`이 지나지 않은 액터는 루프 첫머리에서 건너뛴다(`5319-5323`). 건너뛴 액터에 드는 비용은 재지 않았다.
- ②는 **Consider List**만 돈다(`5528-5679`). Consider List에 들어가는 것은 ①에서 건너뛰지 않은 액터다(`5438`).
- 그래서 활성 목록을 줄이는 후보는 ①과 ②를 함께 줄이고, Consider List만 줄이는 후보는 ②를 주로 줄인다.

지금 자원 노드가 Consider List에 들어가는 간격: 빈도 100은 다음 고려 시각이 10\~43.3ms 뒤라 평균 1.3프레임에 한 번이다([engine-notes.md](../../Docs/Reference/engine-notes.md) "업데이트 빈도의 스케줄링", `dormancy6`, `update-frequency3`에서 확인). 4,887 ÷ 1.3 = 약 3,760개가 프레임마다 Consider List에 든다(계산. Consider List 길이는 세지 않았다).

## 2. 후보

### A. 자원 노드의 Net Update Frequency를 2로 낮추기

backlog.md "작업 중 떠오른 것"의 "자원 노드의 `NetUpdateFrequency` 낮추기"다.

- **원리**: 다음 고려 시각은 지금 + 난수 지연(0\~한 프레임) + 1 ÷ Net Update Frequency다(`NetDriver.cpp:5420-5425`). 2로 낮추면 0.5초 뒤라, 30Hz에서 16프레임에 한 번 Consider List에 든다(0.5 ÷ 0.0333 = 15프레임, 그다음 프레임). 활성 목록에서는 빠지지 않는다.
- **구현**: `ALabResourceNode` 생성자에서 `SetNetUpdateFrequency(2.f)`. 전후 비교용 인자를 `-NpcUpdateFrequency`처럼 하나 더한다(예: `-NodeUpdateFrequency 100`). 채집으로 바뀐 상태가 최대 0.5초 늦지 않도록 `Harvest()`, `Respawn()`의 `FlushNetDormancy()`를 `ForceNetUpdate()`로 바꾼다. `ForceNetUpdate()`는 다음 고려 시각을 지금 - 0.01초로 당기고(`NetDriver.cpp:4885-4887`) Dormant 상태면 `FlushNetDormancy()`도 부른다(`Actor.cpp:3013-3031`). 수십 줄이다.
- **값 2의 근거**: 엔진의 `MinNetUpdateFrequency` 기본값이 2다(`Actor.cpp:296`). Adaptive Net Update Frequency가 오래 보내지 않은 액터를 낮추는 하한과 같다(`NetDriver.cpp:5393-5408`).
- **대가**: 플레이어가 다가와 150m 안에 들어온 자원 노드가 처음 보이기까지 최대 약 0.53초(16프레임) 늦다. 달리기 속도 약 9.8m/s(STATUS.md "명령", 28초에 약 275m)로 약 5m다. 가까이 있는 자원 노드의 상태 변화는 위 `ForceNetUpdate()`로 늦지 않는다.
- **바꾸지 않는 것**: 활성 목록(①의 루프 길이), 자원 노드별 Relevancy, 1막 구성의 인자(`-AlwaysRelevant`, `-NoNodeDormancy`), 클라이언트에 Dormant 상태로 남는 자원 노드.

### B. 구역 매니저 + FastArray

2막 설계 6절 표의 원래 후보다. 자원 노드 하나에 액터 하나 대신, 맵을 구역으로 나눠 구역마다 액터 하나가 자원 노드 목록을 FastArray로 리플리케이트한다.

- **원리**: 활성 목록의 자원 노드 4,887개가 구역 수로 바뀐다. 맵은 1,900m × 1,900m(`LabScenarioConfig.h`의 `WorldHalfExtent` 95,000cm)라 150m 구역이면 13 × 13 = 169개, 구역 하나에 자원 노드 약 30개다(계산).
- **구현**: 구역 액터와 FastArray 항목 구조체, 클라이언트 표시(인스턴스 메시 등 C++로), 채집 경로(`LabPlayerController.cpp:180-205`가 `TActorIterator<ALabResourceNode>`로 가장 가까운 자원 노드를 찾는다), 화면 글자의 자원 노드 수(`LabHUD.cpp:57`), 서버 로그, 1막 구성을 재현하는 전환 인자. 수백 줄이고, 화면 확인이 새로 필요하다.
- **대가**: 자원 노드별 Relevancy를 잃는다. 구역 하나가 통째로 관련성을 얻거나 잃으므로, 구역의 Net Cull Distance를 구역 크기만큼 늘려야 하고 그만큼 먼 자원 노드까지 받는다. 구역에 들어설 때 약 30개를 한 번에 받는다.
- **겹치는 것**: FastArray는 포스팅 9(인벤토리와 FastArray)의 주제다. 포스팅 8에서 먼저 쓰면 포스팅 9의 설명 일부가 앞당겨진다.

### C. Replication Graph의 공간 격자

`UReplicationGraphNode_GridSpatialization2D`(`Engine/Plugins/Runtime/ReplicationGraph/Source/Public/ReplicationGraph.h:579`)에 자원 노드를 Dormant 정적 액터로 넣는다(`AddActor_Dormancy`, `600`). 연결마다 시점 주변의 격자 칸만 모으므로 전체 활성 목록을 돌지 않는다.

- **구현**: 플러그인 활성화, `UReplicationGraph` 하위 클래스, `DefaultEngine.ini`의 `ReplicationDriverClassName`(`NetDriver.h:849`), 이 프로젝트의 모든 리플리케이트 클래스(자원 노드, NPC, 건축물, 플레이어 캐릭터, 컨트롤러, PlayerState, GameplayDebugger)의 경로 지정. 가장 크다.
- **대가**: 자원 노드만이 아니라 모든 액터의 리플리케이션 경로가 바뀐다. "포스팅 하나에 기법 하나"에서 효과가 어디서 났는지 가리기 어렵다.
- **겹치는 것**: 포스팅 13이 Replication Graph로 "포스팅 8과 같은 문제를 다른 구조로 풀어 비교"하는 글이다(backlog.md "우선순위 순" 9번). 포스팅 8에서 쓰면 포스팅 13의 목적을 새로 정해야 한다.

### 고르지 않는 쪽으로 본 것

- **Adaptive Net Update Frequency 켜기**(`net.UseAdaptiveNetUpdateFrequency` 1, 기본값 0, `NetDriver.cpp:523-526`). 2초 넘게 보내지 않은 액터를 5초에 걸쳐 `MinNetUpdateFrequency`(2)까지 낮춘다(`5393-5408`). 멀리 있는 자원 노드는 결국 A와 같아진다. 그러나 전역 설정이라 NPC, 플레이어 액터, 건축물에도 걸린다. 또 backlog.md "NPC가 멈춰 있는 구간"이 이 기능의 재료로 남겨 둔 것이다. A가 같은 효과를 자원 노드에만 준다.
- **맵에 놓은 `DORM_Initial` 액터**. 맵에서 불러온 `DORM_Initial` 액터는 ①에서 바로 활성 목록에서 빠진다(`IsDormInitialStartupActor`, `NetDriver.cpp:8557-8560`, `Actor.cpp:739-743`). 자원 노드를 맵에 놓아야 해서 규칙 "에디터 작업을 만들지 않는다"(자원 노드는 실행 시 코드로 생성한다)와 맞지 않는다.
- **서버가 플레이어에게서 먼 자원 노드의 리플리케이션을 끄고 켜기**(`SetReplicates`). 레거시 경로에서 `SetReplicates(false)`는 Iris에만 알리고(`Actor.cpp:4593-4596`), 활성 목록에서는 ①이 원격 역할이 없는 액터를 빼며 지운다(`NetDriver.cpp:5337-5342`). 이미 열린 채널과 클라이언트의 사본이 어떻게 되는지 확인하지 않았다. 결국 손으로 만든 공간 필터라 C와 겹친다.

## 3. 예상 (계산)

②는 Consider List의 길이에 비례하고, ②의 94\~98%가 자원 노드의 몫이라고 둔다(관찰 자료 6절). ①이 건너뛴 액터에 드는 비용은 모른다.

| 후보 | 활성 목록의 자원 노드 | 프레임마다 Consider List에 드는 자원 노드 | ②(지금 7.208ms) | ① |
| --- | ---: | ---: | --- | --- |
| 지금 | 4,887 | 약 3,760(1.3프레임에 한 번) | 7.21ms | 3.00ms |
| A(빈도 2) | 4,887 | 약 305(16프레임에 한 번) | 약 0.85ms | 건너뛰는 비용만큼 남음. 재 봐야 안다 |
| B(150m 구역) | 구역 169 | 구역 약 130 | 약 0.53ms | 약 0.32ms + 구역 비교 비용 |
| C | 격자 안에서 따로 관리 | 연결마다 시점 주변 칸만 | 측정 구조가 바뀌어 같은 타이머로 비교하기 어렵다 | 같음 |

- A의 ②: 7.208 × 0.96 = 6.92가 자원 노드의 몫, 6.92 × 1.3 ÷ 16 = 0.56, 나머지 7.208 − 6.92 = 0.29를 더해 약 0.85ms.
- B의 ①과 ②: 자원 노드의 몫이 169 ÷ 4,887 = 3.5%로 준다고 보고 6.92 × 0.035 + 0.29 = 약 0.53ms, ①은 3.003 × (385 + 169) ÷ 5,272 = 약 0.32ms에 구역 비교 비용을 더한 값. FastArray 비교 비용은 넣지 않았다.
- 서버 프레임 시간으로는 따지는 두 단계의 대부분(최대 약 9ms)이 줄어드는 범위다. `act2-split-nodes2500-1`에서 자원 노드 절반을 빼자 서버 프레임 시간이 32.0% 짧아졌다.

## 4. 에이전트 의견

**A(자원 노드의 Net Update Frequency를 2로 낮추기)를 추천한다.** 포스팅 7이 찾은 비용의 가장 큰 몫(②, 56.4%)을 수십 줄로 거의 지울 수 있고, 자원 노드별 Relevancy와 1막 구성의 인자를 그대로 둔다. 포스팅 9(FastArray)와 13(Replication Graph)의 주제를 앞당기지 않는다.

A를 고를 때 잃는 것:

- 활성 목록은 그대로라 ①이 남는다. 글의 제목은 "Consider List에서 빼기"와 맞지만 "활성 목록에서 빼기"는 아니다. ①이 얼마나 남는지가 이 글의 결과에서 새로 알게 될 것이다.
- 포스팅 4와 같은 손잡이(Net Update Frequency)다. 글의 새로움은 "Dormant 상태가 되지 못한 액터를 빈도로 거른다"는 쪽에 있다.
- 클라이언트에 Dormant 상태로 남는 자원 노드 문제(backlog.md)는 풀지 않는다.

B를 고르면 활성 목록 자체가 줄어 ①도 함께 줄고, 클라이언트에 남는 자원 노드도 구역 단위로 정리할 길이 생긴다. 대신 구현과 화면 확인이 가장 많이 늘고, 자원 노드별 Relevancy를 잃으며, 포스팅 9의 FastArray를 앞당긴다. C는 포스팅 13과 겹치고 모든 액터의 경로를 바꿔서 포스팅 8에는 추천하지 않는다.

## 5. 확인하지 않은 것

- ①에서 `NextUpdateTime`으로 건너뛴 액터 하나의 비용. A의 결과가 이것을 보여 준다.
- Consider List의 실제 길이(활성 목록의 수만 셌다). 1.3프레임은 1막에서 잰 간격이다.
- A에서 처음 보이기까지의 지연을 화면에서 알아볼 수 있는지.
- B에서 구역 크기를 어떻게 정할지, FastArray 비교 비용.
