# 이미지에 번호 붙은 외곽선 상자를 그려 새 파일로 저장한다. 포스팅의 Insights 이미지에서 본문이 인용하는 값을 가리킬 때 쓴다.
#   powershell -ExecutionPolicy Bypass -File Scripts/annotate-image.ps1 -In Posts/01-baseline/images/insights-r1-timers.png -Out Posts/01-baseline/images/timing.png -Boxes "1953,1532,1040,30;1953,1562,1040,50"
#
# -Boxes는 "x,y,w,h"를 세미콜론으로 이은 것이고 원본 픽셀 좌표다(capture-insights.ps1의 캡처는 3000×2080).
# 상자는 최대 3개이고 노란색 4px 외곽선만 그린다(채우기 없음). 주황은 Insights의 빨강, 갈색 행 바탕에 묻혀서 노랑으로 바꿨다. 선은 상자 안쪽으로 그려서 상자끼리 맞닿아도 겹치지 않는다.
# 번호(①②③)는 상자 왼쪽 위 모서리에 붙인다. 상자 왼쪽에 자리가 있으면 왼쪽 바깥, 없으면 위 바깥, 둘 다 없으면 안쪽에 둔다.
# 원본은 고치지 않는다. -Out이 -In과 같으면 멈춘다.

param(
    [Parameter(Mandatory = $true)][string]$In,
    [Parameter(Mandatory = $true)][string]$Out,
    [Parameter(Mandatory = $true)][string]$Boxes
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$InPath = (Resolve-Path $In).Path
# 상대 경로는 현재 위치 기준으로 푼다. 절대 경로는 그대로 쓴다.
$OutPath = if ([System.IO.Path]::IsPathRooted($Out)) { [System.IO.Path]::GetFullPath($Out) } else { [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Out)) }
if ($InPath -eq $OutPath) {
    throw "-Out must differ from -In. The original image is kept as is."
}

$Rects = @()
foreach ($Part in ($Boxes -split ';' | Where-Object { $_.Trim() })) {
    $N = @($Part.Split(',') | ForEach-Object { [int]$_.Trim() })
    if ($N.Count -ne 4) { throw "Box '$Part' must be x,y,w,h." }
    $Rects += , (New-Object System.Drawing.Rectangle $N[0], $N[1], $N[2], $N[3])
}
if ($Rects.Count -lt 1 -or $Rects.Count -gt 3) {
    throw "Give 1 to 3 boxes (got $($Rects.Count))."
}

# 원본 파일을 잠그지 않도록 메모리로 복사해 연다.
$Source = [System.Drawing.Image]::FromFile($InPath)
$Bitmap = New-Object System.Drawing.Bitmap $Source.Width, $Source.Height
$G = [System.Drawing.Graphics]::FromImage($Bitmap)
$G.DrawImage($Source, 0, 0, $Source.Width, $Source.Height)
$Source.Dispose()

$G.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$G.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit

$Mark = [System.Drawing.Color]::FromArgb(255, 255, 235, 0)
$Pen = New-Object System.Drawing.Pen $Mark, 4
$Pen.Alignment = [System.Drawing.Drawing2D.PenAlignment]::Inset
$LabelBrush = New-Object System.Drawing.SolidBrush $Mark
# 노랑 바탕에서는 흰 글자가 읽히지 않는다.
$TextBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::Black)
$Font = New-Object System.Drawing.Font 'Segoe UI', 22, ([System.Drawing.FontStyle]::Bold), ([System.Drawing.GraphicsUnit]::Pixel)
$Format = New-Object System.Drawing.StringFormat
$Format.Alignment = [System.Drawing.StringAlignment]::Center
$Format.LineAlignment = [System.Drawing.StringAlignment]::Center

$LabelSize = 34
for ($i = 0; $i -lt $Rects.Count; $i++) {
    $R = $Rects[$i]
    $G.DrawRectangle($Pen, $R)

    if ($R.X -ge $LabelSize) {
        $LX = $R.X - $LabelSize; $LY = $R.Y
    } elseif ($R.Y -ge $LabelSize) {
        $LX = $R.X; $LY = $R.Y - $LabelSize
    } else {
        $LX = $R.X; $LY = $R.Y
    }
    $Label = New-Object System.Drawing.RectangleF $LX, $LY, $LabelSize, $LabelSize
    $G.FillRectangle($LabelBrush, $Label)
    # ① = U+2460
    $G.DrawString([string][char](0x2460 + $i), $Font, $TextBrush, $Label, $Format)
}

$G.Dispose()
$Bitmap.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
$Bitmap.Dispose()
Write-Host "Saved $OutPath ($($Rects.Count) boxes)"
