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
