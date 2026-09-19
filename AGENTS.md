# Shadowrun Native Port Instructions

## Objective

Reconstruct US Genesis Shadowrun as a native C++ Windows game. Qt (or GDI)
presents a 320x224 software framebuffer. The finished program runs ordinary
C++ game logic with extracted host assets. A pile of Ghidra C, a disassembly,
or a test-only bus harness is not the game.

## Non-goals

- Do not ship a 68000 interpreter, Genesis emulator, or VDP as the runtime.
- `GenesisBus` is a reconstruction/verification museum. Do not grow it unless
  a hill lifts a proven leaf into `NativePortShell`.
- Do not invent screens, archetypes, buildings, or combat to fill a hole.
- Do not claim visual correctness from a compositor of our own `.srna` pack.

## Sources of truth

1. **Primary:** the US ROM (`Shadow Run (U) [!].bin` /
   `decompilation/input/shadowrun_genesis_us.bin`). Jump to the screen entry,
   follow tables, RAM, and callees.
2. **Secondary:** Ghidra C under `decompilation/export-seeded/`. Cross-check
   only. Never copy it in as the native implementation.
3. **Tertiary:** real-game visuals. `screen-capture.webm` is storyboard and
   timing. Pixel goldens must be integer-scale 320x224 captures. Until a
   golden exists, dump the native framebuffer BMP and look at it.

## Hill rule (take and defend)

One hill at a time. A hill is one screen or one shared mechanism.

A hill is **defended** only when all of these exist:

1. ROM entry, cursor/RAM, draw path, and exit traced.
2. Typed native state in `NativePortShell` (not a Qt widget machine).
3. A CTest that fails if that behavior or those pixels regress.
4. A CLI framebuffer dump (`actual.bmp`) for that scene.

`pending` = entry known, not reconstructed.  
`partial` = subpath native, not fully defended.  
`defended` = named test + dump. Record this in
`decompilation/docs/decompiled-function-status.tsv` (one ROM entry per row).

If a test only re-encodes the same assets the renderer uses, it does not
defend the hill. Leftover-clear tests (return to a screen, pixels match a
fresh instance) defend the garbled-text class without goldens.

Progress index: `decompilation/docs/HILLS.md`.  
Screen map: `decompilation/docs/screen-system-audit.md` and
`decompilation/docs/reconstruction-exploratory-analysis.md`.

## Runtime shape

- CMake target `shadowrun_native_pack` runs `shadowrun_graphics_extract`
  `--native-pack` whenever the native/Qt exe is built, writing
  `decompilation/assets/native/shadowrun_native.srna`. Do not attach
  extraction to CTest as an extra per-test step.
- Native exe and Qt exe consume `decompilation/assets/native/shadowrun_native.srna`
  only. They must not open the ROM.
- Genesis 4-bit tiles and 9-bit CRAM become RGBA8 at extraction. Preserve the
  look, not VRAM limits.
- Host may run at 60 FPS. Animation cadence still follows ROM counters.
- Qt is one `QWidget` that blits the native framebuffer with nearest-neighbor
  scaling and forwards keys into `InputState`. No second menu state machine.
- Unresolved indirect calls stay explicit boundaries.

## Build

C++17, existing CMake presets under `decompilation/`. Debug + Release CTest
for each completed hill. Visual dump:

```text
shadowrun_visual_dump.exe <pack.srna> <out-dir> --scene title --frames 0,90,128,500
```

Do not describe the project as playable until a native/Qt executable accepts
controls and produces visible game graphics that a defended hill covers.
