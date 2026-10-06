# 클라이언트에서 NPC 위치를 보간하기: 측정 기록

[본문](README.md)이 쓰는 수치의 근거다. 본문은 유효 숫자 세 자리로 줄여 쓰고, 정밀한 값과 계산식, 실행 라벨, 엔진 소스 위치는 여기에 둔다. 후보 기법과 고른 값은 [관찰 자료](candidates.md)에 있다. 품질 지표의 정의는 [ADR-0020](../../Docs/Decisions/0020-npc-motion-quality-metrics.md)과 [measurement.md](../../Docs/Guides/measurement.md) "수치의 이름과 출처"에 있다.

## 1. 실행 조건

- 구성: [포스팅 10](../10-inventory-fastarray/measurements.md)의 최종 구성. 2막의 요소 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`, `-NodeUpdateFrequency 2 -InventoryOwnerOnly -InventoryFastArray`에 위치 기록 `-MotionLog`를 더했다. 클라이언트 8, 자원 노드 5,000과 검증용 1, 맵 전체의 NPC 300, 준비 30초, 측정 60초, 서버 논리 프로세서 2\~7, 연결당 송신 한도 350,000바이트/초, 트레이스 켬.
- 두 묶음을 세 번씩 연달아 쟀다. 2026-10-07 06:15\~06:31, 본체 화면, 실행 중 PC 조작 없음. 6회 모두 종료 코드 0이었다. 측정 직전 5초 동안 CPU를 쓰는 다른 프로세스가 없음을 확인했다(7절의 첫 묶음 때문에).

| 묶음 | 본문의 이름 | 더한 인자 | 중앙값 실행(`work_avg_ms`) |
| --- | --- | --- | --- |
| `act2-interp-base2` | 적용 전 | 없음 | `r1` |
| `act2-interp2` | 적용 후 | `-NpcInterpDelay 150` | `r1` |

- 빌드는 커밋 `6274fd5`의 소스다. 인자 `-NpcInterpDelay 150`(서버와 클라이언트 `-LabNpcInterpDelay=150`)이 서버에서 `ALabNpc::ServerFrame`을 보내게 하고(조건 `COND_None`, 끄면 `COND_Never`), 클라이언트에서 보간을 켠다. 인자가 없으면 엔진 기본 동작(`AActor::PostNetReceiveLocationAndRotation`)대로 받은 위치로 바로 옮긴다.
- `-MotionLog`는 서버와 클라이언트 8개가 NPC 위치를 기록한다(`ULabMotionLogSubsystem`). 서버가 기록에 쓴 시간은 프레임당 0.0655\~0.0673ms다(서버 로그 `lab_motion_log server record_ms_per_frame`). 두 묶음 모두 켰다.
- 품질 지표는 `Scripts/analyze-motion.ps1 -Label <라벨>-rN`이 계산했다. 표본은 측정 구간 60초 안의 (클라이언트, NPC, 클라이언트 프레임)이고, 실행마다 약 77만 개다(4절).
- Timing 값은 `Scripts/export-insights.ps1`로 내보냈다. 프레임 수는 `WorldTick` Count가 CSV `frames`와 같았다(1,788, 1,788).

## 2. 본문의 수치와 출처

| 본문의 수치 | 정밀한 값 | 출처 |
| --- | --- | --- |
| NPC의 Net Update Frequency를 10으로 낮췄다 | `FLabServerConfig::NpcUpdateFrequency` 기본값 10(`Source/DSOptLab/LabScenarioConfig.h`) | 프로젝트 소스, [포스팅 4](../04-update-frequency/measurements.md) |
| 클라이언트는 받은 위치로 NPC를 바로 옮긴다 | `AActor::OnRep_ReplicatedMovement`가 시뮬레이티드 프록시에서 `PostNetReceiveLocationAndRotation`을 부르고, 여기서 `SetActorLocationAndRotation`으로 옮긴다(`Engine/Source/Runtime/Engine/Private/ActorReplication.cpp:182-275, 277-285`) | 엔진 소스 |
| 표시 속도 오차 평균 440 → 15.0cm/s, -96.6% | 중앙값 439.9 → 15.0(`act2-interp-base2`, `act2-interp2`). 15.0 ÷ 439.9 − 1 = -96.59% | 4절 |
| 표시 지연 74 → 155ms, +81ms | 중앙값 74 → 155. 155 − 74 = 81 | 4절 |
| 연결당 송신 대역폭 11,800 → 12,700바이트/초, +7.29% | CSV `out_bytes_per_sec_per_conn` 중앙값 11,820 → 12,682. 12,682 ÷ 11,820 − 1 = 7.293% | 3절 |
| 서버 프레임 시간 평균 8.88 → 8.89ms, 구별되지 않음 | Timing Insights 8.880 → 8.893(중앙값 실행 `r1`끼리, ADR-0010). CSV `work_avg_ms` 중앙값 8.601 → 8.616(+0.015)이 두 묶음의 변동 폭 0.138, 0.215 가운데 큰 쪽보다 작다 | 3절, 5절 |
| 가장 큰 원뿔이 10m를 왕복하는 시연용 NPC, 4배 느리게 | `ALabShowcaseNpc`의 왕복 구간은 0번 자리에서 (+10m, +2.5m) → (+10m, +12.5m)(`LabGameMode.cpp`의 `SetPatrol` 호출). GIF는 `setpts=4*PTS`(6절) | 프로젝트 소스, 6절 |
| NPC는 300cm/s로 걷는다 | `ALabNpc::MoveSpeed` 300(`Source/DSOptLab/LabNpc.h`) | 프로젝트 소스 |
| 약 134ms마다 보낸다(갱신 간격) | 수신 간격 평균 133.6ms(`act2-interp-base2` 중앙값). 서버 프레임 4개다(포스팅 4 관찰 자료 2절) | 4절 |
| 약 40cm를 건너뛴다 | 300 × 0.1336 = 40.1cm | 계산 |
| 위치 그래프(문제, 결과) | `Scripts/make-interp-trace.ps1`. `act2-interp-base2-r1`과 `act2-interp2-r1`의 0번 클라이언트에서 NetGUID 20인 NPC, 측정 구간 시작 10.0초 뒤부터 1.2초 | 6절 |
| 세 프레임 동안 멈췄다가 한 프레임에 크게 움직인다 | 표시 위치가 바뀐 간격 평균 133.6ms, 클라이언트 프레임 간격 평균 35.3ms. 133.6 ÷ 35.3 = 3.8프레임마다 한 번 움직인다 | 4절, 계산 |
| 표시 위치 오차 평균 22.3cm | 중앙값 22.25(`act2-interp-base2`) | 4절 |
| 서버와 클라이언트 8개가 같은 시계로 기록, 측정 구간 60초 | `FPlatformTime::Cycles64()`는 `QueryPerformanceCounter` 값이다(`Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformTime.h:39-44`). NPC는 NetGUID로 짝짓는다(`FNetGUIDCache::GetNetGUID`, `Classes/Engine/PackageMapClient.h:226`). 측정 구간은 서버의 `Lab_MeasureStart`, `Lab_MeasureEnd`와 같은 순간이다 | 엔진 소스, ADR-0020 |
| 보간 지연 150ms | 사용자가 정한 값(2026-10-07, 관찰 자료 5절). 갱신 간격 133.6ms에 전달 여유를 더했다 | 관찰 자료 |
| 원리의 도식 | `Scripts/make-interp-timeline.ps1`. 갱신 간격 133.3ms, 보간 지연 150ms, 전달 시간 0의 예시 | 6절 |
| 클라이언트가 패킷을 프레임마다 한 번 처리해 받은 시각의 간격이 흔들린다 | 받은 시각이 클라이언트 프레임(평균 35.3ms) 단위로 끊긴다. 보간 없음의 수신 간격 P99는 166.8ms로 갱신 간격 133.6ms보다 한 프레임쯤 길다 | 4절 |
| 번호는 8비트 | `uint8 ServerFrame`, `GFrameCounter & 0xFF`. 30Hz에서 256 ÷ 30 = 8.5초마다 한 바퀴 | 프로젝트 소스 |
| 가장 빨리 도착한 갱신들로 서버의 시계와 프레임 길이를 추정 | 0.5초 칸마다 (받은 시각 − 프레임 번호 × 33.3ms)가 가장 작은 갱신 하나를 최근 8초 동안 남기고, 점들에 직선을 맞춰 기울기(프레임 길이)를 구하고 아래쪽 경계로 절편을 정한다(`LabNpc.cpp`의 `FLabServerClock`) | 프로젝트 소스 |
| 예상: 표시 지연 약 157ms, 전달 시간 약 7ms | 보간 없음의 표시 지연 74ms에서 갱신 간격의 절반 66.8ms를 빼면 7.2ms. 150 + 7.2 = 157.2 | 계산 |
| 예상: 표시 위치 오차 약 47cm | 300 × 0.1572 = 47.2cm | 계산 |
| 예상: 갱신 한 번에 16비트, 송신 대역폭 약 7.2% 증가 | 바뀐 프로퍼티마다 핸들 8비트와 값(`uint8` 8비트)(`Engine/Source/Runtime/Engine/Private/RepLayout.cpp`의 `SerializeIntPacked` 핸들, [포스팅 10 측정 기록](../10-inventory-fastarray/measurements.md) 2절). 연결 하나가 1초에 받는 NPC 갱신은 205,338 ÷ 8 ÷ 60.023 = 427.6번(`act2-interp-base2-r1`의 수신 간격 n). 427.6 × 16 ÷ 8 = 855바이트/초, 855 ÷ 11,820 = 7.24% | 계산 |
| 적용의 코드 | `Source/DSOptLab/LabNpc.h`, `LabNpc.cpp`(커밋 `f44bbfd`, 프레임 길이 추정은 `6274fd5`). 본문은 줄여 옮겼다 | 프로젝트 소스 |
| NPC 갱신 한 번은 101비트에서 117비트, 늘어난 몫이 프레임 번호 | Networking Insights의 `LabNpc` Incl ÷ Count: 2,706,415 ÷ 26,736 = 101.23비트(`act2-interp-base2-r1`), 3,133,781 ÷ 26,738 = 117.20비트(`act2-interp2-r1`). 차이 15.97비트. 적용 후의 `ServerFrame`은 26,738번, 427,808비트로 한 번에 16비트(I.Max 16)다 | 5.1절 |
| 결과 표의 예상 약 12,700 | 11,820 + 855 = 12,675 | 계산 |
| 결과 표의 실제 | 15.0, 155, 46.38, 12,682(중앙값) | 3절, 4절 |
| 차트의 값 | 439.9, 15.0 | 4절 |
| 표시 속도 오차의 P99는 339cm/s | 중앙값 338.5(`act2-interp2`, 적용 전 957.6) | 4절 |
| 방향을 바꾸는 순간의 오차로 추정 | 확인하지 않았다(8절) | |
| NPC를 리플리케이트하는 시간 0.857 → 0.886ms | `GameNetDriver` 아래 `LabNpc`의 프레임당 Incl(`r1`끼리) | 5절 |
| 표시 위치 오차는 두 배 | 46.38 ÷ 22.25 = 2.08 | 계산 |
| 처음 구현에서 표시 지연이 139\~144ms | `act2-interp1-r1`\~`r3`의 표시 지연 139, 143, 144ms. 서버 프레임은 60초에 1,771, 1,776, 1,779였다 | 7절 |
| NPC 갱신 한 번은 111비트, 그중 92비트가 이동 데이터 | `update-frequency3-r1`의 `LabNpc` I.Avg 111비트, `ReplicatedMovement` 227,424 ÷ 2,472 = 92.0비트(1막의 최종 구성) | [포스팅 4 관찰 자료](../04-update-frequency/candidates.md) 2절 |

## 3. 서버가 남긴 CSV

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-interp-base2-r1` | 1788 | 8.601 | 12.616 | 1 | 4.162 | 11820 | 77 | 0.000 |
| `act2-interp-base2-r2` | 1788 | 8.515 | 12.304 | 1 | 4.108 | 11826 | 77 | 0.000 |
| `act2-interp-base2-r3` | 1797 | 8.653 | 13.000 | 1 | 4.145 | 11815 | 77 | 0.000 |
| **중앙값** | 1788 | 8.601 | 12.616 | 1 | 4.145 | 11820 | 77 | 0.000 |
| **변동 폭** | 9 | 0.138 | 0.696 | 0 | 0.054 | 11 | 0 | 0.000 |
| `act2-interp2-r1` | 1788 | 8.616 | 12.632 | 1 | 4.188 | 12682 | 77 | 0.000 |
| `act2-interp2-r2` | 1797 | 8.525 | 13.092 | 1 | 4.094 | 12675 | 77 | 0.000 |
| `act2-interp2-r3` | 1796 | 8.740 | 13.181 | 0 | 4.212 | 12689 | 77 | 0.000 |
| **중앙값** | 1796 | 8.616 | 13.092 | 1 | 4.188 | 12682 | 77 | 0.000 |
| **변동 폭** | 9 | 0.215 | 0.549 | 1 | 0.118 | 14 | 0 | 0.000 |

- `out_bytes_per_sec_per_conn`은 862(7.29%) 늘었다. 변동 폭(11, 14)보다 훨씬 크다.
- `work_avg_ms`(+0.015), `work_p99_ms`(+0.476), `netflush_avg_ms`(+0.043)는 변동 폭 가운데 큰 쪽보다 작아 구별하지 못했다(measurement.md "차이의 판단").
- `act2-interp-base2`는 포스팅 10의 `act2-fastarr1`(8.586, 11,807)에 `-MotionLog`를 더한 구성이다. 다른 날의 묶음이라 비교하지 않았다.

## 4. 품질 지표

`analyze-motion.ps1`의 값이다(`Saved/LabMotion/<라벨>/summary.txt`, `Saved/LabMetrics/motion.csv`). 클라이언트 8개, NPC 66개, 이 NPC들의 서버 기록과 짝이 맞지 않은 클라이언트 기록은 없었다.

| 라벨 | 표본 | 표시 위치 오차 평균 / P99(cm) | 표시 속도 오차 평균 / P99(cm/s) | 표시 위치가 바뀐 간격 평균 / P99(ms) | 수신 간격 평균 / P99(ms) | 표시 지연(ms) | 클라이언트 프레임 간격 평균(ms) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `act2-interp-base2-r1` | 772,371 | 22.28 / 42.27 | 439.6 / 959.5 | 133.6 / 167.0 | 133.6 / 166.8 | 74 | 35.41 |
| `act2-interp-base2-r2` | 776,160 | 22.25 / 42.31 | 440.3 / 957.6 | 133.7 / 167.3 | 133.6 / 166.8 | 74 | 35.21 |
| `act2-interp-base2-r3` | 774,185 | 22.16 / 42.09 | 439.9 / 952.5 | 133.5 / 166.6 | 133.4 / 166.6 | 74 | 35.31 |
| **중앙값** | | 22.25 / 42.27 | 439.9 / 957.6 | 133.6 / 167.0 | 133.6 / 166.8 | 74 | 35.31 |
| **변동 폭** | | 0.12 / 0.22 | 0.7 / 7.0 | 0.2 / 0.7 | 0.2 / 0.2 | 0 | 0.20 |
| `act2-interp2-r1` | 771,878 | 45.78 / 49.37 | 15.0 / 338.5 | 35.4 / 45.4 | 133.6 / 166.8 | 154 | 35.41 |
| `act2-interp2-r2` | 773,073 | 46.38 / 52.43 | 15.0 / 335.7 | 35.4 / 45.5 | 133.4 / 166.6 | 155 | 35.36 |
| `act2-interp2-r3` | 776,397 | 46.53 / 52.53 | 14.9 / 343.8 | 35.2 / 45.2 | 133.4 / 166.7 | 155 | 35.21 |
| **중앙값** | | 46.38 / 52.43 | 15.0 / 338.5 | 35.4 / 45.4 | 133.4 / 166.7 | 155 | 35.36 |
| **변동 폭** | | 0.75 / 3.16 | 0.1 / 8.1 | 0.2 / 0.3 | 0.2 / 0.2 | 1 | 0.20 |

- 보간 없음의 값은 ADR-0020의 계산 예상(표시 위치 오차 약 25cm, 표시 속도 오차 약 450cm/s)과 맞는다.
- 보간 후 표시 위치가 바뀐 간격은 클라이언트 프레임 간격과 같다. 프레임마다 위치가 바뀐다.
- 표시 지연은 평균 위치 오차가 가장 작아지는 τ다. 그 τ에서의 평균 오차는 보간 없음 약 10.0cm, 보간 1.17\~1.43cm다(`summary.txt`의 `display_lag_ms` 줄). 보간 없음의 10cm는 계단 모양 때문에 남는 오차다.
- 클라이언트는 확정 규모에서 30fps를 다 지키지 못했다(프레임 간격 평균 35.3ms, 약 28fps).

## 5. Timing Insights (중앙값 실행 `r1`)

| 항목(프레임당) | `act2-interp-base2-r1` | `act2-interp2-r1` |
| --- | ---: | ---: |
| 서버 프레임 시간 평균 / P99(ADR-0010) | 8.880 / 12.875ms | 8.893 / 12.987ms |
| `GameNetDriver` Incl / Excl | 3.907 / 2.389ms | 3.934 / 2.389ms |
| `LabNpc` Count / Incl | 114.9 / 0.857ms | 114.8 / 0.886ms |
| `LabCharacter` Incl | 0.480ms | 0.481ms |

- `Saved/InsightsExport/<라벨>/summary.txt`에서 옮겼다. 구간은 두 북마크 사이(60.023초, 60.000초)다.
- `LabNpc`의 차이(+0.029ms)는 실행 하나씩의 값이라 구별 여부를 판단하지 않았다.

### 5.1 Networking Insights (`Game Instance 0 [Server]`, `Connection 0`, `Outgoing`)

| 줄 | `act2-interp-base2-r1` Count / Incl / I.Avg | `act2-interp2-r1` Count / Incl / I.Avg |
| --- | ---: | ---: |
| `Actor` | 38,243 / 5,569,125 / 145 | 38,214 / 5,994,747 / 156 |
| `LabNpc` | 26,736 / 2,706,415 / 101 | 26,738 / 3,133,781 / 117 |
| `ServerFrame` | 없음 | 26,738 / 427,808 / 16 |
| `ReplicatedMovement` | 36,579 / 3,465,610 / 94 | 36,580 / 3,465,749 / 94 |
| `PropertyHandle` | 38,496 / 307,968 / 8 | 38,475 / 307,800 / 8 |
| `PacketHeaderAndInfo` | 1,899 / 174,708 / 92 | 1,901 / 174,892 / 92 |

- 패킷 범위는 측정 구간이 아니라 그래프의 정상 구간에서 약 1,900패킷을 손으로 골랐다(두 트레이스에서 같은 화면 위치를 클릭과 Shift-클릭). 그래서 이 표의 Count와 Incl은 서로 비교하지 않고, 갱신 한 번의 평균(Incl ÷ Count)만 쓴다. 창은 2912 폭 화면에서 최대화했다.
- `ServerFrame`의 16비트는 모두 그 아래의 `Shared`다(패킷 하나의 Packet Content에서 `ServerFrame` Size 16 bits, 아래 `Shared` Size 16 bits). `PropertyHandle`의 Count가 두 트레이스에서 거의 같아(38,496, 38,475), 프로퍼티 핸들은 따로 더 가지 않았다. 원리의 예상(핸들 8비트 + 값 8비트)과 합계는 같고, 엔진이 핸들을 `Shared` 안에 기록한 것으로 본다.
- `ReplicatedMovement`에는 플레이어 캐릭터의 이동도 들어 있다. 패킷 하나에서 읽은 NPC 하나의 `ReplicatedMovement` 아래 `Shared`는 83비트였다.

## 6. 시각 자료

- 요약의 GIF([images/npc-compare.gif](images/npc-compare.gif))는 시각 자료 전용 실행 `visual20`(적용 전 구성 + `-ShowcaseNpc`)과 `visual21`(적용 후 구성 + `-ShowcaseNpc`)에서 찍었다. 두 실행은 `-NoTrace`, 한 번씩이고 수치는 쓰지 않는다. 시작 신호 48.5초 뒤에 `Scripts/capture-video.ps1 -Region "0,0,960,540" -RaiseSlots "0" -Fps 60 -Seconds 8 -NoMouse -AllowMeasuring`로 0번 창을 8초 찍었다(`Saved/Screenshots/Lab/visual20-npc.mp4`, `visual21-npc.mp4`).
- GIF는 ffmpeg로 두 영상에서 영역 520,245 440×80을 잘라 위아래로 붙였다. 적용 전은 60\~299번 프레임, 적용 후는 100\~339번 프레임이다. 적용 후를 40프레임 늦게 자른 것은 시연용 NPC가 같은 자리에 오게 하려고서다(두 영상을 30프레임마다 잘라 큰 원뿔의 x를 비교했다). 4배 느리게(`setpts=4*PTS`), 15fps, 64색(`palettegen=max_colors=64:stats_mode=full`, `paletteuse=dither=none`), 808KB다.
- 위치 그래프 [images/npc-trace-before.svg](images/npc-trace-before.svg), [images/npc-trace-after.svg](images/npc-trace-after.svg)는 `Scripts/make-interp-trace.ps1`이 모션 기록으로 그린 실제 값이다. 측정 구간 시작 10초 뒤부터 0.5초씩 옮기며 1.2초 동안 곧게 걸은 NPC 가운데 NetGUID가 가장 작은 것을 골랐고, 두 실행 모두 NetGUID 20, 10.0초였다. 세로축은 구간 시작의 서버 위치에서 이동 방향으로 잰 거리다. 두 실행의 NPC는 같은 NetGUID지만 서로 다른 실행이다.
- 원리의 도식 [images/interp-timeline.svg](images/interp-timeline.svg)은 `Scripts/make-interp-timeline.ps1`이 그린 예시다(SMIL 애니메이션, 서버 시각 800ms를 5초에 재생). Edge 헤드리스로 두 시점(1.0초, 3.3초)을 찍어 움직임을 확인했다.

## 7. 첫 묶음과 시계 수정

- 처음 잰 `act2-interp-base1`, `act2-interp1`(2026-10-07 01:32\~01:50)은 쓰지 않는다. 측정 시작 직후(01:33:12)부터 다른 세션이 남긴 `grep.exe`가 코어 하나를 계속 썼다. 기준 묶음의 `work_avg_ms`가 13.431\~14.156으로 전날 같은 구성(`act2-fastarr-base1`, 8.852)보다 컸다. Consider List 길이(418.6, 420.0)와 타이머의 호출 횟수는 같았고, 모든 타이머가 약 1.5배 느렸다(`GameNetDriver` 프레임당 4.112 → 6.468ms, `act2-fastarr-base1-r3`과 `act2-interp-base1-r3`).
- 이 묶음에서 보간 시계의 결함을 찾았다. 처음 구현은 프레임 길이를 33.3ms로 두고 (받은 시각 − 서버 시각)의 최근 2초 최솟값을 시계 차로 썼다. 서버가 60초에 1,771\~1,779프레임만 돌자 서버 시각이 약 1.5% 늦게 갔고, 표시 지연이 139\~144ms, 표시 속도 오차 평균이 20.9\~32.1cm/s였다. 받은 시각에 직선을 맞춰 프레임 길이를 추정하게 고쳤다(커밋 `6274fd5`).
- 고친 뒤 서버를 28Hz로 돌린 작은 규모 확인(`tsmall-motion5-r1`, `run-scenario.ps1 -ServerTickRate 28`, 클라이언트 2, NPC 60)에서 표시 지연 151ms, 표시 속도 오차 평균 12.9cm/s였다. 같은 규모의 30Hz 실행(`tsmall-motion2-r1`, 고치기 전)은 165ms, 12.3cm/s였다.
- 작은 규모 확인의 다른 값: 보간 없음(`tsmall-motion1-r1`) 표시 위치 오차 21.19cm, 표시 속도 오차 448.7cm/s. 서버의 기록 비용은 처음에 NPC 350개에서 프레임당 0.0789ms(`tsmall-motion3-r1`)였고, 아직 보내지 않은 NPC의 NetGUID를 1초에 한 번만 찾게 해 0.0159ms가 됐다(`tsmall-motion4-r1`).

## 8. 확인하지 않은 것

- `ServerFrame`의 `Shared` 16비트 안에서 핸들과 값이 각각 몇 비트인지. 패킷 내용 화면은 더 나누지 않았다.
- 표시 속도 오차 P99(약 339cm/s)가 NPC가 방향을 바꾸는 순간에서 오는지.
- 0번이 아닌 클라이언트별 값의 차이(`summary.txt`에 클라이언트별 평균이 있다).
- 보간이 클라이언트 CPU에 주는 비용.
- 지연의 흔들림과 패킷 손실에서 버퍼가 비는 빈도. 포스팅 13에서 잰다.
