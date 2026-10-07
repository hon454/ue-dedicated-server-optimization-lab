# NPC 움직임의 품질 지표를 계산한다(ADR-0020).
# run-scenario.ps1 -MotionLog 실행이 Saved\LabMotion\<라벨>-rN\에 남긴 server.bin과 clientK.bin을 읽는다.
# 결과는 같은 폴더의 summary.txt와 Saved\LabMetrics\motion.csv의 한 행이다(같은 라벨의 행이 있으면 더하지 않는다).
#
# 정의(ADR-0020 "정의"):
#   시각은 서버의 측정 구간 시작(Lab_MeasureStart와 같은 순간)에서 잰 초다. 두 프로세스가 같은 QueryPerformanceCounter를 쓴다.
#   서버 위치 p_s(t)는 서버 프레임 기록 사이를 선형 보간한 값이다.
#   표본은 측정 구간 안의 (클라이언트, NPC, 클라이언트 프레임)이다. NPC가 그 클라이언트에 나타난 뒤 1초 안의 표본은 뺀다.
#   NPC 하나의 클라이언트 기록이 0.5초 넘게 끊기면 다시 나타난 것으로 본다(관련성을 잃고 다시 얻음).
#   표시 위치 오차 = |p_c(t_k) - p_s(t_k)|(cm). 표시 속도 오차 = |Δp_c - Δp_s| ÷ Δt(cm/s), Δ는 클라이언트 프레임 한 칸.
#   표시 위치가 바뀐 간격 = 표시 위치가 1cm 넘게 바뀐 프레임 사이의 시간(ms).
#   수신 간격 = NPC 하나의 이동 갱신을 받은 시각의 차(ms). 2초가 넘는 간격(다시 나타남)은 뺀다.
#   표시 지연 = 평균 |p_c(t) - p_s(t - τ)|가 가장 작은 τ(0~600ms, 5ms 간격으로 찾고 그 주변을 1ms 간격으로 다시 찾음).
#     ADR-0020은 0~300ms로 적었다. 포스팅 13의 지연 100~200ms가 보간 지연 150ms에 더해지면 300ms를 넘으므로 범위만 넓혔다(2026-10-07).
#     계산량을 줄이려고 표본 다섯 개 중 하나만 쓴다.
#   P99는 정렬 후 ceil(N × 0.99)번째 값이다(수치 CSV와 같다).
#
# 사용법: powershell -ExecutionPolicy Bypass -File Scripts/analyze-motion.ps1 -Label <라벨>-rN
param(
    [Parameter(Mandatory = $true)][string]$Label
)

. "$PSScriptRoot\common.ps1"

$Dir = "$ProjectDir\Saved\LabMotion\$Label"
if (-not (Test-Path "$Dir\server.bin")) {
    Write-Host "FAIL: $Dir\server.bin does not exist. Run run-scenario.ps1 with -MotionLog first."
    exit 1
}
$ClientFiles = @(Get-ChildItem "$Dir\client*.bin" | Sort-Object Name | ForEach-Object { $_.FullName })
if ($ClientFiles.Count -eq 0) {
    Write-Host "FAIL: no client*.bin in $Dir"
    exit 1
}

if (-not ('LabMotionAnalyzer' -as [type])) {
    Add-Type -Language CSharp -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

public class LabMotionFile
{
    public int Kind;
    public int Slot;
    public double SecondsPerCycle;
    public ulong StartSignal;
    public ulong MeasureStart;
    public ulong MeasureEnd;
    public ulong[] PCycles;
    public ulong[] PGuid;
    public float[] PX;
    public float[] PY;
    public float[] PZ;
    public ulong[] RCycles;
    public ulong[] RGuid;
    public byte[] RFrame;

    public static LabMotionFile Read(string Path)
    {
        LabMotionFile F = new LabMotionFile();
        using (BinaryReader R = new BinaryReader(File.OpenRead(Path)))
        {
            uint Magic = R.ReadUInt32();
            if (Magic != 0x4D42414C) { throw new Exception("not a motion log: " + Path); }
            int Version = R.ReadInt32();
            if (Version != 1) { throw new Exception("unknown version " + Version + ": " + Path); }
            F.Kind = R.ReadInt32();
            F.Slot = R.ReadInt32();
            F.SecondsPerCycle = R.ReadDouble();
            F.StartSignal = R.ReadUInt64();
            F.MeasureStart = R.ReadUInt64();
            F.MeasureEnd = R.ReadUInt64();
            int N = R.ReadInt32();
            F.PCycles = new ulong[N]; F.PGuid = new ulong[N]; F.PX = new float[N]; F.PY = new float[N]; F.PZ = new float[N];
            for (int i = 0; i < N; i++)
            {
                F.PCycles[i] = R.ReadUInt64(); F.PGuid[i] = R.ReadUInt64();
                F.PX[i] = R.ReadSingle(); F.PY[i] = R.ReadSingle(); F.PZ[i] = R.ReadSingle();
            }
            int M = R.ReadInt32();
            F.RCycles = new ulong[M]; F.RGuid = new ulong[M]; F.RFrame = new byte[M];
            for (int i = 0; i < M; i++)
            {
                F.RCycles[i] = R.ReadUInt64(); F.RGuid[i] = R.ReadUInt64(); F.RFrame[i] = R.ReadByte();
            }
        }
        return F;
    }
}

public class LabTrack
{
    public List<double> TL = new List<double>();
    public List<double> XL = new List<double>();
    public List<double> YL = new List<double>();
    public List<double> ZL = new List<double>();
    public double[] T; public double[] X; public double[] Y; public double[] Z;

    public void Add(double t, double x, double y, double z) { TL.Add(t); XL.Add(x); YL.Add(y); ZL.Add(z); }
    public void Freeze() { T = TL.ToArray(); X = XL.ToArray(); Y = YL.ToArray(); Z = ZL.ToArray(); TL = null; XL = null; YL = null; ZL = null; }

    // 서버 위치를 t에서 선형 보간한다. 기록 범위 밖이면 false.
    public bool Eval(double t, out double x, out double y, out double z)
    {
        x = 0; y = 0; z = 0;
        if (T.Length == 0 || t < T[0] || t > T[T.Length - 1]) { return false; }
        int i = Array.BinarySearch(T, t);
        if (i >= 0) { x = X[i]; y = Y[i]; z = Z[i]; return true; }
        i = ~i; // T[i-1] < t < T[i]
        double a = (t - T[i - 1]) / (T[i] - T[i - 1]);
        x = X[i - 1] + (X[i] - X[i - 1]) * a;
        y = Y[i - 1] + (Y[i] - Y[i - 1]) * a;
        z = Z[i - 1] + (Z[i] - Z[i - 1]) * a;
        return true;
    }
}

public static class LabMotionAnalyzer
{
    const double AppearSkip = 1.0;   // 나타난 뒤 뺄 시간(초)
    const double SegmentGap = 0.5;   // 이보다 길게 끊기면 다시 나타난 것으로 봄(초)
    const double ReceiveGapMax = 2.0;
    const double ChangeEpsilon = 1.0; // cm
    const int LagStride = 5;

    static double Avg(List<double> V) { if (V.Count == 0) { return 0; } double S = 0; foreach (double v in V) { S += v; } return S / V.Count; }
    static double P99(List<double> V)
    {
        if (V.Count == 0) { return 0; }
        double[] A = V.ToArray(); Array.Sort(A);
        int i = (int)Math.Ceiling(A.Length * 0.99) - 1;
        if (i < 0) { i = 0; } if (i >= A.Length) { i = A.Length - 1; }
        return A[i];
    }
    static double Dist(double ax, double ay, double az, double bx, double by, double bz)
    {
        double dx = ax - bx, dy = ay - by, dz = az - bz;
        return Math.Sqrt(dx * dx + dy * dy + dz * dz);
    }

    struct LagSample { public LabTrack Track; public double T; public double X; public double Y; public double Z; }

    public static string Analyze(string ServerPath, string[] ClientPaths, string Label, out string CsvRow)
    {
        CultureInfo C = CultureInfo.InvariantCulture;
        LabMotionFile S = LabMotionFile.Read(ServerPath);
        if (S.MeasureStart == 0 || S.MeasureEnd == 0) { throw new Exception("server file has no measure window"); }
        double Spc = S.SecondsPerCycle;
        double Ms = (double)S.MeasureStart;
        double Window = ((double)S.MeasureEnd - Ms) * Spc;

        Dictionary<ulong, LabTrack> Server = new Dictionary<ulong, LabTrack>();
        for (int i = 0; i < S.PCycles.Length; i++)
        {
            LabTrack Tr;
            if (!Server.TryGetValue(S.PGuid[i], out Tr)) { Tr = new LabTrack(); Server.Add(S.PGuid[i], Tr); }
            Tr.Add(((double)S.PCycles[i] - Ms) * Spc, S.PX[i], S.PY[i], S.PZ[i]);
        }
        foreach (LabTrack Tr in Server.Values) { Tr.Freeze(); }

        List<double> PosErr = new List<double>();
        List<double> VelErr = new List<double>();
        List<double> Change = new List<double>();
        List<double> Recv = new List<double>();
        List<double> FrameIntervals = new List<double>();
        List<LagSample> Lag = new List<LagSample>();
        HashSet<ulong> Seen = new HashSet<ulong>();
        int Unmatched = 0;
        StringBuilder PerClient = new StringBuilder();

        foreach (string Path in ClientPaths)
        {
            LabMotionFile F = LabMotionFile.Read(Path);
            int PosBefore = PosErr.Count; int VelBefore = VelErr.Count;
            double PosSumBefore = 0; foreach (double v in PosErr) { PosSumBefore += v; }
            double VelSumBefore = 0; foreach (double v in VelErr) { VelSumBefore += v; }

            // 클라이언트 프레임 간격(같은 프레임의 기록은 시각이 같다).
            double PrevFrame = double.NaN;
            for (int i = 0; i < F.PCycles.Length; i++)
            {
                double t = ((double)F.PCycles[i] - Ms) * Spc;
                if (t == PrevFrame) { continue; }
                if (!double.IsNaN(PrevFrame) && t >= 0 && t <= Window) { FrameIntervals.Add((t - PrevFrame) * 1000.0); }
                PrevFrame = t;
            }

            // NPC마다 기록을 모은다. 기록은 프레임 순서라 NPC마다 시각이 늘어난다.
            Dictionary<ulong, LabTrack> Client = new Dictionary<ulong, LabTrack>();
            for (int i = 0; i < F.PCycles.Length; i++)
            {
                LabTrack Tr;
                if (!Client.TryGetValue(F.PGuid[i], out Tr)) { Tr = new LabTrack(); Client.Add(F.PGuid[i], Tr); }
                Tr.Add(((double)F.PCycles[i] - Ms) * Spc, F.PX[i], F.PY[i], F.PZ[i]);
            }

            foreach (KeyValuePair<ulong, LabTrack> Pair in Client)
            {
                LabTrack Cl = Pair.Value; Cl.Freeze();
                LabTrack Sv;
                if (!Server.TryGetValue(Pair.Key, out Sv)) { Unmatched++; continue; }

                double SegStart = Cl.T.Length > 0 ? Cl.T[0] : 0;
                double LastChange = double.NaN;
                for (int k = 1; k < Cl.T.Length; k++)
                {
                    double t = Cl.T[k], tp = Cl.T[k - 1];
                    if (t - tp > SegmentGap) { SegStart = t; LastChange = double.NaN; continue; }

                    bool Moved = Dist(Cl.X[k], Cl.Y[k], Cl.Z[k], Cl.X[k - 1], Cl.Y[k - 1], Cl.Z[k - 1]) > ChangeEpsilon;
                    bool InWindow = t >= 0 && t <= Window && t - SegStart >= AppearSkip;
                    if (Moved)
                    {
                        if (InWindow && !double.IsNaN(LastChange)) { Change.Add((t - LastChange) * 1000.0); }
                        LastChange = t;
                    }
                    if (!InWindow) { continue; }

                    double sx, sy, sz, px, py, pz;
                    if (!Sv.Eval(t, out sx, out sy, out sz) || !Sv.Eval(tp, out px, out py, out pz)) { continue; }
                    Seen.Add(Pair.Key);
                    PosErr.Add(Dist(Cl.X[k], Cl.Y[k], Cl.Z[k], sx, sy, sz));
                    double dt = t - tp;
                    if (dt > 0.001)
                    {
                        double ex = (Cl.X[k] - Cl.X[k - 1]) - (sx - px);
                        double ey = (Cl.Y[k] - Cl.Y[k - 1]) - (sy - py);
                        double ez = (Cl.Z[k] - Cl.Z[k - 1]) - (sz - pz);
                        VelErr.Add(Math.Sqrt(ex * ex + ey * ey + ez * ez) / dt);
                    }
                    if (PosErr.Count % LagStride == 0 && t - 0.3 >= Sv.T[0])
                    {
                        LagSample L; L.Track = Sv; L.T = t; L.X = Cl.X[k]; L.Y = Cl.Y[k]; L.Z = Cl.Z[k];
                        Lag.Add(L);
                    }
                }
            }

            // 수신 간격.
            Dictionary<ulong, double> LastRecv = new Dictionary<ulong, double>();
            for (int i = 0; i < F.RCycles.Length; i++)
            {
                double t = ((double)F.RCycles[i] - Ms) * Spc;
                double Prev;
                if (LastRecv.TryGetValue(F.RGuid[i], out Prev))
                {
                    double Gap = t - Prev;
                    if (t >= 0 && t <= Window && Gap <= ReceiveGapMax) { Recv.Add(Gap * 1000.0); }
                }
                LastRecv[F.RGuid[i]] = t;
            }

            double PosSum = 0; foreach (double v in PosErr) { PosSum += v; }
            double VelSum = 0; foreach (double v in VelErr) { VelSum += v; }
            int PosN = PosErr.Count - PosBefore, VelN = VelErr.Count - VelBefore;
            PerClient.AppendFormat(C, "client{0}: samples={1} pos_err_avg_cm={2:F2} vel_err_avg_cm_s={3:F1}\r\n", F.Slot, PosN,
                PosN > 0 ? (PosSum - PosSumBefore) / PosN : 0, VelN > 0 ? (VelSum - VelSumBefore) / VelN : 0);
        }

        // 표시 지연: 평균 |p_c(t) - p_s(t - τ)|가 가장 작은 τ.
        double BestTau = 0, BestErr = double.MaxValue;
        Func<double, double> MeanErrAt = delegate (double Tau)
        {
            double Sum = 0; int N = 0;
            foreach (LagSample L in Lag)
            {
                double x, y, z;
                if (L.Track.Eval(L.T - Tau, out x, out y, out z)) { Sum += Dist(L.X, L.Y, L.Z, x, y, z); N++; }
            }
            return N > 0 ? Sum / N : double.MaxValue;
        };
        for (int ms = 0; ms <= 600; ms += 5)
        {
            double E = MeanErrAt(ms / 1000.0);
            if (E < BestErr) { BestErr = E; BestTau = ms; }
        }
        int Lo = Math.Max(0, (int)BestTau - 5), Hi = Math.Min(600, (int)BestTau + 5);
        for (int ms = Lo; ms <= Hi; ms++)
        {
            double E = MeanErrAt(ms / 1000.0);
            if (E < BestErr) { BestErr = E; BestTau = ms; }
        }

        StringBuilder Out = new StringBuilder();
        Out.AppendFormat(C, "label={0}\r\n", Label);
        Out.AppendFormat(C, "measure_window_s={0:F3} clients={1} npcs_seen={2} unmatched_tracks={3} samples={4}\r\n", Window, ClientPaths.Length, Seen.Count, Unmatched, PosErr.Count);
        Out.AppendFormat(C, "pos_err_avg_cm={0:F2} pos_err_p99_cm={1:F2}\r\n", Avg(PosErr), P99(PosErr));
        Out.AppendFormat(C, "vel_err_avg_cm_s={0:F1} vel_err_p99_cm_s={1:F1}\r\n", Avg(VelErr), P99(VelErr));
        Out.AppendFormat(C, "change_interval_avg_ms={0:F1} change_interval_p99_ms={1:F1} (n={2})\r\n", Avg(Change), P99(Change), Change.Count);
        Out.AppendFormat(C, "recv_interval_avg_ms={0:F1} recv_interval_p99_ms={1:F1} (n={2})\r\n", Avg(Recv), P99(Recv), Recv.Count);
        Out.AppendFormat(C, "display_lag_ms={0:F0} (mean pos err at that lag {1:F2} cm, lag samples={2})\r\n", BestTau, BestErr, Lag.Count);
        Out.AppendFormat(C, "client_frame_avg_ms={0:F2} client_frame_p99_ms={1:F2}\r\n", Avg(FrameIntervals), P99(FrameIntervals));
        Out.Append(PerClient.ToString());

        CsvRow = string.Format(C, "{0},{1},{2},{3},{4},{5:F2},{6:F2},{7:F1},{8:F1},{9:F1},{10:F1},{11:F1},{12:F1},{13:F0},{14:F2}",
            Label, DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss", C), ClientPaths.Length, Seen.Count, PosErr.Count,
            Avg(PosErr), P99(PosErr), Avg(VelErr), P99(VelErr), Avg(Change), P99(Change), Avg(Recv), P99(Recv), BestTau, Avg(FrameIntervals));
        return Out.ToString();
    }
}
'@
}

$CsvRow = $null
try {
    $Text = [LabMotionAnalyzer]::Analyze("$Dir\server.bin", [string[]]$ClientFiles, $Label, [ref]$CsvRow)
}
catch {
    Write-Host "FAIL: $($_.Exception.Message)"
    exit 1
}

Set-Content -Path "$Dir\summary.txt" -Value $Text -Encoding UTF8
Write-Host $Text

$Csv = "$ProjectDir\Saved\LabMetrics\motion.csv"
$Header = "label,timestamp,clients,npcs_seen,samples,pos_err_avg_cm,pos_err_p99_cm,vel_err_avg_cm_s,vel_err_p99_cm_s,change_interval_avg_ms,change_interval_p99_ms,recv_interval_avg_ms,recv_interval_p99_ms,display_lag_ms,client_frame_avg_ms"
if (-not (Test-Path $Csv)) {
    Set-Content -Path $Csv -Value $Header -Encoding ASCII
}
if (@(Get-Content $Csv | Where-Object { $_.StartsWith("$Label,") }).Count -gt 0) {
    Write-Host "motion.csv already has a row for $Label. Not appended."
}
else {
    Add-Content -Path $Csv -Value $CsvRow -Encoding ASCII
    Write-Host "appended to $Csv"
}
