param(
    [string]$VideoPath = 'C:\Github\shadowrun\video analysis\long_gameplay_video_early_sections_ffmpeg_30fps_320x224\section_02_archetype_opening_story_0056-0118\frame_000573.png',
    [string]$OurPath = 'C:\Users\Administrator\AppData\Local\Temp\opening_dump\frame-800.bmp'
)
Add-Type -AssemblyName System.Drawing

function Get-Pixels([string]$path) {
    $bmp = [System.Drawing.Bitmap]::new($path)
    $rect = [System.Drawing.Rectangle]::new(0, 0, $bmp.Width, $bmp.Height)
    $data = $bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $stride = $data.Stride
    $bytes = New-Object byte[] ($stride * $bmp.Height)
    [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
    $bmp.UnlockBits($data)
    $w = $bmp.Width; $h = $bmp.Height
    $bmp.Dispose()
    return @{ Bytes = $bytes; Stride = $stride; W = $w; H = $h }
}

$video = Get-Pixels $VideoPath
$ours = Get-Pixels $OurPath

# 8x8-cell region grid (40x28 px cells), report mean gray diff per cell.
Write-Output 'region diffs (cell = 40x28, mean abs gray diff):'
for ($cy = 0; $cy -lt 8; $cy++) {
    $line = ''
    for ($cx = 0; $cx -lt 8; $cx++) {
        $sum = 0
        for ($y = $cy * 28; $y -lt ($cy + 1) * 28; $y += 2) {
            for ($x = $cx * 40; $x -lt ($cx + 1) * 40; $x += 2) {
                $vo = $y * $video.Stride + $x * 4
                $oo = $y * $ours.Stride + $x * 4
                $vg = ($video.Bytes[$vo] + $video.Bytes[$vo + 1] + $video.Bytes[$vo + 2]) / 3
                $og = ($ours.Bytes[$oo] + $ours.Bytes[$oo + 1] + $ours.Bytes[$oo + 2]) / 3
                $d = $vg - $og
                if ($d -lt 0) { $d = -$d }
                $sum += $d
            }
        }
        $avg = [int]($sum / 280)
        $line += ('{0,4} ' -f $avg)
    }
    Write-Output $line
}
