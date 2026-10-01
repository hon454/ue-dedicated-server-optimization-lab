# 엔진 소스 확인 기록 (UE 5.8.3)

- 확인일: 2026-10-01
- 엔진: `G:\Epic Games\UE_Source` (git 태그 `5.8.3-release`, `Engine/Build/Build.version`의 5.8.3)
- 경로는 엔진 루트 기준이다. 줄 번호는 이 태그의 것이다.
- 구현 계획 태스크 2.5의 결과다. 포스팅 0의 "엔진에서 확인한 것"과 이후 코드의 근거로 쓴다.

## 0. 프로젝트 구성

계획의 초안은 `Game/` 폴더 아래의 `ServerLab` 프로젝트와 접두사 `SL`을 가정했다. 실제 구성은 아래와 같고, 2026-10-01에 설계 문서 4절과 구현 계획을 여기에 맞춰 고쳤다.

| 항목 | 값 |
| --- | --- |
| 프로젝트 | `DSOptLab.uproject` (레포 루트가 프로젝트 폴더) |
| 모듈과 빌드 타깃 | `DSOptLab` (`Source/DSOptLab/`, `DSOPTLAB_API`), `DSOptLabEditor` |
| 게임 모드 경로 | `/Script/DSOptLab.LabGameMode` |
| 접두사 | `Lab`. 클래스(`ALabResourceNode`), 실행 인자(`-LabNodes=`), 폴더(`Saved/LabMetrics/`, `Saved/Screenshots/Lab/`), 로그 카테고리(`LogLabMetrics`), 북마크(`Lab_MeasureStart`) |
| 엔진 연결 | `EngineAssociation`이 `UE_DSOptLab`(이 PC의 레지스트리에서 `G:\Epic Games\UE_Source`). 스크립트는 `Scripts/common.ps1`에서 같은 값을 찾아 쓴다. 런처 설치본 `G:\Epic Games\UE_5.8`과 섞어 쓰면 같은 `Binaries/`에 번갈아 빌드하게 되므로 쓰지 않는다 |
| 문서 | `Docs/STATUS.md`, `Docs/Planning/` |
| 포스팅 | `Posts/NN-이름/` |

`.gitignore`는 레포를 만들 때 들어간 GitHub의 Unreal 템플릿(루트 기준)을 그대로 쓰고 `.idea/`, `*.slnx`, `*.utrace`만 더했다.

## 가. 기본값과 이름

| 항목 | 찾은 값 | 위치 | 결론 |
| --- | --- | --- | --- |
| 서버 틱 기본값 | `NetServerMaxTickRate=30` | `Engine/Config/BaseEngine.ini:1867` (`[/Script/OnlineSubsystemUtils.IpNetDriver]`) | 기억값 30과 같다. 틱 예산 1000 ÷ 30 = 33.3ms |
| 서버 틱의 적용 | Dedicated Server면 `MaxTickRate = Clamp(NetDriver->GetNetServerMaxTickRate(), 1, 1000)` | `Engine/Source/Runtime/Engine/Private/GameEngine.cpp:1719-1765` (`UGameEngine::GetMaxTickRate`) | 실행 중 적용값은 태스크 7.4에서 프레임 수로 확인한다 |
| 컬 거리 기본값 | `SetNetCullDistanceSquared(225000000.0f)` | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` | 기억값과 같다. √225,000,000 = 15,000cm = 150m |
| 업데이트 빈도 기본값 | `SetNetUpdateFrequency(100.0f)`, `SetMinNetUpdateFrequency(2.0f)` | `Actor.cpp:295-296` | 기억값과 같다 |
| 설정자 | `SetNetUpdateFrequency`, `SetMinNetUpdateFrequency`, `SetNetCullDistanceSquared`, `SetNetDormancy`, `FlushNetDormancy`, `SetReplicatingMovement` 모두 있음. 멤버 `NetCullDistanceSquared`, `NetUpdateFrequency`, `MinNetUpdateFrequency`의 직접 접근은 5.5부터 사용 중단 | `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:898-910, 3174, 3178, 4557, 4624, 4636, 4648` | 계획의 코드 그대로 쓴다. `NetDormancy`(869줄)는 사용 중단 표시가 없는 public 멤버라 생성자에서 직접 대입할 수 있다 |
| 연결당 송신 한도(엔진 기본값) | `ConfiguredInternetSpeed=100000`, `ConfiguredLanSpeed=100000`, `MaxClientRate=100000`, `MaxInternetClientRate=100000` (바이트/초) | `BaseEngine.ini:1839-1840` (`[/Script/Engine.Player]`), `1860-1861` (`[/Script/OnlineSubsystemUtils.IpNetDriver]`) | 기본 한도는 연결당 초당 100,000바이트다. 계획 8.4a의 사전 추정(135~180KB/s)보다 낮으므로 기준선이 포화될 가능성이 높다 |
| 한도가 정해지는 과정 | 클라이언트가 자기 `ConfiguredInternetSpeed`를 `NMT_Netspeed`로 보내고, 서버가 `Clamp(Rate, 1800, NetDriver->MaxClientRate)`로 받는다. LAN이 아니면 서버의 `MaxClientRate`는 `MaxInternetClientRate`로 낮춰진다 | `Engine/Source/Runtime/Engine/Private/NetConnection.cpp:588`, `PendingNetGame.cpp:396`, `World.cpp:7465-7473`, `World.cpp:7987-7990` | **한도를 올리려면 세 키를 모두 올려야 한다**: `[/Script/Engine.Player] ConfiguredInternetSpeed`(클라이언트가 읽는다), `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`와 `MaxInternetClientRate`(서버가 읽는다). 서버 로그에 `Client netspeed is N`이 찍힌다 |
| `OutTotalBytes` | `int32`. `FlushNet`에서 패킷마다 `SendBuffer.GetNumBytes() + PacketOverhead`를 더한다 | `Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h:571`, `NetConnection.cpp:2562, 2586` | 패킷 헤더와 오버헤드를 포함한 송신 바이트의 누계다. `int32`라서 연결 하나가 약 2.1GB를 넘기면 넘친다. 10MB/s로 110초를 보내도 1.1GB라 이 시나리오에서는 넘치지 않는다 |
| `ActorChannelsNum()` | `ActorChannels.Num()` | `NetConnection.h:751` | 계획 그대로 쓴다 |
| `CurrentNetSpeed` | public `int32` | `NetConnection.cpp:588-597` | 계획 그대로 쓴다 |
| `IsNetReady` | `bool IsNetReady() const`. 인자를 받는 `IsNetReady(bool bSaturate)`는 5.6부터 사용 중단. 판정은 `QueuedBits + SendBuffer.GetNumBits() <= 0` | `NetConnection.h:1085-1089`, `NetConnection.cpp:2731-2751` | **계획과 다르다.** `IsNetReady(false)` 대신 `IsNetReady()`를 쓴다 |
| `QueuedBits` | 보낼 때 `PacketBytes * 8`을 더하고, 연결 틱마다 `CurrentNetSpeed × 경과 시간 × 8`을 뺀다 | `NetConnection.cpp:2574, 5122-5131` | 양수면 한도 초과다 |
| 채널 수 상한 | `DefaultMaxChannelSize(32767)`, 콘솔 변수 `net.MaxChannelSize`(기본 0이면 앞의 값) | `NetConnection.cpp:80, 405, 450-454` | 출발값(약 5,310개)은 상한 안이다. 노드 20,000개까지도 안이다 |
| 종료 코드를 주는 종료 | `static void RequestExitWithStatus(bool Force, uint8 ReturnCode, const TCHAR* CallSite = nullptr)` | `Engine/Source/Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h:1096` | 계획 그대로 쓴다 |

## 나. 동작 순서와 방식

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

결론: **계획과 다르다.** 계획의 태스크 5.2는 클라이언트의 `TickTopDown`에서만 플래그를 껐다. 그러면 플래그를 끄기 전에(스폰 직후 낙하 중에) 이미 보낸 카메라 위치가 서버의 캐시에 남을 수 있고, 서버 쪽 플래그는 참이라 서버가 캐시를 스스로 갱신하지 않는다. 그래서 `ALabPlayerController::SpawnPlayerCameraManager`를 재정의해 서버와 클라이언트 양쪽에서, 모든 플레이어에 대해 `bUseClientSideCameraUpdates = false`로 둔다. 서버는 폰을 뷰 타깃으로 계산한 3인칭 카메라 위치(폰 뒤 수 미터)를 기준으로 판정하고, 내려다보기 카메라는 서버에 전달되지 않는다. 모든 클라이언트가 같은 방식이라 클라이언트마다 기준이 다르지 않다. 포스팅 0에 이 사실을 적는다.

### 그 밖의 시그니처

| 항목 | 5.8.3 | 위치 | 결론 |
| --- | --- | --- | --- |
| 월드 서브시스템 | `virtual bool ShouldCreateSubsystem(UObject* Outer) const override`, `virtual void OnWorldBeginPlay(UWorld& InWorld)`, `virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const`(protected) | `Engine/Source/Runtime/Engine/Public/Subsystems/WorldSubsystem.h:34, 43, 66` | 계획 그대로 |
| 월드 틱 델리게이트 | `FOnWorldTickStart`, `FOnWorldPostActorTick` 모두 `(UWorld*, ELevelTick, float)` | `Engine/Source/Runtime/Engine/Classes/Engine/World.h:4516-4527` | 계획 그대로 |
| 디버그 점 | `DrawDebugPoint(const UWorld*, FVector const& Position, float Size, FColor const&, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0)` | `Engine/Source/Runtime/Engine/Public/DrawDebugHelpers.h:24` | 계획 그대로. `SDPG_Foreground`를 마지막 인자로 준다 |
| 스크린샷 | `FScreenshotRequest::RequestScreenshot(const FString& InFilename, bool bInShowUI, bool bAddFilenameSuffix, ...)` | `Engine/Source/Runtime/Engine/Public/UnrealClient.h:219` | 계획 그대로 |

### 업데이트 빈도의 스케줄링

- `ServerReplicateActors_BuildConsiderList`가 `World->TimeSeconds <= ActorInfo->NextUpdateTime`인 액터를 건너뛴다(`NetDriver.cpp:5319-5323`).
- 다음 시각은 `NextUpdateTime = TimeSeconds + RandDelay + NextUpdateDelta`이고, `NextUpdateDelta`는 적응형이 꺼져 있으면 `1 / NetUpdateFrequency`다(`NetDriver.cpp:5420-5425`). `RandDelay`는 `0 ~ ServerTickTime` 사이의 난수다.
- `net.UseAdaptiveNetUpdateFrequency`의 기본값은 0이다(`NetDriver.cpp:523-526`).

결론: 정적 노드는 적응형 감소를 받지 않는다. 기본 빈도 100Hz는 서버 틱 30Hz보다 높으므로, 노드와 NPC 모두 매 틱(또는 `RandDelay` 때문에 한 틱 건너) 고려 대상이 된다. 포스팅 4에서 NPC 빈도를 10으로 낮추면 고려 횟수가 약 3분의 1이 된다(30Hz 기준 계산값).

### 휴면 액터와 관련성

- `ServerReplicateActors_PrioritizeActors`는 채널이 없는 액터에만 관련성을 먼저 검사하고(`NetDriver.cpp:5580-5593`), 그 뒤 이 연결에 대해 휴면이면 건너뛴다(`5618-5624`, `IsActorDormant`는 `ActorInfo->DormantConnections.Contains`, `5499-5503`).
- 휴면 진입은 채널이 있어야 한다(`ShouldActorGoDormant`, `NetDriver.cpp:5506-5526`). 실행 중 스폰한 액터는 먼저 채널이 열려 초기 상태가 전송된 뒤에 휴면에 들어간다.
- 관련성을 잃은 액터의 채널을 닫는 코드는 `Channel != NULL`일 때만 동작한다(`NetDriver.cpp:5877-5889`).

결론: 계획의 가정과 같다. 휴면으로 채널이 닫힌 액터는 관련성 밖으로 나가도 서버가 닫을 채널이 없어서 클라이언트에 남는다. 포스팅 3에서 내려다보기 화면에 지나온 길을 따라 점이 남는 것이 이 동작이다. 실행으로는 포스팅 3에서 확인한다.

## 다. 템플릿에서 남긴 것 (태스크 2.6, 2026-10-01 정리)

5.8의 TPP 템플릿은 `Variant_Combat`, `Variant_Platforming`, `Variant_SideScrolling` 코드와 콘텐츠, 템플릿 레벨을 함께 만든다. 쓰지 않는 것을 지우고 남긴 코드에 접두사 `Lab`을 붙였다.

| 남긴 것 | 내용 |
| --- | --- |
| `Source/DSOptLab/LabCharacter.h/.cpp` | 템플릿의 `ADSOptLabCharacter`를 `ALabCharacter`로 이름만 바꿨다. `BP_ThirdPersonCharacter`의 부모 클래스다 |
| `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter` | 플레이어 폰. `ALabGameMode`가 경로 `/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`로 읽는다 |
| `Content/Characters/Mannequins/` | 위 블루프린트가 참조하는 메시, 머티리얼, 텍스처, 릭, 기본 이동 애니메이션 46개 |
| `Content/Input/` | 입력 액션 4개, 매핑 2개, 터치 인터페이스 1개 |
| `Content/Maps/L_Lab` | 태스크 1.4의 맵 |

- 남길 에셋은 `L_Lab`, `BP_ThirdPersonCharacter`, `IMC_Default`, `IMC_MouseLook`에서 에셋 파일 안의 `/Game/...` 경로 문자열을 따라가 정했다. 754개 중 55개(89.6MB)가 남았다. 제거 뒤 `smoke5-r1`이 종료 코드 0으로 끝났고 서버와 클라이언트 로그에 로드 실패가 없었다.
- **리다이렉트.** `BP_ThirdPersonCharacter`는 부모 클래스를 템플릿 원본 이름 `/Script/TP_ThirdPerson.TP_ThirdPersonCharacter`로 저장하고 있다(에셋의 이름 표에서 확인). `Config/DefaultEngine.ini`의 `ActiveGameNameRedirects`(`TP_ThirdPerson` → `/Script/DSOptLab`)와 `ActiveClassRedirects`(`TP_ThirdPersonCharacter` → `LabCharacter`)가 이것을 `ALabCharacter`로 잇는다. 이 두 줄을 지우면 폰이 로드되지 않는다. 블루프린트를 에디터에서 다시 저장하면 새 이름이 에셋에 들어가지만, 하지 않아도 동작한다.
- 템플릿의 `ADSOptLabPlayerController`와 `ADSOptLabGameMode`, 그 블루프린트는 지웠다. 템플릿 컨트롤러가 블루프린트에서 지정하던 입력 매핑(`IMC_Default`, `IMC_MouseLook`)은 `ALabPlayerController`가 생성자에서 읽어 `SetupInputComponent`에서 등록한다. 터치 조작 위젯은 옮기지 않았다.
- 모듈 의존성에서 `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`를, `.uproject`에서 `StateTree`, `GameplayStateTree` 플러그인을 뺐다.

## 마. 첫 실행에서 확인한 것 (태스크 7.3~7.7, 2026-10-01)

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
- **클라이언트가 시작 직후 엔진 내부 단언으로 죽을 수 있다.** `smoke1`에서 0번 클라이언트가 첫 프레임 전에 `Assertion failed: RefCount.load(std::memory_order_relaxed) == 0`(`Engine/Source/Runtime/Core/Private/Async/InheritedContext.cpp:130`, DDC IO 스레드)로 종료했다. 이 프로젝트의 코드가 실행되기 전이다. 네 번 실행 중 한 번 일어났다. 콜스택은 `DDC IO ThreadPool #1` 스레드의 `FMemoryCacheStore::Get` 아래 mimalloc이고 `HttpConnectionPool` 스레드도 함께 죽었다(`Saved/Logs/client0-smoke1-r1.log:1461-1509`). 130줄은 참조 중인 확장 데이터를 해제할 때 걸리는 검사라 DDC 요청 경로의 수명 경합으로 보이지만 추정이다. 크래시 뒤 2.2초 만에 프로세스가 끝났다. 지금까지 약 27번의 프로세스 실행 중 1번이다(`smoke1`~`smoke7`과 수동 실행). 스크립트는 시작 신호 전에 죽은 클라이언트를 같은 인자로 다시 띄우고(실행당 최대 3번), 시작 신호 뒤에 죽으면 실패로 처리한다. 다시 띄우는 경로는 아직 실행으로 확인하지 않았다.
- **`-server`, `-game` 실행에서 Python 시작 스크립트가 오류를 낸다.** `DSOptLab.uproject`가 켠 `AllToolsets`(에디터 전용 실험 플러그인 묶음)의 `Content/Python/init_unreal.py`가 시작할 때 실행되는데, 이 스크립트들이 쓰는 `unreal.ToolsetDefinition`, `unreal.AgentSkill`, `unreal.PythonTestRunner`는 `ToolsetRegistry` 모듈(`ToolsetRegistry.uplugin`, `Type: Editor`)에 있어 `-server`, `-game`에서는 로드되지 않는다. 그래서 실행마다 `LogPython: Error`가 58줄 남았다(`smoke5`, `smoke6`의 서버와 클라이언트 로그). 오류는 시작할 때만 나고 이 프로젝트의 코드와는 관계없다. 이 프로젝트는 Python을 쓰지 않는다(`Source`, `Config`, `Content`에 Python 관련 내용 없음). 사용자가 `AllToolsets`와 `ModelContextProtocol`을 에디터에서 쓰고 있어(2026-10-01 사용자 확인) `.uproject`는 그대로 두고, 실행 스크립트의 서버와 클라이언트 인자에 `-DisablePython`(`PythonScriptPlugin.cpp:116`, `IsPythonEnabled`)을 넣었다. `smoke7`에서 세 로그 모두 `LogPython: Error`가 0줄이다. 스크립트를 거치지 않고 `UnrealEditor.exe -server`를 직접 띄우면 같은 오류가 다시 난다.
  - **`.uproject`에서 에디터 전용으로 제한해도 소용없다.** 플러그인 참조의 `TargetAllowList`는 빌드 타깃 종류로 거른다(`PluginReferenceDescriptor.cpp:63-84`, `IsEnabledForTarget`). 이 프로젝트는 `-server`, `-game`도 `UnrealEditor.exe`로 실행하므로 타깃은 항상 `Editor`라 걸러지지 않는다. 실행 모드로 거르는 것은 모듈 단위의 `Type: Editor`이고(`ModuleDescriptor.cpp:723-732`, `GIsEditor`일 때만 로드), 두 플러그인의 에디터 모듈은 이미 그렇게 되어 있다. MCP HTTP 서버를 자동으로 여는 코드도 에디터 모듈(`ModelContextProtocolEditor.cpp:64-68`)에 있어 서버와 클라이언트에서는 포트를 열지 않는다(`smoke7` 로그에 MCP 수신 대기 줄 없음). 새던 것은 플러그인 `Content/Python`의 시작 스크립트뿐이고, 이것은 `-DisablePython`이 막는다.
  - **측정 조건이 바뀐 것이다.** Python이 켜져 있으면 플러그인이 코어 티커를 등록해 매 프레임 `Tick`을 부른다(`PythonScriptPlugin.cpp:1376-1379`). 끄면 서버 프레임에서 이 비용이 빠진다. `smoke7-r1`의 `work_avg_ms` 2.741은 `smoke2`~`smoke5`의 범위(2.581~2.787) 안이라 이 규모에서는 차이가 보이지 않았다. 기준선(태스크 8) 전에 바꿨으므로 비교가 어긋나지 않는다. 앞으로의 측정은 모두 `-DisablePython`이 들어간 `run-scenario.ps1`로만 실행한다. `smoke6`까지의 실행은 Python이 켜진 조건이다. 클라이언트 로그 끝의 `LogNet: Error: ... Host closed the connection.`은 측정이 끝나 서버가 종료하면서 남는 것이다.

## 바. 태스크 8 시작 전 점검에서 확인한 것 (2026-10-01)

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

## 사. 같은 구성의 실행 사이에 서버 속도가 세 배 달라지는 원인 (2026-10-01)

모든 실행은 클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 트레이스 켬, `net_speed` 350000이다(`calib-b-r1`만 10000000). 엔진 소스가 아니라 이 PC에서의 실행으로 확인한 것이다.

**증상.** 서버가 약 1.5Hz로 도는 느린 상태와 약 5.5Hz로 도는 빠른 상태 둘 중 하나에 있고, 실행 도중에도 오간다. 서버 로그의 5초 간격 줄에서 프레임 번호의 차로 계산했다. `calib-b-r1`과 `calib-d-r1`은 내내 느렸고, `calib-e-r1`은 준비 구간 끝에 빨라졌고, `calib-c-r1`과 `diag-a-r1`은 측정 중에 여러 번 오갔다.

**확인한 것.**

| 확인 | 결과 | 근거 |
| --- | --- | --- |
| 느린 구간에 특정 코드가 더 도는가 | 아니다. 게임 스레드의 모든 타이머가 프레임당 호출 횟수는 같고 호출당 시간만 2.3~2.9배 길다. `LabResourceNode` 6.9µs 대 2.4µs, `LabNpc` 18.0µs 대 7.2µs, `ServerMovePacked` 234µs 대 99µs, `USkeletalMeshComponent_TickAnimation` 43µs 대 18µs | `calib-e-r1.utrace`의 게임 스레드 타이밍 이벤트를 `UnrealInsights.exe -NoUI -ExecOnAnalysisCompleteCmd`의 `TimingInsights.ExportTimingEvents`로 내보내, 트레이스 시각 65~85초(39프레임)와 125~155초(155프레임)를 비교 |
| 느린 구간에 게임 스레드가 CPU를 얼마나 받는가 | 초당 0.20~0.49초. 빠른 구간에는 0.95~1.01초. 그동안 논리 프로세서 1번의 사용률은 최대치에 붙어 있고, 그 시간을 쓰는 다른 프로세스는 없다 | `diag-a-r1` 실행 중 약 1초 간격으로 서버 스레드별 `TotalProcessorTime`의 차, `\Processor Information(0,N)\% Processor Utility`, 프로세스별 CPU 시간의 차를 기록 |
| 논리 프로세서 1번에 무엇이 있는가 | 시나리오가 도는 동안 DPC가 초당 약 15,000개, 인터럽트가 초당 약 12,000~16,000개 처리되고 DPC 시간이 32~65%다. 0번과 2~7번의 DPC 시간은 0~2%다. UE 프로세스가 없을 때 1번의 DPC는 초당 약 480개다 | `diag-b-r1` 실행 중 `\Processor Information(0,0..7)\% DPC Time`, `DPCs Queued/sec`, `Interrupts/sec` 기록. 이 실행은 내내 빨랐고 1번의 사용자 시간은 대체로 0~11%였다 |
| 서버를 1번에만 고정하면 | 내내 느리다. `frames` 90, `work_avg_ms` 670.713, 5초 간격 틱 1.3~1.5Hz | `diag-c-r1` (`-ServerMask 2`) |
| 서버를 2~7번에 고정하면 | 내내 빠르다. `frames` 340, `work_avg_ms` 176.630, `work_p99_ms` 216.841, 5초 간격 틱 4.7~5.8Hz | `diag-d-r1` (`-ServerMask 252`) |

**결론.** 서버 선호도 마스크 `0xFF`에 들어 있는 논리 프로세서 1번이 시나리오 실행 중 DPC와 인터럽트를 도맡는다. 서버 게임 스레드가 1번에 올라가 있는 동안에는 CPU를 절반 넘게 빼앗긴다. `work`는 경과 시간이라 빼앗긴 시간이 그대로 들어간다. Windows 스케줄러가 스레드를 어느 코어에 두는지가 실행마다, 실행 도중에도 달라져 수치가 세 배까지 흔들렸다.

**확인하지 않은 것.** DPC와 인터럽트를 만드는 장치나 드라이버(루프백 네트워크인지 GPU인지). 재부팅 뒤에도 1번에 몰리는지. 0번(1번과 같은 물리 코어의 SMT 짝)만 쓸 때의 속도.

프레임당 송신량은 상태와 무관하다. 30Hz 환산 송신량이 `calib-b-r1` 172,638, `calib-e-r1` 172,756, `diag-d-r1` 172,271(32,540 × 30 ÷ (340 ÷ 60))이라 고정한 송신 한도 350,000은 그대로 맞다.

## 라. 계획 초안의 코드에서 바꾼 것 요약

| 태스크 | 바꾼 것 | 이유 |
| --- | --- | --- |
| 전체 | 경로, 모듈 이름, 접두사(`SL` → `Lab`) | 0절 |
| 5.1, 5.2 | 생성자에서 입력 매핑을 읽고 `SetupInputComponent`에서 등록 | 다절 |
| 5.2 | `TickTopDown`의 플래그 설정을 지우고 `SpawnPlayerCameraManager` 재정의로 옮김(서버와 클라이언트, 모든 플레이어) | 나절 "관련성 판정의 기준 위치" |
| 5.2 | `PlayerTick` 앞에 `IsLocalController()` 검사 추가, 지역 변수 `Role`을 `View`로 바꿈 | 서버에서 실행되지 않게 함. `AActor::Role` 멤버와 이름이 겹침 |
| 5.2 | `TickTopDown`에서 `bClientSimulatingViewTarget`을 켜고 매 틱 뷰 타깃을 확인 | 마절 |
| 6.2 | `IsNetReady(false)` → `IsNetReady()` | 가절 |
| 2.1~2.3 | `env.ps1` 대신 `common.ps1`이 `EngineAssociation`으로 엔진을 찾음 | 0절 |
| 7.1 | 선호도를 기다리는 동안 다시 설정, 클라이언트가 먼저 죽으면 바로 실패 | 마절 |
| 7.1, 7.2 | 스크립트를 UTF-8 BOM으로 저장 | `powershell`(5.1)이 BOM 없는 한글을 잘못 읽음 |
| 8.4a | 올릴 설정 키가 세 개 | 가절 "한도가 정해지는 과정" |
| 7.1, 7.2 | 서버와 클라이언트 인자에 `-DisablePython` 추가 | 마절 |
| 6.2 | `saturated_ratio`를 프레임 끝의 `IsNetReady()`에서 엔진의 포화 기록(`GetSaturationAnalytics`)으로 바꿈. 5초 간격 로그의 `saturated=`가 `saturated_replications=끊긴 횟수/시도 횟수`가 됨 | 바절 "포화를 판정하는 시점" |
| 7.1 | 시작 신호 전에 죽은 클라이언트를 다시 띄움(실행당 최대 3번). 서버 종료 시점의 클라이언트 생존, `.utrace` 존재, 측정 시작 뒤의 선호도 재설정, 라벨의 로그와 트레이스 재사용, 이미 떠 있는 `UnrealEditor`를 검사. 클라이언트와 `-NoTrace` 서버에 `-traceautostart=0` | 마절(단언 크래시), 바절 |
| 8.4, 8.4a | 포화를 먼저 없앤 뒤 예산 초과를 판단. 한도는 규모가 정해진 뒤 30Hz 환산 송신량의 약 두 배로 한 번만 고정 | 바절 "대역폭 예산과 서버 틱" |
