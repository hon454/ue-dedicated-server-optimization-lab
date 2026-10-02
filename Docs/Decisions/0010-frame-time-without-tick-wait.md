# ADR-0010: 서버 프레임 시간에서 틱 속도 제한 대기를 뺀다

- 상태: 승인됨
- 날짜: 2026-10-02 (사용자 승인)
- 출처: [구현 계획](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-implementation-plan.md) "수치의 이름과 출처"의 "서버 프레임 시간", [설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md) 8.3절, [ADR-0004](0004-insights-and-csv-metrics.md), [engine-notes.md](../Reference/engine-notes.md)의 프레임 순서 확인

## 맥락

"서버 프레임 시간"은 Timing Insights의 프레임 시간이고, 지금까지는 측정 구간 길이 ÷ `Frame` Count로 읽었다. 설계 문서 8.3절은 이 지표의 용도를 "틱 예산 33.3ms 대비 여유"로 적었다.

`Frame` 안에는 틱 속도 제한의 대기가 들어 있다. 엔진은 `GEngine->UpdateTimeAndHandleMaxTickRate()`에서 다음 틱 시각까지 잠들고(`Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp:3058-3123`), 트레이스에는 이 구간이 GameThread의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate` 타이머로 남는다. 기준선은 틱 예산을 늘 넘어서 이 대기가 프레임당 0.01\~0.02ms였고, 그래서 지금까지는 문제가 드러나지 않았다.

관련성을 적용한 `relevancy2`에서는 서버가 틱 예산 안으로 들어와 대기가 생겼다. 측정 구간의 값은 다음과 같다(Insights 내보내기로 읽은 GameThread 이벤트, 구간은 두 북마크 사이).

| 실행 | 구간 ÷ `Frame` Count(지금 정의) | 구간 안의 대기 | (구간 − 대기) ÷ `Frame` Count | CSV `work_avg_ms` | CSV `frames` |
| --- | --- | --- | --- | --- | --- |
| `relevancy2-r1` | 45.99ms | 37.048초 | 17.60ms | 17.254 | 1,304 |
| `relevancy2-r2` | 40.31ms | 33.570초 | 17.76ms | 17.447 | 1,488 |
| `relevancy2-r3` | 33.38ms | 28.297초 | 17.64ms | 17.380 | 1,797 |

지금 정의의 값은 세 실행에서 33.38\~45.99ms로 흔들리고, 흔들림은 거의 모두 대기에서 나온다. 서버가 실제로 일한 시간은 17.60\~17.76ms로 거의 같다. 대기가 길어진 이유는 확인하지 않았다(잠들기의 정밀도가 실행마다 다른 것으로 보인다). 지금 정의로는 서버가 틱 예산 안에 들어온 뒤의 여유를 읽을 수 없고, 전후 비교에서 기법의 효과보다 대기의 흔들림을 재게 된다. 결정의 근거("Insights의 프레임 시간으로 여유를 본다")가 실행 결과와 맞지 않는다.

## 결정

포스팅과 README의 "서버 프레임 시간"은 프레임에서 틱 속도 제한 대기를 뺀 시간으로 한다.

- **평균**: (측정 구간 길이 − 구간 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate` Incl) ÷ `Frame` Count. Timers 패널에서 두 값을 읽어 계산한다.
- **P99**: 측정 구간에 걸친 프레임마다 (`Frame` 길이 − 그 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate` 길이)를 구해 정렬한 뒤 ceil(N × 0.99)번째 값. `TimingInsights.ExportTimingEvents`로 두 타이머를 내보내 계산한다([insights-reading.md](../Guides/insights-reading.md)의 "P99 읽기").
- 리플리케이션 시간, 연결당 송신 대역폭 같은 다른 지표의 정의는 바꾸지 않는다. ADR-0004의 나머지도 그대로다.

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| 지금 정의 그대로 두기 | 틱 예산 안에서는 값이 틱 간격과 대기의 흔들림만 보여 준다. `relevancy2`의 세 실행이 33.38\~45.99ms로 흔들려, 뒤의 기법 효과를 이 지표로 구별할 수 없다 |
| 포스팅에 CSV `work_avg_ms`를 쓰기 | ADR-0004가 포스팅에는 Insights 값을 쓰기로 정했다. `work`는 경과 시간이라 이름이 다르다 |
| `WorldTick`의 프레임당 Incl을 쓰기 | `WorldTick` 밖의 프레임 작업(엔진 루프의 나머지)이 빠져 기준선과 이어지지 않는다. 대기를 뺀 프레임 시간은 기준선에서 지금 정의와 0.01\~0.02ms 차이다 |

## 결과

- 승인되면 구현 계획 "수치의 이름과 출처"의 "서버 프레임 시간" 정의, 설계 문서 8.3절의 해당 줄, [insights-reading.md](../Guides/insights-reading.md)의 대조 표와 "P99 읽기"를 새 정의로 고친다.
- 포스팅 1과 README의 기준선 값이 새 정의로 바뀐다. 평균 198.44 → 198.43ms(중앙값 실행 `r1`), P99 262.97 → 262.96ms(중앙값 실행 `r3`). 세 실행의 값은 평균 198.43 / 204.81 / 179.45ms, P99 268.69 / 239.03 / 262.96ms다.
- `relevancy2`의 값은 평균 17.60 / 17.76 / 17.64ms(중앙값 17.64, 변동 폭 0.16), P99 26.64 / 30.18 / 26.12ms(중앙값 26.64, 변동 폭 4.06)다.
- 서버가 틱 예산 안에 들어오면 `frames`가 실행마다 달라지고(`relevancy2`: 1,304\~1,797), 초당 값인 연결당 송신 대역폭도 함께 흔들린다. 이 ADR은 그것을 고치지 않는다. 포스팅에 한계로 적는다.
