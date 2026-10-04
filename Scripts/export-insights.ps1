# 실행 라벨의 트레이스에서 측정 구간의 Timing Insights 값을 창 없이 내보낸다.
#   powershell -ExecutionPolicy Bypass -File Scripts/export-insights.ps1 -Label act2-dormancy1-r2
# 결과: Saved/InsightsExport/<라벨>/ 에 frames.csv(Frame과 틱 대기 이벤트), stats.csv(타이머 통계), callees.csv(WorldTick 아래 트리),
#   summary.txt(측정 구간, 서버 프레임 시간 평균과 P99, GameNetDriver 아래 클래스별 프레임당 값)를 남긴다. 이미 있는 CSV는 다시 내보내지 않는다.
#
# 측정 구간은 서버 로그의 프레임 번호로 정한다(Docs/Guides/insights-reading.md "에이전트가 직접 열 때 알아 둘 것").
# `Measuring 60s` 줄이 측정을 시작한 프레임, CSV 행을 찍은 줄이 끝난 프레임, 로그의 마지막 줄이 서버의 마지막 프레임이다.
# 로그의 둘째 대괄호는 프레임 번호를 1,000으로 나눈 나머지라서, 내보낸 Frame 이벤트의 마지막을 로그의 마지막 프레임에 맞춰 센다.
# 서버 프레임 시간은 프레임마다 (Frame − 그 안의 FEngineLoop_UpdateTimeAndHandleMaxTickRate)다(ADR-0010).
# 타이머 통계와 Callees는 응답 파일(-ExecOnAnalysisCompleteCmd="@=<파일>", ExportCommandsTests.cpp:234)로 한 번에 내보낸다.
# 맞게 골랐으면 선택 구간의 WorldTick Count가 CSV frames와 같다. 다르면 실패로 끝낸다.
# Networking Insights(패킷과 Net Stats)는 내보내기 명령이 없어 창에서 읽는다.

param(
    [Parameter(Mandatory = $true)][string]$Label,
    [string]$OutDir = ''
)

. "$PSScriptRoot\common.ps1"

$ErrorActionPreference = 'Stop'
$Inv = [Globalization.CultureInfo]::InvariantCulture
$Insights = "$UE_ROOT\Engine\Binaries\Win64\UnrealInsights.exe"
if (-not $OutDir) { $OutDir = "$ProjectDir\Saved\InsightsExport\$Label" }
$Trace = "$ProjectDir\Saved\Traces\$Label.utrace"
$ServerLog = "$ProjectDir\Saved\Logs\server-$Label.log"

foreach ($Path in @($Insights, $Trace, $ServerLog)) {
    if (-not (Test-Path $Path)) { Write-Host "FAIL: not found: $Path"; exit 1 }
}
# 트레이스 분석은 CPU를 많이 쓴다. 측정 중에는 하지 않는다.
if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) {
    Write-Host "FAIL: UnrealEditor is running. Do not analyze traces during a measurement."
    exit 1
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

function Invoke-InsightsExport([string]$Command, [string]$LogName) {
    $ArgList = @("-OpenTraceFile=`"$Trace`"", "-ABSLOG=`"$(Join-Path $OutDir $LogName)`"", "-AutoQuit", "-NoUI",
        "-ExecOnAnalysisCompleteCmd=`"$Command`"", "-log")
    $Process = Start-Process $Insights -ArgumentList $ArgList -PassThru -WindowStyle Minimized
    if (-not $Process.WaitForExit(600000)) { throw "Unreal Insights did not finish within 600 seconds for $Trace" }
}

function Get-FrameMod([string]$Line) {
    if ($Line -match '^\[[^\]]+\]\[\s*(\d+)\]') { return [int]$Matches[1] }
    throw "no frame number in log line: $Line"
}

# 서버 로그에서 측정 구간의 시작, 끝, 마지막 프레임과 CSV의 frames를 읽는다.
$LogLines = Get-Content $ServerLog
$MeasureLine = $LogLines | Where-Object { $_ -match 'LogLabMetrics: Display: Measuring' } | Select-Object -First 1
$RowLine = $LogLines | Where-Object { $_ -match "LogLabMetrics: Display: $([regex]::Escape($Label))," } | Select-Object -First 1
$LastLine = $LogLines | Where-Object { $_ -match '^\[[^\]]+\]\[\s*\d+\]' } | Select-Object -Last 1
if (-not $MeasureLine -or -not $RowLine) { Write-Host "FAIL: 'Measuring' line or the CSV row is missing in $ServerLog"; exit 1 }
$Frames = [int](($RowLine -split ',')[5])
$StartMod = Get-FrameMod $MeasureLine
$EndMod = Get-FrameMod $RowLine
$LastMod = Get-FrameMod $LastLine

# 1. Frame과 틱 대기 이벤트를 내보내 측정 구간의 시각을 정한다.
$EventsCsv = Join-Path $OutDir 'frames.csv'
if (-not (Test-Path $EventsCsv)) {
    Invoke-InsightsExport "TimingInsights.ExportTimingEvents $EventsCsv -columns=ThreadName,TimerName,StartTime,EndTime,Duration -threads=GameThread -timers=Frame,FEngineLoop_UpdateTimeAndHandleMaxTickRate" 'insights-frames.log'
}
$Events = Import-Csv $EventsCsv
$FrameEvents = @($Events | Where-Object { $_.TimerName -eq 'Frame' } | Sort-Object { [double]$_.StartTime })
$Waits = @($Events | Where-Object { $_.TimerName -eq 'FEngineLoop_UpdateTimeAndHandleMaxTickRate' } | Sort-Object { [double]$_.StartTime })
$EndIndex = ($FrameEvents.Count - 1) - (((($LastMod - $EndMod) % 1000) + 1000) % 1000)
$StartIndex = $EndIndex - $Frames
if ($StartIndex -lt 0 -or ((($EndMod - $Frames) % 1000) + 1000) % 1000 -ne $StartMod) {
    Write-Host "FAIL: the start frame from the log ($StartMod) does not match end frame $EndMod minus frames $Frames"
    exit 1
}
$StartTime = [double]$FrameEvents[$StartIndex].StartTime
$EndTime = [double]$FrameEvents[$EndIndex].StartTime

# 2. 서버 프레임 시간: 측정 구간의 프레임마다 Frame 길이에서 그 안의 대기를 뺀다.
$Work = [System.Collections.Generic.List[double]]::new()
$WaitIndex = 0
for ($i = $StartIndex; $i -lt $EndIndex; $i++) {
    $FrameStart = [double]$FrameEvents[$i].StartTime
    $FrameEnd = [double]$FrameEvents[$i].EndTime
    while ($WaitIndex -lt $Waits.Count -and [double]$Waits[$WaitIndex].StartTime -lt $FrameStart) { $WaitIndex++ }
    $Wait = 0.0
    for ($k = $WaitIndex; $k -lt $Waits.Count -and [double]$Waits[$k].StartTime -lt $FrameEnd; $k++) { $Wait += [double]$Waits[$k].Duration }
    $Work.Add(($FrameEnd - $FrameStart - $Wait) * 1000.0)
}
$WorkAvg = ($Work | Measure-Object -Average).Average
$Sorted = $Work.ToArray()
[Array]::Sort($Sorted)
$WorkP99 = $Sorted[[Math]::Ceiling($Sorted.Count * 0.99) - 1]

# 3. 측정 구간의 타이머 통계와 WorldTick 아래 트리를 한 번에 내보낸다.
$StatsCsv = Join-Path $OutDir 'stats.csv'
$CalleesCsv = Join-Path $OutDir 'callees.csv'
if (-not (Test-Path $StatsCsv) -or -not (Test-Path $CalleesCsv)) {
    $Start = $StartTime.ToString('R', $Inv)
    $End = $EndTime.ToString('R', $Inv)
    $Response = Join-Path $OutDir 'commands.rsp'
    @(
        "TimingInsights.ExportTimerStatistics $StatsCsv -threads=GameThread -startTime=$Start -endTime=$End",
        "TimingInsights.ExportTimerCallees $CalleesCsv -timers=WorldTick -threads=GameThread -startTime=$Start -endTime=$End"
    ) | Set-Content -Encoding ASCII $Response
    Invoke-InsightsExport "@=$Response" 'insights-stats.log'
}

# 4. 요약. 같은 타이머가 여러 부모 아래에 나오므로 Callees는 줄의 ParentId로 부모를 가린다.
$Callees = @(Import-Csv $CalleesCsv)
$WorldTick = $Callees | Where-Object { $_.TimerName -eq 'WorldTick' -and $_.ParentId -eq '-1' } | Select-Object -First 1
$WorldTickCount = [int]$WorldTick.Count
$Driver = $Callees | Where-Object { $_.TimerName -eq 'GameNetDriver' } | Select-Object -First 1
$PerFrame = { param([string]$Seconds) [double]$Seconds * 1000.0 / $WorldTickCount }

$Summary = [System.Collections.Generic.List[string]]::new()
$Summary.Add("label: $Label")
$Summary.Add("window: frames $StartIndex..$EndIndex of $($FrameEvents.Count) events, -startTime $($StartTime.ToString('R', $Inv)) -endTime $($EndTime.ToString('R', $Inv)) ($(($EndTime - $StartTime).ToString('0.000', $Inv)) s)")
$Summary.Add("WorldTick count: $WorldTickCount (CSV frames $Frames)")
$Summary.Add("server frame time: avg $($WorkAvg.ToString('0.000', $Inv)) ms, p99 $($WorkP99.ToString('0.000', $Inv)) ms (ADR-0010)")
if ($Driver) {
    $Summary.Add("GameNetDriver per frame: incl $((& $PerFrame $Driver.'Inc.Time').ToString('0.000', $Inv)) ms, excl $((& $PerFrame $Driver.'Exc.Time').ToString('0.000', $Inv)) ms")
    $Summary.Add("children of GameNetDriver (per frame: count, incl ms, excl ms):")
    foreach ($Row in ($Callees | Where-Object { $_.ParentId -eq $Driver.TimerId } | Sort-Object { - [double]$_.'Inc.Time' })) {
        $Summary.Add(("  {0,-40} {1,10} {2,9} {3,9}" -f $Row.TimerName, ([double]$Row.Count / $WorldTickCount).ToString('0.0', $Inv),
            (& $PerFrame $Row.'Inc.Time').ToString('0.000', $Inv), (& $PerFrame $Row.'Exc.Time').ToString('0.000', $Inv)))
    }
}
$Summary | Set-Content -Encoding UTF8 (Join-Path $OutDir 'summary.txt')
$Summary | ForEach-Object { Write-Host $_ }

if ($WorldTickCount -ne $Frames) {
    Write-Host "FAIL: WorldTick count $WorldTickCount differs from CSV frames $Frames. Check the window."
    exit 1
}
Write-Host "SAVED: $OutDir"
