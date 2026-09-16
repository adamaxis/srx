# Shadowrun Native Port Instructions

## Objective

Reconstruct the Sega Genesis Shadowrun ROM as a native C++ Windows game. The
finished program must run the game logic as ordinary C++ and provide abstract
controls, graphics, timing, audio, and persistence interfaces that can later be
implemented with SDL, DirectX, or another frontend.

The acceptance target is a runnable native game with controls and visible
graphics. A collection of translated functions, a disassembly listing, or a
test-only memory harness is not a finished port.

## Explicit non-goal

Do **not** build a 68000 interpreter, Genesis emulator, or instruction-level
compatibility layer as the runtime solution. The ROM is evidence and a source
of code/data/assets; the port itself executes native C++ control flow.

`GenesisBus` and recorded VDP writes are transitional reverse-engineering and
verification tools. They must not become a permanent requirement for the
native game executable.

## Porting method

For each ROM routine:

1. Identify the call site and the state it actually reads and writes.
2. Recover the relevant tables, asset formats, flags, and boundary behavior
   directly from the binary.
3. Express that behavior as typed C++ state and functions, preserving the
   original observable behavior rather than copying decompiler pseudocode.
4. Refactor address-based state into native structures when the surrounding
   behavior is understood.
5. Connect the result to the native update loop, input model, or render model.
6. Add a focused behavior test and commit the completed slice.

Unresolved indirect calls must remain explicit boundaries until their behavior
is recovered. Do not hide unknown behavior behind guessed game logic.

## Native graphics boundary

Genesis VBlank work is a synchronization and update mechanism, not the final
rendering API. Translate it into native render operations such as:

- texture or tile-data updates;
- tilemap/background updates;
- sprite draw commands;
- camera/layer state changes; and
- frame submission/presentation.

The native renderer must not depend on Genesis VRAM size, VDP address layout,
or palette-indexed GPU rendering. Those details may be retained in the
extraction/verification path when they are needed to reproduce behavior.

## Asset conversion

Extract graphics and data from the ROM into ordinary host-usable assets while
preserving the raw source and conversion metadata. Genesis 4-bit tile pixels
and 9-bit CRAM colors should be expanded to RGBA8 during extraction. PNG or
another normal Windows-readable texture format is appropriate for static
assets; BMP is acceptable for simple diagnostic frame output.

Palette data is an extraction-time source format, not a runtime constraint.
Dynamic palette effects should become RGBA texture updates, color swaps,
tints, or other native material operations. Preserve the visual effect, not
the 35-year-old VRAM/palette limitation.

## Native controls

Replace Genesis controller probing with an `InputState` supplied by a Windows
keyboard/gamepad adapter. Preserve edge-triggered versus held-button behavior
at the game-logic boundary, but do not require serial controller handshakes or
Genesis I/O addresses in the native frontend.

## Milestones

1. Native headless update loop with abstract `InputState` and render commands.
2. ROM asset extraction and RGBA/BMP/PNG conversion for a visible frame.
3. Native Windows software-rendered window proving controls and graphics.
4. SDL/DirectX frontend replacement behind the same native interfaces.
5. Incremental replacement of transitional bus/address code with typed game
   state, followed by emulator/screenshot/trace comparisons.

Each milestone must produce a runnable artifact. Keep the original ROM
unchanged and retain raw assets alongside converted assets.

## Current implementation status

The native Windows port is fed by the ROM's reviewed startup data.
`graphics.cpp` decodes the three `0x1530` pattern uploads at `0x1F310E`,
`0x67CB0`, and `0x6875C`, reconstructs the `0x1728` delta-coded tilemap at
`0x67DF6`, expands the selected CRAM image at `0x1FF0` to RGBA8, and submits a
320x224 frame through `ScreenWriter`. `shadowrun_native.exe` accepts an
optional ROM path and maps Windows keyboard input into `InputState`.

`NativePortShell` now has an explicit native scene boundary. A ROM-backed
instance starts in `Title`; the pressed `Start` edge models the reviewed
`0x0BF2`/`0x1A9E` exit condition and transitions to `Field`. The field update
is ordinary typed C++ state with held directional movement and edge-triggered
actions.

The field background is now the recovered mode-2 presentation rather than a
placeholder composition. The mode record at `0x172C80` selects the raw
`0x166B82` pattern upload (`0x6E20` bytes), the compressed 4x4 VDP name-word
stamps at `0x170102`, and 60x42 primary/secondary map tables at `0x16D9A2` and
`0x16ED52`. `graphics.cpp` translates those records, palette attributes, and
flips into a 1920x1344 host RGBA map and the native shell presents a cropped
viewport through `ScreenWriter`. Directional input now advances typed world
coordinates and follows the marker at the viewport edge, so the field scrolls
over the recovered map instead of moving only a screen-space placeholder.

This is now a runnable native title-to-field slice with controls, actual
ROM-derived field graphics, and a ROM-derived entity frame rendered through
the native screen boundary. The seven-frame set follows the typed field camera
through the reviewed `0x5A62` projection; `0x600E`/`0x3E86` data is converted
to RGBA8 at load time and advances while movement is held. This is not a claim
that the whole game is playable. The next port slices should replace the
native input-anchor scaffold with recovered entity records and connect field
UI to the typed session/VBlank boundaries.

## Build and source discipline

- Use C++17 and the existing CMake build.
- Keep platform/frontend code separate from native game logic.
- Preserve unrelated worktree changes.
- Run Debug and Release builds plus CTest for each completed slice.
- Commit completed topic changes with a focused message.
- Do not describe the project as playable until a native executable accepts
  controls and produces visible game graphics.
