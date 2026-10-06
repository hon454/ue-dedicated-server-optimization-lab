# 시리즈 웹 페이지: UE Dedicated Server, 단계별로 최적화해 보기

[index.html](index.html)은 의존성이 없는 한 파일이다. main에 푸시하면 [배포 워크플로](../.github/workflows/pages.yml)가 이 폴더를 GitHub Pages에 올린다. 주소는 `https://hon454.github.io/ue-dedicated-server-optimization-lab/`다.

페이지는 테스트베드 두 개를 고를 수 있다. 1막은 네 단계(0단계 기준선, 1단계 Relevancy, 2단계 Dormancy, 3단계 Net Update Frequency)이고, 2막은 일곱 단계(0단계 기준선부터 6단계 인벤토리 FastArray까지)다(2026-10-06에 더했다). 단계를 고르면 브라우저에서 도는 모형이 서버 프레임 하나의 횟수를 세어 보여 준다. 측정한 조합만 고를 수 있게 했다.

## 수치의 출처: 1막

| 페이지의 값 | 종류 | 출처 |
| --- | --- | --- |
| 단계마다의 서버 프레임 시간 평균과 "실제 서버에서 측정한 값"의 네 지표 | 측정값 | 각 포스팅의 측정 기록. 단계마다 그 글이 비교한 두 묶음을 쓴다: 1단계 `baseline3` → `relevancy2`, 2단계 `relevancy2` → `dormancy2`, 3단계 `dormancy6` → `update-frequency3`. 다른 날 잰 묶음끼리는 비교하지 않는다. 구별되지 않은 변화(2단계의 연결당 송신 대역폭, 3단계의 서버 프레임 시간 평균)는 그렇게 적는다. 클라이언트에 존재하는 자원 노드는 1번 클라이언트의 t=45s 화면 글자다 |
| "측정: 프레임당 N쌍" | 측정값 | `LabResourceNode`와 `LabNpc` 타이머의 Count ÷ `WorldTick` Count. 기준선 42,408(5,301 × 8), Relevancy 647\~821((75.4 + 5.5) × 8\~(95.6 + 7.0) × 8), Dormancy 약 44(5.50 × 8), Net Update Frequency 약 14(1.78 × 8). 각 포스팅의 측정 기록 |
| 서버 프레임 간격 198ms(기준선) | 측정값 | 기준선의 서버 프레임 시간 평균 198.43ms. 나머지 단계는 틱 예산 안이라 33.3ms(30Hz)로 둔다 |
| 맵 2km, 배치 영역 ±950m, 클라이언트 8, 자원 노드 5,001, NPC 300 | 소스의 값 | `Source/DSOptLab/LabScenarioConfig.h` |
| 플레이어 자리(반지름 500m 원의 16자리 중 0\~7번), 경로(한 변 100m 정사각형, 500cm/s) | 소스의 값 | `LabGameMode.cpp`의 `GetSlotLocation`, `LabPlayerController.h`의 `WaypointSide`, `LabCharacter.cpp`의 `MaxWalkSpeed` |
| NPC 300cm/s, 채집 주기 약 26초 | 소스의 값 | `LabNpc.h`의 `MoveSpeed`, `LabResourceNode.h`의 `RespawnSeconds` |
| Net Cull Distance 150m, Relevancy를 잃은 채널을 닫기까지 5초 | 엔진 소스 | `Actor.cpp:312`, `BaseEngine.ini:1864`의 `RelevantTimeout` |
| 다음 고려 시각 = 지금 + 난수(0\~1/30초) + 1 ÷ 빈도 | 엔진 소스 | `NetDriver.cpp:5420-5425`, `6341-6348` |
| 흐름의 순서(①고려할 시각, ②거리 검사, ③Dormant 상태, ④프로퍼티 비교) | 엔진 소스 | [기준선 글의 측정 기록](../Posts/01-baseline/measurements.md) 7절 |
| 자원 노드와 NPC의 위치, NPC의 배회 경로 | 예시 | 페이지의 고정 시드. 게임의 배치와 같지 않다 |
| 흐름의 횟수, 클라이언트의 액터 수, NPC의 갱신 간격 | 모형이 센 값 | 측정값이 아니다 |

## 모형과 측정의 대조: 1막

모형이 센 값이 측정값과 맞는지 확인했다(2026-10-03, 로컬 미리보기).

| 단계 | ④ 프로퍼티를 비교한 쌍(모형) | 측정 | NPC의 갱신 간격(모형) | 측정 |
| --- | --- | --- | --- | --- |
| 기준선 | 42,408 | 42,408 | 198ms | 읽지 않음 |
| Relevancy | 약 700 | 647\~821 | 약 46ms | 46.4ms(화면 위치가 바뀌는 간격, Dormancy 구성) |
| Dormancy | 약 43 | 약 44 | 약 42ms | 같음 |
| Net Update Frequency | 약 15 | 약 14 | 약 133ms | 130.6ms |

다른 값이 하나 있다. Dormancy 단계에서 걸어 다니는 클라이언트가 가진 자원 노드는 모형에서 약 190개에 머문다(한 변 100m 경로를 150m 원으로 쓸어 낸 넓이 약 0.14km² × 밀도). 측정값은 301\~313개다. 제자리에 서 있는 0번 클라이언트도 측정에서는 195개로, 150m 안의 80개보다 많다. 두 클라이언트 모두 약 110개가 더 있는 셈이다. 접속 직후 폰이 자리에 놓이기 전의 시점 위치 주변에서 자원 노드를 받아 Dormant 상태로 남긴 것으로 추정하고, 확인하지 않았다.

## 수치의 출처: 2막

| 페이지의 값 | 종류 | 출처 |
| --- | --- | --- |
| 기준선과 1\~3단계의 서버 프레임 시간 평균과 네 지표 | 측정값 | [1막의 세 최적화를 2막에 다시 적용](../Posts/06-three-techniques-again/README.md)이 비교한 `act2-baseline11`, `act2-relevancy1`, `act2-dormancy1`, `act2-update-frequency1`([누적 수치의 측정 기록](../Posts/measurements.md) "2막: 1막의 세 단계" 1절). 연결당 송신 대역폭은 Networking Insights `Connection 0`이다. 구별되지 않은 변화는 2단계의 연결당 송신 대역폭, 3단계의 서버 프레임 시간 평균과 리플리케이션 시간이다 |
| 4단계 | 측정값 | [포스팅 8 측정 기록](../Posts/08-node-update-frequency/measurements.md) 2절의 `r3`끼리(`act2-nodeuf-base1`, `act2-nodeuf1`). 연결당 송신 대역폭은 CSV 중앙값이고 구별되지 않았다. Consider List 3,887 → 413 |
| 5단계 | 측정값 | [포스팅 9 측정 기록](../Posts/09-inventory-owner-only/measurements.md) 2절의 `r2`끼리(`act2-invown-base1`, `act2-invown1`). 연결당 송신 대역폭과 인벤토리 송신량은 Networking Insights `Connection 0`이다. 서버 프레임 시간 평균과 리플리케이션 시간은 구별되지 않았다(CSV `netflush_avg_ms` -0.067, 변동 폭 0.121) |
| 6단계 | 측정값 | [포스팅 10 측정 기록](../Posts/10-inventory-fastarray/measurements.md) 2절과 [관찰 자료](../Posts/10-inventory-fastarray/candidates.md) 13절(`act2-fastarr-base1`, `act2-fastarr1`). 서버 프레임 시간 평균은 구별되지 않았다. 인벤토리 한 번의 최대 크기 18,190 → 345비트 |
| 인벤토리 카드의 "인벤토리 송신량"(4\~6단계) | 측정값 | 4단계는 5단계의 적용 전 값 4,545다(같은 구성). 0\~3단계는 측정하지 않았다 |
| "측정: 활성 목록, Consider List" | 측정값 | 서버 로그 `lab_network_objects active=`, `lab_consider_list avg_per_frame=`. 3단계는 `act2-nodeuf-base1-r3`(같은 구성을 포스팅 8에서 잰 값), 4\~6단계는 각 글의 적용 후 실행이다. 활성 목록에는 플레이어 캐릭터와 컨트롤러 같은 다른 액터가 들어 있다 |
| 서버 프레임 간격 214ms(기준선), 35.9ms(1단계) | 측정값 | 두 단계의 서버 프레임 시간 평균. 나머지 단계는 틱 예산 안이라 33.3ms로 둔다 |
| 플레이어 자리(대각선 위 3m 간격), 경로(한 변 70m 정사각형, 짝수 자리는 꼭짓점에서 정방향, 홀수 자리는 변의 가운데에서 반대 방향) | 소스의 값 | `LabGameMode.cpp`의 `GetSlotLocation`, `PlaceAndStart`, `LabPlayerController.h`의 `SpacedWaypointSide` |
| 무리 중심(경로 중심의 평균), 건축물 500개(중심에서 80m 안, 경로에서 6m 밖), 1초마다 하나를 새로 지음, 주변 NPC 50명(40m 안), NPC 배회(집에서 ±30m) | 소스의 값 | `LabGameMode.cpp`의 `BuildPlayerClusters`, `SpawnBuilding`, `RebuildOne`, `SpawnClusterNpcs`, `LabGameMode.h`, `LabNpc.h`의 `WanderRadius` |
| 인벤토리 200칸, 0.5초마다 한 사람의 맨 앞 칸을 지우고 맨 뒤에 더함 | 소스의 값 | `-InventoryItems 200 -InventoryChurn 4`, `LabGameMode.cpp`의 `ChurnOneInventory` |
| 자원 노드 빈도 2의 다음 고려 시각 | 엔진 소스 | 1막과 같은 식(`NetDriver.cpp:5420-5425`) |
| 모든 연결에서 Dormant 상태인 액터는 활성 목록에서 빠진다 | 엔진 소스 | `NetworkObjectList.cpp`의 `FNetworkObjectList::MarkDormant`([engine-notes.md](../Docs/Reference/engine-notes.md) 2절 "휴면 액터와 관련성") |
| 인벤토리의 아이템, 자원 노드와 NPC, 건축물의 위치 | 예시 | 페이지의 고정 시드 |

## 모형과 측정의 대조: 2막

모형이 센 값이 측정값과 맞는지 확인했다(2026-10-06, 로컬 미리보기, 단계마다 모형 시간 60초 뒤의 96프레임 평균).

| 단계 | 모형 | 측정 |
| --- | --- | --- |
| 3단계 Consider List | 약 3,860 | 3,887(`act2-nodeuf-base1-r3`) |
| 4단계 Consider List | 약 394 | 413(`act2-nodeuf1-r3`) |
| 3단계 활성 목록 | 약 5,250 | 5,272. 모형에 없는 플레이어 캐릭터와 컨트롤러 같은 액터가 있다 |
| 3단계 걸어 다니는 클라이언트의 자원 노드 | 178 | 176(포스팅 8의 화면 글자 `nodes=176`) |
| 1단계 걸어 다니는 클라이언트의 열린 액터 채널 | 약 666 | 695(CSV, 8개 연결 평균. 모형에 없는 액터가 있다) |

4단계에서 자원 노드는 16프레임마다 몰려서 고려된다. 단계를 바꾼 프레임에 모든 자원 노드의 다음 고려 시각이 함께 정해지기 때문이다. 흐름의 숫자는 그래서 96프레임(16의 배수)을 평균한다.

## 모형이 줄인 것

- 플레이어, 게임 상태 같은 다른 액터와 우선순위 정렬, 패킷 조립, 송신 한도를 다루지 않는다.
- 자원 노드는 채널이 열려 한 번 처리된 프레임에 바로 Dormant 상태가 된다. 엔진에서는 그 뒤에 채널이 닫힌다.
- 단계를 Dormancy 이전으로 되돌리면 채널 없이 남아 있던 자원 노드를 바로 지운다.
- 왕복하는 NPC 하나는 흐름의 횟수에 넣지 않는다.
- 2막: 인벤토리는 앞 칸 지우기만 다루고 채집으로 수량이 바뀌는 것은 다루지 않는다. FastArray 클라이언트의 칸 순서가 서버와 달라지는 것도 다루지 않는다. 건축물을 허물 때 클라이언트에서 바로 지운다.
- 1막은 2026-10-06 수정에서 모든 연결에서 Dormant 상태인 액터를 활성 목록에서 빼게 됐다. 1막의 플레이어는 서로 멀어 그런 자원 노드가 거의 없어서 흐름의 숫자는 그대로다(Dormancy 단계의 ④가 약 43 → 약 39).

## 로컬에서 보기

`Site/`를 정적 서버로 연다. `file://`로 열어도 동작한다.
