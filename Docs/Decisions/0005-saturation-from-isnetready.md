# ADR-0005: 송신 한도 포화를 프레임 끝의 `IsNetReady()`로 판정한다

- 상태: 대체됨 → [ADR-0006](0006-saturation-from-engine-analytics.md)
- 날짜: 2026-10-01 (태스크 6). 대체된 뒤 기록으로 남기려고 같은 날 작성했다.
- 출처: [engine-notes.md](../Planning/engine-notes.md) 가절 `IsNetReady`, 라절

## 맥락

기준선이 연결당 송신 한도에 포화되었는지 알아야 한다. 레거시는 포화된 연결의 리플리케이션을 미루므로, 포화 상태의 수치는 서버 처리 비용을 실제보다 작게 보여 준다. 측정 서브시스템(태스크 6)이 포화를 CSV의 한 열로 남겨야 했다.

## 결정

측정 구간의 매 프레임 끝에서 연결마다 `IsNetReady()`를 읽고, 거짓인 연결의 비율을 프레임마다 구해 평균한 값을 `saturated_ratio`로 남긴다. `IsNetReady()`는 `QueuedBits + SendBuffer.GetNumBits() <= 0`일 때 참이다(`NetConnection.cpp:2731-2751`).

## 결과

이 정의로는 지속적인 포화를 잡지 못한다는 것을 태스크 8 시작 전 점검에서 소스로 확인했다. 프레임 끝에서는 이미 그 프레임의 송신 예산이 `QueuedBits`에서 빠진 뒤라 `IsNetReady()`가 거의 항상 참이다. [ADR-0006](0006-saturation-from-engine-analytics.md)이 이 정의를 대체했다.

`smoke7`까지의 CSV 행은 이 정의로 잰 값이다.
