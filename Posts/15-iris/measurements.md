# 레거시 리플리케이션을 Iris로 바꾸기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 실행별 CSV, Insights 값, 서버 로그, 품질 지표는 [관찰 자료](candidates.md) 11\~13절에 있다. 아래에서 `ND.cpp`는 `Engine/Source/Runtime/Engine/Private/NetDriver.cpp`, `ERB.cpp`는 `Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/EngineReplicationBridge.cpp`, `ORB.cpp`는 `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ObjectReplicationBridge.cpp`, `RS.cpp`는 같은 폴더의 `ReplicationSystem.cpp`, `RSDB.cpp`는 `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationState/ReplicationStateDescriptorBuilder.cpp`다(언리얼 엔진 5.8.3).

## 1. 실행 조건

- 구성: [포스팅 12](../12-npc-move-netserialize/measurements.md)의 최종 구성(⑧)에 `-MotionLog`. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, 기법 `-NodeUpdateFrequency 2 -InventoryOwnerOnly -InventoryFastArray -NpcInterpDelay 150 -NpcCompactMove`. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 본문의 수치는 둘째 묶음(2026-10-19 22:30\~23:02)이다. 본체 화면, 실행 중 PC 조작 없음, 12회 모두 종료 코드 0이다. 빌드는 커밋 `67cceff`의 소스다.

| 라벨 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-iris-base2` | 레거시(지금 구성) | 없음 | `r1` |
| `act2-iris-rg2` | Replication Graph | `-ReplicationGraph` | `r1` |
| `act2-iris2` | Iris | `-Iris -IrisFlatPriority` | `r2` |
| `act2-iris-prio2` | Iris, 기본 우선순위 | `-Iris` | `r1` |

- 첫 묶음(`act2-iris-*1`, 21:36\~22:09)은 보간 수정 전의 빌드(`4214580`)다. 본문의 "처음 잰 묶음"이고 수치는 쓰지 않는다([관찰 자료](candidates.md) 12절).
- 서버 로그로 구성을 확인했다: 리플리케이션 모델 `Generic`, `LabReplicationGraph`, `Iris`(`ND.cpp:1939`의 `InitBase` 줄), Iris 실행의 `lab_iris_class`(클래스별 폴링 간격)와 `lab_iris_prioritizer`(`outer=1 outside=1` 또는 `outer=0.2 outside=0.1`). 스크립트가 모든 클라이언트 로그에서도 모델 `Iris`를 확인했다.
- 서버 프레임 시간과 리플리케이션 시간은 `Scripts/export-insights.ps1`로 중앙값 실행의 트레이스에서 읽었다.
- 연결당 송신 대역폭은 CSV `out_bytes_per_sec_per_conn`의 중앙값이다. 구별의 기준은 [measurement.md](../../Docs/Guides/measurement.md)대로 CSV 중앙값의 변화가 큰 쪽 변동 폭보다 큰지다.
- Iris 실행에서는 CSV의 `open_actor_channels_per_conn`과 `saturated_ratio`가 늘 0이다(관찰 자료 4절). 포화는 서버 로그 `lab_saturated_frames`로 보았고 12회 모두 0이었다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 7.64 → 6.23ms, -18.4% | Timing Insights 7.636(`act2-iris-base2-r1`) → 6.228(`act2-iris2-r2`), 6.228 ÷ 7.636 − 1 = -18.44% | 관찰 자료 13.2절 |
| 리플리케이션 시간 3.35 → 1.89ms, -43.6% | `GameNetDriver` 프레임당 Incl 3.346 → 1.888, 1.888 ÷ 3.346 − 1 = -43.57% | 관찰 자료 13.2절 |
| 연결당 송신 대역폭 9,790 → 10,100바이트/초, +3.25% | CSV 중앙값 9,791 → 10,109(변동 폭 38, 6), 10,109 ÷ 9,791 − 1 = +3.25% | 관찰 자료 13.1절 |
| 1번 클라이언트에 존재하는 자원 노드 176 → 112개, -64개 | 화면 글자 `nodes=176`(`act2-iris-base2-r2-topdown-04`), `nodes=112`(`act2-iris2-r2-topdown-04`), t=75초. 112 − 176 = −64. 요약의 두 이미지가 이 두 파일이다 | 자동 스크린샷 |
| Iris는 150m 밖으로 벗어난 자원 노드를 클라이언트에서 지운다 | 격자 필터의 컬 거리는 액터의 `NetCullDistanceSquared`(150m)다(`Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/NetActorFactory.cpp:659-661`). 범위를 벗어난 객체는 파괴 경로로 간다(`Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ReplicationWriter.cpp:930-975`). 레거시는 Dormant 상태로 닫힌 채널의 액터를 클라이언트에 남긴다(`ND.cpp:5877-5889`) | 엔진 소스, 자동 스크린샷 |
| Iris는 언리얼 엔진의 새 리플리케이션 시스템 | 엔진 플러그인 `Engine/Plugins/Experimental/Iris`(`Iris.uplugin:13`, 기본으로 꺼짐), 명령줄 `-UseIrisReplication=1`이 켠다(`Engine/Source/Runtime/Net/Iris/Private/Iris/IrisConfig.cpp:15-16, 33-44`) | 엔진 소스 |
| 레거시는 엔진의 기본 리플리케이션 방식 | `net.Iris.UseIrisReplication` 기본값 0, `ReplicationDriverClassName`이 비면 드라이버 없이 넷 드라이버가 보낸다(`ND.cpp:1820-1831`). 서버 로그 모델 `Generic` | 엔진 소스, [engine-notes.md](../../Docs/Reference/engine-notes.md) 2절 |
| 수치는 구성마다 세 번 실행한 중앙값 | 중앙값 실행은 1절의 표, CSV 중앙값은 관찰 자료 13.1절 | 실행 조건 |
| 레거시는 클라이언트마다 액터 채널을 두고 액터 채널마다 액터를 보낸다 | `UActorChannel::ReplicateActor`(`Engine/Source/Runtime/Engine/Private/DataChannel.cpp`)를 연결마다 액터마다 부른다(`ND.cpp:5830`). 클래스 타이머(`DataChannel.cpp:3622-3625`의 `SCOPE_CYCLE_UOBJECT`)가 그 호출이다 | 엔진 소스 |
| 문제의 표: 확인하는 일 2.00, 1.05ms, 보내는 일 1.35, 1.61ms | 레거시: `GameNetDriver` Excl 2.001, 보내는 일 = Incl 3.346 − Excl 2.001 = 1.345(바로 아래 줄의 합 1.344). 그래프: `GameNetDriver` Excl 0.343 + `UReplicationGraph_ServerReplicateActors` Excl 0.705 = 1.048, 보내는 일 = 2.310 − 0.705 = 1.605(`act2-iris-base2-r1`, `act2-iris-rg2-r1`). 확인과 보내기의 나눔은 [포스팅 14 측정 기록](../14-replication-graph/measurements.md)과 같다 | 관찰 자료 13.2절 |
| 같은 시각에 잰 두 구성 | 둘째 묶음 안에서 연달아 잼(22:30\~22:46) | 실행 조건 |
| 액터 채널이 프레임마다 약 183번 액터를 보냈다 | 클래스 타이머의 프레임당 Count 합(RPC `ClientMoveResponsePacked` 제외): `LabNpc` 114.5 + `LabCharacter` 49.0 + `GameplayDebuggerCategoryReplicator` 6.2 + `LabPlayerController` 6.2 + `PlayerState` 2.1 + `GameStateBase` 2.0 + `WorldSettings` 2.0 + `LabBuilding` 0.9 + `LabResourceNode` 0.2 = 183.1(`act2-iris-base2-r1`) | `Saved/InsightsExport/act2-iris-base2-r1/summary.txt`, 계산 |
| Iris에는 액터 채널이 없다, 상태를 한 번 변환해 두고 클라이언트마다 쓴다 | Iris 실행의 `open_actor_channels_per_conn` 0(CSV). `UReplicationSystem::NetUpdate`가 필터, 폴링(`CallPreSendUpdate`), 양자화(`ExecuteQuantizeDirtyStateData`), 범위 갱신, 우선순위를 프레임에 한 번 한다(`RS.cpp:1085-1135`). 클라이언트마다 `UDataStreamChannel::Tick`이 패킷을 쓴다(`Engine/Source/Runtime/Engine/Private/Net/Experimental/Iris/DataStreamChannel.cpp:233-282`, 프레임당 8.0회) | 엔진 소스, 관찰 자료 13.2절 |
| 흐름도의 "정해진 간격에 폴링", "격자로 확인" | 폴링 간격은 `NetUpdateFrequency`에서(`NetActorFactory.cpp:122`, `ObjectPollFrequencyLimiter.h:114-128`), 격자 필터는 `UNetObjectGridWorldLocFilter`(`Engine/Config/BaseEngine.ini:1498, 1512`) | 엔진 소스, 관찰 자료 3절 |
| 폴링은 프로퍼티를 읽어 바뀌었는지 보는 일 | `UObjectReplicationBridge::BuildPollList`(`ORB.cpp:2038-2069`)가 고른 객체를 폴러가 읽고 복사한다(`FObjectPoller::PollAndCopy`) | 엔진 소스 |
| 폴링 간격: NPC 4프레임, 자원 노드 16프레임. 엔진 그대로면 NPC 3프레임 | 레거시 평균 간격 = (30 ÷ 빈도) + 1(빈도 10 → 4, 2 → 16). Iris는 내림(30 ÷ 빈도) − 1프레임을 건너뛰어(`ObjectPollFrequencyLimiter.h:114-128`) 빈도 10이면 3프레임마다 폴링한다. 이 글은 빈도를 30 ÷ (P + 0.5)로 바꿔 맞췄다. 서버 로그 `lab_iris_class`: `LabNpc` 10 → 6.66667(4프레임), `LabResourceNode` 2 → 1.81818(16프레임) | 엔진 소스, 계산, 서버 로그 |
| 우선순위 1.0, 엔진 그대로면 먼 액터를 늦게 보낸다 | `SphereNetObjectPrioritizerConfig`의 `OuterPriority`, `OutsidePriority`를 명령줄 `-ini:`로 1.0으로 덮어씀(서버 로그 `lab_iris_prioritizer ... outer=1 outside=1`). 기본값과 동작은 아래 우선순위 인용구의 줄 | `Scripts/run-scenario.ps1`, 서버 로그 |
| NPC 이동 구조체를 등록하지 않으면 Iris는 `NetSerialize`를 부르지 않는다 | 구조체 프로퍼티의 기본은 `FDefaultStructPropertyNetSerializerInfo`(`Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationState/PropertyNetSerializerInfoRegistry.cpp:170-175`)이고, 등록된 직렬화기가 없으면 멤버별 기술자를 만들며 경고를 남긴다(`RSDB.cpp:2622-2632`). LastResort 직렬화기는 `NetSerialize`의 결과를 덩어리로 보낸다(`LastResortPropertyNetSerializer.cpp:105-109, 167-171`) | 엔진 소스, 관찰 자료 3절 |
| 예상: 약 183번이 클라이언트마다 쓰는 8번이 된다 | 위의 183.1, 클라이언트 8 | 계산 |
| 예상: 보내는 일 1.35ms | 위의 1.345 | 관찰 자료 13.2절 |
| 예상: 대역폭이 Replication Graph처럼 약 6.8% 는다 | 그래프 10,452 ÷ 레거시 9,791 − 1 = +6.75%(둘째 묶음). 캐릭터 간격 1.3 → 1프레임은 그래프와 Iris가 같다(`lab_iris_class`의 `BP_LabCharacter_C` 1프레임) | 관찰 자료 13.1절, 계산 |
| `SetupIrisSupport`가 없으면 Iris는 FastArray를 보내지 않는다 | `UE_NET_HAS_IRIS_FASTARRAY_BINDING`이 없으면 UHT가 빈 바인딩을 만들고(`Intermediate/.../LabInventoryFastArrayComponent.gen.cpp:148-152`), Iris가 "not registered and won't be replicated"를 남긴다(`RSDB.cpp:2915-2921`). 정의는 `ModuleRules.SetupIrisSupport`(`Engine/Source/Programs/UnrealBuildTool/Configuration/Rules/ModuleRules.cs:1747-1762`) | 엔진 소스 |
| `-Iris`는 엔진 인자 `-UseIrisReplication=1`을 준다 | `Scripts/run-scenario.ps1`의 `$IrisArgs`. 서브오브젝트 등록 목록 `-ini:Engine:[SystemSettings]:net.SubObjects.DefaultUseSubObjectReplicationList=1`과 기록용 `-LabIris`도 준다 | 스크립트 |
| 예상과 실제: 보내는 일 0.404ms | `UDataStreamChannel_Tick` Incl 0.394 + `ClientMoveResponsePacked` 0.010 = 0.404(`act2-iris2-r2`) | 관찰 자료 13.2절 |
| 예상과 실제: 3.25% 늘었다 | 위 | 관찰 자료 13.1절 |
| Iris는 Replication Graph보다 11.2% 짧았다 | 6.228 ÷ 7.017 − 1 = -11.24%. CSV `work_avg_ms` 중앙값 6.755 → 5.966, 차이 0.789가 변동 폭 0.394, 0.064보다 크다 | 관찰 자료 13.1\~13.2절 |
| 결과 표: 서버 프레임 시간 7.64, 7.02, 6.23ms | Timing Insights 7.636, 7.017, 6.228(1절의 중앙값 실행) | 관찰 자료 13.2절 |
| 결과 표: 확인하는 일 2.00, 1.05, 1.48ms, 보내는 일 1.35, 1.61, 0.404ms | Iris: `GameNetDriver` Excl 0.370 + `ReplicationSystem_NetUpdate` Incl 1.114 = 1.484, 보내는 일 0.404. 합 1.888 = `GameNetDriver` Incl. 레거시와 그래프는 위 | 관찰 자료 13.2절, 계산 |
| 차트의 값 | 7.636, 7.017, 6.228 | 관찰 자료 13.2절 |
| 보내는 일은 레거시의 30.0%로 줄었다 | 0.404 ÷ 1.345 = 30.04% | 계산 |
| 클라이언트 8개에 프레임마다 한 번씩 쓴다 | `UDataStreamChannel_Tick` 프레임당 Count 8.0 | 관찰 자료 13.2절 |
| Iris의 확인하는 일에 폴링, 상태 변환, 격자로 확인하기, 우선순위가 든다 | `ReplicationSystem_NetUpdate`(`RS.cpp:1035-1142`) 안의 순서: `UpdateFiltering`, `CallPreSendUpdate`(폴링), `ExecuteQuantizeDirtyStateData`, `UpdateObjectScopes`, `UpdatePrioritization`(`RS.cpp:1085-1135`) | 엔진 소스 |
| 대역폭이 예상보다 덜 늘어난 이유는 확인하지 않았다 | Iris 묶음의 Networking Insights를 읽지 않았다(8절) | |
| 우선순위: 엔진 그대로의 Iris는 송신 대역폭을 9.28% 줄였다 | CSV 중앙값 9,791 → 8,882(`act2-iris-prio2`), 8,882 ÷ 9,791 − 1 = -9.28% | 관찰 자료 13.1절 |
| 우선순위: 20m 안 1.0, 100m 밖 0.1 | 기본 우선순위기 `SphereNetObjectPrioritizer`(`BaseEngine.ini:1494`), `InnerRadius` 2,000, `OuterRadius` 10,000, `InnerPriority` 1.0, `OuterPriority` 0.2, `OutsidePriority` 0.1(`Engine/Source/Runtime/Net/Iris/Public/Iris/ReplicationSystem/Prioritization/SphereNetObjectPrioritizer.h:18-35`). 20\~100m는 1.0에서 0.2로 준다 | 엔진 소스 |
| 우선순위: 바뀐 액터의 우선순위를 프레임마다 더하고 1.0 이상만 보낸다 | `FReplicationWriter::ScheduleObjects`(`ReplicationWriter.cpp:1202-1235`), `SchedulingThresholdPriority` 1.0(`ReplicationWriter.h:69`) | 엔진 소스 |
| 100m 밖의 NPC는 10프레임마다 간다 | 1.0 ÷ 0.1 = 10. NPC 수신 간격 P99 407.2\~407.6ms(약 12프레임, 폴링 간격 4프레임과 겹침) | 계산, 관찰 자료 13.3절 |
| 표시 속도 오차 초당 15.0cm → 98.6cm | 세 실행의 중앙값: `act2-iris2` 14.9, 15.0, 15.1 → 15.0, `act2-iris-prio2` 99.5, 98.6, 97.4 → 98.6 | 관찰 자료 13.3절 |
| 표시 속도 오차의 정의 | [ADR-0020](../../Docs/Decisions/0020-npc-motion-quality-metrics.md), [measurement.md](../../Docs/Guides/measurement.md) "수치의 이름과 출처" | 측정 정의 |
| 서버 프레임 시간은 구별되지 않았다(기본 우선순위) | CSV `work_avg_ms` 중앙값 5.966 → 6.159, 차이 0.193이 변동 폭 0.231보다 작다 | 관찰 자료 13.1절 |
| 화면 속 NPC의 두 지표는 세 구성에서 구별되지 않았다 | 표시 위치 오차 평균 45.00\~45.41cm, 표시 지연 151\~152ms, 표시 속도 오차 평균 14.9\~15.4cm/s(레거시, 그래프, Iris의 9실행) | 관찰 자료 13.3절 |
| 처음 잰 묶음에서 Iris의 일부 클라이언트에서 NPC가 멈췄다 | `act2-iris1-r1`의 클라이언트 0, 2, 3, 5, 7이 표시 속도 오차 67\~124cm/s | 관찰 자료 12.3절 |
| 어느 클라이언트에도 보내지 않는 액터를 폴링하지 않는다, 마지막으로 폴링한 상태로 간다 | `BuildPollList`가 `GetRelevantObjectsInternalIndices`(모든 연결의 범위의 합, `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/Filtering/ReplicationFiltering.cpp:396-410`)에서만 고른다(`ORB.cpp:2038-2069`). 범위에 새로 든 객체를 바로 폴링하는 설정은 없다(`net.Iris.*` 콘솔 변수 grep) | 엔진 소스, 관찰 자료 12.5절 |
| 그 상태에 든 서버 프레임 번호도 오래되었다 | 예: `act2-iris1-r1` 클라이언트 3, NPC 10646의 첫 수신 105, 33ms 뒤 239(다른 NPC의 최신 번호 237) | 모션 기록, 관찰 자료 12.5절 |
| 오래된 번호가 앞선 번호로 읽히면 시계가 최대 약 4초 당겨진다 | 8비트 번호를 ±128프레임 안에서 펼친다(`Source/DSOptLab/LabNpc.cpp`의 `FLabServerClock::Unwrap`). 132프레임 묵은 번호는 124프레임 앞선 번호가 되고, 124 × 33.4ms = 4.14초. 펼칠 수 있는 최대는 127프레임(약 4.2초) | 프로젝트 소스, 계산 |
| 클라이언트가 NPC의 첫 상태를 시계 계산에서 빼게 고쳤다 | 커밋 `67cceff`(`ALabNpc::AddSnapshot`). 같은 조건의 작은 규모에서 클라이언트 0, 5의 128.5, 78.6cm/s(`tsmall-iris4-r1`)가 8개 모두 15.4\~16.1cm/s(`tsmall-iris5-r1`)가 됐다 | 관찰 자료 12.5절 |
| 열린 액터 채널 수와 송신 한도 포화 비율을 재지 못했다 | CSV `open_actor_channels_per_conn`, `saturated_ratio`(1절) | 관찰 자료 4절 |

## 3. 서버가 남긴 CSV

실행별 행은 [관찰 자료](candidates.md) 13.1절에 있다. 중앙값(변동 폭)이다.

| 라벨 | work_avg_ms | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn |
| --- | --- | --- | --- | --- |
| `act2-iris-base2` | 7.388(0.100) | 12.405 | 3.566 | 9791(38) |
| `act2-iris-rg2` | 6.755(0.394) | 10.897 | 2.890 | 10452(11) |
| `act2-iris2` | 5.966(0.064) | 9.307 | 2.138 | 10109(6) |
| `act2-iris-prio2` | 6.159(0.231) | 9.827 | 2.205 | 8882(26) |

## 4. Timing Insights (중앙값 실행, 프레임당 ms)

| 값 | `act2-iris-base2-r1` | `act2-iris-rg2-r1` | `act2-iris2-r2` | `act2-iris-prio2-r1` |
| --- | ---: | ---: | ---: | ---: |
| 서버 프레임 시간 평균 / P99 | 7.636 / 12.774 | 7.017 / 11.314 | 6.228 / 9.676 | 6.433 / 10.308 |
| `GameNetDriver` Incl / Excl | 3.346 / 2.001 | 2.654 / 0.343 | 1.888 / 0.370 | 1.940 / 0.382 |
| 확인하는 일 | 2.001 | 1.048 | 1.484 | 1.545 |
| 보내는 일 | 1.345 | 1.605 | 0.404 | 0.395 |

## 5. 시각 자료

- 요약의 두 이미지는 1번 클라이언트의 자동 스크린샷(t=75초) `act2-iris-base2-r2-topdown-04.png`, `act2-iris2-r2-topdown-04.png`를 복사했다(`images/topdown-legacy.png`, `images/topdown-iris.png`).
- 원리의 흐름도는 Mermaid로 그렸다. GitHub의 미리보기로는 아직 확인하지 않았다.
- Insights 캡처는 찍지 않았다. 4절의 값은 `Scripts/export-insights.ps1`로 창 없이 읽었다. 영상은 허가를 받은 뒤에 찍는다(관찰 자료 14절).

## 6. 확인하지 않은 것

- Iris가 Replication Graph보다 대역폭을 덜 늘린 이유. 캐릭터 간격은 같으므로 Iris의 패킷 머리 정보가 작다고 추정하지만, Iris 묶음의 Networking Insights를 읽지 않았다.
- `ReplicationSystem_NetUpdate` 안의 단계별 시간(필터, 폴링, 양자화, 우선순위).
- 보간 수정(`67cceff`)이 레거시와 그래프의 품질 지표를 바꿨는지. 첫 묶음과 둘째 묶음은 시각이 달라 비교하지 않았다. 둘째 묶음의 레거시와 그래프 지표는 포스팅 14의 값과 같은 범위다(표시 속도 오차 15.0\~15.4cm/s).
- 범위에 새로 든 NPC가 오래된 위치로 한 번 나타나는 것이 화면에서 보이는지.
