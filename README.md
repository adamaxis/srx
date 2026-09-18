# shadowrun

## Visual Studio Code build

Open `C:\Github\shadowrun` as the VS Code workspace. CMake Tools uses the
presets in `decompilation/CMakePresets.json`:

- select `Debug (MSYS2 UCRT64)` and `Build Native (Debug)` for development;
- select `Release (MSYS2 UCRT64)` and `Build Native (Release)` for release;
- use `Run and Debug` -> `Shadowrun Native (Debug)` to build and launch; or
- run the `Shadowrun: Test (Debug)` task to execute CTest.

Building `shadowrun_native` also regenerates `shadowrun_native.srna` from the
ROM in `decompilation/input`. The VS Code configuration uses the MSYS2 UCRT64
tools in `C:\msys64\ucrt64\bin`.
