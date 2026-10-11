# Shadowrun Ghidra Conversion

The active Shadowrun implementation is the typed C++/Qt project in
`decompilation/ghidra_conversion/`. The outer `decompilation/` directory is a
shared source-data and research root; it also still contains an older native
port that is not the current development target.

## Active project

- `decompilation/ghidra_conversion/include/`, `src/`, and `tests/` contain the
  source-derived conversion model and checks.
- `decompilation/ghidra_conversion/qt_runtime/` contains the Qt executable and
  its adapters.
- `decompilation/input/`, `decompilation/export/`, and
  `decompilation/assets/` provide the ROM, Ghidra evidence, and individually
  named extracted assets consumed by the active project.
- `decompilation/gameplay/scripts/` contains Lua content loaded by the Qt
  runtime. `decompilation/docs/` and `video analysis/` contain research and
  visual references.
- `decompilation/ghidra_conversion/tools/map_entity_viewer/` is the standalone
  Qt map and entity authoring app.

The older native-port sources are under `decompilation/include/`,
`decompilation/src/`, and `decompilation/tests/`. They are retained for
reference and are not the project to build or extend for current work.

## Build and run the Release Qt app

Open `shadowrun.code-workspace` in VS Code. Its CMake Tools project selector,
build tasks, and launch configurations point only to the Ghidra conversion.
The root CMake umbrella and the legacy native-port presets are not part of the
normal workflow.

From `decompilation/ghidra_conversion/`:

```powershell
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
../../build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets ../assets
```

The canonical Release executable is
`build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe`.
Debug output is kept separately under `build/conversion/qt/debug/`.

## Standalone map and entity viewer

The viewer has its own source, dependencies, build launcher, and executable
folder under `decompilation/ghidra_conversion/tools/map_entity_viewer/`. Run
`build.cmd` there to create its standalone Windows executable in `dist/`; see
that folder's README for Python launch and authoring details.

## Workspace layout

`build/conversion/` holds active Ghidra Release and Debug outputs. `build/` also
contains `archive/` for earlier binaries and QA captures; those archived files
are retained as evidence and are not active executables. See
[`decompilation/WORKSPACE.md`](decompilation/WORKSPACE.md) for ownership and
path conventions.
