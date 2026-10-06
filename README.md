# shadowrun

## Repository map

- `decompilation/input/` — canonical ROM input used by the tools.
- `decompilation/export/` — Ghidra exports and trace evidence; not runtime code.
- `decompilation/include/shadowrun/<domain>/`, `src/<domain>/`, and
  `tests/<domain>/` — native-port contracts, implementations, and checks,
  grouped by audio, core, entities, field, menus, rendering, session, and
  title/intro ownership.
- `decompilation/ghidra_conversion/` — separate typed conversion workbench,
  with gameplay-domain modules under `include/shadowrun_ghidra/`, `src/`, and
  `tests/`.
- `decompilation/ghidra_conversion/qt_runtime/` — Qt host adapters and their
  audio, dialogue, field, menu, and rendering tests, organized by screen/system
  ownership.
- `decompilation/assets/` — named host assets consumed directly by the
  executables. The offline importer is `decompilation/tools/`.
- `decompilation/docs/` — source maps, screen audits, hill status, and visual
  evidence. `video analysis/` contains the independent gameplay capture and
  its reference frames.
- `build/` — the only CMake output root. `build/archive/` holds older build
  trees and their generated captures; active presets write under `build/ide/`,
  `build/native/`, and `build/conversion/`.

See [`decompilation/WORKSPACE.md`](decompilation/WORKSPACE.md) for the full
directory ownership map and naming rules.

## Visual Studio Code build

Open `C:\Github\shadowrun\shadowrun.code-workspace` in VS Code, or open
`C:\Github\shadowrun` as a folder. CMake Tools is configured with two
independent source directories in the same workspace: the native port uses the
presets in `decompilation/CMakePresets.json`, and the Ghidra conversion uses
the presets in `decompilation/ghidra_conversion/CMakePresets.json`. The
conversion project is not a native-port target or a runtime dependency.

For the native port:

- use `Release (MSYS2 UCRT64)` and `Build Native (Release)` as the normal
  build;
- use `Run and Debug` -> `Shadowrun Native (Release)` to build and launch; or
- run `Shadowrun: Test (Release)` for the normal CTest pass. Debug builds and
  tests remain available for diagnosis and configuration coverage.
- the Qt frontend's normal build/run tasks are `Build Qt Frontend (Release)`
  and `Run Qt Frontend (Release)`.

For the standalone Ghidra conversion, use `CMake: Select Active Folder` (the
folder/project selector in the CMake Tools status bar) and choose
`decompilation/ghidra_conversion`, then:

- select `Ghidra Conversion Qt (Release, MSYS2 UCRT64)` and use the Release
  Qt build/run task; or
- select `Ghidra Conversion Workbench (Release, MSYS2 UCRT64)` for the
  standalone conversion runner. Debug presets remain available for diagnosis.

The native frontends consume the checked-in individually named assets under
`decompilation/assets/` plus `decompilation/assets/native/state.bin`; extraction is an
explicit tool operation and is not part of a normal compile. The Qt Widgets frontend is
available through the `Shadowrun: Run Qt Frontend (Release)` task. The VS
Code configuration uses the MSYS2 UCRT64 tools in `C:\msys64\ucrt64\bin` and
the installed Qt kit in `C:\Qt\6.9.1\mingw_64`.

The Qt frontend deliberately uses one focused custom `QWidget` as the game
surface. It paints the extracted 320x224 framebuffer with nearest-neighbor
scaling and forwards keyboard edges into the same C++ `InputState` used by the
tests. Native Qt buttons would introduce platform chrome and a second menu
state machine, so the Genesis panels, focus blocks, inventory cursor, and
timing remain source-rendered inside the widget.

## Full Visual Studio CMake build

If using Visual Studio 2022 rather than VS Code, open the repository folder as
a CMake project. The root `CMakeLists.txt` is an IDE-only umbrella: it keeps
`decompilation/` and `decompilation/ghidra_conversion/` as separate source
projects while exposing both target families to Visual Studio's build and
startup-item bars.

Choose the `Shadowrun IDE (Release, MSYS2 UCRT64)` configure preset, then choose
`Build Ghidra Conversion (IDE Release)`. The startup-item list contains
`shadowrun_ghidra_conversion_runner`; its debugger arguments already point at
`decompilation/assets`. The root umbrella does not link the conversion into
the native port or emulator.
