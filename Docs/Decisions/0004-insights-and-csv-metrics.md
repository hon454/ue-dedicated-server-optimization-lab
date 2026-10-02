# ADR-0004: 수치를 Insights와 CSV 두 갈래로 나눈다

- 상태: 승인됨
- 날짜: 2026-10-01
- 출처: [설계 문서](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-portfolio-design.md) 8.3절, 11.2절, [구현 계획](https://github.com/hon454/ue-dedicated-server-optimization-lab/blob/post-04-update-frequency/Docs/Planning/2026-10-01-short-term-implementation-plan.md) "수치의 이름과 출처"

## 맥락

측정 결과를 읽는 쪽이 둘이다. 에이전트는 사람 없이 매 실행의 전후를 비교해야 하고, 포스팅 독자는 Unreal Insights에서 확인할 수 있는 수치를 봐야 한다. 서버 코드가 직접 잴 수 있는 값과 Insights가 보여 주는 값은 정의가 다르다.

## 결정

- 서버가 측정 구간 동안 수치를 모아 `Saved/LabMetrics/summary.csv`에 한 줄을 남긴다. 에이전트의 전후 비교와 `Docs/STATUS.md` 기록에는 이 CSV를 쓰고, CSV의 열 이름(`work`, `netflush` 등) 그대로 부른다.
- 포스팅과 README의 표에는 사람이 Insights에서 읽은 값(서버 프레임 시간, 리플리케이션 시간 등)을 쓴다.
- 서버가 측정 구간의 시작과 끝에 `Lab_MeasureStart`, `Lab_MeasureEnd` 북마크를 남겨, 두 도구가 같은 구간을 보게 한다.
- 두 값의 관계는 규모 확정 단계(태스크 8.5)에서 한 번 대조해 테스트베드 포스팅에 적는다.

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| CSV만 쓰기 | `work`와 `netflush`는 경과 시간이라 스레드 대기와 스케줄링 지연이 들어 있다. 이 값을 "프레임 시간"이나 "리플리케이션 시간"이라고 부를 수 없다 |
| Insights만 쓰기 | 매 실행마다 사람이 트레이스를 열어야 해서 에이전트가 결과를 스스로 검증할 수 없고 반복이 느려진다 |

## 결과

- 같은 이름으로 다른 값을 부르지 않는다. 이름과 출처는 구현 계획의 "수치의 이름과 출처" 표가 정한다.
- 리플리케이션 시간으로 쓸 Insights 타이머는 태스크 8.5에서 하나를 골라 고정한다.
- 연결당 열린 액터 채널 수(CSV)와 클라이언트에 존재하는 액터 수(화면 위 글자)도 다른 수치로 다룬다. 휴면 액터는 채널이 닫혀도 클라이언트에 남는다.
