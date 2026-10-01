# 현재 상태

마지막 갱신: 2026-10-01

## 단계

1일차 범위(태스크 1~7)를 끝냈다. 사용자가 7.6(화면)과 7.8(수동 조작)을 확인했다(2026-10-01). 작은 규모 실행(클라이언트 2, 노드 101, NPC 10)이 종료 코드 0으로 끝나고 CSV 행과 자동 스크린샷(글자와 점)이 남는 것을 `smoke2`~`smoke5`로 확인했다(`smoke5`는 템플릿 정리 후). 확인 내용은 [engine-notes.md](Planning/engine-notes.md) 마절에 있다.

엔진은 소스 빌드로 통일했다. `DSOptLab.uproject`의 `EngineAssociation`이 `UE_DSOptLab`이고, 스크립트는 `Scripts/common.ps1`에서 같은 값을 레지스트리로 찾는다. 런처 설치본(`G:\Epic Games\UE_5.8`)으로 프로젝트를 열면 같은 `Binaries/`에 다시 빌드되므로 열지 않는다.

## 다음 할 일

1. 푸시는 사용자가 한다. Variant 코드와 쓰지 않는 에셋을 지우고(남긴 것은 engine-notes.md 다절) `482fbaf` 위에 커밋을 다시 쌓았다. 정리 전 기록은 로컬 브랜치 `backup/pre-cleanup`에 있다.
2. 2일차: 태스크 8(출발값으로 트레이스와 함께 실행, 기준선 조건 확인). 엔진 기본 송신 한도가 100,000바이트/초라 8.4a(한도 올리기)가 필요할 가능성이 높다.

## 포스팅 진행

| 포스팅 | 상태 | 태그 |
| --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 시작 전 | |
| 1. 무법지대 측정 | 시작 전 | |
| 2. 관련성과 컬 거리 | 시작 전 | |
| 3. 자원 노드 휴면 | 시작 전 | |
| 4. AI NPC 업데이트 빈도 | 시작 전 | |

## 명령

구현 계획의 태스크 8에서 확정한다.

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (2026-10-01 성공 확인. 에디터가 열려 있으면 DLL 잠금으로 실패한다)
- 시나리오 실행(`-Label`과 `-Runs` 없이): 미정(태스크 8에서 확정). 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 수치 CSV 위치: `Saved/LabMetrics/summary.csv`
- 리플리케이션 시간으로 쓰는 Insights 타이머: 미정

## 확정할 값

설계 문서 8절의 출발값을 엔진 소스 확인(1일차, 태스크 2.5)과 실측(2일차, 태스크 8)으로 확정해 여기에 적는다.

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 수 | 8 | | |
| 자원 노드 수 | 5,000 | | |
| AI NPC 수 | 300 | | |
| 준비 구간 | 30초 | | |
| `NetServerMaxTickRate` 기본값 | 30 (기억값) | 30 | 엔진 소스: `Engine/Config/BaseEngine.ini:1867`. 실행 중 적용값도 30(30초에 898프레임, `smoke2-r1`) |
| `NetCullDistanceSquared` 기본값 | 225,000,000 (기억값) | 225,000,000 (150m) | 엔진 소스: `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 | 100 (기억값) | 100 (`MinNetUpdateFrequency` 2) | 엔진 소스: `Actor.cpp:295-296` |
| 연결당 송신 한도(엔진 기본값) | 모름 | 100,000바이트/초 | 엔진 소스: `BaseEngine.ini:1839-1840, 1860-1861`. 올릴 때는 세 키를 함께 올린다(engine-notes.md 가절) |
| 연결당 송신 한도(이 프로젝트에서 고정한 값) | 기준선 실측 송신량의 약 두 배 | | |
| 리플리케이션 시스템 | 레거시여야 함 | 레거시(소스 기준) | 엔진 소스: `IrisConfig.cpp:15-16`의 `net.Iris.UseIrisReplication` 기본값 0. 서버 로그 `using replication model Generic` 확인(`smoke2-r1`) |

## 기준선 조건

구현 계획 태스크 8.4~8.6에서 채운다.

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | | |
| 지속적인 예산 초과 | | |
| 가장 큰 비용이 네트워크 | | |
| 송신 한도에 포화되지 않음(포화되면 한도를 올린다. 2026-10-01 결정) | | |

## 측정 결과

CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |

## 포스팅 주기 진행

구현 계획 태스크 11은 포스팅 2, 3, 4에 반복해서 쓴다. 현재 포스팅과 끝낸 단계를 여기에 적는다.

- 현재 포스팅: 없음
- 끝낸 단계: 없음

## 막힌 것

없음.

## 사용자에게 요청한 일

없음. 푸시는 사용자가 정한 시점에 한다.
