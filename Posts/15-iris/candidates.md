# 포스팅 15 후보 기법 자료 (Iris)

2막 구현 계획 단계 5의 2번(관찰 자료)이다. 기준 구성은 포스팅 12의 최종 구성(⑧, `-NodeUpdateFrequency 2 -InventoryOwnerOnly -InventoryFastArray -NpcInterpDelay 150 -NpcCompactMove`)이다. 이번에는 새로 재지 않았다. 엔진 소스와 프로젝트 소스만 읽었고, Iris로 실행해 본 적은 아직 없다. 엔진 소스는 언리얼 엔진 5.8.3(`G:\Epic Games\UE_Source`, 태그 `5.8.3-release`)이다. 아래에서 `ND.cpp`는 `Engine/Source/Runtime/Engine/Private/NetDriver.cpp`, `ERB.cpp`는 `Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/EngineReplicationBridge.cpp`, `ORB.cpp`는 `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ObjectReplicationBridge.cpp`, `RSDB.cpp`는 `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationState/ReplicationStateDescriptorBuilder.cpp`, `BaseEngine.ini`는 `Engine/Config/BaseEngine.ini`다. 소스에서 확인한 것과 추론을 나눠 적는다. 추론은 "(추론)"으로 표시했고, 작은 규모 실행으로 확인할 것이다.

## 1. 출발점: 포스팅 14의 둘째 묶음

같은 날 연달아 잰 묶음이다(2026-10-19 17:30\~17:53, ⑧에 `-MotionLog`, [Worklog/14-replication-graph.md](../../Docs/Worklog/14-replication-graph.md) "태스크 32 준비"). Iris 묶음과는 다시 연달아 재야 비교할 수 있다.

| 묶음 | 리플리케이션 | `work_avg_ms` | `out_bytes_per_sec_per_conn` | `GameNetDriver` 프레임당(`r3`) |
| --- | --- | ---: | ---: | ---: |
| `act2-rg-trk-base1` | 레거시(⑧) | 8.555 | 10,080 | 3.889ms |
| `act2-rg-trk1` | Replication Graph(⑨) | 7.362 | 10,969 | 2.880ms |

- 레거시에서 남은 비용은 활성 목록을 따지는 두 단계(`Consider Actors Time` 0.959, `Prioritize Actors Time` 0.845ms, ④에서 잼)였다. 그래프는 이 두 단계를 돌지 않아 줄였다([포스팅 14 관찰 자료](../14-replication-graph/candidates.md) 1절, 9절).
- 그래프가 더 보낸 8.8%는 캐릭터 몫이다. 엔진이 간격을 정수 프레임으로 다뤄서, 레거시에서 평균 1.3프레임 간격이던 캐릭터를 그래프는 매 프레임 보냈다([engine-notes.md](../../Docs/Reference/engine-notes.md) 15절).

## 2. Iris로 바꾸는 방법 (엔진 소스)

| 사실 | 소스 위치 |
| --- | --- |
| 콘솔 변수 `net.Iris.UseIrisReplication`의 코드 기본값은 0이다. 명령줄 `-UseIrisReplication=1`이 모든 설정을 덮어쓴다 | `Engine/Source/Runtime/Net/Iris/Private/Iris/IrisConfig.cpp:15-16, 33-44`, `Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp:14864-14874` |
| `GameNetDriver`는 Iris를 쓸 수 있는 넷 드라이버로 등록되어 있다(`bCanUseIris=true`) | `BaseEngine.ini:346` |
| Iris를 쓰려면 `IrisCore` 모듈이 로드되어 있어야 한다. 로드되어 있지 않으면 ensure를 띄우고 레거시로 돈다. `IrisCore`를 로드하는 것은 Iris 플러그인 모듈이고, 이 플러그인은 기본으로 꺼져 있다 | `UnrealEngine.cpp:14878-14884`, `Engine/Plugins/Experimental/Iris/Source/Iris/Private/Iris/IrisModule.cpp:8-11`, `Iris.uplugin:13` |
| 게임 모듈의 Build.cs가 `SetupIrisSupport(Target)`를 부르지 않으면 `UE_NET_HAS_IRIS_FASTARRAY_BINDING`이 정의되지 않는다. 그러면 UHT가 FastArray에 빈 바인딩(STUB)을 만들고, Iris는 그 FastArray를 리플리케이트하지 않는다(로그 `FFastArray property ... not registered and won't be replicated`). 이 프로젝트의 Build.cs에는 이 호출이 없다 | `Engine/Source/Programs/UnrealBuildTool/Configuration/Rules/ModuleRules.cs:1747-1762`, `Intermediate/.../LabInventoryFastArrayComponent.gen.cpp:148-152`, `RSDB.cpp:2915-2921`, `Source/DSOptLab/DSOptLab.Build.cs` |
| Iris는 서브오브젝트를 등록 목록으로 리플리케이트하기를 요구한다(`net.SubObjects.DefaultUseSubObjectReplicationList=1`이 아니면 ensure). 이 값은 액터와 컴포넌트의 생성자에서 기본값으로 쓰이므로, 켜면 레거시 실행의 경로도 바뀐다. 엔진과 프로젝트 설정에는 이 값이 없다 | `ERB.cpp:234`, `Actor.cpp:292`, `Components/ActorComponent.cpp:115-118, 578` |
| 클라이언트는 접속할 때 자기가 Iris를 쓰는지 서버에 알리고(`GetNetworkRuntimeFeatures`), 서버와 다르면 넷 드라이버를 서버에 맞추려 한다(`TryUpgradeNetworkFeatures`). 맞추지 못하면 접속이 끊긴다 | `ND.cpp:2702-2775`, `NetConnection.cpp:1336-1375` |
| Iris를 쓰면 리플리케이션 드라이버를 만들지 않으므로 Replication Graph와 함께 쓸 수 없다 | `ND.cpp:1867-1870` |
| 서버 로그의 `using replication model Iris`로 확인한다. 지금 실행의 CSV 메타데이터에는 `iris="0"`이 남는다 | `ND.cpp:1939, 2369-2382`, 서버 로그(`act2-rg-notrk1`) |
| Iris를 쓰는 동안에는 Push Model 핸들을 만들지 않는다. 이 프로젝트는 Push Model을 쓰지 않으므로 영향이 없다 | `ND.cpp:1931-1935` |

## 3. ①\~⑧이 Iris에서 어떻게 되나

| 단계 | 레거시에서 바꾼 것 | Iris에서 (소스) | 확인할 것 |
| --- | --- | --- | --- |
| ① 거리 판정 | `bAlwaysRelevant`를 끄고 150m 거리 판정 | 액터의 기본 필터는 격자 필터 `UNetObjectGridWorldLocFilter`다. 컬 거리는 같은 `NetCullDistanceSquared`에서 가져오고(`NetActorFactory.cpp:659-661`), 칸(200m) 단위가 아니라 실제 거리로 판정한다(`bUseExactCullDistance` true, `NetObjectGridFilter.h:59-83`). 클래스 기본 객체가 `bAlwaysRelevant`, `bOnlyRelevantToOwner`, `bNetUseOwnerRelevancy` 가운데 하나라도 켜져 있으면 격자 필터를 쓰지 않는다(`ERB.cpp:242-253`). 범위를 벗어나도 6프레임은 유지한다(`BaseEngine.ini:1502-1503`). `IsNetRelevantFor`와 `GetNetPriority`는 부르지 않는다(grep 0건) | 클래스 기본 객체가 명령줄 설정을 따르는지(생성자가 `FLabServerConfig::Get()`를 읽는다) |
| ② Dormant 상태 | 자원 노드와 건축물을 Dormant 상태로 | `NetDormancy`가 객체 하나의 "Dormant 상태를 원함" 비트로 간다(`ReplicationSystemUtil.cpp:672-689`). 이 비트가 켜진 객체는 프레임마다의 폴링(값이 바뀌었는지 확인)에서 빠지고, `FlushNetDormancy`가 오면 한 번 폴링된다(`net.Iris.UseDormancyToFilterPolling` 기본값 true, `ORB.cpp:77-82, 2075-2098`). 연결마다 채널을 닫는 구조가 아니다 | 필터 범위를 벗어난 Dormant 자원 노드를 클라이언트에서 지우는지(추론: 지운다. 범위 밖 객체는 `ReplicationWriter.cpp:930-975`의 파괴 경로로 간다). 지우면 클라이언트의 자원 노드가 레거시의 176개보다 적다 |
| ③ NPC 업데이트 빈도, ④ 자원 노드 업데이트 빈도 | NPC 10, 자원 노드 2 | `NetUpdateFrequency`가 폴링 빈도가 되고(`NetActorFactory.cpp:122`), "몇 프레임마다"로 바뀐다: 내림(30 ÷ 빈도) − 1프레임을 건너뛴다(`ObjectPollFrequencyLimiter.h:114-128`, 기준은 `NetServerMaxTickRate`, `ERB.cpp:753`). 난수는 없고, 같은 빈도의 객체끼리 정해진 순서로 나눠 돈다(`ObjectPollFrequencyLimiter.h:90-98`). NPC는 3프레임, 자원 노드는 15프레임, 캐릭터(100)는 매 프레임마다다. 실행 중 `SetNetUpdateFrequency`도 따라간다(`net.Iris.EnableDynamicNetUpdateFrequency` 기본값 true) | 레거시의 평균 간격은 (30 ÷ 빈도) + 1프레임이다(NPC 4, 자원 노드 16, 캐릭터 1.3, 포스팅 14 관찰 자료 7절). 소스대로라면 Iris는 NPC를 3분의 1 더 자주 보낸다(추론) |
| ⑤ 인벤토리를 소유자에게만 | `COND_OwnerOnly` | 지원한다. 연결마다 소유자인지 따진다(`ReplicationConditionals.cpp:915-919`). `COND_OwnerOnly` 프로퍼티는 따로 묶인 상태(`HasLifetimeConditionals`)로 들어간다(`RSDB.cpp:2064-2080`) | 이 프로젝트는 조건을 서버에만 준다: 인자 `-LabInventoryOwnerOnly`가 서버에만 가서 클라이언트는 `COND_None`으로 등록한다(`LabInventoryComponent.cpp:12-18`, `run-scenario.ps1:278`). 레거시는 조건을 보내는 쪽만 쓰므로 문제가 없었다. Iris는 클라이언트가 받은 프로토콜 ID를 자기 것과 비교해 다르면 그 객체를 묶지 않는다(`ORB.cpp:1700-1726`, 로그 `Protocol mismatch prevents binding`). 조건에 따라 상태 묶음이 달라지므로 인벤토리가 클라이언트에서 묶이지 않을 수 있다(추론). 클라이언트에도 인자를 주면 피한다 |
| ⑥ 인벤토리 FastArray | `FFastArraySerializer` | 레거시 방식의 FastArray를 알아보고 FastArray용 리플리케이션 조각을 쓴다(`RSDB.cpp:2423-2486`, `net.Iris.UseNativeFastArray` 기본값 true). 단, 2절의 `SetupIrisSupport`가 있어야 한다 | Build.cs에 한 줄을 더한 뒤 인벤토리 패널로 바뀐 칸이 오는지 |
| ⑦ NPC 보간 | 서버 프레임 번호를 보내고 150ms 늦게 보간 | `PreReplication`을 Iris도 부른다(`ERB.cpp:114-125`). `ServerFrame`의 `COND_Never`는 상태에서 아예 빠진다(`RSDB.cpp:2899-2902`). 보간 인자는 서버와 클라이언트에 모두 가므로(`run-scenario.ps1:199, 293`) 양쪽 상태가 같다 | 품질 지표(표시 지연, 표시 속도 오차)가 레거시와 같은지 |
| ⑧ NPC 이동 구조체 | `FLabNpcMove`를 `NetSerialize`로 42비트에 직접 씀 | Iris는 구조체의 `NetSerialize`를 쓰지 않는다. 등록된 Iris 직렬화기가 없으면 `UPROPERTY` 멤버(`FVector2D Offset`, `float Yaw`, `uint8 ServerFrame`)를 각자의 기본 직렬화기로 보내고 경고 `Generating descriptor for struct LabNpcMove that has custom serialization`을 남긴다(`PropertyNetSerializerInfoRegistry.cpp:170-175`, `RSDB.cpp:2622-2632`). 1cm 반올림, Yaw 1바이트 같은 압축이 빠진다. `MoveOrigin`의 `COND_InitialOnly`는 지원한다(`RSDB.cpp:2093-2098`) | NPC 갱신 한 번의 비트(Networking Insights). 포스팅 12에서는 69.04비트였다 |

`ReplicatedMovement`(캐릭터)는 Iris 전용 직렬화기 `FRepMovementNetSerializer`가 있다(`Engine/Source/Runtime/Engine/Private/RepMovementNetSerializer.cpp:22-116`). 프로젝트의 RPC는 `ServerReportReady`, `ClientStartScenario`, `ServerHarvest` 셋이고 모두 Reliable이다(`LabPlayerController.h:27, 41, 45`). RPC에 Iris만의 제약이 있는지는 확인하지 않았다.

### 3.1 ⑧을 Iris에서 살리는 방법

| 방법 | 구현 | Iris에서 보내는 것 |
| --- | --- | --- |
| 그대로 둠 | 없음 | 멤버별 기본 직렬화. 바뀐 멤버만 간다 |
| LastResort 직렬화기로 등록 | 매크로 한 줄(`UE_NET_IMPLEMENT_NAMED_STRUCT_LASTRESORT_NETSERIALIZER_AND_REGISTRY_DELEGATES`, `PropertyNetSerializerInfoRegistry.h:296-302`). 엔진도 같은 매크로를 쓴다(`NetworkPhysicsComponent.cpp:455`) | `NetSerialize`가 쓴 비트를 그대로 덩어리로 보낸다(`LastResortPropertyNetSerializer.cpp:105-109, 167-171`). 덩어리의 길이를 함께 보내는지, 그 비트 수는 확인하지 않았다 |
| Iris 직렬화기를 직접 작성 | `FNetSerializer`(양자화, 직렬화, 비교) 약 150줄 | 레거시의 42비트와 같게 쓸 수 있다 |

## 4. 측정 지표가 Iris에서 남는지

| 지표 | Iris에서 | 근거 |
| --- | --- | --- |
| `work_avg_ms`, `netflush_avg_ms` | 남는다 | Iris의 일(`InternalIrisUpdateTransactional`)이 `TickFlush` 안에서 돈다(`ND.cpp:1210-1214`) |
| `out_bytes_per_sec_per_conn` | 남는다 | 연결의 `OutTotalBytes`는 패킷을 보낼 때 쌓인다(`NetConnection.cpp:2585-2589`, `LabMetricsSubsystem.cpp:152`) |
| 리플리케이션 시간 타이머 `GameNetDriver` | 남는다 | `TickFlush`의 `TRACE_CPUPROFILER_EVENT_SCOPE_TEXT`(`ND.cpp:1174`)가 Iris의 일도 감싼다 |
| `open_actor_channels_per_conn` | 0이 된다(추론) | Iris는 액터 채널 대신 `UDataStreamChannel` 하나로 보낸다. 지표는 `ActorChannelsNum()`을 센다(`LabMetricsSubsystem.cpp:153`) |
| `saturated_ratio` | 늘 0이 된다 | 리플리케이션 횟수를 세는 `TrackReplicationForAnalytics`는 레거시의 `ServerReplicateActors` 안에만 있다(`ND.cpp:6022-6023`). 지표는 분모가 0이면 0을 낸다(`LabMetricsSubsystem.cpp:351-352`). 연결이 프레임마다 남기는 포화 프레임 수(`TrackFrame(!IsNetReady())`, `NetConnection.cpp:5110`, `GetNumberOfSaturatedFrames`, `NetAnalyticsTypes.h:235-245`)는 두 시스템에서 모두 남는다 |
| Consider List 길이 | 남지 않는다 | Consider List를 만들지 않는다(`ND.cpp:5454`) |
| `lab_network_objects`(활성 목록) | 빈 목록이다 | Iris에서는 활성 목록에 넣지 않는다(`ND.cpp:976-982`) |
| 클래스 타이머(`LabNpc`, `LabCharacter`) | 남지 않는다(추론) | 레거시의 `UActorChannel::ReplicateActor` 안에 있다(`DataChannel.cpp:3622-3625`). Iris의 단계 타이머는 `IRIS_PROFILER_SCOPE`(`IrisProfiler.h:46`)이고 이름이 다르다 |
| Networking Insights | 남는다(추론) | Iris도 `UE_NET_TRACE`를 쓴다(`ReplicationSystem.cpp:1081`, `DataStreamChannel.cpp:360-365`) |
| 품질 지표(모션 기록) | 남는다(추론) | 클라이언트가 받은 NPC 위치를 기록한다. 리플리케이션 경로와 상관없다 |

송신 한도 350,000바이트/초는 Iris에도 걸린다(추론): Iris의 `UDataStreamChannel`도 `IsNetReady()`와 `QueuedBits`를 보고(`DataStreamChannel.cpp:217, 248, 303-311`), `QueuedBits`는 연결의 `CurrentNetSpeed`로 채워진다(`NetConnection.cpp:5122`).

## 5. 비용에 영향을 주는 Iris 설정 (기본값)

| 콘솔 변수와 설정 | 기본값 | 뜻 | 소스 위치 |
| --- | --- | --- | --- |
| `net.Iris.UseFrequencyBasedPolling` | true | `NetUpdateFrequency`로 폴링 간격을 정한다 | `ORB.cpp:70-75` |
| `net.Iris.UseDormancyToFilterPolling` | true | Dormant 객체를 폴링하지 않는다 | `ORB.cpp:77-82` |
| `PollConfigs`(`ObjectReplicationBridgeConfig`) | 없음 | 클래스마다 폴링 빈도를 덮어쓴다 | `BaseEngine.ini:1509`(예시 주석) |
| `net.Iris.EnableDeltaCompression` | true | 기준 상태와의 차이만 보낸다. 쓰는 클래스는 `DeltaCompressionConfigs`에 있는 것뿐이다(Pawn, PlayerState 등) | `DeltaCompressionBaselineManager.cpp:29-31`, `BaseEngine.ini:1525-1528` |
| `net.Iris.ReplicationWriterMaxAllowedPacketsIfNotHugeObject` | 3 | 한 프레임에 연결 하나가 보내는 패킷 수 상한 | `DataStreamChannel.cpp:47-48` |
| `net.Iris.ReplicationWriterMaxDestroyObjectsPerFrame` | 150 | 한 프레임에 보내는 파괴 수 상한 | `ReplicationWriter.cpp:87-89` |
| `net.Iris.UseChangeMaskForTArray` | true | 일반 `TArray`를 원소 단위로 바뀐 것만 보낸다 | `RSDB.cpp:64-65` |

마지막 줄은 ⑥을 끈 구성(일반 `TArray` 인벤토리)이 Iris에서 레거시와 다르게 움직인다는 뜻이다(추론). 이번 비교의 ⑧ 구성은 ⑥을 켜므로 영향이 없다.

## 6. 구현 비용

| 항목 | 내용 | 크기 |
| --- | --- | --- |
| 실행 인자 | `run-scenario.ps1 -Iris`: 서버와 클라이언트에 `-UseIrisReplication=1`, 서버에 기록용 `-LabIris`(`config` 열에 `Iris`). `-ReplicationGraph`와 함께 주면 거부 | 수십 줄 |
| 플러그인과 빌드 | `.uproject`에 `Iris` 플러그인, `DSOptLab.Build.cs`에 `SetupIrisSupport(Target)` | 몇 줄. 레거시 실행에서는 Iris 모듈이 떠 있기만 하고 쓰이지 않는다(추론) |
| 서브오브젝트 목록 | `net.SubObjects.DefaultUseSubObjectReplicationList=1`. ini에 넣으면 레거시 묶음도 바뀌므로, Iris 실행에만 명령줄로 줄지 모든 묶음에 줄지 정해야 한다. 생성자가 읽는 값이라 명령줄로 일찍 적용되는지 확인해야 한다 | 한 줄과 확인 실행 |
| 인벤토리 조건 | 클라이언트에도 `-LabInventoryOwnerOnly`를 준다(3절 ⑤) | 한 줄 |
| ⑧ 구조체 | 3.1의 셋 가운데 하나 | 0\~150줄 |
| 지표 | `open_actor_channels_per_conn`과 `saturated_ratio`를 Iris에서 대신할 값, `lab_network_objects`를 대신할 값(클라이언트의 자원 노드 수는 지금도 화면 글자로 남는다) | 수십 줄 |
| 확인 실행 | 작은 규모: 모델 `Iris`, `Protocol mismatch`와 FastArray 오류가 없음, 인벤토리 패널, NPC 이동, 클라이언트의 자원 노드 수 | 몇 회 |

## 7. 비교할 묶음의 후보

[ADR-0021](../../Docs/Decisions/0021-replication-system-comparison-table.md)에 따라 Iris는 ⑧의 인자를 그대로 두고 리플리케이션 시스템만 바꾼 구성이다. 모두 ⑧의 인자에 `-MotionLog`를 더한다.

| 묶음 | 리플리케이션 | 무엇을 보나 |
| --- | --- | --- |
| Z0 | 레거시(⑧) | 기준 |
| Z1 | Replication Graph(⑨, trickle 기본) | 갈래 표의 세 칸을 한 묶음에서 잼 |
| Z2 | Iris | 엔진 기본 동작 그대로 |
| Z3 | Iris, 간격을 레거시에 맞춤 | `PollConfigs`로 NPC를 4프레임, 자원 노드를 16프레임 간격에 맞춤(빈도 7.5와 1.875). 포스팅 14의 그래프와 같은 방식. 캐릭터는 정수 간격이라 1프레임 그대로 |

정해야 할 것:

1. 간격: Z2(엔진 그대로), Z3(레거시 평균에 맞춤), 또는 둘 다.
2. ⑧ 구조체: 3.1의 셋 가운데 하나.
3. 서브오브젝트 목록 설정을 Iris 묶음에만 줄지 모든 묶음에 줄지.
4. Z1을 넣을지. 넣지 않으면 갈래 표의 그래프 칸은 README를 다시 잴 때 채운다.

세 묶음이면 연달아 약 25분, 네 묶음이면 약 33분이다(포스팅 14의 9회가 23분, 12회가 32분).

## 8. 에이전트 의견

- **묶음은 Z0, Z1, Z3을 추천한다.** Z3은 포스팅 14가 그래프의 간격을 레거시 평균에 맞춘 것과 같은 기준이다. 그래야 Z1과 Z3의 차이가 "리플리케이션 시스템"에서 온다. Z2를 고르면 엔진 기본 동작을 그대로 보여 주지만, NPC를 3분의 1 더 자주 보내는 효과가 섞인다. 그 효과는 포스팅 14에서 이미 따로 보인 것이다. Z1을 넣으면 갈래 표의 세 칸이 같은 묶음에서 나온다.
- **⑧ 구조체는 LastResort 등록을 추천한다.** 매크로 한 줄로 `NetSerialize`의 비트를 살린다. 그대로 두면 ⑧(송신량 -20.4%)이 Iris에서만 빠져, 송신량 차이에 리플리케이션 시스템과 ⑧이 섞인다. 직접 작성하는 직렬화기는 가장 정확하지만 약 150줄이고, 다루는 것이 Iris API라 이 글의 주제(같은 설정에서 시스템만 바꿈)를 넘는다.
- **서브오브젝트 목록은 Iris 묶음에만 주는 것을 추천한다.** 레거시와 그래프 묶음이 포스팅 14와 같은 경로로 돈다. 명령줄로 일찍 적용되지 않으면 모든 묶음에 줘야 하므로, 작은 규모에서 먼저 확인한다.
- 근거로 삼은 수치는 1절의 둘째 묶음과 포스팅 14 관찰 자료 7절의 간격 계산이다. Iris의 비용이 레거시나 그래프보다 작을지는 소스로 예상하지 못했다. 폴링, 필터, 우선순위 단계가 연결 수와 객체 수에 따라 어떻게 늘지 재 봐야 안다.

## 9. 확인하지 않은 것

- 모든 "(추론)" 항목: 액터 채널 수가 0인지, Dormant 자원 노드를 클라이언트에서 지우는지, 인벤토리 조건의 프로토콜 불일치, 송신 한도가 걸리는지, Networking Insights와 품질 지표가 남는지.
- 클라이언트에 Iris 플러그인이 없을 때 접속이 끊기는지. 두 쪽에 같은 인자와 같은 빌드를 쓰므로 이번 측정에는 상관없다.
- LastResort 직렬화기가 덩어리마다 붙이는 비트 수.
- RPC에 Iris만의 제약이 있는지.
- 프로토콜 ID를 만들 때 조건이 어떻게 들어가는지(`ORB.cpp`의 비교까지만 봤다).

## 10. 관찰과 선택

사용자가 7절의 정해야 할 것 넷을 정한다. 정한 것을 여기에 옮긴다.
