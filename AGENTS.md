# Shadowrun Ghidra Conversion Project Instructions

## Current project focus

Current development is exclusively in `decompilation/ghidra_conversion/`.
Its `CMakeLists.txt` and `CMakePresets.json` build the typed C++/Qt project from
the Ghidra source analysis. The retired native-port source is preserved under
`archive/legacy_native_port/`; do not extend or build it unless the user
explicitly reactivates it. The Ghidra database is archived under
`archive/ghidra_analysis_project/`.

The active project consumes shared files from `decompilation/input/`,
`decompilation/export/`, `decompilation/assets/`,
`decompilation/gameplay/scripts/`, `decompilation/gameplay/world/`, and
`decompilation/docs/ghidra/`. Gameplay video and numbered frame analysis remain
under `video analysis/`.

## Objective

Reconstruct the US Genesis game in `decompilation/ghidra_conversion/`. The Qt
executable is the host application; the C++ conversion model and Lua content
should express recovered behavior through readable, typed systems. Archived
native-port files are historical and are not a migration target.

Use the ROM and Ghidra export together to recover behavior. Keep source
addresses as provenance, then implement behavior through the active project's
C++/Lua abstractions rather than copying disassembly or introducing another
emulator-shaped runtime.

Before adding more emulation-shaped code, de-engineer what has already been
constructed: identify runtime code that reproduces bytecode, VDP/name-table
operations, ROM address arithmetic, or source-specific drawing helpers; retain
the useful behavioral knowledge and one-time extraction data, then replace the
runtime machinery with explicit native state, assets, layouts, and systems.

## Non-goals

- Do not ship a 68000 interpreter, Genesis emulator, or VDP as the runtime.
- Do not move current behavior work into the archived native-port project.
- Do not implement menus or game systems as bytecode copies, ROM-address
  interpreters, VDP simulations, or generic runtime `decode` pipelines. A
  decoder may exist in an offline extractor to turn source resources into host
  assets; the game runtime consumes those assets directly.
- Do not invent screens, archetypes, buildings, or combat to fill a hole.
- Do not claim visual correctness from a compositor of the same extracted
  assets used by the renderer.

## Sources of truth

1. **Primary:** the US ROM (`Shadow Run (U) [!].bin` /
   `decompilation/input/shadowrun_genesis_us.bin`). Jump to the screen entry, follow tables,
   RAM, and callees.
2. **Secondary:** Ghidra C under `decompilation/export/`. Cross-check only. Use it to
   understand behavior and validate the ROM trace; never copy it in as the
   native implementation.
3. **Tertiary:** real-game visuals. The current visual sanity check and timing
   reference is `video analysis/long_gameplay_video.mp4` and its one-time
   `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`
   FFmpeg dump, which contains the numbered analysis sections. The older
   `screen-capture.webm`/`archive/retired_analysis/analysis/retired/webm_ffmpeg_microframes`
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
`decompilation/docs/ghidra/decompiled-function-status.tsv` (one ROM entry per row).

If a test only re-encodes the same assets the renderer uses, it does not defend
the hill. Leftover-clear tests (return to a screen, pixels match a fresh
instance) defend the garbled-text class without goldens.

Progress index: `decompilation/docs/ghidra/HILLS.md`.
Screen map: `decompilation/docs/ghidra/screen-system-audit.md` and
`decompilation/docs/ghidra/reconstruction-exploratory-analysis.md`.

## Runtime shape

- The Ghidra Qt executable consumes individually named files under
  `decompilation/assets/`
  directly. Do not create or depend on an aggregate serializer or pack.
- `shadowrun_graphics_extract --all-assets` is the asset import step. The
  CMake build runs it by default (`SRX_AUTO_EXTRACT_ASSETS=ON`, ROM from
  `SHADOWRUN_US_ROM`, default `decompilation/input/shadowrun_genesis_us.bin`)
  through `cmake/srx_extract_assets.cmake`: it extracts into the build tree and
  copies into `decompilation/assets/` without overwriting git-tracked files,
  and re-runs only when the extractor, the ROM, or `resources.tsv` change. A
  missing ROM is a configure warning, not an error. Opt out with
  `-DSRX_AUTO_EXTRACT_ASSETS=OFF` and run the extractor by hand. Do not attach
  extraction to CTest.
- Genesis 4-bit tiles and 9-bit CRAM become RGBA8 at extraction. Preserve the
  look, not VRAM limits.
- Host may run at 60 FPS. Animation cadence still follows ROM counters.
- Lua owns authoritative gameplay behavior and content: actor AI, movement
  state and decisions, animation state and cadence, combat policy, world/map
  events, quests, dialogue text and portraits, and menu choices and labels.
  Keep mutable actor state in Lua under stable instance IDs such as
  `game.actors[actor_id]`. Qt presents script-defined state and forwards input;
  it must not create a parallel gameplay or menu state machine.
- Schedule one Lua behavior heartbeat for each active ROM actor record per
  source actor-service pass and a separate animation heartbeat per active
  record in the source render pass. Trace the ROM cadence and conditions first;
  do not substitute a wall-clock timer or call an actor only because it is
  visible in the current viewport.
- Native C++/Qt provides narrow host services: focused-window input
  collection, typed world facts, map and collision queries, asset rendering,
  audio playback, and persistence. Scripts own movement/path policy, animation
  timing, AI, combat decisions, quest/dialogue flow, and menu behavior. C++ may
  return structured collision results and render the resulting Lua state, but
  it must not run a second movement or animation state machine. The archived
  native-port tree is not a migration target for current work.
- A software framebuffer/widget remains valid for scenes that need exact
  sprite compositing, but it is a presentation choice, not a requirement that
  every screen be rendered as a Genesis-style tile plane.
- Input belongs to the focused Qt window and is forwarded as typed input facts
  to the authoritative Lua gameplay/menu state. There must be no duplicate
  menu state machine or unfocused-window input capture.
- Unresolved indirect calls stay explicit boundaries.

## De-engineering and modernization rule

The active Ghidra reconstruction is subject to deliberate de-engineering. When
a runtime function is shaped like `decode_*`, walks ROM tables, reproduces name
tables, or mimics a source renderer solely because the ROM did so, determine
what behavior it was trying to recover and replace that runtime path with a
small typed host model. Keep offline extraction code only where it produces a
stable host asset or records source evidence. Keep source addresses and
disassembly references in comments or documentation as provenance, not as
runtime dependencies.

For static or lightly animated interfaces, prefer authored Qt layouts and
explicit sprite/background placement. For moving scenes, use host update loops
and sprite components with source-derived states, paths, and durations. The goal
is visual and behavioral equivalence on x86, not implementation equivalence
with the Genesis.

## Build

C++17. Run CMake preset commands from `decompilation/ghidra_conversion/` using
its `CMakePresets.json`; Release Qt is the normal executable. Debug + Release CTest
applies to each completed hill. Visual dump:

```text
build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe --startup-dump start_options 2 500 frame.bmp --assets ../assets
```

Do not describe the project as playable until the Ghidra Qt executable accepts
controls and produces visible game graphics that a defended hill covers.
