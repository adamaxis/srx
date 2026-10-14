# Ghidra conversion project

The repository root is the active C++/Qt project for behavior recovered from
the canonical ROM and Ghidra export. Its functions use typed host values and
fixed-size containers; source addresses are provenance, not runtime lookup
keys. The legacy native port is archived under
archive/legacy_native_port/ and is not part of current development.

## Layout

- include/, src/, qt_runtime/, and tests/ contain the active typed conversion
  model, Qt host, and QA targets.
- gameplay/scripts/ contains Lua behavior and content.
- input/, export/, and assets/ contain the ROM, Ghidra evidence, and
  individually named runtime assets.
- docs/ghidra/ contains behavior audits, screen records, and hill status.
- tools/rom_research/ contains offline extraction and source-analysis tools.
- tools/map_entity_viewer/ contains the standalone Qt map/entity viewer.

The Qt application consumes extracted assets directly. It does not run a
68000 interpreter, emulate a VDP, or use ROM-addressed host memory. Offline
extractors may decode source resources into stable host assets; runtime code
uses those files.

## Release build

Run from the repository root:

    C:/msys64/ucrt64/bin/cmake.exe --preset ghidra-conversion-qt-release
    C:/msys64/ucrt64/bin/cmake.exe --build --preset ghidra-conversion-qt-release
    build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --assets assets

The canonical Release output is
build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe. Debug output
is under build/conversion/qt/debug/; workbench output is under
build/conversion/workbench/{debug,release}/.

VS Code and shadowrun.code-workspace use the root CMake project and its
presets. Run the corresponding ctest --preset ... command when a completed
behavior hill requires its Debug and Release CTest verification.

## Map/entity viewer

Run python tools/map_entity_viewer/map_entity_viewer.py --scene 2 from the
repository root. The viewer reads assets/field/ and authors map entity data
under gameplay/world/. Its source README documents editing and packaging.

## Source and verification boundaries

The US ROM at input/shadowrun_genesis_us.bin is primary. Use export/ as a
Ghidra cross-check, and use the independent gameplay evidence under
video analysis/ for visual corroboration. Internal Qt dumps are useful for
regression checks but do not independently establish Genesis fidelity.

See [the subsystem ownership guide](ghidra/ghidra-conversion-source-layout.md)
and [the hill index](ghidra/HILLS.md).
