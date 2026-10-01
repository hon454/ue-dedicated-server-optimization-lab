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
- **클라이언트가 시작 직후 엔진 내부 단언으로 죽을 수 있다.** `smoke1`에서 0번 클라이언트가 첫 프레임 전에 `Assertion failed: RefCount.load(std::memory_order_relaxed) == 0`(`Engine/Source/Runtime/Core/Private/Async/InheritedContext.cpp:130`, DDC IO 스레드)로 종료했다. 이 프로젝트의 코드가 실행되기 전이다. 네 번 실행 중 한 번 일어났다. 스크립트는 클라이언트가 서버보다 먼저 끝나면 바로 실패로 처리하므로, 같은 일이 생기면 새 라벨로 다시 실행한다.
- **`-server`, `-game` 실행에서 Python 시작 스크립트가 오류를 낸다.** `DSOptLab.uproject`가 켠 `AllToolsets`(에디터 전용 실험 플러그인 묶음)의 `Content/Python/init_unreal.py`가 시작할 때 실행되는데, 이 스크립트들이 쓰는 `unreal.ToolsetDefinition`, `unreal.AgentSkill`, `unreal.PythonTestRunner`는 `ToolsetRegistry` 모듈(`ToolsetRegistry.uplugin`, `Type: Editor`)에 있어 `-server`, `-game`에서는 로드되지 않는다. 그래서 실행마다 `LogPython: Error`가 58줄 남았다(`smoke5`, `smoke6`의 서버와 클라이언트 로그). 오류는 시작할 때만 나고 이 프로젝트의 코드와는 관계없다. 이 프로젝트는 Python을 쓰지 않는다(`Source`, `Config`, `Content`에 Python 관련 내용 없음). 사용자가 `AllToolsets`와 `ModelContextProtocol`을 에디터에서 쓰고 있어(2026-10-01 사용자 확인) `.uproject`는 그대로 두고, 실행 스크립트의 서버와 클라이언트 인자에 `-DisablePython`(`PythonScriptPlugin.cpp:116`, `IsPythonEnabled`)을 넣었다. `smoke7`에서 세 로그 모두 `LogPython: Error`가 0줄이다. 스크립트를 거치지 않고 `UnrealEditor.exe -server`를 직접 띄우면 같은 오류가 다시 난다.
  - **`.uproject`에서 에디터 전용으로 제한해도 소용없다.** 플러그인 참조의 `TargetAllowList`는 빌드 타깃 종류로 거른다(`PluginReferenceDescriptor.cpp:63-84`, `IsEnabledForTarget`). 이 프로젝트는 `-server`, `-game`도 `UnrealEditor.exe`로 실행하므로 타깃은 항상 `Editor`라 걸러지지 않는다. 실행 모드로 거르는 것은 모듈 단위의 `Type: Editor`이고(`ModuleDescriptor.cpp:723-732`, `GIsEditor`일 때만 로드), 두 플러그인의 에디터 모듈은 이미 그렇게 되어 있다. MCP HTTP 서버를 자동으로 여는 코드도 에디터 모듈(`ModelContextProtocolEditor.cpp:64-68`)에 있어 서버와 클라이언트에서는 포트를 열지 않는다(`smoke7` 로그에 MCP 수신 대기 줄 없음). 새던 것은 플러그인 `Content/Python`의 시작 스크립트뿐이고, 이것은 `-DisablePython`이 막는다.
  - **측정 조건이 바뀐 것이다.** Python이 켜져 있으면 플러그인이 코어 티커를 등록해 매 프레임 `Tick`을 부른다(`PythonScriptPlugin.cpp:1376-1379`). 끄면 서버 프레임에서 이 비용이 빠진다. `smoke7-r1`의 `work_avg_ms` 2.741은 `smoke2`~`smoke5`의 범위(2.581~2.787) 안이라 이 규모에서는 차이가 보이지 않았다. 기준선(태스크 8) 전에 바꿨으므로 비교가 어긋나지 않는다. 앞으로의 측정은 모두 `-DisablePython`이 들어간 `run-scenario.ps1`로만 실행한다. `smoke6`까지의 실행은 Python이 켜진 조건이다. 클라이언트 로그 끝의 `LogNet: Error: ... Host closed the connection.`은 측정이 끝나 서버가 종료하면서 남는 것이다.

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
