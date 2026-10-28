# CyberInfo Run Info placement

## ROM trace

ROM `0x18AA6` owns the two-row CyberInfo selector. The Run Info branch at
`0x18C28` reads signed `FFF0E3`: negative is active, zero is no run, and
positive is complete. `0x18C42` selects the corresponding text stream.

The text writer at `0xC0DA` uses the plane-A address in A2. The plane uses 64
cells per row (`0x80` bytes), two bytes per cell, and 8-pixel tiles. A `0x80`
text byte advances to the next tile row. Preserving that control is necessary:
the two long messages would otherwise run past the visible right edge. The
source addresses therefore map to these screen positions:

| ROM write | Text/data | Plane address | Pixel origin |
|---|---|---:|---:|
| `0x18C50` | `You are not on` / `a matrix run!!` | `$C42C` | `(176,64)`, `(176,72)` |
| `0x18C66` | `Shadowrun is` / ` complete!!` | `$C42E` | `(184,64)`, `(184,72)` |
| `0x18C76` | `Node:`, `Node Function:`, `Action:` | `$C328` with `0x80` row breaks | `(160,48)`, `(160,64)`, `(160,88)` |
| `0x18C94` / `0x18CAA` | `CPU` / `Datastore` | `$C334` | `(208,48)` |
| `0x18CED` | CPU function or datastore file class | `$C4AC` | `(176,72)` |
| `0x18CD4` | action selected from `0xE44C8` | `$C630` | `(192,96)` |

Active CPU runs use `Cntrl Processor` and `Crash System`. Datastore runs use
the objective's file class and the action selected from signed `FFFBF6`.

## CyberInfo selector placement

At `0x18AB0`, the ROM passes the stream at `0xE3886` to the text writer with
plane address `$C32C`. The ROM bytes begin `20 20 "Cyberdeck Info" 80 5B`,
followed by twelve `5C` glyphs. Each `0x80` advances one eight-pixel tile row.
The two leading spaces place the visible title at `(192,48)`; `[` and the
twelve-glyph separator begin at `(176,56)`. Two row controls then place
`Deck Stats` at `(192,72)`, and the next two place `Run Info` at `(192,88)`.
Their two leading spaces are source data, not presentation padding to strip.
The initial cursor base at `$C4AC` is `(176,72)`, with the second row 16 pixels
below it.

Lua now retains those exact title and option strings. The Qt presenter draws
the separator and cursor at the ROM origins. The earlier host title
`Cyberdeck Information` exceeded the source wording and was clipped; the
source-backed `Cyberdeck Info` now fits through its final glyph.

### Retained parent Cyberdeck surface

ROM `0x18AA6` calls `0x1897C`, which fills the right panel beginning at Plane-A
`$C126` (pixel origin `(152,16)`). It does not clear the left program grid,
selected parent row (`FFF0D1`), or the footer written by `0x17708`. The child
cursor uses the separate `FFF0DC` value. The Run Info and Deck Stats branches
also call `0x1897C` before drawing their right-panel text, so those displays
retain the same parent grid and footer.

The host previously rebuilt CyberInfo on a blank left grid and omitted its
parent selection. Lua now carries the parent deck row/column and publishes the
live ratings, installed programs, and slot statuses to the typed snapshot.
The presenter keeps the selected row and active program icons in CyberInfo,
Run Info, and Deck Stats, along with the loaded-program footer. The CyberInfo
QA checks the retained source assets in full 320x224 frames for all three
views. This is a source-backed regression check; it does not by itself prove
independent Genesis pixel fidelity.

### Loaded-program prompt

ROM `0x19238` calls `0x18998` to redraw the right panel beginning at `$C628`
while leaving the selected parent row, program grid, top detail panel, and
`0x17708` footer in place. The stream at `0xE3859` contains
`Program is already`, a `0x80` row advance, and `loaded in memory.`; ROM
`0xC0DA` treats `0x80` as the next tile row and `0xFF` as the stream end. The
separate streams at `0xE3823`, `0xE382E`, and `0xE387E` begin at `$C72E`,
`$C7AA`, and `$C8BA`.

The typed Lua child state preserves the parent Analyze program name and detail
panel while publishing the current ratings, loaded programs, and slot statuses.
The presenter keeps that panel and parent deck chrome visible, then places the
prompt lines at `(160,96/104)`, the erase question at `(184,112)` and
`(168,120)`, and the Yes/No rows at `(232,136/152)`. `FFF0DC` defaults to 1,
so the cursor is at `(200,152)`. The CyberInfo route QA writes
`deck-program-prompt/actual.bmp` and checks the first and last glyph cells at
each traced origin, the parent Analyze icon/details, grid, and full footer in
both configurations. Debug and Release captures have matching hashes. No
independent Genesis frame of this prompt is currently available.

## Change

Lua now returns a typed Run Info view for all three `FFF0E3` branches. The Qt
presenter draws each field at its ROM-derived address using the imported Matrix
font. It no longer flattens the labels and values into the generic Cyberdeck
message block.

## Deck Stats placement

ROM `0x18B5C` selects the Deck Stats branch. It writes the label template from
`0xE3902` at `$C2A8`, then reads the active deck record at `FFFBBE`. The maker
and model table is at `0x1AB100` (the no-model/default row) and `0x1AB118`
(five 24-byte records). The maker and model are separated by a `0x80` line
control and written from `$C1AE`.

The table below maps each `0xC216` numeric write to its source field and
rightmost digit cell. The formatter emits least-significant digits first,
walking left, with its comma glyph after each group of three digits.

| Source field | Plane address | Pixel origin | Screen row |
|---|---:|---:|---|
| MPCP (`FFFBBE`) | `$C346` | `(280,48)` | MPCP |
| Hardening (`FFFBBF`) | `$C3C6` | `(280,56)` | Hardening |
| Memory (`FFFBC0`) | `$C4C6` | `(280,72)` | Memory |
| Storage (`FFFBC2`) | `$C546` | `(280,80)` | Storage |
| Load/IO speed (`FFFBC4`) | `$C5C6` | `(280,88)` | Load/IO Spd |
| Response (`FFFBC5`) | `$C6C6` | `(280,104)` | Response |
| Bod / Evasion / Masking / Sensor (`FFFBC6`–`FFFBC9`) | `$C7C6`, `$C846`, `$C8C6`, `$C946` | `(280,120)`, `(280,128)`, `(280,136)`, `(280,144)` | Persona ratings |

Lua now returns the live Cyberdeck record with the ROM maker/model labels and
the static labels from `0xE3902`. The Qt presenter draws those labels and
grouped decimal values at the traced origins. The route CTest writes
`qt_runtime/field-matrix-cyber-info-qa/deck-stats.bmp` and checks the maker/model rows, every
label row, and the numeric origins. This verifies the source-position text
layer; it does not establish full CyberInfo panel art or independent Genesis
pixel fidelity.

## Cyberdeck chrome palette and layout

ROM `0x181F8` copies the common palette at `0xE2E3A` into the palette buffer,
then calls `0x17F80`, which reaches `0x566E2`. That routine copies sixteen
words from `0xE2EBA` into `FFD89E`, replacing CRAM line 1. The deck frame and
selector rows use that line (`D2=0x2000` at `0x18296` and `0x189D8`). The asset
extractor had decoded those tiles with the initial line at `0xE2E5A`; this
produced green/yellow chrome in place of the blue frame in the game. The
extractor now uses the live `0xE2EBA` palette for the deck background,
selector, cursor, runner headers, and detail strips. Program icons and the HUD
footer retain their separate CRAM line 3 palettes.

The `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` test now checks
the border pixel at `(6,12)` against live palette entry 10 (`0x0A40`, RGB
`0,73,182`) and requires nonblank art at the bottom edge. Its Debug and
Release runs write `cyberdeck.bmp` at 320x224; the images were inspected beside
the independent longplay frame
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_15_source_2820-3120/frame_004220.png`.
The full-frame comparison exposed an offline stream-expansion bug: `0x1728`
uses an inclusive `BGE` loop for every step run, including decreasing runs.
The extractor had treated decreasing runs as `BGT`, omitting four cells from
`D9C96` and eleven from `D8316`. It now expands the complete 880-cell Plane-B
background and 240-cell footer. QA verifies every populated grid icon and the
selected Attack detail icon, the two installed footer icons at `(80,184)` and
`(128,184)`, the three empty slots, and all five status strips.
The longplay fixture uses a 15 Mp deck: the loaded Deception and Analyze
programs occupy 2 Mp and 3 Mp, leaving the displayed 10 Mp free; the selected
Attack 3 entry separately reports its 18 Mp size and two load steps. Run Info
and Deck Stats retain the selected program row, installed grid icons, and
shared live footer. Their QA checks each named icon and footer asset at its
source position instead of expecting an empty left panel.
The Qt visual QA now seeds those visible inputs from independent frame
`004220` instead of the earlier all-programs-at-rating-4 test deck. The
selected Attack rating, free memory, loaded Deception/Analyze slots, and
visible program presence match that frame; Medic's exact rating and the
hardware I/O value are not visible, so the fixture uses minimum nonzero Medic
rating and an I/O value that produces the displayed two-step load time. The
Pull Plug capture then switches the footer to Deception/Attack as seen in
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_15_source_2820-3120/frame_005400.png`. These are screenshot comparison fixtures, not defaults for
gameplay state.
The updated full-frame dump was reopened beside the independent longplay
capture; video compression changes exact colors, so this remains a visual
comparison rather than a raw-pixel golden. ROM `0x196F2` rotates palette
entries 8-11 every five calls. That palette cycle is now represented by a
Lua-owned countdown and phase shared with the CyberInfo, Program, Storage,
Map, and Pull Plug routes. The ROM directly calls the rotator from the main
Cyberdeck, CyberInfo, and Program loops; Storage and Map preserve the current
colors without advancing the counter. The first call advances immediately
because `FFF10A` is initialized to zero.

The offline importer writes four separately named background, cursor, and
selected-row images by applying the ROM's four CRAM line-1 rotations to the
source tiles. The Qt renderer selects those named images from the typed phase
on the active Lua menu state. C++ does not own a palette timer or decode CRAM
at runtime. The deck QA captures all four phases and checks that the visible
background, cursor, and selection overlay use the corresponding assets.
Debug and Release outputs match byte for byte. The longplay sequence around
frames `004200`-`004237` visibly cycles through the same blue frame colors;
the exact phase offset against a numbered compressed video frame is not used
as timing evidence. Matrix traversal remains open.

The direct System Search route opens this Cyberdeck after the source
20-VBlank jack-in. ROM `0x181F8` uploads `0xD78AA` through `0x177CC`, then
`0x182A6` calls `0x17708`, which draws the `0xD8316` footer stream at Plane-B
`$EB00`. That ROM-composed footer already contains the silver portrait at
`(16,184)`. A later port overlay incorrectly painted the field-menu preview
selected from a live runner record over that art, producing a different face.
The overlay is removed. QA now checks that the full 32x32 portrait cell in the
rendered Cyberdeck remains pixel-identical to the corresponding cell in the
source-composed `cyberdeck-background.bmp`.

The corrected Debug and Release `cyberdeck.bmp` frames were reopened at full
320x224 size beside independent longplay frame `004220`. The silver portrait's
size and visible shape now agree with the reference; Debug and Release output
is byte-identical. The video frame remains a visual reference rather than a
raw-pixel golden because of video compression. The panel bounds, selected row,
program icons, and footer were also inspected in the full frames. The four
palette captures are `cyberdeck.bmp` and
`cyberdeck-palette-phase-{1,2,3}.bmp`.

## Cyberdeck grid selection and numeric panel (2026-10-27)

ROM `0x18326` waits for a VBlank before sampling the Cyberdeck controller. The
direction checks at `0x1833C` through `0x18418` give Up, Down, Left, Right
priority and wrap the selected row and column through `0..3`. `0x18A92`
computes `row * 4 + column`; `0x1882E` uses that index to select a program
rating, icon, and right-panel content.

The uninstalled-program stream at `0xE39B7` begins with three `0x20` spaces,
then `Empty`, a `0x80` row advance, and `Utility Slot`. ROM `0x1884C` writes it
at `$C42C`, which starts at `(176,64)`. Each leading space occupies one
8-pixel tile, so `Empty` begins at `(200,64)` and `Utility Slot` begins at
`(176,72)`. The presenter had used two spaces and started `Empty` at `x=192`;
it now uses the ROM's three spaces.

The Debug/Release route QA now walks all sixteen cells with ordinary
controller input and saves `cyberdeck-cell-00.bmp` through
`cyberdeck-cell-15.bmp`. It checks the row/column and label at each cell, the
source selected-row and cursor assets, the retained program grid/footer, and
that no prior cell's numeric detail survives a move. Eight uninstalled
programs show the ROM `Empty / Utility Slot` text at the coordinates above;
the four utility cells show their own names and icons without program stats.

For the reference deck fixture, the ROM footprint bytes at `0xE2DF2` and
rating-squared calculation at `0x1952C` produce Attack 3 = 18 Mp, Medic 1 =
4 Mp, Deception 1 = 2 Mp, and Analyze 1 = 3 Mp. Its 15 Mp memory and loaded
Deception/Analyze slots (2 + 3 Mp) produce 10 Mp free via `0x1957E`. Load/IO
speed 10 yields two steps for Attack and one for the other installed programs
via the ceiling and six-step cap at `0x195A6`. These values come from the
active Cyberdeck record; the runner Success/Defense row remains `Unknown` in
this no-active-run fixture. The independent frame `004220` confirms the
selected Attack 3 detail and the visible program/footer state. Medic's rating
and the other numeric panels are source-derived QA states, not independent
Genesis observations; no matching capture of an uninstalled cell is present.

The expanded `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` passes
in Debug and Release. All sixteen cell BMPs match across configurations. This
extends the source-backed selection and detail checks; the complete Matrix
hill remains partial.

## Pull Plug placement

The row-3/column-3 branch at `0x18450` keeps the selected Command grid while it
draws the prompt. It writes the optional Black IC warning from `0xE43B0` at
Plane A `$C228`, the question from `0xE438A` at `$C426`, and the Yes/No stream
from `0xE387E` at `$C638`. With 64 cells per row and two bytes per cell, these
addresses map to `(160,32)`, `(152,64)`, and `(224,96)`. The cursor helper at
`0xC376` uses `$C632`, placing the selected arrow at `(200,96)` with 16-pixel
row spacing. The initial `FFF0DC=1` selects No at y=112.

The host had treated Pull Plug as a generic submenu: it dropped the selected
program grid and footer and drew the prompt at the wrong origin. Lua now
publishes the current active program and slot-status records with the child
screen. The presenter retains the selected Command row and loaded-slot footer,
then places the optional warning, question, options, and cursor at the ROM
addresses above. The full-frame QA writes `pull-plug.bmp` and checks the
prompt, selected icon, installed and empty slots, five status strips, and the
complete 320x224 frame bounds. I inspected the Release capture against
independent Section 15 longplay frame `section_15_source_2820-3120/frame_005400.png`; the prompt, grid row, selected Pull Plug
icon, cursor, and bottom toolbar align. The Debug and Release captures have
matching SHA-256 hashes.

The 2026-10-19 rerun of `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa`
passed in Debug and Release. Both generated `cyberdeck.bmp` files have the
same SHA-256 (`BC1A6275E8CFD91B59896C7C3D05670DA4B1692B1C5A76D7D2A80C9F3E7C1C05`);
I inspected the full 320x224 Release capture beside independent frame `004920`,
which shows the matching Attack 3 detail view and intact bottom toolbar.

## CyberInfo input and shared palette cadence (2026-10-27)

ROM `0x18AA6` starts the selector at `FFF0DC=0`. Each pass at `0x18ACC`
waits, samples input, calls the shared palette routine `0x196F2` at
`0x18ADE`, then tests controls. Up (`FFDD22` bit 8) is checked before Down
(bit 9); both update the selector through `0xC376` with maximum row 1. That
helper wraps below row 0 to row 1 and above row 1 to row 0, drawing the cursor
from `$C4AC` with an `0x80`-byte row stride. A, B, and C are checked next
(`FFDD22` bits 14, 12, and 13) and all open the selected child: row 0 draws
Deck Stats at `0x18B5C`, row 1 draws Run Info at `0x18C28`. Start (bit 15)
calls `0x1882E` to restore the selected program detail and returns.

`CyberInfo.input` follows the same direction/action/Start order. The parent
passes its `deck_palette` state to CyberInfo, and `Cyberdeck.child_returned`
keeps the typed Deck Stats or Run Info result on the selected parent surface.
The source and host both advance the palette once per selector pass; this
confirms call-count behavior, not a wall-clock frame duration. The available
numbered video is sampled at 30 fps, so its palette phase does not settle the
timing between ROM VBlanks and host updates.

## Deck Stats longplay screen search (2026-10-27)

I inspected independent Section 15 frames `004200`, `004210`, `004220`,
`004230`, `004237`, and `004920` at native size. They show the main Cyberdeck
grid: an empty utility slot, Attack 3, and Medic 1. They do not show the
CyberInfo selector or the Deck Stats child selected from it. The Attack and
Medic details belong to the `0x18326`/`0x1882E` program-selection path; Deck
Stats is the separate `0x18AA6` selector and `0x18B5C` child.

I also sampled every fifteenth image (2 Hz) across the 324,461 numbered
320x224 frames in the early and rest longplay dumps, checking the green
`MPCP` and `Load/IO Spd` labels at their Deck Stats positions. No sample
produced a strong match for the full label set. This is a screen-search aid,
not proof that the screen never appears: a page shown for less than half a
second between samples, or a differently cropped/compressed view, could be
missed. The exact Deck Stats panel therefore remains without an independent
Genesis comparison. The source-positioned `deck-stats.bmp` remains a host QA
capture, not a substitute for that reference.

## Verification

- `shadowrun_ghidra_gameplay_script_menu_route` and
  `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` pass in Debug
  and Release.
- Route captures are written under
  `qt_runtime/field-matrix-cyber-info-qa/` at 320x224, including `actual.bmp`,
  `no-run.bmp`, `complete.bmp`, `cyber-info.bmp`, `cyberdeck.bmp`,
  `deck-stats.bmp`, and `pull-plug.bmp`.
- CyberInfo selector QA checks the exact Lua strings, final title glyph, both
  ends of the ROM separator, option-row origins, and cursor origin. Full
  Debug and Release frames check the retained parent row, program grid, and
  footer in CyberInfo, Run Info, and Deck Stats; the title is no longer clipped.
- Cyberdeck QA requires every pixel of the lower-left 32x32 footer portrait to
  match the source-composed `D8316` background region, preventing a field-menu
  preview from obscuring the silver avatar.
- The test checks the active CPU labels and values at their ROM positions and
  checks the signed absent/completed branches and their separate origins.
- These are source-position dumps of the Run Info text layer. The Cyberdeck
  chrome has an independent visual comparison above; its four ROM palette
  phases are implemented and covered by the route QA. The exact offset to a
  numbered compressed longplay frame is not asserted.
- Deck Stats follows the same boundary: its fields are source-positioned, but
  full panel art and an independent Genesis comparison remain open.
- Pull Plug's source-positioned prompt and retained deck chrome were checked
  against independent Section 15 `frame_005400.png`; the complete Matrix traversal viewport
  remains partial.
