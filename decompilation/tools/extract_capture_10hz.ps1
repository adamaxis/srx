# Extract screen-capture.webm at 10 Hz (every 0.1s) cropped to the player iframe.
# Usage: from repo root or this directory.
param(
    [string]$Webm = "",
    [string]$OutDir = "",
    [double]$Seconds = 175
)

$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
if (-not $Webm) { $Webm = Join-Path $Root "screen-capture.webm" }
if (-not $OutDir) {
    $OutDir = Join-Path $Root "decompilation\webm_ffmpeg_microframes"
}
$Ffmpeg = Join-Path $PSScriptRoot "ffmpeg\ffmpeg-9.0.1-essentials_build\bin\ffmpeg.exe"
if (-not (Test-Path $Ffmpeg)) {
    $Ffmpeg = "ffmpeg"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
# Player iframe measured from a 3698x2064 retrogames still.
& $Ffmpeg -y -i $Webm -t $Seconds `
    -vf "crop=2666:1706:40:242,scale=320:224:flags=lanczos,fps=10" `
    (Join-Path $OutDir "t%04d.png")
Write-Host "wrote" (Get-ChildItem $OutDir -Filter t*.png).Count "frames to $OutDir"
