# 다른 스크립트가 불러 쓴다. 프로젝트 경로와 엔진 경로를 정한다.
#
# 엔진 경로는 저장소에 적지 않는다. DSOptLab.uproject의 EngineAssociation을 이 PC의 레지스트리에서 찾는다.
# 에디터가 .uproject를 열 때 쓰는 것과 같은 방법이라서, 에디터와 스크립트가 항상 같은 엔진을 쓴다.
#   1. 소스 빌드: HKCU\Software\Epic Games\Unreal Engine\Builds 의 <EngineAssociation> 값
#   2. 런처 설치본: HKLM\SOFTWARE\EpicGames\Unreal Engine\<EngineAssociation> 의 InstalledDirectory

$ProjectDir = (Resolve-Path "$PSScriptRoot\..").Path
$Project = "$ProjectDir\DSOptLab.uproject"

function Resolve-UERoot {
    $Association = (Get-Content $Project -Raw | ConvertFrom-Json).EngineAssociation

    $Builds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
    if ($Builds -and $Builds.PSObject.Properties[$Association]) {
        return $Builds.PSObject.Properties[$Association].Value
    }

    $Installed = Get-ItemProperty "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$Association" -ErrorAction SilentlyContinue
    if ($Installed -and $Installed.InstalledDirectory) {
        return $Installed.InstalledDirectory
    }

    throw "Engine '$Association' (EngineAssociation in DSOptLab.uproject) is not registered on this PC. Right-click DSOptLab.uproject and choose 'Switch Unreal Engine version'."
}

$UE_ROOT = (Resolve-UERoot).TrimEnd('\', '/')
$Editor = "$UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe"
if (-not (Test-Path $Editor)) {
    throw "UnrealEditor.exe not found: $Editor"
}

# 프로세스가 요청한 타이머 해상도를 Windows가 무시하지 못하게 한다(ADR-0012).
# Windows 11은 창이 최소화되거나 완전히 가려진 프로세스의 타이머 해상도 요청(엔진의 timeBeginPeriod(1))을 보장하지 않는다.
# 그러면 서버의 틱 속도 제한이 15.625ms 단위로만 깨어나 30Hz가 아니라 약 21Hz로 돈다(engine-notes.md 아절).
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class LabProcess {
    [StructLayout(LayoutKind.Sequential)] public struct PROCESS_POWER_THROTTLING_STATE { public uint Version, ControlMask, StateMask; }
    [DllImport("kernel32.dll", SetLastError = true)] public static extern bool SetProcessInformation(IntPtr process, int infoClass, ref PROCESS_POWER_THROTTLING_STATE info, uint size);
    [StructLayout(LayoutKind.Sequential)] public struct JOBOBJECT_BASIC_LIMIT_INFORMATION {
        public long PerProcessUserTimeLimit, PerJobUserTimeLimit; public uint LimitFlags;
        public UIntPtr MinimumWorkingSetSize, MaximumWorkingSetSize; public uint ActiveProcessLimit;
        public UIntPtr Affinity; public uint PriorityClass, SchedulingClass;
    }
    [DllImport("kernel32.dll", SetLastError = true)] public static extern IntPtr CreateJobObject(IntPtr attributes, string name);
    [DllImport("kernel32.dll", SetLastError = true)] public static extern bool SetInformationJobObject(IntPtr job, int infoClass, ref JOBOBJECT_BASIC_LIMIT_INFORMATION info, uint size);
    [DllImport("kernel32.dll", SetLastError = true)] public static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);
}
"@

# 프로세스를 선호도 제한이 걸린 Job 객체에 넣는다. 프로세스 선호도(ProcessorAffinity)만 설정하면, 엔진이 스레드마다
# SetThreadGroupAffinity로 그룹의 모든 코어를 요청할 때 Windows가 프로세스 선호도를 전체 코어로 넓힌다
# (WindowsRunnableThread.cpp:140-142, engine-notes.md 마절). Job의 제한은 스레드가 넓히지 못한다. 그 요청은 실패하고(오류 31)
# 엔진은 경고 한 줄을 남긴 채 계속 돈다.
function Set-LabJobAffinity($Process, [long]$Mask) {
    $Job = [LabProcess]::CreateJobObject([IntPtr]::Zero, $null)
    if ($Job -eq [IntPtr]::Zero) {
        throw "CreateJobObject failed (error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))."
    }
    $Limit = New-Object LabProcess+JOBOBJECT_BASIC_LIMIT_INFORMATION
    $Limit.LimitFlags = 0x10  # JOB_OBJECT_LIMIT_AFFINITY
    $Limit.Affinity = [UIntPtr][uint64]$Mask
    $JobObjectBasicLimitInformation = 2
    $Size = [Runtime.InteropServices.Marshal]::SizeOf([type][LabProcess+JOBOBJECT_BASIC_LIMIT_INFORMATION])
    if (-not [LabProcess]::SetInformationJobObject($Job, $JobObjectBasicLimitInformation, [ref]$Limit, $Size)) {
        throw "SetInformationJobObject(affinity $Mask) failed (error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))."
    }
    if (-not [LabProcess]::AssignProcessToJobObject($Job, $Process.Handle)) {
        throw "AssignProcessToJobObject failed for pid $($Process.Id) (error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))."
    }
    Write-Host "JOB: pid $($Process.Id) is limited to affinity $Mask"
}

function Disable-LabTimerThrottle($Process) {
    $ProcessPowerThrottling = 4
    $State = New-Object LabProcess+PROCESS_POWER_THROTTLING_STATE
    $State.Version = 1
    # ControlMask에 넣고 StateMask에서 뺀 항목은 "항상 끔"이다. 0x4 = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION.
    $State.ControlMask = 0x4
    $State.StateMask = 0
    if (-not [LabProcess]::SetProcessInformation($Process.Handle, $ProcessPowerThrottling, [ref]$State, 12)) {
        throw "SetProcessInformation(ProcessPowerThrottling) failed for pid $($Process.Id) (error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))."
    }
    Write-Host "TIMER: pid $($Process.Id) keeps its requested timer resolution"
}

# 서버와 클라이언트의 실행 인자 가운데 run-scenario.ps1과 run-manual.ps1이 함께 쓰는 부분.
# 실행마다 다른 인자는 돌려받은 배열 뒤에 붙인다.
function Get-LabServerArgs([string]$LogName, [int]$Nodes, [int]$Npcs) {
    return @(
        "`"$Project`"", "/Game/Maps/L_Lab", "-server", "-log", "-DisablePython",
        "-LOG=$LogName", "-LabNodes=$Nodes", "-LabNpcs=$Npcs"
    )
}

function Get-LabClientArgs([int]$Slot, [string]$Label, [string]$LogName, [int]$ResX, [int]$ResY, [int]$WinX, [int]$WinY) {
    return @(
        "`"$Project`"", "127.0.0.1", "-game", "-windowed",
        "-ResX=$ResX", "-ResY=$ResY", "-WinX=$WinX", "-WinY=$WinY",
        # -log는 로그 콘솔 창을 띄울 뿐이다(LaunchEngineLoop.cpp의 "Show log if wanted"). 로그 파일은 -LOG=만으로 남는다.
        "-LOG=$LogName", "-nosound", "-DisablePython",
        "-LabSlot=$Slot", "-LabLabel=$Label"
    )
}
