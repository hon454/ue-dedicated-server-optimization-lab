param(
    [Parameter(Mandatory = $true)][string]$Label,
    [int]$Clients = 8,
    [int]$Nodes = 5000,
    [int]$Npcs = 300,
    [int]$Warmup = 30,
    [int]$Measure = 60,
    [int]$Runs = 1,
    [int]$TimeoutSeconds = 900,
    [switch]$NoTrace
)

. "$PSScriptRoot\common.ps1"

$TraceDir = "$ProjectDir\Saved\Traces"
$Summary = "$ProjectDir\Saved\LabMetrics\summary.csv"
New-Item -ItemType Directory -Force -Path $TraceDir | Out-Null

function Get-RowCount([string]$Prefix) {
    if (-not (Test-Path $Summary)) { return 0 }
    return @(Get-Content $Summary | Where-Object { $_.StartsWith($Prefix) }).Count
}

# 같은 라벨을 다시 쓰면 CSV 행과 트레이스, 스크린샷의 대응이 어긋난다.
if ((Get-RowCount "$Label-r") -gt 0) {
    Write-Host "FAIL: label '$Label' already has rows in summary.csv. Use a new label."
    exit 1
}

# 서버는 논리 프로세서 0~7, 클라이언트는 나머지에 고정한다.
$Logical = [Environment]::ProcessorCount
$ServerMask = [long]0xFF
$ClientMask = ([long][math]::Pow(2, $Logical) - 1) - $ServerMask

# 에디터 실행 파일은 시작하는 동안 프로세스 선호도가 전체 코어로 되돌아간다(2026-10-01 smoke1, smoke3에서 관찰).
# 그래서 실행 직후 한 번 설정하고, 서버를 기다리는 동안 2초마다 다시 읽어 달라져 있으면 다시 설정한다.
# 되돌린 횟수를 돌려준다.
function Set-Affinity($Process, [long]$Mask) {
    if ($Process.HasExited) { return 0 }
    $Actual = (Get-Process -Id $Process.Id).ProcessorAffinity.ToInt64()
    if ($Actual -eq $Mask) { return 0 }
    $Process.ProcessorAffinity = [IntPtr]$Mask
    return 1
}

$Failed = $false

for ($Run = 1; $Run -le $Runs; $Run++) {
    $RunLabel = "$Label-r$Run"
    Write-Host "=== $RunLabel ==="

    $ServerArgs = @(
        "`"$Project`"", "/Game/Maps/L_Lab", "-server", "-log", "-unattended", "-DisablePython",
        "-LOG=server-$RunLabel.log",
        "-LabMeasure", "-LabLabel=$RunLabel",
        "-LabNodes=$Nodes", "-LabNpcs=$Npcs",
        "-LabExpectedClients=$Clients", "-LabWarmup=$Warmup", "-LabMeasureSeconds=$Measure"
    )
    if (-not $NoTrace) {
        $ServerArgs += @("-trace=default,net", "-NetTrace=1", "-tracefile=`"$TraceDir\$RunLabel.utrace`"")
    }

    $Server = $null
    $ClientProcesses = @()
    $RunOk = $false

    try {
        $Server = Start-Process -FilePath $Editor -ArgumentList $ServerArgs -PassThru
        # 핸들을 미리 잡아 두어야 종료 후 ExitCode를 읽을 수 있다.
        $null = $Server.Handle
        $null = Set-Affinity $Server $ServerMask

        # 서버가 맵을 열고 월드를 생성할 시간.
        Start-Sleep -Seconds 20

        for ($Index = 0; $Index -lt $Clients; $Index++) {
            # 640x360 창을 4열로 배치한다.
            $X = ($Index % 4) * 640
            $Y = [math]::Floor($Index / 4) * 390
            $ClientArgs = @(
                "`"$Project`"", "127.0.0.1", "-game", "-windowed",
                "-ResX=640", "-ResY=360", "-WinX=$X", "-WinY=$Y",
                "-log", "-LOG=client$Index-$RunLabel.log", "-nosound", "-unattended", "-DisablePython",
                "-LabSlot=$Index", "-LabAutoMove", "-LabLabel=$RunLabel",
                "-ExecCmds=`"t.MaxFPS 30`""
            )
            # 0번은 제자리 채집과 3인칭 스크린샷, 1번은 내려다보기 스크린샷을 맡는다.
            if ($Index -eq 0) {
                $ClientArgs += @("-LabAutoHarvest", "-LabAutoScreenshot")
            }
            if ($Index -eq 1) {
                $ClientArgs += @("-LabTopDown", "-LabAutoScreenshot")
            }
            $Client = Start-Process -FilePath $Editor -ArgumentList $ClientArgs -PassThru
            $null = Set-Affinity $Client $ClientMask
            $ClientProcesses += $Client
            Start-Sleep -Seconds 3
        }

        # 서버가 끝나기를 기다리면서, 클라이언트가 먼저 죽으면 바로 실패로 끝낸다.
        $Deadline = (Get-Date).AddSeconds($TimeoutSeconds)
        $DeadClient = $null
        $LastAffinityFix = Get-Date
        while (-not $Server.WaitForExit(2000)) {
            $Fixed = Set-Affinity $Server $ServerMask
            foreach ($Client in $ClientProcesses) { $Fixed += Set-Affinity $Client $ClientMask }
            if ($Fixed -gt 0) { $LastAffinityFix = Get-Date }
            $DeadClient = $ClientProcesses | Where-Object { $_.HasExited } | Select-Object -First 1
            if ($DeadClient -or (Get-Date) -gt $Deadline) { break }
        }

        if ($DeadClient) {
            $DeadIndex = [array]::IndexOf($ClientProcesses, $DeadClient)
            Write-Host "FAIL: client$DeadIndex exited before the server finished. See Saved\Logs\client$DeadIndex-$RunLabel.log"
        }
        elseif (-not $Server.HasExited) {
            Write-Host "FAIL: server did not finish within $TimeoutSeconds seconds"
        }
        elseif ($Server.ExitCode -ne 0) {
            Write-Host "FAIL: server exit code $($Server.ExitCode). See Saved\Logs\server-$RunLabel.log"
        }
        elseif ((Get-RowCount "$RunLabel,") -ne 1) {
            Write-Host "FAIL: expected exactly one new row for $RunLabel in summary.csv"
        }
        else {
            $RunOk = $true
            Write-Host "affinity: server=$ServerMask clients=$ClientMask, last re-applied at $($LastAffinityFix.ToString('HH:mm:ss'))"
        }
    }
    finally {
        foreach ($Client in $ClientProcesses) {
            Stop-Process -Id $Client.Id -Force -ErrorAction SilentlyContinue
        }
        if ($Server -and -not $Server.HasExited) {
            Stop-Process -Id $Server.Id -Force -ErrorAction SilentlyContinue
        }
    }

    if (-not $RunOk) {
        $Failed = $true
        break
    }
    Start-Sleep -Seconds 5
}

if (Test-Path $Summary) {
    Get-Content $Summary | Select-Object -First 1
    Get-Content $Summary | Where-Object { $_.StartsWith("$Label-r") }
}

if ($Failed) { exit 1 }
