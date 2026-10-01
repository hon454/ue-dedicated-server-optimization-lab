# ADR-0001: 레거시 리플리케이션으로 시작하고 Iris와 Replication Graph는 미룬다

- 상태: 승인됨
- 날짜: 2026-10-01
- 출처: [설계 문서](../Planning/2026-10-01-short-term-portfolio-design.md) 3절, [engine-notes.md](../Planning/engine-notes.md) 나절 "실제로 쓰는 리플리케이션 시스템"

## 맥락

UE 5.8.3에는 리플리케이션 시스템이 셋 있다. 기본 NetDriver 리플리케이션(레거시), Replication Graph, Iris다. 이 시리즈는 기법 하나를 적용할 때마다 같은 시나리오로 전후를 비교한다. 테스트베드 구축 단계에서 리플리케이션 시스템 자체의 동작을 확인하는 데 시간을 쓰면 첫 측정이 늦어진다.

## 결정

레거시 리플리케이션을 쓴다. 게임 코드는 표준 `UPROPERTY` 리플리케이션과 RPC만 쓴다. Iris는 장기 백로그 1순위, Replication Graph는 장기 백로그에 둔다.

5.8.3에서는 레거시가 기본이라 `Config/`에 설정을 넣지 않았다. `net.Iris.UseIrisReplication`의 기본값이 0이고(`IrisConfig.cpp:15-16`), 서버 로그에 `using replication model Generic`이 찍힌다(`smoke2-r1`).

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| Iris로 시작 | 5.8.3에서 Iris가 이 시나리오대로 동작하는지 먼저 확인해야 해서 테스트베드 구축 단계의 위험이 커진다. 비교할 대상(튜닝된 레거시)도 아직 없다 |
| Replication Graph | 노드 설계가 별도의 구현 범위다 |

## 결과

- 레거시는 연결 수 × 액터 수로 비용이 늘어서 기준선의 문제가 뚜렷하게 보인다.
- 게임 코드를 표준 리플리케이션과 RPC로 제한해 나중에 Iris로 옮기는 비용을 낮게 유지한다.
- 포스팅마다 있는 "Iris에서는" 섹션은 실행해 보지 않은 예고이고 그렇게 표기한다. Iris로 전환한 뒤 튜닝된 레거시와 같은 시나리오로 실측해 이 섹션을 바꾼다.
