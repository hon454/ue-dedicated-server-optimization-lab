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

(여기에 추가)
