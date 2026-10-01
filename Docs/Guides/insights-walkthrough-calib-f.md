# Insights로 "가장 큰 비용이 네트워크인가"를 확인한 과정 (`calib-f-r1`)

구현 계획 태스크 8.5를 Unreal Insights에서 실제로 수행한 기록이다(2026-10-01). 화면을 하나씩 캡처하고, 행동마다 왜 그렇게 했는지와 무엇을 읽었는지를 적었다. Insights 사용법을 나중에 다시 볼 때 쓰는 문서이기도 하다.

- 대상 트레이스: `Saved/Traces/calib-f-r1.utrace` (클라이언트 8, 노드 5,001, NPC 300, 준비 30초, 측정 60초, 서버 마스크 252, 송신 한도 350,000)
- 대조할 CSV 행(`calib-f-r1`): `frames` 356, `work_avg_ms` 168.309, `netflush_avg_ms` 161.744, `out_bytes_per_sec_per_conn` 34,164
- 조작은 에이전트가 했고, 판단(가장 큰 비용이 네트워크다, 리플리케이션 시간 타이머는 `GameNetDriver`다)은 사용자가 했다.
- 이 문서의 수치는 모두 아래 캡처 화면에서 읽은 값이거나 그 값으로 계산한 값이다. 계산에는 식을 적었다.

짧은 절차만 필요하면 [insights-reading.md](insights-reading.md)를 본다.

## 결론 먼저

| 질문 | 답 | 근거(아래 단계) |
| --- | --- | --- |
| 서버 프레임에서 가장 큰 비용이 네트워크인가 | 그렇다. `WorldTick`의 96.0%가 `GameNetDriver`다 | 5, 6단계 |
| 리플리케이션 시간으로 쓸 타이머 | `GameNetDriver` | 6단계 |
| 그 타이머와 CSV `netflush_avg_ms`의 관계 | 프레임당 161.4ms와 161.744ms로 0.2% 차이 | 5단계 |
| 프레임 시간과 CSV `work_avg_ms`의 관계 | 168.1ms와 168.309ms로 0.1% 차이 | 5단계 |
| Network Insights의 연결당 송신량이 CSV와 비슷한가 | 비슷하다. 33,176\~36,430바이트/초 사이에 CSV 34,164가 들어간다 | 10단계 |

## 1단계. 트레이스를 연다

```powershell
powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label calib-f-r1
```

**왜.** Insights를 그냥 실행하면 세션 목록에서 파일을 찾아야 한다. 스크립트는 `-OpenTraceFile=`로 라벨의 트레이스를 바로 연다. 시나리오가 실행 중이면(에디터 프로세스가 있으면) 열기를 거부한다. Insights가 CPU를 써서 측정에 섞이기 때문이다.

![트레이스를 연 직후의 화면](images/insights-calib-f/01-opened.png)

**읽은 것.** 창 제목이 `calib-f-r1`이다. 위쪽 탭은 `Session`, `Timing Insights`, `Networking Insights`다. 마지막에 쓴 탭이 먼저 열리므로 이번에는 Networking Insights가 보였다. `-trace=default,net`로 수집했기 때문에 Networking 탭이 있다.

## 2단계. Timing Insights 탭으로 간다

**왜.** "서버가 프레임 시간을 어디에 쓰는가"는 CPU 타이머로 본다. 패킷 내용은 그다음이다.

![Timing Insights의 세션 전체 화면](images/insights-calib-f/02-timing-whole-session.png)

**읽은 것.**

- 왼쪽 위 그래프가 프레임 시간이다. 가로축은 프레임 번호, 세로 눈금은 16.67ms(60fps), 33.33ms(30fps), 66.67ms, 100ms다. 프레임 780번 무렵부터 막대가 100ms 선을 넘어 붉게 찬다. 클라이언트가 접속해 리플리케이션이 시작된 뒤다.
- 가운데가 스레드별 타임라인이다. `GameThread` 줄이 서버의 게임 스레드다.
- 오른쪽 Timers 패널은 타이머별 집계다. 구간을 선택하지 않았으므로 세션 전체(엔진 시작, 로딩 포함) 값이다. 이 상태의 수치는 쓰지 않는다.

## 3단계. 측정 구간의 북마크를 찾는다

아래쪽 Log View의 검색 칸에 `Lab_Measure`를 넣는다.

**왜.** 서버는 측정 구간의 시작과 끝에 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 남긴다(`LabMetricsSubsystem.cpp`의 `TRACE_BOOKMARK`). CSV 수치는 이 두 시점 사이만 집계한 값이다. Insights에서도 같은 구간을 봐야 두 수치를 대조할 수 있다. 로그가 1,901줄이라 검색으로 거른다.

![Lab_Measure로 거른 로그](images/insights-calib-f/03-log-bookmarks.png)

**읽은 것.** 두 줄이 남는다. `Lab_MeasureStart`는 1분 34.919411초, `Lab_MeasureEnd`는 2분 34.937361초다. 차이는 60.018초로 측정 구간 설정(60초)과 맞다.

## 4단계. 두 북마크 사이를 시간 구간으로 선택한다

첫 줄을 클릭하고 둘째 줄을 Shift-클릭한다.

**왜.** 로그 두 줄을 범위로 선택하면 타임라인의 시간 선택이 그 두 시각 사이로 잡힌다. 눈금을 손으로 드래그하는 것보다 정확하다. 시간 구간을 선택하면 Timers 패널이 그 구간만 집계한다.

![측정 구간을 선택한 화면](images/insights-calib-f/04-range-selected.png)

**읽은 것.** 타임라인에 `Lab_MeasureStart` 표시부터 파란 선택 영역이 생기고 위에 `1m 0s`라고 찍힌다. Timers 패널의 수치가 2단계와 달라졌다(예: `Frame`의 Count가 1,563에서 357로).

## 5단계. Timers 패널에서 큰 타이머를 읽는다

Timers 패널은 Incl(자식 포함 시간) 내림차순이다. 위에서부터 읽는다.

**왜.** Incl이 큰 순서로 내려가면 호출 계층을 따라 내려가는 것과 같다. 어느 깊이에서 Incl이 크게 줄어드는지 보면 시간이 쓰이는 곳이 드러난다. Count는 호출 횟수라서, 프레임 수(356)로 나누면 프레임당 몇 번 불리는지 알 수 있다.

![측정 구간의 Timers 패널](images/insights-calib-f/05-timers.png)

**읽은 것.**

| 타이머 | Count | Incl | Excl | 프레임당 Incl (Incl ÷ 356) |
| --- | --- | --- | --- | --- |
| `Frame` | 357 | 1분 0초 | 16.86ms | 168.1ms (60.018초 ÷ 357) |
| `WorldTick` | 356 | 59.84초 | 225.18ms | 168.1ms |
| `NetBroadcastTickTime` | 356 | 57.45초 | 1.86ms | 161.4ms |
| `GameNetDriver` | 356 | 57.45초 | 21.51초 | 161.4ms |
| `LabResourceNode` | 14,242,850 | 30.05초 | 30.05초 | 84.4ms |
| `LabNpc` | 961,200 | 5.76초 | 5.72초 | 16.2ms |

- **프레임 시간.** 60.018초 ÷ 357 = 168.1ms. CSV `work_avg_ms` 168.309와 0.1% 차이다. CSV의 `frames` 356은 `WorldTick`의 Count와 같다.
- **네트워크 타이머.** `GameNetDriver` 57.45초 ÷ 356 = 161.4ms. CSV `netflush_avg_ms` 161.744와 0.2% 차이다. CSV `netflush`는 "액터 틱 종료부터 프레임 끝까지"라서 리플리케이션 전용 시간이 아니지만, 이 실행에서는 그 구간이 사실상 전부 `GameNetDriver`다.
- **`Frame`부터 `WorldTick`까지 Incl이 거의 줄지 않는다.** 프레임 시간이 월드 틱 밖(대기, 엔진의 다른 틱)에서 쓰이지 않는다는 뜻이다. 서버가 쉬는 시간 없이 계속 밀려 있다.
- **`LabResourceNode`의 Count.** 14,242,850은 5,001(노드) × 8(연결) × 356(프레임) = 14,242,848과 2 차이다. 모든 노드가 모든 연결에 대해 매 프레임 한 번씩 처리된다.
- **주의: 이 패널은 모든 스레드의 합이다.** 맨 위 `CPU` 줄의 Incl이 7분 55.6초로 60초보다 큰 것이 그 표시다. `FWindowsPlatformFile_IterateDirectoryCommon_WithCallback` 15초는 큰 값이지만 6단계의 `WorldTick` 하위 목록에 없다. 게임 스레드의 프레임 시간에 들어 있지 않다는 뜻이다. 어느 스레드인지는 확인하지 않았다.

## 6단계. `WorldTick`을 골라 하위 항목(Callees)을 본다

Timers 패널에서 `WorldTick` 줄을 클릭하면 아래 Callees 패널에 그 타이머가 부른 것들이 트리로 나온다.

**왜.** 5단계의 표는 스레드 구분 없는 합이라서 "프레임 시간 안에서의 비율"을 말해 주지 않는다. Callees는 고른 타이머 아래에서 실제로 불린 것만 보여 주고, `% Parent`와 `% Root`로 비율을 준다. `WorldTick`은 프레임 시간의 거의 전부이므로(5단계) 여기서 본 비율이 곧 프레임 안의 비율이다.

![WorldTick의 Callees](images/insights-calib-f/06-worldtick-callees.png)

**읽은 것.**

| `WorldTick` 아래 | Count | Incl | `WorldTick` 대비 |
| --- | --- | --- | --- |
| `NetBroadcastTickTime` > `GameNetDriver` | 356 | 57.45초 | 96.01% |
| `UNetConnection_ReceivedPacket` | 12,570 | 1.44초 | 2.40% |
| `TickCompletionEvents` | 1,424 | 673.86ms | 1.13% |
| `ConditionalCollectGarbage` | 356 | 34.75ms | 0.06% |
| 그 밖 | | 각각 10ms 미만 | 0.01% 이하 |

| `GameNetDriver` 아래 | Count | Incl | `GameNetDriver` 대비 |
| --- | --- | --- | --- |
| `LabResourceNode` | 14,242,848 | 30.05초 | 52.30% |
| `LabNpc` | 854,400 | 5.58초 | 9.72% |
| `LabCharacter`, `LabPlayerController` 등 나머지 일곱 | | 합 약 0.31초 | 각각 0.14% 이하 |
| `GameNetDriver` 자체(Excl) | | 21.51초 | 37.4% (21.51 ÷ 57.45) |

- **가장 큰 비용은 네트워크 송신 쪽이다.** `WorldTick`의 96.01%가 `GameNetDriver`다. 패킷 수신(2.40%)과 액터 틱(`TickCompletionEvents` 1.13%)은 합쳐도 3.5%다.
- **`NetBroadcastTickTime`과 `GameNetDriver`는 Incl이 같다.** 앞의 것은 Excl이 1.86ms뿐인 껍데기다. 리플리케이션 시간 타이머로는 안쪽 내용이 직접 달린 `GameNetDriver`를 골랐다(사용자 결정).
- **`GameNetDriver` 안에서 액터 클래스 이름의 타이머가 나온다.** 액터 하나를 연결 하나에 리플리케이트하는 구간이 그 클래스 이름으로 찍힌다. `LabNpc` 854,400은 300 × 8 × 356과 같다. 5단계의 `LabNpc` Count 961,200과의 차 106,800은 300 × 356이고, `GameNetDriver` 밖에서 프레임당 NPC당 한 번 불리는 것이다(NPC의 틱으로 추정하며 확인하지 않았다).
- **`GameNetDriver`의 Excl 21.51초(37.4%)는 이 트레이스에서 더 나뉘지 않는다.** 액터별 구간 밖에서 드라이버가 직접 쓴 시간이다. 무엇에 쓰였는지는 이 화면으로 알 수 없다.
- 계획에서 예로 든 `ServerReplicateActors`, `NetDriver TickFlush`라는 이름의 타이머는 상위 목록에 없었다.

## 7단계. 타임라인을 확대해 프레임 하나하나를 본다

타임라인 위에서 마우스 휠을 올려 확대하고, `GameThread` 줄의 막대에 마우스를 올린다.

**왜.** 5, 6단계는 60초의 합이다. 합만 보면 "가끔 튀는 프레임 몇 개가 평균을 올린 것"인지 "모든 프레임이 그런 것"인지 구분할 수 없다. 프레임을 직접 보면 알 수 있다. 막대의 세로 순서는 호출 깊이다.

![확대한 GameThread와 GameNetDriver 툴팁](images/insights-calib-f/07-frames-zoomed.png)

**읽은 것.**

- 위에서부터 `FEngineLoop::Tick` > `Frame` > `Tick_Engine` > `UWorld_Tick` > `WorldTick` > `NetBroadcastTickTime` > `GameNetDriver` 순으로 쌓여 있고, 일곱 줄의 막대 길이가 거의 같다. 프레임 전체가 `GameNetDriver`다.
- 화면에 보이는 프레임이 모두 같은 모양이다. `Frame` 막대의 길이가 145\~243ms로, 한두 프레임이 튀는 것이 아니라 매 프레임이 틱 예산 33.3ms의 네 배를 넘는다.
- 마우스를 올린 `GameNetDriver` 하나의 툴팁: Inclusive 140.09ms, Exclusive 50.61ms(36.13%), `% of Root` 96.06%. 6단계의 합계 비율(96.01%, 37.4%)과 같은 모양이다.
- 프레임과 프레임 사이에 빈 곳이 없다. 서버가 다음 틱까지 기다리는 시간이 없다.

## 8단계. Networking Insights에서 서버의 송신 방향을 고른다

`Networking Insights` 탭으로 가서 위쪽 드롭다운을 `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`으로 맞춘다.

**왜.** CSV `out_bytes_per_sec_per_conn`은 서버가 연결 하나로 보낸 바이트다. 같은 것을 보려면 서버 인스턴스의 Outgoing이어야 한다. 기본값 `Incoming`은 클라이언트가 서버로 보낸 것(이동 입력)이라 막대가 거의 없다. 연결은 0\~7 여덟 개이고 0번을 봤다. 기준선은 모든 액터가 모든 연결에 가므로 연결마다 내용이 같다고 보고 하나만 봤다(다른 연결은 확인하지 않았다).

![방향 드롭다운](images/insights-calib-f/08-net-choose-outgoing.png)

## 9단계. 측정 구간의 첫 패킷을 찾는다

패킷 막대를 클릭하면 툴팁에 그 패킷의 Timestamp가 나온다. Timestamp가 `Lab_MeasureStart`(1분 34.92초) 바로 뒤인 곳을 찾을 때까지 클릭 위치를 옮긴다.

**왜.** 이 창의 가로축은 시간이 아니라 패킷 순번이다. 북마크가 보이지 않으므로 측정 구간이 어디인지는 패킷의 Timestamp로 찾아야 한다.

![측정 구간의 첫 패킷과 내용](images/insights-calib-f/09-net-packet-start.png)

**읽은 것.**

- 고른 패킷: Sequence 7,118, Timestamp 1분 34.94초, Engine Frame 1,207, Total Size 1,023바이트.
- 막대 높이가 접속 초기를 빼면 모두 8,000비트 근처로 같다. 패킷이 전부 최대 크기로 채워져 나간다.
- 아래 Packet Content에 이 패킷의 내용이 펼쳐진다. 맨 앞에 플레이어 캐릭터, `GameStateBase`, `PlayerState` 번치가 하나씩 있고 그 뒤로 `LabNpc` 번치가 줄지어 있다. 오른쪽 Net Stats로 보면 이 패킷의 액터 번치 56개 중 53개가 `LabNpc`다. 번치마다 `Properties` > `ReplicatedMovement`가 약 95비트다. 측정 구간의 패킷은 NPC 이동으로 차 있다.

## 10단계. 측정 구간의 끝 패킷까지 범위로 선택하고 Net Stats를 읽는다

마지막 막대를 Shift-클릭한다. 선택 범위 위에 패킷 수와 시간 길이가 찍히고, 오른쪽 Net Stats가 선택 범위의 합계로 바뀐다.

**왜.** 패킷 한두 개의 크기로는 초당 송신량을 알 수 없다. 범위를 선택하면 Insights가 패킷 수, 시간 길이, 내용 종류별 비트 합계를 준다.

![선택한 패킷 범위](images/insights-calib-f/10-net-range-selected.png)

![선택 범위의 Net Stats](images/insights-calib-f/11-net-stats.png)

**읽은 것.**

- 선택 범위: 2,132패킷, 59.870초. 끝 패킷은 Sequence 9,249, Timestamp 2분 34.81초, Engine Frame 1,562. `Lab_MeasureEnd`(2분 34.94초) 직전이다.
- Net Stats의 Incl은 비트 수다.

| 항목 | Count | Incl(비트) | 뜻 |
| --- | --- | --- | --- |
| `Actor` | 107,571 | 15,713,136 | 액터 번치 전체(헤더 포함) |
| `LabNpc` | 106,608 | 11,945,853 | 그중 NPC |
| `ReplicatedMovement` | 106,608 | 9,920,301 | NPC의 이동 프로퍼티 |
| `BunchHeader` | 107,571 | 3,702,345 | 번치마다 붙는 헤더 |
| `PacketHeaderAndInfo` | 2,132 | 176,608 | 패킷마다 붙는 헤더 |
| `LabResourceNode` | 9 | 576 | 자원 노드 |

- **연결당 송신량(내용 기준).** (15,713,136 + 176,608) ÷ 8 ÷ 59.870 = 33,176바이트/초.
- **연결당 송신량(패킷 크기 기준, 상한).** 2,132 × 1,023 ÷ 59.870 = 36,430바이트/초. 모든 패킷이 1,023바이트라고 가정한 값이다.
- CSV `out_bytes_per_sec_per_conn` 34,164는 두 값 사이에 있다(내용 기준보다 3.0% 크고 패킷 크기 기준보다 6.2% 작다). 같은 수준이라고 판단했다. 두 값이 정확히 같지 않은 이유는 확인하지 않았다.
- **NPC가 송신량의 대부분이다.** `LabNpc` 11,945,853 ÷ `Actor` 15,713,136 = 76.0%이고, 나머지는 거의 번치 헤더다. `LabNpc` 106,608 ÷ 356프레임 = 299.5로, 프레임마다 NPC 300명이 한 번씩 나간다.
- **자원 노드는 거의 나가지 않는다.** 59.87초 동안 9번, 576비트다. 채집으로 체력이 바뀐 검증용 노드뿐이다.

## 판단: 왜 네트워크인가

1. 프레임 시간이 월드 틱 밖에서 쓰이지 않는다. `Frame` 60초 중 `WorldTick`이 59.84초다(5단계).
2. `WorldTick`의 96.01%가 `GameNetDriver`다. 수신은 2.40%, 액터 틱은 1.13%다(6단계).
3. 평균이 아니라 매 프레임이 그렇다. 확대해 본 프레임이 모두 같은 모양이다(7단계).
4. Insights 값과 CSV 값이 맞는다. 프레임 시간 168.1ms와 168.309ms, `GameNetDriver` 161.4ms와 161.744ms, 송신량 33,176\~36,430과 34,164(5, 10단계). CSV를 믿고 전후 비교에 써도 된다는 근거다.

따라서 기준선 조건 "가장 큰 비용이 네트워크"는 예이고, 리플리케이션 시간은 `GameNetDriver`의 프레임당 Incl로 읽는다.

## 에이전트 의견 (판단은 사용자가 한다)

화면에서 읽은 사실로부터 에이전트가 끌어낸 해석이다. 포스팅 1의 "관찰"을 쓸 때 참고할 수 있지만, 기준선 3회 측정의 트레이스에서 다시 확인해야 한다.

- **CPU를 쓰는 대상과 대역폭을 쓰는 대상이 다르다.** 자원 노드는 `GameNetDriver` 시간의 52.3%를 쓰지만 60초 동안 576비트만 나갔다. NPC는 시간의 9.7%를 쓰고 비트의 76.0%를 차지한다. 노드의 비용은 "보내는 것"이 아니라 "보낼 것이 있는지 매 프레임 연결마다 확인하는 것"으로 보인다. 호출 한 번은 30.05초 ÷ 14,242,848 = 2.1마이크로초로 작고, 횟수가 많다.
- **`GameNetDriver` 자체 시간 37.4%의 내용은 모른다.** 프레임당 21.51초 ÷ 356 = 60.4ms다. 액터별 구간으로 나뉘지 않아서, 이 부분이 무엇에 비례하는지(액터 수, 연결 수, 둘의 곱)는 이 트레이스 하나로 말할 수 없다. 기법을 적용한 뒤 이 값이 어떻게 변하는지가 단서가 된다.
- **프레임당 송신량은 서버가 느려도 일정하다.** 초당 5.9프레임(356 ÷ 60)에서 프레임마다 NPC 300명분이 나간다. 서버가 30Hz로 돌면 초당 송신량은 지금의 약 다섯 배가 된다(STATUS.md의 30Hz 환산 172,739바이트/초). 서버 프레임 시간이 줄어드는 기법은 초당 송신량을 늘릴 수 있으므로, 전후 비교에서 송신량은 `frames`와 함께 읽어야 한다.

## 확인하지 않은 것

- `FWindowsPlatformFile_IterateDirectoryCommon_WithCallback` 15초가 어느 스레드에서 무엇 때문에 도는지.
- `GameNetDriver`의 Excl 21.51초의 내용.
- 연결 1\~7의 패킷 내용이 연결 0과 같은지.
- Insights의 송신량 두 값과 CSV 값이 3\~6% 다른 이유.
