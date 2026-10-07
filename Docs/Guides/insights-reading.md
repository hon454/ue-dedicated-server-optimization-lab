# Insights로 트레이스를 읽는 절차

측정 실행의 `.utrace`를 Unreal Insights로 열어 수치를 읽는 순서다. 포스팅마다 같은 순서로 읽어야 전후를 비교할 수 있다. 화면 캡처와 행동마다의 이유는 [insights-walkthrough-calib-f.md](insights-walkthrough-calib-f.md)에 있다.

## 누가 무엇을 하는가

| 일 | 누가 |
| --- | --- |
| Insights를 열고 아래 순서로 값을 읽어 표로 만든다 | 에이전트(컴퓨터 조작 도구로 직접 연다). 사용자가 직접 해도 된다 |
| 읽은 값과 CSV를 대조하고, 판단에 도움이 되는 의견을 낸다 | 에이전트. 사실과 의견을 구분해 적고, 확인하지 않은 것을 따로 적는다 |
| 판단(가장 큰 비용이 무엇인가, 어떤 기법을 고를 것인가) | 사용자 |
| 그 판단을 옮긴 포스팅의 "문제", "원리" 초안 | 에이전트가 쓰고 사용자가 승인한다([ADR-0015](../Decisions/0015-post-body-and-measurement-record.md)) |
| 포스팅에 넣을 Insights 스크린샷 | 에이전트가 찍어 후보로 주고, 사용자가 고르거나 직접 찍는다 |

## 여는 법

```powershell
powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label <라벨>-r1
```

측정 중에는 열지 않는다. 에디터 프로세스가 떠 있으면 스크립트가 거부한다.

## 읽는 순서

1. **Timing Insights 탭.** 아래 Log View 검색 칸에 `Lab_Measure`를 넣는다. `Lab_MeasureStart` 줄을 클릭하고 `Lab_MeasureEnd` 줄을 Shift-클릭한다. 타임라인에 약 60초의 선택 영역이 생긴다.
2. **Timers 패널.** `Frame`, `WorldTick`, `GameNetDriver`, `FEngineLoop_UpdateTimeAndHandleMaxTickRate`(틱 속도 제한 대기)의 Count, Incl, Excl을 읽는다. 프레임당 값은 Incl ÷ `WorldTick`의 Count다. 이 패널은 모든 스레드의 합이므로 비율은 여기서 계산하지 않는다.
3. **Callees 패널.** Timers에서 `WorldTick`을 클릭한다. `GameNetDriver`의 `% Parent`와 그 아래 액터 클래스별 Count, Incl, `% Parent`를 읽는다. `GameNetDriver`의 Excl도 적는다.
4. **타임라인 확대.** 마우스 휠로 프레임 십여 개가 보일 때까지 확대해, 프레임들이 같은 모양인지 본다.
5. **Networking Insights 탭.** 드롭다운을 `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`으로 맞춘다.
6. **패킷 범위 선택.** 막대를 클릭해 툴팁의 Timestamp가 `Lab_MeasureStart` 직후인 패킷을 찾고, 마지막 막대를 Shift-클릭한다. 선택 범위 위의 패킷 수와 시간 길이, Net Stats의 `Actor`, 액터 클래스별 줄, `PacketHeaderAndInfo`의 Count와 Incl(비트)을 읽는다.
7. **대조.** 아래 표를 채운다.

| Insights에서 읽은 값 | 식 | 대조할 CSV 열 |
| --- | --- | --- |
| 서버 프레임 시간 | (선택 구간 길이 − `FEngineLoop_UpdateTimeAndHandleMaxTickRate`의 Incl) ÷ `Frame`의 Count | `work_avg_ms` |
| 서버 프레임 시간 P99 | 측정 구간에 걸친 프레임마다 (`Frame` 길이 − 그 안의 `FEngineLoop_UpdateTimeAndHandleMaxTickRate` 길이)를 정렬한 뒤 ceil(N × 0.99)번째 값(아래 "P99 읽기") | `work_p99_ms` |
| 리플리케이션 시간 | `GameNetDriver`의 Incl ÷ `WorldTick`의 Count | `netflush_avg_ms` |
| 연결당 송신량 | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위의 시간 길이 | `out_bytes_per_sec_per_conn` |

`calib-f-r1`에서 세 값의 차이는 각각 0.1%, 0.2%, 3.0%였다. 이보다 크게 벌어지면 구간 선택이 틀렸는지 먼저 본다.

리플리케이션 시간으로 쓰는 타이머는 `GameNetDriver`다(태스크 8.5에서 사용자가 확정). 이후 바꾸지 않는다.

**P99 읽기.** Timers 패널에는 백분위가 없어서, 프레임 하나하나의 길이를 Insights의 내보내기 명령으로 받는다(`TimingInsights.ExportTimingEvents`, `Engine/Source/Developer/TraceInsights/Private/Insights/TimingProfiler/TimingProfilerManager.cpp:802`). 창 없이 실행하는 방법은 엔진 테스트 `ExportCommandsTests.cpp`와 같다.

```
<엔진>\Engine\Binaries\Win64\UnrealInsights.exe -OpenTraceFile="Saved\Traces\<라벨>-rN.utrace" -AutoQuit -NoUI -log -ExecOnAnalysisCompleteCmd="TimingInsights.ExportTimingEvents <출력>.csv -columns=ThreadName,TimerName,StartTime,EndTime,Duration -threads=GameThread -timers=Frame,FEngineLoop_UpdateTimeAndHandleMaxTickRate"
```

출력의 `StartTime`, `EndTime`은 Log View의 Session Time과 같은 기준(초)이다. 두 북마크 시각에 걸친 `Frame` 이벤트(시작이 `Lab_MeasureEnd`보다 앞이고 끝이 `Lab_MeasureStart`보다 뒤)를 고르면 개수가 Timers 패널의 `Frame` Count와 같다(`baseline3-r1`\~`r3`: 303, 293, 335). 프레임마다 길이에서 그 안에 든 `FEngineLoop_UpdateTimeAndHandleMaxTickRate` 이벤트의 길이를 빼고([ADR-0010](../Decisions/0010-frame-time-without-tick-wait.md)), 정렬해 ceil(N × 0.99)번째 값을 읽는다. 서버 CSV의 `work_p99_ms`(`LabMetricsSubsystem.cpp`의 `Percentile99`)와 같은 방식이다. 같은 이벤트로 계산한 평균이 2단계의 서버 프레임 시간과 같은지도 확인한다. P99와 `work_p99_ms`의 차이는 `baseline3`에서 +0.08\~+0.12%, `relevancy2`에서 +1.6\~+2.5%였다. 서버가 틱 예산 안에 들어오면 대기가 프레임당 15\~28ms가 되므로(`relevancy2`) 빼지 않으면 값이 틱 간격에 붙는다.

## 에이전트가 직접 열 때 알아 둘 것

- Insights는 시작 메뉴에 없는 실행 파일이라, 실행한 뒤 컴퓨터 조작 권한을 실행 파일의 전체 경로(`<엔진>\Engine\Binaries\Win64\UnrealInsights.exe`)로 요청한다. 이름만으로는 잡히지 않았다. 창이 뜨기 전에 요청하면 설치되지 않은 앱으로 거절된다(2026-10-04, 창이 뜬 뒤 다시 요청해 허용됨).
- 창 크기(아래의 3000×2080)를 PowerShell에서 `MoveWindow`로 정할 때는 먼저 `SetProcessDPIAware()`를 부른다. 부르지 않으면 150% 배율에서 값이 논리 좌표로 들어가 창이 화면보다 커진다(2026-10-04).
- 원격 데스크톱 세션에서는 컴퓨터 조작 도구의 마우스 이동이 듣지 않는다(클릭은 현재 커서 자리에 들어간다). 커서를 PowerShell(`[System.Windows.Forms.Cursor]::Position`)로 옮긴 뒤 클릭한다. 원격 창을 닫기만 한 연결 끊김 세션에서는 화면이 그려지지 않아 조작도 캡처도 되지 않는다([troubleshooting.md](troubleshooting.md)).
- **Timing 값은 `Scripts/export-insights.ps1 -Label <라벨>-rN`이 아래 두 항목의 절차를 한 번에 한다(2026-10-05).** 서버 로그로 측정 구간을 정하고, `Frame` 이벤트로 서버 프레임 시간 평균과 P99를 계산하고, 타이머 통계와 `WorldTick` 아래 트리를 내보낸다. 결과는 `Saved/InsightsExport/<라벨>-rN/`의 CSV와 `summary.txt`(`GameNetDriver` 아래 클래스별 프레임당 Count, Incl, Excl)다. 선택 구간의 `WorldTick` Count가 CSV `frames`와 다르면 실패로 끝난다. `act2-update-frequency1-r1`에서 손으로 고른 구간의 값과 같았다([Posts/06-three-techniques-again/candidates.md](../../Posts/06-three-techniques-again/candidates.md) 1\~3절). Networking Insights는 내보내기 명령이 없어 창에서 읽는다.
- 창을 조작하기 어려우면 북마크 시각만 Log View에서 읽고, Timers와 Callees 값은 내보내기 명령으로 창 없이 얻는다: 아래 "P99 읽기"의 명령에서 `-ExecOnAnalysisCompleteCmd`를 `TimingInsights.ExportTimerStatistics <출력>.csv -threads=GameThread -startTime=<초> -endTime=<초>`나 `TimingInsights.ExportTimerCallees <출력>.csv -timers=WorldTick -threads=GameThread -startTime=<초> -endTime=<초>`로 바꾼다. `dormancy2-r2`에서 창의 값과 같았다([Posts/03-dormancy/candidates.md](../../Posts/03-dormancy/candidates.md)).
- 북마크 시각도 창 없이 구할 수 있다. 서버 로그 줄의 둘째 대괄호는 프레임 번호를 1,000으로 나눈 나머지다. `Measuring 60s` 줄이 측정을 시작한 프레임, CSV 행을 찍은 줄이 끝난 프레임, 로그의 마지막 줄이 서버의 마지막 프레임이다. `TimingInsights.ExportTimingEvents`로 `Frame` 이벤트를 받으면 마지막 이벤트가 로그의 마지막 프레임이므로, 끝난 프레임의 이벤트와 그 `frames`개 앞의 이벤트가 시작한 시각을 `-endTime`, `-startTime`으로 준다. 북마크는 그 프레임 안의 어느 시점이라 한 프레임까지 어긋날 수 있다. 맞게 골랐으면 선택 구간의 `WorldTick` Count가 CSV의 `frames`와 같다(`calib2-a-r1` 1,794, `calib2-d3-r1` 325. [Worklog/05-expanded-testbed.md](../Worklog/05-expanded-testbed.md) "태스크 20.4"). Networking Insights에서는 이 시각을 막대 툴팁의 Timestamp나 Engine Frame Number와 맞춘다.
- 포스팅용 캡처는 전후를 같은 창 크기(3000×2080)로 찍는다. 창 크기가 다르면 Networking 화면의 한 픽셀에 드는 패킷 수가 달라져 같은 범위를 고를 수 없다(`dormancy2-r2`: 1728 폭에서 1,788패킷, 3000 폭에서 1,785패킷).
- 권한이 없는 다른 창은 Insights 뒤에 있어도 그 자리가 스크린샷에서 가려진다. 가리는 창을 치우거나 보기 권한을 받는다.
- 문서에 넣을 캡처는 화면 복사가 아니라 창 내용만 찍는다: `powershell -ExecutionPolicy Bypass -File Scripts/capture-insights.ps1 -Label <라벨>-rN -Out <경로>.png`(`-Height N`이면 위쪽 N픽셀만). 화면 복사에는 에이전트가 화면을 조작하는 동안 화면 가장자리에 그려지는 주황 테두리가 들어간다(2026-10-02, `Posts/00-testbed`와 `Posts/01-baseline`의 이미지를 이 스크립트로 다시 찍었다). 타임라인 툴팁도 함께 찍힌다. 포스팅에 넣을 때는 본문이 인용하는 값에 `Scripts/annotate-image.ps1`로 번호 붙은 상자를 최대 3개 그려 포스팅용 이름으로 저장하고, 원본은 그대로 둔다.
- 이 PC는 3840×2160에 150% 배율이다. Claude 앱의 작은 창이 화면 오른쪽 위에 항상 떠 있어서, 조작하는 동안 Insights 창을 그 왼쪽에 들어가는 크기(3000×2080픽셀, 왼쪽 위 0,0)로 둔다. 뒤에 있는 권한 없는 창(작업 관리자 같은 관리자 권한 창은 옮길 수 없다)이 조작용 스크린샷을 가리면 그 앱의 보기 권한을 받는다.
- 툴바의 `Callers`, `Callees` 단추는 패널을 켜고 끄는 단추다. 줄을 고르려다 누르면 패널이 사라진다.
- Networking Insights의 패킷 그래프는 그래프 위에서 마우스 휠을 올리면 가로축이 확대된다. 큰 패킷 막대를 클릭하면 아래 Packet Content에 그 패킷의 Bunch와 프로퍼티가 펼쳐진다. 전후 캡처는 같은 패킷 수(예: 약 120패킷)가 보이게 휠 횟수를 맞춘다. 트레이스마다 패킷 수가 달라 같은 휠 횟수로는 폭이 달라진다(`act2-invown-base1-r2`, `act2-invown1-r2`, 2026-10-06).
- 한 막대에 드는 패킷 수는 트레이스의 전체 패킷 수에 따라 달라서, 측정 구간의 첫 막대와 끝 막대의 위치는 트레이스마다 툴팁으로 다시 찾는다(`act2-invown1-r2`는 같은 창 폭에서 `act2-invown-base1-r2`와 배율이 달랐다).
- Networking Insights의 가로축은 패킷 순번이다. 화면 한 픽셀에 패킷 여러 개가 들어가므로, 툴팁의 Timestamp로 위치를 확인한다. 툴팁(Timestamp, Engine Frame Number)은 막대에 마우스를 올려야 보인다. `Find Packet`의 화살표로 선택을 옮기면 툴팁이 나오지 않는다. 측정 구간의 첫 프레임과 마지막 프레임이 든 막대를 툴팁으로 찾아 클릭과 Shift-클릭으로 고른다(`act2-baseline1-r2`, [Posts/05-expanded-testbed/candidates.md](../../Posts/05-expanded-testbed/candidates.md) 6절).
