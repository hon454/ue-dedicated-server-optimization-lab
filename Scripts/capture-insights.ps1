# 열려 있는 Unreal Insights 창의 내용만 PNG로 저장한다. 문서에 넣을 Insights 캡처에 쓴다.
#   powershell -ExecutionPolicy Bypass -File Scripts/capture-insights.ps1 -Label baseline3-r1 -Out Posts/01-baseline/images/insights-r1-timers.png
#   -Height N 을 주면 창의 위쪽 N픽셀만 저장한다(Networking 탭처럼 아래가 비는 화면).
#
# 화면을 복사하지 않고 창이 직접 그린 내용을 받는다(PrintWindow, PW_RENDERFULLCONTENT).
# 화면 복사에는 에이전트가 화면을 조작하는 동안 화면 가장자리에 그려지는 주황 테두리와 창 위에 겹친 다른 창이 들어간다.
# 툴팁은 별도 창이라서, 같은 프로세스의 보이는 창(툴팁, 드롭다운)을 각각 받아 같은 자리에 겹쳐 그린다.

param(
    [Parameter(Mandatory = $true)][string]$Label,
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$Height = 0
)

. "$PSScriptRoot\common.ps1"

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class LabCapture {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
    public delegate bool EnumProc(IntPtr hWnd, IntPtr lParam);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc proc, IntPtr lParam);

    // 이 프로세스의 보이는 최상위 창을 위에서 아래 순서로 돌려준다.
    public static System.Collections.Generic.List<IntPtr> VisibleWindows(uint processId) {
        var result = new System.Collections.Generic.List<IntPtr>();
        EnumWindows((h, l) => {
            uint pid;
            GetWindowThreadProcessId(h, out pid);
            if (pid == processId && IsWindowVisible(h)) { result.Add(h); }
            return true;
        }, IntPtr.Zero);
        return result;
    }
}
"@
# 150% 배율에서도 실제 픽셀 크기로 받는다.
[LabCapture]::SetProcessDPIAware() | Out-Null

$Process = Get-Process UnrealInsights -ErrorAction SilentlyContinue |
    Where-Object { $_.MainWindowTitle -like "$Label - *" } | Select-Object -First 1
if (-not $Process) {
    throw "Unreal Insights window for '$Label' not found. Open it with Scripts/open-insights.ps1 first."
}

$Rect = New-Object LabCapture+RECT
[LabCapture]::GetWindowRect($Process.MainWindowHandle, [ref]$Rect) | Out-Null
$Width = $Rect.Right - $Rect.Left
$FullHeight = $Rect.Bottom - $Rect.Top

$PW_RENDERFULLCONTENT = 2
function Get-WindowImage([IntPtr]$Handle, [int]$W, [int]$H) {
    $Image = New-Object System.Drawing.Bitmap $W, $H
    $G = [System.Drawing.Graphics]::FromImage($Image)
    $Hdc = $G.GetHdc()
    $Ok = [LabCapture]::PrintWindow($Handle, $Hdc, $PW_RENDERFULLCONTENT)
    $G.ReleaseHdc($Hdc)
    $G.Dispose()
    if (-not $Ok) { $Image.Dispose(); return $null }
    return $Image
}

$Bitmap = Get-WindowImage $Process.MainWindowHandle $Width $FullHeight
if (-not $Bitmap) {
    throw "PrintWindow failed."
}

# 툴팁 같은 다른 창을 아래 창부터 겹쳐 그린다.
$Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
$Others = @([LabCapture]::VisibleWindows([uint32]$Process.Id) | Where-Object { $_ -ne $Process.MainWindowHandle })
if ($Others.Count -gt 1) { [array]::Reverse($Others) }
foreach ($Handle in $Others) {
    $R = New-Object LabCapture+RECT
    [LabCapture]::GetWindowRect($Handle, [ref]$R) | Out-Null
    $W = $R.Right - $R.Left; $H = $R.Bottom - $R.Top
    if ($W -le 0 -or $H -le 0 -or $R.Right -le $Rect.Left -or $R.Left -ge $Rect.Right -or $R.Bottom -le $Rect.Top -or $R.Top -ge $Rect.Bottom) { continue }
    $Image = Get-WindowImage $Handle $W $H
    if ($Image) {
        $Graphics.DrawImage($Image, $R.Left - $Rect.Left, $R.Top - $Rect.Top, $W, $H)
        $Image.Dispose()
        Write-Host ("OVERLAY: {0}x{1} at {2},{3}" -f $W, $H, ($R.Left - $Rect.Left), ($R.Top - $Rect.Top))
    }
}
$Graphics.Dispose()

if ($Height -gt 0 -and $Height -lt $FullHeight) {
    $Cropped = $Bitmap.Clone((New-Object System.Drawing.Rectangle 0, 0, $Width, $Height), $Bitmap.PixelFormat)
    $Bitmap.Dispose()
    $Bitmap = $Cropped
}

$OutPath = if ([System.IO.Path]::IsPathRooted($Out)) { $Out } else { Join-Path $ProjectDir $Out }
$Bitmap.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
Write-Host ("SAVED: {0} ({1}x{2})" -f $OutPath, $Bitmap.Width, $Bitmap.Height)
$Bitmap.Dispose()