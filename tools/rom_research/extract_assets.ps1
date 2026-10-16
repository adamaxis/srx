[CmdletBinding()]
param(
    [string]$RomPath,
    [string]$BuildDir,
    [string]$OutputRoot
)

$ErrorActionPreference = 'Stop'
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $scriptRoot '..\..')).Path
if ([string]::IsNullOrWhiteSpace($RomPath)) {
    $RomPath = Join-Path $repoRoot 'input\shadowrun_genesis_us.bin'
}
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot 'build\conversion\qt\release'
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $repoRoot 'assets'
}

$rom = (Resolve-Path -LiteralPath $RomPath).Path
$extractor = Join-Path (Resolve-Path -LiteralPath $BuildDir).Path 'shadowrun_graphics_extract.exe'

if (-not (Test-Path -LiteralPath $extractor -PathType Leaf)) {
    throw "Extractor not found: $extractor. Build shadowrun_graphics_extract first."
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

# The importer owns the complete direct-asset tree. It writes individually
# named host resources and native/state.bin; it must never create an aggregate
# serializer pack or raw audio-fragment directory.
& $extractor $rom $OutputRoot --all-assets
if ($LASTEXITCODE -ne 0) {
    throw "Direct asset extraction failed with exit code $LASTEXITCODE."
}

Write-Host "Extracted Shadowrun assets into $((Resolve-Path -LiteralPath $OutputRoot).Path)"
Write-Host "Provenance: $(Join-Path $repoRoot 'docs\resource-catalog.tsv')"
