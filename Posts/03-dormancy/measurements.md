# 자원 노드 Dormancy: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값은 [후보 기법 자료](candidates.md)에 있다.

## 1. 실행 조건

- 적용 전은 `relevancy2`, 적용 후는 `dormancy2`다. 각각 같은 명령으로 세 번 실행했다(`r1`\~`r3`).
- 조건: 클라이언트 8, 자원 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 여섯 실행 모두 종료 코드 0으로 끝났고 `saturated_ratio`는 0.000이다.
- Insights 값은 두 북마크(`Lab_MeasureStart`, `Lab_MeasureEnd`) 사이의 측정 구간에서 읽었다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 17.6ms → 14.7ms(-17%) | 17.64ms → 14.69ms(-16.7%) | Timing Insights, 세 실행의 중앙값(`relevancy2-r3`, `dormancy2-r2`). 3절 |
| 리플리케이션 시간 13.1ms → 10.2ms(-23%) | 13.14ms → 10.17ms(-22.6%) | Timing Insights `GameNetDriver`, 세 실행의 중앙값(`relevancy2-r1`, `dormancy2-r2`). 3절 |
| 연결당 열린 액터 채널 수 118 → 20(-83%) | -83.1% | CSV, 세 실행 모두 같음 |
| 클라이언트에 존재하는 자원 노드 114 → 301개(45초), 111 → 313개(75초) | 같음 | 1번 클라이언트(이동) 자동 스크린샷의 화면 글자(`relevancy2-r3`, `dormancy2-r2`) |
| 150m 안의 자원 노드 약 100개 | 연결 하나가 프레임마다 75.4\~95.6개 처리 | `relevancy2`의 `LabResourceNode` Count ÷ `WorldTick` Count ÷ 8. 균등 배치의 계산값은 약 98개 |
| 60초 동안 클라이언트 하나에 약 13만 5천 번 처리 | 1,083,238 ÷ 8 = 135,405 | `relevancy2-r3`의 `LabResourceNode` Count |
| 실제로 보낸 횟수 27번 | 27번, 1,314비트 | `relevancy2-r3` `Connection 0`(제자리에서 채집)의 Network Insights. 이동하는 `Connection 1`은 145번이다(새로 150m 안에 들어온 자원 노드) |
| 비교에 프레임당 1.74\~1.98ms | `r3` 1.74, `r2` 1.86, `r1` 1.98 | `relevancy2`의 `LabResourceNode` Incl ÷ `WorldTick` Count |
| 약 4,900개와 약 100개(흐름도) | 5,001 − 약 98 | 균등 배치의 계산값 |
| 자원 노드의 처리 1.98ms → 0.007ms(-99.6%) | 같음 | `relevancy2-r1`, `dormancy2-r2`. 5절 |
| 처리 횟수 약 100만 번 → 609번 | 997,693 → 609 | `LabResourceNode` Count(`relevancy2-r1`, `dormancy2-r2`). `dormancy2` 세 실행에서 546\~609 |
| 네트워크 드라이버 자체 시간 10.4ms → 9.43ms, 구별되지 않음 | 10.43ms → 9.43ms | `GameNetDriver` Exclusive. 세 실행의 범위가 9.32\~10.60ms(적용 후)와 10.43\~10.80ms(적용 전)로 겹친다 |
| 리플리케이션 시간의 93%가 네트워크 드라이버 자체 시간 | 9.43 ÷ 10.17 = 92.7% | 5절 |
| 송신 비트의 72%가 NPC 이동 | 836,895 ÷ 1,163,648 = 71.9% | `dormancy2-r2` `Connection 0`의 `LabNpc` ÷ `Actor` |
| 관찰자 화면의 자원 노드 193 → 312개 | 같음 | 수동 확인(`Scripts/run-manual.ps1`). 7절 |

## 3. 세 실행의 Insights 값

| 지표 | `r1` | `r2` | `r3` | 중앙값 | 변동 폭 | Relevancy 중앙값(변동 폭) |
| --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 16.58 | 14.69 | 14.45 | 14.69 | 2.13 | 17.64(0.16) |
| 서버 프레임 시간 P99(ms) | 25.02 | 22.53 | 21.34 | 22.53 | 3.68 | 26.64(4.06) |
| 리플리케이션 시간(ms/프레임) | 11.50 | 10.17 | 10.04 | 10.17 | 1.46 | 13.14(0.18) |
| 연결당 송신 대역폭(바이트/초, `Connection 0`) | 읽지 않음 | 2,772 | 읽지 않음 | | | 2,897(`r3`) |
| 연결당 열린 액터 채널 수(CSV) | 20 | 20 | 20 | 20 | 0 | 118(0) |

- **서버 프레임 시간 평균과 리플리케이션 시간은 구별되는 차이다.** 중앙값의 변화(2.95ms, 2.97ms)가 두 구성의 변동 폭 중 큰 쪽(2.13ms, 1.46ms)보다 크다.
- **P99는 구별된다고 보지 않는다.** 변화 4.11ms가 Relevancy의 변동 폭 4.06ms와 거의 같고, CSV `work_p99_ms`로는 변화 3.921이 변동 폭 4.026보다 작다.
- **연결당 송신 대역폭도 구별되지 않는다.** 차이 125바이트/초(-4.3%)는 CSV에서 보이는 실행 사이의 흔들림(변동 폭 586\~604바이트/초)보다 작다.
- **`r1`만 느리다.** 세 지표 모두 `r1`이 `r2`, `r3`보다 1.11\~1.17배 크다. 프레임 수도 `r1`만 적다(1,289, 나머지는 1,789와 1,790). 이유는 확인하지 않았다.
- 서버 프레임 시간 평균은 (측정 구간 − 틱 속도 제한 대기) ÷ `Frame` Count다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md). `r2`: (60.015초 − 33.732초) ÷ 1,789). P99는 프레임마다 대기를 뺀 길이를 정렬한 ceil(N × 0.99)번째 값이다.
- 리플리케이션 시간은 `GameNetDriver` Incl ÷ `WorldTick` Count다(`r2`: 18.18초 ÷ 1,788).
- 연결당 송신 대역폭은 (`Actor` Incl 1,163,648 + `PacketHeaderAndInfo` Incl 164,220)비트 ÷ 8 ÷ 59.883초다(`r2` `Connection 0` `Outgoing`, 1,785패킷).

## 4. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `dormancy2-r1` | 1,288 | 16.205 | 24.410 | 11.774 | 3,734 | 20 | 0.000 |
| `dormancy2-r2` | 1,788 | 14.409 | 22.063 | 10.430 | 4,316 | 20 | 0.000 |
| `dormancy2-r3` | 1,789 | 14.173 | 21.116 | 10.298 | 4,320 | 20 | 0.000 |
| 중앙값 | 1,788 | 14.409 | 22.063 | 10.430 | 4,316 | 20 | 0.000 |
| 변동 폭 | 501 | 2.032 | 3.294 | 1.476 | 586 | 0 | 0.000 |
| Relevancy 중앙값 | 1,488 | 17.380 | 25.984 | 13.357 | 4,076 | 118 | 0.000 |

Insights 값과의 차이는 서버 프레임 시간 평균과 `work_avg_ms`가 +1.9\~+2.3%, P99와 `work_p99_ms`가 +1.1\~+2.5%, 리플리케이션 시간과 `netflush_avg_ms`가 -2.3\~-2.5%다.

## 5. 리플리케이션 시간의 내역

| 타이머(프레임당) | Relevancy `r1` | Dormancy `r2` | 변화 |
| --- | --- | --- | --- |
| `LabResourceNode` | 1.98ms (15.1%) | 0.007ms (0.07%) | -99.6% |
| `LabNpc` | 0.42ms (3.2%) | 0.41ms (4.0%) | -2.4% |
| `GameNetDriver` Exclusive | 10.43ms (79.4%) | 9.43ms (92.7%) | -9.6% |
| 나머지 | 0.31ms | 0.32ms | |
| 합계(리플리케이션 시간) | 13.14ms | 10.17ms | -22.6% |

- 각 값은 타이머 Incl(Exclusive는 Excl) ÷ `WorldTick` Count이고, 괄호는 리플리케이션 시간 대비 비율이다. 두 구성 모두 리플리케이션 시간의 중앙값 실행이다.
- 연결 하나가 프레임마다 처리하는 자원 노드는 95.6개에서 0.043개가 됐다(609 ÷ 1,788 ÷ 8).
- `Connection 0`의 `LabResourceNode` 송신은 27번 1,314비트에서 9번 810비트(`Health` 9, `bDepleted` 9)가 됐다.

## 6. Insights 화면

적용 전(`relevancy2-r3`)과 적용 후(`dormancy2-r2`)의 측정 구간이다.

| 적용 전 | 적용 후 |
| --- | --- |
| ![적용 전 Timers와 WorldTick Callees](images/before-timing.png) | ![적용 후 Timers와 WorldTick Callees](images/after-timing.png) |
| ![적용 전 Connection 0 Outgoing의 Net Stats](images/before-network.png) | ![적용 후 Connection 0 Outgoing의 Net Stats](images/after-network.png) |

- Timing Insights 적용 후: ① `GameNetDriver`(`% Root` 71.48%, Excl 16.86초), ② `LabResourceNode`(Count 609, 12.52ms, `% Parent` 0.07%), ③ 두 북마크.
- Network Insights 적용 후: ① `Actor`와 `LabNpc`의 비트(1,163,648, 836,895), ② `LabResourceNode`의 비트(9번, 810), ③ 고른 측정 구간(1,785패킷, 59.883초).

## 7. 정확성 확인

| 항목 | 적용 전(`relevancy2-r3`) | 적용 후(`dormancy2-r2`) | 근거 |
| --- | --- | --- | --- |
| 연결당 열린 액터 채널 수 | 118 | 20 | CSV |
| 클라이언트에 존재하는 노드 수(1번, 이동, t=45s) | 114 | 301 | 자동 스크린샷의 화면 글자 |
| 클라이언트에 존재하는 노드 수(1번, t=75s) | 111 | 313 | 같음 |
| 클라이언트에 존재하는 노드 수(0번, 채집, t=75s) | 80 | 195 | 같음 |
| 클라이언트에 존재하는 NPC 수(1번, t=45s / t=75s) | 7 / 4 | 7 / 4 | 같음 |
| 측정 구간에 `Connection 0`이 받은 노드 갱신 | 27번 | 9번 | Network Insights |

채집과 재생성의 확인은 측정과 별도로 서버 하나에 클라이언트 두 개를 띄워서 했다(`Scripts/run-manual.ps1`, 자원 노드 5,001, NPC 300). 하나는 검증용 노드 옆에서 채집하고, 다른 하나(관찰자)는 직접 조작했다. 두 창을 1\~4초 간격으로 찍어 비교했다.

| 확인한 것 | 결과 |
| --- | --- |
| 채집 담당이 검증용 노드를 고갈시킬 때 관찰자 화면에서도 사라지고 20초 뒤 다시 나타나는가 | 예. 두 창에서 같은 표본에 사라지고 같은 표본에 다시 나타났다. 사라져 있는 시간은 표본 간격(4초) 안에서 20초와 맞았다(`RespawnSeconds` 20, `Source/DSOptLab/LabResourceNode.h`) |
| 관찰자가 200m 이상 멀어졌다가 돌아왔을 때, 그 사이에 바뀐 상태가 맞게 보이는가 | 예. 관찰자를 270m 밖에 40초 두었다가(그동안 노드는 한 번 이상 재생성되고 고갈된다. 주기 약 26초) 되돌렸다. 도착한 순간 두 창 모두 노드가 고갈된 상태였고, 7초 뒤 같은 표본에 다시 나타났다. 278m 밖에 27초 둔 다른 실행에서도 돌아온 뒤 한 주기 동안 두 창이 일치했다 |
| 관찰자가 멀리 있을 때 화면의 노드 수가 줄지 않는가 | 예. 멀어지는 동안 193에서 312로 늘었고, 278m 밖에 서 있는 27초 동안 312로 유지됐다. 돌아온 뒤에도 312였다 |

두 번째 항목에는 한계가 있다. 노드는 주기의 대부분(26초 중 20초)을 고갈 상태로 보내서, 도착한 순간의 일치만으로는 멀리 있는 동안의 변화가 전달된 것인지 가릴 수 없다. 확인한 것은 돌아온 뒤 화면이 채집 담당과 어긋나지 않았다는 것까지다.

## 8. 엔진 소스 위치

| 사실 | 위치 |
| --- | --- |
| Dormant 상태에 들어가려면 채널이 있어야 한다(`ShouldActorGoDormant`는 채널이 없으면 false) | `Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5506-5526` |
| 서버는 이 연결에서 Dormant 상태인 액터를 건너뛴다 | `NetDriver.cpp:5618-5624`(`IsActorDormant`, `5499-5503`) |
| 채널이 없는 액터의 거리 검사는 Dormant 상태 검사보다 앞에 있다 | `NetDriver.cpp:5580-5593` |
| `FlushNetDormancy()`는 Dormant 상태를 풀고 `NetDormancy` 값은 그대로 둔다 | `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:3176-3178`, `Actor.cpp:3102` |
| Consider List는 활성 목록만 돈다 | `NetDriver.cpp:5315` |
| 액터는 Dormant 상태인 연결 수가 전체 연결 수와 같아질 때 활성 목록에서 빠진다 | `FNetworkObjectList::MarkDormant`, `Engine/Source/Runtime/Engine/Private/NetworkObjectList.cpp:348-376` |
| 연결별 Dormant 상태 등록은 액터 채널에서만 불린다 | `Engine/Source/Runtime/Engine/Private/DataChannel.cpp:2354, 2461, 2728` |
| 맵에 미리 놓인 `DORM_Initial` 액터는 처음부터 활성 목록에서 빠진다. 이 프로젝트의 자원 노드는 실행 중에 스폰해서 해당하지 않는다 | `NetDriver.cpp:5369-5378` |
| Relevancy를 잃은 액터의 채널을 닫는 코드는 채널이 있을 때만 동작한다 | `NetDriver.cpp:5877-5889` |

정리한 내용은 [engine-notes.md](../../Docs/Reference/engine-notes.md)의 "휴면 액터와 관련성"에 있다.

## 9. 시각 자료의 사정

- 요약의 내려다보기 화면은 1번 클라이언트의 t=75s다(`nodes=111` → `nodes=313`). t=45s의 화면은 `images/before-topdown.png`, `images/after-topdown.png`에 있다.
- 3인칭 화면(`images/before-tpp.png`, `images/after-tpp.png`, 0번 클라이언트 t=75s)은 보이는 모습이 같고 화면 글자의 노드 수만 80에서 195로 달라서 본문에 넣지 않았다.
- 전후 영상(`images/before-clip.gif`, `images/after-clip.gif`)은 수치를 쓰지 않는 시각 자료 전용 실행 `visual2-r1`, `visual5-r1`에서 찍었다. 같은 경로를 걷는 1번과 2번 클라이언트의 10초다(적용 전 t=49\~58s, 적용 후 t=50\~59s). 적용 전에는 노드 수가 108\~115에 머물고, 적용 후에는 306에서 313으로 는다. 본문에는 넣지 않았다.
- 개념도 `dormancy-trail.svg`는 `Scripts/make-dormancy-trail.ps1`이 만든다. 경로(한 변 100m), 150m 원, 점의 밀도는 소스의 값이고, 점의 위치는 예시다. Relevancy만 적용한 쪽에서 원 밖으로 나간 액터는 실제로는 약 5초(`RelevantTimeout`) 뒤에 사라지는데, 도식은 바로 끈다. 8배속이다.

## 10. 확인하지 않은 것

- `GameNetDriver` Exclusive의 내역. 거리 검사, 우선순위 정렬, 송신이 섞여 있다. 1.00ms 줄어든 것이 실제 변화인지 이 측정으로는 가릴 수 없다.
- `r1`이 다른 두 실행보다 11\~17% 느린 이유.
- `r1`, `r3`과 `Connection 1`\~`7`의 연결당 송신 대역폭. `Connection 0`은 제자리에서 채집하는 연결이라 이동하는 연결과 다를 수 있다.
- 오래 돌아다닌 클라이언트에 쌓이는 자원 노드의 메모리 비용과 해결 방법([backlog.md](../../Docs/backlog.md) "작업 중 떠오른 것").
