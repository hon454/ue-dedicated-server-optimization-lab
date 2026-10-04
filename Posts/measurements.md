# 누적 수치의 측정 기록

[루트 README](../README.md)의 "결과 한눈에 보기"가 쓰는 수치의 근거다. README는 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 실행 라벨은 여기에 둔다. 구성마다의 실행별 값과 계산식은 각 포스팅의 측정 기록에 있다.

| 구성 | 실행 라벨 | 측정 기록 |
| --- | --- | --- |
| 기준선 | `baseline3` | [Always Relevant 기준선](01-baseline/measurements.md) |
| Relevancy | `relevancy2` | [Relevancy와 Net Cull Distance](02-relevancy/measurements.md) |
| Dormancy | `dormancy6`(README의 표와 차트), `dormancy2`(Dormancy 글) | [자원 노드 Dormancy](03-dormancy/measurements.md), [AI NPC Net Update Frequency](04-update-frequency/measurements.md) |
| Net Update Frequency | `update-frequency3` | [AI NPC Net Update Frequency](04-update-frequency/measurements.md) |

규모와 측정 절차의 근거는 [테스트베드와 측정 방법의 측정 기록](00-testbed/measurements.md)에 있다.

## 1. 구성별 중앙값

모든 값은 구성마다 3회 실행한 중앙값이다.

| 지표 | 기준선<br>`baseline3` | Relevancy<br>`relevancy2` | Dormancy<br>`dormancy6` | Net Update Frequency<br>`update-frequency3` |
| --- | ---: | ---: | ---: | ---: |
| 서버 프레임 시간 평균(ms) | 198.43 | 17.64 | 13.60 | 13.16 |
| 서버 프레임 시간 P99(ms) | 262.96 | 26.64 | 20.84 | 19.50 |
| 리플리케이션 시간(ms/프레임) | 188.97 | 13.14 | 9.35 | 8.73 |
| 연결당 송신 대역폭(바이트/초) | 28,048 | 2,897 | 2,793 | 1,303 |
| 연결당 열린 액터 채널 수 | 5,314 | 118 | 20 | 20 |

- **출처.** 서버 프레임 시간과 리플리케이션 시간은 Timing Insights, 연결당 송신 대역폭은 Network Insights(`Connection 0`)에서 측정 구간을 읽은 값이고, 연결당 열린 액터 채널 수는 서버 CSV다.
- **서버 프레임 시간**은 프레임 시간에서 틱 속도 제한 대기를 뺀 시간이다([ADR-0010](../Docs/Decisions/0010-frame-time-without-tick-wait.md)). P99는 측정 구간의 프레임마다 이 값을 Insights에서 내보내 읽은 99백분위 경계값이다.
- **Dormancy 값.** README의 표와 차트의 Dormancy는 Net Update Frequency와 연달아 잰 `dormancy6`이다. 같은 코드를 Relevancy 직후에 잰 `dormancy2`는 서버 프레임 시간 평균이 14.69ms로 1.09ms 컸고(측정한 화면 조건이 달랐다), 기법별 변화는 연달아 잰 묶음끼리 계산했다. 기준선과 `update-frequency3`도 화면 조건이 다르지만, 이 1.09ms는 둘의 차이 185.27ms에 비해 작다.
- **연결당 송신 대역폭**은 세 실행 가운데 일부만 Network Insights에서 읽었고, 어느 실행의 값인지는 각 포스팅의 측정 기록에 있다. Relevancy부터는 연결마다 받는 액터가 위치에 따라 달라, `Connection 0`(제자리에서 채집하는 클라이언트)이 서버 CSV의 8개 연결 평균보다 35% 작다([Relevancy의 측정 기록](02-relevancy/measurements.md) 4절).

## 2. README의 수치와 출처

| README의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 198ms → 13.2ms | 198.43ms → 13.16ms(-93.4%) | 1절 |
| 틱 예산 33.3ms, 6배에서 절반 이하로 | 198.43 ÷ 33.3 = 5.96, 13.16 ÷ 33.3 = 0.40 | 틱 예산은 1000 ÷ `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`) |
| 줄어든 시간의 대부분은 Relevancy | 줄어든 185.27ms 가운데 180.79ms | 198.43 − 13.16, 198.43 − 17.64 |
| 처리 대상을 50분의 1로 | 연결 하나가 프레임마다 처리하는 자원 노드 5,001 → 75\~96개 | [Relevancy의 측정 기록](02-relevancy/measurements.md) 6절 |
| 클라이언트 8, 자원 노드 5,001, NPC 300, 맵 2km × 2km | 같음 | [테스트베드의 측정 기록](00-testbed/measurements.md) 1절 |
| UE 프로세스의 메모리 합계 27.5GB | 같음 | 같은 문서 1절(`calib-a-r1`) |
| 클라이언트 8개의 창, 서버는 논리 프로세서 2\~7, 클라이언트는 8번부터 | 서버 마스크 252, 클라이언트 마스크 4294967040 | 화면은 시각 자료 전용 실행 `visual13-r1`(2026-10-05, 2막 구성, 세 기법 적용)의 측정 구간에서 찍었다. [테스트베드 확장의 측정 기록](05-expanded-testbed/measurements.md) 11절. 마스크는 [ADR-0009](../Docs/Decisions/0009-server-cores-without-dpc-load.md) |
| 플레이어 여덟 명이 3m 간격으로 모여 서로 다른 방향으로 돈다 | `-PlayerSpacing 3`, 한 변 70m 정사각형, 홀수 자리는 반대 방향 | [2막 설계](../Docs/Planning/2026-10-03-act-2-design.md) 3.1, `Source/DSOptLab/LabGameMode.cpp`의 `GetSlotLocation` |

## 3. 기준선과 최종 구성

| 지표 | 기준선(`baseline3`) | 세 기법 적용(`update-frequency3`) | 변화 |
| --- | ---: | ---: | ---: |
| 서버 프레임 시간 평균(ms) | 198.43 | 13.16 | -93.4% |
| 서버 프레임 시간 P99(ms) | 262.96 | 19.50 | -92.6% |
| 리플리케이션 시간(ms/프레임) | 188.97 | 8.73 | -95.4% |
| 연결당 송신 대역폭(바이트/초) | 28,048 | 1,303 | -95.4% |
| 연결당 열린 액터 채널 수 | 5,314 | 20 | -99.6% |

## 4. 기법별 변화

각 기법은 바로 앞 구성과 비교한다. "구별되지 않음"은 중앙값의 변화가 두 구성 중 큰 쪽의 변동 폭(세 실행의 최댓값 − 최솟값)보다 작다는 뜻이다.

| 기법 | 비교한 실행 | 줄어든 것 | 구별되지 않음 |
| --- | --- | --- | --- |
| [Relevancy](02-relevancy/README.md) | `baseline3` → `relevancy2` | 서버 프레임 시간 평균 -91.1%, P99 -89.9%, 리플리케이션 시간 -93.0%, 연결당 송신 대역폭 -89.7%, 연결당 열린 액터 채널 수 5,314 → 118 | 없음 |
| [자원 노드 Dormancy](03-dormancy/README.md) | `relevancy2` → `dormancy2` | 서버 프레임 시간 평균 -16.7%, 리플리케이션 시간 -22.6%, 연결당 열린 액터 채널 수 118 → 20 | 서버 프레임 시간 P99, 연결당 송신 대역폭 |
| [AI NPC Net Update Frequency](04-update-frequency/README.md) | `dormancy6` → `update-frequency3` | 연결당 송신 대역폭 -53.3%, 서버 프레임 시간 P99 -6.4%, 리플리케이션 시간 -6.6% | 서버 프레임 시간 평균 |

## 5. 차트

기준선이 다른 구성보다 열 배 이상 커서, Relevancy 이후의 구성만 따로 그린다. "Dormancy"는 `dormancy6`이다.

```mermaid
xychart-beta
    title "서버 프레임 시간 평균 (ms), Relevancy 이후"
    x-axis ["Relevancy", "Dormancy", "Net Update Frequency"]
    y-axis "ms" 0 --> 20
    bar [17.64, 13.60, 13.16]
```

```mermaid
xychart-beta
    title "리플리케이션 시간 (ms/프레임), Relevancy 이후"
    x-axis ["Relevancy", "Dormancy", "Net Update Frequency"]
    y-axis "ms" 0 --> 15
    bar [13.14, 9.35, 8.73]
```

```mermaid
xychart-beta
    title "연결당 송신 대역폭 (바이트/초), Relevancy 이후"
    x-axis ["Relevancy", "Dormancy", "Net Update Frequency"]
    y-axis "바이트/초" 0 --> 3000
    bar [2897, 2793, 1303]
```
