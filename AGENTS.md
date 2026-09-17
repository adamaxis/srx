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

The native Windows port can be fed by a host-only asset pack generated from
the ROM's reviewed startup data. `graphics.cpp` decodes the three `0x1530`
pattern uploads at `0x1F310E`,
`0x67CB0`, and `0x6875C`, reconstructs the `0x1728` delta-coded tilemap at
`0x67DF6`, expands the selected CRAM image at `0x1FF0` to RGBA8, and submits a
320x224 frame through `ScreenWriter`. `decode_genesis_native_assets` packages
the expanded title/field/entity images, host terrain grid, initial field
positions/camera, entity template, and reviewed direction table into `.srna`.
`shadowrun_native.exe` accepts that pack directly, so ROM decoding is an
extraction/verification path rather than a runtime requirement; it also keeps
the optional ROM path for development comparisons. Windows keyboard input is
mapped into `InputState`.

`NativePortShell` now has an explicit native scene boundary. A ROM-backed
instance starts in `Title`; the pressed `Start` edge models the reviewed
`0x0BF2`/`0x1A9E` exit condition and transitions to `Field`. The field update
is ordinary typed C++ state with held directional movement and edge-triggered
actions. Field `Start` opens a typed `FieldMenu` instead of resetting the map;
the menu preserves the active record/camera, uses the seven selector values
and four-row secondary index from ROM `0x10D30`/`0x114A0`/`0x11564`, and
renders the reviewed labels from `0xD1D28` through the native screen boundary.

The host controller boundary now follows ROM 0x195C: a sampled Genesis word
has the current controller state in its low byte and newly asserted bits in its
high byte, with bit order Up, Down, Left, Right, B, C, A, Start. The Windows
frontend polls keys into that word and passes it through
`decode_genesis_controller_word`, so the native update loop consumes the same
held/edge distinction as the ROM. SDL or another frontend should provide the
same sampled word rather than mapping directly to `InputState`.

The field background is now the recovered mode-2 presentation rather than a
placeholder composition. The mode record at `0x172C80` selects the raw
`0x166B82` pattern upload (`0x6E20` bytes), the compressed 4x4 VDP name-word
stamps at `0x170102`, and 60x42 primary/secondary map tables at `0x16D9A2` and
`0x16ED52`. `read_genesis_mode2_field_resources` follows the record's
`0x172C50` setup pointer and exposes those pointers, dimensions, palette, and
initial fixed-point camera as a typed native resource descriptor. Genesis
`0x166B82` graphics are linear pixel-packed tiles: each 8-pixel row uses four
bytes, with the high nibble followed by the low nibble. Treating those bytes
as interleaved bitplanes creates the vertical/noise corruption that this
native extraction path must avoid. The descriptor also retains the three
fixed-point field-record pairs at `+0x42..+0x4C`, which ROM
`0x2B34` copies into the first three `0x100`-byte records. The decoder
translates those records, palette attributes, and flips into a 1920x1344 host
RGBA map and the native shell presents a top-left-camera viewport through
`ScreenWriter`. Directional input now advances the first recovered field
record instead of a presentation-only marker, so the field scrolls over the
recovered map using the same world origin as the direct projection path. The
field loop's reviewed raw `0x2000` dispatch to `0x391A` is represented by the
native C edge: it selects the next of the three initialized records and
retains each selected record's fixed-point position. This is a controlled
record boundary, not speculative rendering of duplicate party members.

The same mode-2 descriptor exposes the terrain table at `0x172DAA` and the
direct bounds words used by ROM `0xE7C8`. `decode_genesis_mode2_terrain_map`
rebuilds its 101 sentinel-terminated rectangles into a 64x64 host status
grid, retaining each status byte for native movement and collision. The native
shell now drives record zero from the first `0x2B34` coordinate pair and probes
the candidate position at all four `0xE8A0` corner offsets before committing a
move. Blocked candidates remain in place, while the camera follows the record
only after it reaches the reviewed projection window. The renderer-independent
`terrain::probe` ports the direct `0xE7C8` bounds/status path and its
`0xE8A0/0xE9EE/0xEA36/0xEAD2` helpers over that host grid.

The mode-2 interaction path is now carried through the native area boundary.
ROM `0x167AE` indexes the descriptor table at `0x172CDC` with the low five
bits of the retained raw terrain status; the first Redmond entrance class is
therefore a proven destination mode `0x14` (decimal 20), not mode 14. The
native asset extractor packages a `NativeFieldArea` catalog with converted
RGBA maps, terrain grids, spawn records, camera origins, and event tables.
Mode `0x14` uses the ROM's raw 16-bit name-word stamp table at `0x1CD29A`
(941 four-by-four stamps, 50x30 map entries, 1600x960 host map), while modes
2 and 10..19 retain their compressed resource streams. The native field loop
now loads that extracted mode when the reviewed C0 doorway status is reached;
the focused test crosses a synthetic doorway and verifies the mode-20 spawn
`0x5A00/0x0800`. Every non-empty destination named by the mode-2 Redmond event
table (`0x14`, `0x1A`, `0x0D`, `0x13`, `0x34`, `0x32`, and `0x33`) now has a
decoded native area record; transition geometry and exits still need focused
in-game verification.

The same descriptor layout is now resolved through the ROM mode pointer table
at `0x3232`, rather than being hard-coded to mode 2. The shared decoder and
extractor cover the small indoor-style resources at modes `10..19`, including
their reversed layer-pointer order and the 10x21 scrolling map at mode 11.
The native shell now consumes the extracted mode-2, mode-13, mode-20, mode-19,
mode-26, and mode-50..52 area records. Modes 26 and 50..52 use the reviewed
raw name-word tables; modes whose metatile streams are not yet stamp-aligned
remain extraction-only until their distinct format is recovered.

The extracted field entity set currently contains eight directional states.
The native shell selects one state from movement direction, but does not yet
advance the ROM's time-based state-list animation index. Walking animation,
NPC/object animation, audio/music output, authentic screen composition, and
the remaining field/interior transitions remain explicit native-port work
rather than being hidden behind placeholder logic.

This is now a runnable native title-to-field slice with controls, actual
ROM-derived field graphics, collision rejection, and a ROM-derived entity
frame rendered through the native screen boundary. The field-record
directional state follows the direct `0x600E` kind/state indirection
(`0x62AC` -> kind-3 table `0x6378` -> lists beginning at `0x9334`) through
the reviewed `0x5A62` projection; its eight `0x8002` descriptors are decoded
to host RGBA at extraction time and selected from the native movement vector.
The native record stores the recovered fixed-point world position, exact VDP
projection words, host screen coordinates, visibility, collision result, and
directional state; its remaining 0x100-byte fields stay in the transitional
bus-backed session layer until recovered. This is not a claim that the whole
game is playable. The next port slices should expand typed entity/session
records and connect the deeper field command handlers and additional object
behavior to the native update/render interfaces.

The first display-state bytes are now part of that native record boundary:
template offsets `+0x56` and `+0x57` are extracted as signed
`display_state`/`display_substate` values. The native projection preserves the
`0x36FC` visibility gate (`+0x56 < 0` or `+0x56 == 0x7F` suppresses the
sprite), and the values are serialized in native asset-pack version 6. The
native field command screen also carries the eight `+0x66` item codes and the
adjacent phase bytes, with the 50-name item catalog extracted from `0xD4540`.
This is a direct behavior port, not a new animation rule. The native C runner
selection is a recovered field-loop action. Field A/B now enter the reviewed
shared command dispatch boundary and retain the selected command as typed
state; Inventory now renders the ROM-selected slots and names and ports the
reviewed armor toggle at `0x1219C`/`0x121B0`. Its remaining weapon/medical/
discard effects and the other command subsystems remain to be ported from
their ROM tables rather than being represented by placeholder flashes.
The Clips command now has a native submenu for the ROM `0x11B58` selector and
the extracted `+0x4E`/`+0x4F` ammunition values; its reload/trade quantity
mutations remain separate typed handler work.
The Cyberware command now has the ROM `0x11950` display path: `+0x90` is
preserved as a fifteen-bit installed-cyberware field, and the FF-terminated
names behind the `0xD2072` pointer table are extracted into the native pack.
The initial active record consequently exposes `Hand Razors`; the display
handler returns on Start without adding mutation behavior that the ROM does
not perform. Native asset-pack version 6 carries this catalog and field.
The Attributes/Skills command now exposes the direct `0xD01C` data path:
eight attribute bytes, twelve skill bytes, four preserved auxiliary bytes, and
the twenty names behind `0xC1A2A` are serialized into version 6. Its two-column
and ten-row navigation is ported; unresolved point-distribution mutations stay
out of the native state until their exact shared handlers are recovered.

## Build and source discipline

- Use C++17 and the existing CMake build.
- Keep platform/frontend code separate from native game logic.
- Preserve unrelated worktree changes.
- Run Debug and Release builds plus CTest for each completed slice.
- Commit completed topic changes with a focused message.
- Do not describe the project as playable until a native executable accepts
  controls and produces visible game graphics.
