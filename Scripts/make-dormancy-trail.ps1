# Dormancy 포스팅의 움직이는 도식(클라이언트 하나가 가진 자원 노드)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-dormancy-trail.ps1
# 결과:   Posts/03-dormancy/images/dormancy-trail.svg
#
# 실제 비율로 그리는 것: 이동 경로(한 변 100m 정사각형), Net Cull Distance 150m, 자원 노드와 NPC의 밀도(5,000개와 300명 ÷ 1.9km × 1.9km).
# 예시로 그리는 것: 자원 노드와 NPC의 위치(이 스크립트의 고정 시드로 고르게 뿌린다).
# 줄인 것: Relevancy만 적용한 쪽에서 원 밖으로 나간 액터는 실제로는 약 5초(RelevantTimeout) 뒤에 사라진다. 도식은 바로 끈다.
# 재생 속도: 걷기 500cm/s로 400m 한 바퀴가 80초이고, 도식은 10초에 한 바퀴(8배속) 돈 뒤 2초 멈춘다.

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\03-dormancy\images\dormancy-trail.svg')
)

$ErrorActionPreference = 'Stop'
$Inv = [cultureinfo]::InvariantCulture
function Fmt([double]$Value) { return $Value.ToString('0.#', $Inv) }

# 패널 400px가 500m다(1m = 0.8px).
$PanelPx = 400.0
$PxPerMeter = 0.8
$PanelKm2 = [Math]::Pow($PanelPx / $PxPerMeter / 1000.0, 2)
$AreaKm2 = 1.9 * 1.9
$NodeCount = [int][Math]::Round(5000 * $PanelKm2 / $AreaKm2)
$NpcCount = [int][Math]::Round(300 * $PanelKm2 / $AreaKm2)

$Rng = [System.Random]::new(20261003)
function DotPath([int]$Count) {
	$Builder = [System.Text.StringBuilder]::new()
	for ($i = 0; $i -lt $Count; $i++) {
		$x = $Rng.NextDouble() * $PanelPx
		$y = $Rng.NextDouble() * $PanelPx
		[void]$Builder.Append("M$(Fmt $x) $(Fmt $y)h0")
	}
	return $Builder.ToString()
}
$NodePath = DotPath $NodeCount
$NpcPath = DotPath $NpcCount

$Cull = Fmt (150.0 * $PxPerMeter)
$Side = 100.0 * $PxPerMeter
$x0 = Fmt 160; $y0 = Fmt 160
$x1 = Fmt (160 + $Side); $y1 = Fmt (160 + $Side)
$Lap = Fmt (4 * $Side)

# 10초에 한 바퀴 돌고 2초 멈춘다.
$Anim = 'keyTimes="0;.2083;.4167;.625;.8333;1" dur="12s" repeatCount="indefinite"'
$MoveX = "$x0;$x1;$x1;$x0;$x0;$x0"
$MoveY = "$y0;$y0;$y1;$y1;$y0;$y0"
$MoveXY = "$x0 $y0;$x1 $y0;$x1 $y1;$x0 $y1;$x0 $y0;$x0 $y0"

$Player = "<g transform=`"translate($x0 $y0)`"><animateTransform attributeName=`"transform`" type=`"translate`" values=`"$MoveXY`" $Anim/><circle r=`"$Cull`" class=`"range`"/><circle r=`"4`" class=`"player`"/></g>"
$Route = "<path d=`"M$x0 $y0 H$x1 V$y1 H$x0 Z`" class=`"route`"/>"

$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 520" width="900" height="520" role="img" aria-label="클라이언트 하나가 가진 자원 노드. 거리 판정만 켜면 원 밖으로 나간 자원 노드가 사라지고, 자원 노드를 Dormant 상태로 두면 지나온 길의 자원 노드가 남는다.">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.title{font-size:17px;font-weight:700}
.sub{font-size:13px;fill:#9aa7b5}
.legend{font-size:13px;fill:#c9d4df}
.map{fill:#151b24;stroke:#2b3442}
.dots{fill:none;stroke-linecap:round}
.off{stroke:#465061;stroke-width:3}
.node{stroke:#3fb950;stroke-width:4.5}
.npc{stroke:#ff6b61;stroke-width:6.5}
.range{fill:none;stroke:#e6edf3;stroke-width:1.2;stroke-opacity:.85}
.route{fill:none;stroke:#e6edf3;stroke-width:1;stroke-opacity:.35;stroke-dasharray:4 4}
.player{fill:#ffffff;stroke:#0f141b;stroke-width:1}
</style>
<defs>
<path id="nodes" d="$NodePath"/>
<path id="npcs" d="$NpcPath"/>
<clipPath id="inRange">
<circle cx="$x0" cy="$y0" r="$Cull"><animate attributeName="cx" values="$MoveX" $Anim/><animate attributeName="cy" values="$MoveY" $Anim/></circle>
</clipPath>
<mask id="visited" maskUnits="userSpaceOnUse" x="0" y="0" width="400" height="400">
<circle cx="$x0" cy="$y0" r="$Cull" fill="#fff"/>
<path d="M$x0 $y0 H$x1 V$y1 H$x0 Z" fill="none" stroke="#fff" stroke-width="$(Fmt (300.0 * $PxPerMeter))" stroke-linejoin="round" stroke-linecap="round" stroke-dasharray="$Lap" stroke-dashoffset="$Lap"><animate attributeName="stroke-dashoffset" values="$Lap;0;0" keyTimes="0;.8333;1" dur="12s" repeatCount="indefinite"/></path>
</mask>
<clipPath id="panel"><rect width="400" height="400"/></clipPath>
</defs>
<rect width="900" height="520" rx="10" fill="#0f141b"/>

<text x="30" y="34" class="title">거리 판정만</text>
<text x="30" y="54" class="sub">원 밖으로 나간 자원 노드는 클라이언트에서 사라진다</text>
<g transform="translate(30 68)" clip-path="url(#panel)">
<rect width="400" height="400" class="map"/>
<use href="#nodes" class="dots off"/>
<use href="#npcs" class="dots off"/>
<g clip-path="url(#inRange)">
<use href="#nodes" class="dots node"/>
<use href="#npcs" class="dots npc"/>
</g>
$Route
$Player
</g>

<text x="470" y="34" class="title">자원 노드를 Dormant 상태로</text>
<text x="470" y="54" class="sub">지나온 길의 자원 노드가 클라이언트에 남는다</text>
<g transform="translate(470 68)" clip-path="url(#panel)">
<rect width="400" height="400" class="map"/>
<use href="#nodes" class="dots off"/>
<use href="#npcs" class="dots off"/>
<g mask="url(#visited)">
<use href="#nodes" class="dots node"/>
</g>
<g clip-path="url(#inRange)">
<use href="#npcs" class="dots npc"/>
</g>
$Route
$Player
</g>

<g transform="translate(30 497)" class="legend">
<circle cx="5" cy="-4" r="4" fill="#3fb950"/><text x="16" y="0" class="legend">클라이언트에 있는 자원 노드</text>
<circle cx="205" cy="-4" r="4" fill="#ff6b61"/><text x="216" y="0" class="legend">클라이언트에 있는 NPC</text>
<circle cx="380" cy="-4" r="4" fill="#465061"/><text x="391" y="0" class="legend">서버에만 있는 액터</text>
<text x="540" y="0" class="legend">원은 150m, 8배속. 점의 위치는 예시다</text>
</g>
</svg>
"@

$OutPath = [System.IO.Path]::GetFullPath($Out)
[System.IO.File]::WriteAllText($OutPath, $Svg, [System.Text.UTF8Encoding]::new($false))
Write-Host "wrote $OutPath ($([Math]::Round((Get-Item $OutPath).Length / 1KB)) KB), nodes $NodeCount, npcs $NpcCount"
