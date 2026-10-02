# Insights로 트레이스를 읽는 절차

측정 실행의 `.utrace`를 Unreal Insights로 열어 수치를 읽는 순서다. 포스팅마다 같은 순서로 읽어야 전후를 비교할 수 있다. 화면 캡처와 행동마다의 이유는 [insights-walkthrough-calib-f.md](insights-walkthrough-calib-f.md)에 있다.

## 누가 무엇을 하는가

| 일 | 누가 |
| --- | --- |
| Insights를 열고 아래 순서로 값을 읽어 표로 만든다 | 에이전트(컴퓨터 조작 도구로 직접 연다). 사용자가 직접 해도 된다 |
| 읽은 값과 CSV를 대조하고, 판단에 도움이 되는 의견을 낸다 | 에이전트. 사실과 의견을 구분해 적고, 확인하지 않은 것을 따로 적는다 |
| 판단(가장 큰 비용이 무엇인가, 어떤 기법을 고를 것인가)과 포스팅의 "관찰", "선택" 섹션 | 사용자 |
| 포스팅에 넣을 Insights 스크린샷 | 에이전트가 찍어 후보로 주고, 사용자가 고르거나 직접 찍는다 |

## 여는 법

```powershell
powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label <라벨>-r1
```

측정 중에는 열지 않는다. 에디터 프로세스가 떠 있으면 스크립트가 거부한다.

## 읽는 순서

1. **Timing Insights 탭.** 아래 Log View 검색 칸에 `Lab_Measure`를 넣는다. `Lab_MeasureStart` 줄을 클릭하고 `Lab_MeasureEnd` 줄을 Shift-클릭한다. 타임라인에 약 60초의 선택 영역이 생긴다.
2. **Timers 패널.** `Frame`, `WorldTick`, `GameNetDriver`의 Count, Incl, Excl을 읽는다. 프레임당 값은 Incl ÷ `WorldTick`의 Count다. 이 패널은 모든 스레드의 합이므로 비율은 여기서 계산하지 않는다.
3. **Callees 패널.** Timers에서 `WorldTick`을 클릭한다. `GameNetDriver`의 `% Parent`와 그 아래 액터 클래스별 Count, Incl, `% Parent`를 읽는다. `GameNetDriver`의 Excl도 적는다.
4. **타임라인 확대.** 마우스 휠로 프레임 십여 개가 보일 때까지 확대해, 프레임들이 같은 모양인지 본다.
5. **Networking Insights 탭.** 드롭다운을 `Game Instance 0 [Server]`, `Connection 0`, `Outgoing`으로 맞춘다.
6. **패킷 범위 선택.** 막대를 클릭해 툴팁의 Timestamp가 `Lab_MeasureStart` 직후인 패킷을 찾고, 마지막 막대를 Shift-클릭한다. 선택 범위 위의 패킷 수와 시간 길이, Net Stats의 `Actor`, 액터 클래스별 줄, `PacketHeaderAndInfo`의 Count와 Incl(비트)을 읽는다.
7. **대조.** 아래 표를 채운다.

| Insights에서 읽은 값 | 식 | 대조할 CSV 열 |
| --- | --- | --- |
| 서버 프레임 시간 | 선택 구간 길이 ÷ `Frame`의 Count | `work_avg_ms` |
| 서버 프레임 시간 P99 | 측정 구간에 걸친 `Frame` 이벤트 길이를 정렬한 뒤 ceil(N × 0.99)번째 값(아래 "P99 읽기") | `work_p99_ms` |
| 리플리케이션 시간 | `GameNetDriver`의 Incl ÷ `WorldTick`의 Count | `netflush_avg_ms` |
| 연결당 송신량 | (`Actor` Incl + `PacketHeaderAndInfo` Incl) ÷ 8 ÷ 선택 범위의 시간 길이 | `out_bytes_per_sec_per_conn` |

`calib-f-r1`에서 세 값의 차이는 각각 0.1%, 0.2%, 3.0%였다. 이보다 크게 벌어지면 구간 선택이 틀렸는지 먼저 본다.

리플리케이션 시간으로 쓰는 타이머는 `GameNetDriver`다(태스크 8.5에서 사용자가 확정). 이후 바꾸지 않는다.

**P99 읽기.** Timers 패널에는 백분위가 없어서, 프레임 하나하나의 길이를 Insights의 내보내기 명령으로 받는다(`TimingInsights.ExportTimingEvents`, `Engine/Source/Developer/TraceInsights/Private/Insights/TimingProfiler/TimingProfilerManager.cpp:802`). 창 없이 실행하는 방법은 엔진 테스트 `ExportCommandsTests.cpp`와 같다.

```
<엔진>\Engine\Binaries\Win64\UnrealInsights.exe -OpenTraceFile="Saved\Traces\<라벨>-rN.utrace" -AutoQuit -NoUI -log -ExecOnAnalysisCompleteCmd="TimingInsights.ExportTimingEvents <출력>.csv -columns=ThreadName,TimerName,StartTime,EndTime,Duration -threads=GameThread -timers=Frame"
```

출력의 `StartTime`, `EndTime`은 Log View의 Session Time과 같은 기준(초)이다. 두 북마크 시각에 걸친 `Frame` 이벤트(시작이 `Lab_MeasureEnd`보다 앞이고 끝이 `Lab_MeasureStart`보다 뒤)를 고르면 개수가 Timers 패널의 `Frame` Count와 같다(`baseline3-r1`\~`r3`: 303, 293, 335). 그 길이를 정렬해 ceil(N × 0.99)번째 값을 읽는다. 서버 CSV의 `work_p99_ms`(`LabMetricsSubsystem.cpp`의 `Percentile99`)와 같은 방식이다. 같은 이벤트로 계산한 평균(구간 길이 ÷ 개수)이 2단계의 서버 프레임 시간과 같은지도 확인한다. `baseline3`에서 P99와 `work_p99_ms`의 차이는 +0.08\~+0.13%였다.

## 에이전트가 직접 열 때 알아 둘 것

- Insights는 시작 메뉴에 없는 실행 파일이라, 실행한 뒤 컴퓨터 조작 권한을 `UnrealInsights.exe` 이름으로 요청한다.
- 권한이 없는 다른 창은 Insights 뒤에 있어도 그 자리가 스크린샷에서 가려진다. 가리는 창을 치우거나 보기 권한을 받는다.
- 문서에 넣을 캡처는 화면 복사가 아니라 창 내용만 찍는다: `powershell -ExecutionPolicy Bypass -File Scripts/capture-insights.ps1 -Label <라벨>-rN -Out <경로>.png`(`-Height N`이면 위쪽 N픽셀만). 화면 복사에는 에이전트가 화면을 조작하는 동안 화면 가장자리에 그려지는 주황 테두리가 들어간다(2026-10-02, `Posts/00-testbed`와 `Posts/01-baseline`의 이미지를 이 스크립트로 다시 찍었다). 타임라인 툴팁도 함께 찍힌다. 포스팅에 넣을 때는 본문이 인용하는 값에 `Scripts/annotate-image.ps1`로 번호 붙은 상자를 최대 3개 그려 포스팅용 이름으로 저장하고, 원본은 그대로 둔다.
- 이 PC는 3840×2160에 150% 배율이다. Claude 앱의 작은 창이 화면 오른쪽 위에 항상 떠 있어서, 조작하는 동안 Insights 창을 그 왼쪽에 들어가는 크기(3000×2080픽셀, 왼쪽 위 0,0)로 둔다. 뒤에 있는 권한 없는 창(작업 관리자 같은 관리자 권한 창은 옮길 수 없다)이 조작용 스크린샷을 가리면 그 앱의 보기 권한을 받는다.
- 툴바의 `Callers`, `Callees` 단추는 패널을 켜고 끄는 단추다. 줄을 고르려다 누르면 패널이 사라진다.
- Networking Insights의 가로축은 패킷 순번이다. 화면 한 픽셀에 패킷 여러 개가 들어가므로, 툴팁의 Timestamp로 위치를 확인한다.
