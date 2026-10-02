param(
    [int]$Nodes = 5000,
    [int]$Npcs = 300
)

. "$PSScriptRoot\common.ps1"

$Processes = @()
try {
    $Server = Start-Process -FilePath $Editor -PassThru -ArgumentList (Get-LabServerArgs "server-manual.log" $Nodes $Npcs)
    $Processes += $Server
    Disable-LabTimerThrottle $Server
    Start-Sleep -Seconds 20

    # 채집 담당. 검증용 노드 옆에 서서 채집한다.
    $Processes += Start-Process -FilePath $Editor -PassThru -ArgumentList (
        (Get-LabClientArgs 0 "manual" "client0-manual.log" 960 540 0 0) + @("-LabAutoHarvest"))
    Start-Sleep -Seconds 3

    # 관찰자. 사람이 직접 조작한다.
    $Processes += Start-Process -FilePath $Editor -PassThru -ArgumentList (
        Get-LabClientArgs 0 "manual" "client1-manual.log" 960 540 960 0)

    Read-Host "Enter를 누르면 모두 종료합니다"
}
finally {
    foreach ($Process in $Processes) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}
