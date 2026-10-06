# FastArray 포스팅의 도식(추가, 삭제, 변경에서 서버가 보내는 것과 클라이언트가 하는 일)을 SVG로 만든다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-fastarray-cases.ps1
# 결과:   Posts/10-inventory-fastarray/images/fastarray-cases.svg
#
# 엔진 소스에서 옮긴 것: 머리는 int32 넷(128비트, FastArraySerializer.h의 WriteDeltaHeader), 지운 칸은 번호 int32다.
#   칸 안 델타 직렬화가 기본으로 켜져 있어(FastArraySerializer.cpp 생성자) 칸마다 번호 uint32, 1비트, 프로퍼티마다 핸들 8비트와 값,
#   끝 핸들 8비트를 쓴다(RepLayout.cpp DeltaSerializeFastArrayProperty). 새 칸은 프로퍼티를 모두 보내고(net.DeltaInitialFastArrayElements 기본 0),
#   바뀐 칸은 바뀐 프로퍼티만 보낸다. 실행 act2-fastarr1-r2의 ChangedElement 2,382비트 = 15 x 121 + 7 x 81과 맞는다. 클라이언트는 새 칸을 맨 뒤에 더하고
#   (1524, AddDefaulted_GetRef) 지운 칸을 마지막에 RemoveAtSwap으로 지운다(1186-1197). 칸의 콜백은 FFastArraySerializerItem의 것이고,
#   PostReplicatedReceive는 배열 구조체에 정의돼 있을 때 받은 묶음마다 한 번 불린다(699-707).
# 이 테스트베드에서 옮긴 것: 칸 하나는 ItemId와 Count 두 int32(64비트)다.
# 예시로 그리는 것: 칸 다섯 개와 그 번호. 실제 인벤토리는 200칸이다. 프로퍼티 머리 같은 덧붙는 비트는 그리지 않는다.

param(
	[string]$Out = (Join-Path $PSScriptRoot '..\Posts\10-inventory-fastarray\images\fastarray-cases.svg')
)

$ErrorActionPreference = 'Stop'

$Size = 38
$Pitch = 42
$Colors = @{ 1 = '#4dabf7'; 2 = '#69db7c'; 3 = '#ffd43b'; 4 = '#da77f2'; 5 = '#ff8787'; 6 = '#63e6be' }

# 번호가 적힌 칸 하나. Mark는 강조(바뀐 칸), Ghost는 지운 자리의 점선 칸이다.
function Cell([double]$X, [double]$Y, [int]$Id, [switch]$Mark, [switch]$Ghost) {
	if ($Ghost) {
		return "<rect x=`"$X`" y=`"$Y`" width=`"$Size`" height=`"$Size`" rx=`"4`" class=`"ghost`"/><text x=`"$($X + $Size / 2)`" y=`"$($Y + 25)`" class=`"cid gid`">#$Id</text>"
	}
	$Class = if ($Mark) { 'cell mark' } else { 'cell' }
	return "<rect x=`"$X`" y=`"$Y`" width=`"$Size`" height=`"$Size`" rx=`"4`" class=`"$Class`" fill=`"$($Colors[$Id])`"/><text x=`"$($X + $Size / 2)`" y=`"$($Y + 25)`" class=`"cid`">#$Id</text>"
}

# 칸을 왼쪽부터 늘어놓는다. 원소는 번호, 음수는 강조할 칸, 0은 빈자리, 'g<번호>'는 점선 칸이다.
function Row([double]$X, [double]$Y, [object[]]$Items) {
	$Parts = @()
	for ($i = 0; $i -lt $Items.Count; $i++) {
		$Item = [string]$Items[$i]
		$Cx = $X + $i * $Pitch
		if ($Item -eq '0') { continue }
		if ($Item.StartsWith('g')) { $Parts += Cell $Cx $Y ([int]$Item.Substring(1)) -Ghost; continue }
		$Id = [int]$Item
		if ($Id -lt 0) { $Parts += Cell $Cx $Y (-$Id) -Mark } else { $Parts += Cell $Cx $Y $Id }
	}
	return $Parts -join "`n"
}

# 세 칸(추가, 삭제, 변경)의 내용
$Cols = @(
	@{ Title = '추가'; Sub = '서버가 맨 뒤에 새 칸 #6을 더한다'
	   Server = @(1, 2, 3, 4, 5, -6)
	   Wire = @('#6 번호 32 + 프로퍼티 모두 89', '= 121비트')
	   Plain = '일반 배열이면: 새 칸 하나와 배열 길이'
	   Client = @(1, 2, 3, 4, 5, -6)
	   Act = '맨 뒤에 붙인다'; Callback = 'PostReplicatedAdd' },
	@{ Title = '삭제'; Sub = '서버가 맨 앞 칸 #1을 지운다. 뒤 칸이 당겨진다'
	   Server = @('g1', 2, 3, 4, 5)
	   Wire = @('지운 번호 #1', '= 32비트')
	   Plain = '일반 배열이면: 당겨진 네 칸 모두와 배열 길이'
	   Client = @(-5, 2, 3, 4, 'g5')
	   Act = '맨 뒤 칸을 지운 자리로 옮긴다'; Callback = 'PreReplicatedRemove, RemoveAtSwap' },
	@{ Title = '변경'; Sub = '서버가 #3의 수량을 늘린다(채집)'
	   Server = @(1, 2, -3, 4, 5)
	   Wire = @('#3 번호 32 + 바뀐 Count만 49', '= 81비트')
	   Plain = '일반 배열이면: 그 칸의 Count 하나'
	   Client = @(1, 2, -3, 4, 5)
	   Act = '같은 칸을 덮어쓴다'; Callback = 'PostReplicatedChange' }
)

$Body = @()
for ($c = 0; $c -lt 3; $c++) {
	$Col = $Cols[$c]
	$X0 = 20 + $c * 295
	$Cx = $X0 + 10
	$Body += "<rect x=`"$X0`" y=`"70`" width=`"280`" height=`"390`" rx=`"8`" class=`"panel`"/>"
	$Body += "<text x=`"$($X0 + 14)`" y=`"98`" class=`"head`">$($Col.Title)</text>"
	$Body += "<text x=`"$($X0 + 14)`" y=`"118`" class=`"sub`">$($Col.Sub)</text>"

	$Body += "<text x=`"$Cx`" y=`"146`" class=`"label`">서버 배열</text>"
	$Body += Row $Cx 154 $Col.Server
	if ($c -eq 1) {
		# 지운 칸 위에 X를 긋는다.
		$Body += "<path d=`"M$($Cx + 6) 160 l26 26 M$($Cx + 32) 160 l-26 26`" class=`"cross`"/>"
	}

	$Body += "<text x=`"$Cx`" y=`"218`" class=`"label`">가는 것</text>"
	$Body += "<rect x=`"$Cx`" y=`"226`" width=`"252`" height=`"52`" rx=`"6`" class=`"wire`"/>"
	$Body += "<text x=`"$($Cx + 10)`" y=`"247`" class=`"wtext`">$($Col.Wire[0])</text>"
	$Body += "<text x=`"$($Cx + 10)`" y=`"268`" class=`"wtext`">$($Col.Wire[1])</text>"
	$Body += "<text x=`"$Cx`" y=`"296`" class=`"plain`">$($Col.Plain)</text>"

	$Body += "<text x=`"$Cx`" y=`"330`" class=`"label`">클라이언트 배열</text>"
	$Body += Row $Cx 338 $Col.Client
	if ($c -eq 1) {
		# 맨 뒤 칸 #5가 지운 자리(0번)로 옮겨 간다.
		$From = $Cx + 4 * $Pitch + $Size / 2
		$To = $Cx + $Size / 2
		$Body += "<path d=`"M$From 382 C$From 410 $To 410 $To 380`" class=`"move`" marker-end=`"url(#arrow)`"/>"
	}
	$Body += "<text x=`"$Cx`" y=`"428`" class=`"act`">$($Col.Act)</text>"
	$Body += "<text x=`"$Cx`" y=`"448`" class=`"cb`">$($Col.Callback)</text>"
}

$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 584" width="900" height="584" role="img" aria-label="FastArray의 추가, 삭제, 변경. 서버는 번호로 바뀐 칸과 지운 번호만 보낸다. 클라이언트는 새 칸을 맨 뒤에 붙이고, 지운 자리에 맨 뒤 칸을 옮기고, 바뀐 칸을 덮어쓴다.">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.bg{fill:#0f141b}
.title{font-size:17px;font-weight:700}
.note{font-size:13px;fill:#c9d4df}
.panel{fill:#151b24;stroke:#2b3442}
.head{font-size:16px;font-weight:700}
.sub{font-size:12px;fill:#9aa7b5}
.label{font-size:12px;fill:#9aa7b5}
.cell{stroke:#0f141b;stroke-width:1}
.mark{stroke:#ffffff;stroke-width:3}
.ghost{fill:none;stroke:#7d8896;stroke-width:1.2;stroke-dasharray:4 3}
.cid{font-size:13px;font-weight:700;text-anchor:middle;fill:#0f141b}
.gid{fill:#7d8896}
.cross{stroke:#ff6b61;stroke-width:2.5;fill:none}
.wire{fill:#2a2110;stroke:#f08c00}
.wtext{font-size:13px;fill:#ffe8b0}
.plain{font-size:12px;fill:#7d8896}
.move{fill:none;stroke:#ffffff;stroke-width:1.5}
.act{font-size:13px}
.cb{font-size:12px;font-family:Consolas,'Courier New',monospace;fill:#9aa7b5}
</style>
<defs><marker id="arrow" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0 0L10 5L0 10z" fill="#ffffff"/></marker></defs>
<rect width="900" height="584" class="bg"/>
<text x="20" y="34" class="title">FastArray가 보내는 것과 클라이언트가 하는 일</text>
<text x="20" y="56" class="note">칸마다 번호(#)가 붙는다. 서버는 자리가 아니라 번호로 바뀐 것을 찾는다. 흰 테두리가 바뀐 칸, 노란 상자가 칸 하나에 가는 것이다.</text>
$($Body -join "`n")
<text x="20" y="494" class="note">받은 묶음마다 머리 128비트가 붙고, 클라이언트는 묶음 끝에 PostReplicatedReceive를 한 번 부른다(정의돼 있을 때).</text>
<text x="20" y="516" class="note">이 테스트베드의 "앞 칸 지우기"는 삭제와 추가가 한 묶음으로 간다: 머리 128 + 지운 번호 32 + 새 칸 121 = 281비트다.</text>`r`n<text x="20" y="538" class="note">그래서 클라이언트에서는 새 칸이 지운 자리로 들어가, 칸 순서가 서버와 달라진다.</text>`r`n<text x="20" y="560" class="note">칸 다섯 개는 예시이고 실제는 200칸이다. 비트는 칸이 ItemId와 Count 두 int32일 때의 값이다.</text>
</svg>
"@

$OutPath = [System.IO.Path]::GetFullPath($Out)
[System.IO.File]::WriteAllText($OutPath, $Svg, [System.Text.UTF8Encoding]::new($false))
Write-Host "wrote $OutPath ($([Math]::Round((Get-Item $OutPath).Length / 1KB)) KB)"
