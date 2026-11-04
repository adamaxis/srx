# Shadowrun Genesis reconstruction

The active Ghidra/Qt project is in [`decompilation/ghidra_conversion`](decompilation/ghidra_conversion/README.md).
Shared ROM input, Ghidra exports, runtime assets, Lua content, and research
notes are under [`decompilation/`](decompilation/).

Open `shadowrun.code-workspace` in VS Code, or build the Release Qt app from
`decompilation/ghidra_conversion/`:

```powershell
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets ../assets
```

The retired native port was removed; git history keeps it. The asset importer's
sources live in `decompilation/ghidra_conversion/asset_import/`.
Gameplay video and numbered frame analysis remain in `video analysis/`.
