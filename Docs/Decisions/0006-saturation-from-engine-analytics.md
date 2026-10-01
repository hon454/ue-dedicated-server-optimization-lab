# ADR-0006: 송신 한도 포화를 엔진의 연결별 포화 기록으로 판정한다

- 상태: 승인됨. [ADR-0005](0005-saturation-from-isnetready.md)를 대체한다.
- 날짜: 2026-10-01 (태스크 8 시작 전 점검)
- 출처: [engine-notes.md](../Planning/engine-notes.md) 바절 "포화를 판정하는 시점", [구현 계획](../Planning/2026-10-01-short-term-implementation-plan.md) "수치의 이름과 출처"

## 맥락

[ADR-0005](0005-saturation-from-isnetready.md)의 정의는 프레임 끝에서 `IsNetReady()`를 읽는다. 5.8.3 소스에서 순서를 따라가 보면 이 시점의 값은 지속적인 포화에서도 거의 항상 참이다.

1. `ServerReplicateActors`는 연결의 `IsNetReady()`가 거짓이 되는 즉시 그 연결의 리플리케이션을 멈춘다(`NetDriver.cpp:5695, 5868`).
2. 그 뒤 `UNetConnection::Tick`이 `QueuedBits`에서 이번 프레임의 예산을 뺀다(`NetConnection.cpp:5112-5145`).
3. 리플리케이션은 `QueuedBits`가 양수가 되자마자 멈추므로 초과분이 작고, 한 프레임의 예산(100,000 ÷ 30 ≈ 3,333바이트, 계산값)을 빼면 다시 음수가 된다.

## 결정

`saturated_ratio`를 엔진이 `ServerReplicateActors`에서 연결마다 남기는 기록으로 잰다. `Connection->GetSaturationAnalytics()`(`NetConnection.cpp:6122`)의 `GetNumberOfSaturatedReplications()` ÷ `GetNumberOfReplications()`를 측정 구간의 시작과 끝의 차로 구해 모든 연결에 대해 더한다. 기록 자체는 `TrackReplicationForAnalytics(bWasSaturated)`(`NetDriver.cpp:6022-6023`)가 남긴다.

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| 프레임 끝의 `IsNetReady()`(ADR-0005) | 위의 순서 때문에 지속적인 포화를 잡지 못한다 |
| `out_bytes_per_sec_per_conn`을 `net_speed`와 비교 | 서버가 30Hz를 지키지 못하면 예산 자체가 줄어든다(`NetConnection.cpp:4816, 5117-5119`). 비교하려면 `net_speed × frames ÷ (30 × 측정 초)`로 고쳐야 해서 간접적이다 |

## 결과

- 실행 확인: `smoke8-r1` 서버 로그에서 접속 직후 `saturated_replications=14/184`가 찍히고 그 뒤로 14에서 늘지 않았다. 옛 정의는 같은 구간에서 항상 0이었다.
- 포화 판정 기준은 `saturated_ratio` 0.01 이상이다(구현 계획 태스크 8.4).
- `smoke7`까지의 행과 `smoke8`부터의 행은 이 열의 정의가 다르므로 비교하지 않는다.
- 옛 정의가 지속적인 포화를 놓친다는 것은 소스를 읽어 얻은 결론이다. 측정 구간 내내 포화된 실행으로는 아직 확인하지 않았고, 태스크 8의 출발값 실행이 첫 확인이 된다.
