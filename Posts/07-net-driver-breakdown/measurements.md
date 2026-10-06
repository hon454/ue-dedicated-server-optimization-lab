# 네트워크 드라이버 자체 시간 나누기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값과 실행별 CSV는 [관찰 자료](candidates.md)에 있다.

## 1. 실행 조건

- 구성: [포스팅 6](../06-three-techniques-again/measurements.md)의 최종 구성(`config` 열 `default`). 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`. 클라이언트 8, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 네 묶음을 세 번씩 쟀다. 모두 2026-10-05, 본체 화면, 실행 중 PC 조작 없음. 24회 모두 종료 코드 0이고 선호도 재설정이 없었다.

| 묶음 | 본문의 이름 | 더한 인자 | 시각 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- | --- |
| `act2-split-base1` | 기본 트레이스 | 없음 | 22:16\~22:24 | `r2` |
| `act2-split1` | 타이머를 더 켠 실행 | `-StatNamedEvents` | 22:24\~22:31 | `r3` |
| `act2-split2` | 대조 실행의 기준 | `-StatNamedEvents` | 23:26\~23:34 | `r2` |
| `act2-split-nodes2500-1` | 자원 노드 절반 | `-StatNamedEvents -Nodes 2500` | 23:34\~23:42 | `r3` |

- 빌드는 커밋 `3a82d7b`의 소스다. 이 커밋이 서버 로그 `lab_network_objects`를 더했다. 측정 구간이 끝나고 CSV 행을 쓴 뒤 한 번 세므로 측정된 프레임에는 들지 않는다.
- `-StatNamedEvents`는 서버에 `-statnamedevents`를 넘긴다(`Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp:1759-1762`). 이 트레이스에서 달라지는 것(클래스 타이머의 부모, `WorldTick`)은 [engine-notes.md](../../Docs/Reference/engine-notes.md) 차절에 있다. 프레임당 값은 이 트레이스에서 `GameNetDriver` Count로 나눴고, Count가 CSV `frames`와 같았다.
- 프레임당 값은 Incl ÷ 프레임 수다. 읽은 명령은 `Scripts/export-insights.ps1`이다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 리플리케이션 시간 12.8ms | `GameNetDriver` 프레임당 Incl 12.755ms(`act2-split-base1-r2`) | 관찰 자료 2절 |
| 그 86%는 내역을 알 수 없는 시간, 자체 시간 10.9ms(86%) | `GameNetDriver` 프레임당 Excl 10.930ms, 10.930 ÷ 12.755 = 85.7% | 관찰 자료 2절 |
| 클래스 타이머의 합 1.83ms | 12.755 − 10.930 = 1.825ms | 관찰 자료 2절 |
| 남은 리플리케이션 시간의 80%는 따지는 일, ①과 ②를 합하면 79.9% | (7.208 + 3.003) ÷ 12.787 = 79.9%(`act2-split1-r3`) | 관찰 자료 3절 |
| 서버가 따지는 액터의 93%는 자원 노드 | 활성 목록 5,272개 가운데 `LabResourceNode` 4,887개, 92.7% | 관찰 자료 4절(서버 로그 `lab_network_objects`, `act2-split1-r3`) |
| 우선순위 목록 만들기 7.21ms, 56.4% | `Prioritize Actors Time` 7.208ms, 7.208 ÷ 12.787 = 56.4% | 관찰 자료 3절 |
| Consider List 만들기 3.00ms, 23.5% | `Consider Actors Time` 3.003ms, 23.5% | 관찰 자료 3절 |
| 액터 처리 2.11ms, 16.5% | `Process Prioritized Actors Time` Incl 2.114ms, 16.5% | 관찰 자료 3절 |
| 연결 틱과 그 밖 0.462ms, 3.6% | 12.787 − 7.208 − 3.003 − 2.114 = 0.462ms. `NetConnection Tick` 0.269, `ServerReplicateActors Time` Excl 0.093, `GameNetDriver` Excl 0.043과 0.5% 미만의 타이머 | 관찰 자료 3절 |
| 이 클라이언트에는 자원 노드 176개 | 화면 글자 `nodes=176`(`act2-split-base1-r2-topdown-03`, t=60초) | 자동 스크린샷 |
| 서버는 자원 노드 4,887개를 프레임마다 따진다 | 활성 목록의 `LabResourceNode` 4,887개, Net Update Frequency 100 | 관찰 자료 4절, 아래 "고려 간격" 줄 |
| 고려 간격 0.01초, 프레임 간격 0.033초 | `ALabResourceNode`는 `NetUpdateFrequency`를 바꾸지 않아 엔진 기본값 100(`Engine/Source/Runtime/Engine/Private/Actor.cpp:295`), 1 ÷ 100 = 0.01초. `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`), 1 ÷ 30 = 0.0333초 | 엔진 소스, `Source/DSOptLab/LabResourceNode.cpp` |
| 자원 노드가 프레임마다 Consider List에 들어간다 | `NextUpdateTime`이 지난 액터만 고른다(`NetDriver.cpp:5319`). 다음 시각은 지금 + 임의 지연 + 1 ÷ Net Update Frequency(`NetDriver.cpp:5420-5425`). Adaptive Net Update Frequency는 기본값이 꺼짐이라(`net.UseAdaptiveNetUpdateFrequency` 0, `NetDriver.cpp:523-526`) 간격이 늘지 않는다 | 엔진 소스 |
| ①은 활성 목록 전체를 한 번 돈다 | `ServerReplicateActors_BuildConsiderList`(`Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5303-5455`), 프레임당 Count 1.0 | 엔진 소스, 관찰 자료 3절 |
| ②는 Consider List 전체를 연결마다 돈다. 거리 검사, Dormant 상태 확인, 우선순위 정렬 | `ServerReplicateActors_PrioritizeActors`(`NetDriver.cpp:5528-5679`). 채널이 없는 액터의 거리 검사 `5580-5593`, Dormant 상태 확인 `5618-5624`, 정렬 `5669`. 프레임당 Count 8.0 | 엔진 소스, 관찰 자료 3절 |
| 모든 연결에서 Dormant 상태여야 활성 목록에서 빠진다 | `FNetworkObjectList::MarkDormant`(`Engine/Source/Runtime/Engine/Private/NetworkObjectList.cpp:348-376`) | 엔진 소스, [engine-notes.md](../../Docs/Reference/engine-notes.md) "휴면 액터와 관련성" |
| 한 번 보낸 액터만 그 연결에서 Dormant 상태가 된다 | 채널이 없으면 Dormant 상태로 들어가지 않는다(`ShouldActorGoDormant`의 `!Channel`, `NetDriver.cpp:5506-5526`). 연결별 등록은 액터 채널에서만 불린다(`DataChannel.cpp:2354, 2461, 2728`) | 엔진 소스 |
| 150m | `NetCullDistanceSquared` 225,000,000(`Actor.cpp:312`), √225,000,000 = 15,000cm | 엔진 소스 |
| 무리 곁 8개/8개, 먼 곳 0개/0개 | 두 경우를 그린 예시. 실제 수는 모든 연결에서 Dormant 상태 114개, 일부 연결에서만 76개, 나머지 4,811개 | 관찰 자료 4절 |
| 예상: 5,272개 → 2,835개, 54% | 활성 목록 5,272(`act2-split2`), 2,835(`act2-split-nodes2500-1`), 2,835 ÷ 5,272 = 53.8% | 관찰 자료 6절 |
| 서버 프레임 시간을 2.0% 늘렸다 | CSV `work_avg_ms` 중앙값 17.358(`act2-split-base1`) → 17.703(`act2-split1`), +1.99% | 관찰 자료 1절 |
| 단계에 속하지 않고 남은 자체 시간 0.3% | `GameNetDriver` Excl 0.043 ÷ 12.787 = 0.34% | 관찰 자료 3절 |
| 소켓 송신 1.2% | `IpConnection Socket SendTo` 0.154 ÷ 12.787 = 1.2% | 관찰 자료 3절 |
| 연결 8개를 합하면 프레임마다 39,096번, 4만 번 | 4,887 × 8 = 39,096. 활성 목록의 자원 노드 수와 연결 수로 계산한 값이고 검사 횟수를 직접 세지는 않았다 | 계산 |
| 활성 목록: 자원 노드 4,887, NPC 350, 그 밖 35, 합계 5,272 | `LabNpc` 350, `BP_LabCharacter_C`, `LabPlayerController`, `PlayerState`, `GameplayDebuggerCategoryReplicator` 각 8, `WorldSettings`, `GameStateBase`, `LabBuilding` 각 1 | 관찰 자료 4절 |
| 자원 노드 5,001개 가운데 114개, 건축물 500개 가운데 499개 | `LabResourceNode` 4,887 / 76 / 114, `LabBuilding` 1 / 0 / 499 | 관찰 자료 4절 |
| 자원 노드를 5,001개에서 2,501개로 | `-Nodes 2500`과 검증용 1개(CSV `nodes` 2501) | 관찰 자료 6절 |
| 남은 비율: 활성 목록 53.8%, 예상 53.8%, 실제 51.2% | 실제는 (①+②) ÷ ③의 중앙값 2.558(`act2-split-nodes2500-1`) ÷ 4.996(`act2-split2`) = 51.2% | 관찰 자료 6절 |
| 실행마다 서버의 빠르기가 달랐다 | `act2-split2`의 `work_avg_ms` 14.853, 18.472, 19.042. `r1`은 처리 횟수가 같고 모든 타이머가 빠르다(`Process Prioritized Actors Time` 1.820, `r2` 2.145) | 관찰 자료 6절 |
| 따지는 시간의 94\~98%가 자원 노드의 몫 | (4.996 − 2.558) × 4,887 ÷ 2,437 ÷ 4.996 = 98%. `act2-split1-r3`의 4.830을 쓰면 94% | 관찰 자료 6절 |
| 서버 프레임 시간이 32.0% 짧았다 | Timing Insights 평균 18.771ms(`act2-split2-r2`) → 12.765ms(`act2-split-nodes2500-1-r3`), 12.765 ÷ 18.771 − 1 = -32.0% | 관찰 자료 6절 |
| 줄인 시간은 거의 모두 따지는 일에서 | 서버 프레임 시간 -6.006ms 가운데 ①+② -5.739ms(10.780 → 5.041), 95.6% | 관찰 자료 6절 |
| `TickCompletionEvents` 2.38ms | 4.248398초 ÷ 1,788프레임 = 2.376ms(`act2-split-base1-r2`, 기본 트레이스) | 관찰 자료 5절 |
| NPC 이동과 물리가 69% | NPC의 액터 틱 32.2% + 물리 29.0% + 8.1% = 69.3%(`act2-split1-r3`) | 4절 |
| 리플리케이션 시간의 절반을 넘는다 | ② 56.4% | 관찰 자료 3절 |

## 3. 서버가 남긴 CSV

네 묶음의 실행별 CSV와 중앙값, 변동 폭은 [관찰 자료](candidates.md) 1절(`act2-split-base1`, `act2-split1`)과 6절(`act2-split2`, `act2-split-nodes2500-1`)에 있다.

| 묶음 | `work_avg_ms` 중앙값 | 변동 폭 | `open_actor_channels_per_conn` |
| --- | ---: | ---: | ---: |
| `act2-split-base1` | 17.358 | 0.155 | 77 |
| `act2-split1` | 17.703 | 0.229 | 77 |
| `act2-split2` | 18.472 | 4.189 | 77 |
| `act2-split-nodes2500-1` | 12.479 | 0.357 | 78 |

`act2-split2`는 두 시간 앞의 `act2-split1`과 구성이 같다. 몇 시간 떨어진 묶음이라 두 묶음의 값은 비교하지 않았다. 대조는 연달아 잰 `act2-split2`와 `act2-split-nodes2500-1`끼리 했다.

## 4. `TickCompletionEvents`의 내역

본문은 두 줄로 줄였다. `act2-split1-r3`, 프레임당이고, 비율은 `TickCompletionEvents` Incl 2.575ms에 대한 값이다(타이머를 더 켠 실행의 값이라 비율로만 읽는다).

| 타이머 | 프레임당 Count | ms | 비율 |
| --- | ---: | ---: | ---: |
| NPC의 액터 틱(`LabNpc`) | 350 | 0.830 | 32.2% |
| 물리 시작(`FStartPhysicsTickFunction_ExecuteTick`) | 1.0 | 0.747 | 29.0% |
| 물리 끝(`FEndPhysicsTickFunction_ExecuteTick`) | 1.0 | 0.209 | 8.1% |
| 플레이어 캐릭터의 메시(`CharacterMesh0`, 애니메이션) | 8.0 | 0.371 | 14.4% |
| 그 밖 | | | 16.3% |

기본 트레이스에서는 `ProcessUntilTasksComplete` Excl이 Incl의 39.0%라 이 내역이 보이지 않는다(`act2-split-base1-r2`).

## 5. 시각 자료

- 요약의 이미지는 `act2-split-base1-r2-topdown-03.png`(t=60초)를 `images/topdown.png`로 복사했다. 기본 트레이스 묶음의 중앙값 실행이다.
- 원리의 흐름도는 Mermaid로 그렸다. 단계 이름은 stat 타이머(`Consider Actors Time`, `Prioritize Actors Time`, `Process Prioritized Actors Time`)에 대응한다.
- 결과의 차트 값은 `act2-split1-r3`의 프레임당 ms다.
- Insights 캡처는 `act2-split1-r3`의 Timing Insights다. Log View에서 `Lab_MeasureStart`와 `Lab_MeasureEnd`를 골라 구간을 정하고(이 트레이스에서는 `WorldTick`이 프레임을 감싸지 않는다. engine-notes.md 9절), Timers에서 `GameNetDriver`를 골라 Callees 패널을 띄웠다. 구간의 `GameNetDriver` Count 1,787은 CSV `frames`와 같고, Incl 22.84초 ÷ 1,787 = 12.78ms/프레임이 내보낸 요약의 12.787ms와 같다. 번호 상자는 원리의 흐름도와 같은 번호다: ① `Consider Actors Time`(5.36초, % Root 23.48%), ② `Prioritize Actors Time`(12.88초, 56.37%), ③ `Process Prioritized Actors Time`(3.78초, 16.54%). 2026-10-06에 찍었다.

![act2-split1-r3의 GameNetDriver Callees. ① Consider Actors Time, ② Prioritize Actors Time, ③ Process Prioritized Actors Time](images/insights-r3-callees-crop.png)

![같은 화면의 창 전체. 아래 Log View에서 두 북마크를 골라 측정 구간을 정했다](images/insights-r3-callees.png)

## 6. 확인하지 않은 것

- 타이머를 더 켠 트레이스에서 `WorldTick` 타이머가 프레임을 감싸지 않는 원인.
- Consider List의 프레임당 길이. 활성 목록의 수만 셌다.
- 활성 목록의 수는 측정이 끝난 순간 한 번 센 값이다. 채널 수가 측정 구간 내내 77이라 크게 다르지 않았을 것으로 본다.
- ①과 ②의 시간을 클래스별로 나누는 타이머는 없다. 자원 노드의 몫은 대조 실행의 비례로 계산했다.
- 맵 전체의 NPC는 자원 노드와 같은 난수에서 위치를 뽑는다(`Source/DSOptLab/LabGameMode.cpp:82-122`). 그래서 대조 실행은 NPC 배치도 다르다. 열린 채널(77, 78)과 `LabNpc` 처리 횟수(프레임당 114.9, 116.0)는 비슷했다.
