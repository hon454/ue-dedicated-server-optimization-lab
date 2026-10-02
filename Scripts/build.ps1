. "$PSScriptRoot\common.ps1"

# 다른 체크아웃의 측정이 떠 있으면 빌드가 그 실행의 CPU를 쓴다. 이 프로젝트의 에디터나 실행이 떠 있으면 DLL이 잠겨 빌드가 실패한다.
$Running = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
if ($Running.Count -gt 0) {
    Write-Host "FAIL: UnrealEditor is running (pid $($Running.Id -join ', ')). Wait for the run to finish or close the editor, then build."
    exit 1
}

& "$UE_ROOT\Engine\Build\BatchFiles\Build.bat" DSOptLabEditor Win64 Development "-project=$Project" -waitmutex
exit $LASTEXITCODE
