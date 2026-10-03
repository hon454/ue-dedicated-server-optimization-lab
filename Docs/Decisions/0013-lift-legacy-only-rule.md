# ADR-0013: 레거시 리플리케이션만 쓴다는 제약을 푼다

- 상태: 승인됨
- 날짜: 2026-10-03 (사용자 결정)
- 출처: [ADR-0001](0001-legacy-replication.md), [backlog.md](../backlog.md), [AI NPC Net Update Frequency](../../Posts/04-update-frequency/README.md)의 "결과"와 "한계와 다음"

## 맥락

ADR-0001은 레거시 리플리케이션으로 시작하고 게임 코드를 표준 `UPROPERTY` 리플리케이션과 RPC로 제한했다. Iris로 시작하는 대안을 버린 이유는 둘이었다. 테스트베드 구축 단계의 위험이 커지고, 비교할 대상(튜닝된 레거시)이 아직 없었다.

단기 범위가 끝나 두 이유가 모두 사라졌다. 테스트베드는 고정됐고, 세 기법을 적용한 레거시 구성(`update-frequency3`)이 비교 대상으로 있다. 이 구성에서 리플리케이션 시간 8.73ms 가운데 8.29ms(95.0%)가 `GameNetDriver` Exclusive로 남았고(포스팅 4 "결과"의 표), 이것을 줄이는 후보에 Replication Graph와 Iris가 들어 있다. backlog.md의 다른 주제(FastArray, Push Model)도 "표준 `UPROPERTY` 리플리케이션과 RPC만"이라는 제한 밖이다.

## 결정

"레거시 리플리케이션만 쓴다"는 규칙을 없앤다. 포스팅이 다루는 주제에 따라 Replication Graph, Iris, FastArray, Push Model을 쓴다.

포스팅 0\~4와 태그 `post-00-testbed`\~`post-04-update-frequency`는 레거시 그대로다. 프로젝트의 기본값도 그대로 레거시이고(`net.Iris.UseIrisReplication` 기본값 0, `IrisConfig.cpp:15-16`), 시스템을 바꾸는 포스팅이 그 전환을 "적용"에 적는다.

## 고려한 대안

| 대안 | 버린 이유 |
| --- | --- |
| 규칙을 두고 포스팅마다 예외 ADR을 쓰기 | backlog.md의 Iris, Replication Graph, FastArray, Push Model이 모두 예외가 되어 규칙이 남을 이유가 없다 |

## 결과

- `AGENTS.md`의 규칙 "레거시 리플리케이션만 쓴다"를 지운다.
- ADR-0001은 대체됨이 된다. ADR-0001의 "결과" 가운데 "게임 코드를 표준 리플리케이션과 RPC로 제한"은 더 지키지 않는다.
- 루트 README의 "대상: 레거시 리플리케이션"은 포스팅 0\~4에 대해 사실이라 그대로 두고, 다른 시스템을 다루는 포스팅을 낼 때 고친다.
- 어느 주제를 어떤 순서로 다룰지는 backlog.md가 정한다. 이 ADR은 순서를 정하지 않는다.
