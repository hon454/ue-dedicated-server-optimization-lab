# 실행 라벨의 트레이스를 Unreal Insights로 연다.
#   powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label calib-f-r1
# 측정 중에는 열지 않는다. 읽는 순서는 Docs/Guides/insights-reading.md에 있다.

param(
    [Parameter(Mandatory = $true)][string]$Label
)

. "$PSScriptRoot\common.ps1"

$Insights = "$UE_ROOT\Engine\Binaries\Win64\UnrealInsights.exe"
if (-not (Test-Path $Insights)) {
    throw "UnrealInsights.exe not found: $Insights"
}

$Trace = "$ProjectDir\Saved\Traces\$Label.utrace"
if (-not (Test-Path $Trace)) {
    throw "Trace not found: $Trace"
}

if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) {
    throw "UnrealEditor is running. Do not open Insights during a measurement."
}

Start-Process $Insights -ArgumentList "-OpenTraceFile=`"$Trace`""
Write-Host "OPENED: $Trace"
