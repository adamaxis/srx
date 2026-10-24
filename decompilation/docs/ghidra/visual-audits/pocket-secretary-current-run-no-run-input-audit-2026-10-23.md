# Pocket Secretary Current Run No-Run Input Audit

Date: 2026-10-23
ROM: US Genesis Shadowrun
Scope: Notebook `Current Run` with no accepted run, source response text,
controller wait, screen replacement, and return to the Notebook.

## Audit status note (2026-10-24)

The original scope and no-run findings remain valid. The positive-branch
paragraphs below preserve the 2026-10-23 source-analysis checkpoint; statements
that the page was unimplemented or lacked positive-page QA are superseded by
the dated implementation and QA section near the end of this audit. The hill
remains partial because an independent Genesis capture of the positive page
and restored-save acceptance coverage are still missing.

## ROM behavior

The `0xF2E2` Current Run handler reaches the ROM no-run response when the active
run state is absent. The source text reads:

> You are not currently engaged in a shadowrun. Get a Job!

The source text is laid out as three green rows. The response replaces the
Notebook list and waits through the shared `0xF312` controller loop.

Raw US ROM bytes at `0xF312` call, in order, `0x0BD2` (wait for VBlank),
`0x0BF2` (latch controller events), `0x195C` (sample input), and `0xE660`
(advance the lower-bezel effect). It then reads `FFDD22`, masks with `0xFF00`,
and loops while the result is zero. Any newly pressed controller bit therefore
dismisses the response; the held low-byte sample does not satisfy the wait.
The final `0x195C` sample follows the first nonzero event. Ghidra's
`fn_0000F312.cpp` confirms this loop and is only a cross-check.

The active Lua route in `menu/screens/pocket_secretary.lua` pushes the generic
`MESSAGE` screen with `grid_effect = true` for the no-run state. The shared
`menu/screens/message.lua` defaults its dismissal mask to `Pad.EDGES`, advances
the same E660 presentation state, and pops to the Notebook on the first edge.
This keeps the message and input wait in Lua-owned menu state; Qt paints the
typed state.

## Input and display coverage

`shadowrun_ghidra_conversion_qt_field_secretary_qa` now checks the ordinary
physical-input route. It enters the no-run response, checks all three text
rows and removal of the Notebook rows, holds the view for 30 frames, confirms
the text stays fixed while the lower bezel changes, and checks a clean return
to selected Notebook row 0.

The physical route then reopens the response and tests every nonzero controller
byte from `0x01` through `0xFF`, one fresh edge at a time. This covers each
direction, each face/Start button, and all simultaneous-button combinations.
Before each edge, QA requires the same three-row text and no intervening
visual change in its text region. After each edge, it requires the no-run
response to be gone, the Notebook to remain open on row 0, and no stale
response text. The zero-edge case is the preceding 30-frame hold, so the
manifest contains 256 entries: mask zero held, plus 255 dismissing masks.

Debug and Release CTest
`shadowrun_ghidra_conversion_qt_field_secretary_qa` pass. The manifest is
`qt_runtime/field-secretary-qa/notebook-current-run-input-masks.tsv`; both
configurations contain all 256 rows and have matching SHA-256
`72F60D8BB7C181A304F0AA10B5A272A556CBC824D38EDA47A8F962997CBD750E`. The
Debug full-screen captures `notebook-current-run-unavailable.bmp`,
`notebook-current-run-held.bmp`, and `notebook-current-run-return.bmp` were
opened at 320x224. They show the exact response replacing the Notebook list,
the held response with animated bezel, and the clean five-row Notebook return.
These are internal host captures, not an independent Genesis golden.

## Independent longplay search

To look for the exact no-run response, I sampled 10,815 frames at one-second
intervals across longplay sections 01-41. The locator compared a green-pixel
mask of the host's three-line text region `(48,48)-(216,72)` against the same
region in each sampled frame. Its strongest match scored 0.3551. I opened the
leading candidate at section 09/frame `003600` and representatives from
sections 03, 26, and 28; they show other NPC dialogue, not the Current Run
response. This is a phrase locator rather than OCR, and one-second sampling
cannot rule out a brief or differently rendered page. No independent Genesis
frame of this response was identified.

I separately reviewed the prebuilt longplay contact sheets for all 41 sections:
the early sections show 5-second anchors, and sections 08-41 show 10-second
anchors. None of the sampled frames shows the positive Current Run detail
page. This is sparse negative evidence; a brief visit between anchors could be
missed, so the independent positive-page screenshot remains unresolved.

## Positive Current Run branch boundary

The active-run path is distinct from the tested no-run message. Raw ROM at
`0xF2E2` tests word `FFFBEC`; a negative value takes the no-run text, while a
nonnegative value calls `0x1E72C` and then enters the same `0xF312` wait. The
dispatcher at `0x1E72C` indexes the longword table at `0xF3DFA` with
`FFFBED`. Current source run types map to these ROM handlers:

| `FFFBED` | Run type | Handler |
|---:|---|---:|
| 0 | Ghoul bounty | `0x1E770` |
| 1 | Bodyguard escort | `0x1EA94` |
| 2 | Package delivery | `0x1EB8E` |
| 3 | Gang contract | `0x1EC88` |
| 4 | Package recovery | `0x1E87A` |
| 5 | Employee recruitment | `0x1E8FA` |
| 6 | Matrix objective | `0x1EDF2` |

The same type-specific helpers also build Johnson offer text: `0x2148A` calls
the offer initializer, and `0x214B0` selects the first formatter from table
`0xF3E1E`. Its entries for types 0-6 point to the same helpers listed below
that the Current Run handlers call after common builder `0x1EE64`. Thus the
field mapping below covers both ROM text paths.

Each supported handler calls common builder `0x1EE64`, which clears the inner
panel and writes context text from table `0xF1D8C` at `$C208`, run-type text
from `0xF1DEE` at `$C29A`, and the live word `FFFBF4` at `$C322`. It then
formats type-specific text into `FFB740`; the handlers use different lookup
tables and bytes among `FFFBEE..FFFBF2`. The Matrix type-6 handler calls
`0x1EE0E`, which selects a named source string from `0xF25AE` using `FFFBEE`
and conditionally adds a `0xC19C6` location/event string when the source bounds
pass. After the handler returns, `0xF2E2` waits through the same `0xF312` loop.

### Positive-page input and return trace

`FFFBEC` is the signed active-run context word: negative selects the no-run
message, while contexts 0-4 are valid active-run values. It is not a Boolean
availability flag. From the Notebook loop at `0xF236`, A dispatches the
selected row; the positive `0xF2E2` branch builds the page, then both that
branch and the no-run branch enter `0xF312`. That wait polls once per VBlank,
advances E660, and returns on any nonzero high-byte controller edge. The
Notebook loop then redraws and resets its selection to row 0. The positive
view is a single detail page with an any-edge return; it has no detail-page
selector or scrolling action.

At the 2026-10-23 audit checkpoint, the all-mask physical QA only exercised
the no-run page. The implementation below adds positive-page input coverage
for every controller mask and Qt captures for all seven handler types; Lua
content tests separately cover the 13 initializers, route/client cases,
description variants, and Matrix objectives.

### Per-type display field trace

Raw handler helpers read these fields when building the current-run text.
`F1E58` supplies area names, and `F1F1A` supplies the nested location names.
Lua keeps the corresponding values in the accepted `run_registry.active_offer`.

| Type | ROM text inputs | Active-offer data path |
|---:|---|---|
| 0 Ghoul bounty | `0x1E78C` selects the area with `FFFBEE` for the fixed bounty text. The live `FFFBF0` ghoul count is not read by this text helper; it is multiplied by `FFFBF4` at payout. | `area_id`; `ghoul_kill_count`; `payment.negotiated_amount` (per ghoul). |
| 1 Bodyguard escort | `0x1EAB4` reads origin `FFFBEE/FFFBEF` through the area/location tables and client index `FFFBF0` through `F35A6`; `0x1EB32` reads destination `FFFBF1/FFFBF2`. | `route.origin`, `route.destination`, and `route.client_id` / `route.client_name`. |
| 2 Package delivery | `0x1EBAA` reads origin `FFFBEE/FFFBEF` and destination `FFFBF1/FFFBF2`; its final line is a fixed delivery instruction. It does not read `FFFBF0`. | `route.origin` and `route.destination`; the helper ignores the initializer's other random byte. |
| 3 Gang contract | `0x1ECA4` reads target `FFFBEE/FFFBEF` and selects the description variant from `F36FA[FFFBF0]`. | `route.target` and `variant`. |
| 4 Package recovery | `0x1E896` reads target `FFFBEE/FFFBEF` and appends fixed recovery text; it does not read `FFFBF0`. | `route.target`; the source initializer selects the four-site, two-site, or fixed Renraku target pool. |
| 5 Employee recruitment | `0x1E91A` reads target `FFFBEE/FFFBEF`, client index `FFFBF0` through `F35A6`, and fixed follow-up text; `0x1E9AC` appends the additional employer statement. | `route.target`, `client_id`, and `employee_name`. |
| 6 Matrix objective | `0x1EE0E` selects objective text from `F25AE[FFFBEE]`. If the objective index is below `0x11` and signed `FFFBEF` is nonnegative, it also selects a system/location string through `C19C6`. | `objective_index`, objective data, and `target_system_id`; Lua's `objective_text` composes the system name only for objective indices below 17. |

The related Ghoul offer generator has a separate context-sensitive area path:
raw ROM `0x1E744` fixes context 0 to area 2 and rolls 0-3 for other
contexts, while `0x1E78C` uses that field for its detail text. Lua stores the
selected area in `offer.area_id`. The offer presentation had hard-coded area 2;
it now uses the selected area. The Debug/Release `shadowrun_ghidra_run_actor_defeat`
test confirms context 0 remains Redmond and checks all four Mortimer/context-2
area rolls against their corresponding labels. This tests the Johnson offer text path; it does not implement
or visually verify Notebook Current Run.

At the 2026-10-23 audit checkpoint, the Current Run page had not yet been
projected from the Lua offer into the Qt presenter. Initializer bounds and
accepted-save invariants were still open at that time; the later implementation
and remaining checks are recorded below.

At that checkpoint, the positive branch was still marked as an explicit
`CURRENT_RUN_UNRESOLVED` boundary. ROM initialization at `0x1E69C` selects a
context row, stores the progress byte in `FFFBF3`, computes `FFFBF4` as the
row's base payment plus five times a bounded negotiation roll, and dispatches
through initializer table `0xF3DC6`. Shared payout at `0x1F00A` uses `FFFBF4`
directly except for Ghoul Bounty (type 0), where it multiplies by `FFFBF0`,
the live ghoul count. For Matrix type 6, `FFFBF0` is the target-system ID. The
active Lua offer retains the source initializer, context, canonical type,
payment, progress delta, and per-run data. The 2026-10-24 section below records
the completed typed presentation and the remaining save/visual boundaries.

## Shared text-builder detail and live offer corroboration (2026-10-24)

The common positive-page builder at `0x1EE64` clears the inner panel, writes
the static `JOHNSON:   TYPE:    PAY:      $` labels from `0xF3DA3` at `C208`,
indexes the five context names at `0xF1D8C` into `C21A`, indexes the seven
canonical type names at `0xF1DEE` into `C29A`, and writes saved negotiated
amount `FFFBF4` through grouped-decimal helper `0xC216` at `C322`. For Ghoul
Bounty the stored amount is per ghoul; shared payout multiplies it by live
`FFFBF0`. For Matrix, `FFFBF0` is the target-system ID. The other displayed
amounts use the saved negotiated amount directly.

The detail handlers append source fragments into `FFB740`, then call text
helper `0xC168` at `C40A` with `D1=0x1A`, `D2=0x6000`. Its decrement loop
scans 27 cells and wraps at the last fitting space; a trailing separator can
consume one scan cell, while a line without that separator can display 27
glyphs. Copy helper `0xC134` preserves `0x80` row-break markers, adds one
trailing space when `D1=0`, adds two when `D1=1`, and adds none for negative
`D1`. This is why route phrase boundaries must remain a continuous stream:
only source-marked breaks force a blank row; all other breaks depend on route
text length.

Type-specific body assembly recovered from the US ROM:

- Type 0 appends `0xF2520`, the area suffix selected through `0xF1E58`, and
  `0xF2549`; the live ghoul count is not part of this body.
- Type 1 appends `0xF23B2` (“Go to”), origin location and area, `0xF34C8`
  (“There you will meet my client,”), and the client at `0xF35A6`; it then
  appends `0xF34E7` (two row breaks before “Escort my client to”), destination
  location/area, and `0xF34FD`.
- Type 2 appends `0xF23B2`, origin location/area, `0xF23B8` (two row breaks
  before “Take it to”), destination location/area, and `0xF23ED`.
- Type 3 appends the target location/area and one of four `0xF36FA` variants
  selected by `FFFBF0`.
- Type 4 appends “Go to”, target location/area, and fixed recovery text at
  `0xF367C`.
- Type 5 appends `0xF39BE`, target location/area with the area period changed
  to a comma, client from `0xF35A6`, `0xF39CD`, and follow-up text at
  `0xF39EC` with its embedded blank row.
- Type 6 selects objective text from `0xF25AE[FFFBEE]`. For objective indices
  below `0x11` and nonnegative signed `FFFBEF`, it prepends the optional
  system/location text selected through `0xC19C6`.

The independent Genesis capture at
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_03_jump_house_stoker_job_0118-0238/frame_003150.png`
(global time 223s) shows a Courier offer: the source two-row break after
“A contact there will give you a package.” is present, followed by “Take it
to”; the remaining phrase boundaries wrap naturally. Frame `003300` (228s)
shows the context-zero offer changing from base `$40` to negotiated `$55`,
consistent with negotiation skill at least 3 and a bounded roll of 3. These
frames corroborate the shared run-offer text helper and payment path; they do
not show the positive Notebook Current Run page chrome. Bodyguard offer frames
from Section 04 similarly corroborate the shared body helper's route/client
phrases, not the Current Run presenter.

The Courier offer implementation previously inserted hard breaks after each
route phrase. `run_package_delivery.lua` now streams the origin/area and
destination/area phrases with the ROM's two trailing spaces, retaining only
the two explicit breaks from `0xF23B8`. Its focused test enumerates all legal
source Courier route tuples across the five contexts (12,090 combinations),
including same-location rejection for the context-zero pool. The route-matrix
test and `git diff --check` pass. The separate Current Run page implementation
and its seven-handler QA are documented below.

The 146-frame sample at 5-second intervals across the early 12-minute longplay
did not contain a positive Current Run page. This is sparse negative evidence;
it does not establish that the game never displayed the page. The independent
positive-page capture remains unresolved.

## Positive-page implementation and handler audit (2026-10-24)

`run_support.lua` now builds a typed Current Run view from the accepted
`run_registry.active_offer`. It resolves the source initializer to one of the
seven ROM handler types, carries the saved negotiated payment unchanged,
selects Johnson/type labels from the source tables, and assembles each handler's
source-derived body. The body uses the ROM's 27-cell `0xC168` scan and retains
the `0xC134` control breaks and trailing phrase separators. The Lua view test
covers all 13 source initializers, all 24 Matrix objectives, 12,090 legal
Courier/Bodyguard route pairs, and 145,080 Bodyguard route/client combinations;
it also covers all four gang-description variants and the recovery/recruitment
initializer pools.

The Lua menu opens `NOTEBOOK_CURRENT_RUN` (screen 41) when A is pressed on
Current Run and owns the shared `0xF312` wait. Any new high-byte controller
edge returns to Notebook row 0; the Lua route advances E660 once per source
VBlank. The typed `ScriptNotebookCurrentRunView` crosses to Qt, which paints
the preserved Notebook shell and the ROM-derived header/body coordinates. A
field QA failure exposed that screen 41 was missing from the Notebook-tree
screen predicate: the overlay closed even though the route snapshot contained
the page. Adding that screen to the predicate fixed the visible positive route.

The Debug/Release `shadowrun_ghidra_conversion_qt_field_secretary_qa` now
captures and checks all seven rendered handler types using accepted source
context rows spanning all five Johnson contexts. The Recovery frame uses
initializer 9 and its two-site target pool. It compares each label, number,
and body glyph against the extracted field font at the traced 8-pixel
coordinates, verifies the 27-cell
width, and checks a clean return to the same Notebook panel. The Matrix frame
uses the source Expert initializer/context row and exercises the grouped
maximum `$6,700`: base `$6,100`, Negotiation rank 12, bound 121, roll 120.
Its objective/system is an Expert-eligible Aztechnology target, so the same
capture checks the conditional system line. The Ghoul frame covers the low
payment edge: context 0, source base `$10`, Negotiation 0, bound 1, roll 0,
displayed `$10`. The video-matched Courier capture uses context 0, origin Ares
Weapon
Emporium `(2,11)`, destination Jackal's Lantern `(2,5)`, and the source Gator
Shaman negotiation row: base `$40`, skill 3, bound 4, roll 3, saved payment
`$55`. It holds that positive page for 30 frames, verifies E660 changes only
the lower bezel, reopens it, and tests all 255 nonzero controller masks. The
positive mask manifest has 257 lines (header, zero-edge hold, and masks
1..255) with matching Debug/Release SHA-256
`72F60D8BB7C181A304F0AA10B5A272A556CBC824D38EDA47A8F962997CBD750E`.

`run_registry.validate_saved_state()` now joins each source-backed accepted
record back to its exact context/initializer/base/progress row before restore.
It also requires a matching canonical handler and validates the stored
Negotiation rank (0..12), bound (`row scale * rank + 1`), in-bound roll, and
amount (`base + 5 * roll`) against the ROM's 16-bit payment field. The new Lua
save-validation audit exercises all 40 rows at all 13 legal skill ranks and
both roll endpoints (1,040 states), rejects mismatched context/initializer and
corrupt payment aliases, then accepts, saves, restores, and reprojects the
Section 03 Courier's `$40` to `$55` payment. A separate Debug/Release field
dialogue test drives that Mr. Gunderson offer through the Johnson interaction.
Its body checks follow the actual `||` control break: the package phrase stays
in the first paragraph and the destination/payment phrase stays in the second.

The seven Debug and Release type `actual.bmp` files match by SHA-256. Debug
captures are under
`decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/field-secretary-qa/notebook-records/current-run/type-{0..6}/actual.bmp`;
Release uses the same path under `build/conversion/qt/release`. These are
internal host captures and regression artifacts. The Section 03 Genesis video
shows the same Ares-to-Jackal offer text and negotiated `$40` to `$55` change,
but does not show the positive Notebook detail page chrome.

## Disposition

The no-run and positive Current Run branches now have Debug/Release physical
input coverage, and each canonical positive handler has a checked Qt frame.
The source-row/context/initializer/progress/payment restore invariants pass
the 1,040-state Lua matrix, and the live Johnson dialogue test accepts the
video-matched Courier at `$55`. Keep the Pocket Secretary hill **partial**:
there is no independent Genesis screenshot of the positive detail page, and
the live acceptance-to-save path has not been driven for every source row or
each handler type. The Qt seed exercises one valid context row per canonical
type; the Lua view matrix provides broader initializer, route, client, and
Matrix objective coverage. See the Notebook row in `HILLS.md`, the `0xF2E2`,
`0x1E72C`, and `0xF312` rows in `decompiled-function-status.tsv`, and the
field entries in `visual-analysis/qt-visual-audit.tsv`.
