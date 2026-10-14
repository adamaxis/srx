# Shadowrun Ghidra Conversion

The repository root is the active Ghidra reconstruction project. Its C++/Qt
host application and Lua content are the current implementation; the retired
native port and Ghidra database are preserved under `archive/`.

## Project layout

- `include/`, `src/`, and `qt_runtime/` contain the typed C++ model and Qt
  application.
- `gameplay/scripts/` contains Lua gameplay, dialogue, menu, and event content.
- `assets/` contains named runtime assets. `input/` holds the US Genesis ROM,
  while `export/` holds Ghidra-derived evidence.
- `docs/ghidra/` contains source and behavior audits; `docs/handoffs/` contains
  project handoffs.
- `tools/map_entity_viewer/` is the standalone Qt map and entity viewer.
- `tools/rom_research/` contains offline ROM/Ghidra research tools.
- `video analysis/` remains the gameplay-video and frame-analysis archive.
- `archive/legacy_native_port/` preserves the inactive native-port source.
  `archive/ghidra_analysis_project/` preserves the local Ghidra database.

## Build and run the Release Qt app

Open `shadowrun.code-workspace` in VS Code, or run these commands from the
repository root:

```powershell
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets assets
```

The canonical Release executable is
`build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe`. Debug output
is kept separately under `build/conversion/qt/debug/`.

## Standalone map and entity viewer

Run `python tools/map_entity_viewer/map_entity_viewer.py` from the repository
root. To package the standalone Windows application, run
`tools/map_entity_viewer/build.cmd`; its ignored `dist/` output stays beside
the viewer source.

## Build output

`build/conversion/` contains the active Ghidra workbench and Qt Release/Debug
builds. Other archived build outputs are disposable; source and research
archives live under `archive/`.
