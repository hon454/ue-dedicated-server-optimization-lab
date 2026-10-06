# 장기 백로그

1막(포스팅 0\~4)이 끝난 뒤의 주제다. 작업 중 떠오른 아이디어는 여기에만 적고, 진행 중인 포스팅에 넣지 않는다.

2막은 테스트베드를 한 번 확장해 새 기준선을 잡고 시작한다([ADR-0014](Decisions/0014-act-2-testbed-expansion.md), [2막 설계](Planning/2026-10-03-act-2-design.md), [2막 구현 계획](Planning/2026-10-03-act-2-implementation-plan.md)). 다음 포스팅은 직전 포스팅의 "한계와 다음"이 가리킨 것에서 출발한다.

## 우선순위 순

포스팅 7부터의 순서는 태스크 23에서 사용자가 확정했다(2026-10-05). 기준은 포스팅 6의 최종 구성에서 남은 비용의 크기다. 리플리케이션 시간의 84.7%가 `GameNetDriver` Excl이고, 보낸 액터 데이터의 29.5%가 인벤토리다([포스팅 6 관찰 자료](../Posts/06-three-techniques-again/candidates.md) 3절, 6절). 포스팅 8은 포스팅 7의 결과에 달려 있어서, Excl의 큰 몫이 Consider List가 아니면 포스팅 8부터 다시 고른다. 포스팅마다 겨냥하는 것, 쓰는 요소, 확인하지 않은 것은 2막 설계 6절의 표에 있다(그 표의 번호는 잠정 순서다).

1. **포스팅 5: 테스트베드 확장과 새 기준선.** 플레이어가 모이는 배치, 상태 값, 인벤토리, 건축물을 넣고 규모를 보정한다.
2. **포스팅 6: 세 기법 다시 적용(약식).** 새 기준선에 Relevancy, Dormancy, Net Update Frequency를 차례로 켠다.
3. **포스팅 7: `GameNetDriver` 자체 시간 나누기**(진단, 독립 포스팅): 아래 "작업 중 떠오른 것"의 같은 이름 항목. 세 기법 뒤에 남은 리플리케이션 시간 11.967ms 가운데 10.141ms(84.7%)가 Excl이다(`act2-update-frequency1-r1`).
4. **포스팅 8: 액터를 Consider List에서 빼기**(완료, `post-08-node-update-frequency`): 사용자가 자원 노드의 Net Update Frequency를 2로 낮추기를 골랐다(2026-10-06, [후보 비교](../Posts/08-node-update-frequency/candidates.md)). 고르지 않은 후보는 아래와 같다. 후보는 구역 매니저 + FastArray(노드당 액터 하나에서 구역별 매니저로 전환. 노드별 관련성을 잃는 트레이드오프), Replication Graph의 공간 격자. 아래 "노드를 고려 목록에서 빼기", "휴면 노드가 클라이언트에 남는 문제". 플레이어가 모인 2막에서는 무리에서 150m 밖의 자원 노드가 어느 연결에서도 채널이 열리지 않아 Dormant 상태가 되지 못하고 활성 목록에 남는다(engine-notes.md "휴면 액터와 관련성"). 이것이 Excl의 큰 몫인지는 포스팅 7에서 확인했다. 활성 목록을 도는 두 단계(`Prioritize Actors Time`, `Consider Actors Time`)가 `GameNetDriver` Incl의 79.9%이고, 활성 목록의 93%가 자원 노드다([포스팅 7 측정 기록](../Posts/07-net-driver-breakdown/measurements.md)).
5. **포스팅 9: 인벤토리와 FastArray**: 일반 `TArray` 리플리케이션과 FastArray의 비교. 앞 칸을 지울 때의 전송 바이트와, 고려할 때마다 모든 칸을 비교하는 CPU를 본다(engine-notes.md 자절). 소유자 전용 전송. 세 기법 뒤에 보낸 액터 데이터의 29.5%이고, 한 번 바뀔 때 항목 약 189개를 다시 보낸다(포스팅 6 관찰 자료 6절, 7절).
6. **포스팅 10: NPC 이동의 클라이언트 보간**: 포스팅 4가 남긴 끊김. 아래 "작업 중 떠오른 것"의 "NPC 이동의 클라이언트 보간", "끊김을 수치로 재기". 품질 지표(ADR)를 여기서 만들고 포스팅 11, 12가 쓴다.
7. **포스팅 11: NPC 이동의 `NetSerialize`**: NPC 갱신 한 번 111비트 가운데 92비트가 `ReplicatedMovement`이고, NPC는 1막의 최종 구성에서 액터 비트의 59.5%다(`update-frequency3-r1`, 포스팅 4). `FRepMovement::NetSerialize`는 플래그, 위치, 회전 세 성분, 선속도를 보낸다(`ReplicatedState.cpp:67-117`). 평면 이동에 맞춘 구조체로 바꾸고, 낮춘 정밀도는 보간 포스팅의 품질 지표로 잰다. 92비트의 내역과 Iris가 구조체의 `NetSerialize`를 그대로 쓰는지는 확인하지 않았다(2026-10-03 사용자 제안). 정밀도를 낮춘 결과를 포스팅 10의 품질 지표로 재므로 그 뒤에 둔다. 측정 대상은 이동 하나로 둔다(2026-10-05 사용자 결정). 실무 맥락은 본문에서 설명한다: 엔진의 정밀도 설정이 기본값부터 가장 거친 단계라 더 줄이려면 `NetSerialize`가 필요하다는 것(`LocationQuantizationLevel`, `VelocityQuantizationLevel`이 `RoundWholeNumber`, `RotationQuantizationLevel`이 `ByteComponents`, `ReplicatedState.cpp:34-36`), 범위 안에서 정밀도 낮추기, 필요 없는 성분 빼기, 있을 때만 보내는 필드, 비트 묶기, 구조체가 통째로 보내지는 트레이드오프, 엔진의 예(`FHitResult::NetSerialize`, `HitResult.h:233`, GAS의 `GameplayAbilityTargetTypes.h`, `GameplayPrediction.h`). 상태 값을 한 구조체로 묶는 예와 새 요소(전투 판정)는 넣지 않는다.
8. **포스팅 12: 지연과 패킷 손실에서의 동기화 품질**(진단). 포스팅 10, 11의 품질 지표를 다시 쓴다.
9. **포스팅 13: Replication Graph**: 같은 문제를 그래프 노드로 푸는 방식. 포스팅 8과 같은 문제를 다른 구조로 풀어 비교한다.
10. **포스팅 14: Iris 전환과 실측 비교**: 튜닝된 레거시와 같은 시나리오로 비교. 포스팅 1\~4에서 뺀 "Iris에서는" 섹션의 내용([ADR-0011](Decisions/0011-no-iris-preview-section.md))은 아래 "Iris 전환 때 볼 소스 위치"에 있다.

## 2막 밖

2막에서 다루지 않는다(2막 설계 7절).

- **Test 패키지로 재측정**: 기준선과 세 기법을 모두 적용한 구성을 Test 구성의 패키지(서버, 클라이언트)로 다시 재서, 에디터 빌드에서 본 개선이 출시 빌드에 가까운 조건에서도 유지되는지 확인한다. Shipping은 트레이스가 컴파일되지 않아 Insights로 볼 수 없으므로(엔진 소스: `TraceLog/Public/Trace/Config.h:12-17`, `NetTraceConfig.h:10-16`) Test가 잴 수 있는 가장 Shipping에 가까운 구성이다. 실행을 두 종류로 나눈다.
   - **수치 실행**: CSV 수치와 `GameNetDriver` 타이머는 이 실행에서 얻는다. `GameNetDriver` 타이머는 Test에서도 남는다(`NetDriver.cpp:1174`의 `TRACE_CPUPROFILER_EVENT_SCOPE_TEXT`).
   - **분해 실행**: `-statnamedevents`를 더한 별도 실행(`LaunchEngineLoop.cpp:1759`). Test에서는 `STATS`가 꺼져서 리플리케이션 시간을 클래스별로 나눠 보여 주는 타이머(`LabResourceNode`, `LabNpc`)가 이 인자 없이는 남지 않는다(`DataChannel.cpp:3624`의 `SCOPE_CYCLE_UOBJECT`, `UObjectBaseUtility.h:1073-1098`, `Build.h:311`). 이 인자는 이벤트를 더 기록하므로 이 실행의 수치를 수치 실행과 비교하지 않고, 비율을 보는 데만 쓴다.
   - 서버 타깃(`DSOptLabServer.Target.cs`)을 새로 만들어야 하고, 엔진을 소스로 빌드하므로 서버 타깃과 클라이언트 타깃이 각각 엔진 전체를 컴파일한다.

- **서버 틱 최적화**: 애니메이션, 물리, AI 틱. NPC는 2막에서도 움직이는 리플리케이트 액터의 대역이다.
- **다양한 OS와 플랫폼에서의 프로파일링**(공고 우대사항).

## 순서 미정

2막의 요소로 다룰 수 있지만 포스팅으로 잡지 않은 것이다.

- **Push Model**: 상태 값의 "바뀐 것이 있는지 확인하는 비용". 에디터 타깃은 이미 컴파일돼 있고(`TargetRules.cs:1524`), `Net.IsPushModelEnabled`의 기본값은 false다(`PushModel.cpp:434`). Push Model이 아닌 프로퍼티는 항상 바뀐 것으로 취급된다(`RepLayout.cpp:1506`). 포스팅 6의 최종 구성에서 겨냥할 비용이 작아 순서에서 뺐다(2026-10-05 태스크 23). `LabStateComponent`는 프레임당 0.264ms(`calib2-a-r1`)로, 같은 구성 두 실행의 `work_avg_ms` 차이 0.421(`act2-update-frequency1-r1` 16.732, `r3` 16.311)보다 작다.
- **송신 한도 포화와 `NetPriority`**: 한도에 걸렸을 때 무엇이 미뤄지는지, `NetPriority`로 무엇을 먼저 보낼지. 1막의 최종 구성은 `out_bytes_per_sec_per_conn` 2,400으로 엔진 기본 한도 100,000바이트/초의 2.4%라 포화 조건이 없었다. 포화 조건이 없어 순서에서 뺐다(2026-10-05 태스크 23). 2막의 최종 구성은 연결당 15,926바이트/초로 엔진 기본 한도의 15.9%다(포스팅 6 관찰 자료 6절). 다루려면 시나리오를 바꿔야 한다.
- 캐릭터 무브먼트 리플리케이션 비용(플레이어가 모이는 배치)
- 접속 시 초기 전송 폭주 완화(건축물)
- 건축물의 Dormancy: 플레이어가 배치하는 정적 액터를 깨우는 경로와 초기 전송 비용

## Iris 전환 때 볼 소스 위치

포스팅 1\~4의 "Iris에서는" 섹션에 있던 내용이다(2026-10-02에 옮김, [ADR-0011](Decisions/0011-no-iris-preview-section.md)). 모두 언리얼 엔진 5.8.3 소스에서 읽은 것이고 실행해 보지 않았다.

- **관련성(포스팅 1, 2).** Iris에서는 어떤 객체를 어떤 연결에 보낼지를 필터가 정한다. 엔진 기본 설정에서 액터의 기본 필터는 격자 기반 공간 필터 `UNetObjectGridWorldLocFilter`다(`Engine/Config/BaseEngine.ini:1498`의 `Spatial` 정의와 `1512`의 `DefaultSpatialFilterName=Spatial`). 이 필터의 컬 거리는 레거시와 같은 액터의 `NetCullDistanceSquared`에서 가져온다(`Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/NetActorFactory.cpp:659`). `bAlwaysRelevant`인 클래스에는 공간 필터를 쓰지 않는다(같은 폴더 `EngineReplicationBridge.cpp:242-253`). 소스대로라면 Iris에서도 같은 두 줄이 같은 Always Relevant 기준선을 만들고, 두 줄을 지우면 기본 공간 필터로 돌아간다. 필터 설정 `UNetObjectGridFilterConfig`의 기본값은 격자 칸 200m × 200m(`CellSizeX`, `CellSizeY` 20,000cm)이고, `bUseExactCullDistance`가 true라서 칸 단위가 아니라 객체와 시점 사이의 실제 거리로 판정한다(`Engine/Source/Runtime/Net/Iris/Public/Iris/ReplicationSystem/Filtering/NetObjectGridFilter.h:66-80`, 엔진 `BaseEngine.ini`에는 이 클래스의 설정 섹션이 없다). 비용이 어디에 얼마나 드는지는 모른다.
- **휴면(포스팅 3).** 액터의 `NetDormancy`가 객체 하나의 "휴면을 원함" 비트로 전달되고(`FReplicationSystemUtil::NotifyActorDormancyChange` → `SetObjectWantsToBeDormant`, `Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/ReplicationSystemUtil.cpp:672-687`), 이 비트가 켜진 객체는 프레임마다의 폴링 대상에서 빠진다(`net.Iris.UseDormancyToFilterPolling` 기본값 true, `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ObjectReplicationBridge.cpp:77-80, 2075-2080`). `FlushNetDormancy()`는 그 객체를 한 번 폴링하게 한다(같은 파일 `2083-2098`). 소스대로라면 레거시처럼 연결마다 채널을 거쳐 휴면에 들어가는 구조가 아니라서 포스팅 3에서 남은 비용의 모양이 다를 텐데, 얼마나 다른지는 모른다.
- **업데이트 빈도(포스팅 4).** 액터의 `NetUpdateFrequency`가 객체의 폴링 빈도로 전달된다(`OutParams.PollFrequency = Actor->GetNetUpdateFrequency()`, `Engine/Source/Runtime/Engine/Private/Net/Iris/ReplicationSystem/NetActorFactory.cpp:123`). 폴링 빈도는 "몇 프레임마다 한 번"으로 바뀌어 저장되고(`ConvertFrequencyToFramesBetweenUpdates`, `Engine/Source/Runtime/Net/Iris/Private/Iris/ReplicationSystem/ObjectPollFrequencyLimiter.h:114-128`), 전제하는 갱신 빈도가 30이면 빈도 10은 3프레임에 한 번이다. 소스대로라면 레거시의 난수 지연이 없어 간격이 4프레임이 아니라 3프레임일 텐데, 실행해서 확인하지 않았다.

## 작업 중 떠오른 것

- **`GameNetDriver` 자체 시간 나누기(에디터 빌드).** 기본 트레이스에서는 `GameNetDriver` Excl(기준선 리플리케이션 시간의 37%)에 고려 목록 만들기, 연결마다의 우선순위 정렬, `Connection->Tick`의 송신이 섞여 나뉘지 않는다([Posts/01-baseline/candidates.md](../Posts/01-baseline/candidates.md) 2절). `-statnamedevents`(`LaunchEngineLoop.cpp:1759`)를 더한 별도 실행으로 `STAT_NetConsiderActorsTime`(`NetDriver.cpp:5305`), `STAT_NetPrioritizeActorsTime`(`NetDriver.cpp:5530`), `Stat_NetConnectionTick` 같은 stat을 Insights 타이머로 보면 나눌 수 있다. 측정 조건이 달라지므로 그 실행의 수치는 비교에 쓰지 않고 비율만 본다. 클래스 타이머의 이름이 바뀌는지 먼저 확인한다(소스에서 읽은 추론). 위 "Test 패키지로 재측정"의 분해 실행과 같은 방식이다(2026-10-02, 태스크 10.3).
- **같은 구성의 실행 사이 흔들림의 원인.** `baseline3`에서 느린 실행은 일의 양과 구성 비율이 같고 모든 하위 타이머가 1.12\~1.18배 느렸다(candidates.md 4절). CPU 클럭이나 같은 코어를 쓰는 다른 작업 같은 서버 밖의 요인으로 보이며 확인하지 않았다(2026-10-02, 태스크 10.3).
- **휴면 노드가 클라이언트에 남는 문제.** 휴면으로 채널이 닫힌 노드는 플레이어가 멀어져도 서버가 닫을 채널이 없어 클라이언트에 남는다(`NetDriver.cpp:5877-5889`). `dormancy2-r2`에서 이동하는 클라이언트의 노드 수가 t=75s에 313개였다(적용 전 111개, [Posts/03-dormancy/candidates.md](../Posts/03-dormancy/candidates.md) 3절). 오래 돌아다니면 맵의 노드를 모두 갖게 되고, 멀리 있는 동안 바뀐 상태는 다시 관련성 안에 들어와 채널이 열릴 때 받는다. 해결 후보: 클라이언트가 거리 밖의 휴면 액터를 스스로 지우기, 구역 단위로 묶어 구역이 관련성을 잃을 때 정리하기(위 "자원 노드 구역 매니저"), Replication Graph나 Iris의 필터(2026-10-02, 태스크 12).
- **NPC 이동의 클라이언트 보간.** NPC의 `NetUpdateFrequency`를 10으로 낮추면 NPC 하나의 갱신 간격이 약 134ms(4패킷)가 되고 그 사이 NPC는 약 40cm를 움직인다(`update-frequency3-r1`, [Posts/04-update-frequency/candidates.md](../Posts/04-update-frequency/candidates.md) 2절). `LabNpc`는 받은 위치를 그대로 적용하므로 끊겨 보일 수 있다. 클라이언트에서 받은 위치 사이를 보간하는 것은 포스팅 4에 넣지 않았다(2026-10-02, 태스크 13).
- **끊김을 수치로 재기.** 포스팅 4에서는 시연용 NPC(`-LabShowcaseNpc`)를 60fps로 찍어 프레임마다 화면 위치를 읽었다(위치가 바뀐 간격 평균 46.4ms → 130.6ms, `visual9-r1`, `visual10-r1`). 남은 것: 클라이언트에서 NPC 위치가 바뀐 프레임의 간격을 직접 기록하기, `NetUpdateFrequency` 값을 여러 개(5, 10, 20)로 바꿔 대역폭과 간격을 함께 재기, 다른 거리와 이동 방향에서 보기(2026-10-02, 태스크 13).
- **같은 코드의 두 묶음이 다른 이유.** `dormancy2`(원격 데스크톱 화면)와 `dormancy6`(본체 화면)은 코드가 같은데 `work_avg_ms` 중앙값이 14.409와 13.343이다. 측정 중 선호도 재설정도 포스팅 4의 측정 일곱 묶음 가운데 네 묶음에서 나왔다. 화면 조건, 포그라운드 창, 다른 프로그램 가운데 무엇이 원인인지 확인하지 않았다(2026-10-02, 태스크 13).
- **노드를 고려 목록에서 빼기.** 휴면을 적용해도 `GameNetDriver` 자체 시간은 프레임당 9.43ms가 남았다(`dormancy2-r2`, 적용 전 10.50ms). 노드가 활성 목록에서 빠지려면 모든 연결에서 휴면이어야 하는데(`NetworkObjectList.cpp:348-376`), 실행 중 스폰한 노드는 채널을 연 연결에서만 휴면이 된다(engine-notes.md "휴면 액터와 관련성"). 맵에 놓인 `DORM_Initial` 액터는 고려 목록을 만들 때 바로 빠진다(`NetDriver.cpp:5369-5378`). 노드를 맵에 놓는 방식(에디터 작업 없이 가능한지 확인 필요), 구역 매니저, Replication Graph의 공간 격자로 이 몫을 재 본다(2026-10-02, 태스크 12).
- **시리즈 웹 페이지에 2막의 구성 더하기.** [Site/index.html](../Site/index.html)(UE Dedicated Server, 단계별로 최적화해 보기)은 1막의 네 단계만 다룬다(2026-10-03에 만들었다. 수치의 출처는 [Site/README.md](../Site/README.md)). 2막에서 새 기준선과 새 기법을 측정하면 단계를 더한다. 측정하지 않은 조합은 고를 수 없게 둔다.
- **Dormancy 뒤 클라이언트에 자원 노드가 약 110개 더 있는 이유.** 웹 페이지의 모형은 걸어 다니는 클라이언트의 자원 노드를 약 190개로 세는데 측정값은 301\~313개이고, 제자리에 서 있는 0번 클라이언트도 195개로 150m 안의 80개보다 많다(`dormancy2-r2`, `dormancy6-r1`). 접속 직후 폰이 자리에 놓이기 전의 시점 위치 주변에서 받은 자원 노드가 Dormant 상태로 남은 것으로 추정하고, 확인하지 않았다([Site/README.md](../Site/README.md) "모형과 측정의 대조").
- **NPC가 멈춰 있는 구간.** NPC는 멈추지 않고 움직인다(`LabNpc.cpp`의 `Tick`이 목표에 닿으면 바로 다음 목표를 고른다). 멈추는 구간을 넣으면 Adaptive Net Update Frequency와 NPC Dormancy의 재료가 된다. 2막 기준선에는 넣지 않았다. 포스팅 7\~16 후보가 쓰지 않고, 보정할 값(멈추는 비율)이 하나 늘기 때문이다. 넣으면 시나리오가 달라져 기준선을 다시 잡아야 한다(2026-10-03, [2막 설계](Planning/2026-10-03-act-2-design.md) 3.1).
- **내레이션이 있는 설명 영상.** 기법 하나를 3Blue1Brown 방식으로 설명하는 영상이다. 음성 합성 API 키가 필요해 미뤘다(2026-10-03).
- **서버의 액터 틱 줄이기.** 세 기법 뒤 `GameNetDriver` 밖의 `TickCompletionEvents`(프레임당 2.575ms)는 리플리케이션이 아니라 틱이다. NPC 이동 32.2%, 물리 37.1%, 서버에서 도는 플레이어 캐릭터 8명의 애니메이션 14.4%다(`act2-split1-r3`, [Posts/07-net-driver-breakdown/candidates.md](../Posts/07-net-driver-breakdown/candidates.md) 5절). 후보: 데디케이티드 서버에서 메시 애니메이션을 줄이는 `EVisibilityBasedAnimTickOption`(`SkinnedMeshComponent.h:96`), NPC 틱 간격. 리플리케이션 시리즈의 주제가 아니라 2막 순서에 넣지 않았다(2026-10-05, 태스크 24).
- **자원 노드의 `NetUpdateFrequency` 낮추기.** 자원 노드는 엔진 기본값 100이라 활성 목록에 있는 한 매 프레임 Consider List에 들어간다(`NetDriver.cpp:5319`의 `NextUpdateTime` 검사). 모든 클라이언트에서 휴면이 되지 못한 4,887개가 남아 `Consider Actors Time`과 `Prioritize Actors Time`이 리플리케이션 시간의 79.9%다(`act2-split1-r3`, [Posts/07-net-driver-breakdown/candidates.md](../Posts/07-net-driver-breakdown/candidates.md) 3절, 4절). 예를 들어 2로 낮추면 1초에 두 번쯤만 들어간다. 대가는 다가온 플레이어에게 처음 보이기까지 최대 약 0.5초 늦는 것이고, 채집으로 바뀐 상태는 `FlushNetDormancy`가 앞당기는지 확인해야 한다. 포스팅 8 후보다(에이전트 의견, 재지 않음, 2026-10-05 태스크 24).
