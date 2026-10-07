# NPC 이동의 NetSerialize 포스팅의 도식(NPC 갱신 한 번에 이동으로 가는 비트를 값별 막대로)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-move-bits.ps1
# 결과:   Posts/12-npc-move-netserialize/images/move-bits.svg
#
# 엔진 소스에서 옮긴 것(engine-notes.md 13절): FRepMovement::NetSerialize는 플래그 4비트, 위치(축당 비트 수를 적는 헤더 7비트와 X, Y, Z를 N비트씩),
#   회전(축마다 "0이 아님" 1비트와 0이 아닌 축의 1바이트), 선속도(속도 0이면 헤더 7비트와 축마다 1비트), 가속도 있음 1비트를 쓴다
#   (ReplicatedState.cpp:67-152, QuantizedVectorSerialization.cpp:90-96, UnrealMath.cpp의 TRotator::SerializeCompressed).
#   프로퍼티 핸들은 8비트다(RepLayout.cpp:1922-1932).
# 이 테스트베드에서 옮긴 것: NPC는 평면에서 속도 0으로 움직여 Pitch, Roll, 속도가 늘 0이고 Z가 늘 50이다. ServerFrame은 uint8이다(포스팅 11).
#   FLabNpcMove는 X, Y 13비트씩, Yaw 8비트, 서버 프레임 번호 8비트다(LabNpc.cpp의 FLabNpcMove::NetSerialize).
# 예시로 그리는 것: 위치의 축마다 쓰는 비트 수 N = 14. act2-interp2-r1에서 받은 갱신의 59.4%가 14였고 평균은 13.81이었다(candidates.md 1.2절).

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\12-npc-move-netserialize\images\move-bits.svg')
)

$ErrorActionPreference = 'Stop'

$PerBit = 8
$X0 = 40

# 값 하나: 이름, 비트 수, 종류(need = NPC에게 필요, handle = 프로퍼티 핸들, waste = 필요 없음), 막대 아래에 이름을 적을지
function Seg([string]$Name, [int]$Bits, [string]$Kind, [switch]$Below) {
	return @{ Name = $Name; Bits = $Bits; Kind = $Kind; Below = [bool]$Below }
}

# 막대 하나를 그린다. 이름이 들어가지 않는 좁은 칸은 칸 안에 비트 수만 쓰고 이름은 아래에 적는다.
function Bar([double]$Y, [object[]]$Segs) {
	$Parts = @()
	$X = $X0
	foreach ($S in $Segs) {
		$W = $S.Bits * $PerBit
		$Parts += "<rect x=`"$X`" y=`"$Y`" width=`"$W`" height=`"40`" class=`"$($S.Kind)`"/>"
		$Cx = $X + $W / 2
		if ($S.Below) {
			if ($W -ge 20) { $Parts += "<text x=`"$Cx`" y=`"$($Y + 26)`" class=`"bits`">$($S.Bits)</text>" }
			$Parts += "<path d=`"M$Cx $($Y + 42) V$($Y + 54)`" class=`"lead`"/>"
			$Parts += "<text x=`"$Cx`" y=`"$($Y + 68)`" class=`"small`">$($S.Name) $($S.Bits)</text>"
		} else {
			$Parts += "<text x=`"$Cx`" y=`"$($Y + 26)`" class=`"bits`">$($S.Name) $($S.Bits)</text>"
		}
		$X += $W
	}
	return $Parts -join "`n"
}

$Before = @(
	(Seg '핸들' 8 'handle'), (Seg '플래그' 4 'waste' -Below), (Seg '헤더' 7 'waste'),
	(Seg 'X' 14 'need'), (Seg 'Y' 14 'need'), (Seg 'Z' 14 'waste'),
	(Seg '회전 있음' 3 'waste' -Below), (Seg 'Yaw' 8 'need'), (Seg '속도' 10 'waste'), (Seg '가속도 있음' 1 'waste' -Below),
	(Seg '핸들' 8 'handle'), (Seg '프레임' 8 'need')
)
$After = @(
	(Seg '핸들' 8 'handle'), (Seg 'X' 13 'need'), (Seg 'Y' 13 'need'), (Seg 'Yaw' 8 'need'), (Seg '프레임' 8 'need')
)

# 지금 막대에서 ReplicatedMovement(핸들부터 가속도 있음까지 83비트)와 ServerFrame(16비트)을 나누는 괄호
$RmEnd = $X0 + 83 * $PerBit
$SfEnd = $RmEnd + 16 * $PerBit
$AfterEnd = $X0 + 50 * $PerBit

$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 430" width="900" height="430" role="img" aria-label="NPC 갱신 한 번에 이동으로 가는 비트. 지금은 ReplicatedMovement 83비트와 ServerFrame 16비트로 99비트이고 그중 Z, 속도, 위치 헤더, 늘 같은 플래그가 필요 없다. FLabNpcMove는 집 기준 X, Y 13비트씩, Yaw, 프레임 번호와 핸들로 50비트다.">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.bg{fill:#0f141b}
.title{font-size:17px;font-weight:700}
.note{font-size:13px;fill:#c9d4df}
.head{font-size:14px;font-weight:700}
.need{fill:#4dabf7;stroke:#0f141b;stroke-width:1}
.handle{fill:#868e96;stroke:#0f141b;stroke-width:1}
.waste{fill:#ff8787;stroke:#0f141b;stroke-width:1}
.bits{font-size:12px;font-weight:700;text-anchor:middle;fill:#0f141b}
.small{font-size:11px;text-anchor:middle;fill:#c9d4df}
.lead{stroke:#7d8896;stroke-width:1}
.brace{fill:none;stroke:#9aa7b5;stroke-width:1.2}
.bl{font-size:12px;fill:#9aa7b5;text-anchor:middle}
.total{font-size:14px;font-weight:700}
</style>
<rect width="900" height="430" class="bg"/>
<text x="20" y="34" class="title">NPC 갱신 한 번에 이동으로 가는 비트</text>
<text x="20" y="56" class="note">칸 하나가 값 하나이고, 폭이 비트 수다. 위치를 축마다 14비트로 쓸 때(이 테스트베드에서 가장 흔한 값)의 예다.</text>

<text x="$X0" y="80" class="head">지금: 엔진의 ReplicatedMovement와 포스팅 11의 ServerFrame</text>
<path d="M$X0 110 V104 H$RmEnd V110" class="brace"/>
<text x="$(($X0 + $RmEnd) / 2)" y="99" class="bl">ReplicatedMovement 83</text>
<path d="M$($RmEnd + 2) 110 V104 H$SfEnd V110" class="brace"/>
<text x="$(($RmEnd + $SfEnd) / 2)" y="99" class="bl">ServerFrame 16</text>
$(Bar 112 $Before)
<text x="$($SfEnd + 10)" y="138" class="total">99</text>

<text x="$X0" y="232" class="head">포스팅 12: 평면 이동만 담은 FLabNpcMove</text>
$(Bar 246 $After)
<text x="$($AfterEnd + 10)" y="272" class="total">50</text>
<text x="$($AfterEnd + 50)" y="264" class="note">X, Y는 집에서 ±30m 안이라 13비트로 1cm를 담는다.</text>
<text x="$($AfterEnd + 50)" y="284" class="note">Z, 속도, 헤더, 플래그는 보내지 않는다.</text>

<rect x="$X0" y="322" width="14" height="14" class="need"/><text x="$($X0 + 22)" y="334" class="note">NPC에게 필요한 것</text>
<rect x="$($X0 + 170)" y="322" width="14" height="14" class="handle"/><text x="$($X0 + 192)" y="334" class="note">프로퍼티 핸들(프로퍼티마다 하나)</text>
<rect x="$($X0 + 430)" y="322" width="14" height="14" class="waste"/><text x="$($X0 + 452)" y="334" class="note">필요 없는 것(늘 같은 값, 범위를 알면 필요 없는 헤더)</text>
<text x="20" y="372" class="note">축마다 쓰는 비트 수는 좌표가 클수록 늘어서, 맵 원점에서 멀면 커진다. 이 테스트베드의 평균은 13.8비트, 맵 가장자리는 18비트다.</text>
<text x="20" y="394" class="note">FLabNpcMove는 집을 기준으로 재므로 맵 어디서나 50비트다.</text>
</svg>
"@

$OutPath = [System.IO.Path]::GetFullPath($Out)
New-Item -ItemType Directory -Force (Split-Path $OutPath) | Out-Null
[System.IO.File]::WriteAllText($OutPath, $Svg, [System.Text.UTF8Encoding]::new($false))
Write-Host "wrote $OutPath ($([Math]::Round((Get-Item $OutPath).Length / 1KB)) KB)"
