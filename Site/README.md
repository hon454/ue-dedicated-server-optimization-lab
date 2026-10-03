# 시리즈 웹 페이지: UE Dedicated Server, 단계별로 최적화해 보기

[index.html](index.html)은 의존성이 없는 한 파일이다. main에 푸시하면 [배포 워크플로](../.github/workflows/pages.yml)가 이 폴더를 GitHub Pages에 올린다. 주소는 `https://hon454.github.io/ue-dedicated-server-optimization-lab/`다.

페이지는 네 단계(0단계 기준선, 1단계 Relevancy, 2단계 Dormancy, 3단계 Net Update Frequency)를 차례로 고르면 브라우저에서 도는 모형이 서버 프레임 하나의 횟수를 세어 보여 준다. 측정한 조합만 고를 수 있게 했다.

## 수치의 출처

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

## 모형과 측정의 대조

모형이 센 값이 측정값과 맞는지 확인했다(2026-10-03, 로컬 미리보기).

| 단계 | ④ 프로퍼티를 비교한 쌍(모형) | 측정 | NPC의 갱신 간격(모형) | 측정 |
| --- | --- | --- | --- | --- |
| 기준선 | 42,408 | 42,408 | 198ms | 읽지 않음 |
| Relevancy | 약 700 | 647\~821 | 약 46ms | 46.4ms(화면 위치가 바뀌는 간격, Dormancy 구성) |
| Dormancy | 약 43 | 약 44 | 약 42ms | 같음 |
| Net Update Frequency | 약 15 | 약 14 | 약 133ms | 130.6ms |

다른 값이 하나 있다. Dormancy 단계에서 걸어 다니는 클라이언트가 가진 자원 노드는 모형에서 약 190개에 머문다(한 변 100m 경로를 150m 원으로 쓸어 낸 넓이 약 0.14km² × 밀도). 측정값은 301\~313개다. 제자리에 서 있는 0번 클라이언트도 측정에서는 195개로, 150m 안의 80개보다 많다. 두 클라이언트 모두 약 110개가 더 있는 셈이다. 접속 직후 폰이 자리에 놓이기 전의 시점 위치 주변에서 자원 노드를 받아 Dormant 상태로 남긴 것으로 추정하고, 확인하지 않았다.

## 모형이 줄인 것

- 플레이어, 게임 상태 같은 다른 액터와 우선순위 정렬, 패킷 조립, 송신 한도를 다루지 않는다.
- 자원 노드는 채널이 열려 한 번 처리된 프레임에 바로 Dormant 상태가 된다. 엔진에서는 그 뒤에 채널이 닫힌다.
- 단계를 Dormancy 이전으로 되돌리면 채널 없이 남아 있던 자원 노드를 바로 지운다.
- 왕복하는 NPC 하나는 흐름의 횟수에 넣지 않는다.

## 로컬에서 보기

`Site/`를 정적 서버로 연다. `file://`로 열어도 동작한다.
