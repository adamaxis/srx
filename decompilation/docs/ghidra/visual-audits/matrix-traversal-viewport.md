# Matrix traversal viewport

## Independent visual evidence

The supplied longplay has clear full-frame Matrix traversal views in
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_15_source_2820-3120/`.
Frames `004800` and `006600` in that Section 15 directory show a blue
perspective grid, the silver runner avatar, a faceted node, and live Matrix
status text. Section 15 frame `004800` is an Analyze-2 Active Alert view with
the raised-arm action pose and amber world effect. In the separate Section 21
directory, frames `004800` and `004820` show the same centered runner
silhouette; those frames do not establish an action-pose change. Section 21
frame `004820` shows No Alert and installed program icons; `004830` shows
Active Alert. Section 15 frame `005400` is a different state: the cyberdeck
command interface with a Pull Plug confirmation. The same frame number in
Section 21 shows traversal art, so comparisons below include their section
paths.
These references establish that the traversal viewport and cyberdeck menu are
separate screens.

Section 21 frames `004620` and `004680` are not traversal captures: the first
is a Datastore action panel, and the second is black during a transition.
The Sub-Processor Unit appears as a small green model at Section 21 frame
`004740`, grows at `004760` and `004800`, and reaches its large settled
presentation around `004830`. The Green-4 System Access Network is visible in
Section 15 frame `004440`; Section 21 frame `004440` is a different Matrix
node. These longplay frames establish the visual progression and model color;
they do not provide ROM timer or camera RAM values. A static centered model
asset cannot stand in for the source-driven entry motion.

## Current host behavior

ROM `0xE47A` waits 20 VBlanks, then enters `0x181F8`, which prepares the
ten-slot Matrix menu and falls through to the deck/program loop at `0x1822C`.
`matrix_entry.lua` now follows that route, and the ordinary System Search QA
checks that the Cyberdeck is the first Matrix screen. The `matrix-map-route-qa`
capture is a separate 14x10 topology view. It has no runner, faceted node,
perspective floor, or traversal status header, so it cannot stand in for the
longplay traversal frame. The cyberdeck menu capture is a separate view and
aligns with longplay frame `004220` in the CyberInfo/deck audit.

ROM exports distinguish the selected-system Matrix interface from its world
presentation. `0x181F8` initializes the ten-slot Matrix menu and falls through
to `0x1822C`, which owns the Matrix cursor/program loop. The field-mode scene
at `0x1745E` calls the 61-VBlank `0x177FC` setup loop (`0x3C` through zero,
inclusive), then `0x266E4` loads the
selected world resources and `0x2679E` runs the world-entry command stream.
That stream updates view offsets, draws the selected world tiles, and publishes
sprite records through `0x268E6`. The append routine reads packets from
`FFB740`, adds tile base `0x393`, and positions each packet relative to
`FFDD2A/FFDD28`. The extracted background does not include these live packets.
The six world-object model sources are identified. The first runner asset used
here came from descriptor E's single arms-down pose; it did not match the
longplay traversal runner. The final compact world-entry frame instead retains
phase four of the world-sequence figure. The host now composes the fixed
world-sequence body lists with all four selected animation groups for that
phase. The 56-frame source sequence timing and eleven independently measured
model extents are covered for the System 2 SPU route; other model profiles,
source packet projection, other program effects, and full actor cadence remain
open.

### Traversal HUD text path

`0x1745E` calls `0x1789C` during its initial field-mode setup. The latter writes
the menu HUD grid batches, then calls `0x17BC0`, `0x1989A`, `0x17A5E`, and
`0x17998` in that order. The renderers are also called from the later menu
service paths (`0x19D66` and `0x19F18`), so this is live state layered over the
traversal background, not text baked into the background asset.

At `0x17998`, the current node's `FF1100` record selects the function label
from `0xE309E` and writes it at Plane-A address `$C0B0`. The associated state
label is written at `$C0B8`; when the selected HUD row permits it, the security
value is written at `$C0C6` and the file-class line at `$C12E`. Fallback text
uses `$C134`. `0x17A5E` then uses the current node's status/class bytes
(`FF110F`, `FF1111`–`FF1113`) with the source tables at `0xE30DE`, `0xE30E7`,
and `0xE31A7`; its two status rows are placed at `$C1AE` and `$C22E`, with
numeric values gated by the selected HUD row. `0x17BC0` refreshes the active
deck/program row from `$FFFBD8` at `$C086`, or writes the repeated source grid
when that row has no active label. These addresses are byte addresses in the
name table; each 64-cell row is `0x80` bytes, or eight pixels vertically.

The independent Section 21 frame `004820` visibly carries both the node status
header and the active deck row. Those layers now come from Lua's traversal view
and deck record. The ROM's green top brackets were a separate omission: ROM
`0x1789C` calls `0x178C8` twice, writing five Plane-A grids per call from the
twelve words at `0xD86C8`. The host asset extractor now resolves those words
against the 147-tile Matrix menu atlas from `0xD78AA`, with the active HUD
palette at `0xE2E9A`, and emits
`cyberspace/matrix/traversal-hud-frame.png`. The presenter draws this named
transparent 320x224 layer over the Plane-B background and under the live
sprites. The CyberInfo Debug/Release QA checks source-expected green markers
at `(17,39)` and `(177,39)` in its full-frame traversal capture.

The previous arms-down descriptor-E asset has been withdrawn as a traversal
visual match. The correct settled entry pose combines the fixed lists
`0x1D428` and `0x1D452` with the selected groups at `0x1DAF4`, `0x1DB08`,
`0xEA490`, and `0xEA4A4`. `0x1D280` initializes the figure at phase zero with a
four-tick countdown; `0x1B364` advances its four source tables. The compact
entry stream lasts 239 VBlanks and leaves phase four visible. The offline
extractor emits that phase as
`cyberspace/matrix/traversal-runner-world-sequence-phase-4.png`, and the
presenter uses it instead of the narrow idle image. The traversal-entry
Debug/Release CTest checks the final runner's wide bounds and the full
320x224 capture. Its eleven green model bounds are compared against independent
Section 21 frames `004730` through `004830`.

### Runtime world-effect connection (2026-10-20)

Lua selects the effect mode from the current node's ROM IC-combo code and
alert state using the `0xE31A7` table. It keeps the selected mode, phase, and
elapsed source ticks in the Matrix session. Mode cadence and phase counts are
the bytes in the nine records addressed by `0x1A9CE`; a negative selector uses
the separate six-phase record at `0x1AA10`. C++ presents the corresponding
named, transparent images extracted offline from the descriptors and tile
banks, over the node model and behind the runner. The runtime does not decode
ROM data.

The earlier note tying
`build/conversion/qt/release/matrix-system16-node29-effect.bmp` to frame 2050
of `matrix-traversal-debug-interactive.srxkey` was incorrect. Replaying that
recording with the active frame debugger reaches Debug Pick System
(`menu.route.screen=28`, title `Pick System`, value `00`) at frame 2050; it has
not entered the Matrix. That old image is not a replayable capture from this
session and is withdrawn as route evidence.

The active Release executable was then driven from that paused frame with the
Qt frame debugger: select system 14, accept the source 20-frame entry wait,
open the node view, select Enter System, and confirm its first link. The
resulting full 320x224 capture is
`build/conversion/qt/release/matrix-debug-current-destination.bmp`. It shows
the Sub-Processor Unit, Ice Unknown, No Alert, the live runner, and an empty
deck footer. Pixel `(15,160)` is RGB `(0,0,146)` in both this production-app
capture and the focused QA capture, matching the blue floor in the independent
longplay sequence. This earlier capture is not a same-state comparison with
Section 21 frame `004820`: the reference identifies an SPU on Green-3 with No
Alert and installed programs, while the host frame has unknown node labels and
an empty footer.

After wiring the source `0x19788` service through Lua, the active debugger
replay loaded Medic-4 and Analyze-4 and waited through the ROM slot-delay
cadence. Its full-frame capture verifies the loader and icon placement but is
not the same program state as the Medic-1 longplay. The focused Debug/Release
CyberInfo CTest uses a Deception-1 Orange-4 node and an active world-effect
overlay; it now waits for the entry view to settle, checks both HUD-frame
markers, compares the selected model asset in unobscured regions, and retains
the complete blue floor and footer. The separate traversal-entry CTest routes
to the SPU Green-3 and compares the model's green-pixel bounds at eleven source
sequence anchors with independent Section 21 frames `004730` through `004830`.
The final full-size output also shows the phase-four wide runner. These focused
comparisons fix the HUD and runner mismatches and verify one model's entry
projection; the other node combinations, effect timing, and complete Matrix
screen remain partial.

The ROM chooses the entry stream at `0x14C11E` for a zero current-node type or
`0x14C186` otherwise. Each begins with `0x37`, which loads `D6`; the `DBF` at
`0x268E0` then publishes 56 model frames. A prior byte-count pass was wrong:
it classified high-bit bytes consumed as the second sample byte as camera
commands and did not follow the compact route's zero-sample branch. The source
reads are distinct: a high-bit first byte takes `0x26814` and subtracts its
upper/lower three-bit steps from `FFDD28/FFDD2A`; a nonnegative first byte takes
`0x2683C`, adds its upper nibble to `FFE1A2`, and consumes one second byte. A
nonzero second byte adds its low nibble to `FFE1A4` and publishes a model
frame. A zero second byte consumes one delay value through `A4=0x14C1EB`, then
continues without publishing a frame.

The corrected direct ROM walk yields 56 published frames on both routes. The
primary stream has 11 camera commands (total steps 11/10), accumulated sample
values `FFE1A2/FFE1A4 = 109/386`, no zero-sample delays, and final camera state
`FFDD28/FFDD2A = 0x113/0xDA` from the initializer's `0x11E/0xE4`. Its service
loop takes 179 VBlanks: 56 three-wait render cycles plus 11 single-wait camera
corrections. The compact stream has 11 camera commands (total steps 18/21),
sample values `84/410`, and six zero-sample delays from the independent stream
at `0x14C1EB`: `14, 12, 10, 8, 6, 4`. Each stored delay value adds that many
waits plus the branch's final wait, for 60 additional waits; the route takes
239 VBlanks total. Its final camera state is `0x10C/0xCF`. The command cursors
after frame 56 are `0x14C19A` and `0x14C20E`; the compact delay cursor advances
from `0x14C1EB` to `0x14C1F1`.

These accumulator totals are inputs to the `0x24F28` resampling pass. They are
not the model's pixel dimensions, screen coordinates, or a direct Qt scale;
the source model dimensions remain 112x64 for model 0 and 96x80 for the
compact models. This correction withdraws the earlier endpoint offsets,
sample totals, timing totals, and the claim that those camera counters proved
the host model origin. The observed `(110,28)` model-canvas origin remains an
independent longplay template measurement; its relation to the transformed
source sprite packet is unresolved.

The four-way dispatcher at `0x24E20` is now traced to its concrete world-entry
roles. Handler 0 (`0x24E44`) initializes shared lookup state and publishes the
`0x2621E` packed-byte-to-tile map. The selected resource record is passed in
through `A5` by the caller. Handler 1 (`0x24E84`) maps packed source bytes
through `0x2621E` into tile IDs. Handler 2 (`0x24EAC`) partitions the selected
resource grid into at most 4x4 sprite blocks and writes those packets to
`FFB740`. I initially classified them as floor chunks, but direct extraction
of the six source model grids shows they are Matrix world objects. Handler 3
(`0x24F28`) resamples the selected model patterns into the per-frame sprite
buffer; its input fields and packed-pixel transform are traced below, while the
host's per-frame size and placement conversion remains unverified.

The direct handler trace resolves the resampler's input contract. At `0x266E4`,
the selected model record supplies its pixel dimensions, copied into world
transform state at `FFE192`; the source tile grid is `14x8` for the `112x64`
model and `12x10` for the `96x80` models. Setup clears the horizontal and
vertical traversal accumulators. `0x2679E` points the source and destination
buffers at `FF9700` and `FF8700`, then accumulates the horizontal and vertical
transform values from the selected entry stream. `0x24F28` builds per-axis
sample tables from those values and the model dimensions, then runs the
packed-pixel transform. Its computed jump and bit-mask kernel are rendering
details, not a host runtime contract. `+0x10` and `+0x12` are requested sample
counts, but the live field route does not process all `+0x12` rows in one
resampler pass. The horizontal kernel pairs samples into packed output bytes
and handles an odd final sample; the destination layout tables define the
tile-address steps.

The field initializer sets `+0x16=1`, selecting the partial-group layout
builder at `0x26172` for this route; `0x2611C` is the full-group path. The
upload is fixed at `0xE00` bytes for a 14x8 model or `0xF00` bytes for a 12x10
model. `0x26172` builds 64 or 80 destination row-step words at `+0xAAC`.

The previous row-limit interpretation is withdrawn. Handler 0 (`0x24E44`)
clears `+0x24`, and neither `0x266E4` nor the caller before `0x2679E` sets it.
In `0x24F28`, a clear `+0x24` branches around the 32-row clamp at `0x25094`
and takes the return path at `0x25C84`; that clamp and its continuation are
used when `+0x24` is set. The old claim that the active route consumes
`min(+0x12, 32)` rows and preserves the remaining initialized image was based
on reading those branches backwards.

The active initializer also sets `+0x14=1`. The `0x24F28` setup path clears
`width_tiles * height_tiles * 32` bytes through `+0x1C`; at this call the
destination is `FF8700`, so the output model buffer is cleared before the
packed-pixel transform. The earlier statement that the remaining rows retain
the model image is withdrawn. The compact stream's `+0x12` accumulator reaches
410 while `0x26172` writes 80 row-step words. The exact interaction among this
count, the `+0x30/+0x3C` loop limit, row-table reads, and destination addressing
is unresolved. The transformed frame and source-derived screen placement stay
open; the measured Lua canvas remains a visual calibration, not a ROM
resampler implementation.

The sprite path has separate transition and regular descriptor behavior.
`0x1D1F4` and `0x1D280` set `FFDF8C` while the world-sequence reveal uses
fixed lists `0x1D428` and `0x1D452` with source pattern data at `0xDA038`.
During that transition, `0x1B364` also adds four selected groups from
`0x1DAF4`, `0x1DB08`, `0x0EA490`, and `0x0EA4A4`, loading their patterns at
VRAM keys `0x780` and `0x7C0`. The fixed lists also appear as phase 1 in
behavior descriptor C below; identifying that pair alone does not identify
the full live runner.

The initial field setup at `0x1745E` attaches behavior descriptor C for the
61-VBlank menu/HUD setup loop. That is not the descriptor used by the traversal
loop: `0x1ADE6` attaches descriptor E (`0x1B680`) before its first normal
world-effect frame. Descriptor E selects the primary and secondary list tables
at `0x1C34A` and `0x1C344`; each table has maximum phase index zero, so the
steady runner is a single source pose. `0x1C506` loads D1 from `FF158E=0x120`
and D2 from `FF1590=0x12F`. The sprite builder `0x1378` adds the first signed
list word to D1 and writes that result to SAT X, then adds the second signed
word to D2 and writes that result to SAT Y. The source origin is therefore
X=160, Y=175 after subtracting the Genesis SAT offset of 128. This
descriptor-E arms-down idle pose is not the phase-four world-sequence pose
visible in the settled entry frames.

The earlier diagnostic had both its list-axis interpretation and base
coordinate labels wrong; its descriptor-C comparisons are withdrawn. The
first replacement also selected descriptor E's arms-down primary and
secondary lists and was visually wrong. The longplay's traversal pose is the
four-group world-sequence figure, not those idle lists. ROM `0x1D280`
initializes `FFDF8C` at phase zero with a four-tick countdown. During the
compact entry, `0x1B364` selects groups from `0x1DAF4`, `0x1DB08`, `0xEA490`,
and `0xEA4A4` alongside fixed lists `0x1D428`/`0x1D452`, using the pattern
bank at `0xDA038`. The 239-VBlank sequence retains phase four. The extractor
composes those ROM-selected groups in draw order and emits the 320x224
`traversal-runner-world-sequence-phase-4.png`; the runtime draws this asset.
The final full-frame QA checks the expected wide silver bounds, while the
source-frame checks also validate the model's growth at eleven points from
Section 21 frames `004730` through `004830`. This supports the settled
System 2 SPU entry view; other nodes and the behavior-A raised-arm action pose
remain separate work.

The ROM's behavior-A action attaches at `0x1B5B8` after the Matrix world-action
selection path. Its primary and secondary tables are `0x1C33A` and `0x1C330`,
both with phase 0 and phase 1. Phase 0 reuses the compact bent-arm lists; phase
1 selects `0x1D50E` and `0x1D4E4`, which add raised-arm tiles from the `0xDAAB8`
bank at tile `0x16F` alongside body tiles in `0xDA038` at tile `0x11B`.
`0x1C3FA` advances both layer timers from 4 to 0 and then advances each phase,
so both arms reach phase 1 after five source frame calls. The action path's
`0x2D` countdown later reattaches descriptor E. The offline importer emits
both Behavior-A runner phases and the active Analyze route selects them from
the Lua countdown. Section 15 frame `004800` independently shows the raised-arm
Analyze-2 pose with the paired rings; the end-to-end QA matches the uncovered
runner pixels and ring phase to the named assets and checks its 42x41 bounds.
The exact longplay timer value is still unknown, but the ROM-derived cadence is
covered by the runtime test.

### Raised-hand effect trace

The first hand-effect diagnostic dropped the local X/Y offsets from the
`0x1DAE0` and `0x1DAEA` sprite records, and its `hand-ring` filenames therefore
claimed more than the rendered data supported. That diagnostic has been
corrected: `0x1BB6A` submits one 3x3 sprite from each list when the active
`FFE0CA` countdown's low three bits are at least four. The records use tile
`0x35A`, size `0x0A`, and mirrored attributes `0x835A` / `0x8B5A`. Including
their signed local offsets puts the sprite origins at screen `(86,106)` and
`(209,106)`. The extracted sprites make partial arcs near the hands; they do
not produce the full 36-pixel blue outlines in the capture.

The tile source is the uploaded `0xDAAB8` pattern bank, which `0x176BA` places
at VRAM byte address `0x2DE0`; tile `0x35A` is within that bank. The concurrent
`0x1B45C` noise waveform is not changing this tile: its `0x1B56E` transfer
writes 448 words starting at VRAM address zero, ending at byte `0x037F`, while
tile `0x35A` begins at `0x6B40`. The static partial arc is therefore the actual
art used by these two action sprites, not a stale preview of a later upload.

The paired lists advanced by `0x1C61C` also land near both hands. Its 32-entry
phase table selects lists from `0x1D480` through `0x1D4D0` and applies mirrored
horizontal offsets around the runner origin. I rendered all 32 phases and
composited the `0x1BB6A` paired sprites after them, matching the main loop's
draw order. The source layers form partial arcs and changing wrist accents;
no combined phase produces the complete outlines. These are adjacent action
effects, not a proven source for the circles.

The separate mode-animation art at `0x1AA40` contains a similar blue circular
glyph, but its four-phase table is drawn from the initialized origin
`(0x120,0xD0)`, or screen `(160,80)`. The inspected phase images show one
centered glyph, unlike the two circles that pulse around the runner's hands.

The adjacent three-phase cycle at `0x1A48A` is another single centered group.
It selects phase lists from `0xF1BB2` (`0xEB660`, `0xEB872`, `0xEBB4C` after
the eight-byte headers), then builds them from screen origin `(160,80)` with
local offsets. The complete sprite rectangles stay within `x=134..190`,
`y=60..102`. Its one-to-three sprite group does not reach either hand anchor.

The secondary effect service at `0x1CD5A` is another distinct candidate. Its
three pointer tables resolve to nine short streams: `0x1D158` selects
`0x1D906/0x1D918/0x1D922`, `0x1D140` selects `0x1D934/0x1D93E/0x1D948`, and
`0x1D14C` selects `0x1D952/0x1D95C/0x1D966`. Each stream contains one or two
records using tiles `0x310..0x34C` from the `0xDAAB8` upload. The service
starts at screen origin `(192,135)` after the Genesis SAT offset and moves
left/up by about `(0.5,0.875)` pixels per source frame. Its paired and
six-position variants spread those same small records around that origin.
Per-frame snapshot composition across its 61 source updates produces a
compact sprite scatter whose union reaches `(143,33)..(210,151)`; it does not
form outlines at the two hand anchors `(96,110)` and `(222,110)`.
The contact sheet is
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/secondary-effect-0cd5a-sequence-contact.png`.

The persistent record services `0x1CC24` and `0x1CCDC` share the nine sprite
streams returned by `0x1D0DE`: `0x1D538..0x1D588`. Their first source records
are 16–32 pixels wide but the decoded opaque art is only a few scattered
pixels. `0x1CC24` trails up to seven copies through a six-slot position ring;
`0x1CCDC` draws its separate record directly. The ROM-derived phase contact
sheet is
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-effect-motion-contact.png`.
The source art and trail geometry cannot account for the two continuous blue
hand circles in Section 15 frame `004800`.

The action-spawn path `0x1CB6A` uses those same glint streams. Its left/right
call sites seed separate records at `(128,135)` and `(192,135)`, closer to the
runner's torso than the hand-circle centers; `0x1CC24` renders their moving
positions and short trails. These are the small comet-like marks in
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/matrix-world-effect-motion-spawn-left-variant-0-phase-0.png`
and its right counterpart, not the hand rings.

The distinct `0x1CA12` motion path is also excluded. Its 32-entry phase table
`0x1CA8C` indexes nine direct lists through `0x1CAAC` (`0x1D862..0x1D8B2`);
each update draws one small record from tile `0x29C..0x2AE`. The opposing
variants begin at screen X 140 and 180, Y 130, then move inward and upward for
30 updates. The ROM-derived frame contact stays within X 132..187 and Y
75..133 and shows thin line marks, not two 36-pixel circles. Its contact
sheet is
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-motion-1ca12-sequence-contact.png`.

The 12-phase service at `0x1C866` is another separate animation. Its
four-tick phase counter chooses lists through `0x1C926` or `0x1C956`; those
tables reuse four lists each from `0x1D772..0x1D7C0` and their mirrored
counterparts at `0x1D7EA..0x1D838`. A service call selects one origin and one
table, not a left/right pair. These sprites use the `0x14C222` upload (216
tiles at VRAM tile `0x462`, including the referenced tiles `0x493..0x538`).
The two possible origins are screen `(140,130)` and `(180,130)`. Rendering all
12 phases puts the irregular three-to-five-sprite silhouettes around
`x=109..177` or `x=143..211`, `y=72..131`; the alpha-only render shows
twisting electrical clouds and does not match the circles around the raised
hands. The per-side phase contacts are
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-motion-1c866-left-phase-contact.png`
and
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-motion-1c866-right-phase-contact.png`.

The main-loop `0x1B87A` phase dispatcher submits one more candidate from its
four-entry `0x1BAD4` table. The selected lists `0x1D72C`, `0x1D736`, and
`0x1D740` each contain one 32x32 sprite, using tiles `0x252`, `0x262`, or
`0x272`. Their ROM-derived render is a square portal mark with a diagonal
stroke; the service builds one selected list at `FFE04A/FFE04C` per call.
That asset cannot supply the two circular outlines either. The centered list
preview is
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-phase-dispatch-1b87a-list-contact.png`.

The `0x1A570` helper also draws its two fixed lists (`0x0F1B1E` and
`0x0F1B28`) at screen origins `(170,128)` and `(149,128)`. Their opaque bounds
are `(163,118)..(188,137)` and `(131,118)..(156,137)`, with small streak-like
pixel art. They stay near the runner's center and do not form the separated hand
circles. The preview is
`build/conversion/qt/release/matrix-world-effect-trace-20261019j/world-effect-two-sprite-lists-1a570.png`.

### Analyze profile-transition ring source

The hand circles are the four-phase profile transition reached by the Analyze
handler at `0x1BE82`. Its success path sets `FFE0D6`, attaches behavior A with
`0x1B5B8`, initializes `FFE00C` to `0x3C`, and sets bit 7 in the runner's
`FF1592` behavior flags. The world-effect loop at `0x1ADE6` services
`0x1BEBA` while that bit is set. Each service pass decrements the timer and,
while it remains nonnegative, selects phase `((-FFE00C >> 1) & 3)` from the
four direct pointers at `0x1C136` (`0x1D8C6`, `0x1D8D0`, `0x1D8DA`,
`0x1D8E4`). The selected sprite list is submitted twice through `0x1378` at
SAT anchors `(0x15E,0xEE)` and `(0xE0,0xEE)`, which are screen positions
`(222,110)` and `(96,110)` after the Genesis coordinate bias.

The lists use the action pattern upload at `0xDAAB8`, placed at VRAM tile
`0x16F` by `0x176D8`/`0x176DE`. Their art uses tiles `0x2B7..0x2D3` and
`0x307..0x30F`; phases 0–2 draw one sprite per hand and phase 3 draws four
mirrored 3x3 sprites per hand. The builder's `D6=0x2000` selects CRAM line 1.
The traversal palette block begins at `0xE2E3A`, so its line 1 is `0xE2E5A`.
`0xE2EBA` is the Cyberdeck replacement palette installed by `0x17F80` and is
not the palette for these traversal effects. Using that Cyberdeck palette
produced the wrong pink/red diagnostic; the corrected offline extraction uses
`0xE2E5A`.

The normal asset import now emits four named 320x224 transparent overlays:
`assets/cyberspace/matrix/traversal-analyze-ring-phase-0.png` through phase 3.
I inspected the full phase images against independent Section 15 frame `004800`;
phase 3 has 42x41 px bounds per ring, and its blue outlines match the captured
anchors and visible size. The longplay sequence shows the rings during frames
`004784`–`004817` and gone by `004818`. That confirms visible shape and
sequence only: without live ROM RAM/SAT capture, I am not assigning a specific
timer value to Section 15 frame `004800`.

### Selected program actions

The active Lua node-action route checks the selected loaded program before
dispatching the node action. Left/right selects one of five slots; ROM
`0x199D6` maps C to attack, A to balanced, and B to defense, checking C before
A if the input word contains both. Analyze calls `matrix_programs.analyze`,
ORs its result into the node's existing `FFFB30` knowledge nibble as the ROM
does at `0x19D50`, and owns the
`FFE00C=0x3C` transition counter. The source decrements before selecting
`((-FFE00C >> 1) & 3)`, so the initial action VBlank enters the same service
sequence and the transition completes on its 61st service call when the signed
counter becomes negative. The first four calls use the bent-arm pose; the
fifth advances both Behavior-A layers to the raised pose. At completion Lua
publishes Success/Resisted and returns the runner to its separately extracted
idle pose.

Qt consumes only the typed runner/ring phases and named 320x224 assets. The
Debug and Release traversal-entry QA drives right+A, checks the selected
Analyze-2 slot and balanced mode, verifies phase 3 with the raised runner at call 18, verifies
that countdown zero is still active, then checks the result and idle pose
after call 61. It also compares every opaque pixel in the rendered ring layer
and uncovered runner pixels with the named assets, and checks the ring bounds
against the independently measured Section 15 frame `004800` geometry. The
completion pose and Resisted label match Section 15 frame `004818`; the test's
System 2 node is a different world state, so no whole-frame pixel match is
claimed.

The same field route then moves left to the loaded Medic-1 slot and confirms
Medic. Its ROM handler at `0x19C76` rolls the loaded rating against a target
number selected from current persona condition, adds successes up to 10, and
does not unload the program on failure. Lua commits the changed condition and
publishes the ROM success/failure slot phase. Deterministic Lua tests cover
both outcomes; Debug and Release field QA capture the complete 320x224 result
frame. No matching independent Genesis frame for this Medic use is available,
so that capture verifies route integration and full-frame composition only.

The Degrade slot follows the ROM dispatcher at `0x1B81A`: it runs the common
decker test and IC opposition, publishes result phases 7/8/9, and calls
`0x564D6` only for positive net successes. That routine subtracts
`max(1, net >> 1)` from the current node's security byte and clamps it to two.
The route commits the slot result and refreshes the typed node view. Debug and
Release Lua route tests cover success, blocked, and failed rolls. The same
field route seeds Degrade-3 after the independent entry and Analyze/Medic
checks: both builds publish Evaded, retain the node's security 3, and capture
the complete frame at
`field-matrix-traversal-entry-qa/matrix-degrade-action-result.bmp`. The hashes
match across builds. No independent Genesis frame of Degrade being used is
available, so this capture verifies route integration and full-frame
composition only. Attack (`0x1B734`) and Slow (`0x1B696`) now use the same
selected-slot gate and opposed rolls. Their result tails call shared ROM
helpers `0x56434` (Attack) and `0x56482` (Slow), also used by field-menu
service variants `0x19E32` and `0x19EA6`. Attack subtracts net successes from
the current IC condition; Slow subtracts twice net successes plus half the
loaded Slow rating from reaction, clearing condition if reaction crosses
below zero. Both publish the ROM result phase and retain their loaded program.
An IC defeat decrements signed `FFF0CE`; a negative result rolls the counter
to 15 and awards one Karma byte to each present party member. A typed optional
world-state adapter transports and saves that counter, and both programs
clear the signed passive-IC probe state at `FFF0A3`. Routes block these two
actions when the karma counter is unknown. ROM `0x00C2C` clears `FFF0CE`
during full RAM initialization and preserves it on the warm-reset short-clear
path. Field setup separately writes `FFF0A3 = 1` at `0x174FC` before Matrix
entry.
The field runtime's separate consumers of this shared counter remain open.
The helper at `0x564D6` is called by Degrade entry `0x1B81A`, not the program
entry itself. Other program handlers and the broader Matrix hill remain open.

The separate four-phase burst at `0x1BA6E` advances on a five-call timer, so
its timing initially looked plausible. The corrected extractor reads each
phase's raw pattern range from its upload packet and renders the sprite records
at a labeled center anchor. The four phase bounds grow from 40x41 through
96x80 pixels and form a broad noisy burst; they do not match the two
approximately 36-pixel outlines. This burst is another ruled-out visual
candidate, not the hand-ring source.

The earlier diagnostic also mislabeled the separate record installed at
`0x1B5FC` as descriptor B. The actual behavior B is installed at `0x1B612`
and uses tables `0x1C29E`/`0x1C294`; `0x1B5FC` selects the independent
single-phase lists `0x1D6CA`/`0x1D47C`. The extractor now renders the A and B
phase pairs using the correct pattern banks and gives the `0x1B5FC` path a
neutral name.

The mode-selected sprite path at `0x1AA40` is separate from descriptor E.
`0x1A908` calls `0x19B40`, selects a mode record through `0x1A9CE`, and
`0x1AA40` passes the selected record's coordinate fields as D1 then D2 to
`0x13BE`. Since the builder maps D1 to X and D2 to Y, the prior diagnostic's
reported lower-left placement came from the same axis-label error and is not a
valid comparison. The per-mode art and pattern-bank identity have been
inspected. Mode 1 does contain a blue circular glyph, but the `0x1AA40` origin
is centered and its single glyph does not match the two hand circles. The
separate `0x1A7BE` state sprite uses records from `0xF1AEE`; its screen role
remains distinct from descriptor E's static runner pose.

The three conditional `0x1AA40` overlays use tables `0xF1BDE`, `0xF1BCE`, and
`0xF1BBE`. The corrected images show that only the background table has a
blue circular glyph, as a single sprite at its centered inspection anchor;
its live X is dynamic (`FFE0C6 >> 4`). That glyph's location and internal
markings do not match the two hand circles in Section 15 frame `004800`. The separate
`FFB740` packets are six world-object models selected through `0xE2EDA`, not
the runner sprite.

I traced the nested overlay callers as well: `0x1AA40` operates on the one
record at `FF163E` and emits its selected mode descriptor once per scene tick.
The primary and secondary overlays use separate records at the same fixed
screen Y (57) and moving X values derived from `FFE0BE >> 6` and
`FFE0BA >> 6`; their inspected source frames are small colored marks. They do
not submit the two large outlines at `(96,110)` and `(222,110)` seen in frame
`004800`.

The remaining six-phase center animation is `0x1B218`, enabled by `FFE0F6`.
Its pointer table at `0x1DB92` selects lists `0xF092E`, `0xF0BA8`, `0xF0DE2`,
`0xF103C`, `0xF1236`, and `0xF1490`; it builds them from origin
`(0x10D,0x12E)` with tile base `0x6E0`. Each list starts at local Y -19 to
-29, placing its first large sprite at screen Y 145 to 155, below the hand
circles. It is not their source.

The alternate presenter at `0x53704` is also not the wide traversal figure.
`0x53248` builds one 16x16 sprite from the single record at `0x15FBFA`, and
`0x537FA` adds one 16x16 sprite from `0x15FC04`; both use per-runner origins
from the selected `FF1100` record. The pointer tables at `0x15FC0E` and
`0x15FC6A` feed the runner-selection grid cells. This branch can render the
selection UI, but its two small sprites do not account for the approximately
240-pixel-wide silver figure in Section 15 traversal frame `004800`.

### World-object model source: 0x17858 / 0xE2EDA

After the 60-tick service loop at `0x177FC`, the routine selects a 12-byte
model record at `0xE2EDA + (FF1100[FFF118].byte0 * 12)` at `0x17858`: it reads
the current Matrix node from `FFF118`, indexes that node's 20-byte record in
`FF1100`, and uses its first byte as the model id. Each record stores pixel
width and height, a tile-map pointer, and a raw pattern-bank pointer. The six
records decode to one 112x64 model and five 96x80 models. `0x26696` copies the
map-selected 8x8 patterns into the world buffer; `0x24E84` converts packed
pixels through `0x2621E`; `0x24EAC` partitions that grid into linked Genesis
sprite blocks. `0x268E6` applies the live view offsets and appends those blocks
to the SAT.

Lua's `matrix_session.node_status` exposes this source-derived id as
`world_model_id`, and the typed route snapshot validates the 0..5 range. The
traversal presenter consumes it to choose the named model asset. For the
compact System 2 SPU route, Lua maps elapsed source ticks to a measured canvas
using the independently observed green bounds; the Debug/Release entry CTest
checks eleven positions. The bound scan starts below y=45: the previous scan
included the green HUD bracket at y=38, so its measurements were not
object-only. The corrected samples use the green-object extents in independent
Section 21 frames `004730` through `004830`. Debug and Release captures now
match those reference bounds within two pixels at every anchor. This remains
visual scale and placement calibration, not a decoded source-packet transform.
Other model IDs and the source packet-to-visible projection remain open.

The world-model palette selection is dynamic. `0x176EA` initially copies the
four-line block at `0xE2E3A` into the palette shadow, and the world-model SAT
attributes select palette line 0. During metadata refresh, `0x19DF2` reads the
current node's security color from `FF1105`, multiplies it by `0x20`, and calls
`0x19DD8`; `0x566D8` then copies 16 colors from `0xE2D52 + color*0x20` to
`FFD87E`, replacing line 0. Live Matrix data uses security-color IDs 1..4,
with source rows at `0xE2D72`, `0xE2D92`, `0xE2DB2`, and `0xE2DD2`. The earlier
extractor incorrectly baked the initial gray palette `0xE2E3A` into every
model. It now imports each of the six model patterns in all four active
security colors, and the presenter selects the asset from Lua's typed
`palette_theme`. The floor remains on its independent blue palette.

I reopened the independent Section 21 frames at full resolution. Frame `004830`
shows the Sub-Processor Unit's faceted model in green; the old host comparison
used a different system/node and rendered the model in gray. The earlier claim
that the gray model asset matched the green reference, plus the claim that its
center color established a separate amber model layer, is withdrawn. The
security-color palette mapping is ROM-traced. The corrected host scan excludes
the HUD bracket, and the eleven System 2 SPU samples now use the object-only
bounds from Section 21 frames `004730` through `004830`. Rendered bounds are
within two pixels of the independent frame measurements at every anchor. The
host still scales one named model image; its facet shape and the ROM's
per-frame packed-pixel resampler remain unresolved. Other model profiles,
entry streams, and same-state world-effect comparisons remain open.

The mode selector at `0x1A908` calls `0x19B40`; the selector reads the current
node's `+0x13` IC-combo code and applies `0xE31A7` under the `FFDF84` and
`FFF0A0` gates. Lua's imported node data feeds that selector into the
traversal view, and the presenter draws its extracted phase. The active-alert
longplay state still needs a matched capture in the runtime. The asset match
alone does not prove perspective placement or complete scene timing. All six
source model identities are mapped; independent longplay comparisons currently
anchor models 0 and 2. The other four models still lack independent visible-node
anchors, and full projection behavior remains open. This corrects the earlier
floor-chunk classification.
The silver runner is traced separately below.

### World sprite placement resolution

The direct `0x24EAC` packet builder divides the 12x10 model grid into 4x4-tile
sprite blocks. Its first packet has local X/Y `(0,0)`; the remaining packets
advance by 32 pixels along the grid axes. `0x268E6` adds `FFDD28` to packet X
and `FFDD2A` to packet Y before publishing those words in the sprite records.
The corrected compact stream ends with `FFDD28/FFDD2A = 0x10C/0xCF`. For the
first packet, `0x268E6` adds those values to its local `(0,0)` offsets; subtracting
the Genesis SAT bias of 128 gives packet coordinates `(140,79)`. This does not
explain the visible model's placement in Section 21 frame `004830`, so the
earlier claim that `(110,28)` was derived from the compact command stream is
withdrawn. The `(110,28)` origin remains a visual placement template for the
compact models, not a source-derived packet transform. For System 2's SPU,
Lua interpolates a measured canvas through the eleven independent bounds and
Debug/Release QA checks each source-frame anchor. The other model profiles and
model 0's `(104,48)` fallback placement remain unverified.

The prior template scale measurements used mismatched frame identities:
Section 21 frame `004620` is a node-action panel and `004680` is black. Those scale values and
the claim that the model was already full-size at `004760` are withdrawn. The
valid independent Section 21 growth sequence is `004740`, `004760`, `004800`, `004820`,
and `004830`. A masked template comparison anchors model 2, security color 2,
at approximately 0.925 scale and origin `(113,33)` in Section 21 frame
`004830`; this is a visual measurement, not a ROM-derived transform. The host
now reproduces the measured System 2 SPU growth at eleven source-tick anchors;
this does not reconstruct the ROM's packed-pixel resampler or establish
transforms for the other models.

### Settled SPU extent correction (2026-10-21)

The entry QA's green-model scan began at screen row 45 to avoid the HUD
brackets. The settled SPU in independent Section 21 frame `004830` extends
above that line, so the scan clipped its top and recorded `(120,45,76x61)`.
That clipped extent drove an overly short vertical canvas in Lua. The full
independent object extent is `(122,38,74x67)`, matching the separate measured
anchor already recorded in the status below. The Lua endpoint now uses those
full bounds. QA still scans the viewport below the HUD baseline, then checks
the final top facet in a narrow central strip that excludes the bracket. The
Debug and Release traversal tests pass, and both full 320x224 captures were
reopened beside frame `004830`; the settled host object now reaches the same
top and bottom rows. This corrects the endpoint canvas calibration only. The
ROM resampler, intermediate facet changes, and packet-derived placement remain
unimplemented or unverified.

The offline `--matrix-title-figure-atlases` diagnostic emits descriptor-C
phase pairs, the world-sequence groups, behavior-A/behavior-B phase pairs, the
`0x1B5FC` transition, and descriptor D/E lists. All lists sharing `0x1C506`
use X=160 and Y=175, with the source X word before the Y word. The world-
sequence phase-four asset and behavior-A raised-action layer have separate
catalog entries. Typed traversal state and a static viewport presenter are
connected; action-pose timing and the complete viewport remain open.

The transition diagnostic reads descriptor `D2` as a byte count.
`0x0F5C` consumes `D2` in 64-byte blocks, then trailing words and an optional
final word. The first figure block is `0xC0` bytes and ends exactly at its
following descriptor. For both live and transition records, the size/link
word's low byte supplies the sprite size: bits 0-1 encode vertical tiles and
bits 2-3 encode horizontal tiles, and the ROM helper promotes that byte into
the Genesis SAT size field. The Sega software manual maps multi-tile sprite
patterns in vertical columns. These are source-derived extraction details;
the complete live pose still needs a matching capture comparison.

The world-effect driver at `0x1ADE6` calls `0x54662` after its world-sequence
transition completes, unless `FFF0A9` selects runner selection at `0x53704`.
`0x54662` is the node-action loop, not the perspective viewport. Its six-row
action table at `0x1664D8` names System Access, Central Processing Unit,
Sub-Processor Unit, Input/Output Port, Datastore, and Slave Module. Their
visible actions are Enter System; Go To Node / Cancel Alert / Crash System;
Leave Node; Leave Node / Lockout; Leave Node / Transfer Data / Erase; and
Leave Node / Turn Off Node. The corresponding action-pointer lists are rooted
at `0x166540`, `0x166590`, `0x1665C6`, `0x1665FE`, `0x16667E`, and `0x16663A`.
The row's action-count byte and function-pointer list select these callbacks.
The loop's Start edge hands off to the Matrix program loop at `0x1822C`.

The Grok plan's Phase 9 debugger (`SR/refactor/debugger/srxdbg` and
`srxcap.py`) targets the retired SR port and is not available in the active
Ghidra conversion tree. The active `frame_debugger.py` provides headless host
status, replay seeking, stepping, input, state inspection, and full-frame dumps.
I used replay seek plus ordinary input to reach System 16's node 29, then
inspected its typed Lua state and full 320x224 host capture at source frame
2050. The debugger exposes host state, not live ROM SAT/RAM. ROM bytes and
independent longplay frames remain the sources for the Matrix trace.

The active route connects typed Lua traversal state to a static Qt presenter
for the runner, model, perspective background, HUD frame/text, deck footer,
and ROM-selected world-effect frame. The mode selector and cadence come from
the ROM records at `0x1A9CE` and `0x1AA10`; extracted phase images are named
assets. The Debug/Release entry CTest reaches the System 2 SPU route and checks
eleven independent model extents, the phase-four wide runner, and a full
320x224 capture. The CyberInfo CTest checks the HUD markers, settled Orange-4
model, blue floor, and footer. Full captures were reviewed beside the longplay;
video compression and alert/effect timing differences mean the tests do not
claim pixel-identical full-screen goldens. Other model profiles, action hand
effects and timing, actor service cadence, and the complete traversal return
remain open.

### Traversal floor palette correction (2026-10-20)

The presenter previously selected one of four floor images from the current
node's `palette_theme`. That produced an orange floor for the QA node labeled
SAN Orange-4. The independent longplay shows the floor stays blue with a Green-4
SAN in Section 15 frame `004440`, and remains blue through the SPU traversal in
Section 21 frames `004740`, `004760`, `004800`, `004820`, and `004830`. Section
21 frame `004620` is a Datastore action panel and `004680` is black during a
transition; neither is a traversal-floor reference. The ROM field-mode setup
at `0x1745E` installs the shared traversal field patterns and Plane-B streams;
the node color remains a separate typed status value. The presenter now uses
the blue base floor regardless of that status color.

The focused Debug/Release `field_matrix_cyber_info_qa` checks a blue-dominant
floor-grid pixel at `(15,160)` in the full 320x224 traversal dump. In the
independent Section 15 frame `004440`, that pixel is RGB `(3,1,133)`; this
identifies the blue-floor reference, while the test checks blue dominance
rather than claiming exact pixel equality. It fails on the prior orange
renderer output. The new captures were opened at full resolution; the footer
and bottom scanlines remain present. This fixes the floor palette mismatch
only; the complete viewport remains partial.

## Status

The System 2 SPU projection is visually calibrated, not derived from the ROM's
packed-pixel transform. Its measured bounds progress from `(159,100,3x3)` at
Section 21 frame `004730` to `(122,38,74x67)` at `004830`; the Debug/Release
entry test compares eleven intermediate extents. The final host capture shows
the phase-four runner with its arms outstretched, the green faceted SPU, the
top HUD frame, blue grid, and complete footer. The CyberInfo capture verifies
the same HUD layer on a different selected model. The complete Matrix hill
remains partial: other node/model profiles, action overlays, their exact
cadence, and the field return path are not defended. Do not use the flat
system-map capture or node-action panel as a traversal reference.
