# Shadowrun Native Port Instructions

## Objective

Reconstruct US Genesis Shadowrun as a native x86 C++ Windows game. Qt (or GDI)
presents the game using extracted host assets and ordinary native game logic.
The presentation size is chosen to reproduce the supplied reference images;
Genesis resolution and VDP limits are not runtime requirements. The finished
program is efficient, modern Qt/C++ (or a suitable Qt-compatible game engine),
not a pile of Ghidra C, a disassembly, or a test-only bus harness.

The ROM and Ghidra output are de-engineering references. They explain how the
original game behaves, how its systems interact, how screens are arranged, and
which assets and timings produce the observed result. They are not a runtime
implementation specification to be copied literally. Reconstruct the
behavior in clear native C++ abstractions and choose the simplest appropriate
host representation.

Before adding more emulation-shaped code, de-engineer what has already been
constructed: identify runtime code that reproduces bytecode, VDP/name-table
operations, ROM address arithmetic, or source-specific drawing helpers; retain
the useful behavioral knowledge and one-time extraction data, then replace the
runtime machinery with explicit native state, assets, layouts, and systems.

## Non-goals

- Do not ship a 68000 interpreter, Genesis emulator, or VDP as the runtime.
- `GenesisBus` is a reconstruction/verification museum. Do not grow it unless
  a hill lifts a proven leaf into `NativePortShell`.
- Do not implement menus or game systems as bytecode copies, ROM-address
  interpreters, VDP simulations, or generic runtime "decode" pipelines. A
  decoder may exist in an offline extractor to turn source resources into host
  assets; the game runtime consumes those assets directly.
- Do not invent screens, archetypes, buildings, or combat to fill a hole.
- Do not claim visual correctness from a compositor of the same extracted
  assets used by the renderer.

## Sources of truth

1. **Primary:** the US ROM (`Shadow Run (U) [!].bin` /
   `decompilation/input/shadowrun_genesis_us.bin`). Jump to the screen entry,
   follow tables, RAM, and callees.
2. **Secondary:** Ghidra C under `decompilation/export/`. Cross-check
   only. Use it to understand behavior and validate the ROM trace; never copy
   it in as the native implementation.
3. **Tertiary:** real-game visuals. The current visual sanity check and timing
   reference is `video analysis/long_gameplay_video.mp4` and its one-time
   `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`
   FFmpeg dump, which contains the numbered analysis sections.
   The older `screen-capture.webm`/`decompilation/webm_ffmpeg_microframes`
   analysis is retired and is not a source for new visual claims. Until an
   independent reference is available, dump the native image and inspect it.

Source analysis must precede implementation, but source analysis does not mean
source-shaped implementation. For each behavior, record the source-derived
inputs, transitions, placement, and timing, then express them as typed native
state and ordinary C++ code. Validate an internal assumption before applying a
fix; do not create a metric from the broken implementation and use that metric
as proof that the assumption was correct.

## Hill rule (take and defend)

One hill at a time. A hill is one screen or one shared mechanism.

A hill is **defended** only when all of these exist:

1. ROM entry, cursor/RAM, draw path, and exit traced.
2. Typed native state in the game engine/controller layer. Qt widgets or Qt
   Quick controls may present the state, but they must not become a second,
   conflicting game-state machine.
3. A CTest that fails if that behavior or those pixels regress.
4. A CLI image dump (`actual.bmp`) for that scene.

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

- The native/Qt executables consume `decompilation/assets/` directly. Each
  background, sprite, animation frame, map, and HUD resource remains an
  individually named extracted file. `assets/native/state.bin` contains only
  the small typed non-graphics state needed to start without the ROM.
- Native/Qt executables consume the named files under `decompilation/assets/`
  and `assets/native/state.bin` directly. Do not create or depend on an
  aggregate serializer or pack.
- `shadowrun_graphics_extract --all-assets` is the explicit offline import
  step. Do not attach extraction to CTest or ordinary executable builds.
- Genesis 4-bit tiles and 9-bit CRAM become RGBA8 at extraction. Preserve the
  look, not VRAM limits.
- Host may run at 60 FPS. Animation cadence still follows ROM counters.
- Use modern Qt/C++ for the runtime. Menus, inventory screens, dialogs, and
  other interfaces with limited moving parts may be implemented directly in
  Qt Widgets, `.ui` files, or Qt Quick. Their controls are bound to the typed
  native game state; they must not duplicate authoritative behavior.
- Use a native C++ game engine layer for field movement, combat, world state,
  animation, and other behavior that needs it. If an existing Qt-compatible
  engine satisfies the requirements, evaluate and use it; otherwise build the
  smallest engine needed rather than recreating Genesis hardware.
- A software framebuffer/widget remains valid for scenes that need exact
  sprite compositing, but it is a presentation choice, not a requirement that
  every screen be rendered as a Genesis-style tile plane.
- Input belongs to the focused Qt window and is forwarded to the authoritative
  native controller. There must be no duplicate menu state machine or
  unfocused-window input capture.
- Unresolved indirect calls stay explicit boundaries.

## De-engineering and modernization rule

The existing reconstruction is subject to deliberate de-engineering. When a
runtime function is shaped like `decode_*`, walks ROM tables, reproduces name
tables, or mimics a source renderer solely because the ROM did so, determine
what behavior it was trying to recover and replace that runtime path with a
small native model. Keep offline extraction code only where it produces a
stable host asset or records source evidence. Keep source addresses and
disassembly references in comments or documentation as provenance, not as
runtime dependencies.

For static or lightly animated interfaces, prefer authored Qt layouts and
explicit sprite/background placement. For moving scenes, use a native update
loop and sprite components with source-derived states, paths, and durations.
The goal is visual and behavioral equivalence on x86, not implementation
equivalence with the Genesis.

## Build

C++17, existing CMake presets under `decompilation/`. Debug + Release CTest
for each completed hill. Visual dump:

```text
shadowrun_visual_dump.exe <asset-dir> <out-dir> --scene title --frames 0,90,128,500
```

Do not describe the project as playable until a native/Qt executable accepts
controls and produces visible game graphics that a defended hill covers.
