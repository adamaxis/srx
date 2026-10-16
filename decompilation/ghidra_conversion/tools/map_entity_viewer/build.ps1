$ErrorActionPreference = "Stop"

$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$SharedRoot = (Resolve-Path (Join-Path $ProjectRoot "..")).Path
$Requirements = Join-Path $PSScriptRoot "requirements-build.txt"
$EntryPoint = Join-Path $PSScriptRoot "map_entity_viewer.py"
$OutputDirectory = Join-Path $PSScriptRoot "dist"
$WorkDirectory = Join-Path $ProjectRoot "build\map-entity-viewer\pyinstaller-work"
$SpecDirectory = Join-Path $ProjectRoot "build\map-entity-viewer\pyinstaller-spec"

if (-not (Test-Path (Join-Path $SharedRoot "assets\field\maps\index.tsv"))) {
    throw "Could not locate shared Shadowrun assets beside the project folder: $SharedRoot"
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
