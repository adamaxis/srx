param(
    [string]$VideoDir = 'C:\Github\shadowrun\video analysis\long_gameplay_video_early_sections_ffmpeg_30fps_320x224\section_02_archetype_opening_story_0056-0118',
    [string]$OurDir = 'C:\Users\Administrator\AppData\Local\Temp\opening_dump',
    [int]$VideoCount = 3600,
    [int]$OurCount = 2600
)
Add-Type -AssemblyName System.Drawing

function Get-Signature([string]$path) {
    $bmp = [System.Drawing.Bitmap]::new($path)
    $rect = [System.Drawing.Rectangle]::new(0, 0, $bmp.Width, $bmp.Height)
    $data = $bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $stride = $data.Stride
    $bytes = New-Object byte[] ($stride * $bmp.Height)
    [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
    $bmp.UnlockBits($data)
    $w = $bmp.Width; $h = $bmp.Height
    $bmp.Dispose()
    $sig = New-Object byte[] (80 * 56)
    $i = 0
    for ($y = 0; $y -lt $h; $y += 4) {
        for ($x = 0; $x -lt $w; $x += 4) {
            $o = $y * $stride + $x * 4
            $b = $bytes[$o]; $g = $bytes[$o + 1]; $r = $bytes[$o + 2]
            $sig[$i] = [byte]((($r + $g + $b) / 3) -band 0xFF)
            $i++
        }
    }
    return $sig
}

function Compare-Sig($a, $b) {
    $d = 0
    for ($i = 0; $i -lt $a.Length; $i++) {
        $v = [int]$a[$i] - [int]$b[$i]
        if ($v -lt 0) { $v = -$v }
        $d += $v
    }
    return $d
}

Write-Output "collecting video signatures..."
$videoSig = New-Object 'System.Collections.Generic.List[byte[]]'
for ($n = 0; $n -lt $VideoCount; $n++) {
    $name = 'frame_{0:D6}.png' -f $n
    $path = Join-Path $VideoDir $name
    if (-not (Test-Path $path)) { break }
    $videoSig.Add((Get-Signature $path))
    if (($n % 300) -eq 0) { Write-Output ("  video {0}" -f $n) }
}
Write-Output ("video frames: {0}" -f $videoSig.Count)

Write-Output "collecting our signatures..."
$ourSig = New-Object 'System.Collections.Generic.List[byte[]]'
for ($n = 0; $n -lt $OurCount; $n++) {
    $name = 'frame-{0}.bmp' -f $n
    $path = Join-Path $OurDir $name
    if (-not (Test-Path $path)) { break }
    $ourSig.Add((Get-Signature $path))
    if (($n % 250) -eq 0) { Write-Output ("  ours {0}" -f $n) }
}
Write-Output ("our frames: {0}" -f $ourSig.Count)

Write-Output "aligning sample frames..."
$samples = @()
foreach ($s in @(0, 50, 100, 150, 200, 250, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500)) {
    if ($s -ge $ourSig.Count) { continue }
    $best = -1; $bestD = [int64]::MaxValue
    for ($v = 0; $v -lt $videoSig.Count; $v++) {
        $d = Compare-Sig $ourSig[$s] $videoSig[$v]
        if ($d -lt $bestD) { $bestD = $d; $best = $v }
    }
    $samples += [pscustomobject]@{ Ours = $s; Video = $best; Diff = $bestD }
    Write-Output ("  ours {0} -> video {1} (diff {2})" -f $s, $best, $bestD)
}
$samples | Export-Csv "$OurDir\alignment.csv" -NoTypeInformation
Write-Output "alignment written to alignment.csv"
