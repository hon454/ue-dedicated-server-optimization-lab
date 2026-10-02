# 장기 백로그

단기 범위([설계 문서](2026-10-01-short-term-portfolio-design.md))가 끝난 뒤에 다룬다. 작업 중 떠오른 아이디어는 여기에만 적고, 진행 중인 포스팅에 넣지 않는다.

## 우선순위 순

1. **Iris 전환과 실측 비교**: 튜닝된 레거시와 같은 시나리오로 비교. 각 포스팅의 "Iris에서는" 섹션을 실측으로 교체.
2. **인벤토리와 FastArray**: 일반 `TArray` 리플리케이션과 FastArray의 전송 바이트 비교. 소유자 전용 전송.
3. **자원 노드 구역 매니저**: 노드당 액터 하나에서 구역별 매니저 + FastArray로 전환. 노드별 관련성을 잃는 트레이드오프.
4. **기본 송신 한도에서의 포화와 우선순위**: 단기에서는 측정 조건으로 연결당 송신 한도를 올렸다. 엔진 기본 한도로 되돌렸을 때 무엇이 미뤄지는지, `NetPriority`로 무엇을 먼저 보낼지 다룬다.
5. **건축물**: 플레이어가 배치하는 정적 액터의 휴면과 초기 전송 비용.
6. **Replication Graph**: 레거시, Replication Graph, Iris 세 시스템 비교.
7. **Test 패키지로 재측정**: 기준선과 세 기법을 모두 적용한 구성을 Test 구성의 패키지(서버, 클라이언트)로 다시 재서, 에디터 빌드에서 본 개선이 출시 빌드에 가까운 조건에서도 유지되는지 확인한다. Shipping은 트레이스가 컴파일되지 않아 Insights로 볼 수 없으므로(엔진 소스: `TraceLog/Public/Trace/Config.h:12-17`, `NetTraceConfig.h:10-16`) Test가 잴 수 있는 가장 Shipping에 가까운 구성이다. 실행을 두 종류로 나눈다.
   - **수치 실행**: CSV 수치와 `GameNetDriver` 타이머는 이 실행에서 얻는다. `GameNetDriver` 타이머는 Test에서도 남는다(`NetDriver.cpp:1174`의 `TRACE_CPUPROFILER_EVENT_SCOPE_TEXT`).
   - **분해 실행**: `-statnamedevents`를 더한 별도 실행(`LaunchEngineLoop.cpp:1759`). Test에서는 `STATS`가 꺼져서 리플리케이션 시간을 클래스별로 나눠 보여 주는 타이머(`LabResourceNode`, `LabNpc`)가 이 인자 없이는 남지 않는다(`DataChannel.cpp:3624`의 `SCOPE_CYCLE_UOBJECT`, `UObjectBaseUtility.h:1073-1098`, `Build.h:311`). 이 인자는 이벤트를 더 기록하므로 이 실행의 수치를 수치 실행과 비교하지 않고, 비율을 보는 데만 쓴다.
   - 서버 타깃(`DSOptLabServer.Target.cs`)을 새로 만들어야 하고, 엔진을 소스로 빌드하므로 서버 타깃과 클라이언트 타깃이 각각 엔진 전체를 컴파일한다.

## 순서 미정

- 푸시 모델
- 지연과 패킷 손실 시뮬레이션에서의 동기화 품질
- 서버 틱 최적화: 애니메이션, 물리, AI 틱
- 캐릭터 무브먼트 리플리케이션 비용
- 접속 시 초기 전송 폭주 완화
- 다양한 OS와 플랫폼에서의 프로파일링 (공고 우대사항)

## 작업 중 떠오른 것

- **`GameNetDriver` 자체 시간 나누기(에디터 빌드).** 기본 트레이스에서는 `GameNetDriver` Excl(기준선 리플리케이션 시간의 37%)에 고려 목록 만들기, 연결마다의 우선순위 정렬, `Connection->Tick`의 송신이 섞여 나뉘지 않는다([Posts/01-baseline/candidates.md](../../Posts/01-baseline/candidates.md) 2절). `-statnamedevents`(`LaunchEngineLoop.cpp:1759`)를 더한 별도 실행으로 `STAT_NetConsiderActorsTime`(`NetDriver.cpp:5305`), `STAT_NetPrioritizeActorsTime`(`NetDriver.cpp:5530`), `Stat_NetConnectionTick` 같은 stat을 Insights 타이머로 보면 나눌 수 있다. 측정 조건이 달라지므로 그 실행의 수치는 비교에 쓰지 않고 비율만 본다. 클래스 타이머의 이름이 바뀌는지 먼저 확인한다(소스에서 읽은 추론). 위 "Test 패키지로 재측정"의 분해 실행과 같은 방식이다(2026-10-02, 태스크 10.3).
- **같은 구성의 실행 사이 흔들림의 원인.** `baseline3`에서 느린 실행은 일의 양과 구성 비율이 같고 모든 하위 타이머가 1.12\~1.18배 느렸다(candidates.md 4절). CPU 클럭이나 같은 코어를 쓰는 다른 작업 같은 서버 밖의 요인으로 보이며 확인하지 않았다(2026-10-02, 태스크 10.3).
- **휴면 노드가 클라이언트에 남는 문제.** 휴면으로 채널이 닫힌 노드는 플레이어가 멀어져도 서버가 닫을 채널이 없어 클라이언트에 남는다(`NetDriver.cpp:5877-5889`). `dormancy2-r2`에서 이동하는 클라이언트의 노드 수가 t=75s에 313개였다(적용 전 111개, [Posts/03-dormancy/candidates.md](../../Posts/03-dormancy/candidates.md) 3절). 오래 돌아다니면 맵의 노드를 모두 갖게 되고, 멀리 있는 동안 바뀐 상태는 다시 관련성 안에 들어와 채널이 열릴 때 받는다. 해결 후보: 클라이언트가 거리 밖의 휴면 액터를 스스로 지우기, 구역 단위로 묶어 구역이 관련성을 잃을 때 정리하기(위 "자원 노드 구역 매니저"), Replication Graph나 Iris의 필터(2026-10-02, 태스크 12).
- **노드를 고려 목록에서 빼기.** 휴면을 적용해도 `GameNetDriver` 자체 시간은 프레임당 9.43ms가 남았다(`dormancy2-r2`, 적용 전 10.50ms). 노드가 활성 목록에서 빠지려면 모든 연결에서 휴면이어야 하는데(`NetworkObjectList.cpp:348-376`), 실행 중 스폰한 노드는 채널을 연 연결에서만 휴면이 된다(engine-notes.md "휴면 액터와 관련성"). 맵에 놓인 `DORM_Initial` 액터는 고려 목록을 만들 때 바로 빠진다(`NetDriver.cpp:5369-5378`). 노드를 맵에 놓는 방식(에디터 작업 없이 가능한지 확인 필요), 구역 매니저, Replication Graph의 공간 격자로 이 몫을 재 본다(2026-10-02, 태스크 12).
