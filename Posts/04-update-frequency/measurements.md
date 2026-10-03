# AI NPC Net Update Frequency: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값과 영상의 가공 값은 [후보 기법 자료](candidates.md)에 있다.

## 1. 실행 조건

- 적용 전은 `dormancy6`, 적용 후는 `update-frequency3`이다. 각각 같은 명령으로 세 번 실행했다(`r1`\~`r3`).
- `dormancy6`은 `SetNetUpdateFrequency` 한 줄을 되돌린 빌드로 쟀고, 이어서 `update-frequency3`을 쟀다. 두 묶음은 같은 날 같은 화면(본체 모니터, 3840×2160)에서 연달아 쟀다.
- 조건: 클라이언트 8, 자원 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 여섯 실행 모두 종료 코드 0으로 끝났고 `saturated_ratio`는 0.000이다.
- **Dormancy 글의 수치와 다른 이유.** [자원 노드 Dormancy](../03-dormancy/README.md)의 적용 후 수치(`dormancy2`)는 이 글의 적용 전과 코드가 같은데 `work_avg_ms` 중앙값이 14.409로 `dormancy6`의 13.343보다 1.066ms 크다(Insights의 서버 프레임 시간 평균으로는 14.69ms와 13.60ms). 이번 기법의 변화(0.449ms)보다 크고 변동 폭도 커서(`work_avg_ms` 2.032), 이 글의 비교에는 연달아 잰 `dormancy6`을 썼다. 두 묶음의 차이가 어디서 왔는지는 확인하지 않았다. 두 글의 수치를 직접 비교하면 안 된다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 연결당 송신 대역폭 2,790 → 1,300바이트/초(-53%) | 2,793 → 1,303바이트/초(-53.3%) | Network Insights `Connection 0`(두 구성 모두 `r1`). 3절 |
| 리플리케이션 시간 9.35ms → 8.73ms(-6.6%) | 같음 | Timing Insights `GameNetDriver`, 세 실행의 중앙값(`dormancy6-r1`, `update-frequency3-r3`) |
| 서버 프레임 시간 평균 13.6ms → 13.2ms, 구별되지 않음 | 13.60ms → 13.16ms(-3.2%) | Timing Insights, 세 실행의 중앙값(둘 다 `r1`). 변화 0.44ms가 변동 폭 0.51ms보다 작다 |
| NPC의 화면 위치가 바뀌는 간격 46ms → 131ms, 2.8배 | 46.4ms → 130.6ms | 60fps 영상(`visual9-r1`, `visual10-r1`). 7절 |
| 송신 비트의 72%가 NPC | 846,108 ÷ 1,177,510 = 71.9% | `dormancy6-r1` `Connection 0`의 `LabNpc` ÷ `Actor` |
| 리플리케이션 시간의 4% | 0.375 ÷ 9.35 = 4.0% | `dormancy6-r1`의 `LabNpc` 671.55ms ÷ `WorldTick` 1,792 |
| 60초에 NPC 갱신 7,622번, 한 번 111비트, 이동 정보 92비트 | 846,108 ÷ 7,622 = 111.0 | `dormancy6-r1` `Connection 0`, 1,795패킷. 패킷 하나에 평균 4.25번. `ReplicatedMovement`가 92비트 |
| 빈도 100: 10\~43ms, 평균 약 43ms | 1 ÷ 100 + 0\~1/30초. 난수가 0.7보다 작으면 다음 프레임(33.3ms), 아니면 그다음 프레임(66.7ms). 0.7 × 1 + 0.3 × 2 = 1.3프레임 | 계산값. 6절의 엔진 소스 |
| 빈도 10: 100\~133ms, 네 번째 프레임, 약 133ms | 1 ÷ 10 + 0\~1/30초. 세 프레임 뒤는 100ms라 거의 항상 이르다 | 계산값 |
| 예상 0.325배 | 1.3 ÷ 4 | 계산값 |
| 실제 2,472번(0.324배) | 2,472 ÷ 7,622 | `update-frequency3-r1` `Connection 0`, 1,797패킷 |
| NPC 하나의 갱신 간격 약 134ms, 초당 약 7.5번 | 4패킷 × 33.4ms | `update-frequency3-r1`의 NetId 1306이 실린 패킷 순번 6,329, 6,333, 6,337, 6,341, 6,345, 6,349. 패킷 간격은 60.075초 ÷ 1,797 |
| NPC의 처리 시간 0.375ms → 0.147ms | -60.8% | `LabNpc` Incl ÷ `WorldTick` Count(`dormancy6-r1`, `update-frequency3-r3`) |
| 변화 0.44ms, 실행 사이의 차이 0.51ms | 13.60 − 13.16, `dormancy6`의 변동 폭 | 3절 |
| 한 번에 움직이는 거리 약 11cm → 약 41cm | 3.7px → 14.1px(중앙값) | 왕복 구간 10m가 화면에서 약 348px. 7절 |
| NPC는 초당 300cm | `MoveSpeed` | `Source/DSOptLab/LabNpc.h` |
| 8개 연결의 평균으로는 -44% | 4,317 → 2,400(-44.4%) | CSV `out_bytes_per_sec_per_conn`의 중앙값. 4절 |
| 리플리케이션 시간의 95%가 네트워크 드라이버 자체 시간 | 8.29 ÷ 8.73 = 95.0% | 5절 |

## 3. 세 실행의 Insights 값

| 지표 | Dormancy `r1` | `r2` | `r3` | 중앙값(변동 폭) | Net Update Frequency `r1` | `r2` | `r3` | 중앙값(변동 폭) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 서버 프레임 시간 평균(ms) | 13.60 | 13.86 | 13.35 | 13.60(0.51) | 13.16 | 12.74 | 13.19 | 13.16(0.45) |
| 서버 프레임 시간 P99(ms) | 20.84 | 21.47 | 20.50 | 20.84(0.97) | 19.50 | 18.63 | 19.59 | 19.50(0.96) |
| 리플리케이션 시간(ms/프레임) | 9.35 | 9.46 | 8.98 | 9.35(0.48) | 8.82 | 8.48 | 8.73 | 8.73(0.34) |
| 연결당 송신 대역폭(바이트/초, `Connection 0`) | 2,793 | 읽지 않음 | 읽지 않음 | | 1,303 | 읽지 않음 | 읽지 않음 | |
| 연결당 열린 액터 채널 수(CSV) | 20 | 20 | 20 | 20(0) | 20 | 20 | 20 | 20(0) |

- **서버 프레임 시간 평균은 구별되지 않는다.** 중앙값의 변화 0.44ms가 두 구성의 변동 폭 중 큰 쪽(0.51ms)보다 작다. 다만 Net Update Frequency의 세 실행(12.74\~13.19)은 모두 Dormancy의 세 실행(13.35\~13.86)보다 낮다.
- **P99와 리플리케이션 시간은 구별되는 차이다.** 변화 1.34ms(-6.4%), 0.62ms(-6.6%)가 변동 폭 중 큰 쪽(0.97ms, 0.48ms)보다 크다.
- 서버 프레임 시간 평균은 (측정 구간 − 틱 속도 제한 대기) ÷ `Frame` Count다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md). Net Update Frequency `r1`: (60.003초 − 36.337초) ÷ 1,798). P99는 프레임마다 대기를 뺀 길이를 정렬한 ceil(N × 0.99)번째 값이다.
- 리플리케이션 시간은 `GameNetDriver` Incl ÷ `WorldTick` Count다(Net Update Frequency `r3`: 15.69초 ÷ 1,798).
- 연결당 송신 대역폭은 (`Actor` Incl + `PacketHeaderAndInfo` Incl)비트 ÷ 8 ÷ 선택 범위의 시간이다. Dormancy `r1`: (1,177,510 + 165,140) ÷ 8 ÷ 60.095초. Net Update Frequency `r1`: (460,936 + 165,324) ÷ 8 ÷ 60.075초.

## 4. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `dormancy6-r1` | 1,792 | 13.343 | 20.604 | 9.600 | 4,323 | 20 | 0.000 |
| `dormancy6-r2` | 1,794 | 13.593 | 21.044 | 9.700 | 4,317 | 20 | 0.000 |
| `dormancy6-r3` | 1,797 | 13.080 | 20.109 | 9.212 | 4,314 | 20 | 0.000 |
| 중앙값 | 1,794 | 13.343 | 20.604 | 9.600 | 4,317 | 20 | 0.000 |
| 변동 폭 | 5 | 0.513 | 0.935 | 0.488 | 9 | 0 | 0.000 |
| `update-frequency3-r1` | 1,797 | 12.894 | 19.080 | 9.054 | 2,402 | 20 | 0.000 |
| `update-frequency3-r2` | 1,795 | 12.479 | 18.298 | 8.719 | 2,400 | 20 | 0.000 |
| `update-frequency3-r3` | 1,798 | 12.923 | 19.327 | 8.969 | 2,400 | 20 | 0.000 |
| 중앙값 | 1,797 | 12.894 | 19.080 | 8.969 | 2,400 | 20 | 0.000 |
| 변동 폭 | 3 | 0.444 | 1.029 | 0.335 | 2 | 0 | 0.000 |

Insights 값과의 차이는 서버 프레임 시간 평균과 `work_avg_ms`가 +1.9\~+2.1%, P99와 `work_p99_ms`가 +1.1\~+2.2%, 리플리케이션 시간과 `netflush_avg_ms`가 -2.5\~-2.7%다.

## 5. 리플리케이션 시간의 내역

| 타이머(프레임당) | Dormancy `r1` | Net Update Frequency `r3` | 변화 |
| --- | --- | --- | --- |
| `LabNpc` | 0.375ms (4.0%) | 0.147ms (1.7%) | -60.8% |
| `LabResourceNode` | 0.006ms (0.07%) | 0.006ms (0.07%) | |
| `GameNetDriver` Exclusive | 8.68ms (92.8%) | 8.29ms (95.0%) | -4.5%(구별되지 않음) |
| 나머지 | 0.30ms | 0.28ms | |
| 합계(리플리케이션 시간) | 9.35ms | 8.73ms | -6.6% |

- 각 값은 타이머 Incl(Exclusive는 Excl) ÷ `WorldTick` Count이고, 괄호는 리플리케이션 시간 대비 비율이다. 두 구성 모두 리플리케이션 시간의 중앙값 실행이다.
- 연결 하나가 프레임마다 처리하는 NPC는 5.50개에서 1.78개가 됐다(Net Update Frequency `r1`: 25,580 ÷ 1,797 ÷ 8. 세 실행에서 1.78\~1.79, Dormancy는 5.49\~5.50). 비율 0.324는 계산값 0.325와 맞는다.
- 줄어든 0.62ms 가운데 NPC 타이머의 몫은 0.23ms다. 나머지 0.39ms는 `GameNetDriver` Exclusive에서 줄었는데, 세 실행의 범위(8.05\~8.36ms)가 Dormancy의 범위(8.33\~8.78ms)와 겹쳐 줄었다고 확정할 수 없다.
- 액터 비트에서 NPC가 차지하는 비율은 71.9%에서 59.5%가 됐다(`Actor` 460,936비트 중 `LabNpc` 274,436비트). 패킷 수는 1,795와 1,797로 그대로다.

## 6. 엔진 소스 위치

| 사실 | 위치 |
| --- | --- |
| 서버는 다음 고려 시각이 오지 않은 액터를 건너뛴다(`World->TimeSeconds <= ActorInfo->NextUpdateTime`) | `Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5319-5323` |
| 다음 고려 시각은 지금 + `RandDelay` + 1 ÷ `NetUpdateFrequency` | `NetDriver.cpp:5420-5425` |
| `RandDelay`는 0에서 서버 틱 시간(1 ÷ 30초) 사이의 난수다 | `NetDriver.cpp:6341-6348` |
| `NetUpdateFrequency` 기본값 100, `MinNetUpdateFrequency` 2 | `Engine/Source/Runtime/Engine/Private/Actor.cpp:295-296` |
| Adaptive Net Update Frequency는 꺼져 있다(`net.UseAdaptiveNetUpdateFrequency` 기본값 0) | `NetDriver.cpp:523-526` |

적용 전 서버는 60.028초에 1,793프레임, 프레임 간격 33.5ms로 돈다(`dormancy6-r1`).

## 7. NPC의 끊김

끊김을 보려고 시각 자료 전용 실행을 따로 돌렸다. 서버 인자 `-LabShowcaseNpc`를 주면 0번 클라이언트(제자리에서 채집) 앞 10m에 NPC 하나가 더 생기고, 화면 오른쪽 절반을 가로지르는 10m 직선을 300cm/s로 왕복한다. 위치가 시작 신호 뒤의 경과 시간만으로 정해져서, 두 실행에서 같은 NPC가 같은 시각에 같은 자리를 지난다. 이 NPC는 측정 실행에는 없다.

확정 규모에 이 NPC를 더해 한 줄을 되돌린 빌드(`visual9-r1`)와 적용한 빌드(`visual10-r1`)를 실행하고, 0번 클라이언트 창을 시작 신호 뒤 약 50초부터 8초 동안 60fps로 찍었다. `before-npc.gif`, `after-npc.gif`는 NPC가 지나가는 띠(440×120픽셀)만 줄이지 않고 잘라 4배 느리게 재생한 것이다.

같은 원본에서 프레임마다 NPC(빨간 원뿔)의 화면 x좌표를 읽었다(481프레임, 빨간색 픽셀의 중심).

| 항목 | 적용 전(`visual9-r1`) | 적용 후(`visual10-r1`) |
| --- | --- | --- |
| 위치가 바뀐 횟수(8초) | 172 | 61 |
| 위치가 바뀐 프레임 사이의 간격(평균) | 46.4ms | 130.6ms |
| 간격의 중앙값 / 최댓값 | 33.3ms / 116.7ms | 133.3ms / 183.3ms |
| 한 번에 움직인 픽셀(중앙값 / 최댓값) | 3.7px / 11.0px | 14.1px / 15.2px |
| 거리로 환산한 중앙값 | 약 11cm | 약 41cm |

- 거리는 왕복 구간 10m가 화면에서 약 348px인 것으로 환산했다(적용 전 345px, 적용 후 351px).
- 클라이언트는 30fps로 그리므로 간격의 최소 단위는 33.3ms다. `LabNpc`는 받은 위치를 그대로 적용하고 보간하지 않는다.
- 화면 위치는 영상에서 색으로 찾은 값이고, 클라이언트가 위치를 적용한 시각을 직접 기록한 것은 아니다.
- 두 클라이언트가 걷는 영상(`before-clip.gif`, `after-clip.gif`, `visual5-r1`과 `visual6-r1`, 8fps)에서는 끊김을 구별하지 못해 본문에 넣지 않았다.

## 8. Insights 화면과 정확성 확인

| 적용 전(`dormancy6-r1`) | 적용 후(`update-frequency3-r1`) |
| --- | --- |
| ![적용 전 Timers와 WorldTick Callees](images/before-timing.png) | ![적용 후 Timers와 WorldTick Callees](images/after-timing.png) |
| ![적용 전 Connection 0 Outgoing의 Net Stats](images/before-network.png) | ![적용 후 Connection 0 Outgoing의 Net Stats](images/after-network.png) |

- Timing Insights: ① `GameNetDriver`(Incl 16.76초, Excl 15.55초 → Incl 15.84초, Excl 15.02초), ② `LabNpc`(Count 78,876, 671.55ms → Count 25,580, 279.12ms), ③ 두 북마크.
- Network Insights: ① `Actor`의 비트(1,177,510 → 460,936), ② `LabNpc`의 비트(7,622번 846,108 → 2,472번 274,436), ③ 고른 측정 구간(1,795패킷 60.095초 → 1,797패킷 60.075초).

| 항목 | 적용 전(`dormancy6-r1`) | 적용 후(`update-frequency3-r1`) | 근거 |
| --- | --- | --- | --- |
| 연결당 열린 액터 채널 수 | 20 | 20 | CSV |
| 클라이언트에 존재하는 노드 수와 NPC 수(1번, 이동, t=45s) | 301, 7 | 301, 7 | 자동 스크린샷의 화면 글자 |
| 클라이언트에 존재하는 노드 수와 NPC 수(1번, t=75s) | 313, 4 | 313, 5 | 같음 |
| 클라이언트에 존재하는 노드 수와 NPC 수(0번, 채집, t=75s) | 195, 5 | 195, 5 | 같음 |

화면의 액터 수와 열린 채널 수는 예상대로 변하지 않았다. 한 곳만 다르다. 1번 클라이언트의 t=75s 화면에서 NPC 수가 4에서 5가 됐다. 화면에 보이는 빨간 점은 두 화면 모두 네 개이고 위치도 같다. 다섯 번째 NPC가 왜 남아 있는지는 확인하지 않았다. 내려다보기 화면과 3인칭 화면(`images/before-topdown.png` 등)은 전후가 같아 본문에 넣지 않았다.

## 9. 확인하지 않은 것

- 10이 아닌 값에서의 대역폭과 갱신 간격.
- 송신 한도에 포화된 조건에서의 효과. 두 구성 모두 `saturated_ratio`가 0.000이다.
- 거리나 이동 방향이 다른 NPC, 프레임 수가 다른 클라이언트에서의 끊김. 시연용 NPC 하나로만 확인했다.
- `GameNetDriver` Exclusive의 내역과, 0.39ms 준 것이 실제 변화인지.
- `r2`, `r3`과 `Connection 1`\~`7`의 연결당 송신 대역폭. 적용 전의 갱신 간격도 패킷에서 따로 읽지 않았다.
- `dormancy2`와 `dormancy6`이 7\~8% 다른 이유.
