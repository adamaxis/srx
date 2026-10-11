$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Requirements = Join-Path $PSScriptRoot "requirements-map-viewer-build.txt"
$EntryPoint = Join-Path $PSScriptRoot "map_entity_viewer.py"
$OutputDirectory = Join-Path $RepoRoot "build\map-entity-viewer"
$WorkDirectory = Join-Path $OutputDirectory "pyinstaller-work"
$SpecDirectory = Join-Path $OutputDirectory "pyinstaller-spec"

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
