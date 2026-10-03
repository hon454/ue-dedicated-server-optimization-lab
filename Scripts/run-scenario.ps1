param(
    [Parameter(Mandatory = $true)][string]$Label,
    [int]$Clients = 8,
    [int]$Nodes = 5000,
    [int]$Npcs = 300,
    [int]$Warmup = 30,
    [int]$Measure = 60,
    [int]$Runs = 1,
    [int]$TimeoutSeconds = 900,
    # 서버를 고정할 논리 프로세서의 비트 마스크. 기본값 252(0xFC)는 2~7이다(ADR-0009). 진단용으로만 바꾼다.
    [long]$ServerMask = 0xFC,
    [switch]$NoTrace,
    # 0번 자리 앞을 왕복하는 영상용 NPC 하나를 더 스폰한다(-LabShowcaseNpc). 수치를 쓰지 않는 visualN 라벨에서만 쓴다.
    [switch]$ShowcaseNpc,
    # 1막의 세 기법을 켜고 끈다. 주지 않으면 세 기법이 모두 적용된 구성이다(수치 CSV의 config 열이 default).
    # 기준선은 -AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100, Relevancy는 -NoNodeDormancy -NpcUpdateFrequency 100,
    # Dormancy는 -NpcUpdateFrequency 100이다.
    [switch]$AlwaysRelevant,
    [switch]$NoNodeDormancy,
    # NPC의 NetUpdateFrequency. 0이면 인자를 넘기지 않아 코드의 기본값(10)을 쓴다. 엔진 기본값은 100이다.
    [int]$NpcUpdateFrequency = 0
)

. "$PSScriptRoot\common.ps1"

if ($ShowcaseNpc -and $Label -notmatch '^visual\d+$') {
    Write-Host "FAIL: -ShowcaseNpc adds an actor to the scenario, so it is only allowed with a visual-only label (visualN)."
    exit 1
}

$TraceDir = "$ProjectDir\Saved\Traces"
$LogDir = "$ProjectDir\Saved\Logs"
$Summary = "$ProjectDir\Saved\LabMetrics\summary.csv"
New-Item -ItemType Directory -Force -Path $TraceDir | Out-Null

function Get-RowCount([string]$Prefix, [string]$File = $Summary) {
    if (-not (Test-Path $File)) { return 0 }
    return @(Get-Content $File | Where-Object { $_.StartsWith($Prefix) }).Count
}

# 같은 라벨을 다시 쓰면 CSV 행과 트레이스, 스크린샷의 대응이 어긋난다.
# 1막의 행은 config 열을 더하기 전의 파일(summary-act1.csv)에 있다.
$SummaryAct1 = "$ProjectDir\Saved\LabMetrics\summary-act1.csv"
if ((Get-RowCount "$Label-r") -gt 0 -or (Get-RowCount "$Label-r" $SummaryAct1) -gt 0) {
    Write-Host "FAIL: label '$Label' already has rows in summary.csv or summary-act1.csv. Use a new label."
    exit 1
}

# 실패한 실행은 CSV 행을 남기지 않으므로 로그와 트레이스로도 라벨의 재사용을 막는다.
# 트레이스 파일이 이미 있으면 엔진이 트레이스를 시작하지 않는다(TraceAuxiliary.cpp의 "Trace file ... already exists").
for ($Run = 1; $Run -le $Runs; $Run++) {
    foreach ($Existing in @("$LogDir\server-$Label-r$Run.log", "$TraceDir\$Label-r$Run.utrace")) {
        if (Test-Path $Existing) {
            Write-Host "FAIL: label '$Label' was already used ($Existing exists). Use a new label."
            exit 1
        }
    }
}

# 이전 실행의 서버가 남아 있으면 새 클라이언트가 그쪽으로 접속한다. 열려 있는 에디터도 측정에 섞인다.
$Running = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
if ($Running.Count -gt 0) {
    Write-Host "FAIL: UnrealEditor is already running (pid $($Running.Id -join ', ')). Close it first."
    exit 1
}

# 서버는 논리 프로세서 2~7, 클라이언트는 8 이상에 고정한다. 0번과 1번은 비운다(1번에 DPC가 몰린다. ADR-0009).
$Logical = [Environment]::ProcessorCount
# 8개 이하면 클라이언트에 줄 논리 프로세서가 없고, 63개 이상이면 마스크가 64비트 정수를 넘는다.
if ($Logical -le 8 -or $Logical -gt 62) {
    Write-Host "FAIL: this script needs 9 to 62 logical processors (found $Logical). The server uses 2-7 and the clients use 8 and above."
    exit 1
}
$ClientMask = (([long]1 -shl $Logical) - 1) - [long]0xFF

# 에디터 실행 파일은 시작하는 동안 프로세스 선호도가 전체 코어로 되돌아간다(2026-10-01 smoke1, smoke3에서 관찰).
# 그래서 실행 직후 한 번 설정하고, 서버를 기다리는 동안 2초마다 다시 읽어 달라져 있으면 다시 설정한다.
# 되돌린 횟수를 돌려준다.
function Set-Affinity($Process, [long]$Mask) {
    if ($Process.HasExited) { return 0 }
    # 종료 중인 프로세스는 선호도를 읽을 수 없다. 읽지 못한 것은 달라진 것이 아니므로 다시 설정하지 않고 줄만 남긴다.
    $Actual = $null
    try { $Actual = (Get-Process -Id $Process.Id -ErrorAction Stop).ProcessorAffinity.ToInt64() } catch { }
    if ($null -eq $Actual) {
        Write-Host "AFFINITY: pid $($Process.Id) could not be read at $((Get-Date).ToString('HH:mm:ss')) (exited=$($Process.HasExited))"
        return 0
    }
    if ($Actual -eq $Mask) { return 0 }
    Write-Host "AFFINITY: pid $($Process.Id) was $Actual, re-applied $Mask at $((Get-Date).ToString('HH:mm:ss'))"
    $Process.ProcessorAffinity = [IntPtr]$Mask
    return 1
}

function Start-LabClient([int]$Index, [string]$RunLabel) {
    # 640x360 창을 4열로 배치한다.
    $X = ($Index % 4) * 640
    $Y = [math]::Floor($Index / 4) * 390
    $ClientArgs = (Get-LabClientArgs $Index $RunLabel "client$Index-$RunLabel.log" 640 360 $X $Y) + @(
        "-unattended",
        # Unreal Insights가 떠 있으면 프로세스가 스스로 트레이스 서버에 연결한다(TraceAuxiliary.cpp의 TryAutoConnect). 그것을 막는다.
        "-traceautostart=0",
        "-LabAutoMove",
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
    return $Client
}

# 서버 로그에서 LogLabMetrics의 한 줄을 찾아 그 줄의 시각(로컬 시각)을 돌려준다. 없으면 $null.
# 로그의 시각은 UTC이고 형식은 [2026.10.01-11.26.38:200]이다.
function Get-ServerLogTime([string]$ServerLog, [string]$Text) {
    if (-not (Test-Path $ServerLog)) { return $null }
    $Line = Get-Content $ServerLog -ErrorAction SilentlyContinue |
        Where-Object { $_.Contains("LogLabMetrics") -and $_.Contains($Text) } | Select-Object -First 1
    if (-not $Line -or $Line -notmatch '^\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}):') { return $null }
    $Utc = [datetime]::ParseExact($Matches[1], 'yyyy.MM.dd-HH.mm.ss', [Globalization.CultureInfo]::InvariantCulture,
        [Globalization.DateTimeStyles]::AssumeUniversal -bor [Globalization.DateTimeStyles]::AdjustToUniversal)
    return $Utc.ToLocalTime()
}

# 시작 신호 전에 죽은 클라이언트를 다시 띄우는 횟수의 상한(실행당).
$MaxClientRestarts = 3

$Failed = $false

for ($Run = 1; $Run -le $Runs; $Run++) {
    $RunLabel = "$Label-r$Run"
    $ServerLog = "$LogDir\server-$RunLabel.log"
    $TraceFile = "$TraceDir\$RunLabel.utrace"
    Write-Host "=== $RunLabel ==="

    $ServerArgs = (Get-LabServerArgs "server-$RunLabel.log" $Nodes $Npcs) + @(
        "-unattended",
        "-LabMeasure", "-LabLabel=$RunLabel",
        "-LabExpectedClients=$Clients", "-LabWarmup=$Warmup", "-LabMeasureSeconds=$Measure"
    )
    if ($ShowcaseNpc) {
        $ServerArgs += "-LabShowcaseNpc"
    }
    if ($AlwaysRelevant) {
        $ServerArgs += "-LabAlwaysRelevant"
    }
    if ($NoNodeDormancy) {
        $ServerArgs += "-LabNoNodeDormancy"
    }
    if ($NpcUpdateFrequency -gt 0) {
        $ServerArgs += "-LabNpcUpdateFrequency=$NpcUpdateFrequency"
    }
    if (-not $NoTrace) {
        $ServerArgs += @("-trace=default,net", "-NetTrace=1", "-tracefile=`"$TraceFile`"")
    }
    else {
        # 이 인자는 -tracefile의 시작도 막으므로 트레이스를 켠 서버에는 주지 않는다.
        $ServerArgs += "-traceautostart=0"
    }

    $Server = $null
    $ClientProcesses = @()
    $RunOk = $false

    try {
        $Server = Start-Process -FilePath $Editor -ArgumentList $ServerArgs -PassThru
        # 핸들을 미리 잡아 두어야 종료 후 ExitCode를 읽을 수 있다.
        $null = $Server.Handle
        # 서버 콘솔 창이 가려지거나 최소화돼도 틱이 약 21Hz로 떨어지지 않게 한다(ADR-0012).
        Disable-LabTimerThrottle $Server
        $null = Set-Affinity $Server $ServerMask

        # 서버가 맵을 열고 월드를 생성할 시간.
        Start-Sleep -Seconds 20

        for ($Index = 0; $Index -lt $Clients; $Index++) {
            $ClientProcesses += Start-LabClient $Index $RunLabel
            Start-Sleep -Seconds 3
        }
        Write-Host "pids: server=$($Server.Id) clients=$(($ClientProcesses | ForEach-Object { $_.Id }) -join ',')"

        # 서버가 끝나기를 기다린다.
        # 시작 신호 전에 죽은 클라이언트는 다시 띄운다. 서버는 모든 클라이언트가 준비를 보고해야 시작 신호를 내므로 측정에 영향이 없다.
        # 시작 신호 뒤에 죽으면 바로 실패로 끝낸다.
        $Deadline = (Get-Date).AddSeconds($TimeoutSeconds)
        $DeadIndex = -1
        $Restarts = 0
        $ScenarioStarted = $false
        $LastAffinityFix = Get-Date
        while (-not $Server.WaitForExit(2000)) {
            $Fixed = Set-Affinity $Server $ServerMask
            foreach ($Client in $ClientProcesses) { $Fixed += Set-Affinity $Client $ClientMask }
            if ($Fixed -gt 0) { $LastAffinityFix = Get-Date }

            if (-not $ScenarioStarted -and (Get-ServerLogTime $ServerLog "Scenario started")) {
                $ScenarioStarted = $true
            }

            for ($Index = 0; $Index -lt $ClientProcesses.Count; $Index++) {
                if (-not $ClientProcesses[$Index].HasExited) { continue }
                if ($ScenarioStarted -or $Restarts -ge $MaxClientRestarts) {
                    $DeadIndex = $Index
                    break
                }
                $Restarts++
                Write-Host "RESTART: client$Index exited before the start signal. Relaunching ($Restarts/$MaxClientRestarts). The old log is kept as a backup in Saved\Logs."
                $ClientProcesses[$Index] = Start-LabClient $Index $RunLabel
                $LastAffinityFix = Get-Date
            }

            if ($DeadIndex -ge 0 -or (Get-Date) -gt $Deadline) { break }
        }

        # 서버가 끝난 시점에 클라이언트가 모두 살아 있어야 한다. 측정 끝 무렵에 죽은 클라이언트를 여기서 잡는다.
        if ($DeadIndex -lt 0 -and $Server.HasExited) {
            for ($Index = 0; $Index -lt $ClientProcesses.Count; $Index++) {
                if ($ClientProcesses[$Index].HasExited) { $DeadIndex = $Index; break }
            }
        }

        $MeasureStart = Get-ServerLogTime $ServerLog "Measuring"

        if ($DeadIndex -ge 0) {
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
        elseif (-not $MeasureStart) {
            Write-Host "FAIL: could not find the 'Measuring' line in Saved\Logs\server-$RunLabel.log"
        }
        elseif ($LastAffinityFix -gt $MeasureStart) {
            Write-Host "FAIL: processor affinity was re-applied at $($LastAffinityFix.ToString('HH:mm:ss')), after measuring started at $($MeasureStart.ToString('HH:mm:ss'))"
        }
        elseif (-not $NoTrace -and (-not (Test-Path $TraceFile) -or (Get-Item $TraceFile).Length -eq 0)) {
            Write-Host "FAIL: trace file is missing or empty: $TraceFile"
        }
        else {
            $RunOk = $true
            Write-Host "affinity: server=$ServerMask clients=$ClientMask, last re-applied at $($LastAffinityFix.ToString('HH:mm:ss')), measuring started at $($MeasureStart.ToString('HH:mm:ss'))"
            Write-Host "client restarts before the start signal: $Restarts"
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
