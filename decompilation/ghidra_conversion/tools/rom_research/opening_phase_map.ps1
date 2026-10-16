param(
    [string]$VideoDir = 'C:\Github\shadowrun\video analysis\long_gameplay_video_early_sections_ffmpeg_30fps_320x224\section_02_archetype_opening_story_0056-0118',
    [string]$OurDir = 'C:\Users\Administrator\AppData\Local\Temp\opening_dump',
    [int]$OurStart = 700,
    [int]$OurEnd = 950,
    [int]$VideoStart = 500,
    [int]$VideoEnd = 700
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

Write-Output "loading video window signatures..."
$videoSig = @{}
for ($v = $VideoStart; $v -le $VideoEnd; $v++) {
    $name = 'frame_{0:D6}.png' -f $v
    $videoSig[$v] = Get-Signature (Join-Path $VideoDir $name)
}

Write-Output "mapping our frames every 10..."
for ($o = $OurStart; $o -le $OurEnd; $o += 10) {
    $name = 'frame-{0}.bmp' -f $o
    $path = Join-Path $OurDir $name
    if (-not (Test-Path $path)) { continue }
    $sig = Get-Signature $path
    $best = -1; $bestD = [int64]::MaxValue
    foreach ($v in $videoSig.Keys) {
        $d = Compare-Sig $sig $videoSig[$v]
        if ($d -lt $bestD) { $bestD = $d; $best = $v }
    }
    Write-Output ("ours {0} -> video {1} diff {2}" -f $o, $best, $bestD)
}
