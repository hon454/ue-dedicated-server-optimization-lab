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

사용자가 8절의 추천대로 정했다(2026-10-19).

1. 간격: 레거시 평균에 맞춘다(Z3).
2. ⑧ 구조체: LastResort 직렬화기로 등록한다.
3. 서브오브젝트 목록 설정: Iris 묶음에만 준다.
4. 묶음: Z0(레거시 ⑧), Z1(Replication Graph ⑨, trickle 기본), Z3(Iris, 간격 맞춤). 모두 ⑧의 인자에 `-MotionLog`를 더한다.

구현에서 정한 것(에이전트):

- 실행 인자는 `-Iris`다. 서버와 클라이언트에 `-UseIrisReplication=1`, 기록용 `-LabIris`(`config` 열에 `Iris`), `-ini:Engine:[SystemSettings]:net.SubObjects.DefaultUseSubObjectReplicationList=1`을 준다. `-ReplicationGraph`와 함께 주면 스크립트가 거부한다. 서버와 모든 클라이언트의 로그에 `GameNetDriver`의 `using replication model Iris`가 없으면 실패로 본다.
- `.uproject`에 `Iris` 플러그인을, `DSOptLab.Build.cs`에 `SetupIrisSupport(Target)`를 더했다. 모든 묶음의 빌드가 바뀌므로 Z0과 Z1도 포스팅 14의 묶음과 비교하지 않는다(어차피 연달아 다시 잰다).
- 간격은 액터의 Net Update Frequency를 Iris가 레거시 평균 간격 P프레임마다 폴링하는 값 틱 ÷ (P + 0.5)로 바꿔 맞춘다. P는 포스팅 14와 같이 반올림((30 ÷ 빈도) + 1)이다. Iris로 도는 서버에서만, 게임 모드가 이미 있는 액터와 새로 스폰되는 액터에 적용한다(`ALabGameMode::AlignIrisPollInterval`). 결과는 서버 로그 `lab_iris_class`다. `PollConfigs`를 쓰지 않은 것은 자원 노드 빈도처럼 인자로 바뀌는 값을 ini에 고정하지 않으려고다.
- `FLabNpcMove`는 `LabNpc.cpp`에서 `UE_NET_IMPLEMENT_NAMED_STRUCT_LASTRESORT_NETSERIALIZER_AND_REGISTRY_DELEGATES(LabNpcMove)`로 등록했다.
- 인벤토리 조건 인자 `-LabInventoryOwnerOnly`를 클라이언트에도 준다(모든 묶음). 레거시는 조건을 보내는 쪽만 쓰므로 레거시와 그래프의 송신은 바뀌지 않는다.
- `saturated_ratio`가 Iris에서 늘 0이므로, 측정 구간의 포화 프레임 수를 서버 로그 `lab_saturated_frames measured=<포화>/<기록>`으로 모든 묶음에 남긴다. CSV 열은 바꾸지 않았다.

## 11. 작은 규모 확인

클라이언트 2, 자원 노드 100, NPC 10과 플레이어 주변 50, ⑧의 인자, `-MotionLog`, `-InventoryPanelSlot 0`, `-NoTrace`로 돌렸다(2026-10-19 20:56\~21:05). 수치는 비교에 쓰지 않는다.

| 값 | `tsmall-iris-base1-r1`(레거시) | `tsmall-iris2-r1`(Iris, 간격 맞춤) | `tsmall-iris3-r1`(Iris, 간격 맞춤, 우선순위 1.0) |
| --- | ---: | ---: | ---: |
| `out_bytes_per_sec_per_conn` | 6,517 | 7,044 | 7,372 |
| `open_actor_channels_per_conn` | 58 | 0 | 0 |
| NPC 수신 간격 평균 / P99(ms) | 133.5 / 166.7 | 138.4 / 400.0 | 133.4 / 133.9 |
| 표시 속도 오차 평균(cm/s) | 12.9 | 88.8 | 13.1 |
| 표시 위치 오차 평균(cm) / 표시 지연(ms) | 46.69 / 155 | 47.56 / 153 | 48.83 / 165 |
| 1번 클라이언트의 자원 노드(t=45s, 자동 스크린샷) | 7 | 4(`tsmall-iris1-r1`) | 3 |

- 세 실행 모두 서버와 클라이언트 로그에 `Protocol mismatch`, FastArray 미등록 오류, 서브오브젝트 목록 경고, `has custom serialization` 경고가 없다. 서버 로그의 모델은 `Iris`이고, 클라이언트 로그에는 `PendingNetDriver (NetDriverDefinition GameNetDriver) using replication model Iris`가 남는다. 서브오브젝트 목록을 명령줄(`-ini:Engine:[SystemSettings]:...`)로 준 설정이 적용되었다.
- 인벤토리 패널: 자기 인벤토리 200칸이 오고, 다른 플레이어의 칸은 오지 않는다(`COND_OwnerOnly`). 맨 앞 칸 번호가 바뀌므로 FastArray 갱신이 온다(`tsmall-iris1-r1-tpp-02.png`).
- `lab_iris_class`: `LabNpc` 10 → 6.67(4프레임), `LabResourceNode` 2 → 1.82(16프레임), `BP_LabCharacter_C`와 `LabBuilding` 100 → 20(1프레임), `PlayerState` 1 → 0.95(31프레임), `GameStateBase`와 `WorldSettings` 10 → 6.67(4프레임). `LabPlayerController`는 기록되지 않았다. 빈도가 100이라 맞춰도 1프레임으로 같다.
- 첫 실행(`tsmall-iris1-r1`)에서는 모션 기록이 비었다. 모션 기록은 NetGUID로 서버와 클라이언트의 NPC를 짝짓는데, Iris는 NetGUID를 정하지 않는다. 그래서 Iris에서는 `NetRefHandle`의 ID로 짝짓게 고쳤다(`LabMotionLogSubsystem.cpp`의 `GetGuid`).
- Iris 클라이언트의 자원 노드가 레거시보다 적다. 3절 ②의 추론대로 Iris는 필터 범위를 벗어난 Dormant 자원 노드를 클라이언트에서 지운다.

### 11.1 Iris의 거리 우선순위가 먼 NPC를 늦춘다

간격을 맞춘 `tsmall-iris2-r1`에서 NPC 수신 간격 P99가 400ms(12프레임), 표시 속도 오차 평균이 88.8cm/s였다(레거시는 166.7ms, 12.9cm/s). 엔진 소스:

| 사실 | 소스 위치 |
| --- | --- |
| 액터의 기본 우선순위기는 `SphereNetObjectPrioritizer`다 | `BaseEngine.ini:1494` |
| 시점에서 20m 안은 우선순위 1.0이다. 20\~100m에서는 1.0에서 0.2로 줄고, 100m 밖은 0.1이다 | `Engine/Source/Runtime/Net/Iris/Public/Iris/ReplicationSystem/Prioritization/SphereNetObjectPrioritizer.h:18-35`(`InnerRadius` 2,000, `OuterRadius` 10,000, `InnerPriority` 1.0, `OuterPriority` 0.2, `OutsidePriority` 0.1) |
| 바뀐 객체는 프레임마다 우선순위를 쌓고, 쌓인 값이 1.0 이상인 객체만 보낸다 | `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ReplicationWriter.cpp:1202-1235`, `ReplicationWriter.h:69`(`SchedulingThresholdPriority` 1.0) |

그래서 100m 밖의 NPC는 우선순위를 쌓는 데 10프레임이 걸리고, 폴링 간격(4프레임)보다 늦게 간다. 레거시는 고려한 액터를 송신 한도가 허락하는 한 모두 보낸다. 우선순위를 거리와 상관없이 1.0으로 둔 `tsmall-iris3-r1`(`-IrisFlatPriority`, 서버 로그 `lab_iris_prioritizer ... outer=1 outside=1`)에서는 NPC 수신 간격이 정확히 4프레임이었다(평균 133.4ms, P99 133.9ms). 표시 속도 오차도 레거시와 같았다. 송신량은 7,044에서 7,372로 늘었다.

에이전트 의견: Z3의 목적은 레거시와 같은 송신 일정에서 시스템만 바꾸는 것이고, 그 목적에는 우선순위 1.0이 맞다. Iris의 기본 우선순위기는 먼 객체의 갱신을 줄여 송신량을 낮추는 Iris의 기능이고, 그 대가는 화면 품질이다. 이 효과는 확정 규모에서 한 묶음을 따로 재면 수치로 보일 수 있다.

사용자가 Z3에 `-IrisFlatPriority`를 주고, 기본 우선순위기 묶음을 더해 네 묶음을 재기로 했다(2026-10-19).

## 12. 확정 규모 네 묶음

2026-10-19 21:36\~22:09에 본체 화면에서 연달아 쟀고, 12회 모두 종료 코드 0이었다. 사용자가 무거운 프로그램을 닫았다. 모든 묶음이 ⑧의 인자에 `-MotionLog`를 더한다. 묶음과 라벨은 Z0 `act2-iris-base1`(레거시), Z1 `act2-iris-rg1`(`-ReplicationGraph`, trickle 기본), Z3 `act2-iris1`(`-Iris -IrisFlatPriority`), Z3d `act2-iris-prio1`(`-Iris`, 기본 우선순위기)이다. 배치 기록은 `Saved/Logs/act2-iris-batch.txt`에 있다.

실행마다 확인한 것: 서버 로그의 모델(`Generic`, `LabReplicationGraph`, `Iris`), `lab_config`, `lab_buildings clusters=1 per_cluster=500`, `lab_npcs_near_players clusters=1 per_cluster=50`, `lab_nodes_moved_from_harvest_spot=1`이다. `lab_npc_move_clamped`는 없었다. `lab_saturated_frames`는 모두 0이었다(예: 0/14,328). 그래프 묶음에는 `lab_repgraph_class` 4줄이, Iris 묶음에는 `lab_iris_class` 8줄이 있었다. `lab_iris_prioritizer`는 Z3이 `outer=1 outside=1`, Z3d가 `outer=0.2 outside=0.1`이었다. 서버와 클라이언트 로그에 `Protocol mismatch`, FastArray 미등록 오류, 서브오브젝트 목록 경고, `has custom serialization` 경고는 없었다.

### 12.1 CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | netflush_p99_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-iris-base1-r1` | 1791 | 8.798 | 14.347 | 0 | 4.305 | 8.677 | 9779 | 77 | 0.000 |
| `act2-iris-base1-r2` | 1795 | 9.659 | 17.972 | 1 | 4.794 | 10.425 | 9752 | 77 | 0.000 |
| `act2-iris-base1-r3` | 1795 | 7.950 | 13.719 | 0 | 3.861 | 7.669 | 9781 | 77 | 0.000 |
| `act2-iris-rg1-r1` | 1794 | 7.387 | 11.593 | 1 | 3.207 | 5.704 | 10426 | 77 | 0.000 |
| `act2-iris-rg1-r2` | 1793 | 7.209 | 11.016 | 1 | 3.118 | 5.279 | 10400 | 77 | 0.000 |
| `act2-iris-rg1-r3` | 1794 | 7.116 | 10.789 | 0 | 3.066 | 5.249 | 10448 | 77 | 0.000 |
| `act2-iris1-r1` | 1794 | 6.483 | 10.604 | 1 | 2.316 | 3.949 | 10092 | 0 | 0.000 |
| `act2-iris1-r2` | 1795 | 6.663 | 11.353 | 0 | 2.374 | 4.044 | 10099 | 0 | 0.000 |
| `act2-iris1-r3` | 1794 | 6.429 | 9.660 | 0 | 2.273 | 3.684 | 10100 | 0 | 0.000 |
| `act2-iris-prio1-r1` | 1795 | 6.695 | 11.028 | 1 | 2.389 | 4.149 | 8876 | 0 | 0.000 |
| `act2-iris-prio1-r2` | 1796 | 6.834 | 10.928 | 1 | 2.448 | 4.148 | 8894 | 0 | 0.000 |
| `act2-iris-prio1-r3` | 1797 | 7.659 | 18.103 | 1 | 2.836 | 9.904 | 8898 | 0 | 0.000 |

| 묶음 | work_avg_ms 중앙값(변동 폭) | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn 중앙값(변동 폭) |
| --- | --- | --- | --- | --- |
| Z0 레거시 | 8.798(1.709) | 14.347 | 4.305 | 9779(29) |
| Z1 Replication Graph | 7.209(0.271) | 11.016 | 3.118 | 10426(48) |
| Z3 Iris, 우선순위 1.0 | 6.483(0.234) | 10.604 | 2.316 | 10099(8) |
| Z3d Iris, 기본 우선순위기 | 6.834(0.964) | 11.028 | 2.448 | 8894(22) |

- 계산: Z0 → Z3에서 `work_avg_ms`는 −2.315(−26.3%), Z1 → Z3에서는 −0.726이다. 두 차이 모두 각 묶음의 변동 폭보다 크다. 다만 Z0의 변동 폭 1.709는 이 프로젝트의 레거시 묶음으로는 크다(`r2`가 9.659). 원인은 확인하지 않았다.
- 송신량: Z0 대비 Z1 +6.6%, Z3 +3.3%, Z3d −9.1%다.
- Z0의 송신량 9,779는 포스팅 14 둘째 묶음의 레거시 10,080보다 3% 적다. 이번 빌드에서 Iris 플러그인, `SetupIrisSupport`, 클라이언트에도 주는 인벤토리 조건 인자가 더해졌다. 다른 날의 묶음이라 비교하지 않으며, 원인은 확인하지 않았다.

### 12.2 Timing Insights (중앙값 실행, `export-insights.ps1`, 프레임당 ms)

| 값 | Z0 `base1-r1` | Z1 `rg1-r2` | Z3 `iris1-r1` | Z3d `prio1-r2` |
| --- | ---: | ---: | ---: | ---: |
| 서버 프레임 시간 평균 / P99 | 9.092 / 14.867 | 7.471 / 11.342 | 6.749 / 11.175 | 7.108 / 11.367 |
| `GameNetDriver` Incl / Excl | 4.038 / 2.476 | 2.870 / 0.355 | 2.038 / 0.398 | 2.177 / 0.431 |
| 그 아래 | 클래스 타이머 합 1.562(`LabNpc` 0.891, `LabCharacter` 0.489) | `UReplicationGraph_ServerReplicateActors` 2.515(그 아래 `LabNpc` 1.050, `LabCharacter` 0.531) | `ReplicationSystem_NetUpdate` 1.198, `UDataStreamChannel_Tick` 0.431(8회) | `ReplicationSystem_NetUpdate` 1.291, `UDataStreamChannel_Tick` 0.444(8회) |

Iris에서는 레거시의 클래스 타이머가 남지 않는다(4절의 추론대로). `ReplicationSystem_NetUpdate` 아래의 단계(폴링, 필터, 우선순위, 양자화)는 아직 나누지 않았다.

### 12.3 품질 지표 (`analyze-motion.ps1`)

| 묶음 | 표시 위치 오차 평균(cm) | 표시 속도 오차 평균(cm/s) | NPC 수신 간격 평균 / P99(ms) | 표시 지연(ms) |
| --- | --- | --- | --- | --- |
| Z0 레거시 | 45.20, 45.43, 45.39 | 16.1, 17.0, 16.0 | 133.4\~133.5 / 171.3\~171.4 | 152\~153 |
| Z1 Replication Graph | 45.30, 45.37, 45.41 | 15.7, 15.5, 15.2 | 133.8\~133.9 / 171.0\~171.2 | 152 |
| Z3 Iris, 우선순위 1.0 | 42.90, 43.59, 44.65 | 56.4, 42.5, 22.0 | 133.7\~133.8 / 171.1\~171.2 | 150\~152 |
| Z3d Iris, 기본 우선순위기 | 48.70, 48.82, 48.31 | 114.6, 114.4, 127.5 | 143.7\~143.9 / 406.7\~407.4 | 153 |

- Z3d는 11.1절과 같다. 먼 NPC가 늦게 가서 수신 간격 P99가 407ms이고, 표시 속도 오차가 레거시의 약 7배다.
- Z3은 수신 간격이 레거시와 같은데 표시 속도 오차가 크다. 클라이언트마다 나뉜다. `act2-iris1-r1`에서는 클라이언트 0, 2, 3, 5, 7이 67\~124cm/s, 나머지 셋이 약 15cm/s였다. `r3`에서는 클라이언트 2(68.0)만 높았다. 높은 클라이언트는 표시 위치 오차가 오히려 작다(38.6\~44.1cm, 레거시 약 45cm). 보간 버퍼가 비어 NPC가 멈췄다 가는 것으로 보인다. 작은 규모(클라이언트 2, `tsmall-iris3-r1`)에서는 나타나지 않았다. 원인은 확인하지 않았다. 좋은 클라이언트와 나쁜 클라이언트의 로그는 같다(Iris 경고는 모두 `BroadcastLoadedModulesUpdated` 2줄뿐).

### 12.4 클라이언트에 가는 자원 노드 (1번 클라이언트, t=75s, 자동 스크린샷 `r2`)

| Z0 레거시 | Z1 Replication Graph | Z3 Iris | Z3d Iris |
| ---: | ---: | ---: | ---: |
| 176 | 380 | 112 | 112 |

Iris는 범위를 벗어난 Dormant 자원 노드를 지우므로, 150m 안의 자원 노드만 남는다(3절 ②).

### 12.5 Iris 묶음의 표시 속도 오차: 범위에 들어온 NPC의 첫 상태가 오래되었다

원인을 찾았다. 모션 기록의 수신 기록(시각, NPC, 8비트 서버 프레임 번호)을 클라이언트별로 읽었다.

1. 나쁜 클라이언트에서는 일부 갱신의 도착 시각이 기준선보다 0.5\~4초 늦은 것으로 계산되었다(`act2-iris1-r1`의 클라이언트 0, 2, 3, 5, 7, 늦음 p90 510\~4,108ms). 좋은 클라이언트(1, 4, 6)와 레거시, 그래프의 모든 클라이언트는 p99가 100ms 아래였다. 늦음은 한 번에 크게 뛴 시계에서 계산된 값이다. 갱신 하나가 늦게 온 것이 아니다.
2. 시계가 뛴 순간마다 핸들 ID가 낮은 NPC(맵 전체의 NPC, 예: 10630, 10646, 10666)의 첫 수신이 있었다. 그 첫 수신은 오래된 프레임 번호를 달고 왔고, 바로 다음 수신(1\~4프레임 뒤)은 새 번호를 달고 왔다. 예: 클라이언트 3에서 NPC 10646의 첫 수신은 105, 33ms 뒤의 둘째 수신은 239였다. 다른 NPC들의 최신 번호는 237이었다.
3. 클라이언트의 서버 시계는 8비트 번호를 지금까지 본 가장 큰 번호에 가장 가까운 값으로 펼친다(`LabNpc.cpp`의 `FLabServerClock::Unwrap`, ±128프레임). 그래서 132프레임 묵은 105는 124프레임 앞선 번호로 펼쳐진다. 시계는 도착이 가장 빨랐던 점들로 기준선을 잡는다. 이 점 하나 때문에 기준선이 124프레임(약 4.1초) 당겨지고, 그동안 그리는 프레임이 받은 위치보다 앞서 NPC가 멈춘다(보간 버퍼가 빔). 묵은 번호가 뒤쪽으로 펼쳐진 경우는 버려지므로 해가 없다.
4. 엔진 소스: Iris는 연결의 범위에 든 객체만 폴링한다(`ObjectReplicationBridge.cpp:2038-2069`의 `BuildPollList`, `GetRelevantObjectsInternalIndices`). 폴링은 정해진 간격(`FObjectPollFrequencyLimiter::Update`)이나 dirty, ForceNetUpdate일 때만 한다. 범위에 새로 든 객체를 바로 폴링하는 설정은 찾지 못했다(`net.Iris.*` 콘솔 변수 grep). 그래서 어느 연결의 범위에도 없다가 들어온 NPC는 마지막으로 폴링한 때의 상태로 생성되어 간다. 그 상태의 프레임 번호도 위치도 오래되었다. 레거시는 액터를 보낼 때마다 `PreReplication`을 부르므로 첫 상태가 늘 새것이다(추론. 레거시 묶음에 이런 점프가 없다는 것과 맞는다).
5. 재현: 클라이언트 8, 맵 NPC 300, 자원 노드 100의 작은 규모 `tsmall-iris4-r1`(`-Iris -IrisFlatPriority`)에서 같은 현상이 나왔다. 첫 수신이 앞쪽으로 펼쳐진 클라이언트 0(+44프레임)과 5(+110프레임)만 표시 속도 오차가 128.5, 78.6cm/s였다. 나머지 여섯은 15.8\~16.2였다. 뒤쪽으로 펼쳐진 클라이언트 4(−74)와 6(−40)은 정상이었다. 클라이언트 2개, 맵 NPC 10의 `tsmall-iris3-r1`에서 나타나지 않은 것은 범위에 새로 드는 맵 NPC가 거의 없어서다.

Z3d(기본 우선순위기)의 큰 오차는 11.1절의 늦은 송신이 주된 원인이고, 이 현상도 함께 있을 수 있다(나누지 않았다).

에이전트 의견: 이 현상은 Iris의 동작과 이 프로젝트의 8비트 프레임 시계가 만나서 생긴다. 실제 게임에서도 범위에 들어온 액터가 최대 폴링 간격만큼 오래된 위치로 나타날 수 있다는 점은 본문의 인용구 재료가 된다. 품질 비교를 공정하게 하려면 클라이언트가 NPC의 첫 수신을 시계에 넣지 않게 해야 한다. 첫 수신은 위치만 놓는 데 쓰고, 둘째 수신이 첫 수신보다 앞선 번호이면 첫 스냅숏을 버린다. 레거시와 그래프는 첫 상태가 새것이라 이 변경의 영향이 거의 없다고 본다(확인하지 않음).

사용자가 이 수정과 네 묶음의 재측정을 골랐다(2026-10-19). `ALabNpc::AddSnapshot`을 고쳤다(`67cceff`). 첫 수신은 위치만 놓고, 펼치기의 기준 번호를 바꾸지 않으며, 시계 표본에도 넣지 않는다. 둘째 수신이 첫 수신보다 이전 번호이거나 30프레임 넘게 뒤면 첫 스냅숏을 버린다. `tsmall-iris4`와 같은 조건의 `tsmall-iris5-r1`에서는 오래된 첫 수신이 여전히 왔지만(클라이언트 5 +107, 7 +78), 8개 클라이언트 모두 표시 속도 오차가 15.4\~16.1cm/s였다(`tsmall-iris4-r1`은 0번 128.5, 5번 78.6). 12절의 묶음은 이 수정 전의 빌드라 13절과 비교하지 않는다.

## 13. 확정 규모 네 묶음, 둘째 (보간 수정 뒤)

2026-10-19 22:30\~23:02에 본체 화면에서 연달아 쟀고, 12회 모두 종료 코드 0이었다. 조건과 인자는 12절과 같고 빌드만 `67cceff`다. 라벨은 Z0 `act2-iris-base2`, Z1 `act2-iris-rg2`, Z3 `act2-iris2`, Z3d `act2-iris-prio2`이고, 배치 기록은 `Saved/Logs/act2-iris-batch2.txt`에 있다. 실행마다 12절과 같은 것을 확인했고 모두 맞았다: 모델, 구성 기록, 건축물과 주변 NPC 수, 옮긴 자원 노드 1, 잘라 보낸 NPC 위치 없음, 포화 프레임 0, 우선순위 설정, 오류와 경고 없음.

### 13.1 CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | netflush_p99_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-iris-base2-r1` | 1795 | 7.388 | 12.405 | 0 | 3.566 | 7.192 | 9757 | 77 | 0.000 |
| `act2-iris-base2-r2` | 1796 | 7.316 | 12.229 | 0 | 3.510 | 7.303 | 9791 | 77 | 0.000 |
| `act2-iris-base2-r3` | 1797 | 7.416 | 12.447 | 0 | 3.570 | 7.125 | 9795 | 77 | 0.000 |
| `act2-iris-rg2-r1` | 1795 | 6.755 | 10.897 | 0 | 2.890 | 5.295 | 10452 | 77 | 0.000 |
| `act2-iris-rg2-r2` | 1796 | 6.840 | 11.712 | 0 | 2.912 | 5.239 | 10447 | 77 | 0.000 |
| `act2-iris-rg2-r3` | 1796 | 6.446 | 10.662 | 0 | 2.686 | 5.808 | 10458 | 77 | 0.000 |
| `act2-iris2-r1` | 1795 | 5.959 | 9.307 | 0 | 2.119 | 3.338 | 10113 | 0 | 0.000 |
| `act2-iris2-r2` | 1795 | 5.966 | 9.221 | 0 | 2.138 | 3.363 | 10107 | 0 | 0.000 |
| `act2-iris2-r3` | 1796 | 6.023 | 9.765 | 0 | 2.142 | 3.447 | 10109 | 0 | 0.000 |
| `act2-iris-prio2-r1` | 1795 | 6.159 | 9.827 | 0 | 2.205 | 3.803 | 8892 | 0 | 0.000 |
| `act2-iris-prio2-r2` | 1795 | 6.216 | 10.269 | 0 | 2.229 | 3.869 | 8866 | 0 | 0.000 |
| `act2-iris-prio2-r3` | 1795 | 5.985 | 9.564 | 0 | 2.129 | 3.616 | 8882 | 0 | 0.000 |

| 묶음 | work_avg_ms 중앙값(변동 폭) | work_p99_ms | netflush_avg_ms | out_bytes_per_sec_per_conn 중앙값(변동 폭) |
| --- | --- | --- | --- | --- |
| Z0 레거시 | 7.388(0.100) | 12.405 | 3.566 | 9791(38) |
| Z1 Replication Graph | 6.755(0.394) | 10.897 | 2.890 | 10452(11) |
| Z3 Iris, 우선순위 1.0 | 5.966(0.064) | 9.307 | 2.138 | 10109(6) |
| Z3d Iris, 기본 우선순위기 | 6.159(0.231) | 9.827 | 2.205 | 8882(26) |

- 계산: `work_avg_ms`는 Z0 → Z1이 −0.633(−8.6%), Z0 → Z3이 −1.422(−19.2%), Z1 → Z3이 −0.789(−11.7%)다. 셋 모두 두 묶음의 변동 폭보다 크다. Z3과 Z3d의 차이 0.193은 Z3d의 변동 폭 0.231 안이라 구별되지 않는다.
- 송신량: Z0 대비 Z1 +6.8%, Z3 +3.2%, Z3d −9.3%다.
- 12절보다 모든 묶음이 빠르다(레거시 8.798 → 7.388). 다른 시각의 묶음이라 비교하지 않는다.

### 13.2 Timing Insights (중앙값 실행, `export-insights.ps1`, 프레임당 ms)

| 값 | Z0 `base2-r1` | Z1 `rg2-r1` | Z3 `iris2-r2` | Z3d `prio2-r1` |
| --- | ---: | ---: | ---: | ---: |
| 서버 프레임 시간 평균 / P99 | 7.636 / 12.774 | 7.017 / 11.314 | 6.228 / 9.676 | 6.433 / 10.308 |
| `GameNetDriver` Incl / Excl | 3.346 / 2.001 | 2.654 / 0.343 | 1.888 / 0.370 | 1.940 / 0.382 |
| 그 아래 | `LabNpc` 0.782, `LabCharacter` 0.414 등 | `UReplicationGraph_ServerReplicateActors` 2.310(그 아래 `LabNpc` 0.957, `LabCharacter` 0.493) | `ReplicationSystem_NetUpdate` 1.114, `UDataStreamChannel_Tick` 0.394(8회) | `ReplicationSystem_NetUpdate` 1.163, `UDataStreamChannel_Tick` 0.385(8회) |

### 13.3 품질 지표 (`analyze-motion.ps1`)

| 묶음 | 표시 위치 오차 평균(cm) | 표시 속도 오차 평균(cm/s) | 클라이언트별 최댓값(cm/s) | NPC 수신 간격 평균 / P99(ms) | 표시 지연(ms) |
| --- | --- | --- | --- | --- | --- |
| Z0 레거시 | 45.37, 45.21, 45.41 | 15.4, 15.2, 15.0 | 15.5 | 133.4\~133.5 / 171.1\~171.2 | 152 |
| Z1 Replication Graph | 45.37, 45.19, 45.22 | 15.4, 15.3, 15.1 | 15.6 | 133.7\~133.8 / 171.0\~171.1 | 152 |
| Z3 Iris, 우선순위 1.0 | 45.01, 45.15, 45.00 | 14.9, 15.0, 15.1 | 15.3 | 133.7 / 171.0\~171.2 | 151\~152 |
| Z3d Iris, 기본 우선순위기 | 49.13, 49.25, 49.29 | 99.5, 98.6, 97.4 | 162.9 | 143.8\~143.9 / 407.2\~407.6 | 153\~154 |

Z3은 이제 레거시, 그래프와 구별되지 않는다. Z3d는 먼 NPC를 늦게 보내서(11.1절) 표시 속도 오차가 레거시의 약 6.5배다.

### 13.4 클라이언트에 가는 자원 노드

1번 클라이언트의 t=75s 자동 스크린샷에서 Z3 `act2-iris2-r2`가 112개였다(12절과 같다).

## 14. 시각 자료 후보

| 후보 | 상태 | 들어갈 곳 |
| --- | --- | --- |
| 1번 클라이언트의 내려다보기, 레거시와 Iris(`act2-iris-base2-r2-topdown-04`, `act2-iris2-r2-topdown-04`, t=75s, 자원 노드 176, 112개) | `images/topdown-legacy.png`, `images/topdown-iris.png`로 복사함 | 본문 요약 |
| 원리의 흐름도(레거시는 클라이언트마다 액터 채널로, Iris는 액터마다 한 번 변환하고 클라이언트마다 씀) | Mermaid로 그림. GitHub 미리보기는 확인하지 않음 | 본문 원리 |
| 서버 프레임 시간 평균 차트(레거시, 그래프, Iris) | Mermaid로 그림 | 본문 결과 |
| Timing Insights 캡처: 세 시스템의 `GameNetDriver` 아래 트리(중앙값 실행) | 찍음. 처음 요청은 거절되었고 사용자가 권한을 다시 열어 주었다(2026-10-19). `images/insights-callees-legacy.png`, `-graph.png`, `-iris.png`(번호 상자 포함) | 측정 기록 5절 |
| 영상: Iris 기본 우선순위기에서 먼 NPC가 끊기는 장면과, 우선순위 1.0의 같은 장면 | 찍음(2026-10-20 00:01, 00:04). 사용자가 아래 계획을 허가했다. `Saved/Screenshots/Lab/iris-prio-default.gif`(2.47MB), `iris-prio-flat.gif`(2.49MB)와 각 `-preview.png`. 정지 미리보기로는 차이가 보이지 않는다. 차이는 내려다보기의 100m 밖 빨간 점이 갱신되는 빈도(약 0.33초와 0.13초)뿐이고, 3인칭 화면의 NPC는 20m 안이라 같다. 사용자가 보고 고른다 | 본문 결과의 우선순위 인용구(달라진 쪽 하나) |

영상 계획(2026-10-20 사용자 허가, 그대로 찍음. 배치 기록 `Saved/Logs/visual24-25.txt`):

1. `visual24`: 확정 명령, 2막 요소, ⑧의 인자에 `-Iris`(기본 우선순위기)를 주고 `run-scenario.ps1 -Label visual24 -Runs 1`로 띄운다. 수치는 쓰지 않는다.
2. 측정 구간에 1번(내려다보기)과 2번(3인칭 이동) 클라이언트 창을 `capture-video.ps1 -Region "959,0,1922,541" -RaiseSlots "1,2" -NoMouse -AllowMeasuring -Seconds 10 -Out iris-prio-default.gif`로 찍는다. 화면의 959,0부터 1922,541까지의 영역이고, 찍는 동안 두 클라이언트 창을 맨 위로 올린다.
3. `visual25`: 같은 인자에 `-IrisFlatPriority`를 더해 같은 방법으로 `iris-prio-flat.gif`를 찍는다.
4. 미리보기(`Saved/Screenshots/Lab/<이름>-preview.png`)로 내려다보기의 빨간 점(NPC)이 먼 곳에서 끊겨 움직이는지 확인한다.
