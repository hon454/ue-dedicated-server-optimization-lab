# NPC 보간 포스팅의 원리 도식(버퍼 보간)을 움직이는 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-interp-timeline.ps1
# 결과:   Posts/11-npc-interpolation/images/interp-timeline.svg
#
# 예시로 그린다: 갱신 간격 133.3ms(서버 30Hz에서 4프레임, Net Update Frequency 10), 보간 지연 150ms, 전달 시간 0.
#   NPC가 일정한 속도로 움직이므로 위치를 시각과 같은 가로축에 놓는다. 서버 시각 800ms를 5초에 재생한다.
#   "보간 없음"의 점은 마지막으로 받은 위치에 있고, "150ms 보간"의 점은 "지금 - 150ms"를 끼는 두 위치 사이에 있다.

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\11-npc-interpolation\images\interp-timeline.svg')
)

$ErrorActionPreference = 'Stop'
$Inv = [Globalization.CultureInfo]::InvariantCulture
function F([double]$V) { return $V.ToString('0.###', $Inv) }

$Span = 800.0        # 그리는 서버 시각(ms)
$Dur = 5             # 재생 시간(초)
$Interval = 400.0 / 3 # 갱신 간격(ms)
$Delay = 150.0
$X0 = 170.0; $Scale = 0.9  # x = X0 + 서버 시각(ms) × Scale
function Xt([double]$Ms) { return $X0 + $Ms * $Scale }
function Kt([double]$Ms) { return F ($Ms / $Span) }

$RowAxis = 96; $RowServer = 160; $RowSnap = 214; $RowInterp = 268
$Top = 62; $Bottom = 292

$Snaps = @()
for ($k = 0; $k * $Interval -le $Span + 0.01; $k++) { $Snaps += $k * $Interval }

$Body = @()
# 서버 프레임 눈금(33.3ms)
for ($Ms = 0.0; $Ms -le $Span + 0.01; $Ms += 100.0 / 3) {
	$Body += "<line x1=`"$(F (Xt $Ms))`" y1=`"$($RowAxis - 4)`" x2=`"$(F (Xt $Ms))`" y2=`"$($RowAxis + 4)`" class=`"frame`"/>"
}
$Body += "<line x1=`"$(F (Xt 0))`" y1=`"$RowAxis`" x2=`"$(F (Xt $Span))`" y2=`"$RowAxis`" class=`"axis`"/>"
for ($Row = 0; $Row -lt 3; $Row++) {
	$Y = @($RowServer, $RowSnap, $RowInterp)[$Row]
	$Body += "<line x1=`"$(F (Xt 0))`" y1=`"$Y`" x2=`"$(F (Xt $Span))`" y2=`"$Y`" class=`"track`"/>"
}

# 보간하는 두 위치 사이(지금 - 150ms를 끼는 구간)
$Keys = @('0'); $Vals = @(F (Xt 0))
for ($k = 1; $Delay + $k * $Interval -le $Span; $k++) { $Keys += Kt ($Delay + $k * $Interval); $Vals += F (Xt ($k * $Interval)) }
$Body += "<rect y=`"$($RowAxis - 9)`" width=`"$(F ($Interval * $Scale))`" height=`"18`" rx=`"4`" class=`"pair`" x=`"$(F (Xt 0))`"><animate attributeName=`"x`" values=`"$($Vals -join ';')`" keyTimes=`"$($Keys -join ';')`" calcMode=`"discrete`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/></rect>"

# 받은 위치: 지금이 지나가면 나타난다
foreach ($Ms in $Snaps) {
	$Anim = if ($Ms -gt 0) { "<animate attributeName=`"opacity`" values=`"0;1`" keyTimes=`"0;$(Kt $Ms)`" calcMode=`"discrete`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/>" } else { '' }
	$Body += "<circle cx=`"$(F (Xt $Ms))`" cy=`"$RowAxis`" r=`"6`" class=`"snap`">$Anim</circle>"
}

# 지금과 그리는 순간의 세로선
$NowAnim = "<animate attributeName=`"x1`" from=`"$(F (Xt 0))`" to=`"$(F (Xt $Span))`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/><animate attributeName=`"x2`" from=`"$(F (Xt 0))`" to=`"$(F (Xt $Span))`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/>"
$Body += "<line x1=`"$(F (Xt 0))`" y1=`"$Top`" x2=`"$(F (Xt 0))`" y2=`"$Bottom`" class=`"now`">$NowAnim</line>"
$RenderVals = "$(F (Xt 0));$(F (Xt 0));$(F (Xt ($Span - $Delay)))"
$RenderKeys = "0;$(Kt $Delay);1"
$RenderAnim = "<animate attributeName=`"x1`" values=`"$RenderVals`" keyTimes=`"$RenderKeys`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/><animate attributeName=`"x2`" values=`"$RenderVals`" keyTimes=`"$RenderKeys`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/>"
$Body += "<line x1=`"$(F (Xt 0))`" y1=`"$Top`" x2=`"$(F (Xt 0))`" y2=`"$Bottom`" class=`"render`">$RenderAnim</line>"

# 세 NPC
$Body += "<circle cx=`"$(F (Xt 0))`" cy=`"$RowServer`" r=`"9`" class=`"npc server`"><animate attributeName=`"cx`" from=`"$(F (Xt 0))`" to=`"$(F (Xt $Span))`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/></circle>"
$SnapKeys = ($Snaps | Where-Object { $_ -lt $Span } | ForEach-Object { Kt $_ }) -join ';'
$SnapVals = ($Snaps | Where-Object { $_ -lt $Span } | ForEach-Object { F (Xt $_) }) -join ';'
$Body += "<circle cx=`"$(F (Xt 0))`" cy=`"$RowSnap`" r=`"9`" class=`"npc`"><animate attributeName=`"cx`" values=`"$SnapVals`" keyTimes=`"$SnapKeys`" calcMode=`"discrete`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/></circle>"
$Body += "<circle cx=`"$(F (Xt 0))`" cy=`"$RowInterp`" r=`"9`" class=`"npc`"><animate attributeName=`"cx`" values=`"$RenderVals`" keyTimes=`"$RenderKeys`" dur=`"${Dur}s`" repeatCount=`"indefinite`"/></circle>"

$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 330" width="900" height="330" role="img" aria-label="버퍼 보간. 클라이언트는 받은 위치를 쌓아 두고, 지금보다 150ms 앞선 순간을 끼는 두 위치 사이를 이어 그린다. 보간 없음의 NPC는 받을 때마다 건너뛰고, 150ms 보간의 NPC는 서버의 NPC와 같은 속도로 늦게 따라간다.">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.bg{fill:#0f141b}
.title{font-size:17px;font-weight:700}
.label{font-size:14px;fill:#c9d4df}
.note{font-size:12px;fill:#9aa7b5}
.axis{stroke:#7d8896;stroke-width:1.5}
.frame{stroke:#4a5563;stroke-width:1}
.track{stroke:#2b3442;stroke-width:1}
.snap{fill:#f08c00}
.pair{fill:#1f4d5c;stroke:#3bc9db;stroke-width:1.5}
.now{stroke:#f08c00;stroke-width:2}
.render{stroke:#3bc9db;stroke-width:2;stroke-dasharray:6 4}
.npc{fill:#ff6b61}
.server{fill:#9aa7b5}
.leg{font-size:13px}
</style>
<rect width="900" height="330" class="bg"/>
<text x="20" y="32" class="title">받은 위치 두 개 사이에서 150ms 전 순간을 그린다</text>
<line x1="560" y1="44" x2="590" y2="44" class="now"/><text x="596" y="49" class="leg">지금</text>
<line x1="650" y1="44" x2="680" y2="44" class="render"/><text x="686" y="49" class="leg">그리는 순간(지금 - 150ms)</text>
<text x="20" y="$($RowAxis + 5)" class="label">받은 위치</text>
<text x="20" y="$($RowServer + 5)" class="label">서버의 NPC</text>
<text x="20" y="$($RowSnap + 5)" class="label">보간 없음</text>
<text x="20" y="$($RowInterp + 5)" class="label">150ms 보간</text>
$($Body -join "`n")
<text x="20" y="318" class="note">예시: 갱신 간격 133ms(서버 프레임 4개), 전달 시간 0. 가로축은 서버 시각이자 NPC의 위치다(일정한 속도). 실제보다 약 6배 느리게 재생한다.</text>
</svg>
"@

New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
[IO.File]::WriteAllText($Out, $Svg, (New-Object Text.UTF8Encoding $false))
Write-Host "SAVED: $Out"
