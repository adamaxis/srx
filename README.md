# shadowrun

## Visual Studio Code build

Open `C:\Github\shadowrun\shadowrun.code-workspace` in VS Code, or open
`C:\Github\shadowrun` as a folder. CMake Tools is configured with two
independent source directories in the same workspace: the native port uses the
presets in `decompilation/CMakePresets.json`, and the Ghidra conversion uses
the presets in `decompilation/ghidra_conversion/CMakePresets.json`. The
conversion project is not a native-port target or a runtime dependency.

For the native port:

- select `Debug (MSYS2 UCRT64)` and `Build Native (Debug)` for development;
- select `Release (MSYS2 UCRT64)` and `Build Native (Release)` for release;
- use `Run and Debug` -> `Shadowrun Native (Debug)` to build and launch; or
- run the `Shadowrun: Test (Debug)` task to execute CTest.

For the standalone Ghidra conversion, use `CMake: Select Active Folder` (the
folder/project selector in the CMake Tools status bar) and choose
`decompilation/ghidra_conversion`, then:

- select `Ghidra Conversion Workbench (Debug, MSYS2 UCRT64)` and
  `Build Ghidra Conversion Workbench (Debug)`; or
- use `Run and Debug` -> `Shadowrun Ghidra Conversion Workbench (Debug)` to
  build and execute it.

The native frontends consume the checked-in individually named assets under
`decompilation/assets/` plus `assets/native/state.bin`; extraction is an
explicit tool operation and is not part of a normal compile. The Qt Widgets frontend is
available through the `Shadowrun: Run Qt Frontend (Debug)` task. The VS Code
configuration uses the MSYS2 UCRT64 tools in `C:\msys64\ucrt64\bin` and the
installed Qt kit in `C:\Qt\6.9.1\mingw_64`.

The Qt frontend deliberately uses one focused custom `QWidget` as the game
surface. It paints the extracted 320x224 framebuffer with nearest-neighbor
scaling and forwards keyboard edges into the same C++ `InputState` used by the
tests. Native Qt buttons would introduce platform chrome and a second menu
state machine, so the Genesis panels, focus blocks, inventory cursor, and
timing remain source-rendered inside the widget.
