# 지연과 패킷 손실 포스팅의 위치 그래프를 SVG로 만든다. make-interp-trace.ps1과 같은 방식으로, NPC 하나가 1.2초 동안 움직인 거리를
# 서버 위치와 클라이언트 화면의 위치로 그린다. 세 실행(루프백, 지연만, 손실만)을 그린다.
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/make-net-trace.ps1
# 결과:   Posts/13-lag-and-packet-loss/images/npc-trace-loopback.svg (act2-net-base1-r3)
#         Posts/13-lag-and-packet-loss/images/npc-trace-lag.svg      (지연만, act2-net-lag1-r3)
#         Posts/13-lag-and-packet-loss/images/npc-trace-loss.svg     (손실만, act2-net-loss1-r2)
#
# 실제 값으로 그린다: run-scenario.ps1 -MotionLog의 기록(Saved/LabMotion/<라벨>/server.bin, client0.bin, ADR-0020).
#   측정 구간 시작 10초 뒤부터 0.5초씩 옮기며, 1.2초 동안 곧게 걸은(가운데에서 꺾인 각도 2도 미만, 300cm/s의 90% 넘게 움직인)
#   NPC 가운데 NetGUID가 가장 작은 것을 고른다. 손실만 실행은 그 구간 안에 이동 갱신을 받은 간격이 250ms를 넘는 곳(갱신 하나를 잃은 곳)이
#   있는 NPC를 고른다(MinRecvGapMs). 세로축은 구간 시작의 서버 위치에서 구간의 이동 방향으로 잰 거리(cm)다.
#   아래의 눈금은 0번 클라이언트가 그 NPC의 이동 갱신을 받은 시각이다. 클라이언트 화면의 위치는 다음 프레임까지 그대로이므로 계단으로 그린다.

param(
	[string]$LoopbackLabel = 'act2-net-base1-r3',
	[string]$LagLabel = 'act2-net-lag1-r3',
	[string]$LossLabel = 'act2-net-loss1-r2',
	[string]$OutDir = (Join-Path $PSScriptRoot '..\Posts\13-lag-and-packet-loss\images')
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"

if (-not ('LabNetTrace' -as [type])) {
	Add-Type -Language CSharp -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.IO;

public class LabNetTraceResult
{
	public ulong Guid;
	public double StartSec;
	public double[] ServerT; public double[] ServerS;
	public double[] ClientT; public double[] ClientS;
	public double[] ReceiveT;
}

public static class LabNetTrace
{
	class Track { public List<double> T = new List<double>(); public List<double> X = new List<double>(); public List<double> Y = new List<double>(); }

	static void Read(string Path, out double Spc, out ulong MeasureStart, out Dictionary<ulong, Track> Pos, out Dictionary<ulong, List<double>> RecvCycles)
	{
		Pos = new Dictionary<ulong, Track>();
		RecvCycles = new Dictionary<ulong, List<double>>();
		using (BinaryReader R = new BinaryReader(File.OpenRead(Path)))
		{
			if (R.ReadUInt32() != 0x4D42414C) { throw new Exception("not a motion log: " + Path); }
			R.ReadInt32(); R.ReadInt32(); R.ReadInt32();
			Spc = R.ReadDouble();
			R.ReadUInt64(); MeasureStart = R.ReadUInt64(); R.ReadUInt64();
			int N = R.ReadInt32();
			for (int i = 0; i < N; i++)
			{
				double C = (double)R.ReadUInt64(); ulong G = R.ReadUInt64();
				double X = R.ReadSingle(); double Y = R.ReadSingle(); R.ReadSingle();
				Track Tr;
				if (!Pos.TryGetValue(G, out Tr)) { Tr = new Track(); Pos.Add(G, Tr); }
				Tr.T.Add(C); Tr.X.Add(X); Tr.Y.Add(Y);
			}
			int M = R.ReadInt32();
			for (int i = 0; i < M; i++)
			{
				double C = (double)R.ReadUInt64(); ulong G = R.ReadUInt64(); R.ReadByte();
				List<double> L;
				if (!RecvCycles.TryGetValue(G, out L)) { L = new List<double>(); RecvCycles.Add(G, L); }
				L.Add(C);
			}
		}
	}

	static bool Eval(Track Tr, double t, out double x, out double y)
	{
		x = 0; y = 0;
		int n = Tr.T.Count;
		if (n == 0 || t < Tr.T[0] || t > Tr.T[n - 1]) { return false; }
		int i = Tr.T.BinarySearch(t);
		if (i >= 0) { x = Tr.X[i]; y = Tr.Y[i]; return true; }
		i = ~i;
		double a = (t - Tr.T[i - 1]) / (Tr.T[i] - Tr.T[i - 1]);
		x = Tr.X[i - 1] + (Tr.X[i] - Tr.X[i - 1]) * a;
		y = Tr.Y[i - 1] + (Tr.Y[i] - Tr.Y[i - 1]) * a;
		return true;
	}

	public static LabNetTraceResult Find(string ServerPath, string ClientPath, double Length, double MinRecvGapMs)
	{
		double Spc, Spc2; ulong Ms, Ms2;
		Dictionary<ulong, Track> Server, Client; Dictionary<ulong, List<double>> Unused, Recv;
		Read(ServerPath, out Spc, out Ms, out Server, out Unused);
		Read(ClientPath, out Spc2, out Ms2, out Client, out Recv);
		// 시각을 서버 측정 구간 시작에서 잰 초로 바꾼다.
		foreach (Track Tr in Server.Values) { for (int i = 0; i < Tr.T.Count; i++) { Tr.T[i] = (Tr.T[i] - (double)Ms) * Spc; } }
		foreach (Track Tr in Client.Values) { for (int i = 0; i < Tr.T.Count; i++) { Tr.T[i] = (Tr.T[i] - (double)Ms) * Spc; } }

		List<ulong> Guids = new List<ulong>(Client.Keys);
		Guids.Sort();
		for (double t0 = 10.0; t0 < 55.0; t0 += 0.5)
		{
			foreach (ulong G in Guids)
			{
				Track Sv; if (!Server.TryGetValue(G, out Sv)) { continue; }
				Track Cl = Client[G];
				double ax, ay, bx, by, cx, cy;
				if (!Eval(Sv, t0 - 0.3, out ax, out ay) || !Eval(Sv, t0 + Length + 0.3, out ax, out ay)) { continue; }
				Eval(Sv, t0, out ax, out ay); Eval(Sv, t0 + Length / 2, out bx, out by); Eval(Sv, t0 + Length, out cx, out cy);
				double d1x = bx - ax, d1y = by - ay, d2x = cx - bx, d2y = cy - by;
				double l1 = Math.Sqrt(d1x * d1x + d1y * d1y), l2 = Math.Sqrt(d2x * d2x + d2y * d2y);
				if (l1 + l2 < 0.9 * 300.0 * Length || l1 < 1 || l2 < 1) { continue; }
				double Cos = (d1x * d2x + d1y * d2y) / (l1 * l2);
				if (Cos < Math.Cos(2.0 * Math.PI / 180.0)) { continue; }

				// 클라이언트 기록이 구간 1초 전부터 끊기지 않았는지 본다.
				int First = -1, Last = -1;
				bool Ok = true;
				for (int i = 0; i < Cl.T.Count; i++)
				{
					if (Cl.T[i] < t0 - 1.0) { continue; }
					if (Cl.T[i] > t0 + Length) { break; }
					if (First < 0) { First = i; if (i == 0 || Cl.T[i] - Cl.T[i - 1] > 0.5) { Ok = false; break; } }
					else if (Cl.T[i] - Cl.T[Last] > 0.2) { Ok = false; break; }
					Last = i;
				}
				if (!Ok || First < 0) { continue; }

				double Dx = (cx - ax) / Math.Sqrt((cx - ax) * (cx - ax) + (cy - ay) * (cy - ay));
				double Dy = (cy - ay) / Math.Sqrt((cx - ax) * (cx - ax) + (cy - ay) * (cy - ay));
				LabNetTraceResult Res = new LabNetTraceResult();
				Res.Guid = G; Res.StartSec = t0;
				List<double> St = new List<double>(), Ss = new List<double>();
				for (int i = 0; i < Sv.T.Count; i++)
				{
					if (Sv.T[i] < t0 - 0.05 || Sv.T[i] > t0 + Length + 0.05) { continue; }
					St.Add((Sv.T[i] - t0) * 1000.0); Ss.Add((Sv.X[i] - ax) * Dx + (Sv.Y[i] - ay) * Dy);
				}
				List<double> Ct = new List<double>(), Cs = new List<double>();
				for (int i = 0; i < Cl.T.Count; i++)
				{
					if (Cl.T[i] < t0 - 0.05 || Cl.T[i] > t0 + Length + 0.05) { continue; }
					Ct.Add((Cl.T[i] - t0) * 1000.0); Cs.Add((Cl.X[i] - ax) * Dx + (Cl.Y[i] - ay) * Dy);
				}
				List<double> Rt = new List<double>();
				List<double> Rc;
				if (Recv.TryGetValue(G, out Rc))
				{
					foreach (double C in Rc)
					{
						double t = (C - (double)Ms) * Spc;
						if (t >= t0 && t <= t0 + Length) { Rt.Add((t - t0) * 1000.0); }
					}
				}
				if (MinRecvGapMs > 0)
				{
					double MaxGap = 0;
					for (int i = 1; i < Rt.Count; i++) { MaxGap = Math.Max(MaxGap, Rt[i] - Rt[i - 1]); }
					if (MaxGap < MinRecvGapMs) { continue; }
				}
				Res.ServerT = St.ToArray(); Res.ServerS = Ss.ToArray();
				Res.ClientT = Ct.ToArray(); Res.ClientS = Cs.ToArray(); Res.ReceiveT = Rt.ToArray();
				return Res;
			}
		}
		throw new Exception("no straight window found (MinRecvGapMs=" + MinRecvGapMs + ")");
	}
}
'@
}

$Inv = [Globalization.CultureInfo]::InvariantCulture
function F([double]$V) { return $V.ToString('0.#', $Inv) }

$LengthSec = 1.2
$W = 860; $H = 330
$PlotL = 80; $PlotR = 830; $PlotT = 64; $PlotB = 270
$YMin = -60.0; $YMax = 420.0

function Px([double]$Ms) { return $PlotL + ($PlotR - $PlotL) * $Ms / ($LengthSec * 1000) }
function Py([double]$Cm) { return $PlotB - ($PlotB - $PlotT) * ($Cm - $YMin) / ($YMax - $YMin) }

function Make-Svg([string]$Label, [string]$Title, [string]$Aria, [string]$OutFile, [double]$MinRecvGapMs = 0) {
	$Dir = "$ProjectDir\Saved\LabMotion\$Label"
	$R = [LabNetTrace]::Find("$Dir\server.bin", "$Dir\client0.bin", $LengthSec, $MinRecvGapMs)
	Write-Host ("{0}: npc guid={1} window starts {2:F1}s after measure start" -f $Label, $R.Guid, $R.StartSec)

	$Body = @()
	# 격자: 200ms, 100cm마다
	for ($Ms = 0; $Ms -le 1200; $Ms += 200) {
		$X = F (Px $Ms)
		$Body += "<line x1=`"$X`" y1=`"$PlotT`" x2=`"$X`" y2=`"$PlotB`" class=`"grid`"/><text x=`"$X`" y=`"$($PlotB + 18)`" class=`"tick`" text-anchor=`"middle`">$Ms</text>"
	}
	for ($Cm = 0; $Cm -le 400; $Cm += 100) {
		$Y = F (Py $Cm)
		$Body += "<line x1=`"$PlotL`" y1=`"$Y`" x2=`"$PlotR`" y2=`"$Y`" class=`"grid`"/><text x=`"$($PlotL - 8)`" y=`"$([double]$Y + 4)`" class=`"tick`" text-anchor=`"end`">$Cm</text>"
	}
	$Body += "<text x=`"$(($PlotL + $PlotR) / 2)`" y=`"$($PlotB + 40)`" class=`"axis`" text-anchor=`"middle`">시간(ms)</text>"
	$Body += "<text x=`"22`" y=`"$(($PlotT + $PlotB) / 2)`" class=`"axis`" text-anchor=`"middle`" transform=`"rotate(-90 22 $(($PlotT + $PlotB) / 2))`">움직인 거리(cm)</text>"

	# 선은 그림 영역 안만 그린다.
	$Body += "<g clip-path=`"url(#plot)`">"
	# 서버 위치
	$Pts = for ($i = 0; $i -lt $R.ServerT.Length; $i++) { "{0},{1}" -f (F (Px $R.ServerT[$i])), (F (Py $R.ServerS[$i])) }
	$Body += "<polyline points=`"$($Pts -join ' ')`" class=`"server`"/>"

	# 클라이언트 화면의 위치(계단)
	$Pts = @()
	for ($i = 0; $i -lt $R.ClientT.Length; $i++) {
		if ($i -gt 0) { $Pts += "{0},{1}" -f (F (Px $R.ClientT[$i])), (F (Py $R.ClientS[$i - 1])) }
		$Pts += "{0},{1}" -f (F (Px $R.ClientT[$i])), (F (Py $R.ClientS[$i]))
	}
	$Body += "<polyline points=`"$($Pts -join ' ')`" class=`"client`"/>"
	$Body += "</g>"

	# 이동 갱신을 받은 시각
	foreach ($T in $R.ReceiveT) {
		$X = F (Px $T)
		$Body += "<line x1=`"$X`" y1=`"$($PlotB - 10)`" x2=`"$X`" y2=`"$PlotB`" class=`"recv`"/>"
	}

	$Svg = @"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 $W $H" width="$W" height="$H" role="img" aria-label="$Aria">
<style>
text{font-family:'Malgun Gothic','Apple SD Gothic Neo','Noto Sans KR',sans-serif;fill:#e6edf3}
.bg{fill:#0f141b}
.title{font-size:17px;font-weight:700}
.legend{font-size:13px;fill:#c9d4df}
.tick{font-size:12px;fill:#9aa7b5}
.axis{font-size:13px;fill:#c9d4df}
.grid{stroke:#2b3442;stroke-width:1}
.server{fill:none;stroke:#9aa7b5;stroke-width:2;stroke-dasharray:6 4}
.client{fill:none;stroke:#ff6b61;stroke-width:2.5;stroke-linejoin:round}
.recv{stroke:#f08c00;stroke-width:2}
</style>
<defs><clipPath id="plot"><rect x="$PlotL" y="$($PlotT - 10)" width="$($PlotR - $PlotL)" height="$($PlotB - $PlotT + 10)"/></clipPath></defs>
<rect width="$W" height="$H" class="bg"/>
<text x="20" y="30" class="title">$Title</text>
<line x1="470" y1="25" x2="500" y2="25" class="server"/><text x="508" y="30" class="legend">서버의 NPC</text>
<line x1="600" y1="25" x2="630" y2="25" class="client"/><text x="638" y="30" class="legend">클라이언트 화면의 NPC</text>
<line x1="470" y1="40" x2="470" y2="50" class="recv"/><text x="480" y="49" class="legend">이동 갱신을 받은 시각</text>
$($Body -join "`n")
</svg>
"@
	New-Item -ItemType Directory -Force (Split-Path $OutFile) | Out-Null
	[IO.File]::WriteAllText($OutFile, $Svg, (New-Object Text.UTF8Encoding $false))
	Write-Host "SAVED: $OutFile"
}

Make-Svg $LoopbackLabel '루프백: 150ms 보간이 매끄럽게 따라간다' '루프백. 클라이언트 화면의 NPC는 서버의 NPC와 같은 기울기로 매끄럽게 움직이고 약 155ms 늦다.' (Join-Path $OutDir 'npc-trace-loopback.svg')
Make-Svg $LagLabel '지연만(100~200ms): 갱신 직전에 멈추고 두 걸음을 간다' '지연만. 클라이언트 화면의 NPC는 이동 갱신을 받기 직전 프레임에 멈추고 받은 프레임에 두 걸음을 간다. 서버보다 약 286ms 늦다.' (Join-Path $OutDir 'npc-trace-lag.svg')
Make-Svg $LossLabel '손실만(5%): 잃은 갱신 뒤에 멈췄다가 건너뛴다' '손실만. 이동 갱신 하나를 잃은 곳에서 클라이언트 화면의 NPC가 약 118ms 멈춘 뒤 밀린 거리를 한 프레임에 건너뛴다.' (Join-Path $OutDir 'npc-trace-loss.svg') 250
