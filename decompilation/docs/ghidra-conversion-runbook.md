# Ghidra conversion project

The active Qt project is `decompilation/ghidra_conversion/`. The surrounding
`decompilation/` directory holds the shared ROM, Ghidra exports, runtime
assets, gameplay scripts and world data, and implementation documentation.
The native port remains archived under `archive/legacy_native_port/`.

## Layout

- `decompilation/ghidra_conversion/include/`, `src/`, `qt_runtime/`, and
  `tests/` contain the active typed model, Qt host, and QA targets.
- `decompilation/gameplay/scripts/` contains Lua behavior and content.
- `decompilation/input/`, `export/`, and `assets/` contain the US ROM, Ghidra
  evidence, and individually named runtime assets.
- `decompilation/docs/ghidra/` contains behavior audits, screen records, and
  hill status.
- `decompilation/ghidra_conversion/tools/` contains offline extraction,
  source-analysis, and map-viewer tools.

The Qt application consumes extracted assets directly. It does not run a
68000 interpreter, emulate a VDP, or use ROM-addressed host memory. Offline
extractors may decode source resources into stable host assets; runtime code
uses those files.

## Release build

Run from the repository root:

```powershell
Push-Location decompilation/ghidra_conversion
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
./build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets ../assets
Pop-Location
```

The canonical Release output is
`decompilation/ghidra_conversion/build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe`.
Debug output is under
`decompilation/ghidra_conversion/build/conversion/qt/debug/`; workbench output
is under `decompilation/ghidra_conversion/build/conversion/root-workbench/`.
VS Code and `shadowrun.code-workspace` point at this project's CMake presets.
Run the matching Debug and Release CTest presets for each completed behavior
hill.

## Map/entity viewer

Run
`python decompilation/ghidra_conversion/tools/map_entity_viewer/map_entity_viewer.py --scene 2`
from the repository root. The viewer reads `decompilation/assets/field/` and
authors map entity data under `decompilation/gameplay/world/`. Its source
README documents editing and packaging.

## Source and verification boundaries

The US ROM at `decompilation/input/shadowrun_genesis_us.bin` is primary. Use
`decompilation/export/` as a Ghidra cross-check, and use the independent
gameplay evidence under `video analysis/` for visual corroboration. Internal
Qt dumps are useful for regression checks but do not independently establish
Genesis fidelity.

See [the subsystem ownership guide](ghidra/ghidra-conversion-source-layout.md)
and [the hill index](ghidra/HILLS.md).
