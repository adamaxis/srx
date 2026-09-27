# Ghidra conversion workspace

This is the readable-C++ conversion area for behavior recovered from the
canonical Ghidra export. It is intentionally separate from both the native
game and the GenesisBus emulator:

- `decompilation/CMakeLists.txt` does not include this directory.
- It has no dependency on `GenesisBus`, ROM-addressed RAM, VDP objects, or the
  `shadowrun_port` library.
- Its functions use ordinary typed values and fixed-size host containers.
- Source addresses are provenance comments, not runtime lookup keys.

The first slice covers the controller edge expression at `0x195C`, the 64-word
palette operations at `0x1088`/`0x10D6`/`0x1368`, the seventeen-pass
transitions at `0x131C`/`0x12F6`, and the title state shared by `0x1A9E`,
`0x1BAC`, `0x1BEA`, `0x1C1E`, `0x1C6A`, `0x1E4E`, `0x2634`, and `0x26D0`. The `0x1A9E` prompt cadence, `0x1C1E`/`0x1C6A` text/sprite leaf state, and `0x1E4E` logo-column events are typed separately from unresolved writers, projection, and blitters. Vblank
waits, prompt drawing, resource setup, and unresolved hardware calls remain
explicit boundaries until their source inputs are understood.

Build this directory independently when validating the conversion slice. The
presets intentionally use build-ghidra-conversion-* directories and the
shadowrun_ghidra_* target prefix, so this workbench cannot be confused with
the separate native-port builds under decompilation/build-*:

When using VS Code, open the repository-level `shadowrun.code-workspace` file
or the repository folder. CMake Tools exposes this directory as a separate
active project alongside the native-port project. Use `CMake: Select Active
Folder` to choose this directory before selecting its configure/build preset.

```text
C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-debug
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-debug
C:/msys64/ucrt64/bin/ctest.exe --preset ghidra-conversion-debug
```

Run those commands from decompilation/ghidra_conversion. The release variants
are ghidra-conversion-release in each command. To build every conversion test
executable instead of only the workbench runner, use
ghidra-conversion-debug-all or ghidra-conversion-release-all as the build
preset.

The standalone conversion runner can be built and run with:

```text
C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-debug
build-ghidra-conversion-debug/shadowrun_ghidra_conversion_runner.exe --assets ../assets
```

This runner is a conversion-workbench startup probe, not the native game and
not an emulator. It loads the direct-asset alias table, verifies the boot/title
asset set, composes the typed `0x0236 -> 0x073A -> 0x5D746` startup ordering,
and reports the source-derived vblank, sprite, scroll, and resource-stream
counts. Its resource, transfer, palette, audio, and sprite-record callbacks
are explicit conversion boundaries; a `title_ready` result means the parent
control flow and asset requirements were exercised, not that the native port
is playable or that those child systems have been source-verified.

When a callback cannot yet be grounded in the canonical ROM/Ghidra export, it
must remain an explicit reported boundary. Treat an apparently missing field,
resource, or behavior as evidence of an incomplete conversion sweep and
re-audit the source before inventing a replacement routine.

The source-understanding layer is organized by game subsystem under
`include/shadowrun_ghidra/<domain>/`, `src/<domain>/`, and `tests/<domain>/`.
See [`../docs/ghidra-conversion-source-layout.md`](../docs/ghidra-conversion-source-layout.md)
for the domain boundaries and the rule for adding or consolidating files.
