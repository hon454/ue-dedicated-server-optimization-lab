# 인벤토리를 FastArray로 보내기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 실행별 CSV, Insights 값, 서버 로그, 작은 규모 확인은 [관찰 자료](candidates.md) 8절, 10\~16절에 있다.

## 1. 실행 조건

- 구성: [포스팅 9](../09-inventory-owner-only/measurements.md)의 최종 구성. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, `-NodeUpdateFrequency 2`, `-InventoryOwnerOnly`. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초, 트레이스 켬.
- 두 묶음을 세 번씩 연달아 쟀고, 이어서 타이머를 더 켠 실행을 한 번씩 쟀다. 2026-10-06 17:31\~17:52, 본체 화면, 실행 중 PC 조작 없음. 8회 모두 종료 코드 0이었다.

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-fastarr-base1` | 적용 전 | 없음 | `r3` |
| `act2-fastarr1` | 적용 후 | `-InventoryFastArray` | `r2` |
| `act2-fastarr-base-split1` | 결과의 CPU 표, 적용 전 | `-StatNamedEvents` | 한 번 |
| `act2-fastarr-split1` | 결과의 CPU 표, 적용 후 | `-InventoryFastArray -StatNamedEvents` | 한 번 |

- 빌드는 커밋 `243be3d`의 소스다. 인자 `-InventoryFastArray`(서버 `-LabInventoryFastArray`)가 `ULabInventoryComponent` 대신 `ULabInventoryFastArrayComponent`를 붙인다. 두 클래스의 부모 `ULabInventoryBase`는 리플리케이트 프로퍼티가 없다. 적용 전 구성에서 리플리케이트하는 프로퍼티(`Items`), 조건, 지우는 방식은 포스팅 9와 같다.
- 본문의 Networking Insights 값은 `Game Instance 0 [Server]`, `Connection 0`(0번 자리, 채집 담당), `Outgoing`에서 측정 구간의 패킷을 골라 읽었다. 고른 범위는 적용 전 1,815패킷, 59.884초, 적용 후 1,796패킷, 59.964초다(관찰 자료 12절).
- 바이트/초는 Net Stats의 Incl(비트) ÷ 8 ÷ 고른 범위의 시간이다. 연결당 송신량은 `Actor`와 `PacketHeaderAndInfo`의 Incl을 더한 값으로 계산했다.
- Timing 값은 `Scripts/export-insights.ps1`로 내보냈다. 프레임 수는 `WorldTick` Count가 CSV `frames`와 같았다(1,787, 1,797).
- `-StatNamedEvents` 실행은 이벤트를 더 기록해 서버 시간이 달라진다. 이 두 실행의 값은 서로만 비교하고 다른 실행과 비교하지 않는다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 소유자는 바뀔 때마다 200칸을 다시 받았다 | 포스팅 9 적용 후 `ItemId` Count 3,000 = 15번 × 200칸(`act2-invown1-r2`). 이 글의 적용 전도 같다(`act2-fastarr-base1-r3`의 `ItemId` 3,000) | 관찰 자료 2절, 12절 |
| 한 번의 최대 크기 18,200 → 345비트, 약 53분의 1 | 인벤토리 컴포넌트 I.Max 18,190(`LabInventoryComponent`, `act2-fastarr-base1-r3`), 345(`LabInventoryFastArrayComponent`, `act2-fastarr1-r2`). 18,190 ÷ 345 = 52.7 | 관찰 자료 12절, 캡처 |
| 인벤토리 571 → 14.8바이트/초, -97.4% | 273,676 ÷ 8 ÷ 59.884 = 571.26, 7,086 ÷ 8 ÷ 59.964 = 14.77, 14.77 ÷ 571.26 − 1 = -97.41% | 관찰 자료 12절 |
| 연결당 송신 대역폭 11,800 → 11,200바이트/초, -4.95% | (5,499,105 + 166,639) ÷ 8 ÷ 59.884 = 11,826.6, (5,227,069 + 165,232) ÷ 8 ÷ 59.964 = 11,240.7, 11,240.7 ÷ 11,826.6 − 1 = -4.95%. CSV `out_bytes_per_sec_per_conn` 중앙값은 12,387 → 11,807(-4.68%) | 관찰 자료 11절, 12절 |
| 서버 프레임 시간 평균 9.13 → 8.87ms, 구별되지 않음 | Timing Insights 9.133 → 8.866(중앙값 실행끼리). CSV `work_avg_ms` 중앙값 8.852 → 8.586(-0.266)이 두 묶음의 변동 폭 0.181, 0.289 가운데 큰 쪽보다 작다 | 관찰 자료 11절, 13절 |
| 플레이어 캐릭터 8개, 인벤토리 200칸, 칸 하나는 아이템 번호와 수량 | `-Clients 8 -InventoryItems 200`, `FLabItem`과 `FLabFastItem`의 `ItemId`, `Count`(`Source/DSOptLab/LabInventoryComponent.h`, `LabInventoryFastArrayComponent.h`) | 실행 조건, 프로젝트 소스 |
| 하나를 4초마다 바꾼다, 맨 앞 칸을 지우고 맨 뒤에 더한다 | 타이머 간격 4 ÷ 8 = 0.5초로 여덟 인벤토리를 돌아가며 바꾼다(`LabGameMode.cpp`의 `ChurnOneInventory`). `RemoveAt(0)` 뒤 새 칸을 더한다(두 컴포넌트의 `Churn`) | 프로젝트 소스, 계산 |
| 문제의 표: 앞 칸 지우기 15번, 18,200비트 | `act2-fastarr-base1-r3`의 `ItemId` 3,000 ÷ 200 = 15번. 한 번은 I.Max 18,190 | 관찰 자료 12절 |
| 문제의 표: 채집 7번, 약 118비트 | 22 − 15 = 7번. (273,676 − 15 × 18,190) ÷ 7 = 826 ÷ 7 = 118.0. 앞 칸 지우기가 모두 18,190비트라고 보고 나눈 값이다 | 계산 |
| 같은 자리끼리 비교해 당겨진 칸이 모두 바뀐 칸이 된다 | `CompareProperties_Array_r`가 원소를 인덱스마다 비교한다(`Engine/Source/Runtime/Engine/Private/RepLayout.cpp:1692-1775`) | 엔진 소스, [engine-notes.md](../../Docs/Reference/engine-notes.md) 8절 |
| FastArray는 `FFastArraySerializer`, 칸마다 번호와 바뀐 횟수 | `FFastArraySerializerItem`의 `ReplicationID`, `ReplicationKey`(`Engine/Source/Runtime/Net/Core/Classes/Net/Serialization/FastArraySerializer.h:298-332`). `MarkItemDirty`가 번호를 처음 정하고 키를 올린다(`441-466`) | 엔진 소스 |
| 연결마다 지난번에 보낸 번호와 바뀐 횟수의 표를 기억한다 | 연결마다의 상태 `FNetFastTArrayBaseState::IDToCLMap`을 `OldState`로 받아 새 표 `NewIDToKeyMap`을 만든다(`FastArraySerializer.h:1420-1440`) | 엔진 소스 |
| 바뀐 칸, 새 번호, 사라진 번호만 보낸다. 당겨진 칸은 보내지 않는다 | `BuildChangedAndDeletedBuffers`(`FastArraySerializer.h:896-975`), "Stayed the same, it might have moved but we dont care" | 엔진 소스 |
| 배열이 통째로 그대로면 칸을 보지 않는다 | `ConditionalCreateNewDeltaState`가 배열 키와 기준 키가 같으면 거짓을 돌려 쓰지 않고 끝낸다(`FastArraySerializer.h:819-853`, `1426-1430`). 실행: `tsmall-fastarr-on2-r1`의 서버 로그에서 바뀌지 않았을 때 쓴 줄이 없다 | 엔진 소스, 관찰 자료 8.2절 |
| 머리는 128비트, 배열의 바뀐 횟수와 지운 칸 수 같은 값 | `WriteDeltaHeader`가 `ArrayReplicationKey`, `BaseReplicationKey`, 지운 수, 바뀐 수의 `int32` 넷을 쓴다(`FastArraySerializer.h:979-1008`) | 엔진 소스 |
| 클라이언트는 지운 자리에 맨 뒤 칸을 옮겨 채운다 | 새 칸은 맨 뒤에 더하고(`FastArraySerializer.h:1524`, `AddDefaulted_GetRef`), 지운 칸은 마지막에 `RemoveAtSwap`으로 지운다(`1186-1197`) | 엔진 소스, engine-notes.md 10절 |
| 일반 배열의 비교는 객체마다 프레임에 한 번, FastArray는 연결마다 확인한다 | 비교: `RepLayout.cpp:1275-1331`. FastArray 같은 사용자 정의 델타 프로퍼티는 `IsLifetime`을 받지 않아 비교에서 빠지고(`RepLayout.cpp:5848-5855`, `6318-6321`, `1424-1431`), 연결마다 `ReplicateCustomDeltaProperties`가 돈다(`Engine/Source/Runtime/Engine/Private/DataReplication.cpp:1646, 1719`) | 엔진 소스, 관찰 자료 3.2절 |
| 예상: 앞 칸 지우기 약 340비트, 256비트에 덧붙는 비트 | 머리 128 + 지운 번호 32 + 새 칸(번호 32 + `ItemId` 32 + `Count` 32) = 256. 덧붙는 비트는 일반 배열의 채집 118비트에서 `Count` 32비트를 뺀 86비트를 바탕으로 약 80비트로 짐작했다 | 관찰 자료 5.1절 |
| 예상: 인벤토리 약 15바이트/초, 연결당 송신 대역폭 약 4.5% 감소 | (15 × 340 + 7 × 300) ÷ 8 ÷ 59.886 = 15.0. CSV 12,421 − 15 × (18,190 − 340) ÷ 8 ÷ 60 = 11,863, -4.5%(포스팅 9의 `act2-invown1`에서 계산) | 관찰 자료 5.1절 |
| 예상: 인벤토리의 CPU는 조금 준다 | 비교 0.024ms가 빠지고 연결마다의 확인과 번호 표가 생긴다. 합은 프레임당 0.01\~0.02ms 감소로 짐작했다 | 관찰 자료 5.2절 |
| 적용의 코드 | `Source/DSOptLab/LabInventoryFastArrayComponent.h`, `LabInventoryFastArrayComponent.cpp`(커밋 `243be3d`). 본문은 줄여 옮겼다 | 프로젝트 소스 |
| 결과 표의 예상 약 11,300 | Insights 연결당 송신량 11,826.6 − 약 556(인벤토리 571 − 15) = 약 11,270 | 계산 |
| 차트의 값 | 18,190, 345(I.Max) | 관찰 자료 12절 |
| 줄어든 송신량의 95%가 인벤토리 | (571.26 − 14.77) ÷ (11,826.6 − 11,240.7) = 556.49 ÷ 585.9 = 95.0% | 계산 |
| 인벤토리가 이미 4.8% | 273,676 ÷ (5,499,105 + 166,639) = 4.83%(`act2-fastarr-base1-r3`) | 계산 |
| 약 4초마다 큰 패킷이 솟았고, 적용 후 사라졌다 | `Actor` 줄 I.Max 7,630 → 669비트. 두 캡처의 패킷 그래프(4절) | 관찰 자료 12절 |
| CPU 표 | `Replicate Actor Time` → `LabCharacter` 아래. 적용 전 `LabInventoryComponent` 0.104(아래 `Dynamic Property Compare Time` 0.024, `Dynamic Property Rep Time` 0.032), 적용 후 `LabInventoryFastArrayComponent` 0.117(아래 `Custom Delta Property Rep Time` 0.055). 프레임당 Incl ms(`act2-fastarr-base-split1-r1`, `act2-fastarr-split1-r1`) | 관찰 자료 13절 |
| 프레임에 약 50번 | `Custom Delta Property Rep Time` Count 49.7(프레임당) | 관찰 자료 13절 |
| 실행 하나씩이라 줄지 않았다는 것까지만 말한다 | 적용 후 실행은 서버 전체가 느렸다: 서버 프레임 시간 평균 9.738 → 10.180(+4.5%), `LabNpc` 0.942 → 0.989(+5.0%). 인벤토리 컴포넌트는 +12.5% | 관찰 자료 13절 |
| 한 칸만 흰색, 오른쪽으로 한 칸씩 | `visual19-panel.mp4`에서 자기 격자의 흰 칸이 한 칸(약 36픽셀)이고 12프레임(30fps, 0.4초) 이어지며, 121프레임(4.03초) 간격으로 x = 121 → 130 → 139px(한 칸 9px)로 옮겨 간다 | 관찰 자료 14절 |
| 가장 최근에 더한 칸의 아이템 번호가 적용 전과 같은 순서로 바뀌었다 | 화면 글자 "newest id"가 `visual18`과 `visual19` 모두 368 → 628 → 880(`Saved/Screenshots/Lab/visual18-panel-preview.png`, `visual19-panel-preview.png`). 같은 시드로 같은 아이템을 만든다 | 시각 자료 실행 |
| 인벤토리는 연결 하나가 받는 데이터의 0.1% | 7,086 ÷ (5,227,069 + 165,232) = 0.131%(`act2-fastarr1-r2`) | 계산 |
| 한 칸의 값만 바뀌면 FastArray가 오히려 크다 | 일반 배열의 채집 약 118비트(위). FastArray는 머리 128비트에 칸 하나(번호와 내용 96비트)가 더해진다 | 계산, 엔진 소스 |

## 3. 서버가 남긴 CSV

실행별 값은 [관찰 자료](candidates.md) 11절에 있다.

| 묶음 | `work_avg_ms` 중앙값 | 변동 폭 | `netflush_avg_ms` 중앙값 | `out_bytes_per_sec_per_conn` 중앙값 | 변동 폭 | `open_actor_channels_per_conn` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-fastarr-base1` | 8.852 | 0.181 | 4.370 | 12,387 | 26 | 77 |
| `act2-fastarr1` | 8.586 | 0.289 | 4.181 | 11,807 | 29 | 77 |

- `out_bytes_per_sec_per_conn`은 580(4.68%) 줄었다. 변동 폭(26, 29)보다 훨씬 크다.
- `work_avg_ms`는 0.266 줄었다. 변동 폭 가운데 큰 쪽(0.289)보다 작아 구별하지 못했다(measurement.md "차이의 판단").
- `act2-fastarr-base1`은 포스팅 9의 `act2-invown1`(8.493, 12,421)과 구성이 같다. 다른 시각의 묶음이라 비교하지 않았다.

## 4. 시각 자료

- 요약의 두 GIF는 인벤토리 패널을 2번 클라이언트(3인칭 이동)에 띄운 시각 자료 전용 실행 `visual18`(적용 전 구성)과 `visual19`(적용 후 구성)에서 찍었다. 측정 구간에 `Scripts/capture-video.ps1 -Region "1920,0,960,540" -RaiseSlots "2" -NoMouse -AllowMeasuring -Seconds 10 -Out <이름>.mp4`로 10초씩 MP4로 찍고, ffmpeg로 패널(960×540 화면의 610×150 영역)만 잘라 GIF로 바꿨다(폭 915px, 8fps, 64색). 변환 명령은 [STATUS.md](../../Docs/STATUS.md) "명령"의 인벤토리 패널 줄에 있다. 두 실행의 수치는 쓰지 않는다.
- 처음 찍은 `visual16`, `visual17`의 48색 GIF(폭 960px)에서는 적용 후의 한 칸 번쩍임이 팔레트에 들지 못해 사라졌다. 그래서 MP4로 다시 찍고 팔레트를 만들 때만 흰 사각형을 덧그려 흰색을 넣었다(관찰 자료 14절).
- 패널은 `LabHUD.cpp`의 `DrawInventoryPanel`이 그린다. 칸 색은 아이템 번호, 클라이언트가 지난 틱에 본 값과 같은 자리의 값이 다르면 0.4초 흰색, 받은 칸이 없으면 회색이다. 자리는 클라이언트 배열의 순서라서 FastArray에서는 서버의 순서와 다르다.
- 원리의 도식은 `Scripts/make-fastarray-cases.ps1`이 만든 [images/fastarray-cases.svg](images/fastarray-cases.svg)다. 머리, 번호, 칸의 비트와 클라이언트의 처리 순서는 엔진 소스에서 옮겼고, 칸 다섯 개와 그 번호는 예시다. 프로퍼티 머리 같은 덧붙는 비트는 그리지 않았다.
- 결과의 차트 값은 2절 "차트의 값" 줄이다.

### Networking Insights 캡처 (`Connection 0`, `Outgoing`)

| 적용 전(`act2-fastarr-base1-r3`) | 적용 후(`act2-fastarr1-r2`) |
| --- | --- |
| ![적용 전 Networking Insights. 약 4초마다 큰 패킷이 솟는다](images/act2-fastarr-base1-r3-netstats-connection0.png) | ![적용 후 Networking Insights. 큰 패킷이 없다](images/act2-fastarr1-r2-netstats-connection0.png) |

- 측정 구간을 고른 화면이다. 오른쪽 Net Stats에서 적용 전 `LabInventoryComponent`(22번, 273,676비트, 최대 18,190)와 적용 후 `LabInventoryFastArrayComponent`(22번, 7,086비트, 최대 345)를 읽었다.
- 적용 전 그래프에는 약 4초마다 솟는 막대가 있고, 적용 후 그래프에는 없다.
- 창은 2912 폭 화면에서 최대화했다. 포스팅 9의 캡처(3000×2080)와 크기가 달라 한 픽셀에 드는 패킷 수가 다르다. 이 글의 두 캡처끼리는 같다.

## 5. 확인하지 않은 것

- 0번이 아닌 연결의 값. 모든 연결이 자기 인벤토리만 받으므로 비슷하다고 보았다.
- 적용 후 채집 한 번의 비트. 최대 345와 평균 322만 보았고, 앞 칸 지우기와 채집을 패킷마다 고르지 않았다.
- 적용 후 `ItemId`가 15번, `Count`가 22번 나온 까닭. 채집은 `Count`만 보낸 것으로 보이지만, 칸 안의 바뀐 프로퍼티만 보내는 기능은 꺼져 있다(`FastArraySerializer.cpp:33`).
- CPU 차이의 변동 폭. `-StatNamedEvents` 실행은 구성마다 한 번만 쟀다.
- 클라이언트가 FastArray를 받는 데 드는 CPU.
