$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..\..")).Path
$Requirements = Join-Path $PSScriptRoot "requirements-build.txt"
$EntryPoint = Join-Path $PSScriptRoot "map_entity_viewer.py"
$OutputDirectory = Join-Path $PSScriptRoot "dist"
$WorkDirectory = Join-Path $RepoRoot "build\map-entity-viewer\pyinstaller-work"
$SpecDirectory = Join-Path $RepoRoot "build\map-entity-viewer\pyinstaller-spec"

if (-not (Test-Path (Join-Path $RepoRoot "decompilation\assets\field\maps\index.tsv"))) {
    throw "Could not locate the Shadowrun checkout from the viewer folder: $RepoRoot"
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
python -m pip install -r $Requirements
if ($LASTEXITCODE -ne 0) { throw "Could not install the map viewer build requirements." }

python -m PyInstaller `
    --noconfirm `
    --clean `
    --onefile `
    --windowed `
    --name shadowrun_map_entity_viewer `
    --distpath $OutputDirectory `
    --workpath $WorkDirectory `
    --specpath $SpecDirectory `
    $EntryPoint
if ($LASTEXITCODE -ne 0) { throw "PyInstaller failed to build the map viewer." }

Write-Host "Built standalone viewer: $(Join-Path $OutputDirectory 'shadowrun_map_entity_viewer.exe')"
