# Replication Graph 포스팅의 원리 도식(서버가 1번 클라이언트에게 보낼지 확인하는 자원 노드)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-repgraph-cell.ps1
# 결과:   Posts/14-replication-graph/images/repgraph-cell.svg
#
# 왼쪽은 레거시, 오른쪽은 Replication Graph가 1번 클라이언트의 연결 하나에 대해 자원 노드를 어떻게 다루는지 같은 창(700m × 700m)으로 그린다.
# 실제 값으로 그리는 것:
#   - 시점: 1번 클라이언트의 자동 스크린샷 위치 (-5m, 60m)(act2-rg-trk1-r2-topdown-04.png, t=75s).
#   - Net Cull Distance 150m(Actor.cpp의 NetCullDistanceSquared 기본값).
#   - 칸 100m(LabReplicationGraph.cpp의 CellSize 10000)와 칸의 경계. 칸 번호는 (위치 - SpatialBias) ÷ CellSize이고 SpatialBias는
#     -UE_OLD_WORLD_MAX(-2,097,152cm, EngineDefines.h:37)라서 경계가 x, y = 28.48m + 100m × n에 있다(ReplicationGraph.cpp의
#     GetCellInfoForActor, GatherActorListsForConnection).
#   - 칸에 등록되는 범위: 자원 노드는 자기 위치 ± 컬 거리 사각형이 걸치는 모든 칸에 들어가므로, 시점 칸의 목록에는 칸을 150m 넓힌
#     400m × 400m 안의 자원 노드가 있다(GetCellInfoForActor).
#   - 자원 노드의 밀도: 1,900m × 1,900m에 5,001개(LabScenarioConfig.h의 WorldHalfExtent 95,000cm).
# 예시로 그리는 것: 자원 노드의 위치(이 스크립트의 고정 시드로 같은 밀도로 뿌린다. 게임의 배치와 같지 않다), 다른 플레이어는 그리지 않는다.

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\14-replication-graph\images\repgraph-cell.svg')
)

$ErrorActionPreference = 'Stop'
$Inv = [cultureinfo]::InvariantCulture
function Fmt([double]$Value) { return $Value.ToString('0.#', $Inv) }

# 시점과 칸
$ViewX = -5.0; $ViewY = 60.0
$Cell = 100.0
$Offset = 28.48                          # 칸 경계 = 28.48 + 100n (m)
$CellX0 = [Math]::Floor(($ViewX - $Offset) / $Cell) * $Cell + $Offset
$CellY0 = [Math]::Floor(($ViewY - $Offset) / $Cell) * $Cell + $Offset
$Cull = 150.0
$RegX0 = $CellX0 - $Cull; $RegY0 = $CellY0 - $Cull; $RegSize = $Cell + 2 * $Cull

# 창: 시점 칸의 가운데를 중심으로 700m. 1px = 2m라 350px이다.
$Win = 700.0; $MetersPerPx = 2.0; $PanelPx = $Win / $MetersPerPx
$WinX0 = $CellX0 + $Cell / 2 - $Win / 2
$WinY0 = $CellY0 + $Cell / 2 - $Win / 2
$Panels = @(@{ X = 40.0; Graph = $false }, @{ X = 510.0; Graph = $true })
$PanelTop = 92.0

# 같은 밀도의 예시 자원 노드
$Density = 5001.0 / (1900.0 * 1900.0)
$Count = [int][Math]::Round($Density * $Win * $Win)
$Rng = [System.Random]::new(20261019)
$Nodes = for ($i = 0; $i -lt $Count; $i++) { ,@(($WinX0 + $Rng.NextDouble() * $Win), ($WinY0 + $Rng.NextDouble() * $Win)) }

function Classify($N) {
	$dx = $N[0] - $ViewX; $dy = $N[1] - $ViewY
	if ($dx * $dx + $dy * $dy -le $Cull * $Cull) { return 'near' }
	if ($N[0] -ge $RegX0 -and $N[0] -le $RegX0 + $RegSize -and $N[1] -ge $RegY0 -and $N[1] -le $RegY0 + $RegSize) { return 'cell' }
	return 'far'
}

$Svg = [System.Text.StringBuilder]::new()
[void]$Svg.AppendLine('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 610" width="900" height="610" role="img" aria-label="서버가 1번 클라이언트에게 보낼지 확인하는 자원 노드. 레거시는 150m 밖의 자원 노드가 모두 활성 목록에 남아 매 프레임 돈다. 그래프는 시점이 있는 100m 칸의 목록만 모으고, 그 목록에는 150m 밖이지만 칸 범위 안의 자원 노드가 섞여 있다.">')
[void]$Svg.AppendLine(@'
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.title{font-size:17px;font-weight:700}
.sub{font-size:13px;fill:#9aa7b5}
.legend{font-size:13px;fill:#c9d4df}
.map{fill:#151b24;stroke:#2b3442}
.grid{stroke:#2b3442;stroke-width:1}
.cell{fill:#58a6ff;fill-opacity:.22;stroke:#58a6ff;stroke-width:1.5}
.reg{fill:none;stroke:#58a6ff;stroke-width:1.2;stroke-dasharray:5 3}
.range{fill:none;stroke:#e6edf3;stroke-width:1;stroke-opacity:.85}
.near{fill:#3fb950}
.work{fill:#f0a33a}
.far{fill:#465061}
.player{fill:#ffffff;stroke:#0f141b;stroke-width:1}
</style>
'@)
[void]$Svg.AppendLine('<rect width="900" height="610" rx="10" fill="#0f141b"/>')
[void]$Svg.AppendLine('<text class="title" x="40" y="34">서버가 1번 클라이언트에게 보낼지 확인하는 자원 노드</text>')
[void]$Svg.AppendLine('<text class="sub" x="40" y="56">1번 클라이언트 주변 700m. 칸, 150m 원, 칸 목록의 범위는 실제 값이고 자원 노드의 위치는 같은 밀도의 예시다.</text>')

foreach ($P in $Panels) {
	$ox = $P.X
	function Px([double]$m) { return $ox + ($m - $WinX0) / $MetersPerPx }
	function Py([double]$m) { return $PanelTop + ($m - $WinY0) / $MetersPerPx }
	[void]$Svg.AppendLine("<text class=`"title`" x=`"$(Fmt $ox)`" y=`"$(Fmt ($PanelTop - 12))`">$(if ($P.Graph) { 'Replication Graph' } else { '레거시' })</text>")
	[void]$Svg.AppendLine("<rect class=`"map`" x=`"$(Fmt $ox)`" y=`"$(Fmt $PanelTop)`" width=`"$(Fmt $PanelPx)`" height=`"$(Fmt $PanelPx)`"/>")
	if ($P.Graph) {
		for ($g = [Math]::Ceiling(($WinX0 - $Offset) / $Cell) * $Cell + $Offset; $g -lt $WinX0 + $Win; $g += $Cell) {
			[void]$Svg.AppendLine("<line class=`"grid`" x1=`"$(Fmt (Px $g))`" y1=`"$(Fmt $PanelTop)`" x2=`"$(Fmt (Px $g))`" y2=`"$(Fmt ($PanelTop + $PanelPx))`"/>")
		}
		for ($g = [Math]::Ceiling(($WinY0 - $Offset) / $Cell) * $Cell + $Offset; $g -lt $WinY0 + $Win; $g += $Cell) {
			[void]$Svg.AppendLine("<line class=`"grid`" x1=`"$(Fmt $ox)`" y1=`"$(Fmt (Py $g))`" x2=`"$(Fmt ($ox + $PanelPx))`" y2=`"$(Fmt (Py $g))`"/>")
		}
		[void]$Svg.AppendLine("<rect class=`"reg`" x=`"$(Fmt (Px $RegX0))`" y=`"$(Fmt (Py $RegY0))`" width=`"$(Fmt ($RegSize / $MetersPerPx))`" height=`"$(Fmt ($RegSize / $MetersPerPx))`"/>")
		[void]$Svg.AppendLine("<rect class=`"cell`" x=`"$(Fmt (Px $CellX0))`" y=`"$(Fmt (Py $CellY0))`" width=`"$(Fmt ($Cell / $MetersPerPx))`" height=`"$(Fmt ($Cell / $MetersPerPx))`"/>")
	}
	foreach ($N in $Nodes) {
		$Kind = Classify $N
		$Class = if ($Kind -eq 'near') { 'near' }
			elseif (-not $P.Graph) { 'work' }
			elseif ($Kind -eq 'cell') { 'work' }
			else { 'far' }
		[void]$Svg.AppendLine("<circle class=`"$Class`" cx=`"$(Fmt (Px $N[0]))`" cy=`"$(Fmt (Py $N[1]))`" r=`"1.8`"/>")
	}
	[void]$Svg.AppendLine("<circle class=`"range`" cx=`"$(Fmt (Px $ViewX))`" cy=`"$(Fmt (Py $ViewY))`" r=`"$(Fmt ($Cull / $MetersPerPx))`"/>")
	[void]$Svg.AppendLine("<circle class=`"player`" cx=`"$(Fmt (Px $ViewX))`" cy=`"$(Fmt (Py $ViewY))`" r=`"4`"/>")
}

# 범례
$Ly = $PanelTop + $PanelPx + 30
$Legend = @(
	@('player', '1번 클라이언트의 시점과 150m 원'),
	@('near', '150m 안: 한 번 보내면 Dormant 상태가 되어 다시 따지지 않는다'),
	@('work', '왼쪽: 활성 목록에 남아 매 프레임 돈다(맵 전체 약 4,800개)'),
	@('work', '오른쪽: 시점 칸(파란 칸)의 목록에 섞여 매 프레임 거리 검사에서 걸러진다'),
	@('work', '         trickle은 이 점을 한 프레임에 하나씩 보내 Dormant 상태로 만들고 목록에서 지운다'),
	@('far', '오른쪽: 파란 칸의 목록에 없어 1번 클라이언트에게는 확인하지 않는다(점선은 그 목록의 범위 400m)')
)
for ($i = 0; $i -lt $Legend.Count; $i++) {
	$y = $Ly + 22 * $i
	if (-not $Legend[$i][1].StartsWith(' ')) {
		[void]$Svg.AppendLine("<circle class=`"$($Legend[$i][0])`" cx=`"46`" cy=`"$(Fmt ($y - 4))`" r=`"4`"/>")
	}
	[void]$Svg.AppendLine("<text class=`"legend`" x=`"58`" y=`"$(Fmt $y)`">$($Legend[$i][1].Trim())</text>")
}
[void]$Svg.AppendLine('</svg>')

$OutPath = [System.IO.Path]::GetFullPath($Out)
[System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($OutPath)) | Out-Null
[System.IO.File]::WriteAllText($OutPath, $Svg.ToString(), [System.Text.UTF8Encoding]::new($false))
$Kinds = $Nodes | ForEach-Object { Classify $_ } | Group-Object | ForEach-Object { "$($_.Name)=$($_.Count)" }
Write-Host "SAVED: $OutPath (nodes $Count, $($Kinds -join ', '), cell x $(Fmt $CellX0)..$(Fmt ($CellX0 + $Cell)) y $(Fmt $CellY0)..$(Fmt ($CellY0 + $Cell)))"
