# 포스팅 6 관찰 자료 (`act2-baseline11`, `act2-relevancy1`, `act2-dormancy1`, `act2-update-frequency1`)

2막 구현 계획 태스크 22.2의 자료다. 에이전트가 네 구성의 중앙값 실행과 `act2-update-frequency1`의 나머지 두 실행의 트레이스를 창 없이 Unreal Insights 내보내기 명령으로 읽었다([insights-reading.md](../../Docs/Guides/insights-reading.md) "에이전트가 직접 열 때 알아 둘 것"). 실행 조건과 CSV 값은 [측정 기록](measurements.md)에 있다.

- 대상 실행: 기준선 `act2-baseline11-r2`, Relevancy `act2-relevancy1-r2`, Dormancy `act2-dormancy1-r2`, Net Update Frequency `act2-update-frequency1-r1`(구성마다 `work_avg_ms`의 중앙값 실행). 그리고 `act2-update-frequency1-r2`(19.927로 큰 실행)와 `r3`.
- 1\~6절은 내보낸 값, 창에서 읽은 값과 그 값으로 계산한 값이다. 7절 "에이전트 의견"만 해석이다.
- 프레임당 값은 Incl ÷ 선택 구간의 `WorldTick` Count다. "한 번"은 Incl ÷ Count다. "연결 하나당"은 프레임당 Count ÷ 8이다.
- 내보낸 명령은 `TimingInsights.ExportTimingEvents`(`Frame`, `FEngineLoop_UpdateTimeAndHandleMaxTickRate`), `TimingInsights.ExportTimerStatistics`, `TimingInsights.ExportTimerCallees -timers=WorldTick`이고 모두 `-threads=GameThread`다. 두 번째와 세 번째는 응답 파일(`-ExecOnAnalysisCompleteCmd="@=<파일>"`, `ExportCommandsTests.cpp:234`)로 한 번에 실행했다.

## 1. 측정 구간

서버 로그의 프레임 번호로 구했다. `Measuring 60s` 줄이 측정을 시작한 프레임, CSV 행을 찍은 줄이 끝난 프레임이다. 내보낸 `Frame` 이벤트의 마지막이 로그의 마지막 프레임이다.

| 실행 | 로그의 시작, 끝, 마지막 프레임(1,000으로 나눈 나머지) | `-startTime` | `-endTime` | 선택 구간 | `WorldTick` Count | CSV `frames` |
| --- | --- | --- | --- | --- | --- | --- |
| `act2-baseline11-r2` | 173, 454, 455 | 91.5187초 | 151.5826초 | 60.064초 | 281 | 281 |
| `act2-relevancy1-r2` | 200, 840, 841 | 90.7094초 | 150.7191초 | 60.010초 | 1,640 | 1,640 |
| `act2-dormancy1-r2` | 333, 119, 120 | 90.8982초 | 150.9243초 | 60.026초 | 1,786 | 1,786 |
| `act2-update-frequency1-r1` | 347, 142, 143 | 91.0441초 | 151.0599초 | 60.016초 | 1,795 | 1,795 |
| `act2-update-frequency1-r2` | 326, 120, 121 | 90.5118초 | 150.5083초 | 59.996초 | 1,794 | 1,794 |
| `act2-update-frequency1-r3` | 307, 103, 104 | 89.9190초 | 149.9227초 | 60.004초 | 1,796 | 1,796 |

여섯 실행 모두 `WorldTick` Count가 CSV `frames`와 같다. 끝 프레임에서 `frames`를 뺀 프레임의 나머지도 `Measuring 60s` 줄의 프레임과 같다.

## 2. 서버 프레임과 CSV 대조 (Timing Insights)

서버 프레임 시간은 측정 구간의 프레임마다 (`Frame` − 그 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate`)의 평균이다([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)). P99는 같은 값을 정렬한 ceil(N × 0.99)번째다. 리플리케이션 시간은 `GameNetDriver` Incl ÷ `WorldTick` Count다.

| 실행 | 서버 프레임 시간(CSV `work_avg_ms`, 차이) | P99(CSV `work_p99_ms`, 차이) | 리플리케이션 시간(CSV `netflush_avg_ms`, 차이) |
| --- | --- | --- | --- |
| `act2-baseline11-r2` | 213.713ms(213.555, +0.07%) | 262.209ms(261.919, +0.11%) | 204.327ms(204.880, -0.27%) |
| `act2-relevancy1-r2` | 35.904ms(35.674, +0.64%) | 54.276ms(54.066, +0.39%) | 30.934ms(31.224, -0.93%) |
| `act2-dormancy1-r2` | 19.814ms(19.536, +1.42%) | 28.277ms(27.966, +1.11%) | 14.744ms(15.024, -1.86%) |
| `act2-update-frequency1-r1` | 17.022ms(16.732, +1.73%) | 24.490ms(24.051, +1.83%) | 11.967ms(12.225, -2.11%) |
| `act2-update-frequency1-r2` | 20.213ms(19.927, +1.44%) | | 14.525ms(14.818, -1.98%) |
| `act2-update-frequency1-r3` | 16.588ms(16.311, +1.70%) | | 11.656ms(11.913, -2.16%) |

차이는 (Insights − CSV) ÷ CSV다. 틱 예산 안의 구성에서 차이가 1\~2%로 커지는 것은 1막의 `relevancy2`(P99 +1.6\~+2.5%)와 같은 범위다.

## 3. `GameNetDriver` 안의 내역 (중앙값 실행, 프레임당 ms)

`ExportTimerCallees`에서 `GameNetDriver`의 바로 아래 타이머다. 클래스 타이머는 `UActorChannel::ReplicateActor`가 C++ 부모 클래스 이름으로 남긴다(`Engine/Source/Runtime/Engine/Private/DataChannel.cpp:3622-3625`). 0.01ms보다 작은 타이머는 "나머지"에 넣었다.

| 타이머 | 기준선 | Relevancy | Dormancy | Net Update Frequency |
| --- | ---: | ---: | ---: | ---: |
| `GameNetDriver` Incl | 204.327 | 30.934 | 14.744 | 11.967 |
| `GameNetDriver` Excl | 71.274 | 17.940 | 11.156 | 10.141 |
| `LabResourceNode` | 92.042 | 1.818 | 0 | 0 |
| `LabNpc`(`LabStateComponent` 포함) | 28.630 | 3.176 | 2.688 | 0.965 |
| `LabBuilding` | 9.613 | 6.987 | 0.012 | 0.012 |
| `LabCharacter`(인벤토리, 상태 값 포함) | 1.945 | 0.737 | 0.648 | 0.619 |
| 나머지(`PlayerState`, `GameStateBase`, `LabPlayerController` 등) | 0.823 | 0.276 | 0.240 | 0.230 |

| 타이머의 프레임당 Count(연결 하나당) | 기준선 | Relevancy | Dormancy | Net Update Frequency |
| --- | ---: | ---: | ---: | ---: |
| `LabResourceNode` | 40,008(5,001) | 779.6(97.5) | 0 | 0 |
| `LabNpc` | 2,800(350) | 378.1(47.3) | 354.0(44.3) | 114.5(14.3) |
| `LabBuilding` | 4,000(500) | 3,302.4(412.8) | 1.0(0.1) | 1.0(0.1) |
| `LabCharacter` | 64(8) | 53.0(6.6) | 49.5(6.2) | 49.5(6.2) |

| 한 번의 시간(µs) | 기준선 | Relevancy | Dormancy | Net Update Frequency |
| --- | ---: | ---: | ---: | ---: |
| `LabResourceNode` | 2.30 | 2.33 | | |
| `LabNpc` | 10.23 | 8.40 | 7.59 | 8.43 |
| `LabBuilding` | 2.40 | 2.12 | | |

- 나머지 = Incl − Excl − 표의 네 클래스. 기준선은 204.327 − 71.274 − 92.042 − 28.630 − 9.613 − 1.945 = 0.823이다.
- 처리 횟수의 식: 기준선은 Always Relevant라 액터 수 × 연결 8이다(`LabResourceNode` 5,001 × 8 = 40,008, `LabNpc` 350 × 8 = 2,800, `LabBuilding` 500 × 8 = 4,000).
- Net Update Frequency에서 `LabNpc`의 처리 횟수가 Dormancy의 32.3%다(114.5 ÷ 354.0). NPC의 `NetUpdateFrequency` 10 ÷ 서버 틱 30Hz = 33.3%와 비슷하다.
- `GameNetDriver` 밖의 큰 타이머(Net Update Frequency `r1`, 프레임당): `TickCompletionEvents` 2.462ms, `UNetConnection_ReceivedPacket` 1.484ms. `GameNetDriver` Excl 10.141ms 아래에는 더 나눌 타이머가 없다(`ExportTimerStatistics`의 GameThread 타이머 가운데 이 구간을 나누는 이름이 없음).

## 4. 기법마다 줄어든 몫 (중앙값 실행, `GameNetDriver` Incl의 변화, 프레임당 ms)

| 기법 | `GameNetDriver` Incl 변화 | Excl | `LabResourceNode` | `LabNpc` | `LabBuilding` | `LabCharacter`와 나머지 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Relevancy(기준선 → Relevancy) | -173.393 | -53.334 | -90.224 | -25.454 | -2.626 | -1.755 |
| Dormancy(Relevancy → Dormancy) | -16.190 | -6.784 | -1.818 | -0.488 | -6.975 | -0.125 |
| Net Update Frequency(Dormancy → Net Update Frequency) | -2.777 | -1.015 | 0 | -1.723 | 0 | -0.039 |

각 칸은 뒤 구성의 값 − 앞 구성의 값이다. 1막의 같은 변화(CSV `work_avg_ms`)는 Relevancy -91.3%(`baseline3` → `relevancy2`), Dormancy -17.1%(`relevancy2` → `dormancy2`), Net Update Frequency -3.4%(`dormancy6` → `update-frequency3`)이고, 2막은 -83.3%, -45.2%, -14.4%다([측정 기록](measurements.md) 3절, [STATUS.md](../../Docs/STATUS.md) "측정 결과").

## 5. `act2-update-frequency1-r2`가 느린 까닭

| 타이머(프레임당 ms) | `r1` | `r2` | `r3` | `r2` ÷ (`r1`, `r3`의 평균) |
| --- | ---: | ---: | ---: | ---: |
| `GameNetDriver` Excl | 10.141 | 12.380 | 9.863 | 1.24 |
| `LabNpc` | 0.965 | 1.129 | 0.951 | 1.18 |
| `LabCharacter` | 0.619 | 0.728 | 0.608 | 1.19 |
| `TickCompletionEvents` | 2.462 | 2.805 | 2.407 | 1.15 |
| `UNetConnection_ReceivedPacket` | 1.484 | 1.671 | 1.443 | 1.14 |
| `LabNpc` 처리 횟수(프레임당) | 114.5 | 114.5 | 114.5 | 1.00 |
| `LabCharacter` 처리 횟수(프레임당) | 49.5 | 49.3 | 49.5 | 1.00 |

처리 횟수는 세 실행이 같고, 한 번의 시간이 모든 타이머에서 14\~24% 길다. CSV의 `frames`(1,794)와 송신량(16,613)도 다른 두 실행과 같다.

## 6. 연결 하나의 패킷 (Networking Insights)

사용자가 컴퓨터 조작 권한을 허용해 네 중앙값 실행을 창(3000×2080)에서 읽었다(2026-10-05). `Game Instance 0 [Server]`, `Connection 0`(0번 자리, 채집 담당), `Outgoing`이다. 측정 구간의 첫 프레임과 마지막 프레임이 든 패킷 막대를 툴팁의 Engine Frame Number로 찾아 클릭과 Shift-클릭으로 골랐다. 화면 한 픽셀에 패킷 네 개가 들어가 경계가 한 프레임까지 어긋날 수 있다. 값은 Net Stats 패널에서 읽었고, 기준선과 Net Update Frequency는 `Scripts/capture-insights.ps1`로 찍은 창 이미지에서 읽었다.

| 실행 | 첫 막대(Largest Packet의 프레임) | 끝 막대 | 고른 범위 |
| --- | --- | --- | --- |
| `act2-baseline11-r2` | 1,173(1분 31.74초) | 1,453(2분 31.63초) | 2,324패킷, 59.890초 |
| `act2-relevancy1-r2` | 2,202(1분 30.85초) | 3,838(2분 30.70초) | 2,320패킷, 59.846초 |
| `act2-dormancy1-r2` | 2,333(1분 30.96초) | 4,119(2분 30.98초) | 2,186패킷, 60.024초 |
| `act2-update-frequency1-r1` | 2,347(1분 31.11초) | 4,142(2분 31.12초) | 2,126패킷, 60.012초 |

측정 구간은 기준선 1,173\~1,453, Relevancy 2,200\~3,839, Dormancy 2,333\~4,118, Net Update Frequency 2,347\~4,141프레임이다(1절의 프레임 번호 + 1). Relevancy의 끝 막대는 3,839를 담은 다음 막대(패킷 3개, 3,840까지)를 넣지 않았다.

| Net Stats 줄(Count / Incl 비트) | 기준선 | Relevancy | Dormancy | Net Update Frequency |
| --- | ---: | ---: | ---: | ---: |
| `Actor` | 101,530 / 17,271,959 | 87,100 / 14,183,449 | 88,612 / 14,331,238 | 36,357 / 7,454,034 |
| `LabNpc` | 98,316 / 10,859,271 | 75,853 / 7,677,403 | 77,472 / 7,841,975 | 25,182 / 2,549,348 |
| `LabInventoryComponent` | 127 / 2,182,034 | 126 / 2,163,804 | 127 / 2,182,034 | 128 / 2,200,264 |
| `BP_LabCharacter_C` | 2,248 / 325,014 | 9,758 / 1,474,408 | 9,865 / 1,488,111 | 9,875 / 1,491,418 |
| `LabStateComponent` | 4,229 / 364,574 | 753 / 64,230 | 757 / 63,422 | 749 / 64,486 |
| `LabBuilding` | 120 / 4,920 | 139 / 5,699 | 143 / 5,863 | 139 / 5,699 |
| `LabResourceNode` | 9 / 576 | 410 / 17,017 | 20 / 1,252 | 22 / 1,343 |
| `ReplicatedMovement` | 100,283 / 9,244,810 | 85,098 / 7,427,221 | 86,791 / 7,570,545 | 34,531 / 3,275,277 |
| `BunchHeader` | 101,530 / 3,484,734 | 87,100 / 2,717,452 | 88,612 / 2,675,602 | 36,357 / 1,081,235 |
| `PacketHeaderAndInfo`(`Actor` 밖) | 2,324 / 191,335 | 2,320 / 205,927 | 2,186 / 196,712 | 2,126 / 191,951 |

| 계산한 값 | 기준선 | Relevancy | Dormancy | Net Update Frequency |
| --- | ---: | ---: | ---: | ---: |
| 연결당 송신량(바이트/초) | 36,449 | 30,055 | 30,254 | 15,926 |
| CSV `out_bytes_per_sec_per_conn`(차이) | 37,328(-2.4%) | 31,205(-3.7%) | 31,260(-3.2%) | 16,641(-4.3%) |
| `LabNpc` 바이트/초(`Actor` 대비) | 22,665(62.9%) | 16,036(54.1%) | 16,331(54.7%) | 5,310(34.2%) |
| `LabInventoryComponent` 바이트/초(`Actor` 대비) | 4,554(12.6%) | 4,520(15.3%) | 4,544(15.2%) | 4,583(29.5%) |
| `BP_LabCharacter_C` 바이트/초(`Actor` 대비) | 678(1.9%) | 3,080(10.4%) | 3,099(10.4%) | 3,106(20.0%) |
| `LabNpc` 초당 횟수 | 1,641.6 | 1,267.5 | 1,290.7 | 419.6 |
| `BP_LabCharacter_C` 초당 횟수 | 37.5 | 163.1 | 164.4 | 164.6 |
| 서버 프레임(초당, CSV `frames` ÷ 60) | 4.68 | 27.33 | 29.77 | 29.92 |

- 연결당 송신량은 (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 고른 범위의 시간이다. 클래스의 바이트/초는 Incl ÷ 8 ÷ 시간, 초당 횟수는 Count ÷ 시간이다. CSV는 여덟 연결의 평균이라 `Connection 0` 하나와 2\~4% 다르다.
- **NPC의 초당 횟수 = 관련 NPC 수 × 서버 프레임 수.** 기준선은 NPC 350개 × 4.68 = 1,638이고 1,641.6과 같다. Relevancy는 연결 하나에 채널이 열린 NPC 47.3개(3절의 Count ÷ 8) × 27.33 = 1,293이고 1,267.5와 비슷하다. Relevancy가 NPC를 86% 덜 보내게 했지만, 서버가 5.8배 자주 돌아 초당 횟수는 23%만 줄었다.
- 캐릭터도 같다. 매 프레임 보내지는 캐릭터가 프레임이 늘어난 만큼 4.4배 많이 보내진다(37.5 → 163.1).
- 인벤토리는 4초마다 바뀌는 양이라 서버 프레임과 상관없이 초당 약 4,550바이트로 일정하다. Net Update Frequency 뒤에는 `Actor`의 29.5%다.
- Dormancy는 송신량을 거의 바꾸지 않았다(30,055 → 30,254). 건축물과 자원 노드는 Dormancy 전에도 거의 보내지지 않았다(건축물 60초에 139번, 자원 노드 410번).
- Net Update Frequency는 NPC의 초당 횟수를 32.5%로 줄였다(1,290.7 → 419.6).

## 7. 에이전트 의견

화면에서 읽은 사실이 아니라 해석이다. 기법과 순서는 사용자가 정한다.

- **`r2`는 일이 많았던 실행이 아니라 같은 일을 느리게 한 실행으로 보인다.** 처리 횟수가 같고 모든 타이머가 비슷한 비율로 길다(5절). 서버 코어가 그동안 느렸던 것으로 보이지만(클럭이나 같은 코어의 다른 작업), 확인하지 않았다. Job으로 클라이언트가 서버 코어에 오지 못하므로 클라이언트 탓은 아니다. 중앙값은 `r1`이라 결과 표에는 영향이 없다. 다만 변동 폭이 Dormancy 대비 차이보다 크므로, 포스팅에서 Net Update Frequency의 효과를 말할 때는 `r1`, `r3`과 처리 횟수(354.0 → 114.5)를 함께 근거로 드는 편이 낫다고 본다.
- **2막에서는 Dormancy의 몫이 커졌다.** 건축물 500개가 모두 플레이어 근처라 Relevancy로는 거의 줄지 않는다(연결 하나당 412.8개를 계속 처리). Dormancy가 건축물(-6.975)과 함께 `GameNetDriver` Excl도 6.784 줄였다. Dormant 상태의 액터가 Consider List에서 빠지므로 목록을 만들고 정렬하는 비용이 함께 준 것으로 보인다. Excl을 나누는 타이머가 없어 확인하지 않았다.
- **2막의 Relevancy가 송신량을 16%만 줄인 것은 서버가 빨라졌기 때문으로 보인다.** 연결 하나가 받는 NPC는 350개에서 47.3개로 줄었지만, 기준선 서버는 초당 4.68프레임만 돌아 NPC를 드물게 보냈다(6절). 1막은 플레이어가 맵 곳곳에 흩어져 연결마다 가까운 NPC가 적었다. 1막 Relevancy의 송신량 -85.9%(STATUS.md "측정 결과")와 다른 까닭으로 보이지만, 1막 트레이스의 NPC 초당 횟수는 이번에 다시 읽지 않았다.
- **세 기법 뒤에 남은 송신량의 절반은 NPC가 아니다.** Net Update Frequency 구성에서 인벤토리 29.5%, 캐릭터 20.0%다. 인벤토리는 4초마다 배열의 맨 앞 칸을 지우고 맨 뒤에 더하는데, 한 번에 평균 17,189비트, 항목 약 189개(`ItemId` Count 24,200 ÷ 128)를 보낸다. 앞 칸이 지워지면 뒤의 칸이 모두 한 칸씩 당겨져 바뀐 것으로 비교되기 때문으로 보인다. backlog의 FastArray 후보와 이어진다.
- **세 기법 뒤에 남은 비용은 대부분 `GameNetDriver` Excl이다.** Net Update Frequency 구성의 리플리케이션 시간 11.967ms 가운데 10.141ms(84.7%)다. 클래스 타이머로 보이는 일(NPC 0.965, 캐릭터 0.619)은 작다. 태스크 23에서 다음 기법을 고를 때는 Excl의 내역을 먼저 나누는 진단(backlog의 "자체 시간 나누기")이 필요해 보인다.

## 8. 확인하지 않은 것

- `GameNetDriver` Excl의 내역.
- `act2-update-frequency1-r2`에서 서버 코어가 느려진 원인.
