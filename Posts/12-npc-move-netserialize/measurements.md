# NPC 이동을 필요한 비트만 담은 구조체로 보내기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 후보와 고른 값, 지금 이동 데이터의 비트 구성은 [관찰 자료](candidates.md)에 있다. 비트 구성의 엔진 소스는 [engine-notes.md](../../Docs/Reference/engine-notes.md) 13절에 모았다. 품질 지표의 정의는 [ADR-0020](../../Docs/Decisions/0020-npc-motion-quality-metrics.md)과 [measurement.md](../../Docs/Guides/measurement.md) "수치의 이름과 출처"에 있다.

## 1. 실행 조건

- 구성: [포스팅 11](../11-npc-interpolation/measurements.md)의 최종 구성. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, `-NodeUpdateFrequency 2 -InventoryOwnerOnly -InventoryFastArray -NpcInterpDelay 150`에 위치 기록 `-MotionLog`를 더했다. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초, 트레이스 켬.
- 두 묶음을 세 번씩 연달아 쟀다. 2026-10-07 12:07\~12:23, 본체 화면, 실행 중 PC 조작 없음. 6회 모두 종료 코드 0이었다. 측정 직전에 PC 전체의 CPU 사용률이 3% 안팎이었고, 다른 세션이 남긴 `grep.exe` 두 개는 CPU를 쓰지 않았다([포스팅 11 측정 기록](../11-npc-interpolation/measurements.md) 7절의 첫 묶음 때문에 확인했다).

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-npcmove-base1` | 적용 전 | 없음 | `r2` |
| `act2-npcmove1` | 적용 후 | `-NpcCompactMove` | `r2` |

- 빌드는 커밋 `6ce774a`의 소스다. 인자 `-NpcCompactMove`(서버와 클라이언트 `-LabNpcCompactMove`)는 `ALabNpc`의 `ReplicatedMovement`를 끄고(`SetReplicatingMovement(false)`), `ServerFrame`을 보내지 않고(조건 `COND_Never`), `Move`(`FLabNpcMove`, `COND_None`)와 `MoveOrigin`(`FVector_NetQuantize`, `COND_InitialOnly`)을 보낸다. 인자가 없으면 두 프로퍼티는 `COND_Never`이고 포스팅 11과 같다.
- 6회 모두 서버 로그에 `lab_npc_move_clamped`(집 기준 좌표를 잘라 보낸 기록)가 없었다. 서버가 모션 기록에 쓴 시간은 프레임당 0.0687\~0.0721ms다(서버 로그 `lab_motion_log server record_ms_per_frame`).
- 품질 지표는 `Scripts/analyze-motion.ps1 -Label <라벨>-rN`이 계산했다. Timing 값은 `Scripts/export-insights.ps1`로 내보냈다. 프레임 수는 `WorldTick` Count가 CSV `frames`와 같았다(1,788, 1,790).

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| NPC 갱신 한 번 117 → 69.0비트, -41.1% | Networking Insights의 `LabNpc` Incl ÷ Count: 2,952,335 ÷ 25,182 = 117.24(`act2-npcmove-base1-r2`), 1,739,067 ÷ 25,189 = 69.04(`act2-npcmove1-r2`). 69.04 ÷ 117.24 − 1 = -41.11% | 5.1절 |
| 연결당 송신 대역폭 12,700 → 10,100바이트/초, -20.4% | CSV `out_bytes_per_sec_per_conn` 중앙값 12,672 → 10,088. 10,088 ÷ 12,672 − 1 = -20.39% | 3절 |
| 표시 위치 오차 평균 46.6 → 45.9cm, 구별되지 않음 | 중앙값 46.56 → 45.87. 차이 0.69가 두 묶음의 변동 폭 1.01, 0.63 가운데 큰 쪽보다 작다 | 4절 |
| 서버 프레임 시간 평균 8.96 → 8.76ms, -2.20% | Timing Insights 8.957 → 8.760(중앙값 실행 `r2`끼리, ADR-0010). 8.760 ÷ 8.957 − 1 = -2.20% | 5절 |
| 요약의 도식 | `Scripts/make-move-bits.ps1`. 위치를 축마다 N = 14비트로 쓰는 예시. `act2-interp2-r1`에서 받은 갱신의 59.4%가 N = 14였다 | 6절, [관찰 자료](candidates.md) 1.2절 |
| 연결 하나는 1초에 NPC 갱신을 약 428번 받는다 | `act2-npcmove-base1-r2`의 모션 기록에서 클라이언트 8개가 측정 구간 60.022초에 받은 NPC 갱신 수 ÷ 60.022의 평균 427.8번(클라이언트별 419.4\~431.7) | 모션 기록 |
| 0번 연결이 보낸 액터 데이터의 52.3%가 NPC | `LabNpc` Incl ÷ `Actor` Incl = 2,952,335 ÷ 5,645,669 = 52.29%(`act2-npcmove-base1-r2`) | 5.1절 |
| 문제의 표: 이동 데이터 82.3비트 | `act2-npcmove-base1-r2`의 0번 클라이언트가 측정 구간에 받은 NPC 갱신 25,165번마다 그 순간의 서버 위치로 N을 셈하고 41 + 3N(핸들 포함)을 평균한 값 82.28(N 평균 13.759). Insights로 대조하면 `ReplicatedMovement` 3,264,808비트에서 플레이어 캐릭터 몫(적용 후 트레이스의 평균 1,196,449 ÷ 9,281 = 128.91비트 × 9,268번)을 뺀 2,070,033 ÷ 25,182 = 82.20비트다 | 관찰 자료 1.1절의 식, 5.1절 |
| 문제의 표: 서버 프레임 번호 16비트 | `ServerFrame` 402,912 ÷ 25,182 = 16.00(핸들 8 + 값 8) | 5.1절 |
| 문제의 표: 그 밖의 헤더 19.0비트 | 117.24 − 82.28 − 16 = 18.96 | 계산 |
| `ReplicatedMovement`는 위치, 회전, 속도를 담는 엔진의 구조체, X, Y, Z와 속도를 늘 보낸다 | `FRepMovement::NetSerialize`(`Engine/Source/Runtime/Engine/Private/Engine/ReplicatedState.cpp:67-152`). 비트 구성은 [engine-notes.md](../../Docs/Reference/engine-notes.md) 13절 | 엔진 소스 |
| NPC의 높이가 늘 같다 | 스폰 높이 Z = 50(`Source/DSOptLab/LabGameMode.cpp:116, 296`), `ALabNpc::TickMovement`가 Z를 바꾸지 않는다. 모션 기록의 서버 위치 Z도 모두 50이었다 | 프로젝트 소스, 모션 기록 |
| NPC의 속도가 늘 0이다 | 서버는 `GetVelocity()`를 보내고, 이 값은 루트 컴포넌트의 `ComponentVelocity`다. 이동 컴포넌트가 없는 `ALabNpc`는 채우지 않는다([포스팅 11 관찰 자료](../11-npc-interpolation/candidates.md) 1절) | 엔진 소스 |
| 위치를 축마다 14비트로 쓰면 39비트가 필요 없다 | 플래그 4 + 위치 헤더 7 + Z 14 + 회전의 "0이 아님" 비트 3 + 속도 10 + 가속도 있음 1 = 39. 관찰 자료 1.3절은 Yaw의 "0이 아님" 비트를 필요한 쪽으로 세어 38비트로 적었다. 새 구조체는 Yaw를 늘 1바이트로 보내므로 이 비트도 없어진다 | 계산 |
| 정밀도의 기본값이 이미 가장 거친 단계다 | `LocationQuantizationLevel`, `VelocityQuantizationLevel`이 `RoundWholeNumber`(1cm), `RotationQuantizationLevel`이 `ByteComponents`(`ReplicatedState.cpp:34-36`). 열거형에 더 거친 값이 없다(`Engine/Source/Runtime/Engine/Classes/Engine/ReplicatedState.h:11-28`) | 엔진 소스 |
| 구조체에 `NetSerialize`를 두면 엔진이 직렬화를 맡긴다 | `TStructOpsTypeTraits`의 `WithNetSerializer`. `FRepMovement`(`ReplicatedState.h:305-312`)와 `FHitResult`(`Classes/Engine/HitResult.h:305`)가 이렇게 직렬화된다 | 엔진 소스 |
| 엔진은 위치 값의 크기에 맞춰 길이를 정하고 7비트 헤더에 적는다 | `UE::Net::WriteQuantizedVector`(`Engine/Source/Runtime/Net/Core/Private/Net/Core/Serialization/QuantizedVectorSerialization.cpp:90-96`). `SerializeInt(값, 128)`은 7비트다(`Core/Private/Serialization/BitWriter.cpp:142-146`) | 엔진 소스 |
| NPC는 집에서 가로세로 30m 안에서만 걷는다, 축마다 13비트 | `ALabNpc::WanderRadius` 3,000cm, `PickTarget`이 집 ± 3,000cm 정사각형 안의 점을 고른다. 집 기준 1cm로 6,001가지라 13비트(8,192)다. 기준점이 집을 반올림한 값이라 0.5cm까지 더 벗어날 수 있어 -4,096\~4,095를 쓴다(`LabNpc.cpp`의 `SerializeMoveOffset`) | 프로젝트 소스 |
| 프로퍼티 핸들, 프로퍼티 둘을 하나로 합치면 핸들 하나가 준다 | 바뀐 프로퍼티마다 `SerializeIntPacked`로 핸들을 쓴다(`Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1922-1932`). 핸들 번호가 128보다 작아 8비트다 | 엔진 소스 |
| 흐름도 | 서버가 `PreReplication`에서 `Move`를 채우고(`LabNpc.cpp`), 비교는 객체마다 프레임에 한 번이고(engine-notes.md 8절), 직렬화는 공유 직렬화로 한 번이다(아래) | 프로젝트 소스, 엔진 소스 |
| 공유 직렬화 | `WithNetSharedSerialization`인 구조체는 프레임에 한 번 직렬화한 결과를 연결들이 함께 쓴다(`RepLayout.cpp:5555-5557`, `2148-2151`, `2716-2752`의 `WriteSharedProperty`). 이 비트에는 핸들이 들어 있다. `Move`가 실제로 공유 직렬화로 갔다는 것은 5.1절의 `Shared` 줄로 확인했다 | 엔진 소스, 5.1절 |
| 액터 채널, 집의 위치는 채널이 열릴 때 한 번 | `MoveOrigin`의 조건 `COND_InitialOnly`(`LabNpc.cpp`의 `GetLifetimeReplicatedProps`) | 프로젝트 소스 |
| 패킷을 잃으면 그때의 현재 값을 다시 직렬화한다 | `FObjectReplicator::ReceivedNak`가 그 패킷의 바뀐 기록을 재전송 대상으로 표시하고(`Engine/Source/Runtime/Engine/Private/DataReplication.cpp:888-925`), `FRepLayout::UpdateChangelistHistory`가 그 목록을 이번 변경 목록에 합쳐 지금 값으로 보낸다(`RepLayout.cpp:2262-2279`) | 엔진 소스 |
| 예상: 약 98.4비트에서 50비트 | 지금 41 + 3 × 13.81 + 16 = 98.43(`act2-interp2-r1`의 N 평균, 관찰 자료 1.2절). 새 구조체 8 + 13 + 13 + 8 + 8 = 50 | 계산 |
| 예상: 약 2,590바이트/초, 20.4% | 48.43 × 427.7 ÷ 8 = 2,589(`act2-interp2-r1`의 클라이언트 하나가 1초에 받은 NPC 갱신 평균 427.7번). 2,589 ÷ 12,682 = 20.42%(포스팅 11의 `act2-interp2` 중앙값) | 계산 |
| 위치는 엔진과 같은 1cm, 방향도 같은 1바이트 | 엔진은 절대 위치를 1cm로 반올림하고, 새 구조체는 정수 cm인 기준점에서 잰 좌표를 1cm로 반올림한다. Yaw는 같은 `FRotator::CompressAxisToByte`다 | 프로젝트 소스, 엔진 소스 |
| 적용의 코드 | `Source/DSOptLab/LabNpc.h`, `LabNpc.cpp`(커밋 `6ce774a`). 본문은 줄여 옮겼다 | 프로젝트 소스 |
| 결과 표의 예상 약 68.8비트 | 포스팅 11의 117.20(`act2-interp2-r1`) − 48.43 = 68.77. 0번 연결의 N 평균으로 셈하면 117.24 − (82.28 + 16 − 50) = 68.96 | 계산 |
| 결과 표의 예상 약 10,100 | 12,672 − 2,589 = 10,083 | 계산 |
| 결과 표의 실제 | 69.04, 50, 10,088(중앙값), 45.87(중앙값) | 3절, 4절, 5.1절 |
| 차트의 값 | 12,672, 10,088 | 3절 |
| 새 구조체는 갱신 25,189번에서 모두 50비트 | `Move` Count 25,189, Incl 1,259,450 = 25,189 × 50, I.Max 50 | 5.1절 |
| 서버 프레임 번호는 따로 가지 않았다 | 적용 후 트레이스의 Net Stats에서 0이 아닌 줄에 `ServerFrame`이 없다 | 5.1절 |
| 0번 연결에 집의 위치가 60초 동안 네 번, 246비트 | `MoveOrigin` Count 4, Incl 246, I.Max 63 | 5.1절 |
| 서버가 남긴 기록으로 이 차이는 세 실행 사이의 차이를 겨우 넘었다 | CSV `work_avg_ms` 중앙값 8.681 → 8.484(-0.197)가 두 묶음의 변동 폭 0.085, 0.190 가운데 큰 쪽보다 크다(measurement.md "차이의 판단") | 3절 |
| NPC를 리플리케이트하는 시간 0.884 → 0.842ms | `GameNetDriver` 아래 `LabNpc`의 프레임당 Incl(`r2`끼리) | 5절 |
| 엔진이 이동 데이터를 모으지 않게 된 몫으로 추정 | `AActor::GatherCurrentMovement`는 `IsReplicatingMovement()`일 때만 이동 데이터를 모은다(`Engine/Source/Runtime/Engine/Private/ActorReplication.cpp:426-428`). 몫을 나눠 재지 않았다(7절) | 엔진 소스 |
| 표시 지연 155, 154ms | 중앙값 | 4절 |
| 그 지연을 뺀 위치 오차 1.37, 1.22cm | 표시 지연에서의 평균 위치 오차(`summary.txt`의 `display_lag_ms` 줄) 중앙값 | 4절 |
| 줄인 48.4비트 가운데 38.8비트가 필요 없는 값, 범위를 아는 몫 1.62비트 | 평균 N = 13.81일 때 필요 없는 값 4 + 7 + 13.81 + 3 + 10 + 1 = 38.81, 핸들 8, 범위 2 × (13.81 − 13) = 1.62. 합계 48.43 | 계산 |
| 맵 가장자리라면 축마다 18비트 | NPC 배치 범위의 끝 95,000cm(`LabScenarioConfig.h`의 `WorldHalfExtent`)는 부호 비트를 더해 18비트다(`QuantizedVectorSerialization.cpp:13-17`) | 엔진 소스, 계산 |
| 잘라 보낸 횟수는 한 번도 없었다 | 6회의 서버 로그에 `lab_npc_move_clamped`가 없다 | 서버 로그 |

## 3. 서버가 남긴 CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-npcmove-base1-r1` | 1797 | 8.606 | 12.897 | 0 | 4.198 | 12674 | 77 | 0.000 |
| `act2-npcmove-base1-r2` | 1788 | 8.681 | 13.237 | 1 | 4.246 | 12672 | 77 | 0.000 |
| `act2-npcmove-base1-r3` | 1797 | 8.691 | 13.387 | 1 | 4.253 | 12662 | 77 | 0.000 |
| **중앙값** | 1797 | 8.681 | 13.237 | 1 | 4.246 | 12672 | 77 | 0.000 |
| **변동 폭** | 9 | 0.085 | 0.490 | 1 | 0.055 | 12 | 0 | 0.000 |
| `act2-npcmove1-r1` | 1796 | 8.673 | 12.446 | 0 | 4.195 | 10083 | 77 | 0.000 |
| `act2-npcmove1-r2` | 1790 | 8.484 | 12.246 | 1 | 4.075 | 10088 | 77 | 0.000 |
| `act2-npcmove1-r3` | 1788 | 8.483 | 12.279 | 0 | 4.059 | 10104 | 77 | 0.000 |
| **중앙값** | 1790 | 8.484 | 12.279 | 0 | 4.075 | 10088 | 77 | 0.000 |
| **변동 폭** | 8 | 0.190 | 0.200 | 1 | 0.136 | 21 | 0 | 0.000 |

- `out_bytes_per_sec_per_conn`은 2,584(20.39%) 줄었다. 변동 폭(12, 21)보다 훨씬 크고, 예상 2,589와 0.2% 다르다.
- `work_avg_ms`(-0.197), `work_p99_ms`(-0.958), `netflush_avg_ms`(-0.171)는 두 묶음의 변동 폭 가운데 큰 쪽보다 크다. `work_avg_ms`는 0.197 대 0.190으로 겨우 넘었다.
- `act2-npcmove-base1`은 포스팅 11의 `act2-interp2`(8.616, 12,682)와 같은 구성이다. 다른 시간의 묶음이라 비교하지 않았다.

## 4. 품질 지표

`analyze-motion.ps1`의 값이다(`Saved/LabMotion/<라벨>/summary.txt`, `Saved/LabMetrics/motion.csv`). 클라이언트 8개, NPC 66개, 이 NPC들의 서버 기록과 짝이 맞지 않은 클라이언트 기록은 없었다. 표시 지연 옆의 괄호는 그 지연에서의 평균 위치 오차(cm)다.

| 라벨 | 표본 | 표시 위치 오차 평균 / P99(cm) | 표시 속도 오차 평균 / P99(cm/s) | 표시 위치가 바뀐 간격 평균 / P99(ms) | 수신 간격 평균 / P99(ms) | 표시 지연(ms) | 클라이언트 프레임 간격 평균(ms) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-npcmove-base1-r1` | 771,187 | 46.64 / 52.93 | 15.2 / 337.8 | 35.5 / 45.7 | 133.4 / 166.8 | 155(1.46) | 35.47 |
| `act2-npcmove-base1-r2` | 772,056 | 45.63 / 49.61 | 14.9 / 336.3 | 35.4 / 45.6 | 133.6 / 166.8 | 153(1.22) | 35.42 |
| `act2-npcmove-base1-r3` | 774,888 | 46.56 / 53.51 | 14.9 / 342.6 | 35.3 / 45.5 | 133.4 / 166.9 | 155(1.37) | 35.29 |
| **중앙값** | | 46.56 / 52.93 | 14.9 / 337.8 | 35.4 / 45.6 | 133.4 / 166.8 | 155(1.37) | 35.42 |
| **변동 폭** | | 1.01 / 3.90 | 0.3 / 6.3 | 0.2 / 0.2 | 0.2 / 0.1 | 2(0.24) | 0.18 |
| `act2-npcmove1-r1` | 777,338 | 46.33 / 53.61 | 14.8 / 328.1 | 35.2 / 44.7 | 133.4 / 166.8 | 154(1.46) | 35.17 |
| `act2-npcmove1-r2` | 774,382 | 45.87 / 49.53 | 15.2 / 332.5 | 35.3 / 45.4 | 133.5 / 166.7 | 154(1.20) | 35.30 |
| `act2-npcmove1-r3` | 775,579 | 45.70 / 49.70 | 15.0 / 325.8 | 35.2 / 45.2 | 133.6 / 166.9 | 153(1.22) | 35.25 |
| **중앙값** | | 45.87 / 49.70 | 15.0 / 328.1 | 35.2 / 45.2 | 133.5 / 166.8 | 154(1.22) | 35.25 |
| **변동 폭** | | 0.63 / 4.08 | 0.4 / 6.7 | 0.1 / 0.7 | 0.2 / 0.2 | 1(0.26) | 0.13 |

- 모든 지표에서 중앙값의 차이가 두 묶음의 변동 폭 가운데 큰 쪽보다 작거나 같다. 표시 속도 오차 P99의 차이 9.7은 변동 폭 6.7보다 크지만, 방향을 바꾸는 순간의 값이라 판단하지 않았다(7절).
- 수신 간격이 같다. 갱신이 오는 간격은 바뀌지 않았다.

## 5. Timing Insights (중앙값 실행 `r2`)

| 항목(프레임당) | `act2-npcmove-base1-r2` | `act2-npcmove1-r2` |
| --- | ---: | ---: |
| 서버 프레임 시간 평균 / P99(ADR-0010) | 8.957 / 13.703ms | 8.760 / 12.665ms |
| `GameNetDriver` Incl / Excl | 3.993 / 2.449ms | 3.824 / 2.337ms |
| `LabNpc` Count / Incl / Excl | 114.9 / 0.884 / 0.701ms | 114.7 / 0.842 / 0.666ms |
| `LabCharacter` Incl | 0.480ms | 0.470ms |

- `Saved/InsightsExport/<라벨>/summary.txt`에서 옮겼다. 구간은 두 북마크 사이(60.019초, 60.027초)다.
- `LabNpc`의 차이(-0.042ms)와 `GameNetDriver` Excl의 차이(-0.112ms)는 실행 하나씩의 값이라 구별 여부를 판단하지 않았다.

### 5.1 Networking Insights (`Game Instance 0 [Server]`, `Connection 0`, `Outgoing`)

| 줄 | `act2-npcmove-base1-r2` Count / Incl / I.Avg | `act2-npcmove1-r2` Count / Incl / I.Avg |
| --- | ---: | ---: |
| `Actor` | 35,980 / 5,645,669 / 156 | 35,994 / 4,434,164 / 123 |
| `LabNpc` | 25,182 / 2,952,335 / 117 | 25,189 / 1,739,067 / 69 |
| `ReplicatedMovement` | 34,450 / 3,264,808 / 94 | 9,281 / 1,196,449 / 128 |
| `ServerFrame` | 25,182 / 402,912 / 16 | 없음 |
| `Move` | 없음 | 25,189 / 1,259,450 / 50 |
| `MoveOrigin` | 없음 | 4 / 246 / 61 |
| `BP_LabCharacter_C` | 9,811 / 1,480,075 / 150 | 9,820 / 1,481,762 / 150 |
| `PropertyHandle` | 36,218 / 289,744 / 8 | 36,232 / 289,856 / 8 |
| `PacketHeaderAndInfo` | 1,789 / 164,588 / 92 | 1,791 / 164,772 / 92 |
| `Shared` | 61,272 / 3,736,906 / 60 | 36,114 / 2,525,281 / 69 |

- 패킷 범위는 그래프의 정상 구간에서 패킷 약 1,790번부터 그래프 끝까지를 클릭과 Shift-클릭으로 골랐다. 선택 범위는 1,789패킷 60.023초, 1,791패킷 60.028초로 측정 구간과 거의 같다. 두 트레이스 모두 창을 최대화하고 같은 화면 위치를 클릭했다(캡처 3862×2110).
- 적용 후의 `ReplicatedMovement`는 플레이어 캐릭터의 이동만 남은 것이다. Count 9,281이 `BP_LabCharacter_C`의 9,820과 비슷하다.
- 엔진은 공유 직렬화한 비트를 패킷에 복사할 때만 `Shared` 범위를 남긴다(`RepLayout.cpp:2856-2861`, 같은 자리에서 `GNumSharedSerializationHit`를 센다). `Shared`의 Incl에서 `ReplicatedMovement`와 `ServerFrame`(적용 후에는 `Move`)을 빼면 적용 전 69,186비트, 적용 후 69,382비트가 남는다. 남는 몫이 같으므로 `Move` 1,259,450비트는 모두 `Shared` 안에 있다. `Move`도 `ReplicatedMovement`처럼 프레임에 한 번 직렬화되어 연결들이 함께 썼다.
- 0번 연결의 송신량으로 셈하면 (`Actor` + `PacketHeaderAndInfo`) ÷ 8 ÷ 시간이 12,100 → 9,577바이트/초(-20.9%)다. CSV의 연결 평균보다 작은 것은 0번 클라이언트가 채집 담당이라 제자리에 있기 때문으로 보며, 확인하지 않았다.

| 적용 전 | 적용 후 |
| --- | --- |
| ![적용 전 Networking Insights Net Stats. ① ReplicatedMovement ② LabNpc ③ ServerFrame](images/act2-npcmove-base1-r2-netstats-connection0.png) | ![적용 후 Networking Insights Net Stats. ① LabNpc ② Move ③ MoveOrigin](images/act2-npcmove1-r2-netstats-connection0.png) |

적용 전: ① `ReplicatedMovement`(NPC와 플레이어 캐릭터), ② `LabNpc` 117비트, ③ `ServerFrame` 16비트. 적용 후: ① `LabNpc` 69비트, ② `Move` 50비트, ③ `MoveOrigin` 네 번. 원본은 `Saved/Screenshots/Lab/insights-<라벨>-netstats.png`이고 상자는 `Scripts/annotate-image.ps1`로 그렸다.

## 6. 시각 자료

- 요약의 도식 [images/move-bits.svg](images/move-bits.svg)는 `Scripts/make-move-bits.ps1`이 그린 예시다. 위치를 축마다 N = 14비트로 쓸 때 지금 99비트(`ReplicatedMovement` 83 + `ServerFrame` 16)와 `FLabNpcMove` 50비트를 값별로 그렸다. Edge 헤드리스로 그려 글자가 겹치지 않는지 확인했다.
- 원리의 흐름도와 결과의 차트는 본문의 Mermaid다.
- 클라이언트 화면의 영상은 찍지 않았다. 품질 지표가 구별되지 않아 두 영상이 같게 보일 것이기 때문이다(사용자 결정, 2026-10-07).

## 7. 확인하지 않은 것

- 서버 프레임 시간이 준 몫 가운데 직렬화할 비트가 준 몫과 `GatherCurrentMovement`를 건너뛴 몫. 타이머를 나눠 재지 않았다.
- 표시 속도 오차 P99의 차이(337.8 → 328.1cm/s)가 NPC가 방향을 바꾸는 순간에서 오는지.
- 0번 연결의 송신량이 CSV의 연결 평균보다 작은 이유.
- Iris가 구조체의 `NetSerialize`를 그대로 쓰는지([engine-notes.md](../../Docs/Reference/engine-notes.md) 8절의 "확인하지 않은 것").
