. "$PSScriptRoot\common.ps1"
& "$UE_ROOT\Engine\Build\BatchFiles\Build.bat" DSOptLabEditor Win64 Development "-project=$Project" -waitmutex
exit $LASTEXITCODE
