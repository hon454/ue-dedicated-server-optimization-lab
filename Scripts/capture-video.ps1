# 화면을 녹화해 MP4, GIF, WebP로 저장한다. 포스팅에 넣을 클라이언트 영상에 쓴다.
#   powershell -ExecutionPolicy Bypass -File Scripts/capture-video.ps1 -Seconds 10 -Out Posts/00-testbed/images/clip.mp4 -Window "DSOptLab*"
#   -Window <제목 패턴> : 제목이 패턴에 맞는 창을 모두 감싸는 영역을 찍는다(클라이언트 여러 개를 한 화면에).
#   -Region "x,y,w,h"  : 화면 좌표의 영역을 찍는다. -Window와 -Region을 모두 빼면 주 모니터 전체를 찍는다.
#   -Out의 확장자로 형식을 정한다. .gif와 .webp는 MP4로 먼저 찍은 뒤 -GifFps, -Width로 줄여 변환한다.
#   -Delay N 을 주면 N초 기다렸다가 찍는다. -NoMouse는 커서를 빼고 찍는다.
#
# 화면 복제 API(ddagrab)로 받아 GPU 인코더(h264_nvenc)로 바로 넣는다. CPU를 거의 쓰지 않지만 0은 아니고,
# GIF/WebP 변환은 녹화가 끝난 뒤 CPU로 한다. 그래서 측정 중인 서버(-LabMeasure)가 떠 있으면 찍지 않는다.
# 화면을 받으므로 창 위에 겹친 다른 창도 함께 찍힌다. -Window로 고른 창은 녹화하는 동안 맨 위로 올린다.
# -Region이나 전체 화면은 그대로 찍으니 찍을 창을 앞에 두고 실행한다.
#
# 찍은 뒤 6프레임을 한 장에 모은 미리보기를 Saved/Screenshots/Lab/<이름>-preview.png 에 남긴다.
# 에이전트는 영상을 직접 볼 수 없어서 이 미리보기를 열어 제대로 찍혔는지 확인한다.

param(
    [Parameter(Mandatory = $true)][string]$Out,
    [double]$Seconds = 10,
    [string]$Window = "",
    [string]$Region = "",
    [int]$Fps = 30,
    [int]$GifFps = 15,
    [int]$Width = 960,
    [double]$Delay = 0,
    [switch]$NoMouse
)

. "$PSScriptRoot\common.ps1"

# winget으로 설치하면 새 셸부터 PATH에 잡힌다. 지금 셸에서도 찾도록 레지스트리의 PATH를 다시 읽는다.
$Ffmpeg = (Get-Command ffmpeg -ErrorAction SilentlyContinue).Source
if (-not $Ffmpeg) {
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'User') + ';' + [Environment]::GetEnvironmentVariable('Path', 'Machine')
    $Ffmpeg = (Get-Command ffmpeg -ErrorAction SilentlyContinue).Source
}
if (-not $Ffmpeg) {
    throw "ffmpeg not found. Install it with: winget install --id Gyan.FFmpeg -e"
}

# 측정 중인 실행의 수치를 오염시키지 않는다.
$Measuring = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue |
    Where-Object { $_.CommandLine -match '-LabMeasure' })
if ($Measuring.Count -gt 0) {
    throw "A measuring server is running (pid $($Measuring.ProcessId -join ', ')). Record with Scripts/run-manual.ps1 instead."
}

Add-Type @"
using System; using System.Runtime.InteropServices;
public static class LabVideo {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern int GetSystemMetrics(int index);
    // 창 테두리 바깥의 보이지 않는 크기 조절 영역을 뺀, 화면에 보이는 창의 영역.
    [DllImport("dwmapi.dll")] public static extern int DwmGetWindowAttribute(IntPtr hWnd, int attr, out RECT rect, int size);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int cx, int cy, uint flags);
}
"@
# 150% 배율에서도 실제 픽셀 좌표로 받는다. ddagrab도 실제 픽셀 좌표를 쓴다.
[LabVideo]::SetProcessDPIAware() | Out-Null
$ScreenW = [LabVideo]::GetSystemMetrics(0)
$ScreenH = [LabVideo]::GetSystemMetrics(1)

$Targets = @()
if ($Window) {
    $Targets = @(Get-Process | Where-Object { $_.MainWindowHandle -ne 0 -and $_.MainWindowTitle -like $Window })
    if ($Targets.Count -eq 0) {
        throw "No window title matches '$Window'."
    }
    $L = [int]::MaxValue; $T = [int]::MaxValue; $R = [int]::MinValue; $B = [int]::MinValue
    foreach ($P in $Targets) {
        $Rect = New-Object LabVideo+RECT
        $DWMWA_EXTENDED_FRAME_BOUNDS = 9
        [LabVideo]::DwmGetWindowAttribute($P.MainWindowHandle, $DWMWA_EXTENDED_FRAME_BOUNDS, [ref]$Rect, 16) | Out-Null
        Write-Host ("WINDOW: pid {0} '{1}' {2},{3} {4}x{5}" -f $P.Id, $P.MainWindowTitle, $Rect.Left, $Rect.Top, ($Rect.Right - $Rect.Left), ($Rect.Bottom - $Rect.Top))
        $L = [math]::Min($L, $Rect.Left); $T = [math]::Min($T, $Rect.Top)
        $R = [math]::Max($R, $Rect.Right); $B = [math]::Max($B, $Rect.Bottom)
    }
}
elseif ($Region) {
    $V = $Region.Split(',') | ForEach-Object { [int]$_.Trim() }
    if ($V.Count -ne 4) { throw "-Region must be 'x,y,w,h'." }
    $L = $V[0]; $T = $V[1]; $R = $V[0] + $V[2]; $B = $V[1] + $V[3]
}
else {
    $L = 0; $T = 0; $R = $ScreenW; $B = $ScreenH
}

# 주 모니터 안으로 자르고, H.264가 요구하는 짝수 크기로 맞춘다.
$L = [math]::Max(0, $L); $T = [math]::Max(0, $T)
$R = [math]::Min($ScreenW, $R); $B = [math]::Min($ScreenH, $B)
$W = ($R - $L) - (($R - $L) % 2)
$H = ($B - $T) - (($B - $T) % 2)
if ($W -le 0 -or $H -le 0) {
    throw "Capture area is outside the primary monitor."
}

$OutPath = if ([System.IO.Path]::IsPathRooted($Out)) { $Out } else { Join-Path $ProjectDir $Out }
$Ext = [System.IO.Path]::GetExtension($OutPath).ToLowerInvariant()
if ($Ext -notin @('.mp4', '.gif', '.webp')) {
    throw "-Out must end with .mp4, .gif or .webp."
}
New-Item -ItemType Directory -Force (Split-Path $OutPath) | Out-Null
$Mp4Path = if ($Ext -eq '.mp4') { $OutPath } else { Join-Path $env:TEMP ("lab-capture-{0}.mp4" -f [guid]::NewGuid()) }

if ($Delay -gt 0) { Start-Sleep -Seconds $Delay }

$DrawMouse = if ($NoMouse) { 0 } else { 1 }
$Grab = "ddagrab=output_idx=0:framerate=${Fps}:draw_mouse=${DrawMouse}:offset_x=${L}:offset_y=${T}:video_size=${W}x${H}"
Write-Host ("RECORD: {0},{1} {2}x{3} for {4}s at {5} fps" -f $L, $T, $W, $H, $Seconds, $Fps)
# -Window로 고른 창은 녹화하는 동안만 항상 위에 두고 끝나면 되돌린다.
# 백그라운드 프로세스는 SetForegroundWindow로 창을 앞으로 가져올 수 없어서 TOPMOST를 쓴다.
$HWND_TOPMOST = [IntPtr](-1); $HWND_NOTOPMOST = [IntPtr](-2)
$SWP_NOSIZE_NOMOVE_NOACTIVATE = 0x0001 -bor 0x0002 -bor 0x0010
try {
    foreach ($P in $Targets) {
        [LabVideo]::SetWindowPos($P.MainWindowHandle, $HWND_TOPMOST, 0, 0, 0, 0, $SWP_NOSIZE_NOMOVE_NOACTIVATE) | Out-Null
    }
    # NVENC가 화면의 BGRA를 YUV 4:2:0(브라우저가 재생하는 형식)으로 바꿔 넣는다.
    & $Ffmpeg -hide_banner -loglevel error -y -f lavfi -i $Grab -t $Seconds `
        -c:v h264_nvenc -preset p5 -tune hq -rc vbr -cq 23 -b:v 0 `
        -movflags +faststart $Mp4Path
}
finally {
    foreach ($P in $Targets) {
        [LabVideo]::SetWindowPos($P.MainWindowHandle, $HWND_NOTOPMOST, 0, 0, 0, 0, $SWP_NOSIZE_NOMOVE_NOACTIVATE) | Out-Null
    }
}
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Mp4Path)) {
    throw "ffmpeg recording failed (exit $LASTEXITCODE)."
}

$Scale = "scale='min($Width,iw)':-2:flags=lanczos"
if ($Ext -eq '.gif') {
    # 팔레트를 영상에서 뽑아 쓰면 256색으로 줄일 때 색 띠가 덜 생긴다.
    & $Ffmpeg -hide_banner -loglevel error -y -i $Mp4Path `
        -vf "fps=$GifFps,$Scale,split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=sierra2_4a" `
        -loop 0 $OutPath
}
elseif ($Ext -eq '.webp') {
    & $Ffmpeg -hide_banner -loglevel error -y -i $Mp4Path `
        -vf "fps=$GifFps,$Scale" -c:v libwebp_anim -quality 75 -loop 0 $OutPath
}
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $OutPath)) {
    throw "ffmpeg conversion to $Ext failed (exit $LASTEXITCODE)."
}

# 미리보기: 영상 전체에서 고르게 6프레임을 뽑아 3x2로 붙인다.
$PreviewDir = "$ProjectDir\Saved\Screenshots\Lab"
New-Item -ItemType Directory -Force $PreviewDir | Out-Null
$PreviewPath = Join-Path $PreviewDir ("{0}-preview.png" -f [System.IO.Path]::GetFileNameWithoutExtension($OutPath))
$Step = [math]::Max(1, [int]($Seconds * $Fps / 6))
& $Ffmpeg -hide_banner -loglevel error -y -i $Mp4Path `
    -vf "select='not(mod(n\,$Step))',scale=640:-2,tile=3x2" -frames:v 1 -fps_mode passthrough $PreviewPath

if ($Mp4Path -ne $OutPath) { Remove-Item $Mp4Path -ErrorAction SilentlyContinue }

$SizeMB = (Get-Item $OutPath).Length / 1MB
Write-Host ("SAVED: {0} ({1:N2} MB)" -f $OutPath, $SizeMB)
Write-Host ("PREVIEW: {0}" -f $PreviewPath)
