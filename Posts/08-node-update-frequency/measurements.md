# 자원 노드의 Net Update Frequency 낮추기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 실행별 CSV, Insights 값, 서버 로그는 [관찰 자료](candidates.md) 6\~12절에 있다.

## 1. 실행 조건

- 구성: [포스팅 6](../06-three-techniques-again/measurements.md)의 최종 구성. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초.
- 네 묶음을 세 번씩 쟀다. 2026-10-06 02:08\~02:39, 본체 화면, 실행 중 PC 조작 없음. 12회 모두 종료 코드 0이고 측정 구간에 선호도 재설정이 없었다.

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-nodeuf-base1` | 적용 전 | 없음 | `r3` |
| `act2-nodeuf1` | 적용 후 | `-NodeUpdateFrequency 2` | `r3` |
| `act2-nodeuf-split-base1` | 타이머를 더 켠 실행(적용 전) | `-StatNamedEvents` | `r3` |
| `act2-nodeuf-split1` | 타이머를 더 켠 실행(적용 후) | `-StatNamedEvents -NodeUpdateFrequency 2` | `r3` |

- 빌드는 커밋 `a397f55`의 소스다. 이 커밋이 인자 `-LabNodeUpdateFrequency=`와 서버 로그 `lab_consider_list`를 더했다.
- `lab_consider_list avg_per_frame`은 측정 구간의 프레임마다 엔진 지표 `NumConsideredActors`를 읽은 평균이다. 엔진은 Consider List를 만든 끝에 그 길이를 이 지표에 쓴다(`Engine/Source/Runtime/Engine/Private/NetDriver.cpp:5454`). 서버는 프레임 끝(`FCoreDelegates::OnEndFrame`)에 읽는다(`Source/DSOptLab/LabMetricsSubsystem.cpp`의 `SampleConsideredActors`).
- 타이머를 더 켠 실행은 이벤트를 더 기록한다. 그 값은 같은 인자의 두 묶음끼리만 비교하고 기본 묶음과 섞지 않는다([engine-notes.md](../../Docs/Reference/engine-notes.md) 차절).
- 프레임당 값은 Incl ÷ 프레임 수다. 프레임 수는 기본 묶음에서 `WorldTick` Count, 타이머를 더 켠 묶음에서 `GameNetDriver` Count이고, 모두 CSV `frames`와 같았다. 읽은 명령은 `Scripts/export-insights.ps1`이다.

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| 남은 리플리케이션 시간의 80%는 따지는 일 | 79.9%(`act2-split1-r3`) | [포스팅 7 측정 기록](../07-net-driver-breakdown/measurements.md) 2절 |
| Net Update Frequency 100에서 2로 | 엔진 기본값 100(`Engine/Source/Runtime/Engine/Private/Actor.cpp:295`의 `SetNetUpdateFrequency(100.0f)`), 이 글의 값 2(`-LabNodeUpdateFrequency=2`) | 엔진 소스, `Source/DSOptLab/LabResourceNode.cpp` |
| 따지는 액터가 약 9분의 1 | Consider List 3,886.9 → 412.9, 412.9 ÷ 3,886.9 = 0.106(약 1 ÷ 9.4) | 관찰 자료 8절(`act2-nodeuf-base1-r3`, `act2-nodeuf1-r3`) |
| 리플리케이션 시간 13.3ms → 3.93ms, -70.5%, "70% 줄었고" | `GameNetDriver` 프레임당 Incl 13.339 → 3.929, 3.929 ÷ 13.339 − 1 = -70.5% | 관찰 자료 9.1절(`r3`끼리) |
| 서버 프레임 시간 평균 18.3ms → 8.90ms, -51.5%, "절반이 됐다" | Timing Insights 평균 18.347 → 8.895, 8.895 ÷ 18.347 − 1 = -51.5% | 관찰 자료 9.1절(`r3`끼리) |
| 프레임마다 Consider List에 드는 액터 3,887개 → 413개, -89.4% | `lab_consider_list avg_per_frame` 3,886.9 → 412.9, 412.9 ÷ 3,886.9 − 1 = -89.4% | 관찰 자료 8절(서버 로그, `r3`끼리) |
| 연결당 송신 대역폭 16,600바이트/초, 그대로 | CSV `out_bytes_per_sec_per_conn` 중앙값 16,611 → 16,570(-0.2%) | 관찰 자료 7절 |
| 자원 노드 176개 | 화면 글자 `nodes=176`(`act2-nodeuf-base1-r3-topdown-03`, `act2-nodeuf1-r3-topdown-03`, t=60초) | 자동 스크린샷 |
| 활성 목록 5,272개, 그 가운데 자원 노드 4,887개 | 서버 로그 `lab_network_objects active=5272`, `LabResourceNode=4887/76/114`(`act2-nodeuf-base1-r3`) | 관찰 자료 8절 |
| Consider List 3,887개(문제의 표) | `lab_consider_list avg_per_frame=3886.9`(`act2-nodeuf-base1-r3`) | 관찰 자료 8절 |
| 활성 목록의 4분의 3 | 3,886.9 ÷ 5,272 = 73.7% | 계산 |
| 그 대부분은 자원 노드라고 계산된다 | 4,887 ÷ 1.3 = 약 3,759개, 3,759 ÷ 3,886.9 = 96.7%. 1.3프레임은 아래 "평균 1.3프레임마다" 줄 | 계산 |
| 연결 8개 | 클라이언트 8 | 실행 조건 |
| ②가 리플리케이션 시간의 절반을 넘었다 | `Prioritize Actors Time` 7.593 ÷ `GameNetDriver` 13.590 = 55.9%(`act2-nodeuf-split-base1-r3`) | 관찰 자료 9.2절 |
| 다음 고려 시각은 "1 ÷ 빈도"초 뒤에 난수를 더한 시각, 난수는 0에서 한 프레임 시간 사이 | `NextUpdateTime = TimeSeconds + RandDelay + NextUpdateDelta`, `RandDelay = FRand() * ServerTickTime`, `NextUpdateDelta = 1 / NetUpdateFrequency`(`NetDriver.cpp:5420-5425`). Adaptive Net Update Frequency가 꺼져 있을 때다(`net.UseAdaptiveNetUpdateFrequency` 기본값 0, `NetDriver.cpp:523-526`) | 엔진 소스 |
| 그 시각이 되지 않은 액터는 건너뛴다 | `if (!bPendingNetUpdate && World->TimeSeconds <= NextUpdateTime) continue;`(`NetDriver.cpp:5319-5323`). 이 검사는 활성 목록을 도는 루프 안에 있다(`5315`) | 엔진 소스 |
| 30Hz, 프레임 간격 33.3ms | `NetServerMaxTickRate` 30(`Engine/Config/BaseEngine.ini:1867`), 1 ÷ 30 = 33.3ms | 엔진 소스 |
| 빈도 100: 10\~43ms, 평균 1.3프레임마다 | 1 ÷ 100 = 10ms에 난수 0\~33.3ms. 1막에서 실행으로 확인한 간격 | [engine-notes.md](../../Docs/Reference/engine-notes.md) "업데이트 빈도의 스케줄링"(`dormancy6`, `update-frequency3`) |
| 빈도 2: 500\~533ms, 16프레임마다 | 1 ÷ 2 = 500ms에 난수 0\~33.3ms. 500 ÷ 33.3 = 15프레임이 지나도 난수만큼 남으므로 16번째 프레임에 고려한다 | 계산 |
| Adaptive Net Update Frequency는 기본으로 꺼져 있고 하한의 기본값이 2 | `net.UseAdaptiveNetUpdateFrequency` 기본값 0(`NetDriver.cpp:523-526`), `SetMinNetUpdateFrequency(2.0f)`(`Actor.cpp:296`), 하한으로 쓰는 곳 `NetDriver.cpp:5393-5408` | 엔진 소스 |
| 최대 16프레임 뒤, 약 0.53초 | 16 × 33.3ms = 533ms | 계산 |
| 달리는 플레이어는 약 5.3m | `SprintSpeed` 1,000cm/s(`Source/DSOptLab/LabCharacterMovement.h:21`), 10m/s × 0.533초 = 5.33m | 프로젝트 소스, 계산 |
| 보이기 시작하는 거리 150m | `NetCullDistanceSquared` 225,000,000(`Actor.cpp:312`), √225,000,000 = 15,000cm | 엔진 소스 |
| 상태를 바꿀 때 다음 고려 시각을 지금으로 당긴다 | `AActor::ForceNetUpdate`(`Actor.cpp:3013-3031`)가 `UNetDriver::ForceNetUpdate`를 부르고, 레거시 경로는 `NextUpdateTime = TimeSeconds - 0.01f`(`NetDriver.cpp:4885-4887`). Dormant 상태면 `FlushNetDormancy()`도 부른다 | 엔진 소스 |
| 예상: 자원 노드 약 3,760개 → 약 305개 | 4,887 ÷ 1.3 = 3,759, 4,887 ÷ 16 = 305 | 계산 |
| 예상: ② 7.2ms → 약 0.85ms | 포스팅 7의 ② 7.208ms 가운데 자원 노드 몫 96%(94\~98%의 가운데) = 6.92, 6.92 × 1.3 ÷ 16 + (7.208 − 6.92) = 0.85 | [관찰 자료](candidates.md) 3절 |
| 결과 표: Consider List 3,887개 → 413개 | 위 "프레임마다 Consider List에 드는 액터" 줄과 같다 | 관찰 자료 8절 |
| 결과 표: 예상 약 433개, 다른 액터 약 128개 | 다른 액터 = 3,886.9 − 3,759 = 128, 305 + 128 = 433 | 계산 |
| ② 7.59ms → 0.845ms | `Prioritize Actors Time` 7.593 → 0.845(`act2-nodeuf-split-base1-r3` → `act2-nodeuf-split1-r3`) | 관찰 자료 9.2절 |
| ① 3.34ms → 0.959ms, "71% 줄었지만" | `Consider Actors Time` 3.338 → 0.959, 0.959 ÷ 3.338 − 1 = -71.3% | 관찰 자료 9.2절 |
| 차트의 값 | ② 7.593, 0.845, ① 3.338, 0.959, ③ `Process Prioritized Actors Time` 2.181, 1.868(`split` 묶음의 `r3`) | 관찰 자료 9.2절 |
| 줄어든 리플리케이션 시간의 96%가 ①과 ② | ①+② 10.931 → 1.804(-9.127), `GameNetDriver` 13.590 → 4.081(-9.509), 9.127 ÷ 9.509 = 96.0% | 관찰 자료 9.2절 |
| 건너뛰는 액터 하나 약 0.13µs, 고려하는 액터의 약 6분의 1, 남은 ①의 약 3분의 2 | 3,887.1c + 1,384.9s = 3.338, 414.8c + 4,857.2s = 0.959에서 c = 0.813µs, s = 0.128µs. s ÷ c = 0.157. 4,857.2 × 0.128µs = 0.622ms, 0.622 ÷ 0.959 = 64.8%. 건너뛴 수는 활성 목록 5,272에서 Consider List를 뺀 값이다 | 관찰 자료 9.2절, 계산 |
| ③이 리플리케이션 시간의 46% | 1.868 ÷ 4.081 = 45.8%(`act2-nodeuf-split1-r3`) | 관찰 자료 9.2절 |
| ③도 14% 줄었다, 처리한 액터 수는 같았다 | 2.181 → 1.868(-14.4%). `Replicate Actor Time` 프레임당 184.2번과 184.2번. 기본 트레이스의 `LabNpc` 0.981 → 0.853(-13.0%), `LabCharacter` 0.633 → 0.548(-13.4%) | 관찰 자료 9.1절, 9.2절 |
| 열린 액터 채널 수는 바뀌지 않았다 | CSV `open_actor_channels_per_conn` 77과 77 | 관찰 자료 7절 |
| 검증용 자원 노드는 30초에 있고 45초에 고갈되어 사라졌다 | `act2-nodeuf-base1-r3-tpp-01`(t=30초), `-02`(t=45초), `act2-nodeuf1-r3-tpp-01`, `-02` | 자동 스크린샷 |
| 인벤토리는 보낸 액터 데이터의 약 30% | 29.5%(`act2-update-frequency1-r1`) | [포스팅 6 관찰 자료](../06-three-techniques-again/candidates.md) 6절 |

## 3. 서버가 남긴 CSV

실행별 값은 [관찰 자료](candidates.md) 7절에 있다.

| 묶음 | `work_avg_ms` 중앙값 | 변동 폭 | `netflush_avg_ms` 중앙값 | `out_bytes_per_sec_per_conn` 중앙값 | `open_actor_channels_per_conn` |
| --- | ---: | ---: | ---: | ---: | ---: |
| `act2-nodeuf-base1` | 18.060 | 1.412 | 13.605 | 16,611 | 77 |
| `act2-nodeuf1` | 8.615 | 0.223 | 4.177 | 16,570 | 77 |
| `act2-nodeuf-split-base1` | 18.506 | 0.192 | 13.878 | 16,649 | 77 |
| `act2-nodeuf-split1` | 8.960 | 0.195 | 4.340 | 16,644 | 77 |

- `work_avg_ms`는 9.445(52.3%) 줄었다. 변동 폭(1.412, 0.223)보다 훨씬 크다.
- `act2-nodeuf-base1-r2`만 느렸다(`work_avg_ms` 18.830, `over_budget_frames` 9). 중앙값은 `r3`다.
- `act2-nodeuf-base1`은 포스팅 7의 `act2-split-base1`(17.358)과 구성이 같다. 다른 날의 묶음이라 비교하지 않았다.

## 4. 시각 자료

- 요약의 두 이미지는 `act2-nodeuf-base1-r3-topdown-03.png`와 `act2-nodeuf1-r3-topdown-03.png`(t=60초)를 `images/before-topdown.png`, `images/after-topdown.png`로 복사했다. 두 묶음의 중앙값 실행이고 같은 순번이다.
- 원리의 흐름도는 Mermaid로 그렸다. ①\~③은 [포스팅 7](../07-net-driver-breakdown/README.md)의 흐름도와 같은 번호이고 stat 타이머에 대응한다(① `Consider Actors Time`, ② `Prioritize Actors Time`, ③ `Process Prioritized Actors Time`).
- 결과의 차트 값은 `split` 묶음의 중앙값 실행의 프레임당 ms다.
- Insights 캡처는 아직 찍지 않았다.

## 5. 확인하지 않은 것

- ③과 클래스 타이머가 13\~14% 빨라진 원인. 처리 횟수는 같다.
- 다가간 플레이어에게 자원 노드가 처음 보이기까지의 지연. 계산으로는 최대 약 0.53초다.
- 건너뛰는 액터 하나의 비용(0.128µs)은 두 묶음의 연립식으로 푼 근사다. 두 비용이 구성과 상관없이 일정하다고 가정했다.
- Consider List에 든 액터를 클래스별로 세지 않았다. 자원 노드의 수(약 3,759개, 약 305개)는 빈도로 계산한 값이다.
- 값 2 밖의 빈도는 재지 않았다.
