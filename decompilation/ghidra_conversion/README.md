# Shadowrun Ghidra Conversion

The active typed C++/Qt project lives here. Shared ROM evidence and game data
live in the parent `decompilation/` directory.

## Layout

- `include/`, `src/`, `qt_runtime/`, `tests/`, and `third_party/` contain the
  C++ model, Qt host, QA, and dependencies.
- `../gameplay/scripts/` contains Lua gameplay, dialogue, menu, and event
  content; `../gameplay/world/` contains map data.
- `../assets/` contains named runtime assets, `../input/` holds the US Genesis
  ROM, and `../export/` holds Ghidra source exports.
- `../docs/ghidra/` contains source audits and implementation status.
- `tools/map_entity_viewer/` is the standalone Qt map and entity viewer;
  `tools/rom_research/` contains offline research tools.
- The inactive native port and local Ghidra database are preserved in
  `../../archive/`.
- Gameplay video and numbered frame analysis remain in
  `../../video analysis/`.

## Build and run the Release Qt app

Run these commands from this directory:

```powershell
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets ../assets
```

The canonical Release executable is
`build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe`. Debug output
is kept separately under `build/conversion/qt/debug/`.

## Standalone map and entity viewer

Run `python tools/map_entity_viewer/map_entity_viewer.py` from this directory.
To package the standalone Windows application, run
`tools/map_entity_viewer/build.cmd`; its ignored `dist/` output stays beside
the viewer source.

## Build output

`build/conversion/` contains the active Ghidra workbench and Qt Release/Debug
builds. Other archived build outputs are disposable; source and research
archives live under `../../archive/`.
