# 엔진 소스 확인 기록 (UE 5.8.3)

- 확인일: 2026-10-01
- 엔진: `G:\Epic Games\UE_Source` (git 태그 `5.8.3-release`, `Engine/Build/Build.version`의 5.8.3)
- 경로는 엔진 루트 기준이다. 줄 번호는 이 태그의 것이다.
- 구현 계획 태스크 2.5의 결과다. 테스트베드 포스팅의 "엔진에서 확인한 것"과 이후 코드의 근거로 쓴다.
- 절은 숫자로 부른다("engine-notes.md 8절"). 새 절은 마지막 번호 다음 번호로 더한다. 2026-10-06까지는 가나다 순 이름(가절, 나절, …)이었고, 승인된 ADR과 지난 Worklog는 그 이름으로 부른다. 옛 이름과 지금 번호는 가=1, 나=2, 다=3, 마=4, 바=5, 사=6, 아=7, 자=8, 차=9, 카=10, 라=11이다(라는 맨 끝에 있던 절이다).

## 0. 프로젝트 구성

계획의 초안은 `Game/` 폴더 아래의 `ServerLab` 프로젝트와 접두사 `SL`을 가정했다. 실제 구성은 아래와 같고, 2026-10-01에 설계 문서 4절과 구현 계획을 여기에 맞춰 고쳤다.

| 항목 | 값 |
| --- | --- |
| 프로젝트 | `DSOptLab.uproject` (레포 루트가 프로젝트 폴더) |
| 모듈과 빌드 타깃 | `DSOptLab` (`Source/DSOptLab/`, `DSOPTLAB_API`), `DSOptLabEditor` |
| 게임 모드 경로 | `/Script/DSOptLab.LabGameMode` |
| 접두사 | `Lab`. 클래스(`ALabResourceNode`), 실행 인자(`-LabNodes=`), 폴더(`Saved/LabMetrics/`, `Saved/Screenshots/Lab/`), 로그 카테고리(`LogLabMetrics`), 북마크(`Lab_MeasureStart`) |
| 엔진 연결 | `EngineAssociation`이 `UE_DSOptLab`(이 PC의 레지스트리에서 `G:\Epic Games\UE_Source`). 스크립트는 `Scripts/common.ps1`에서 같은 값을 찾아 쓴다. 런처 설치본 `G:\Epic Games\UE_5.8`과 섞어 쓰면 같은 `Binaries/`에 번갈아 빌드하게 되므로 쓰지 않는다 |
| 문서 | `Docs/STATUS.md`, `Docs/Guides/`, `Docs/Reference/`(2026-10-03까지 `Docs/Planning/`) |
| 포스팅 | `Posts/NN-이름/` |

`.gitignore`는 레포를 만들 때 들어간 GitHub의 Unreal 템플릿(루트 기준)을 그대로 쓰고 `.idea/`, `*.slnx`, `*.utrace`만 더했다.

## 1. 기본값과 이름

| 항목 | 찾은 값 | 위치 | 결론 |
| --- | --- | --- | --- |
| 서버 틱 기본값 | `NetServerMaxTickRate=30` | `Engine/Config/BaseEngine.ini:1867` (`[/Script/OnlineSubsystemUtils.IpNetDriver]`) | 기억값 30과 같다. 틱 예산 1000 ÷ 30 = 33.3ms |
| 서버 틱의 적용 | Dedicated Server면 `MaxTickRate = Clamp(NetDriver->GetNetServerMaxTickRate(), 1, 1000)` | `Engine/Source/Runtime/Engine/Private/GameEngine.cpp:1719-1765` (`UGameEngine::GetMaxTickRate`) | 실행 중 적용값은 태스크 7.4에서 프레임 수로 확인한다 |
| 컬 거리 기본값 | `SetNetCullDistanceSquared(225000000.0f)` | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` | 기억값과 같다. √225,000,000 = 15,000cm = 150m |
| 업데이트 빈도 기본값 | `SetNetUpdateFrequency(100.0f)`, `SetMinNetUpdateFrequency(2.0f)` | `Actor.cpp:295-296` | 기억값과 같다 |
| 설정자 | `SetNetUpdateFrequency`, `SetMinNetUpdateFrequency`, `SetNetCullDistanceSquared`, `SetNetDormancy`, `FlushNetDormancy`, `SetReplicatingMovement` 모두 있음. 멤버 `NetCullDistanceSquared`, `NetUpdateFrequency`, `MinNetUpdateFrequency`의 직접 접근은 5.5부터 사용 중단 | `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:898-910, 3174, 3178, 4557, 4624, 4636, 4648` | 계획의 코드 그대로 쓴다. `NetDormancy`(869줄)는 사용 중단 표시가 없는 public 멤버라 생성자에서 직접 대입할 수 있다 |
| 연결당 송신 한도(엔진 기본값) | `ConfiguredInternetSpeed=100000`, `ConfiguredLanSpeed=100000`, `MaxClientRate=100000`, `MaxInternetClientRate=100000` (바이트/초) | `BaseEngine.ini:1839-1840` (`[/Script/Engine.Player]`), `1860-1861` (`[/Script/OnlineSubsystemUtils.IpNetDriver]`) | 기본 한도는 연결당 초당 100,000바이트다. 계획 8.4a의 사전 추정(135\~180KB/s)보다 낮으므로 기준선이 포화될 가능성이 높다 |
| 한도가 정해지는 과정 | 클라이언트가 자기 `ConfiguredInternetSpeed`를 `NMT_Netspeed`로 보내고, 서버가 `Clamp(Rate, 1800, NetDriver->MaxClientRate)`로 받는다. LAN이 아니면 서버의 `MaxClientRate`는 `MaxInternetClientRate`로 낮춰진다 | `Engine/Source/Runtime/Engine/Private/NetConnection.cpp:588`, `PendingNetGame.cpp:396`, `World.cpp:7465-7473`, `World.cpp:7987-7990` | **한도를 올리려면 세 키를 모두 올려야 한다**: `[/Script/Engine.Player] ConfiguredInternetSpeed`(클라이언트가 읽는다), `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`와 `MaxInternetClientRate`(서버가 읽는다). 서버 로그에 `Client netspeed is N`이 찍힌다 |
| `OutTotalBytes` | `int32`. `FlushNet`에서 패킷마다 `SendBuffer.GetNumBytes() + PacketOverhead`를 더한다 | `Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h:571`, `NetConnection.cpp:2562, 2586` | 패킷 헤더와 오버헤드를 포함한 송신 바이트의 누계다. `int32`라서 연결 하나가 약 2.1GB를 넘기면 넘친다. 10MB/s로 110초를 보내도 1.1GB라 이 시나리오에서는 넘치지 않는다 |
| `ActorChannelsNum()` | `ActorChannels.Num()` | `NetConnection.h:751` | 계획 그대로 쓴다 |
| `CurrentNetSpeed` | public `int32` | `NetConnection.cpp:588-597` | 계획 그대로 쓴다 |
| `IsNetReady` | `bool IsNetReady() const`. 인자를 받는 `IsNetReady(bool bSaturate)`는 5.6부터 사용 중단. 판정은 `QueuedBits + SendBuffer.GetNumBits() <= 0` | `NetConnection.h:1085-1089`, `NetConnection.cpp:2731-2751` | **계획과 다르다.** `IsNetReady(false)` 대신 `IsNetReady()`를 쓴다 |
| `QueuedBits` | 보낼 때 `PacketBytes * 8`을 더하고, 연결 틱마다 `CurrentNetSpeed × 경과 시간 × 8`을 뺀다 | `NetConnection.cpp:2574, 5122-5131` | 양수면 한도 초과다 |
| 채널 수 상한 | `DefaultMaxChannelSize(32767)`, 콘솔 변수 `net.MaxChannelSize`(기본 0이면 앞의 값) | `NetConnection.cpp:80, 405, 450-454` | 출발값(약 5,310개)은 상한 안이다. 노드 20,000개까지도 안이다 |
| 종료 코드를 주는 종료 | `static void RequestExitWithStatus(bool Force, uint8 ReturnCode, const TCHAR* CallSite = nullptr)` | `Engine/Source/Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h:1096` | 계획 그대로 쓴다 |

## 2. 동작 순서와 방식

### 한 프레임 안의 순서

`FEngineLoop::Tick` (`Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp`):

1. `FCoreDelegates::OnBeginFrame` (5682줄)
2. `GEngine->UpdateTimeAndHandleMaxTickRate()` (5701줄). 틱 속도 제한의 대기(`FPlatformProcess::SleepNoStats`)가 여기서 일어난다(`Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp:2980, 3058-3123`).
3. `GEngine->Tick()` (5859줄). 이 안에서 `UWorld::Tick` (`Engine/Source/Runtime/Engine/Private/LevelTick.cpp`):
   1. `FWorldDelegates::OnWorldTickStart` (1522줄)
   2. `BroadcastTickDispatch` (1574줄, 수신 처리)
   3. 액터 틱
   4. `FWorldDelegates::OnWorldPostActorTick` (1906줄)
   5. `BroadcastTickFlush` (1929줄). `UNetDriver::TickFlush` → `ServerReplicateActors` (`NetDriver.cpp:1168, 1224-1230`)
   6. `BroadcastPostTickFlush` (1936줄)
   7. `FWorldDelegates::OnWorldTickEnd` (2061줄)
4. `FCoreDelegates::OnEndFrame` (6127줄)

결론: 계획의 가정과 같다. `work`(1522줄 → 6127줄)는 틱 속도 제한의 대기를 포함하지 않는다. `netflush`(1906줄 → 6127줄) 안에 `TickFlush`가 있다. 태스크 6의 델리게이트를 바꾸지 않는다. 둘 다 `GEngine->Tick` 이후 `OnEndFrame`까지의 엔진 루프 나머지 작업도 포함한다.

### 실제로 쓰는 리플리케이션 시스템

- 선택 로직: `UEngine::WillNetDriverUseIris` (`UnrealEngine.cpp:14798-14884`). `bConfigCanUseIris && UE::Net::ShouldUseIrisReplication()`가 기본이고, 실행 인자 `-UseIrisReplication=0/1`이 모든 것을 덮어쓴다.
- `BaseEngine.ini:346`에 `+IrisNetDriverConfigs=(NetDriverDefinition=GameNetDriver, bCanUseIris=true)`가 있지만, 콘솔 변수 `net.Iris.UseIrisReplication`의 코드 기본값이 0이고(`Engine/Source/Runtime/Net/Iris/Private/Iris/IrisConfig.cpp:15-16`) `Engine/Config/Base*.ini`와 이 프로젝트의 `Config/`에 이 값을 켜는 줄이 없다. Iris 플러그인도 `EnabledByDefault: false`다(`Engine/Plugins/Experimental/Iris/Iris.uplugin:13`).
- Replication Graph: `ReplicationDriverClassName`이 비어 있으면 쓰지 않는다(`NetDriver.cpp:1822-1827`). 엔진과 프로젝트 설정 어디에도 값이 없다.
- 결론: 기본이 레거시다. `Config/DefaultEngine.ini`에 아무것도 넣지 않았다(태스크 2.7).
- 서버 로그 확인법: `LogNet: InitBase GameNetDriver (NetDriverDefinition GameNetDriver) using replication model Generic` (`NetDriver.cpp:1939`, 이름은 `GetReplicationModelName`, `NetDriver.cpp:2369-2383`). `Generic`이 레거시, `Iris`가 Iris, 그 밖의 이름은 Replication Graph 클래스다.

### 관련성 판정의 기준 위치

- `FNetViewer`는 `ViewTarget->GetActorLocation()`으로 시작해 `ViewingController->GetPlayerViewPoint(ViewLocation, ViewRotation)`로 덮어쓴다(`NetDriver.cpp:5108-5128`).
- `APlayerController::GetPlayerViewPoint`는 카메라 캐시가 한 번이라도 갱신됐으면 `PlayerCameraManager->GetCameraViewPoint`를, 아니면 뷰 타깃 위치를 준다(`PlayerController.cpp:1001-1031`).
- 클라이언트는 폰이 움직일 때 `bShouldSendClientSideCameraUpdate`를 켜고(`PlayerController.cpp:5668-5676`, 클라이언트의 `bUseClientSideCameraUpdates`가 참일 때만), `APlayerCameraManager::UpdateCamera`가 `ServerUpdateCamera` RPC로 카메라 위치를 보낸다(`PlayerCameraManager.cpp:824-863`).
- 서버의 `ServerUpdateCamera_Implementation`은 **서버 쪽** `PlayerCameraManager->bUseClientSideCameraUpdates`가 거짓이면 무시하고, 참이면 받은 위치로 카메라 캐시를 채운다(`PlayerController.cpp:1836-1870`). 기본값은 참이다(`PlayerCameraManager.cpp:64`).
- 서버 쪽 플래그가 거짓이면 서버가 자기 뷰 타깃(폰)으로 직접 카메라를 계산한다(`PlayerCameraManager.cpp:812-818`, `LevelTick.cpp:1847`).
- `AActor::IsNetRelevantFor`는 `bAlwaysRelevant`면 참, 아니면 마지막에 `IsWithinNetRelevancyDistance(SrcLocation)`로 판정한다(`Engine/Source/Runtime/Engine/Private/ActorReplication.cpp:388-419`). `bUseDistanceBasedRelevancy`의 기본값은 참이다(`GameNetworkManager.cpp:54`).

결론: **계획과 다르다.** 계획의 태스크 5.2는 클라이언트의 `TickTopDown`에서만 플래그를 껐다. 그러면 플래그를 끄기 전에(스폰 직후 낙하 중에) 이미 보낸 카메라 위치가 서버의 캐시에 남을 수 있고, 서버 쪽 플래그는 참이라 서버가 캐시를 스스로 갱신하지 않는다. 그래서 `ALabPlayerController::SpawnPlayerCameraManager`를 재정의해 서버와 클라이언트 양쪽에서, 모든 플레이어에 대해 `bUseClientSideCameraUpdates = false`로 둔다. 서버는 폰을 뷰 타깃으로 계산한 3인칭 카메라 위치(폰 뒤 수 미터)를 기준으로 판정하고, 내려다보기 카메라는 서버에 전달되지 않는다. 모든 클라이언트가 같은 방식이라 클라이언트마다 기준이 다르지 않다. 테스트베드 포스팅에 이 사실을 적는다.

### 그 밖의 시그니처

| 항목 | 5.8.3 | 위치 | 결론 |
| --- | --- | --- | --- |
| 월드 서브시스템 | `virtual bool ShouldCreateSubsystem(UObject* Outer) const override`, `virtual void OnWorldBeginPlay(UWorld& InWorld)`, `virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const`(protected) | `Engine/Source/Runtime/Engine/Public/Subsystems/WorldSubsystem.h:34, 43, 66` | 계획 그대로 |
| 월드 틱 델리게이트 | `FOnWorldTickStart`, `FOnWorldPostActorTick` 모두 `(UWorld*, ELevelTick, float)` | `Engine/Source/Runtime/Engine/Classes/Engine/World.h:4516-4527` | 계획 그대로 |
| 디버그 점 | `DrawDebugPoint(const UWorld*, FVector const& Position, float Size, FColor const&, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0)` | `Engine/Source/Runtime/Engine/Public/DrawDebugHelpers.h:24` | 계획 그대로. `SDPG_Foreground`를 마지막 인자로 준다 |
| 스크린샷 | `FScreenshotRequest::RequestScreenshot(const FString& InFilename, bool bInShowUI, bool bAddFilenameSuffix, ...)` | `Engine/Source/Runtime/Engine/Public/UnrealClient.h:219` | 계획 그대로 |

### 업데이트 빈도의 스케줄링

- `ServerReplicateActors_BuildConsiderList`가 `World->TimeSeconds <= ActorInfo->NextUpdateTime`인 액터를 건너뛴다(`NetDriver.cpp:5319-5323`).
- 다음 시각은 `NextUpdateTime = TimeSeconds + RandDelay + NextUpdateDelta`이고, `NextUpdateDelta`는 적응형이 꺼져 있으면 `1 / NetUpdateFrequency`다(`NetDriver.cpp:5420-5425`). `RandDelay`는 `0`에서 `ServerTickTime` 사이의 난수다.
- `net.UseAdaptiveNetUpdateFrequency`의 기본값은 0이다(`NetDriver.cpp:523-526`).

결론: 정적 노드는 적응형 감소를 받지 않는다. 기본 빈도 100Hz는 서버 틱 30Hz보다 높으므로, 노드와 NPC 모두 매 틱(또는 `RandDelay` 때문에 한 틱 건너) 고려 대상이 된다. 포스팅 4에서 NPC 빈도를 10으로 낮추면 고려 횟수가 약 3분의 1이 된다(30Hz 기준 계산값).

실행에서 확인(2026-10-02, `dormancy6`, `update-frequency3`): 30Hz에서 빈도 100은 다음 고려 시각이 10\~43.3ms 뒤라 평균 1.3프레임에 한 번(난수가 0.7보다 작으면 다음 프레임), 빈도 10은 100\~133.3ms 뒤라 4프레임에 한 번이다. 계산값 비율 0.325가 `LabNpc` 타이머 호출 비율 0.324(연결당 프레임당 5.50 → 1.78), `Connection 0`의 `LabNpc` 전송 횟수 비율 0.324(7,622 → 2,472)와 맞았다. NPC 하나(NetId 1306)는 4패킷마다, 약 134ms에 한 번 실렸다. `NetUpdateFrequency` 10은 초당 10번이 아니라 약 7.5번이다.

### 휴면 액터와 관련성

- `ServerReplicateActors_PrioritizeActors`는 채널이 없는 액터에만 관련성을 먼저 검사하고(`NetDriver.cpp:5580-5593`), 그 뒤 이 연결에 대해 휴면이면 건너뛴다(`5618-5624`, `IsActorDormant`는 `ActorInfo->DormantConnections.Contains`, `5499-5503`).
- 휴면 진입은 채널이 있어야 한다(`ShouldActorGoDormant`, `NetDriver.cpp:5506-5526`). 실행 중 스폰한 액터는 먼저 채널이 열려 초기 상태가 전송된 뒤에 휴면에 들어간다.
- 관련성을 잃은 액터의 채널을 닫는 코드는 `Channel != NULL`일 때만 동작한다(`NetDriver.cpp:5877-5889`).

결론: 계획의 가정과 같다. 휴면으로 채널이 닫힌 액터는 관련성 밖으로 나가도 서버가 닫을 채널이 없어서 클라이언트에 남는다. 포스팅 3에서 내려다보기 화면에 지나온 길을 따라 점이 남는 것이 이 동작이다. 실행으로는 포스팅 3에서 확인한다.

활성 목록에서 빠지는 조건(2026-10-02, 포스팅 3을 시작하며 확인):

- 고려 목록은 활성 목록만 돈다(`ServerReplicateActors_BuildConsiderList`, `NetDriver.cpp:5315`). 액터가 활성 목록에서 빠지는 것은 휴면인 연결 수가 전체 연결 수와 같아질 때다(`FNetworkObjectList::MarkDormant`, `NetworkObjectList.cpp:348-376`).
- 연결별 휴면 등록(`NotifyActorFullyDormantForConnection` → `MarkDormant`, `NetDriver.cpp:7096-7098`)은 액터 채널에서만 불린다(`DataChannel.cpp:2354, 2461, 2728`). 채널이 없는 연결은 휴면으로 등록되지 않는다.
- 예외는 맵에 놓인 `DORM_Initial` 액터다. 고려 목록을 만들 때 바로 활성 목록에서 뺀다(`IsDormInitialStartupActor`, `NetDriver.cpp:5369-5378`). 이 프로젝트의 노드는 실행 중에 스폰하므로 해당하지 않는다.
- 연결별 처리에서 채널이 없는 액터의 거리 검사(`5580-5593`)는 그 연결에 대한 휴면 검사(`5618-5624`)보다 앞에 있다. 이미 휴면에 들어가 채널이 닫힌 액터도 활성 목록에 남아 있는 동안은 프레임마다 거리 검사를 받는다.

결론: **포스팅 1 "선택"과 포스팅 2 "한계와 다음"의 기대(휴면이 노드 5,001 × 연결 8의 검사를 줄인다)와 다르다.** 관련성을 적용한 뒤에는 한 노드에 채널을 여는 연결이 가까이 있는 몇 개뿐이라, 거의 모든 노드가 모든 연결에서 휴면이 되지 못하고 활성 목록에 남는다. 휴면으로 줄어드는 것은 채널이 열려 있던 노드의 프레임마다의 직렬화이고, 거리 검사는 남는다고 본다.

실행에서 확인한 것(`dormancy2-r1`\~`r3`, 2026-10-02): `LabResourceNode` 타이머 호출이 60초에 546\~609번으로 줄었고(적용 전 약 100만 번), `GameNetDriver` Excl은 프레임당 9.32\~10.60ms로 남았다(적용 전 10.43\~10.80ms). 클라이언트의 노드 수는 이동하는 클라이언트에서 t=75s에 313개였다(적용 전 111개). 값은 [Posts/03-dormancy/candidates.md](../../Posts/03-dormancy/candidates.md)에 있다.

## 3. 템플릿에서 남긴 것 (태스크 2.6, 2026-10-01 정리)

5.8의 TPP 템플릿은 `Variant_Combat`, `Variant_Platforming`, `Variant_SideScrolling` 코드와 콘텐츠, 템플릿 레벨을 함께 만든다. 쓰지 않는 것을 지우고 남긴 코드에 접두사 `Lab`을 붙였다.

| 남긴 것 | 내용 |
| --- | --- |
| `Source/DSOptLab/LabCharacter.h/.cpp` | 템플릿의 `ADSOptLabCharacter`를 `ALabCharacter`로 이름만 바꿨다. `BP_LabCharacter`의 부모 클래스다 |
| `Content/Blueprints/BP_LabCharacter` | 플레이어 폰. `ALabGameMode`가 경로 `/Game/Blueprints/BP_LabCharacter`로 읽는다. 템플릿의 `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`를 옮기고 이름을 바꿨다(2026-10-03) |
| `Content/Characters/Mannequins/` | 위 블루프린트가 참조하는 메시, 머티리얼, 텍스처, 릭, 기본 이동 애니메이션 46개 |
| `Content/Input/` | 입력 액션 4개, 매핑 2개, 터치 인터페이스 1개 |
| `Content/Maps/L_Lab` | 태스크 1.4의 맵 |

- 남길 에셋은 `L_Lab`, `BP_ThirdPersonCharacter`(지금의 `BP_LabCharacter`), `IMC_Default`, `IMC_MouseLook`에서 에셋 파일 안의 `/Game/...` 경로 문자열을 따라가 정했다. 754개 중 55개(89.6MB)가 남았다. 제거 뒤 `smoke5-r1`이 종료 코드 0으로 끝났고 서버와 클라이언트 로그에 로드 실패가 없었다.
- **리다이렉트(2026-10-03에 없앰).** 태그 `post-04-update-frequency`까지의 `BP_ThirdPersonCharacter`는 부모 클래스를 템플릿 원본 이름 `/Script/TP_ThirdPerson.TP_ThirdPersonCharacter`로 저장하고 있었고, `Config/DefaultEngine.ini`의 `ActiveGameNameRedirects`와 `ActiveClassRedirects`가 이것을 `ALabCharacter`로 이었다. 이름을 `BP_LabCharacter`로 바꾸며 다시 저장한 뒤로는 에셋이 `/Script/DSOptLab.LabCharacter`를 직접 저장한다(에셋의 이름 표에서 확인). 그래서 리다이렉트 세 줄을 지웠다. 지운 뒤 `refactor-b-r1`이 종료 코드 0으로 끝났다.
- **에셋 이름을 에디터 없이 바꾸는 법.** `UnrealEditor-Cmd.exe <.uproject> -run=pythonscript -script=<파일>`로 `unreal.EditorAssetLibrary.rename_asset(옛 경로, 새 경로)`와 `save_asset`을 부른다. 옛 파일은 디스크에 그대로 남으므로(리디렉터 파일은 생기지 않았다) 참조하는 에셋이 없는지 확인한 뒤 직접 지운다. 그래프 함수 이름(`ExecuteUbergraph_<이름>`)은 `unreal.BlueprintEditorLibrary.compile_blueprint` 뒤에 다시 저장해야 바뀐다. 이 커맨드릿은 에디터처럼 `DefaultEngine.ini`에 `AndroidFileServerRuntimeSettings` 섹션을 다시 써넣는다.
- 템플릿의 `ADSOptLabPlayerController`와 `ADSOptLabGameMode`, 그 블루프린트는 지웠다. 템플릿 컨트롤러가 블루프린트에서 지정하던 입력 매핑(`IMC_Default`, `IMC_MouseLook`)은 `ALabPlayerController`가 생성자에서 읽어 `SetupInputComponent`에서 등록한다. 터치 조작 위젯은 옮기지 않았다.
- 모듈 의존성에서 `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`를, `.uproject`에서 `StateTree`, `GameplayStateTree` 플러그인을 뺐다.

## 4. 첫 실행에서 확인한 것 (태스크 7.3\~7.7, 2026-10-01)

조건: 클라이언트 2, 노드 100 + 검증용 1, NPC 10, 준비 20초, 측정 30초, 트레이스 없음. 라벨 `smoke2-r1`, `smoke3-r1`, `smoke4-r1` 세 실행이 종료 코드 0으로 끝났다. 수치는 동작 확인용이고 결과로 쓰지 않는다.

| 확인 항목 | 결과 | 근거 |
| --- | --- | --- |
| 리플리케이션 시스템 | 레거시 | 서버 로그 `LogNet: InitBase GameNetDriver (NetDriverDefinition GameNetDriver) using replication model Generic` |
| `net_speed` | 100000 | 서버 로그 `Client netspeed is 100000`, CSV의 `net_speed`. 엔진 기본값 그대로다 |
| 서버 틱 적용값 | 30Hz | 측정 30초에 `frames`가 898, 897, 898. 30 × 30 = 900 |
| 열린 액터 채널 수 | 연결당 118 | 노드 101 + NPC 10 = 111. 나머지 7개는 폰 2, 그리고 컨트롤러, 플레이어 상태, 게임 상태 같은 엔진 액터로 보인다(내역은 확인하지 않았다) |
| 화면 위 글자 | `nodes=101 npcs=10` | `Saved/Screenshots/Lab/smoke3-r1-tpp-01.png`, `smoke3-r1-topdown-01.png` |
| 내려다보기 화면의 점 | 초록 점이 찍힌다 | `smoke3-r1-topdown-01.png` |
| 코어 배정 | 서버 255, 클라이언트 4294967040 | `smoke4` 측정 구간 중 `Get-Process`로 읽은 값 |

실행하면서 고친 것:

- **내려다보기 뷰 타깃이 폰으로 되돌아간다.** 클라이언트에서만 `SetViewTarget`을 부르면 서버의 `ClientSetViewTarget`이 폰으로 되돌린다(`PlayerController.cpp:2832-2848`). 클라이언트의 `PlayerCameraManager->bClientSimulatingViewTarget = true`로 막고, `bAutoManageActiveCameraTarget = false`로 두고, 매 틱 뷰 타깃을 확인한다. `smoke2`의 내려다보기 스크린샷은 3인칭 화면이었고 `smoke3`부터 내려다보기 화면이다.
- **프로세스 선호도가 시작 중에 되돌아간다.** 실행 직후 설정한 선호도가 측정 구간에는 전체 코어(4294967295)로 돌아가 있었다(`smoke1`, `smoke3`에서 관찰). 엔진 소스에서 `SetProcessAffinityMask`를 부르는 곳은 `-processaffinity` 인자를 줄 때뿐이라(`WindowsPlatformProcess.cpp:184`, `LaunchWindows.cpp:223-232`) 원인은 찾지 못했다. 시작이 끝난 뒤에 설정하면 유지되므로, 스크립트가 서버를 기다리는 동안 2초마다 다시 읽어 달라져 있으면 다시 설정한다. `smoke4`에서 마지막 재설정은 측정 시작 약 20초 전이었다.
  - **원인(2026-10-05 확인).** 엔진은 스레드 선호도를 `SetThreadGroupAffinity`로 그룹의 모든 코어에 설정한다(`WindowsRunnableThread.cpp:140-142`, `FRunnableThreadWin::SetThreadAffinity`. 프로세스 선호도를 `-processaffinity=`로 준 실행에서만 건너뛴다, 129\~132줄). 이 PC의 Windows(10.0.26300)에서는 이 요청이 프로세스 마스크 밖이어도 성공하고, 프로세스 선호도가 전체 코어로 넓어진다. PowerShell에서 재현했다: 프로세스를 `FFFFFF00`으로 두고 새 스레드가 `FFFFFFFF`를 요청하면 성공하고 프로세스가 `FFFFFFFF`가 된다. 스레드를 만들지 않는 프로세스는 40초 동안 되돌아가지 않았다. 프로세스를 `JOB_OBJECT_LIMIT_AFFINITY`를 건 Job 객체에 넣으면 같은 요청이 오류 31로 실패하고 스레드와 프로세스 모두 `FFFFFF00`에 머문다. 엔진은 `SetThreadAffinity failed` 경고를 남기고 계속 돈다(`tsmall-job1-r1`). 스크립트는 이 방법을 쓴다([ADR-0016](../Decisions/0016-affinity-through-job-objects.md)).
- **클라이언트가 시작 직후 엔진 내부 단언으로 죽을 수 있다.** `smoke1`에서 0번 클라이언트가 첫 프레임 전에 `Assertion failed: RefCount.load(std::memory_order_relaxed) == 0`(`Engine/Source/Runtime/Core/Private/Async/InheritedContext.cpp:130`, DDC IO 스레드)로 종료했다. 이 프로젝트의 코드가 실행되기 전이다. 네 번 실행 중 한 번 일어났다. 콜스택은 `DDC IO ThreadPool #1` 스레드의 `FMemoryCacheStore::Get` 아래 mimalloc이고 `HttpConnectionPool` 스레드도 함께 죽었다(`Saved/Logs/client0-smoke1-r1.log:1461-1509`). 130줄은 참조 중인 확장 데이터를 해제할 때 걸리는 검사라 DDC 요청 경로의 수명 경합으로 보이지만 추정이다. 크래시 뒤 2.2초 만에 프로세스가 끝났다. 지금까지 약 27번의 프로세스 실행 중 1번이다(`smoke1`\~`smoke7`과 수동 실행). 스크립트는 시작 신호 전에 죽은 클라이언트를 같은 인자로 다시 띄우고(실행당 최대 3번), 시작 신호 뒤에 죽으면 실패로 처리한다. 다시 띄우는 경로는 아직 실행으로 확인하지 않았다.
- **`-server`, `-game` 실행에서 Python 시작 스크립트가 오류를 낸다.** `DSOptLab.uproject`가 켠 `AllToolsets`(에디터 전용 실험 플러그인 묶음)의 `Content/Python/init_unreal.py`가 시작할 때 실행되는데, 이 스크립트들이 쓰는 `unreal.ToolsetDefinition`, `unreal.AgentSkill`, `unreal.PythonTestRunner`는 `ToolsetRegistry` 모듈(`ToolsetRegistry.uplugin`, `Type: Editor`)에 있어 `-server`, `-game`에서는 로드되지 않는다. 그래서 실행마다 `LogPython: Error`가 58줄 남았다(`smoke5`, `smoke6`의 서버와 클라이언트 로그). 오류는 시작할 때만 나고 이 프로젝트의 코드와는 관계없다. 이 프로젝트는 Python을 쓰지 않는다(`Source`, `Config`, `Content`에 Python 관련 내용 없음). 사용자가 `AllToolsets`와 `ModelContextProtocol`을 에디터에서 쓰고 있어(2026-10-01 사용자 확인) `.uproject`는 그대로 두고, 실행 스크립트의 서버와 클라이언트 인자에 `-DisablePython`(`PythonScriptPlugin.cpp:116`, `IsPythonEnabled`)을 넣었다. `smoke7`에서 세 로그 모두 `LogPython: Error`가 0줄이다. 스크립트를 거치지 않고 `UnrealEditor.exe -server`를 직접 띄우면 같은 오류가 다시 난다.
  - **`.uproject`에서 에디터 전용으로 제한해도 소용없다.** 플러그인 참조의 `TargetAllowList`는 빌드 타깃 종류로 거른다(`PluginReferenceDescriptor.cpp:63-84`, `IsEnabledForTarget`). 이 프로젝트는 `-server`, `-game`도 `UnrealEditor.exe`로 실행하므로 타깃은 항상 `Editor`라 걸러지지 않는다. 실행 모드로 거르는 것은 모듈 단위의 `Type: Editor`이고(`ModuleDescriptor.cpp:723-732`, `GIsEditor`일 때만 로드), 두 플러그인의 에디터 모듈은 이미 그렇게 되어 있다. MCP HTTP 서버를 자동으로 여는 코드도 에디터 모듈(`ModelContextProtocolEditor.cpp:64-68`)에 있어 서버와 클라이언트에서는 포트를 열지 않는다(`smoke7` 로그에 MCP 수신 대기 줄 없음). 새던 것은 플러그인 `Content/Python`의 시작 스크립트뿐이고, 이것은 `-DisablePython`이 막는다.
  - **측정 조건이 바뀐 것이다.** Python이 켜져 있으면 플러그인이 코어 티커를 등록해 매 프레임 `Tick`을 부른다(`PythonScriptPlugin.cpp:1376-1379`). 끄면 서버 프레임에서 이 비용이 빠진다. `smoke7-r1`의 `work_avg_ms` 2.741은 `smoke2`\~`smoke5`의 범위(2.581\~2.787) 안이라 이 규모에서는 차이가 보이지 않았다. 기준선(태스크 8) 전에 바꿨으므로 비교가 어긋나지 않는다. 앞으로의 측정은 모두 `-DisablePython`이 들어간 `run-scenario.ps1`로만 실행한다. `smoke6`까지의 실행은 Python이 켜진 조건이다. 클라이언트 로그 끝의 `LogNet: Error: ... Host closed the connection.`은 측정이 끝나 서버가 종료하면서 남는 것이다.
- **클라이언트 인자에서 `-log`를 뺐다(2026-10-01).** 클라이언트마다 뜨던 로그 콘솔 창을 없애려는 것이다. `-log`는 콘솔 창을 보이게 할 뿐이다(`LaunchEngineLoop.cpp:6793-6796`, `GLogConsole->Show(true)`). 파일 출력 장치는 `-NODEFAULTLOG`가 없으면 항상 붙고(`GenericPlatformOutputDevices.cpp:28-31`), 파일 이름은 `-LOG=`에서 읽는다(같은 파일 84줄, `GetAbsoluteLogFilename`). `FParse::Param`은 이름 뒤에 공백이나 문자열 끝이 와야 일치하므로(`Parse.cpp:341`) `-LOG=파일`은 `-log`로 읽히지 않는다. `nolog1-r1`(클라이언트 2, 노드 101, NPC 10, 준비 20초, 측정 30초, `-NoTrace`)이 종료 코드 0으로 끝났고 `client0-nolog1-r1.log`(1,631줄)와 `client1-nolog1-r1.log`(1,637줄), 자동 스크린샷 6장이 남았다. 실행 중 클라이언트 프로세스의 보이는 창은 게임 창(창 클래스 `UnrealWindow`) 하나뿐이었고 콘솔 창(`ConsoleWindowClass`)은 서버에만 있었다. 수치는 이전 `smoke` 실행과 같은 범위다(`frames` 897, `work_avg_ms` 1.931, `out_bytes_per_sec_per_conn` 5391, `open_actor_channels_per_conn` 118, `saturated_ratio` 0.000). 서버의 `-log`는 그대로 둔다([ADR-0002](../Decisions/0002-editor-build-without-packaging.md)).
  - **측정 조건이 바뀐 것이다.** 클라이언트가 콘솔 창에 로그를 쓰지 않는다. 클라이언트에 `-log`가 있던 실행은 `smoke1`\~`smoke9`, `calib-a`\~`calib-e`, `diag-a`\~`diag-c`이고, 없는 실행은 `diag-d-r1`, `diag-e-r1`, `nolog1-r1`부터다(각 `client0-<라벨>.log`의 `Command Line` 줄로 확인). 서버 수치에 차이가 나는지는 따로 비교하지 않았다.

## 5. 태스크 8 시작 전 점검에서 확인한 것 (2026-10-01)

### 포화를 판정하는 시점

- `ServerReplicateActors`는 연결의 `IsNetReady()`가 거짓이 되는 즉시 그 연결의 리플리케이션을 멈춘다(`NetDriver.cpp:5695, 5868`). 연결마다 "포화로 끊겼는가"를 `Connection->TrackReplicationForAnalytics(bWasSaturated)`로 기록한다(`NetDriver.cpp:6022-6023`).
- 그 뒤 `UNetConnection::Tick`이 `QueuedBits`에서 이번 프레임의 예산(`CurrentNetSpeed × 경과 시간 × 8`)을 빼고, 아래로는 예산의 두 배까지만 내려가게 자른다(`NetConnection.cpp:5112-5145`). 엔진 자신의 프레임 단위 포화 기록 `SaturationAnalytics.TrackFrame(!IsNetReady())`는 이 감산 **앞**에 있다(`NetConnection.cpp:5110`).
- 그래서 `OnEndFrame`에서 읽은 `IsNetReady()`는 지속적인 포화에서도 거의 항상 참이다. 리플리케이션은 `QueuedBits`가 양수가 되자마자 멈추므로 초과분이 작고, 한 프레임의 예산(100,000 ÷ 30 ≈ 3,333바이트, 계산값)을 빼면 다시 음수가 되기 때문이다. 소스에서 읽은 결론이고 포화된 실행으로 확인한 것은 아니다.
- 결론: `saturated_ratio`를 `Connection->GetSaturationAnalytics()`(`NetConnection.cpp:6122`, 구조체는 `Engine/Public/Net/NetAnalyticsTypes.h:218`)의 `GetNumberOfSaturatedReplications()` ÷ `GetNumberOfReplications()`로 바꿨다. 측정 구간의 시작과 끝의 차를 모든 연결에 대해 더한다. `Engine/Source/Runtime`에서 이 기록을 리셋하는 호출은 `NetConnection.cpp`의 정의 말고는 찾지 못했다.
- 실행 확인: `smoke8-r1` 서버 로그에서 접속 직후 `saturated_replications=14/184`가 찍히고 그 뒤로 14에서 늘지 않았다. 옛 정의에서는 같은 구간이 항상 `saturated=0`이었다. 5초에 시도 횟수가 300씩 는다(연결 2 × 30Hz × 5초).

### 대역폭 예산과 서버 틱

- 예산을 계산하는 경과 시간은 `1 / DesiredTickRate`로 잘린다(`NetConnection.cpp:4816, 5117-5119`). 서버가 틱 예산을 넘겨 30Hz보다 느리게 돌면 초당 보낼 수 있는 양이 `net_speed`보다 작아진다.
- 결론: 포화 여부를 `out_bytes_per_sec_per_conn`과 `net_speed`의 비교로 판단하려면 `net_speed × frames ÷ (30 × 측정 초)`와 비교해야 한다. 기준선의 실측 송신량도 30Hz를 지키지 못한 만큼 작게 나오므로, 한도를 고정할 때는 30Hz로 환산한다(구현 계획 8.4a).

### 트레이스 인자

| 항목 | 5.8.3 | 위치 |
| --- | --- | --- |
| `-trace=<채널>` | 채널 목록. `default`는 `cpu,gpu,frame,log,bookmark,screenshot,region` | `Engine/Source/Runtime/Core/Private/ProfilingDebugging/TraceAuxiliary.cpp:138, 1952` |
| `-tracefile=<경로>` | 파일로 기록. 같은 경로에 파일이 있으면 경고만 남기고 시작하지 않는다(`-tracefiletrunc`를 주면 덮어쓴다) | `TraceAuxiliary.cpp:1064-1068, 1971, 1987` |
| `-NetTrace=<수준>` | 네트워크 트레이스의 상세 수준 | `Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp:2661` |
| 네트워크 트레이스의 컴파일 조건 | Shipping이 아니고 트레이스가 켜진 빌드. 에디터 Development 빌드가 해당한다 | `Engine/Source/Runtime/Net/Core/Public/Net/Core/Trace/NetTraceConfig.h:10-16` |
| 채널 이름 | `NetChannel` | `Engine/Source/Runtime/Net/Core/Private/Net/Core/Trace/Reporters/NetTraceReporter.cpp:16-17` |
| 자동 연결 | Unreal Insights가 떠 있으면(이름 있는 이벤트 `Local\UnrealInsightsAutoConnect`) 아직 연결하지 않은 프로세스가 `default` 채널로 로컬 트레이스 서버에 연결한다 | `TraceAuxiliary.cpp:2742-2756` |
| `-traceautostart=0` | 자동 시작을 끈다. 명령줄의 `-tracefile` 시작도 함께 막는다 | `TraceAuxiliary.cpp:2012-2021, 2378` |

- 실행 확인: `smoke8-r1`에서 `-trace=default,net -NetTrace=1 -tracefile=`로 `Saved/Traces/smoke8-r1.utrace`(5.5MB, 측정 30초)가 생겼다. 안의 네트워크 데이터와 북마크는 Insights로 아직 열어 보지 않았다. 채널 토큰 `net`이 `NetChannel`에 대응하는 규칙의 코드와 `-NetTrace=1`의 1이 뜻하는 수준은 확인하지 않았다.
- 결론: 클라이언트와 `-NoTrace` 서버에는 `-traceautostart=0`을 준다. 트레이스를 켠 서버에는 주지 않는다. 실패한 실행의 라벨을 다시 쓰면 트레이스가 생기지 않으므로 스크립트가 로그와 트레이스 파일로 라벨의 재사용을 막는다.

### 그 밖

- 클라이언트가 닫힘 메시지 없이 죽으면 서버는 `ConnectionTimeout=60.0`(`BaseEngine.ini:1855`)까지 연결을 유지할 수 있다. 더 빨리 알아채는 경로가 있는지는 확인하지 않았다. 그래서 스크립트가 서버 종료 시점에 클라이언트의 생존을 다시 검사한다.
- 내려다보기 화면에 보이는 범위는 가로 700m × 세로 약 394m다(높이 350m, 수평 시야각 90도, 화면비 16:9. 계산값). 1번 자리에서 검증용 노드까지는 약 196m다(반지름 500m의 원 위 16자리 중 이웃한 두 자리, 2 × 500 × sin(11.25°) ≈ 195m에 노드의 3m 오프셋. 계산값). 관련성을 적용하면 컬 거리 150m 밖이라 내려다보기 화면에서 검증용 노드의 검은 점이 보이지 않게 된다.
- 자동 이동은 한 변 100m의 정사각형이고 캐릭터 속도는 500cm/s다(`LabCharacter.cpp`의 `MaxWalkSpeed`). 한 바퀴 400m에 80초가 걸린다(계산값). 준비 30초와 측정 60초 동안 약 한 바퀴를 돈다.
- 수동 조작용 달리기(2026-10-02, 사용자 요청). `ALabCharacter`의 이동 컴포넌트를 `ULabCharacterMovement`로 바꾸고, 왼쪽 Shift를 누르는 동안 `GetMaxSpeed()`가 `SprintSpeed` 1,000cm/s(걷기 500의 두 배)를 돌려준다. 달리기 여부는 저장된 이동의 압축 플래그 `FLAG_Custom_0`(`Engine/Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h:3136`, 5.8.3에서 폐기 예정 표시 없음)으로 보낸다. 서버는 `MoveAutonomous`에서 `UpdateFromCompressedFlags`를 불러(`Engine/Source/Runtime/Engine/Private/Components/CharacterMovementComponent.cpp:10677`) 같은 값을 얻으므로 클라이언트 예측과 서버 이동이 같은 속도를 쓴다. 이미 보내는 플래그 바이트의 한 비트라 송신량이 늘지 않고, 리플리케이트되는 속성이나 RPC는 없다. 입력 액션과 매핑 컨텍스트는 에디터 에셋 없이 `SetupPlayerInputComponent`에서 만든다. 시나리오의 자동 이동(`LabPlayerController.cpp`의 `AddMovementInput`)은 이 플래그를 켜지 않으므로 측정에서는 걷기 속도만 쓰인다. 애니메이션 블렌드 스페이스는 그대로라 달릴 때 발이 미끄러져 보일 수 있다(확인하지 않음).

## 6. 같은 구성의 실행 사이에 서버 속도가 세 배 달라지는 원인 (2026-10-01)

모든 실행은 클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 트레이스 켬, `net_speed` 350000이다(`calib-b-r1`만 10000000). 엔진 소스가 아니라 이 PC에서의 실행으로 확인한 것이다.

**증상.** 서버가 약 1.5Hz로 도는 느린 상태와 약 5.5Hz로 도는 빠른 상태 둘 중 하나에 있고, 실행 도중에도 오간다. 서버 로그의 5초 간격 줄에서 프레임 번호의 차로 계산했다. `calib-b-r1`과 `calib-d-r1`은 내내 느렸고, `calib-e-r1`은 준비 구간 끝에 빨라졌고, `calib-c-r1`과 `diag-a-r1`은 측정 중에 여러 번 오갔다.

**확인한 것.**

| 확인 | 결과 | 근거 |
| --- | --- | --- |
| 느린 구간에 특정 코드가 더 도는가 | 아니다. 게임 스레드의 모든 타이머가 프레임당 호출 횟수는 같고 호출당 시간만 2.3\~2.9배 길다. `LabResourceNode` 6.9µs 대 2.4µs, `LabNpc` 18.0µs 대 7.2µs, `ServerMovePacked` 234µs 대 99µs, `USkeletalMeshComponent_TickAnimation` 43µs 대 18µs | `calib-e-r1.utrace`의 게임 스레드 타이밍 이벤트를 `UnrealInsights.exe -NoUI -ExecOnAnalysisCompleteCmd`의 `TimingInsights.ExportTimingEvents`로 내보내, 트레이스 시각 65\~85초(39프레임)와 125\~155초(155프레임)를 비교 |
| 느린 구간에 게임 스레드가 CPU를 얼마나 받는가 | 초당 0.20\~0.49초. 빠른 구간에는 0.95\~1.01초. 그동안 논리 프로세서 1번의 사용률은 최대치에 붙어 있고, 그 시간을 쓰는 다른 프로세스는 없다 | `diag-a-r1` 실행 중 약 1초 간격으로 서버 스레드별 `TotalProcessorTime`의 차, `\Processor Information(0,N)\% Processor Utility`, 프로세스별 CPU 시간의 차를 기록 |
| 논리 프로세서 1번에 무엇이 있는가 | 시나리오가 도는 동안 DPC가 초당 약 15,000개, 인터럽트가 초당 약 12,000\~16,000개 처리되고 DPC 시간이 32\~65%다. 0번과 2\~7번의 DPC 시간은 0\~2%다. UE 프로세스가 없을 때 1번의 DPC는 초당 약 480개다 | `diag-b-r1` 실행 중 `\Processor Information(0,0..7)\% DPC Time`, `DPCs Queued/sec`, `Interrupts/sec` 기록. 이 실행은 내내 빨랐고 1번의 사용자 시간은 대체로 0\~11%였다 |
| 서버를 1번에만 고정하면 | 내내 느리다. `frames` 90, `work_avg_ms` 670.713, 5초 간격 틱 1.3\~1.5Hz | `diag-c-r1` (`-ServerMask 2`) |
| 서버를 2\~7번에 고정하면 | 내내 빠르다. `frames` 340, `work_avg_ms` 176.630, `work_p99_ms` 216.841, 5초 간격 틱 4.7\~5.8Hz | `diag-d-r1` (`-ServerMask 252`). 이 실행부터 클라이언트 인자에 `-log`가 없다(다른 세션이 22:39:45에 스크립트를 고쳤다). `diag-c-r1`과는 서버 마스크와 클라이언트 `-log` 두 가지가 다르다 |
| 클라이언트 인자가 `diag-d-r1`과 같을 때 서버를 1번에만 고정하면 | 느리다. `frames` 153, `work_avg_ms` 392.972, `work_p99_ms` 1088.979. 5초 간격 틱이 1.1\~1.5Hz로 시작해 측정 구간 10초 무렵부터 2.3\~3.1Hz가 됐다. 그동안 1번의 DPC는 초당 약 15,000\~16,000개, DPC 시간 33\~46%, 사용자 시간 18\~24%로 `-log`가 있을 때와 같다 | `diag-e-r1` (`-ServerMask 2`, 클라이언트 `-log` 없음), 5초 간격 `\Processor Information(0,1)` 기록 |

**결론.** 서버 선호도 마스크 `0xFF`에 들어 있는 논리 프로세서 1번이 시나리오 실행 중 DPC와 인터럽트를 도맡는다. 서버 게임 스레드가 1번에 올라가 있는 동안에는 CPU를 절반 넘게 빼앗긴다. `work`는 경과 시간이라 빼앗긴 시간이 그대로 들어간다. Windows 스케줄러가 스레드를 어느 코어에 두는지가 실행마다, 실행 도중에도 달라져 수치가 세 배까지 흔들렸다.

**확인하지 않은 것.** DPC와 인터럽트를 만드는 장치나 드라이버(루프백 네트워크인지 GPU인지). 서버를 1번에만 고정한 `diag-e-r1`에서 틱이 1.1Hz에서 3.1Hz로 바뀐 이유(마스크가 논리 프로세서 하나라 서버의 다른 스레드도 같은 코어를 쓴다). 클라이언트 `-log`가 없는 조건에서 마스크 `0xFF`의 흔들림. 재부팅 뒤에도 1번에 몰리는지. 0번(1번과 같은 물리 코어의 SMT 짝)만 쓸 때의 속도.

프레임당 송신량은 상태와 무관하다. 30Hz 환산 송신량이 `calib-b-r1` 172,638, `calib-e-r1` 172,756, `diag-d-r1` 172,271(32,540 × 30 ÷ (340 ÷ 60))이라 고정한 송신 한도 350,000은 그대로 맞다.


## 7. 서버 틱이 30Hz가 아니라 약 21Hz로 도는 원인 (2026-10-03)

틱 예산 안의 구성에서 `frames`가 60초에 약 1,790이 아니라 약 1,280으로 나오는 실행이 있었다(`relevancy2-r1` 1,304, `dormancy2-r1` 1,288, `refactor-after2-r1` 1,271, `refactor-after2-r3` 1,287). 서버 로그의 프레임 번호로 보면 시작 신호 전의 대기 구간에서 이미 5초에 약 107프레임이다.

원인은 Windows 11이 서버 프로세스의 타이머 해상도 요청을 무시하는 것이다.

- 엔진은 시작할 때 타이머 해상도 1ms를 요청한다(`WindowsPlatformMisc.cpp:1076`의 `timeBeginPeriod(1)`). 틱 속도 제한은 남은 시간에서 2ms를 뺀 만큼 `::Sleep`으로 자고 나머지를 돌면서 기다린다(`UnrealEngine.cpp:3116`의 `SleepNoStats(WaitTime - 0.002f)`, `WindowsPlatformProcess.cpp:1883`).
- Windows 11은 창을 가진 프로세스의 창이 최소화되거나 완전히 가려지면 그 프로세스에 기본 해상도(15.625ms)보다 높은 해상도를 보장하지 않는다(Microsoft 문서의 `timeBeginPeriod` 설명). 서버는 `-log`의 콘솔 창을 가진 프로세스다(창의 소유 프로세스가 `UnrealEditor`임을 `GetWindowThreadProcessId`로 확인).
- 해상도가 15.625ms면 `::Sleep`이 그 배수에서만 깨어나 프레임 주기가 33.3ms가 아니라 15.625 × 3 = 46.875ms(21.3Hz)에 맞춰진다.

실행으로 확인한 것:

| 실행 | 조건 | 결과 |
| --- | --- | --- |
| `timerdiag1`(서버만) | 콘솔 창을 그대로 → 최소화 → 복원을 20초씩 두 번 | 시스템 타이머 해상도(`NtQueryTimerResolution`)가 1ms → 15.625ms → 1ms로 창 상태를 따라 바뀜. 틱은 30.2Hz → 28.5\~29.7Hz → 30.3Hz |
| `timerdiag-min-r1`, `timerdiag-min2-r1`(작은 규모 시나리오) | 서버 콘솔 창을 최소화 | `frames` 644, 645(30초, 21.5Hz) |
| `timerdiag-optout-r1`, `timerdiag-optout2-r1`(같은 규모) | 최소화하고, 서버 프로세스에 `SetProcessInformation(ProcessPowerThrottling)`으로 `PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION`을 끔 | `frames` 901, 902(30Hz) |

- 측정 실행에서는 아무도 창을 최소화하지 않았다. 클라이언트 창이나 다른 창이 서버 콘솔 창을 완전히 가린 것으로 추정하지만, 가려진 상태만으로 재현하는 실행은 하지 않았다(`timerdiag3`은 다른 프로세스가 해상도 1ms를 잡고 있어 판정하지 못했다).
- 영향: `frames`와 초당 값(`out_bytes_per_sec_per_conn`)이 약 0.71배가 된다. 틱마다 하는 일이 달라져 `work_avg_ms`도 흔들릴 수 있다(`dormancy2-r1` 16.205, `refactor-after2-r3` 14.794. `refactor-after2-r1`은 12.850으로 차이가 없었다). 틱 예산을 넘는 구성(`baseline3`)은 기다리지 않으므로 영향이 없다.
- 대처는 [ADR-0012](../Decisions/0012-server-timer-resolution.md)(승인됨)다. `Scripts/common.ps1`의 `Disable-LabTimerThrottle`이 서버를 띄운 직후 이 스로틀을 끈다. 적용 뒤 `timerfix-min-r1`(서버 창 최소화, 작은 규모)이 `frames` 902, 확정 규모 `timerfix` 세 실행이 1,788 / 1,784 / 1,796이다.

## 8. 2막 계획을 세우며 확인한 것 (2026-10-03)

모두 5.8.3 소스에서 읽은 것이고 실행해 보지 않았다. 쓰이는 곳은 [2막 설계](../Planning/2026-10-03-act-2-design.md) 3절과 6절이다.

| 사실 | 소스 위치 | 쓰이는 곳 |
| --- | --- | --- |
| Push Model은 에디터 타깃에서 기본으로 컴파일된다(`bWithPushModel`의 기본값이 `Type == TargetType.Editor`). 다른 타깃은 `Target.cs`에서 켜야 한다 | `Engine/Source/Programs/UnrealBuildTool/Configuration/Rules/TargetRules.cs:1522-1526` | Push Model 포스팅. Test 패키지로 재측정할 때는 서버 타깃에서 따로 켠다 |
| Push Model은 컴파일돼 있어도 실행 중 기본값이 꺼짐이다(`Net.IsPushModelEnabled`, `bIsPushModelEnabled = false`) | `Engine/Source/Runtime/Net/Core/Private/Net/Core/PushModel/PushModel.cpp:434-437` | Push Model 포스팅 |
| Push Model이 아닌 프로퍼티는 비교할 때 항상 바뀐 것으로 취급된다 | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1506` | Push Model 포스팅. 엔진 기본 프로퍼티 가운데 어느 것이 해당하는지는 보지 않았다 |
| `APlayerState`는 Always Relevant이고 Net Update Frequency가 1이다 | `Engine/Source/Runtime/Engine/Private/PlayerState.cpp:26, 28` | 인벤토리를 어느 액터에 둘지(설계 3.3절) |
| `APawn`의 Net Update Frequency는 100이다. 폰의 관련성은 보는 위치와 액터 위치의 거리가 Net Cull Distance보다 짧은지로 판정한다(소유자와 Always Relevant는 먼저 통과한다) | `Engine/Source/Runtime/Engine/Private/Pawn.cpp:88, 1274-1301`, `ActorReplication.cpp:383-385` | 플레이어가 모이는 배치의 거리 계산(설계 3.1절), 인벤토리를 어느 액터에 둘지 |
| 리플리케이트하는 배열의 원소 수 한도는 65,535개다(핸들이 `uint16`). 넘으면 오류를 남기고 그 비교를 실패로 끝낸다 | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:277-290`(`ValidateArraySize`) | 인벤토리의 칸 수(설계 3.3절) |
| 실행 중에 서버가 `NewObject`로 만들어 등록한 리플리케이트 컴포넌트는 클라이언트에도 만들어진다. 클라이언트는 받은 컴포넌트를 등록하고 리플리케이트하도록 표시한다 | `Engine/Source/Runtime/Engine/Private/Components/ActorComponent.cpp:2953-2957`(`OnCreatedFromReplication`), `ActorReplication.cpp:1008-1020`. 실행 확인: `tsmall-elem-r1`의 화면 글자 `states=2 inventories=2` | 상태 값과 인벤토리를 1막의 클래스에 프로퍼티를 더하지 않고 붙이는 방법(설계 3.2, 3.3절) |
| 반복 타이머는 프레임이 주기보다 길면 밀린 횟수만큼 한 프레임에 여러 번 불린다(`bMaxOncePerFrame`을 켜지 않았을 때) | `Engine/Source/Runtime/Engine/Private/TimerManager.cpp:1235-1248` | 상태 값과 인벤토리를 바꾸는 타이머. 서버가 느린 기준선에서도 초당 바뀌는 횟수가 같다 |
| 프로퍼티 비교는 객체마다 프레임에 한 번만 하고, 그 결과를 연결들이 함께 쓴다(`GShareShadowState`이고 `LastReplicationFrame`이 이번 프레임이면 비교를 건너뛴다) | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1275-1331` | 밀집과 분산의 비교(설계 5절). 밀집에서는 같은 액터를 여러 연결이 받아 비교 횟수가 줄고, 연결마다 하는 일은 그대로다 |
| 일반 `TArray` 프로퍼티는 비교할 때마다 모든 원소를 하나씩 비교하고, 바뀐 원소만 변경 목록에 넣는다. 배열이 줄면 배열 핸들만 넣어 크기를 알린다 | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1692-1775`(`CompareProperties_Array_r`) | 인벤토리와 FastArray 포스팅. 한 칸의 수량만 바꾸면 그 원소만 보내므로, 일반 배열의 비용은 배열 전체를 다시 보내는 바이트가 아니라 고려할 때마다 전체를 비교하는 CPU다. 가운데 원소를 지워 뒤가 밀리면 밀린 원소가 모두 바뀐 것이 된다 |
| `FRepMovement::NetSerialize`는 플래그(2비트 또는 4비트), 위치, 회전 세 성분, 선속도를 보내고, `bRepPhysics`일 때만 각속도를 더 보낸다. 위치와 속도의 정밀도는 `LocationQuantizationLevel`, `VelocityQuantizationLevel`, 회전은 `RotationQuantizationLevel`(바이트 또는 쇼트)이 정한다 | `Engine/Source/Runtime/Engine/Private/Engine/ReplicatedState.cpp:67-117`, `Engine/Source/Runtime/Engine/Classes/Engine/ReplicatedState.h:164-172` | NPC 이동의 `NetSerialize` 포스팅. NPC 갱신의 `ReplicatedMovement` 92비트(`update-frequency3-r1`)가 성분마다 얼마인지는 보지 않았다. 비트 구성은 13절(가속도 있음 1비트가 더 있다) |

확인하지 않은 것: 지연과 패킷 손실을 넣는 설정의 이름과 위치, Iris가 구조체의 `NetSerialize`를 그대로 쓰는지(`PropertyNetSerializerInfoRegistry.cpp:98-118`에 `FLastResortPropertyNetSerializerInfo`가 있다는 것까지만 봤다), `GameNetDriver` 타이머가 Iris에서 같은 범위를 감싸는지.

## 9. `-statnamedevents`가 트레이스에 주는 영향 (2026-10-05, 태스크 24)

포스팅 7(`GameNetDriver` 자체 시간 나누기)을 시작하며 확인했다. 실행은 작은 규모 두 번이다: `tsmall-named-off1-r1`(인자 없음), `tsmall-named-on1-r1`(`run-scenario.ps1 -StatNamedEvents`). 둘 다 클라이언트 2, 자원 노드 100, NPC 10, 2막 요소를 줄인 값(`-PlayerSpacing 3 -NpcsNearPlayers 5 -StateInterval 5 -InventoryItems 20 -InventoryChurn 4 -Buildings 20 -BuildInterval 1`), 측정 30초다.

| 사실 | 근거 |
| --- | --- |
| 명령줄 `-statnamedevents`는 `GCycleStatsShouldEmitNamedEvents`만 올리고 stat 수집(`FThreadStats::bPrimaryEnable`)은 켜지 않는다 | `Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp:1759-1762`. 수집을 켜는 것은 `StatsPrimaryEnableAdd`이고 `stat` 명령에서만 불린다(`Engine/Source/Runtime/Core/Private/Stats/StatsCommand.cpp`) |
| 켜지면 cycle stat 범위(`SCOPE_CYCLE_COUNTER`)가 stat 설명을 이름으로 하는 Insights 타이머가 된다(`STAT_NetConsiderActorsTime`은 `Consider Actors Time`) | `Engine/Source/Runtime/Core/Public/Stats/StatsSystemTypes.h:1526-1543`(`FCycleCounter::Start`), 설명 문자열은 `Engine/Source/Runtime/Engine/Public/EngineStats.h:72-80` |
| 클래스 타이머(`LabNpc`, `LabBuilding` 등)의 이름은 바뀌지 않는다. stat이 켜진 빌드의 `FScopeCycleCounterUObject`는 stat 수집이 꺼져 있으면 named events와 상관없이 객체의 `FName`으로 트레이스한다 | `Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectBaseUtility.h:995-1015`, `UObjectBaseUtility.h:800-815`(`GetStatID`), `Engine/Source/Runtime/CoreUObject/Private/UObject/ObjectBaseUtility.cpp:146-165`. 실행: 두 트레이스의 클래스 타이머 이름이 같고 횟수가 비슷하다(`LabNpc` 15,765번과 15,759번, `LabCharacter` 2,736번과 2,768번) |
| 클래스 타이머의 부모가 바뀐다. `GameNetDriver` 바로 아래가 아니라 `ServerReplicateActors Time` → `Process Prioritized Actors Time` → `Replicate Actor Time` 아래에 있다 | `NetDriver.cpp:6279, 5689`, `DataChannel.cpp:3608, 3624`. 실행: `tsmall-named-on1-r1`의 Callees |
| `GameNetDriver` 위에 `NetDriver TickFlush`가 생긴다(`SCOPE_CYCLE_COUNTER(STAT_NetTickFlush)`이 `GameNetDriver` 범위보다 앞에 있다). `GameNetDriver`의 이름과 Incl은 그대로다 | `NetDriver.cpp:1173-1174`. 실행: 프레임당 Incl 0.344ms와 0.337ms |
| `WorldTick` 타이머가 프레임을 감싸지 않는다. 프레임당 약 1µs이고 아래에 아무것도 없다. 프레임을 감싸는 것은 `World Tick Time`(`STAT_WorldTickTime`)이다. 원인은 확인하지 않았다(`WorldTick`은 `RHI_BREADCRUMB_EVENT_GAMETHREAD`가 만든다, `LevelTick.cpp:1520`) | 실행: `tsmall-named-on1-r1`의 `WorldTick` Incl 0.001초(측정 30초), `World Tick Time` 1.311초. 인자 없는 실행의 `WorldTick`은 1.313초 |
| `GameNetDriver` Excl이 대부분 나뉜다. 작은 규모에서 Excl이 Incl의 70.4%에서 8.0%로 줄었다 | 실행: 0.242 ÷ 0.344, 0.027 ÷ 0.337(프레임당 ms) |
| `TickCompletionEvents` 아래 `ProcessUntilTasksComplete`의 Excl도 나뉜다(물리 `[Scene] - StartFrame`, 플레이어 캐릭터의 애니메이션 `CharacterMesh0`, NPC 이동) | 실행: 인자 없는 실행에서 `ProcessUntilTasksComplete` Excl이 Incl의 49.2%, 켠 실행에서 3.2% |
| 프로파일러가 붙으면 named events를 켜는 콘솔 변수 `stats.AutoEnableNamedEventsWhenProfiling`의 기본값은 꺼짐이다. 그래서 지금까지의 트레이스에는 stat 타이머가 없었다 | `Engine/Source/Runtime/Core/Private/Misc/CoreMisc.cpp:526` |

결론: `run-scenario.ps1 -StatNamedEvents`로 잰 트레이스는 `export-insights.ps1`이 서버 로그의 명령줄을 보고 알아서 `GameNetDriver`를 뿌리로 내보내고, `GameNetDriver`와 `TickCompletionEvents` 아래 트리를 요약에 적는다. 이 실행의 수치는 이벤트가 더 기록되므로 다른 실행과 비교하지 않는다. 클래스별 값을 기본 트레이스와 같은 방식으로 읽으려면 `Replicate Actor Time` 아래를 본다. 평탄한 타이머 통계(`stats.csv`)의 클래스 타이머는 리플리케이션과 액터 틱(`LabNpc`는 둘 다 있다)이 섞인 값이다.

## 10. 리플리케이션 조건과 FastArray (2026-10-06, 태스크 26, 27)

5.8.3 소스에서 읽었고, 조건은 실행으로도 확인했다. 쓰이는 곳은 [포스팅 9](../../Posts/09-inventory-owner-only/README.md)와 포스팅 10(FastArray)이다. 자세한 위치는 [포스팅 9 관찰 자료](../../Posts/09-inventory-owner-only/candidates.md) 4절에 있다.

| 사실 | 소스 위치 또는 실행 |
| --- | --- |
| 리플리케이션 조건은 비교 뒤에 연결마다 확인한다. 비교는 객체마다 프레임에 한 번이고, 조건이 꺼진 연결은 바뀐 목록에서 그 프로퍼티를 뺀다. 그래서 조건은 보내는 바이트와 직렬화만 줄이고 비교는 줄이지 않는다 | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1275-1331`(비교), `2047`, `2660-2680`(`FilterChangeListToActive`). 실행: `act2-invown1`에서 `work_avg_ms`는 구별되지 않았다 |
| 캐릭터의 소유자 연결은 컨트롤러의 연결이다. 액터 채널은 연결마다 `bNetOwner`를 정한다 | `Pawn.cpp:753-760`(`APawn::GetNetConnection`), `DataChannel.cpp:3809-3812` |
| 조건을 `GetLifetimeReplicatedProps`에서 서버 인자로 고르고, 클라이언트는 `COND_None`으로 등록해도 클라이언트가 받는다 | 실행: `tsmall-invown-on1-r1`, `act2-invown1`(화면 글자의 자기 인벤토리 칸 수 200). 받는 쪽 코드를 끝까지 따라가지는 않았다 |
| 한 패킷에 다 들어가지 않는 큰 Bunch는 여러 패킷에 나뉘어 간다(Networking Insights에 `PartialInitial`로 보인다) | 실행: `act2-invown-base1-r2`의 패킷 16,384(인벤토리 18,190비트, `Actor` 줄 7,630비트) |
| FastArray는 칸마다 `ReplicationID`와 `ReplicationKey`를 두고, 연결마다 지난번에 보낸 표와 비교해 키가 바뀐 칸과 없어진 번호만 보낸다. 배열 키가 그대로면 칸을 보지 않는다 | `Engine/Source/Runtime/Net/Core/Classes/Net/Serialization/FastArraySerializer.h:298-332`, `819-853`, `896-975` |
| FastArray의 칸 안 델타 직렬화는 기본으로 켜져 있다. 생성자가 `DeltaFlags`를 `None`으로 초기화한 뒤 본문에서 `SetDeltaSerializationEnabled(true)`를 부르고, 전역 스위치 `net.SupportFastArrayDelta`도 기본 1이다. 처음 이 행에 "기본값은 꺼짐(`FastArraySerializer.cpp:33`)"이라고 적었으나 33행은 초기화 목록이라 틀렸다(2026-10-06 정정) | `Net/Core/Private/Net/Serialization/FastArraySerializer.cpp:24-36`, `FastArraySerializer.h:549-566, 1395-1402`, `Engine/Private/DataReplication.cpp:68-72` |
| 칸 안 델타 직렬화에서 칸 하나는 번호 `uint32`, 1비트, 바뀐 프로퍼티마다 핸들(`SerializeIntPacked`, 작은 값은 8비트)과 값, 끝 핸들 8비트다. 새 칸은 비교를 일부러 실패시켜 프로퍼티를 모두 보내고(`net.DeltaInitialFastArrayElements` 기본 0), 바뀐 칸은 바뀐 프로퍼티만 보낸다. 지운 칸은 헤더 뒤의 번호 32비트뿐이다. 칸 안 비교의 결과(변경 이력)는 연결들이 함께 쓴다 | `Engine/Private/RepLayout.cpp:7700-7720, 7878-7930`(비교), `8031-8086`(쓰기), `110-111`, `1922-1935`(`WritePropertyHandle`). 실행: `act2-fastarr1-r2`의 `ChangedElement` 2,382비트 = 새 칸 15 × 121 + 바뀐 칸 7 × 81, `ItemId` 15번, `Count` 22번 |
| 클라이언트는 지운 칸에 `PreReplicatedRemove`, 새 칸에 `PostReplicatedAdd`, 바뀐 칸에 `PostReplicatedChange`를 이 순서로 부르고, 배열 구조체에 `PostReplicatedReceive`가 있으면 갱신 끝에 한 번 부른다 | `FastArraySerializer.h:1078-1203`(`PostReceiveCleanup`), `699-707`, `1645-1675` |
| FastArray의 클라이언트는 지운 칸을 `RemoveAtSwap`으로 지운다. 클라이언트의 칸 순서가 서버와 달라진다 | `FastArraySerializer.h:1193` |
| FastArray의 클라이언트는 처음 보는 번호의 칸을 배열 맨 뒤에 더하고, 지우기는 마지막에 한다. 맨 앞 칸을 지우고 맨 뒤에 더하면 새 칸이 지운 칸의 자리로 옮겨 와, 클라이언트에서는 새 칸이 앞에서부터 차례로 들어간다 | `FastArraySerializer.h:1524`(`AddDefaulted_GetRef`), `1186-1197`. 실행: `tsmall-fastarr-all1-r1`의 인벤토리 패널에서 맨 윗줄의 한 칸만 번쩍였다 |
| FastArray 같은 사용자 정의 델타 프로퍼티는 객체마다 하는 비교에 들어가지 않는다. 연결마다 리플리케이트할 때 `ReplicateCustomDeltaProperties`가 조건을 확인하고 보낼지 정한다 | `RepLayout.cpp:5848-5855`(`IsCustomDelta`), `6318-6321`, `1424-1431`, `DataReplication.cpp:1646, 1719` |
| FastArray는 바뀌었을 때만 쓴다. 앞 칸을 지운 한 번은 바뀐 칸 하나와 지운 번호 하나다 | 실행: `tsmall-fastarr-on2-r1`의 서버 로그(`LogNetFastTArray Log`)에 `NumChange: 1. NumDel: 1` 100줄, 채집 6줄, 처음 200칸 8줄뿐이다 |
| FastArray는 객체마다의 비교를 없애는 대신 연결마다 리플리케이트할 때 `Custom Delta Property Rep Time`이 돈다. 칸이 드물게 바뀌는 인벤토리 여덟 개에서 그 컴포넌트의 CPU는 줄지 않았다(프레임당 0.104 → 0.117ms). 이 타이머는 `-statnamedevents` 트레이스에 기록된다 | 실행: `act2-fastarr-base-split1-r1`, `act2-fastarr-split1-r1`(한 번씩). `DataReplication.cpp:1661`. [포스팅 10 관찰 자료](../../Posts/10-inventory-fastarray/candidates.md) 13절 |
| `FFastArraySerializer`를 쓰는 모듈은 `NetCore`에 의존해야 한다. 빠지면 `FFastArraySerializer` 생성자 등에서 링크 오류가 난다 | `Net/Core/Classes/Net/Serialization/FastArraySerializer.h`(`NETCORE_API`). 2026-10-06 빌드 |

## 11. 계획 초안의 코드에서 바꾼 것 요약

| 태스크 | 바꾼 것 | 이유 |
| --- | --- | --- |
| 전체 | 경로, 모듈 이름, 접두사(`SL` → `Lab`) | 0절 |
| 5.1, 5.2 | 생성자에서 입력 매핑을 읽고 `SetupInputComponent`에서 등록 | 3절 |
| 5.2 | `TickTopDown`의 플래그 설정을 지우고 `SpawnPlayerCameraManager` 재정의로 옮김(서버와 클라이언트, 모든 플레이어) | 2절 "관련성 판정의 기준 위치" |
| 5.2 | `PlayerTick` 앞에 `IsLocalController()` 검사 추가, 지역 변수 `Role`을 `View`로 바꿈 | 서버에서 실행되지 않게 함. `AActor::Role` 멤버와 이름이 겹침 |
| 5.2 | `TickTopDown`에서 `bClientSimulatingViewTarget`을 켜고 매 틱 뷰 타깃을 확인 | 4절 |
| 6.2 | `IsNetReady(false)` → `IsNetReady()` | 1절 |
| 2.1\~2.3 | `env.ps1` 대신 `common.ps1`이 `EngineAssociation`으로 엔진을 찾음 | 0절 |
| 7.1 | 선호도를 기다리는 동안 다시 설정, 클라이언트가 먼저 죽으면 바로 실패 | 4절 |
| 7.1, 7.2 | 스크립트를 UTF-8 BOM으로 저장 | `powershell`(5.1)이 BOM 없는 한글을 잘못 읽음 |
| 8.4a | 올릴 설정 키가 세 개 | 1절 "한도가 정해지는 과정" |
| 7.1, 7.2 | 서버와 클라이언트 인자에 `-DisablePython` 추가 | 4절 |
| 7.1, 7.2 | 클라이언트 인자에서 `-log`를 뺌 | 4절 |
| 6.2 | `saturated_ratio`를 프레임 끝의 `IsNetReady()`에서 엔진의 포화 기록(`GetSaturationAnalytics`)으로 바꿈. 5초 간격 로그의 `saturated=`가 `saturated_replications=끊긴 횟수/시도 횟수`가 됨 | 5절 "포화를 판정하는 시점" |
| 7.1 | 시작 신호 전에 죽은 클라이언트를 다시 띄움(실행당 최대 3번). 서버 종료 시점의 클라이언트 생존, `.utrace` 존재, 측정 시작 뒤의 선호도 재설정, 라벨의 로그와 트레이스 재사용, 이미 떠 있는 `UnrealEditor`를 검사. 클라이언트와 `-NoTrace` 서버에 `-traceautostart=0` | 4절(단언 크래시), 5절 |
| 8.4, 8.4a | 포화를 먼저 없앤 뒤 예산 초과를 판단. 한도는 규모가 정해진 뒤 30Hz 환산 송신량의 약 두 배로 한 번만 고정 | 5절 "대역폭 예산과 서버 틱" |

## 12. 틱 예산을 넘는 구성에서는 프레임이 길수록 Consider List가 길어진다 (2026-10-06)

엔진 소스의 식과 이 PC의 실행으로 확인한 것이다. 2막 README의 일곱 구성을 다시 재다가 찾았다([Worklog/10-inventory-fastarray.md](../Worklog/10-inventory-fastarray.md) "README와 시각화 페이지 갱신").

**식.** 다음 고려 시각은 고려한 시각에 `1 ÷ 빈도`와 0\~`ServerTickTime`의 난수를 더한 시각이다(2절 "업데이트 빈도의 스케줄링"). 빈도 100, 30Hz에서는 10\~43.3ms 뒤다. 프레임 간격이 D(10\~43.3ms)이면 다음 프레임에 아직 시각이 되지 않아 한 프레임을 건너뛰는 액터의 비율은 (43.3 − D) ÷ 33.3이다(계산값). 틱 예산 안의 구성은 서버가 33.3ms를 채워 기다리므로 D가 고정된다. 예산을 넘는 구성에서는 프레임이 길어질수록 Consider List가 길어지고, D가 43.3ms를 넘으면 활성 목록의 모든 액터가 매 프레임 든다.

**실행.** 2막의 ① 거리 판정 구성(2막 요소 인자에 `-NoNodeDormancy -NpcUpdateFrequency 100`, 활성 목록 5,885개)이다.

| 소스 | 실행 | 서버 프레임 시간 평균 | Consider List(프레임당) | 식으로 계산한 값 |
| --- | --- | ---: | ---: | ---: |
| `92dc93e` | `act2-bisect-92dc93e-r1` | 38.331ms | 5,073.9 | 5,885 ÷ (1 + (43.333 − 38.331) ÷ 33.333) = 5,117 |
| `243be3d`(main) | `act2-all-check2-r2` | 50.067ms | 5,875.4 | 5,885(D가 43.3ms 넘음) |

두 실행에서 액터 한 번의 리플리케이션 비용은 같았다. `LabBuilding`은 호출당 2.079µs와 2.087µs, `LabResourceNode`는 2.29µs와 2.34µs다(`export-insights.ps1`의 프레임당 Incl ÷ Count). 늘어난 것은 프레임당 호출 횟수와 `GameNetDriver` Excl(19.951 → 28.564ms)이다.

**되먹임.** D가 38ms 근처에서 1ms 길어지면 Consider List는 5,885 × (1 ÷ 33.3) ÷ 1.15² ≈ 133개 늘어난다(계산값). 액터 하나에 드는 프레임 시간이 38.331 ÷ 5,073.9 = 7.55µs이므로 약 1.0ms를 더 일한다. 되먹임의 이득이 1에 가까워서, 몇 %의 작은 차이가 D를 43.3ms 넘게 밀어 올린다. 그 위에서는 Consider List가 더 늘지 않아 약 50ms에 머문다.

**관찰한 값.** 같은 ① 구성의 `work_avg_ms`다.

| 실행 | 소스 | 값 |
| --- | --- | --- |
| `act2-relevancy1`(2026-10-05, 포스팅 6) | `0f689c2` | 35.156\~36.623 |
| `act2-all-relevancy1`(2026-10-06) | `243be3d` | 51.549\~64.203 |
| `act2-all-check1`, `act2-all-check2` | `243be3d` | 49.510\~50.669 |
| `act2-all-check-old1`, `act2-all-check-old2` | 포스팅 6 태그 | 37.320\~40.422 |
| `act2-bisect-3a82d7b`, `-a397f55`, `-730f247`, `-92dc93e` | 각 커밋 | 37.288\~38.926 |
| `act2-split-nofa` | `243be3d`에서 FastArray 컴포넌트와 `NetCore` 의존을 뺌 | 41.361 |
| `act2-split-netcore` | `92dc93e`에 `NetCore` 의존만 더함 | 39.219 |
| `act2-split-main` | `243be3d` | 49.920 |

`243be3d`는 이 구성에서 새로 하는 일이 없다. FastArray 컴포넌트는 `-LabInventoryFastArray`가 없으면 만들지 않는다.

**결론.** ①처럼 틱 예산을 조금 넘는 구성은 코드 배치나 PC 상태의 작은 차이로 약 38ms와 약 50ms 사이를 오간다. 이 구성의 서버 프레임 시간은 다른 묶음과 비교하지 않는다. 예산 안의 구성(② 이후)과 크게 넘는 기준선(D가 이미 43.3ms보다 길다)은 해당하지 않는다.

**확인하지 않은 것.** `243be3d`가 어떤 경로로 프레임 시간을 몇 % 늘렸는지(코드 배치로 짐작한다). PC 상태의 차이(포스팅 6 태그의 소스도 35.674 → 37.320\~40.422)의 원인.

## 13. NPC 이동 데이터의 비트 구성 (2026-10-07, 태스크 29)

엔진 소스에서 읽은 식이다. 포스팅 12의 [관찰 자료](../../Posts/12-npc-move-netserialize/candidates.md) 1절에 비트 분포와 계산이 있다.

| 사실 | 소스 위치 |
| --- | --- |
| `FRepMovement::NetSerialize`는 플래그 4비트(연결 버전 25 이상), 위치, 회전, 선속도, 가속도 있음 1비트(버전 35 이상)를 차례로 쓴다. 서버 프레임과 물리 핸들은 0이 아니거나 `INDEX_NONE`이 아닐 때만, 각속도는 `bRepPhysics`일 때만, 가속도는 `bRepAcceleration`일 때만 더 쓴다 | `Engine/Source/Runtime/Engine/Private/Engine/ReplicatedState.cpp:67-152`, `Engine/Source/Runtime/Core/Public/Misc/EngineNetworkCustomVersion.h:40` |
| 위치와 속도는 축당 비트 수 N을 7비트 헤더에 쓰고 X, Y, Z를 N비트씩 쓴다. N은 세 축 가운데 가장 큰 절댓값(정밀도 단위로 반올림)에 부호 비트를 더한 길이다. 0 벡터는 축당 1비트다 | `Engine/Source/Runtime/Net/Core/Private/Net/Core/Serialization/QuantizedVectorSerialization.cpp:13-17, 90-96`, `Engine/Source/Runtime/Core/Private/Serialization/BitWriter.cpp:142-146` |
| `ByteComponents` 회전은 성분마다 "0이 아님" 1비트와, 0이 아니면 1바이트다 | `Engine/Source/Runtime/Core/Private/Math/UnrealMath.cpp:84-` |
| 따라서 평면에서 움직이고 속도가 0인 액터(`ALabNpc`)의 이동 데이터는 33 + 3N비트, 핸들을 더하면 41 + 3N비트다. 맵 원점에서 멀수록 N이 커진다(163m까지 15 이하, 655m를 넘으면 18) | 계산값. `act2-interp2-r1`의 패킷 하나에서 읽은 NPC의 `ReplicatedMovement`(`Shared`) 83비트가 N = 14일 때와 같다 |
| 공유 직렬화(`Shared`)의 비트에는 프로퍼티 핸들이 들어 있다. Networking Insights의 `Shared` 범위는 공유 직렬화한 비트를 패킷에 복사할 때만 남는다. 그래서 어떤 프로퍼티가 공유 직렬화로 갔는지는 `Shared`의 Incl에서 그 프로퍼티의 비트를 빼 보면 안다(포스팅 12 측정 기록 5.1절) | `Engine/Source/Runtime/Engine/Private/RepLayout.cpp:2741-2752`(`WriteSharedProperty`), `2856-2861`(`Shared` 범위와 `GNumSharedSerializationHit`) |
| 구조체를 공유 직렬화하려면 `WithNetSharedSerialization`을 켠다. 켜지 않은 구조체는 연결마다 직렬화한다 | `RepLayout.cpp:5555-5557`, `Engine/Source/Runtime/Engine/Classes/Engine/ReplicatedState.h:305-312` |
| 패킷을 잃으면 그 패킷에 담긴 바뀐 프로퍼티를 재전송 대상으로 표시하고, 다시 보낼 때는 그때의 현재 값을 직렬화한다 | `Engine/Source/Runtime/Engine/Private/DataReplication.cpp:888-925`(`FObjectReplicator::ReceivedNak`), `RepLayout.cpp:2262-2279`(`UpdateChangelistHistory`가 다시 보낼 변경 목록을 이번 변경 목록에 합침) |

## 14. 패킷 시뮬레이션(지연과 손실) (2026-10-07, 태스크 30)

엔진 소스에서 읽은 것이다. 포스팅 13의 [관찰 자료](../../Posts/13-lag-and-packet-loss/candidates.md) 1절이 이것을 쓴다. 적용한 것을 실행에서 확인한 값은 그 2절과 4절에 있다.

| 사실 | 소스 위치 |
| --- | --- |
| 패킷 시뮬레이션 설정은 `FPacketSimulationSettings` 하나에 모여 있고, Shipping이 아닌 빌드에서만 컴파일된다(`DO_ENABLE_NET_TEST`). 송신 쪽은 `PktLoss`(%), `PktLag`와 `PktLagVariance`(일정 지연 ± 흔들림, ms), `PktLagMin`/`PktLagMax`(범위 안의 난수, `PktLag`가 0일 때만), `PktOrder`, `PktDup`, `PktJitter`, 수신 쪽은 `PktIncomingLoss`(%), `PktIncomingLagMin`/`PktIncomingLagMax`(ms)다 | `Engine/Source/Runtime/Engine/Classes/Engine/NetDriver.h:450, 458-586` |
| 넷 드라이버가 뜰 때 `Engine.ini`의 `[PacketSimulationSettings]`를 읽고, 그다음 명령줄을 읽어 덮어쓴다(`-PktLoss=`, `-PktLagMin=`, `-PktIncomingLoss=` 같은 `-Pkt<이름>=`와 `-PktEmulationProfile=<이름>`). 드라이버 정의 이름을 앞에 붙인 키(`GameNetDriverPktLoss=`)가 있으면 그것을 먼저 쓴다. 연결이 만들어질 때 드라이버의 값을 복사한다 | `Engine/Source/Runtime/Engine/Private/NetDriver.cpp:764-789`(`InitPacketSimulationSettings`), `Private/Net/NetEmulationHelper.cpp:384-415`(`LoadConfig`), `534-626`(`ParseSettings`), `Private/NetConnection.cpp:582-585, 691-694, 5500-5504`(`UpdatePacketSimulationSettings`) |
| 프로필은 `Engine.ini`의 `[PacketSimulationProfile.<이름>]` 섹션이다. 읽을 때 설정을 모두 0으로 되돌린 뒤 섹션의 값만 넣는다. 엔진 `BaseEngine.ini`에 `Off`, `Average`(손실 1%, 지연 30\~60ms, 양방향), `Bad`(손실 5%, 지연 100\~200ms, 양방향), `BufferBloat`가 있다. 적용하면 `LogNet`에 `Applying EmulationProfile <이름>`, 섹션이 없으면 `EmulationProfile [...] was not found`를 남긴다 | `NetEmulationHelper.cpp:417-461`(`LoadEmulationProfile`), `540-545`, `Engine/Config/BaseEngine.ini:3533-3568` |
| 송신: `FlushNet`에서 패킷을 보내기 직전에 `PktLoss` 확률로 버리고, 지연 설정이 있으면 보낼 시각을 붙여 `Delayed` 배열에 넣는다. 늦춘 패킷은 연결의 `Tick`에서 시각이 지난 것부터 보내고, 첫 번째로 시각이 안 된 패킷에서 멈춰 순서를 지킨다(`PktJitter`일 때만 순서를 바꾼다). 그래서 지연은 틱 간격(서버 30Hz, 클라이언트 약 28fps) 단위로 반올림된다 | `NetConnection.cpp:2497-2503`, `2601-2693`(`CheckOutgoingPacketEmulation`), `2697-2705`(`ShouldDropOutgoingPacketForLossSimulation`), `4796`, `5156-5192`(`UpdateDelayedPackets`) |
| 수신: 받은 패킷을 처리하기 전에 `PktIncomingLoss` 확률로 버리고, `PktIncomingLagMin`\~`Max` 사이의 난수만큼 늦춰 `DelayedIncomingPackets`에 넣는다. 늦춘 패킷은 월드 틱의 `PostTickDispatch`에서 시각이 지난 것부터 처리하고 역시 순서를 지킨다. 클라이언트의 서버 연결도 이 경로를 탄다 | `NetConnection.cpp:3184-3229`(`CheckIncomingPacketEmulation`), `2304-2312`, `2376-2408`(`ReinjectDelayedPackets`), `NetDriver.cpp:2985` |
| 패킷을 잃으면 그 패킷에 담긴 프로퍼티는 재전송 대상으로 표시되고, 그 액터가 다음에 리플리케이트될 때 그때의 현재 값으로 간다. 다음 차례는 `NetUpdateFrequency`로 정해지므로 손실 하나가 갱신 간격을 약 두 배로 늘린다(NPC는 약 134 → 268ms) | 13절의 마지막 줄, `NetDriver.cpp:5319`(`NextUpdateTime`), `DataReplication.cpp:1872`(`NumNaks`가 있으면 건너뛰지 않음) |
