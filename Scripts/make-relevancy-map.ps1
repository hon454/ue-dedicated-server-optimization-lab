# Relevancy 포스팅의 움직이는 도식(서버가 보는 지도)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-relevancy-map.ps1
# 결과:   Posts/02-relevancy/images/relevancy-map.svg
#
# 실제 비율로 그리는 것: 바닥 2km, 배치 영역 1.9km(WorldHalfExtent 95,000cm), 플레이어 자리(반지름 500m 원의 16자리 중 0~7번),
#   이동 경로(한 변 100m 정사각형), Net Cull Distance 150m.
# 예시로 그리는 것: 자원 노드와 NPC의 위치(이 스크립트의 고정 시드로 고르게 뿌린다. 게임의 배치와 같지 않다).
# 재생 속도: 걷기 500cm/s로 400m 한 바퀴가 80초이고, 도식은 10초에 한 바퀴(8배속)다.
#
# -Act2: 2막의 배치를 맵 전체와 가운데 확대로 나란히 그린다(1막의 세 최적화를 2막에 다시 적용하는 포스팅의 원리 도식).
#   둘 다 거리 판정(Net Cull Distance 150m)을 켠 서버가 보는 지도이고, 같은 액터를 두 축척으로 그린다.
#   powershell -ExecutionPolicy Bypass -File Scripts/make-relevancy-map.ps1 -Act2
#   결과: Posts/06-three-techniques-again/images/relevancy-map-act2.svg
#   2막은 밀집 배치(-PlayerSpacing 3)다. 자리는 대각선 위에 3m 간격, 경로는 한 변 70m 정사각형이고, 짝수 자리는 꼭짓점에서 정방향,
#   홀수 자리는 변의 가운데에서 역방향으로 출발한다(LabGameMode.cpp의 GetSlotLocation과 자리별 출발). 0번은 제자리에서 채집한다.
#   건축물 500개는 무리 중심에서 반지름 80m, 주변 NPC 50개는 40m 안에 놓인다(LabGameMode.h). 왼쪽은 맵 전체를 1px = 5m로,
#   오른쪽은 무리 중심의 400m × 400m를 1px = 1m로 그리고, 왼쪽에 확대한 영역을 점선 사각형으로 표시한다. 70m 정사각형 한 바퀴 280m는 56초이고, 도식은 7초에 한 바퀴(8배속)다.

param(
	[string]$Out = '',
	[switch]$Act2
)
if (-not $Out) {
	$Out = if ($Act2) { Join-Path $PSScriptRoot '..\Posts\06-three-techniques-again\images\relevancy-map-act2.svg' }
		else { Join-Path $PSScriptRoot '..\Posts\02-relevancy\images\relevancy-map.svg' }
}

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

if ($Act2) {
	# 2막만 그린다. 왼쪽은 맵 전체(1px = 5m), 오른쪽은 무리 중심(경로 중심의 평균, 35m, 35m)의 400m × 400m(1px = 1m)다.
	# 같은 액터를 두 축척으로 그린다. 위치는 이 스크립트의 고정 시드로 뿌린 예시다.
	$Center = 35.0
	function ZPx([double]$Meters) { return 200.0 + ($Meters - $Center) }
	function InZoom($P) { return [Math]::Abs($P[0] - $Center) -lt 200 -and [Math]::Abs($P[1] - $Center) -lt 200 }
	$Rng2 = [System.Random]::new(20261005)
	function UniformPoints([int]$Count) {
		$List = [System.Collections.Generic.List[object]]::new()
		for ($i = 0; $i -lt $Count; $i++) { $List.Add(@((($Rng2.NextDouble() * 1900.0) - 950.0), (($Rng2.NextDouble() * 1900.0) - 950.0))) }
		return , $List
	}
	function DiskPoints([int]$Count, [double]$Radius) {
		$List = [System.Collections.Generic.List[object]]::new()
		for ($i = 0; $i -lt $Count; $i++) {
			$d = $Radius * [Math]::Sqrt($Rng2.NextDouble())
			$a = 2.0 * [Math]::PI * $Rng2.NextDouble()
			$List.Add(@(($Center + [Math]::Cos($a) * $d), ($Center + [Math]::Sin($a) * $d)))
		}
		return , $List
	}
	function PathOf($Points, [bool]$Zoom) {
		$Builder = [System.Text.StringBuilder]::new()
		foreach ($P in $Points) {
			if ($Zoom) {
				if (-not (InZoom $P)) { continue }
				[void]$Builder.Append("M$(Fmt (ZPx $P[0])) $(Fmt (ZPx $P[1]))h0")
			}
			else { [void]$Builder.Append("M$(Fmt (ToPx $P[0])) $(Fmt (ToPx $P[1]))h0") }
		}
		return $Builder.ToString()
	}
	$NodesW = UniformPoints 5000
	$NpcsW = UniformPoints 300
	$NpcsW.AddRange((DiskPoints 50 40.0))
	$BuildingsW = DiskPoints 500 80.0

	# 자리마다 경로의 꼭짓점(월드 m)과 keyTimes. 0번은 제자리.
	$Step = 3.0 / [Math]::Sqrt(2.0)
	$Side2 = 70.0
	$Unit = @(@(0, 0), @(1, 0), @(1, 1), @(0, 1))
	$Routes = @()
	for ($Slot = 0; $Slot -lt 8; $Slot++) {
		$o = ($Slot - 3.5) * $Step
		$Corner = { param($c) $u = $Unit[(($c % 4) + 4) % 4]; return , @(($o + $u[0] * $Side2), ($o + $u[1] * $Side2)) }
		if ($Slot -eq 0) { $Routes += , @{ Points = @(, (& $Corner 0)); Times = '' }; continue }
		$q = [Math]::Floor($Slot / 2) % 4
		if ($Slot % 2 -eq 0) {
			$Routes += , @{ Points = @((& $Corner $q), (& $Corner ($q + 1)), (& $Corner ($q + 2)), (& $Corner ($q + 3)), (& $Corner $q)); Times = '0;.25;.5;.75;1' }
		}
		else {
			$a = & $Corner $q; $b = & $Corner ($q + 1)
			$Mid = @((($a[0] + $b[0]) / 2.0), (($a[1] + $b[1]) / 2.0))
			$Routes += , @{ Points = @($Mid, (& $Corner $q), (& $Corner ($q + 3)), (& $Corner ($q + 2)), (& $Corner ($q + 1)), $Mid); Times = '0;.125;.375;.625;.875;1' }
		}
	}
	function PlayersOf([bool]$Zoom, $ClipOut, $PlayersOut) {
		$r = if ($Zoom) { Fmt 150.0 } else { Fmt (150.0 / $MetersPerPx) }
		foreach ($Route in $Routes) {
			$Px = @($Route.Points | ForEach-Object { if ($Zoom) { , @((ZPx $_[0]), (ZPx $_[1])) } else { , @((ToPx $_[0]), (ToPx $_[1])) } })
			$x0 = Fmt $Px[0][0]; $y0 = Fmt $Px[0][1]
			if (-not $Route.Times) {
				[void]$ClipOut.AppendLine("<circle cx=`"$x0`" cy=`"$y0`" r=`"$r`"/>")
				[void]$PlayersOut.AppendLine("<g transform=`"translate($x0 $y0)`"><circle r=`"$r`" class=`"range`"/><circle r=`"3`" class=`"player`"/></g>")
				continue
			}
			$A2 = "keyTimes=`"$($Route.Times)`" dur=`"7s`" repeatCount=`"indefinite`""
			$Xs = ($Px | ForEach-Object { Fmt $_[0] }) -join ';'
			$Ys = ($Px | ForEach-Object { Fmt $_[1] }) -join ';'
			$Ts = ($Px | ForEach-Object { "$(Fmt $_[0]) $(Fmt $_[1])" }) -join ';'
			[void]$ClipOut.AppendLine("<circle cx=`"$x0`" cy=`"$y0`" r=`"$r`"><animate attributeName=`"cx`" values=`"$Xs`" $A2/><animate attributeName=`"cy`" values=`"$Ys`" $A2/></circle>")
			[void]$PlayersOut.AppendLine("<g transform=`"translate($x0 $y0)`"><animateTransform attributeName=`"transform`" type=`"translate`" values=`"$Ts`" $A2/><circle r=`"$r`" class=`"range`"/><circle r=`"3`" class=`"player`"/></g>")
		}
	}
	$ClipFull = [System.Text.StringBuilder]::new(); $PlayersFull = [System.Text.StringBuilder]::new()
	$ClipZoom = [System.Text.StringBuilder]::new(); $PlayersZoom = [System.Text.StringBuilder]::new()
	PlayersOf $false $ClipFull $PlayersFull
	PlayersOf $true $ClipZoom $PlayersZoom

	# 왼쪽 그림의 확대 영역: 월드 (35 ± 200)m를 1px = 5m로.
	$Zx = Fmt (ToPx ($Center - 200.0)); $Zw = Fmt (400.0 / $MetersPerPx)
	$ZRight = 30.0 + (ToPx ($Center + 200.0)); $ZTop = 68.0 + (ToPx ($Center - 200.0)); $ZBottom = 68.0 + (ToPx ($Center + 200.0))

	$Svg2 = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 540" width="900" height="540" role="img" aria-label="거리 판정을 켠 2막의 서버가 보는 지도. 맵 전체에서는 먼 액터가 모두 빠지고 플레이어가 모인 가운데만 남는다. 가운데를 확대하면 건축물 500개가 모두 플레이어의 150m 원 안에 든다.">
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
.building{stroke:#f0a33a;stroke-width:3;stroke-linecap:square}
.range{fill:none;stroke:#e6edf3;stroke-width:1;stroke-opacity:.85}
.player{fill:#ffffff;stroke:#0f141b;stroke-width:1}
.zoom{fill:none;stroke:#e6edf3;stroke-width:1.2;stroke-dasharray:4 3}
</style>
<defs>
<path id="nodesF" d="$(PathOf $NodesW $false)"/>
<path id="npcsF" d="$(PathOf $NpcsW $false)"/>
<path id="buildingsF" d="$(PathOf $BuildingsW $false)"/>
<clipPath id="inRangeF">
$($ClipFull.ToString())</clipPath>
<path id="nodesZ" d="$(PathOf $NodesW $true)"/>
<path id="npcsZ" d="$(PathOf $NpcsW $true)"/>
<path id="buildingsZ" d="$(PathOf $BuildingsW $true)"/>
<clipPath id="inRangeZ">
$($ClipZoom.ToString())</clipPath>
<clipPath id="zoomBox"><rect width="400" height="400"/></clipPath>
</defs>
<rect width="900" height="540" rx="10" fill="#0f141b"/>

<text x="30" y="34" class="title">2막의 맵 전체(2km)</text>
<text x="30" y="54" class="sub">거리 판정: 150m 원 밖의 액터는 처리하지 않는다</text>
<g transform="translate(30 68)">
<rect width="400" height="400" class="map"/>
<use href="#nodesF" class="dots off"/>
<use href="#npcsF" class="dots off"/>
<use href="#buildingsF" class="dots off"/>
<g clip-path="url(#inRangeF)" class="lit">
<use href="#nodesF" class="dots node"/>
<use href="#buildingsF" class="dots building"/>
<use href="#npcsF" class="dots npc"/>
</g>
$($PlayersFull.ToString())<rect x="$Zx" y="$Zx" width="$Zw" height="$Zw" class="zoom"/>
</g>
<line x1="$(Fmt $ZRight)" y1="$(Fmt $ZTop)" x2="470" y2="68" class="zoom"/>
<line x1="$(Fmt $ZRight)" y1="$(Fmt $ZBottom)" x2="470" y2="468" class="zoom"/>

<text x="470" y="34" class="title">가운데 400m 확대</text>
<text x="470" y="54" class="sub">남은 것: 원 안에 건축물 500개가 모두 든다</text>
<g transform="translate(470 68)">
<g clip-path="url(#zoomBox)">
<rect width="400" height="400" class="map"/>
<use href="#nodesZ" class="dots off"/>
<use href="#npcsZ" class="dots off"/>
<use href="#buildingsZ" class="dots off"/>
<g clip-path="url(#inRangeZ)" class="lit">
<use href="#nodesZ" class="dots node"/>
<use href="#buildingsZ" class="dots building"/>
<use href="#npcsZ" class="dots npc"/>
</g>
$($PlayersZoom.ToString())</g>
<rect width="400" height="400" fill="none" stroke="#2b3442"/>
</g>

<g transform="translate(30 497)" class="legend">
<circle cx="5" cy="-4" r="4" fill="#3fb950"/><text x="16" y="0" class="legend">자원 노드</text>
<circle cx="100" cy="-4" r="4" fill="#ff6b61"/><text x="111" y="0" class="legend">NPC</text>
<rect x="161" y="-8" width="8" height="8" fill="#f0a33a"/><text x="176" y="0" class="legend">건축물</text>
<circle cx="245" cy="-4" r="4" fill="#465061"/><text x="256" y="0" class="legend">처리하지 않는 액터</text>
<circle cx="400" cy="-4" r="4" fill="#ffffff"/><text x="411" y="0" class="legend">플레이어와 150m 원</text>
<text x="0" y="24" class="legend">8배속. 맵 크기, 자리, 경로, 150m 원, 건축물과 NPC가 놓이는 반지름은 실제 비율이다. 점의 위치는 예시다</text>
</g>
</svg>
"@
	$OutPath = [System.IO.Path]::GetFullPath($Out)
	New-Item -ItemType Directory -Force -Path (Split-Path $OutPath) | Out-Null
	[System.IO.File]::WriteAllText($OutPath, $Svg2, [System.Text.UTF8Encoding]::new($false))
	Write-Host "wrote $OutPath ($([Math]::Round((Get-Item $OutPath).Length / 1KB)) KB)"
	return
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

<text x="30" y="34" class="title">적용 전: 모든 액터를 항상 보냄</text>
<text x="30" y="54" class="sub">서버가 모든 액터를 클라이언트 8개 모두에게 보낸다</text>
<g transform="translate(30 68)">
<rect width="400" height="400" class="map"/>
<use href="#nodes" class="dots node"/>
<use href="#npcs" class="dots npc"/>
$($PlayersNoRange.ToString())</g>

<text x="470" y="34" class="title">적용 후: 150m 거리 판정</text>
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
