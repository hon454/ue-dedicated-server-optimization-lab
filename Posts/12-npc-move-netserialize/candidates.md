# 포스팅 12 후보 기법 자료 (NPC 이동의 `NetSerialize`)

2막 구현 계획 단계 5의 2번(관찰 자료)이다. 기준 구성은 포스팅 11의 최종 구성(`-NpcInterpDelay 150`)이다. 수치는 포스팅 11의 묶음 `act2-interp2`에서 읽었다. 기준 묶음은 기법을 구현한 뒤 적용 묶음과 연달아 다시 잰다. 품질 지표는 [ADR-0020](../../Docs/Decisions/0020-npc-motion-quality-metrics.md)의 지표다. 아래의 예상은 모두 계산값이고 확인하지 않았다.

## 1. 지금 서버가 NPC 갱신마다 보내는 이동 데이터

### 1.1 엔진 소스로 셈한 비트 구성

`ALabNpc`는 `SetReplicatingMovement(true)`로 엔진의 `ReplicatedMovement`(`FRepMovement`)를 보내고, 포스팅 11의 보간이 쓰는 `ServerFrame`(`uint8`)을 따로 보낸다(`Source/DSOptLab/LabNpc.cpp`). `FRepMovement::NetSerialize`가 쓰는 비트는 다음과 같다(`Engine/Source/Runtime/Engine/Private/Engine/ReplicatedState.cpp:67-152`).

| 순서 | 내용 | 비트 | 소스 |
| --- | --- | ---: | --- |
| 1 | 플래그 4개(물리 휴면, 물리, 서버 프레임 있음, 물리 핸들 있음) | 4 | `ReplicatedState.cpp:73-74`. 연결 버전이 `RepMoveServerFrameAndHandle`(25) 이상이면 4비트(`Core/Public/Misc/EngineNetworkCustomVersion.h:40`) |
| 2 | 위치: 축당 비트 수를 적는 헤더 | 7 | `Net/Core/Private/Net/Core/Serialization/QuantizedVectorSerialization.cpp:90-92`. `SerializeInt(값, 128)`은 7비트(`Core/Private/Serialization/BitWriter.cpp:142-146`) |
| 3 | 위치: X, Y, Z를 같은 비트 수 N으로 | 3 × N | `QuantizedVectorSerialization.cpp:94-96`. N은 세 축 가운데 가장 큰 절댓값(cm 단위로 반올림)에 부호 비트를 더한 길이다(`13-17`) |
| 4 | 회전: 축마다 "0이 아님" 1비트, 0이 아니면 1바이트 | 3 + 8 | `Core/Private/Math/UnrealMath.cpp:84-` (`TRotator::SerializeCompressed`). NPC는 평면에서만 돌아서 Pitch, Roll이 0이다(`LabNpc.cpp`의 `Direction.Rotation()`, `Direction.Z`가 0) |
| 5 | 선속도: 헤더 7비트와 축마다 1비트 | 7 + 3 | NPC의 속도는 늘 0이다([포스팅 11 관찰 자료](../11-npc-interpolation/candidates.md) 1절). 0은 축마다 1비트(`QuantizedVectorSerialization.cpp:13-17`) |
| 6 | 가속도 있음 1비트 | 1 | `ReplicatedState.cpp:133-137`(`RepMoveOptionalAcceleration`, 버전 35) |

- 합계는 33 + 3N비트이고, 프로퍼티 핸들 8비트(`RepLayout.cpp:1922-1932`의 `SerializeIntPacked`)를 더하면 41 + 3N비트다.
- 엔진의 정밀도 설정은 기본값이 가장 거친 단계다. 위치와 속도는 `RoundWholeNumber`(1cm), 회전은 `ByteComponents`(약 1.41°)다(`ReplicatedState.cpp:34-36`). 더 거친 단계는 열거형에 없다(`Engine/Source/Runtime/Engine/Classes/Engine/ReplicatedState.h:11-28`).
- 위치의 N은 맵 원점에서 멀수록 커진다. 원점에서 163m(16,383cm)까지는 N이 15 이하이고, 655m(65,535cm)를 넘으면 18이다. 이 테스트베드에서는 플레이어 무리가 맵 가운데에 있어서 받는 NPC가 원점 가까이에 있다(`LabGameMode.cpp`의 `GetSlotLocation`).

### 1.2 기록으로 셈한 N의 분포

`act2-interp2-r1`의 모션 기록에서 클라이언트 8개가 측정 구간 60초 동안 받은 NPC 갱신 205,279번마다, 그 순간의 서버 위치로 N을 셈했다(`server.bin`의 NPC 위치, `clientK.bin`의 받은 시각과 NetGUID, `FPlatformTime::Cycles64`가 프로세스 사이에 같은 시계라는 가정). 계산 스크립트는 남기지 않았고 식은 1.1절이다.

| N | 비율 | 이동 데이터(핸들 포함) |
| ---: | ---: | ---: |
| 12 이하 | 7.1% | 77비트 이하 |
| 13 | 19.3% | 80비트 |
| 14 | 59.4% | 83비트 |
| 15 | 11.9% | 86비트 |
| 16 | 2.5% | 89비트 |
| **평균** | 13.81 | **82.43비트** |

- 클라이언트 하나가 1초에 받는 NPC 갱신은 419.3\~431.6번, 평균 427.7번이다. `act2-interp-base2-r1`도 같았다(평균 427.8번, N 평균 13.81).
- 포스팅 11에서 Networking Insights의 패킷 하나로 읽은 NPC 하나의 `ReplicatedMovement`(`Shared`)는 83비트였다([포스팅 11 측정 기록](../11-npc-interpolation/measurements.md) 5.1절). N = 14일 때의 41 + 42 = 83비트와 같다. `Shared`에 핸들이 들어 있다는 것은 `ServerFrame`의 `Shared` 16비트(핸들 8 + 값 8)와 맞는다(`RepLayout.cpp:2741-2752`의 `WriteSharedProperty`가 핸들을 함께 쓴다).
- Networking Insights 표의 `ReplicatedMovement` 평균 94비트에는 플레이어 캐릭터의 이동이 섞여 있어서 NPC 몫이 아니다.

### 1.3 NPC에게 필요한 것과 아닌 것

NPC 갱신 한 번은 117.20비트이고(`act2-interp2-r1`, Networking Insights), 그중 이동과 프레임 번호가 약 98.4비트(82.43 + 16)다. N = 14인 갱신의 83비트를 나누면 다음과 같다.

| 값 | 비트 | NPC에게 |
| --- | ---: | --- |
| X, Y | 28 | 필요 |
| Yaw | 8 | 필요(클라이언트가 보간한다) |
| 핸들 | 8 | 프로퍼티 하나에 하나 |
| Z | 14 | 늘 50cm라 필요 없음(`LabGameMode.cpp:116, 296`의 스폰 높이, `TickMovement`가 Z를 바꾸지 않음. 모션 기록의 서버 위치도 모두 50) |
| 속도 | 10 | 늘 0이라 필요 없음 |
| 위치 헤더 | 7 | 범위를 알면 필요 없음 |
| 플래그, 가속도 있음, Pitch와 Roll 있음 | 7 | 늘 같은 값이라 필요 없음 |

필요 없는 값이 38비트다. `ServerFrame`의 핸들 8비트도 이동과 한 구조체에 넣으면 없어진다.

## 2. 후보

모두 같은 틀이다. `ALabNpc`가 `ReplicatedMovement`를 끄고(`SetReplicatingMovement(false)`), NPC 이동만 담은 구조체 `FLabNpcMove`를 `NetSerialize`로 보낸다. 다른 점은 위치의 범위와 담는 값이다.

| 후보 | 위치 | 회전 | 갱신 한 번(핸들 포함) | 지금 대비 | 연결당 송신 대역폭 변화(예상) |
| --- | --- | --- | ---: | ---: | ---: |
| 지금 | `FRepMovement` 41 + 3N, `ServerFrame` 16 | Yaw 8비트 | 98.4비트 | | |
| A. 맵 전체 범위의 고정 길이 | X, Y를 맵 범위 ±980m에서 1cm로, 18비트씩 | Yaw 8비트 | 60비트 | -38.4비트 | -2,055바이트/초(-16.2%) |
| B. 집 기준 상대 좌표 | X, Y를 집에서 ±30m 범위에서 1cm로, 13비트씩 | Yaw 8비트 | 50비트 | -48.4비트 | -2,589바이트/초(-20.4%) |
| C. B에서 회전 빼기 | B와 같음 | 보내지 않음. 클라이언트가 보간한 이동 방향으로 정함 | 42비트 | -56.4비트 | -3,017바이트/초(-23.8%) |

- 세 후보 모두 서버 프레임 번호 8비트를 구조체에 넣는다. 그래서 `ServerFrame`의 핸들 8비트가 없어진다. 따로 두면 세 후보 모두 8비트가 더 든다.
- 대역폭 변화는 (줄어든 비트 × 427.7번/초 ÷ 8)이다. 비율은 `act2-interp2`의 `out_bytes_per_sec_per_conn` 중앙값 12,682에 대한 것이다.
- 맵 범위 ±980m는 NPC 배치 범위 ±950m(`LabScenarioConfig.h`의 `WorldHalfExtent` 95,000cm)에 배회 반경 30m(`LabNpc.h`의 `WanderRadius` 3,000cm)를 더한 것이다. 196,001가지라 18비트(262,144)다.
- 집 기준 ±30m는 NPC가 집에서 축마다 ±30m 정사각형 안의 점으로만 걷기 때문이다(`LabNpc.cpp`의 `PickTarget`, 두 점 사이 직선도 정사각형 안). 6,001가지라 13비트(8,192)다.

### A. 맵 전체 범위의 고정 길이

- 구현: X, Y를 (값 + 98,000)으로 바꿔 18비트씩 쓴다. 클라이언트는 같은 식으로 되돌린다. Z는 보내지 않고 클라이언트가 스폰 때 받은 높이를 쓴다.
- 이 테스트베드에서는 X, Y가 엔진보다 비싸다(18비트 대 평균 13.81비트). 엔진은 크기에 맞춰 길이를 줄이고, 고정 길이는 범위 끝에 맞춰야 해서다. 줄어드는 것은 필요 없는 값을 뺀 몫뿐이다.
- 맵 가장자리의 NPC라면 엔진의 N이 18이라(위치 헤더 7 + 54) A가 훨씬 유리하다. 이 테스트베드는 엔진에 유리한 자리다.

### B. 집 기준 상대 좌표

- 구현: 서버가 `Home`(스폰 위치)을 처음 한 번만 보낸다(`COND_InitialOnly`). 구조체는 (위치 - `Home`) + 3,000을 13비트씩 쓴다. 클라이언트는 받은 값에 `Home`을 더한다.
- `NetSerialize`의 결과는 모든 연결이 함께 쓴다(아래 "구조체 공통"). 그래서 기준점은 연결마다 다를 수 없고, NPC마다 하나로 정해진 값이어야 한다. `Home`이 그런 값이다.
- 범위가 맵 위치와 상관없어서 맵 가장자리의 NPC도 같은 50비트다.
- 대가: `Home`을 채널이 열릴 때마다 한 번 보낸다. 배회 규칙(±30m)에 기댄다. 범위를 벗어나면 잘라 보내고 서버 로그에 남기는 검사를 넣는다.

### C. B에서 회전 빼기

- 구현: B에서 Yaw를 빼고, 클라이언트가 보간한 위치의 진행 방향으로 회전을 정한다.
- 이 테스트베드의 NPC는 늘 진행 방향을 본다(`LabNpc.cpp`의 `TickMovement`). 그래서 화면은 같게 보일 것이다. 실제 게임의 NPC는 멈춰서 다른 곳을 보기도 해서 쓰기 어렵다.
- 회전을 받은 값에서 계산한 값으로 바꾸는 것은 인코딩이 아니라 동작의 변경이다. 한 포스팅에 둘이 섞인다.

### 구조체 공통

- 구조체에 `TStructOpsTypeTraits`로 `WithNetSerializer`와 `WithNetSharedSerialization`을 켠다. `FRepMovement`와 같다(`ReplicatedState.h:305-312`). 두 번째를 켜야 프레임마다 한 번 직렬화한 결과를 모든 연결이 함께 쓴다(`RepLayout.cpp:5555-5557`, `2148-2151`). `FHitResult`는 첫 번째만 켠다(`HitResult.h:305`).
- 비교는 프로퍼티로 표시한 필드의 값으로 한다. 서버는 반올림하기 전의 위치(`double`)를 구조체에 넣으므로, NPC가 움직이는 프레임에는 늘 바뀐 것으로 본다. 지금의 `ReplicatedMovement`와 같다.
- 클라이언트는 `OnRep`에서 받은 값을 포스팅 11의 보간 버퍼에 넣는다. 지금은 `PostNetReceiveLocationAndRotation`에서 넣는다. 모션 기록도 `OnRep`에서 남긴다.
- 실행 인자로 켜고 끈다(예: `-NpcCompactMove`, 서버와 클라이언트에 `-LabNpcCompactMove`). 기본값은 끔이다.
- 구현 비용: `ALabNpc`와 새 구조체, 인자 처리로 약 120\~150줄이다.

### 위치의 단위(B와 C)

| 단위 | 축당 비트 | 갱신 한 번 | 반올림 오차(평면 거리의 평균) |
| --- | ---: | ---: | ---: |
| 1cm(엔진과 같음) | 13 | 50비트 | 0.38cm |
| 2cm | 12 | 48비트 | 0.77cm |
| 4cm | 11 | 46비트 | 1.53cm |

- 반올림 오차는 축마다 ±(단위 ÷ 2)에 고르게 퍼진다고 본 계산값이다(정사각형 안의 고른 점에서 중심까지 평균 거리 = 0.3826 × 한 변).
- 지금 표시 위치 오차 평균은 46.38cm이고, 세 실행의 변동 폭은 0.75cm다(`act2-interp2`). 4cm 단위의 오차 증가(약 1.2cm)는 변동 폭과 크기가 비슷해서, 구별되지 않을 수 있다.

## 3. 고르지 않을 후보

| 후보 | 고르지 않을 이유 |
| --- | --- |
| 엔진의 정밀도 설정 낮추기 | 기본값이 이미 가장 거친 단계다(1.1절). 설정만으로는 더 줄일 수 없다 |
| 직전에 보낸 값과의 차이 보내기 | `NetSerialize`의 결과는 모든 연결이 함께 쓰므로 연결마다의 기준값이 없다. 연결마다 확인받은 값을 기억하려면 `NetDeltaSerialize`(FastArray와 같은 방식)를 직접 짜야 한다. 패킷 손실 처리까지 직접 해야 하는 큰 작업이다 |
| 바뀐 때만 보내는 필드(예: 방향을 틀 때만 Yaw) | 패킷을 잃으면 엔진은 그 프로퍼티를 다시 보내는데, 그때의 현재 값을 직렬화한다(`DataReplication.cpp:888-925`의 `FObjectReplicator::ReceivedNak`가 바뀐 기록을 재전송 대상으로 표시, `RepLayout.cpp:2262-2279`가 그 목록을 이번 변경 목록에 합침). 그 사이 "바뀌지 않음"이 되면 클라이언트는 새 Yaw를 받지 못한다. 안전한 것은 `FHitResult`처럼 기본값이면 빼는 필드다(`HitResult.cpp`의 `NetSerialize` 플래그 8개). NPC의 속도가 그런 경우라 후보 모두 빼기로 했다 |
| Yaw를 별도 프로퍼티로 나눠 바뀔 때만 보내기 | 구조체를 나누는 것이고 `NetSerialize`가 아니다. Yaw는 직선 구간(평균 약 10초)마다 한 번 바뀐다. 보낼 때 핸들 8비트가 더 든다. 다른 기법이라 이 포스팅에 넣지 않는다 |
| Iris로 바꾸기 | 포스팅 15에서 다룬다. 이 시리즈는 그때까지 레거시 리플리케이션이다 |

## 4. 에이전트 의견

화면이나 실행에서 읽은 사실이 아니라 판단이다.

- **B를 1cm 단위로, 서버 프레임 번호를 구조체에 넣어 고르기를 추천한다.** 줄어드는 몫이 크고(예상 -20.4%) 정밀도가 엔진과 같아서, 품질 지표가 그대로인 것으로 "같은 화면을 더 적은 비트로"를 보일 수 있다. 줄어드는 비트의 대부분은 필요 없는 값을 뺀 몫이고, 범위를 아는 몫은 이 테스트베드에서 축당 약 1비트다. 본문에서는 맵 가장자리라면 범위를 아는 몫이 축당 약 5비트로 커진다는 것을 계산으로 보일 수 있다.
- **A를 고르면** `Home`을 보내지 않아 구조가 단순하다. 대신 이 테스트베드에서 X, Y가 엔진보다 비싸져서, 고정 길이가 늘 이득이 아니라는 결과가 섞인다.
- **C를 고르면** 가장 많이 준다. 대신 회전을 받지 않고 계산하는 동작의 변경이 섞이고, 실제 게임의 NPC에 그대로 쓰기 어렵다.
- **4cm 단위를 고르면** 정밀도를 낮춘 대가를 품질 지표로 보이는 글이 된다. 대신 더 주는 것은 4비트(약 1.7%)이고, 품질 변화가 변동 폭 안일 수 있다.
- **서버 프레임 번호를 따로 두면** 이동만 바꾼 결과가 된다. 대신 8비트(약 3.4%)를 덜 줄인다. 프레임 번호는 위치가 어느 순간의 것인지를 말하는 이동의 일부라서 함께 넣는 쪽이 자연스럽다.

## 5. 선택

사용자가 B를 고르고 아래 값을 정했다(2026-10-07).

- 위치: 집 기준 상대 좌표를 1cm 단위로 보낸다. 엔진과 같은 정밀도다.
- 회전: Yaw 8비트(엔진의 `ByteComponents`와 같은 압축)를 보낸다.
- 서버 프레임 번호: 구조체에 넣는다. 따로 보내던 `ServerFrame` 프로퍼티는 이 구성에서 보내지 않는다.
- 실행 인자는 `-NpcCompactMove`(서버와 클라이언트에 `-LabNpcCompactMove`)로 두고, 기본값은 끔이다(에이전트가 정함).
- 구현에서 정한 것(에이전트): 기준점은 `Home`을 cm 정수로 반올림한 값이다. `Home` 자체를 반올림하면 NPC의 난수 시드(`GetTypeHash(Home)`)가 바뀌어 경로가 기준 구성과 달라지기 때문이다. 반올림 때문에 상대 좌표가 ±3,000을 0.5cm 넘을 수 있어서, 13비트의 범위를 다 쓰는 -4,096\~4,095로 둔다. 비트 수는 같다.

## 6. 확인하지 않은 것

- 1.2절은 계산값이다. 받은 갱신마다 그 순간의 서버 위치를 썼고, 서버가 실제로 직렬화한 프레임의 위치와 한 프레임 어긋날 수 있다. N이 바뀌는 경계에서만 차이가 난다.
- 새 구조체의 실제 비트(Networking Insights의 `LabNpc` 갱신 한 번으로 확인한다).
- `WithNetSharedSerialization`을 켠 구조체가 실제로 함께 쓰이는지(`RepLayout.cpp`의 `GNumSharedSerializationHit`).
- `Home`을 처음 한 번 보내는 비용. 채널이 열리는 횟수로 정해진다.
- Iris가 구조체의 `NetSerialize`를 그대로 쓰는지([engine-notes.md](../../Docs/Reference/engine-notes.md) 8절).
