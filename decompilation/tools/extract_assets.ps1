[CmdletBinding()]
param(
    [string]$RomPath,
    [string]$BuildDir,
    [string]$OutputRoot
)

$ErrorActionPreference = 'Stop'
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($RomPath)) {
    $RomPath = Join-Path $scriptRoot '..\input\shadowrun_genesis_us.bin'
}
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $scriptRoot '..\build-debug'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $scriptRoot '..\assets'
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $scriptRoot '..')).Path
$rom = (Resolve-Path -LiteralPath $RomPath).Path
$extractor = Join-Path (Resolve-Path -LiteralPath $BuildDir).Path 'shadowrun_graphics_extract.exe'

if (-not (Test-Path -LiteralPath $extractor -PathType Leaf)) {
    throw "Extractor not found: $extractor. Build shadowrun_graphics_extract first."
}

$folders = @(
    'title', 'startup', 'game\field', 'game\menu', 'game\items',
    'matrix\menu', 'audio', 'native'
)
foreach ($folder in $folders) {
    New-Item -ItemType Directory -Force -Path (Join-Path $OutputRoot $folder) | Out-Null
}

function Invoke-Extraction {
    param([string[]]$Arguments)
    & $extractor @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Asset extraction failed with exit code $LASTEXITCODE."
    }
}

Invoke-Extraction @($rom, (Join-Path $OutputRoot 'title\title-static.bmp'), '--title')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'title\title-frame-0.bmp'), '--native-title-frame', '0')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'title\title-frame-420.bmp'), '--native-title-frame', '420')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'startup\intro.bmp'), '--native-intro')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'startup\start-options.bmp'), '--native-start-options')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'startup\save-load.bmp'), '--native-save-load')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'startup\no-save.bmp'), '--native-no-save')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'startup\archetype.bmp'), '--native-archetype')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'game\field\field.bmp'), '--native-field')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'game\menu\base.bmp'), '--native-field-menu')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'game\menu\inventory.bmp'), '--native-field-menu-inventory')
Invoke-Extraction @($rom, (Join-Path $OutputRoot 'native\shadowrun_native.srna'), '--native-pack')

Write-Host "Extracted Shadowrun assets into $((Resolve-Path -LiteralPath $OutputRoot).Path)"
Write-Host "Provenance: $(Join-Path $repoRoot 'docs\resource-catalog.tsv')"
