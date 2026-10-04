# 세 기법 다시 적용: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. Insights에서 읽은 원래 값은 [관찰 자료](candidates.md)에 있다.

## 1. 실행 조건

- 네 구성을 구성별로 세 번씩 연달아 실행했다(2026-10-05 00:37\~01:09, 본체 화면, 실행 중 PC 조작 없음). 순서는 기준선 `act2-baseline11`, Relevancy `act2-relevancy1`, Dormancy `act2-dormancy1`, Net Update Frequency `act2-update-frequency1`이다. 빌드는 `0f689c2`의 소스다.
- 조건: 클라이언트 8, 자원 노드 5,001, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 2막의 요소: `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`. 구성 인자는 기준선 `-AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100`, Relevancy `-NoNodeDormancy -NpcUpdateFrequency 100`, Dormancy `-NpcUpdateFrequency 100`, Net Update Frequency는 없음이다. 서버 로그의 `lab_config`가 실행마다 이 인자와 같았다.
- 서버와 클라이언트를 Job 객체로 코어에 묶은 스크립트로 쟀다([ADR-0016](../../Docs/Decisions/0016-affinity-through-job-objects.md), 커밋 `35c589b`). 12회 모두 종료 코드 0이고 선호도 재설정이 한 번도 없었다. [포스팅 5의 기준선](../05-expanded-testbed/measurements.md) `act2-baseline1`은 Job 없이 잰 묶음이라 이 표와 비교하지 않는다.
- Job 없이 같은 날 시작한 기준선 `act2-baseline2`, `3`, `5`, `7`, `8`, `9`, `10`의 `r1`은 측정 중 클라이언트 선호도 재설정으로 실패해 쓰지 않는다(`4`, `6`은 시작하지 않았다).
- 채널 수는 측정 시작 전에 안정됐다: 기준선 5,871, Relevancy 694\~697, Dormancy와 Net Update Frequency 77(서버 로그의 준비 구간 마지막 세 줄).

## 2. 본문의 수치와 출처

본문은 구성을 서버에서 바꾼 것으로 부른다. ① 거리 판정은 `act2-relevancy1`, ② Dormant 상태는 `act2-dormancy1`, ③ NPC 업데이트 빈도는 `act2-update-frequency1`의 구성이다. 이 파일과 [관찰 자료](candidates.md)의 표는 라벨을 따라 Relevancy, Dormancy, Net Update Frequency로 부른다.

서버 프레임 시간과 리플리케이션 시간은 구성마다 `work_avg_ms` 중앙값 실행의 Timing Insights 값이다(기준선 `act2-baseline11-r2`, Relevancy `act2-relevancy1-r2`, Dormancy `act2-dormancy1-r2`, Net Update Frequency `act2-update-frequency1-r1`). 연결당 송신 대역폭은 같은 실행의 Networking Insights `Connection 0`이다. 연결당 열린 액터 채널 수는 서버 CSV다.

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간 평균 214ms, 35.9ms, 19.8ms, 17.0ms | 213.713, 35.904, 19.814, 17.022ms | [관찰 자료](candidates.md) 2절 |
| 리플리케이션 시간 204ms, 30.9ms, 14.7ms, 12.0ms | 204.327, 30.934, 14.744, 11.967ms | 관찰 자료 2절, 3절(`GameNetDriver` Incl ÷ `WorldTick` Count) |
| 연결당 송신 대역폭 36,400, 30,100, 30,300, 15,900바이트/초 | 36,449, 30,055, 30,254, 15,926 | 관찰 자료 6절. (`Actor` + `PacketHeaderAndInfo`) ÷ 8 ÷ 고른 범위의 시간 |
| 연결당 열린 액터 채널 수 5,871, 695, 77, 77 | 같음 | 3절의 CSV `open_actor_channels_per_conn` |
| 처리 횟수(프레임마다) 46,917, 4,533, 423, 184 | 46,916.8, 4,532.9, 423.2, 183.6 | 관찰 자료 3절의 Callees에서 계산. `GameNetDriver` 바로 아래 클래스 타이머의 Count 합(RPC `ClientMoveResponsePacked` 제외) ÷ `WorldTick` Count |
| 그 가운데 건축물 4,000, 3,302, 1, 1 | `LabBuilding` 4,000.0, 3,302.4, 1.0, 1.0 | 관찰 자료 3절 |
| 그 가운데 NPC 2,800, 378, 354, 115 | `LabNpc` 2,800.0, 378.1, 354.0, 114.5 | 관찰 자료 3절 |
| 액터 1,000개, 클라이언트 8개면 처리 횟수 8,000번 | 1,000 × 8 | 설명을 위한 계산 예시 |
| 거리 판정만 켠 서버: 처리 횟수 4,533번, 그 가운데 건축물 3,302번(73%) | 3,302.4 ÷ 4,532.9 = 72.9% | 위의 값 |
| ①이 처리 횟수를 10분의 1로 줄였다 | 4,532.9 ÷ 46,916.8 = 9.7% | 위의 값 |
| ③이 NPC의 처리를 3분의 1로 줄였다 | 114.5 ÷ 354.0 = 32.3% | 위의 값 |
| 자원 노드 5,001개, 건축물 500개, NPC 350명 | 5,000 + 검증용 1, `-Buildings 500`, 맵 300 + 무리 곁 50 | 1절의 인자 |
| 틱 예산 33.3ms | 1000 ÷ 30 = 33.33 | `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`) |
| Dormant 상태가 거리 판정 구성의 서버 프레임 시간을 45% 더 줄였다 | 19.814 ÷ 35.904 − 1 = -44.8% | 관찰 자료 2절 |
| 기준선의 다른 측정 216ms | 216.067ms(`act2-baseline1-r2`) | [포스팅 5 측정 기록](../05-expanded-testbed/measurements.md) 2절 |
| 지도: 건축물 500개가 무리 중심에서 80m 안, 무리 곁의 NPC 50명 | `-Buildings 500`, `BuildingClusterRadius` 8,000cm, `-NpcsNearPlayers 50`, `NpcClusterRadius` 4,000cm | `Source/DSOptLab/LabGameMode.h`, 1절의 인자 |
| 거리 판정만 켠 서버: 연결 하나가 프레임마다 처리하는 건축물 500개 가운데 413개 | `LabBuilding` 프레임당 Count 3,302.4 ÷ 8 = 412.8 | 관찰 자료 3절 |
| Net Cull Distance 기본값 150m | `NetCullDistanceSquared` 225,000,000 | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| NPC의 Net Update Frequency 10 | `FLabServerConfig::NpcUpdateFrequency` 기본값 10 | `Source/DSOptLab/LabScenarioConfig.h:40`, `Source/DSOptLab/LabNpc.cpp:24` |
| 예상: 기준선의 건축물 처리 프레임당 9.61ms | 9.613ms | 관찰 자료 3절(`LabBuilding`, `act2-baseline11-r2`) |
| 예상: ②의 몫이 1막의 17%보다 커야 한다 | 1막 `relevancy2` → `dormancy2`의 서버 프레임 시간 평균 -16.7% | [누적 수치의 측정 기록](../measurements.md) 4절 |
| 1막: ① -91%, ② -17%, ③ 구별되지 않음 | -91.1%, -16.7%, 구별되지 않음(13.60 → 13.16ms) | 누적 수치의 측정 기록 4절 |
| 2막: ① -83%, ② -45% | 35.904 ÷ 213.713 − 1 = -83.2%, 19.814 ÷ 35.904 − 1 = -44.8% | 관찰 자료 2절 |
| 2막: ③ 구별되지 않음 | CSV `work_avg_ms` 중앙값의 변화 -2.804(19.536 → 16.732)가 Net Update Frequency의 변동 폭 3.616보다 작다. Insights 값으로는 -14.1%(17.022 ÷ 19.814 − 1) | 3절. 구별의 규칙은 [누적 수치의 측정 기록](../measurements.md) 4절 |
| ① 뒤 건축물 처리 6.99ms, 73%가 남았다 | 6.987ms, 6.987 ÷ 9.613 = 72.7% | 관찰 자료 3절 |
| ② 뒤 건축물 처리 0.012ms | 0.012ms | 관찰 자료 3절 |
| ②가 네트워크 드라이버 자체 시간을 6.78ms 줄였다 | `GameNetDriver` Excl 17.940 → 11.156ms, -6.784 | 관찰 자료 4절 |
| 모든 연결에서 Dormant 상태가 되면 Consider List에서 빠진다 | 활성 목록에서 빠지는 조건과 Consider List를 만드는 곳 | `FNetworkObjectList::MarkDormant`(`NetworkObjectList.cpp:348-376`), `ServerReplicateActors_BuildConsiderList`(`NetDriver.cpp:5315`). [engine-notes.md](../../Docs/Reference/engine-notes.md) "휴면 액터와 관련성" |
| 거리 판정은 대역폭을 18%만 줄였다 | 30,055 ÷ 36,449 − 1 = -17.5% | 관찰 자료 6절 |
| 받는 NPC 350명 → 47명 | 기준선 NPC 350(맵 300 + 무리 곁 50), Relevancy `LabNpc` 프레임당 Count 378.1 ÷ 8 = 47.3 | 관찰 자료 3절 |
| 서버 프레임(초당) 4.68 → 27.3 | `frames` 281 ÷ 60 = 4.68, 1,640 ÷ 60 = 27.33 | 3절 |
| NPC를 보낸 횟수(초당) 1,640 → 1,270 | `LabNpc` Count 98,316 ÷ 59.890 = 1,641.6, 75,853 ÷ 59.846 = 1,267.5 | 관찰 자료 6절 |
| 받는 NPC를 86% 줄였다, 5.8배 자주, 횟수는 23%만 | 47.3 ÷ 350 − 1 = -86.5%, 27.33 ÷ 4.68 = 5.84, 1,267.5 ÷ 1,641.6 − 1 = -22.8% | 위의 값 |
| 보낸 횟수가 3분의 1, 연결당 송신 대역폭 -47% | `LabNpc` 초당 횟수 1,290.7 → 419.6(32.5%), 15,926 ÷ 30,254 − 1 = -47.4% | 관찰 자료 6절 |
| 느린 실행이 3ms쯤 느렸다 | `act2-update-frequency1-r2`의 `work_avg_ms` 19.927, 다른 두 실행 16.732, 16.311 | 3절, 관찰 자료 5절 |
| NPC의 처리에 쓴 시간 2.69ms → 0.965ms | `LabNpc` 프레임당 Count 354.0 → 114.5, Incl 2.688 → 0.965ms | 관찰 자료 3절 |
| ① 뒤 화면: 건축물 500개와 가까운 자원 노드, NPC | 화면 글자 `nodes=125 npcs=58 buildings=500`(`act2-relevancy1-r2-topdown-03`), Dormancy `nodes=176 npcs=58 buildings=500`(`act2-dormancy1-r2-topdown-03`) | 자동 스크린샷(t=60초) |
| 남은 리플리케이션 시간의 85%가 네트워크 드라이버 자체 시간 | 10.141 ÷ 11.967 = 84.7% | 관찰 자료 3절 |
| 보낸 액터 데이터의 30%는 인벤토리 | `LabInventoryComponent` 2,200,264 ÷ `Actor` 7,454,034 = 29.5% | 관찰 자료 6절 |

- 요약의 두 이미지는 `act2-baseline11-r2-topdown-03.png`, `act2-relevancy1-r2-topdown-03.png`(t=60초)를 `baseline-topdown.png`, `relevancy-topdown.png`로 복사했다.
- 원리의 지도는 `Scripts/make-relevancy-map.ps1 -Act2`가 만든다. 자리, 경로, 150m 원, 건축물(80m)과 무리 곁 NPC(40m)의 반지름은 실제 비율이고, 점의 위치는 그 스크립트의 고정 시드로 뿌린 예시다.

## 3. 서버가 남긴 CSV

| 라벨 | `frames` | `work_avg_ms` | `work_p99_ms` | `over_budget_frames` | `netflush_avg_ms` | `out_bytes_per_sec_per_conn` | `open_actor_channels_per_conn` | `saturated_ratio` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-baseline11-r1` | 278 | 216.047 | 279.062 | 278 | 207.362 | 37,001 | 5,871 | 0.000 |
| `act2-baseline11-r2` | 281 | 213.555 | 261.919 | 281 | 204.880 | 37,328 | 5,871 | 0.000 |
| `act2-baseline11-r3` | 283 | 211.993 | 261.048 | 283 | 203.459 | 37,591 | 5,871 | 0.000 |
| 중앙값 | 281 | 213.555 | 261.919 | 281 | 204.880 | 37,328 | 5,871 | 0.000 |
| 변동 폭 | 5 | 4.054 | 18.014 | 5 | 3.903 | 590 | 0 | 0.000 |
| `act2-relevancy1-r1` | 1,595 | 36.623 | 56.858 | 872 | 32.184 | 30,853 | 695 | 0.000 |
| `act2-relevancy1-r2` | 1,640 | 35.674 | 54.066 | 919 | 31.224 | 31,264 | 695 | 0.000 |
| `act2-relevancy1-r3` | 1,653 | 35.156 | 54.672 | 814 | 30.822 | 31,205 | 695 | 0.000 |
| 중앙값 | 1,640 | 35.674 | 54.672 | 872 | 31.224 | 31,205 | 695 | 0.000 |
| 변동 폭 | 58 | 1.467 | 2.792 | 105 | 1.362 | 411 | 0 | 0.000 |
| 기준선 대비 | +1,359 | -177.881(-83.3%) | -207.247(-79.1%) | +591 | -173.656(-84.8%) | -6,123(-16.4%) | -5,176(-88.2%) | 0 |
| `act2-dormancy1-r1` | 1,795 | 19.775 | 28.929 | 2 | 15.259 | 31,276 | 77 | 0.000 |
| `act2-dormancy1-r2` | 1,786 | 19.536 | 27.966 | 1 | 15.024 | 31,257 | 77 | 0.000 |
| `act2-dormancy1-r3` | 1,785 | 19.349 | 28.131 | 1 | 14.914 | 31,260 | 77 | 0.000 |
| 중앙값 | 1,786 | 19.536 | 28.131 | 1 | 15.024 | 31,260 | 77 | 0.000 |
| 변동 폭 | 10 | 0.426 | 0.963 | 1 | 0.345 | 19 | 0 | 0.000 |
| Relevancy 대비 | +146 | -16.138(-45.2%) | -26.541(-48.5%) | -871 | -16.200(-51.9%) | +55(+0.2%) | -618(-88.9%) | 0 |
| `act2-update-frequency1-r1` | 1,795 | 16.732 | 24.051 | 1 | 12.225 | 16,641 | 77 | 0.000 |
| `act2-update-frequency1-r2` | 1,794 | 19.927 | 27.043 | 1 | 14.818 | 16,613 | 77 | 0.000 |
| `act2-update-frequency1-r3` | 1,796 | 16.311 | 23.511 | 1 | 11.913 | 16,642 | 77 | 0.000 |
| 중앙값 | 1,795 | 16.732 | 24.051 | 1 | 12.225 | 16,641 | 77 | 0.000 |
| 변동 폭 | 2 | 3.616 | 3.532 | 0 | 2.905 | 29 | 0 | 0.000 |
| Dormancy 대비 | +9 | -2.804(-14.4%) | -4.080(-14.5%) | 0 | -2.799(-18.6%) | -14,619(-46.8%) | 0 | 0 |

- 변화는 중앙값끼리의 차이다. 백분율은 차이 ÷ 앞 구성의 중앙값이다.
- `act2-update-frequency1-r2`만 `work_avg_ms`가 3ms 넘게 크다(19.927, 다른 두 실행은 16.732, 16.311). `frames`와 송신량은 다른 두 실행과 같다. 그래서 Net Update Frequency의 변동 폭 3.616이 Dormancy 대비 차이 2.804보다 크다. Insights에서는 처리 횟수가 다른 두 실행과 같고 모든 타이머가 14\~24% 길었다([관찰 자료](candidates.md) 5절).

## 4. 확인하지 않은 것

- Relevancy만 적용한 구성에서 연결 하나가 건축물 500개 가운데 412.8개만 프레임마다 처리한 까닭. 건축물은 무리 중심에서 80m 안이고 플레이어의 경로는 무리 중심에서 59m 안이라(경로 꼭짓점 77m, 77m과 중심 35m, 35m의 거리), 2차원 거리로는 모두 139m 안이다. 관련성 판정의 기준 위치나 짓고 허무는 건축물의 채널을 확인하지 않았다.
- `GameNetDriver` Excl이 Dormancy로 6.784ms 준 것 가운데 Consider List가 짧아진 몫. Excl을 나누는 타이머가 없다.
- `act2-update-frequency1-r2`에서 서버 코어가 느려진 원인.
