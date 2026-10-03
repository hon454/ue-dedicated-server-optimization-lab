# Relevancy 포스팅의 움직이는 도식(서버가 보는 지도)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-relevancy-map.ps1
# 결과:   Posts/02-relevancy/images/relevancy-map.svg
#
# 실제 비율로 그리는 것: 바닥 2km, 배치 영역 1.9km(WorldHalfExtent 95,000cm), 플레이어 자리(반지름 500m 원의 16자리 중 0~7번),
#   이동 경로(한 변 100m 정사각형), Net Cull Distance 150m.
# 예시로 그리는 것: 자원 노드와 NPC의 위치(이 스크립트의 고정 시드로 고르게 뿌린다. 게임의 배치와 같지 않다).
# 재생 속도: 걷기 500cm/s로 400m 한 바퀴가 80초이고, 도식은 10초에 한 바퀴(8배속)다.

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\02-relevancy\images\relevancy-map.svg')
)

$ErrorActionPreference = 'Stop'
$Inv = [cultureinfo]::InvariantCulture

# 1px = 5m. 바닥 2km가 400px이다.
$MapPx = 400.0
$MetersPerPx = 5.0
function ToPx([double]$Meters) { return ($MapPx / 2.0) + ($Meters / $MetersPerPx) }
function Fmt([double]$Value) { return $Value.ToString('0.#', $Inv) }

$Rng = [System.Random]::new(20261001)
function DotPath([int]$Count) {
	$Builder = [System.Text.StringBuilder]::new()
	for ($i = 0; $i -lt $Count; $i++) {
		$x = ToPx (($Rng.NextDouble() * 1900.0) - 950.0)
		$y = ToPx (($Rng.NextDouble() * 1900.0) - 950.0)
		[void]$Builder.Append("M$(Fmt $x) $(Fmt $y)h0")
	}
	return $Builder.ToString()
}
$NodePath = DotPath 5000
$NpcPath = DotPath 300

$CullPx = 150.0 / $MetersPerPx
$SidePx = 100.0 / $MetersPerPx
$Anim = 'keyTimes="0;.25;.5;.75;1" dur="10s" repeatCount="indefinite"'

$ClipCircles = [System.Text.StringBuilder]::new()
$Players = [System.Text.StringBuilder]::new()
$PlayersNoRange = [System.Text.StringBuilder]::new()
for ($Slot = 0; $Slot -lt 8; $Slot++) {
	$Angle = 2.0 * [Math]::PI * $Slot / 16.0
	$hx = ToPx ([Math]::Cos($Angle) * 500.0)
	$hy = ToPx ([Math]::Sin($Angle) * 500.0)
	$x0 = Fmt $hx; $y0 = Fmt $hy
	$x1 = Fmt ($hx + $SidePx); $y1 = Fmt ($hy + $SidePx)
	$r = Fmt $CullPx

	if ($Slot -eq 0) {
		# 0번은 제자리에서 채집한다.
		[void]$ClipCircles.AppendLine("<circle cx=`"$x0`" cy=`"$y0`" r=`"$r`"/>")
		[void]$Players.AppendLine("<g transform=`"translate($x0 $y0)`"><circle r=`"$r`" class=`"range`"/><circle r=`"3`" class=`"player`"/></g>")
		[void]$PlayersNoRange.AppendLine("<circle cx=`"$x0`" cy=`"$y0`" r=`"3`" class=`"player`"/>")
		continue
	}

	$Move = "<animateTransform attributeName=`"transform`" type=`"translate`" values=`"$x0 $y0;$x1 $y0;$x1 $y1;$x0 $y1;$x0 $y0`" $Anim/>"
	[void]$ClipCircles.AppendLine("<circle cx=`"$x0`" cy=`"$y0`" r=`"$r`"><animate attributeName=`"cx`" values=`"$x0;$x1;$x1;$x0;$x0`" $Anim/><animate attributeName=`"cy`" values=`"$y0;$y0;$y1;$y1;$y0`" $Anim/></circle>")
	[void]$Players.AppendLine("<g transform=`"translate($x0 $y0)`">$Move<circle r=`"$r`" class=`"range`"/><circle r=`"3`" class=`"player`"/></g>")
	[void]$PlayersNoRange.AppendLine("<g transform=`"translate($x0 $y0)`">$Move<circle r=`"3`" class=`"player`"/></g>")
}

$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 520" width="900" height="520" role="img" aria-label="서버가 보는 지도. 적용 전에는 모든 액터를 모든 클라이언트에게 보내고, 적용 후에는 플레이어 주변 150m 원 안의 액터만 보낸다.">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.title{font-size:17px;font-weight:700}
.sub{font-size:13px;fill:#9aa7b5}
.legend{font-size:13px;fill:#c9d4df}
.map{fill:#151b24;stroke:#2b3442}
.dots{fill:none;stroke-linecap:round}
.off{stroke:#465061;stroke-width:1.5}
.lit .node{stroke-width:2.3}
.node{stroke:#3fb950;stroke-width:1.7}
.npc{stroke:#ff6b61;stroke-width:3.2}
.range{fill:none;stroke:#e6edf3;stroke-width:1;stroke-opacity:.85}
.player{fill:#ffffff;stroke:#0f141b;stroke-width:1}
</style>
<defs>
<path id="nodes" d="$NodePath"/>
<path id="npcs" d="$NpcPath"/>
<clipPath id="inRange">
$($ClipCircles.ToString())</clipPath>
</defs>
<rect width="900" height="520" rx="10" fill="#0f141b"/>

<text x="30" y="34" class="title">적용 전: Always Relevant</text>
<text x="30" y="54" class="sub">서버가 모든 액터를 클라이언트 8개 모두에게 보낸다</text>
<g transform="translate(30 68)">
<rect width="400" height="400" class="map"/>
<use href="#nodes" class="dots node"/>
<use href="#npcs" class="dots npc"/>
$($PlayersNoRange.ToString())</g>

<text x="470" y="34" class="title">적용 후: Net Cull Distance 150m</text>
<text x="470" y="54" class="sub">서버가 원 안의 액터만 그 클라이언트에게 보낸다</text>
<g transform="translate(470 68)">
<rect width="400" height="400" class="map"/>
<use href="#nodes" class="dots off"/>
<use href="#npcs" class="dots off"/>
<g clip-path="url(#inRange)" class="lit">
<use href="#nodes" class="dots node"/>
<use href="#npcs" class="dots npc"/>
</g>
$($Players.ToString())</g>

<g transform="translate(30 497)" class="legend">
<circle cx="5" cy="-4" r="4" fill="#3fb950"/><text x="16" y="0" class="legend">보내는 자원 노드</text>
<circle cx="145" cy="-4" r="4" fill="#ff6b61"/><text x="156" y="0" class="legend">보내는 NPC</text>
<circle cx="255" cy="-4" r="4" fill="#465061"/><text x="266" y="0" class="legend">보내지 않는 액터</text>
<circle cx="395" cy="-4" r="4" fill="#ffffff"/><text x="406" y="0" class="legend">플레이어</text>
<text x="490" y="0" class="legend">맵 2km × 2km, 8배속. 점의 위치는 예시다</text>
</g>
</svg>
"@

$OutPath = [System.IO.Path]::GetFullPath($Out)
[System.IO.File]::WriteAllText($OutPath, $Svg, [System.Text.UTF8Encoding]::new($false))
Write-Host "wrote $OutPath ($([Math]::Round((Get-Item $OutPath).Length / 1KB)) KB)"
