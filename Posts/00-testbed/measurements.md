# 테스트베드와 측정 방법: 측정 기록

[본문](README.md)이 쓰는 값의 근거다. 규모를 정한 경위, 보정 실행의 수치, 엔진 소스에서 확인한 기본값, 측정 절차의 세부를 여기에 둔다. 측정 규약의 원본은 [measurement.md](../../Docs/Guides/measurement.md)이고, 보정 실행의 경위는 [Worklog/00-testbed.md](../../Docs/Worklog/00-testbed.md)에 있다.

## 1. 확정한 값

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 | 8 | 8 | 실행 중 UE 프로세스 9개의 메모리 합계 27.5GB, 사용 가능 메모리 최저 14.9GB(`calib-a-r1`). RAM 64GB에서 줄일 필요가 없었다 |
| 자원 노드 | 5,000 | 5,000(검증용 포함 5,001) | 포화가 없는 실행에서 모든 프레임이 틱 예산을 넘어 늘리지 않았다 |
| AI NPC | 300 | 300 | 네 조건을 모두 만족해 바꾸지 않았다 |
| 맵 | 바닥 2km × 2km | 같음 | 배치 영역은 1.9km × 1.9km(`Source/DSOptLab/LabScenarioConfig.h`의 `WorldHalfExtent` 95,000cm) |
| 서버 틱 | 30Hz | 30Hz(틱 예산 1000 ÷ 30 = 33.3ms) | `NetServerMaxTickRate=30`(`Engine/Config/BaseEngine.ini:1867`) |
| 워밍업 구간 | 30초 | 30초 | 초기 전송이 측정 시작 22초 전에 끝났다(`calib-f-r1`) |
| 측정 구간 | 60초 | 60초 | Insights에서 두 북마크 사이가 60.018초(`calib-f-r1`) |
| 반복 | 3회 | 3회, 중앙값과 변동 폭 | [ADR-0008](../../Docs/Decisions/0008-reproducible-runs.md) |
| 연결당 송신 한도 | 엔진 기본값 100,000바이트/초 | 350,000바이트/초 | 3절 |
| 서버 코어 | 논리 프로세서 0\~7 | 논리 프로세서 2\~7 | 4절. 클라이언트는 8\~31, 0번과 1번은 쓰지 않는다 |

본문 요약의 "한 프레임에 약 198ms, 틱 예산의 약 6배"는 [Always Relevant 기준선](../01-baseline/README.md)의 값이다(세 실행의 중앙값 198.43ms). 이 글의 보정 실행 한 번(`calib-f-r1`)에서는 168.1ms였다.

확정 규모의 실행 명령이다.

```powershell
powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Runs 3 -Clients 8 -Nodes 5000 -Npcs 300 -Warmup 30 -Measure 60
```

## 2. 기준선 조건

규모는 "기준선이 네 가지 조건을 만족하는가"로 정했다. 출발값으로 실행해 조건을 순서대로 판단했고, 송신 한도와 서버 코어 두 가지를 고친 뒤 출발값 규모 그대로 확정했다.

| 기준선 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | 예 | `calib-f-r1` 서버 로그: 측정 시작 22초 전에 `open_actor_channels_per_conn`이 5,314에 도달하고 더 늘지 않음 |
| 송신 한도에 포화되지 않음 | 예(한도를 올린 뒤) | 엔진 기본 한도에서 `saturated_ratio` 1.000(`calib-a-r1`), 350,000에서 0.000(`calib-f-r1`) |
| 지속적인 예산 초과 | 예 | `calib-f-r1`: `over_budget_frames` 356 = `frames` 356. `work_avg_ms` 168.309는 틱 예산 33.3ms의 5.0배 |
| 가장 큰 비용이 네트워크 | 예 | `calib-f-r1`의 Insights 측정 구간: `WorldTick` 59.84초 중 `GameNetDriver` 57.45초(96.01%) |

## 3. 송신 한도를 올린 계산

레거시 리플리케이션은 연결이 송신 한도에 걸리면 그 프레임의 나머지 액터를 다음으로 미룬다. 엔진 기본 한도(연결당 100,000바이트/초)에서 기준선은 완전히 포화됐다(`calib-a-r1`의 `saturated_ratio` 1.000). 포화 상태에서는 처리 비용이 실제보다 작게 나오고, 최적화 뒤에 대역폭이 오히려 느는 식으로 결과를 설명하기 어려워진다. 네트워크가 루프백이라 한도를 올려도 부작용이 없다([ADR-0007](../../Docs/Decisions/0007-raise-send-limit-once.md)).

- 한도를 10,000,000으로 올린 실행(`calib-b-r1`)의 `out_bytes_per_sec_per_conn`은 9,591, `frames`는 100이다.
- 기준선은 30Hz를 지키지 못하므로 30Hz로 환산한다. 환산 송신량 = 9,591 × 30 ÷ (100 ÷ 60) = 172,638바이트/초.
- 그 두 배 345,276을 올림해 350,000으로 고정했다. 최적화로 30Hz가 돌아와도 포화되지 않게 하되, 한도가 없는 것과 같은 값은 피했다.
- 바꾼 키는 `Config/DefaultEngine.ini`의 `[/Script/Engine.Player] ConfiguredInternetSpeed`, `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`, `MaxInternetClientRate` 세 개다. 클라이언트가 첫 번째 값을 서버에 보내고 서버가 나머지 두 값으로 자르기 때문에 셋을 함께 올려야 한다(`NetConnection.cpp:588`, `World.cpp:7465-7473`).

## 4. 서버 코어를 고른 경위

처음에는 서버를 논리 프로세서 0\~7에 고정했다. 그런데 같은 구성의 실행이 세 배까지 달라졌다(`calib-b-r1`의 `frames` 100, `calib-e-r1`의 293). 원인은 논리 프로세서 1번이었다. 시나리오가 도는 동안 1번에서 DPC가 초당 약 15,000개 처리되어 DPC 시간이 32\~65%였고, 서버 게임 스레드가 1번에 올라가 있는 동안 CPU를 그만큼 빼앗겼다([ADR-0009](../../Docs/Decisions/0009-server-cores-without-dpc-load.md)).

| 서버를 고정한 코어 | 라벨 | `frames` | `work_avg_ms` |
| --- | --- | --- | --- |
| 1번만 | `diag-e-r1` | 153 | 392.972 |
| 2\~7번 | `diag-d-r1` | 340 | 176.630 |
| 2\~7번 | `calib-f-r1` | 356 | 168.309 |

어느 코어에 DPC가 몰리는지는 이 PC의 현재 상태에서 확인한 것이라 재부팅이나 드라이버 변경 뒤에 달라질 수 있다. 측정 전에 `\Processor Information(0,N)\% DPC Time`으로 논리 프로세서 2\~7의 DPC 시간이 0\~2%인지 확인한다.

## 5. 엔진에서 확인한 기본값(5.8.3 소스)

| 항목 | 값 | 위치 |
| --- | --- | --- |
| 서버 틱 | `NetServerMaxTickRate=30` | `Engine/Config/BaseEngine.ini:1867`. 적용은 `UGameEngine::GetMaxTickRate`(`GameEngine.cpp:1719-1765`). 실행 중에도 30초에 898프레임(`smoke2-r1`) |
| Net Cull Distance | `NetCullDistanceSquared` 225,000,000(150m) | `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| Net Update Frequency | `NetUpdateFrequency` 100, `MinNetUpdateFrequency` 2 | `Actor.cpp:295-296` |
| 연결당 송신 한도 | 100,000바이트/초 | `BaseEngine.ini:1839-1840, 1860-1861` |
| 리플리케이션 시스템 | 레거시. `net.Iris.UseIrisReplication`의 기본값이 0 | `IrisConfig.cpp:15-16`. 서버 로그 `using replication model Generic`(`smoke2-r1`) |
| Adaptive Net Update Frequency | 꺼짐. `net.UseAdaptiveNetUpdateFrequency` 기본값 0 | `NetDriver.cpp:523-526` |

- Relevancy 판정의 기준 위치는 서버가 계산한 3인칭 카메라 위치다. 클라이언트가 보내는 카메라 위치를 서버가 쓰지 않도록 양쪽에서 `bUseClientSideCameraUpdates`를 껐다. 내려다보기 화면은 클라이언트에서만 보이는 카메라이고 Relevancy 판정에 영향을 주지 않는다(`PlayerController.cpp:1836-1870`, `PlayerCameraManager.cpp:812-818`).
- 기준선에서는 자원 노드와 NPC에 `bAlwaysRelevant = true`를 주어 엔진의 거리 기반 Relevancy 판정을 일부러 끈다([ADR-0003](../../Docs/Decisions/0003-lawless-baseline.md)).

## 6. 측정 절차의 세부

| 항목 | 값 | 이유 |
| --- | --- | --- |
| 워밍업 구간 | 시작 신호 뒤 30초를 버림 | 접속 직후에는 모든 액터의 초기 스폰 데이터가 한꺼번에 나가 정상 상태와 다르다 |
| 측정 구간 | 60초 | 서버가 30Hz를 지키면 1,800프레임이다. 지키지 못하면 더 적으므로 `frames`를 함께 적는다 |
| 반복 | 구성마다 3회, 중앙값 | 한 번의 이상치를 걸러 낸다. 세 값과 변동 폭(최댓값 − 최솟값)을 모두 적는다 |
| 차이의 판단 | 중앙값의 변화가 두 구성의 변동 폭 중 큰 쪽보다 클 때만 차이가 있다고 적음 | 그보다 작으면 "구별되지 않았다"고 적는다 |

- **공통 시작 신호.** 모든 클라이언트가 접속해 준비되면 서버가 시작 신호를 낸다. 클라이언트의 이동과 채집, NPC의 배회, 자동 스크린샷의 시계가 모두 이 신호에서 출발한다. 클라이언트는 실행 인자로 자리 번호를 받아서 접속 순서와 상관없이 같은 자리에서 시작한다.
- **클라이언트.** 640×360 창, 30fps 제한으로 실제로 렌더링한다. 0번은 제자리에서 검증용 노드를 채집한다(2초 간격 3회에 고갈, 20초 뒤 재생성, 약 26초 주기). 1번은 내려다보기 화면으로 걷고, 2\~7번은 3인칭 화면으로 걷는다.
- **실패한 실행.** 다음 중 하나라도 해당하면 수치를 쓰지 않는다. 시작 신호 뒤에 연결 수가 바뀜(서버가 CSV를 쓰지 않고 종료 코드 1로 끝낸다). 서버가 끝나기 전에 클라이언트가 죽음. 서버가 제한 시간 안에 끝나지 않음. CSV 행이 정확히 하나 늘지 않음. 측정 시작 뒤에 프로세스의 코어 고정이 풀려 다시 설정됨. 트레이스 파일이 없거나 비어 있음.

## 7. 지표의 정의와 CSV 대조

| 지표 | 정의 | 출처 |
| --- | --- | --- |
| 서버 프레임 시간(평균, P99) | Timing Insights의 프레임 시간에서 틱 속도 제한 대기를 뺀 시간([ADR-0010](../../Docs/Decisions/0010-frame-time-without-tick-wait.md)) | Insights |
| 리플리케이션 시간(프레임당) | Timing Insights 타이머 `GameNetDriver`의 프레임당 Incl | Insights |
| 연결당 송신 대역폭 | 측정 구간의 연결당 초당 송신 바이트 | Network Insights, 서버 CSV와 대조 |
| 연결당 열린 액터 채널 수 | 연결 하나에 열려 있는 액터 채널 수 | 서버 CSV |
| 클라이언트에 존재하는 액터 수 | 클라이언트 화면 위 글자의 노드 수와 NPC 수 | 스크린샷 |

서버는 측정 구간의 시작과 끝에 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 트레이스에 남기고, 같은 구간의 수치를 CSV 한 줄로 쓴다. CSV의 두 시간 값은 한 프레임 안에서 다음 구간이다.

```mermaid
flowchart LR
    A[틱 속도 제한 대기] --> B[월드 틱 시작]
    B --> C[수신 처리]
    C --> D[액터 틱]
    D --> E[액터 틱 종료]
    E --> F[TickFlush: 리플리케이션과 송신]
    F --> G[프레임 끝]
```

- `work`: 월드 틱 시작부터 프레임 끝까지의 경과 시간이다. 틱 속도 제한의 대기는 들어가지 않는다.
- `netflush`: 액터 틱 종료부터 프레임 끝까지의 경과 시간이다. 리플리케이션 전용 시간이 아니다.
- 둘 다 CPU 실행 시간이 아니라 경과 시간이라 스레드 대기와 OS 스케줄링 지연이 들어간다(순서의 근거: `LaunchEngineLoop.cpp:5682-6127`, `LevelTick.cpp:1522-2061`).

CSV 값과 Insights 값은 정의가 달라서 같은 이름으로 부르지 않는다([ADR-0004](../../Docs/Decisions/0004-insights-and-csv-metrics.md)). 확정 규모 실행(`calib-f-r1`)에서 둘을 대조했다.

| Insights(측정 구간) | 값 | CSV | 값 | 차이 |
| --- | --- | --- | --- | --- |
| 서버 프레임 시간: 60.018초 ÷ `Frame` 357 | 168.1ms | `work_avg_ms` | 168.309 | 0.1% |
| 리플리케이션 시간: `GameNetDriver` 57.45초 ÷ 356 | 161.4ms | `netflush_avg_ms` | 161.744 | 0.2% |
| 연결당 송신량: (`Actor` 15,713,136비트 + `PacketHeaderAndInfo` 176,608비트) ÷ 8 ÷ 59.870초 | 33,176바이트/초 | `out_bytes_per_sec_per_conn` | 34,164 | 3.0% |

| Timing Insights: 측정 구간을 선택한 화면 | Network Insights: 연결 0의 송신 패킷 |
| --- | --- |
| ![Timing Insights](images/timing.png) | ![Network Insights](images/network.png) |

Insights에서 값을 읽은 과정은 [단계별 기록](../../Docs/Guides/insights-walkthrough-calib-f.md)에 있다.

## 8. 시각 자료의 사정

- 3인칭 화면과 내려다보기 화면은 확정 규모 실행(`calib-g-r1`)의 자동 스크린샷이다. 화면 왼쪽 위 상자의 첫 줄은 실행 라벨, 클라이언트 자리 번호와 역할, 시작 신호 후 경과 시간이고, 둘째 줄은 그 클라이언트에 존재하는 노드와 NPC의 수와 플레이어 위치다. 내려다보기 화면은 플레이어를 중심으로 350m 위에서 본 것이고, 검은 점은 고갈된 노드다.
- 8개 창의 화면과 영상은 수치를 쓰지 않는 시각 자료 전용 실행(`visual11-r1`, 태그 `post-01-baseline`의 빌드)에서 찍었다. 창 화면은 시작 신호 41초 뒤이고, 영상은 1번과 2번 클라이언트가 걷는 10초다(t=50\~59s).

## 9. 한계

- 에디터 빌드 실행 파일을 쿠킹 없이 사용한다. 절대 수치는 Shipping 빌드와 다르다([ADR-0002](../../Docs/Decisions/0002-editor-build-without-packaging.md)).
- 서버와 클라이언트가 같은 PC([pc-specs.md](../../Docs/Reference/pc-specs.md))에서 돈다. 서로 다른 코어에 고정하지만 캐시와 메모리 대역폭 경합은 남는다. 네트워크는 루프백이다.
- 서버 코어는 이 PC에서 DPC 부하가 몰리는 코어를 피해 고른 것이다. 다른 PC에서는 같은 번호가 맞지 않을 수 있다.
- 수치는 같은 조건의 전후 비교로만 해석해야 한다. 서버 코드만의 CPU 개선률이 아니다.
- 기준선은 인위적인 출발점이다. 엔진의 기본 Relevancy 판정을 일부러 끄고, 연결당 송신 한도를 100,000에서 350,000바이트/초로 올렸다.
- 기준선은 30Hz를 지키지 못한다(60초에 356프레임). 초당 송신량은 서버가 빨라지면 늘어날 수 있으므로 `frames`와 함께 읽어야 한다.
- AI NPC는 움직이는 리플리케이트 액터를 대신한다. 길 찾기나 비헤이비어 트리 같은 AI 비용은 측정하지 않는다.
- 채집 RPC는 자동 실험용이다. 서버가 대상 탐색과 거리 검사, 호출 간격 제한을 하지만 그 밖의 악의적 호출은 막지 않는다.
