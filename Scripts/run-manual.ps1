param(
    [int]$Nodes = 5000,
    [int]$Npcs = 300
)

. "$PSScriptRoot\common.ps1"

$Processes = @()
try {
    $Processes += Start-Process -FilePath $Editor -PassThru -ArgumentList @(
        "`"$Project`"", "/Game/Maps/L_Lab", "-server", "-log", "-DisablePython", "-LOG=server-manual.log",
        "-LabNodes=$Nodes", "-LabNpcs=$Npcs"
    )
    Start-Sleep -Seconds 20

    # 채집 담당. 검증용 노드 옆에 서서 채집한다.
    $Processes += Start-Process -FilePath $Editor -PassThru -ArgumentList @(
        "`"$Project`"", "127.0.0.1", "-game", "-windowed", "-ResX=960", "-ResY=540", "-WinX=0", "-WinY=0",
        "-log", "-LOG=client0-manual.log", "-nosound", "-DisablePython", "-LabSlot=0", "-LabLabel=manual", "-LabAutoHarvest"
    )
    Start-Sleep -Seconds 3

    # 관찰자. 사람이 직접 조작한다.
    $Processes += Start-Process -FilePath $Editor -PassThru -ArgumentList @(
        "`"$Project`"", "127.0.0.1", "-game", "-windowed", "-ResX=960", "-ResY=540", "-WinX=960", "-WinY=0",
        "-log", "-LOG=client1-manual.log", "-nosound", "-DisablePython", "-LabSlot=0", "-LabLabel=manual"
    )

    Read-Host "Enter를 누르면 모두 종료합니다"
}
finally {
    foreach ($Process in $Processes) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}
