# Hills

Standing order is in `AGENTS.md`. This file is the progress index. A hill is
defended only with a named CTest plus a framebuffer dump. Same-source visual
composites do not count. Apply the question-by-question evidence process in
[`visual-verification-checklist.md`](visual-verification-checklist.md) to each
screen change.

### Field entity edge culling

At ROM `0x4618`, each active entity is projected by `0x5A62`; `0x5A98`–`0x5AB4`
culls on the projected anchor. The accepted window is x `0x70..0x1AD`, y
`0x63..0x179`, or screen x `-16..301`, y `-29..249` on the 320×224 field.
Enemies vanishing outside this anchor window is ROM behavior.

CyberInfo Run Info now keeps the ROM's signed `FFF0E3` branches in Lua and
places active node/function/action text at `$C328`, `$C334`, `$C4AC`, and
`$C630`; the no-run and complete messages use `$C42C` and `$C42E`. Debug and
Release route CTests write matching 320x224 `actual.bmp`, `no-run.bmp`, and
`complete.bmp` frames. Deck Stats returns the live Lua deck record and places
maker/model at `$C1AE` and grouped values at `$C346`, `$C3C6`, `$C4C6`, `$C546`,
`$C5C6`, `$C6C6`, `$C7C6`, `$C846`, `$C8C6`, and `$C946`; its source-position
test writes `deck-stats.bmp`. These checks validate the traced text layers.
The CyberInfo selector also now follows the exact padded stream at `0xE3886`:
title at `(192,48)`, bracketed separator at `(176,56)`, choices at `(192,72)`
and `(192,88)`, and cursor base `$C4AC`. The full CyberInfo frame is captured
in Debug and Release; QA checks the final title glyph and both ends of the
separator. The title no longer clips at the right edge.
ROM `0x18AA6`, `0x18B5C`, and `0x18C28` rewrite the right panel at `$C126` but
leave the selected parent deck row, installed program grid, and footer in
place. Lua now passes that parent row plus live deck artwork state to the
presenter; CyberInfo, Run Info, and Deck Stats QA checks the retained assets in
full frames. The Cyberdeck hill remains partial pending broader Matrix visual
and action coverage.
The 2026-10-27 Cyberdeck route QA now walks all sixteen grid cells with normal
controller input. It checks each selected row, column, label, cursor and
retained deck/footer surface; all four installed program value tuples; and the
ROM-positioned `Empty / Utility Slot` response for eight uninstalled programs.
That pass found and fixed the empty-slot heading's one-tile left shift from
`0xE39B7`; the first glyph now starts at `(200,64)` from `$C42C`. Independent
Genesis comparison is available for the Attack 3 state at frame `004220`; the
other numeric states and empty-cell message are checked against ROM data.
The 2026-10-28 Deck Stats search checked all 324,461 numbered longplay frames
at 30 fps against the ROM-positioned `MPCP` and `Load/IO Spd` text rows. The
strongest combined mask score is 0.2757 on a Stoker dialogue page; Section 15's
highest score is 0.2076 on the `Jack Out` detail page. A known Cyberdeck frame
scores 0.0747 and the host Deck Stats capture scores 1.0000 as a positive
control. No frame strongly matches both rows, so the exact `0x18B5C` Deck Stats
page still lacks an independent Genesis comparison. See the
[CyberInfo and Deck Stats audit](visual-audits/matrix-cyber-info-run-info.md).
The same audit traces `0x18AA6` input priority through `0xC376`: Up before
Down, A/B/C to the selected child, and Start back to the selected program.
ROM entry `0x182F4` clears the shared `FFF10A` palette timer; `0x196F2`
rotates CRAM line 1 entries 8-11 immediately on its first call and every five
VBlank passes thereafter, for a steady 20-pass four-phase cycle. CyberInfo
inherits that counter. Section 15 Attack-3 frames `004215..004224` independently
show the four phases in order, with each classified phase lasting two or three
30-fps samples, consistent with one palette change per five VBlank passes.
Exact phase offset and 60-Hz alignment remain open; the footage does not show
the CyberInfo child. The parent cursor, child dispatch, selector, and Deck
Stats/Run Info draw paths make no local sound-event request; the field caller
separately sends audio command-frame 4 on entry, and event `0x18` is confined
to conditional Pull Plug consequences. No independent sound timing comparison
is available.
The same parent loop resolves simultaneous Up/Down/Left/Right input in that
order, directions before buttons, A before B before C, and commands before
Start. The Lua cursor helper already matches the direction order; `Cyberdeck.input`
now matches the command/Start priority, so combined command-plus-Start input
dispatches the command while Start alone leaves the deck. This change has
ROM/code review plus a 256-mask sweep through the live route at the CyberInfo
cell; `shadowrun_ghidra_gameplay_script_menu_route` passes in Debug and
Release. The test checks cursor state, selected mode, child entry/return, and
Start-only exit. The Cyberdeck/Matrix hill remains partial pending broader
input and independent visual coverage.
The Storage screen at ROM `0x18D1A` keeps that parent row, program grid, and
footer while drawing the source panel and live capacity. View at `0x18F5E`
walks occupied records from slot 4 down to 0, drawing each ROM filename,
separator, and size; Start returns to Storage. Erase begins with the highest
occupied slot, defaults to Yes, cycles through occupied slots on No, deletes
the chosen record on Yes, and shows the remaining list. Empty storage writes
the ROM's two-line message and returns to the deck. The Debug/Release QA writes
full 320x224 `actual.bmp` captures for the root, empty view, file list, erase
prompt, post-erase list, and empty-after-erase state. All six Debug/Release
pairs have matching SHA-256 hashes; the full frames were inspected. These are
source-position and internal regression checks, not independent Genesis pixel
proof. The Storage screen hill is defended by the traced Lua state, named CTest,
and full-frame dumps; the wider Cyberdeck/Matrix hill remains partial.
The loaded-program prompt at `0x19238` also redraws only the right panel,
starting at `$C628`; the selected program detail panel above it and the parent
grid/footer remain visible. Lua carries the parent program's name, rating, and
detail values into the child screen. The `0xE3859` stream advances one text row
on `0x80`, so the two message lines begin at `(160,96)` and `(160,104)`; the
question, options, and default-No cursor follow at `(184,112)`, `(168,120)`,
`(232,136/152)`, and `(200,152)`. The Debug/Release
`shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` test writes the
full-frame `deck-program-prompt/actual.bmp` and checks both prompt glyph bounds,
the retained Analyze detail panel, selected row, installed icons, footer, and
No default. I inspected both 320x224 captures; their SHA-256 hashes match.
There is no independent Genesis capture of this prompt, so this remains
source-position and regression evidence rather than independent pixel proof.
The row-3/column-3 Pull Plug prompt now retains the selected Command grid and
footer. ROM `0x1846E` / `0x18480` / `0x18492` place the optional warning,
question, and Yes/No rows at `$C228`, `$C426`, and `$C638`; `$C632` places the
cursor with No selected by default. QA writes `pull-plug.bmp` in both builds.
I inspected the full Release frame beside independent Section 15 longplay frame `section_15_source_2820-3120/frame_005400.png`;
the grid, prompt, cursor, and footer align. ROM `0x196F2`'s four-phase deck
palette cycle now has Lua-owned cadence and named extracted assets; Debug and
Release QA captures cover all phases. The full Matrix traversal viewport
remains open. A focused ROM trace now places `0x1AA40` ticks inside the
`0x1ADE6` entry sequence; the blocking `0x54662` node-action loop holds the
selected phase, and its first alert probe runs after the entry handoff. The
mode-3 fixture's System 15/node 13 reaction index 15 adds `-2` to raw cadence
7, producing reload 5. Debug/Release route and Lua tests cover the 61-VBlank
setup, 238 effect service calls, deferred alert, and held action-screen
phase. The typed mode-3 fixture predicts phase 4 at handoff, while phase 5 is
the closest presenter match for longplay frame `004800`; the live visual mode
and phase remain open. ROM bytes confirm `0x1745E` clears `FFDF84` on every
outer pass, and its `FFDF40` branch writes the separate `FFE171` byte. Alert
changes at `0x54A8E` and Cancel Alert at `0x54AEA` change `FFF0A0` without
writing `FFDF84`; Lua now tracks the world-effect gate separately. The
`0x1A370` -> `0x561E8` -> `0x56116` controller is skipped while `0xFFDF8C`
marks the active `0x1ADE6` world-sequence transition. That branch runs the
figure animation and bypasses the profile/effect service block containing
the `0x1B706` callers; its one timer helper, `0x1BF7A`, does not call `0x1B706`.
When the sequence completes, `0x1ADE6` immediately enters the blocking
`0x54662` action loop. Mode reselection remains possible in the normal
`0xFFDF8C == 0` controller path before a transition begins. The direct
`0x181F8` menu path now receives the signed
`FFDF84` marker from the field timeout state; a Scene 2 ROM encounter QA path
verifies the negative value reaches Lua when Matrix opens. The live visual
mode and phase remain open.
Matrix map RAM now survives direct re-entry into the same system. At
`0x17490`, the ROM compares `FFFBBC` with `FFFBBD`; when they differ,
`0x53290` clears the `FFFAA4` revealed-cell flags and `FFFB30` cell records
before copying the selected system ID into `FFFBBD`. Its 35 longword writes
clear exactly 140 bytes in each array. The typed Lua bridge now
carries those 140 knowledge/color records and revealed flags across same-
system entries, and clears them on a system change or fresh bootstrap. Analyze
and node-action knowledge updates preserve the low color nibble. This transient
RAM is not added to save data. Lua Debug/Release tests cover same-system
retention, mode selection from a restored rank, and system-change clearing;
the host route test covers the typed round trip. The live frame-debugger route
to System 15's System Map also matches independent Section 15 longplay frame
`004520`: the full 320x224 grid, cell spacing, labels, and visible node
positions align. The longplay shows one additional revealed cell from its
prior play state, so this checks screen geometry and observed cells rather
than asserting equal map memory.
Cyberdeck chrome now uses the live Matrix palette override: ROM `0x17F80` /
`0x566E2` copies sixteen words from `0xE2EBA` into CRAM line 1, replacing the
initial line from `0xE2E5A`. The prior extractor used that initial line, which
made the tabs and frame green/yellow instead of blue. The full 320x224 Debug
and Release captures were inspected side by side with independent longplay
frame `section_15.../frame_004220.png`. That comparison exposed a separate
RLE off-by-one: the offline extractor omitted four main-plane cells and eleven
footer cells from decreasing runs. It now follows the ROM's inclusive loop;
the footer includes the active portrait, installed and empty slots, and status
strips at their source coordinates. The Cyberdeck and Matrix traversal hills
remain partial because the full Matrix screens, actions, and return
choreography are incomplete. The static traversal view now paints the
source-selected world model, fixed blue perspective floor, typed HUD, deck
footer, active world-effect phase, and the wide runner pose visible in the
longplay. Its floor stays blue across node security colors; Debug/Release QA
checks the independent Section 15 frame `004440` grid sample at `(15,160)`.
The four deck-background palette phases remain separately covered by
Debug/Release QA.

The missing top HUD artwork is now an individually named transparent asset.
It is composed offline from the two five-grid Plane-A batches at
`0x1789C`/`0x178C8`, source words at `0xD86C8`, and the menu tile atlas loaded
from `0xD78AA` with palette line 3 from `0xE2E9A`. The presenter places it
above the Plane-B traversal floor and below object and runner sprites. The
CyberInfo Debug/Release CTest checks both bracket markers in full-frame
captures. The former descriptor-E idle image had arms down and did not match
the longplay. The selected entry layer now composes fixed world-sequence lists
`0x1D428`/`0x1D452` with animated groups from `0x1DAF4`, `0x1DB08`, `0xEA490`,
and `0xEA4A4`, retaining phase 4 after the compact 239-VBlank entry.
Debug/Release traversal-entry QA compares eleven model bounds with independent
Section 21 frames `004730`–`004830` and checks the wide runner in its full
320x224 capture. Its scan now starts below the green HUD bracket, which the
previous scan had counted as part of the model. The corrected bounds match the
independent frame extents within two pixels at every anchor; the named image's
facet shape and the source resampler remain unresolved. Both Matrix traversal
CTests pass in Debug and Release.

All six source model identities are mapped. The active importer emits the
neutral unknown-node palette plus all four ROM-selected security-color
palettes for each model: `0x19DF2` selects row zero while the node rank is
unknown, then reads `FF1105` for known nodes; `0x566D8` replaces CRAM line 0
from the matching `0xE2D52` record. Section 21 frames `004620` and `004680`
are a node-action panel and black transition, not model-size samples. The
System 2 SPU entry projection has independent object-bound checks across the
source sequence. Complete packet-to-screen conversion, facet appearance, and
other model entry profiles remain open. Section 15 frame `004440` also anchors
the settled System 1/node 0 Green-4 System Access model 0. Lua projects its primary
179-VBlank endpoint from its 112x64 source canvas; Debug/Release QA matches
the independent green bounds and largest-component silhouette (IoU 0.858).
The host uses Medic-1 while the reference uses Analyze-2, so this compares
the model crop only. Other model colors and intermediate primary projection
remain open. The
changing traversal HUD text path is traced through
`0x17998`/`0x17A5E`/`0x17BC0`, including its Plane-A write addresses, and is
presented from typed Lua state. The nine `0x1A9CE` world-effect modes and the
`0x1AA10` default record import 82 individually named phase overlays using
CRAM line 1 at `0xE2E5A`. At `0x1ADE6`, Lua captures the selected mode from its
typed node, alert, and gate inputs before the entry sequence; source mode
selection also depends on `FFDF84`, which `0x1745E` clears each outer pass.
`0x54662` raises the knowledge rank after that sequence. An alert change alone
does not replace the stored mode. While `0xFFDF8C` marks the entry sequence,
`0x1ADE6` bypasses `0x1A370` and the service block whose handlers call
`0x1B706`; completion enters `0x54662` directly. The `0x1A370` -> `0x561E8` ->
`0x56116` path can still reselect during the normal `0xFFDF8C == 0` controller
service before a transition. During entry, mode zero uses its source
four-phase raw-cadence branch; nonzero modes apply the node reaction delta.
Section 15 has a presenter-only Active Alert comparison for the
rank-zero System 15/node 13 SPU: all six mode-3 phases render as complete
320x224 frames against independent frame `004800`; the best candidate has
full-frame mean RGB error `9.653`, and its bottom band was inspected intact.
That is typed-snapshot evidence, not a live-route match. The live entry's
selected mode/phase at `004800`, the independent VBlank palette service, and
the following `004810`/`004818` sequence remain unresolved.
Analyze and Medic now run from the selected deck slot through the traced Lua
rules. The ROM maps C/A/B to attack/balanced/defense, and the source priority
checks C before A. Medic applies its rating roll to persona condition, capped
at 10, and keeps the program loaded on failure. Degrade now uses the selected
ready slot, applies the shared decker/IC opposed roll, and lowers current-node
security by half the positive net (minimum one), with the ROM floor of two.
Success, blocked, and failure results publish the corresponding slot phase;
the program remains loaded. Deterministic Lua route checks cover all three
outcomes in Debug and Release. Attack (`0x1B734`) and Slow (`0x1B696`) now use
the same selected-ready-slot gate and opposed rolls, with their ROM condition /
reaction updates and result phases. Their IC-defeat path decrements the signed
`FFF0CE` counter, wraps negative values to 15, and awards one Karma byte to
each present party member; the typed world-state adapter carries that counter
through Lua and saves its known/unknown state. Both programs also clear the
signed passive-IC probe counter at `FFF0A3` through the same typed adapter.
Attack entry `0x1B734` calls shared result helper `0x56434`, and Slow entry
`0x1B696` calls `0x56482`; those helpers also serve the field-menu handlers
`0x19E32` and `0x19EA6`. The related ROM helper at `0x564D6` is called by
Degrade entry `0x1B81A`; it is not the program entry.
ROM `0x00C2C` zeros `FFF0CE` during full RAM initialization and preserves it
on the warm-reset short-clear path. Field setup writes `FFF0A3 = 1` at
`0x174FC`; Matrix routes refuse Attack and Slow while `FFF0CE` is unknown.
Field-side consumers of this shared byte and other
program handlers remain open. The Lua session
carries the Analyze timer and signed phase selection; the
presenter uses four individually extracted paired-ring overlays, the bent-arm
pose for the first four service calls, the raised-arm pose from call five, and
the idle pose after the 61st countdown service. Debug/Release traversal QA
checks slot selection, ring bounds and pixels against the independent Section
15 frame `004800`, the raised runner layer, the countdown-zero boundary, and
the idle/result frame corresponding to `004818`. The QA fixture uses a
different node, so this is a layer/geometry comparison rather than a claimed
whole-frame match. Other program handlers, all node/status combinations, full
actor cadence, and Matrix return remain open. Medic's deterministic success
and failure routes pass in Debug/Release; its full-frame result capture is
`matrix-medic-action-result.bmp`. No matching independent Genesis Medic
action frame has been identified, so its capture is runtime integration
evidence, not a visual golden. The Matrix traversal QA also runs Degrade in
the live route: Debug and Release both show Evaded, retain security 3, and
write the complete `matrix-degrade-action-result.bmp`. No independent Genesis
Degrade-use frame has been identified, so this is integration evidence only.
See the
[CyberInfo audit](visual-audits/matrix-cyber-info-run-info.md) and the
[Matrix viewport audit](visual-audits/matrix-traversal-viewport.md).

The ordinary field terminal now routes through Lua for taxi, Vid-Phone, and
Cyberspace. Taxi uses the ROM's seven-region cursor tables and animation, then
loads the confirmed field scene through the existing fade path. Vid-Phone
filters the 13 source contacts, pages the list, and displays each call's
first/repeat text and portrait. Portrait extraction now follows the live
contact pointer table at `0x109682` through each record's `+0x0A` profile
pointer, and the offline importer reads every per-tile delta in the upper
`0x1728` descriptor branch. Six distinct contact portraits match independent
longplay dialogue crops at the source `(16,32)` placement; the odd purple
Gregory portrait is present in its source frame as well. Four contacts point
to the same ROM blank profile at `0x121E20`, and Alesandro and Professor
Jefferson share `0x121F34`. Capt. Quinton and Sharkey have no matching visual
frame in the reviewed longplay. The Vid-Phone visual CTest checks Henry's
portrait pixels and verifies that the image clears when the call returns to
the field menu. No matching independent longplay frame of a Vid-Phone call has been identified,
so the visual hill remains partial. `contacts.json` binds each traced contact
to either a JSON graph and action module or a legacy Lua service function; the
shared terminal resolves these bindings without a contact-ID branch.

Taxi confirmation now starts the Lua cabbie dialogue before the host stages a
destination. ROM `0x20714` checks the service story bit and the already-there
case; `0x2076A` handles the armband and Salish visa gates; `0x208A8` confirms
the fare and applies the insufficient-funds ride; `0x208FE` publishes the
accepted destination and consumes a Salish visa [C]. The Qt taxi QA covers the
already-there response, normal fare deduction, and Downtown arrival, while
`taxi_cabbie_tests` covers the fare comments and gate permutations. The hill
now has an independent Section-41 capture of the Puyallup 40-nuyen quote,
choice labels, `Hop in, chum.` response, and field return. It confirms that
fare branch only; it does not expose the wallet or the other gates. The capture
also shows the cabbie portrait. ROM entry `0x20714` selects profile `0x121F10`;
the offline importer now produces `dialogue.portraits.taxi.cabbie`, and the
Qt speaker uses it. Focused taxi QA selects Puyallup in ROM cursor order,
checks the portrait pixels at `(16,32)`, writes
`field-taxi-fare-offer-actual.bmp`, and declines without charge or travel; it
then completes the paid Downtown route. The fare-page assertion checks the
five ROM-derived text groups, including the forced break after `get` at
`0xFE32C`. The hill remains partial while the
other fare branches lack independent captures. See the [taxi fare prompt
capture audit](visual-audits/taxi-fare-prompt-capture-2026-10-27.md).

Capt. Quinton's full call graph is connected. The ROM menu rooted at `0x1096F4`
reaches Firepower (`0x109750`) and Contacts (`0x109780`); command IDs `0x68`-
`0x6A` dispatch through `0x12CCB4` to handlers `0x22898`, `0x228D0`, and
`0x22908`. His JSON graph owns first/repeat, Hollywood/Weller information,
weapon offers, and the Rigger offer. The typed host applies the 400-nuyen
grenade purchase, 1,000-nuyen HK227-S purchase, and 2,800-nuyen Sharkey
introduction against the live runner wallet, inventory, and contact flags. The
HK phase byte is the Laser Sight and Gas Vent II attachment mask (`0x18`),
separate from the item's 28-round catalog capacity. Debug/Release script,
transaction, and panel-route CTests cover success, insufficient funds, full
inventory, and clean return. Their images render the dialogue panel over
black, so they do not prove cleanup over a live map background.

Alesandro's service flow is a JSON graph reached from the shared first/repeat
call menu. It follows ROM commands `0x6B`-`0x6D` to handlers `0x22934`,
`0x2296E`, and `0x229A8`: level-3/level-5 maglock passkeys cost 10,000/20,000
and store their level in item 20's phase, while Sgt. Macklemann costs 8,000
and sets contact bit 5. JSON owns his three information topics, purchase/offer
choices, and result edges; Lua submits the three typed host transactions. The
direct Lua permutation suite and Debug/Release script/panel CTests cover the
graph branches and render through screen 34. Alesandro's shared portrait
appears in section 23 frame 001770, but no complete matching call sequence was
found in the reviewed longplay.

Max's record at `0x10E400` enters its first/repeat graph before the
Services/Goodbye choice. Commands `0x83`-`0x87` offer a $6,000 corporate badge,
a $1,500 HK227-S, a $2,000 AK-97, and level-4/level-5 passkeys for $30,000/
$42,000. The typed service checks funds and item space; JSON owns the offer,
category, success, insufficient-funds, inventory-full, and return edges. Lua
permutations cover all five transactions and C++ integration checks the live
graph-to-host requests. The panel CTest now captures the call, service,
hardware, category, purchase-result, and Goodbye screens for internal review.
Section 05 frame 001440 shows a Max portrait/text page, not a phone call, so
independent call-screen fidelity remains open.

Professor Jefferson's record at `0x10F0D4` offers a three-page paranormal
lesson, a $1,500 club discount (command `0x88`, FF018F bit 6), and a $2,000
former-student introduction (command `0x89`, FFFC2C bit 6, Gregory contact).
The JSON graph keeps the lesson and both offer flows separate; named Lua
actions request the typed host transactions. The insufficient-funds pointer
at `0x10F12E` targets invalid text bytes at `0x0FED41`, and both low-funds
results remain explicit source boundaries. Lua permutations and C++ host
integration cover first/repeat, lesson, offer, transaction, and result edges.
The panel CTest captures the lesson and club path. Section 23 frame 001770
shows the shared Alesandro/Professor portrait and a generic Services/Goodbye
panel, not the complete phone call.

Mr. Faradouchi's ROM record at `0x10D068` routes to a repeat-call page after
acceptance; on a first call handler `0x22C98` rejects an existing Yakuza tie,
then checks the active runner's Reputation at `$FF018B` against 5 before
offering Mafia membership. Command `0x7D` at `0x22CC8` records contact 7's
history bit 20 and FF018F bit 2 only after Yes. Agira Tetsumi's record at
`0x10D676` mirrors that flow: handler `0x22CE8` rejects a Mafia tie, and
command `0x7F` at `0x22D18` records contact 8's history bit 21 and FF018F bit
1 after Yes. Decline, low
Reputation, and rival-faction rejection keep the history bit clear, so a later
call repeats the first-call branch. JSON contains the pages, Yes/No menus,
typed Reputation conditions, and return-to-caller terminal; Lua registers only
the paired history/discount-flag action. Pure-Lua and Debug/Release native-host
permutation tests cover first/repeat, both faction gates, threshold values,
acceptance, and decline. The reviewed longplay breakdowns have no complete
matching call sequence for either person, so the visual hill remains partial.

Sharkey's ROM contact record at `0x10A7F2`
offers commands `0x6E`-`0x70`: 3,000 nuyen for Salish-Shidhe passage to scene
6, 5,000 for Sinsearach passage to scene 9, or Smart Goggles (item 22) for
3,000. A JSON service graph owns the offer choices, result pages, and typed
travel outcomes. `vid_phone/sharkey.lua` only registers host actions, and
`data/vid_phone_passages.lua` owns the two destination/fare records. The typed
host accepts a generic scene-and-fare request, applies the wallet change, and
schedules the shared scene transition; it does not map those offers through a
C++ action table. The production field-runtime QA selects
Vid-Phone from the ordinary terminal, pays for both passage destinations,
acknowledges each result, and captures both fade/load routes into Scenes 6 and
9. Scene 9's profile spawn `(0x0D80, 0x1100)` projects below the 224-pixel
frame at camera `(0x0A00, 0x0280)`; the host runs ROM `0x08C0`'s 31 camera
passes before `0x12F6`, and the QA requires that runner anchor on the field.
It also exercises the Smart Goggles success/full-inventory branches and
checks clean terminal redraws. Kipp David's record at `0x10AD8A` now has a
JSON service graph for its information pages, offer choices, result branches,
service loops, and call terminal. Lua only registers the purchase actions.
Commands `0x71`-`0x73`: 185,000 nuyen writes the Fairlight Excalibur bytes at
`FFFBBE` (model 4, MPCP 12, Hardening 5, memory 500, storage 1000, load 100,
response 3) and does not set `FF018F`; Rebound and Degrade cost 30,000, pass
the rating-4 storage check at `0x19538`, and store rating 6. The panel QA
captures his service menu, the deck offer, and the disconnect. The pure-Lua
permutation suite covers the Matrix loop, both programs, deck success/failure,
service returns, and unavailable boundaries. Section 33 frame 000690 confirms
the portrait art only; no full Vid-Phone call sequence or physical Kipp-service
route appears in the reviewed longplay. Wilma
Temmenhoff's record at `0x10B71E` follows commands `0x74`/`0x75`: both charge
5,000, accept storage sized as rating 3, and store rating 3 for Rebound and
Degrade. Her JSON service graph owns the information and program pages, both
transaction result branches, and the distinct Services/Goodbye return routes;
Lua only registers the two typed transaction actions. The permutation test
covers both commands, every recognized host result, and both result-menu
destinations. Panel QA captures the offer, download, and sign-off. No matching
independent call frame or physical Wilma-service route is identified, so the
visual hill remains partial. Sgt. Macklemann's record at `0x10BFAC` follows
command `0x76`: 5,000 nuyen sets `FFFC2C` bit 28 for the Hollywood gate and
does not set contact bit 5. His JSON graph owns both information trees, their
Services/Goodbye routes, the Hollywood offer, and its insufficient/success
pages; Lua registers only the badge transaction. The graph permutation suite
covers all menu edges and transaction results, while the typed-service test
checks the charge, gate flag, and unchanged contact bit. Debug/Release script,
service, and panel CTests pass. The panel captures are internal regression
artifacts; no matching independent call frame or physical Macklemann-service
route is identified, so the visual hill remains partial.
Gregory Wilns's record at `0x10C7FA` opens the JSON service graph at `0x10C838`.
It owns the four-page paranormal-creature lesson, both offers, result menus,
and typed transaction branches; Lua registers the two host actions. The
creature lesson returns to Services; command `0x77`
charges 4,000 and sets `FF018F` bit 7 for the 25% Ares shop discount, and
command `0x78` charges 50,000 for item 24 (Power Focus, phase 2) only when the
runner's signed `+0x9B` value is below 2 and an inventory slot is free. The
host passes that saved flag through shop price quotes and confirmations. Debug
and Release script, service, and full-frame panel tests cover the lesson,
Goodbye/Talk Later paths, both purchases, eligibility, insufficient funds,
and full inventory. The graph permutation suite covers every dialogue edge
and recognized action result. These captures are runtime regression evidence; no
independent Genesis call frame or physical route to Gregory's service has been
identified, so the hill remains partial. Its full-screen captures are source/runtime regression
evidence, not an independent Genesis comparison. The QA opens the field
terminal through a controlled entry seam for its transaction tests. A separate
Debug/Release physical-input CTest walks into Scene-0 record 62/event 31, opens
the ordinary four-choice terminal, selects Vid-Phone and Captain Quinton,
uses the source B choice to end the call, then checks the terminal redraw and
the completed 60-frame host return fade. Its full 320x224 contact panel and
returned field frame were inspected. There is no independent longplay call
frame, so the visual hill remains partial. Physical routes to service screens
and independent call-frame matches remain open. Cyberspace checks
deck/MPCP/datajack facts, offers passcodes or System Search, then enters the
existing Matrix session and node-action route. Debug/Release CTests cover these script routes, the phone
pages/calls, the cab selector, and a completed Redmond-to-Downtown map load;
the taxi selector and arrival full-screen dumps are byte-identical across
builds. This hill remains partial: physical input coverage for every contact
service, full Matrix presentation/actions, and the ROM's mode-exit dialogue are
still open.
See
[`field-map-trigger-profile.md`](field-map-trigger-profile.md).

The motel-street collision report is now covered by a dedicated Scene-2 route
from the New Game spawn. It targets the ROM-recorded Stoker frontage, keeps
the C2 doorway event out of the collision check, and logs held wall contact,
diagonal escape, and retreat. Lua now decides the player collision route and
returns C052/C062 animation selectors; C++ provides terrain/route/recovery
services and applies the returned position and selector. The idle reset uses
the selected C052 facing from `+0x2E` even though ROM `0xB442` leaves `+0x36`
unchanged. The Stoker collision-to-dialogue route now continues through the
farewell and a held Down exit. It verifies that Lua retains the `0xB7B8`
`+0xB0` retry bit across the same-scene modal return, then releases it on the
first clear terrain sample through `0xAC5E`; the C2 interaction does not
redispatch during the turn. Debug/Release produce 21 matching full-screen
frames, all reviewed. The Walk / collide hill remains partial: the reproduced
Stoker exit defect is fixed, while general stuck-obstacle paths and independent
Genesis controller evidence remain open. See
[`visual-audits/field-movement-animation.md`](visual-audits/field-movement-animation.md).

Puyallup's Tarislar Garden Apartments route is partial. Scene 1 record
67/event 7 defaults to scene 13. The first-authorized service-21 branch now
uses a JSON graph for the ROM arrival and penthouse pages before the shared
`harlequin.main_conversation` graph. That graph contains the meeting, Michael,
Thon, and Renraku prompts, reads text/options from the catalogs, and branches on
story bit 16. The no-Ito path sets effect bit 7 before the closing update; the
Ito path applies command `0x40`/bit 28, presents the Ito response, then returns
to Lua to run the separate map-information graph. Button menus with source
early-return behavior use explicit `noSelectionNext` edges. Lua applies bit 21
and +2 Karma before the dialogue, then command `0x45`'s effect bits 5/6 after
the arrival page. The route returns to scene 1; its Debug/Release
physical-input QA captures all nine pages and a clean field `actual.bmp`. The
six condition-selected event-7 route records now live in the entity's
`interactions.json`; the raw event-table destination remains scene 13.
The guarded branch now presents the `0xFBCC7` and `0xFBD5A` pages, then a titleless
No Business/More choice: No Business follows `0xFBD94`, while More follows
`0xFBEEC` into the shared Harlequin conversation at `0xF58C0`. Both outcomes
stay on scene 1. Separate Debug/Release physical-input CTests exercise each
choice and save full-screen captures. The repeat
service now uses a JSON graph and JSON conditions for its story/item gates,
greeting at `0xFC111`, questions response at `0xFC222`, and Ito response at
`0xFC757`. A named Lua action applies command `0x40`'s bit-28 effect, then the
graph enters the reusable `harlequin.map_information` JSON conversation.
Its conditions branch on group-item bits 8 and 7; named dialogue actions apply
commands `0x46`/`0x47` and field-effect bits 8, 12, and 14. The same graph
resolves pages and More labels from either building's speaker/menu profile.
Debug/Release script QA checks the no-map, Renraku-only, and paired-map paths,
including both tomb-departure pages. Physical-input QA captures the Tarislar
Renraku path and clean scene-1 `actual.bmp` return. Independent Genesis page
captures remain open. The separate
tomb-offer graph requires bits 30, 21, 16, 28, 26, and 27, plus alternate
result 0; its question is imported from ROM text at `0xFD03D`. The typed
`enter_field_scene` terminal maps to the entity task's stable scene-54 result
after Yes completes all three ROM travel pages; No returns to scene 1. Debug/Release
physical-input QA captures both outcomes and the full-screen scene-1/scene-54
`actual.bmp` files. Signed nonzero FFF0D5 results now have higher-priority Lua
entity routes whose JSON graph maps positive FFF0D5 to ROM-imported package-
delivery text and negative FFF0D5 to package-pickup text. Debug/Release CTests
reach event 7 with ordinary Up input, verify both pages, suppress scene 13,
and capture the clean scene-1 return as `actual.bmp`. The upstream Frosty
conversation and inherited callback reachability remain open. Scene-55's
post-panel cleanup and title return are connected through a direct QA seam;
the native actor trigger and pre-panel presentation are not connected. Source
tracing rules out an `FFE162` carry from its only direct writer into the Tarislar selector.
Scene 54's extracted map is a tall 320x1344 layout, and its profile has no
event table or elevator group. Event 31 stores callback `0xDB26` through the
direct `0xB7B8` path; other route audits map its ordinary field-mode behavior
to the shared terminal. The Ghidra export skips its body, but a direct US-ROM
decode confirms Scene 54's mode-3 path returns at `0xDEE6` before the Matrix
node-selection input loop and selects no new scene. For marker 1
(`FFE186=1`), `0x167AE` schedules before changing scenes, snapshots and clears
`FFE177`, then the first `0x02900`/`0x04388` field-service pass sets it to 6. This
bypasses `0x1DBAA`'s zero-count early return during initial Scene-54 setup.
The Scene-54 profile supplies six Yakuza Weenr records, each with `+0x9E`
bit 6 set. On their qualifying completion path, `0x0AB02` calls `0x16B98`;
ordinary kind 8 returns without a weighted spawn in mode 3, then `0x0AB02`
decrements `FFE177` for that record. The six profile rows therefore match the
initial count. At zero, the service records reset and `FFE12E` is dispatched
and cleared if installed. The only non-clear `FFE12E` stores found in the raw
ROM are the two pending-list branches in `0x043B6`: list `0x1E628` installs
`0x53FFC`, and list `0x1E5C4` installs `0x54286`. Scene 54's marker-1 selector
bypasses its lower `FFE120` list-selection path, so its default actor list does
not itself install either callback. The first field-service pass drains
`FFE120` before marker 1 sets `FFE177=6`: a matching pending list can install
its callback and set the count to 4, after which `0x04388` replaces the count
with 6. Marker-1 selection does not clear an already installed `FFE12E`; at
zero, the six Scene-54 actor completions can dispatch that callback. No
upstream pending-list or callback route into this pass has been established.
The field-exit orchestrator dispatches run type 3 through `0x1EEDC`'s callback
table entry at `0xF3E62` to the `FFE162` writer at `0x1F086`. Its confirmed
target tuples (Scene 1/event 9, Scene 2/event 9, and Scene 3/event 4) exclude
Tarislar's Scene 1/event 7, so that pass cannot make a fresh write from the
accepted target pool. `0x03032` sets `FFFC0E` only for current scenes 6-9 and
44-49; it is called from scene-profile preparation at `0x03074` and `0x03084`.
The three type-3 target scenes (1, 2, and 3) therefore have `FFFC0E=0`. The
outer loop at `0x075E` sends each matched writer result to `0x167AE`; that
selector copies any nonzero `FFE162` into `FFE132`, publishes it as `FFE120`,
clears it at `0x16A62`, and forces Scene 15 in the same transition. A raw
operand sweep found no other direct `FFE162` writer. Tarislar's event-7 route
is not a type-3 target, and no type-3 pointer can carry into a later Tarislar
selection. Prompt command `0x60` reaches a separate ROM code gap at
`0x225D2`, which sets `FFE120` to `0x1E628` behind a 75-nuyen and story gate.
The menu node at `0x111316` presents the entry-fee question and places command
`0x60` in its first choice slot. Adjacent code at `0x225CA` returns that node;
the command-0x60 caller is traced to the Club Penumbra Scene-3/event-0 doorman
path. The extracted actor rows for list `0x1E628` are indexed under Scene 47,
but the pending-list consumer does not establish a transition there. The
separate `0x1E5C4` stores are at `0x2247C` and `0x224EC`; neither has a traced
Tarislar route. Whether a preexisting list pointer or callback can carry into
the Scene-54 service pass remains open. The Scene-54 event-0 path is now traced
through the projected-interaction countdown: after 17 service advances,
`0x163D0` raises `FFDF3E`, the next outer-loop pass changes marker 1 to 2, and
`0x167AE` selects Scene 55 when `FFE162` is clear. The selector queues
`0x20938`, which presents text group `0xFDD1A` with profile `0x121F1C`;
The active Qt runtime now queues a typed arrival continuation and Lua owns
the first prompt with the ROM ten-frame input delay. The first command slot
contains pointer FDD2E; the outer loop clears FFE104 and reloads FFFC04 without
chaining the next record. Scene-55 completion and title return are now
source-traced. Mode 2 advances to mode 3 when `FFF140` is negative and the
source byte at `FF044C` is at most 9. During render-pass effect phase 2,
`0x5D29C` scans the six scene-actor slots only while animation frame `+0x18`
is 3. A live candidate has record bit 0 set, bits 4 and 6 clear, and a
nonzero physical meter at `+0x4C`. If none qualify, the phase timer expires
before `0x5D3D4` queues the special `0x5BAC4` callback through `0x1E2E0`.
The callback opens the single Licourtrix prompt from `0xFDD72` with profile
`0x121EA4`, runs the closing animation and five controller-advanced credits
text panels, then returns through the main title path. Their source-ordered
text blocks begin at `0x5CA92`, `0x5CBFB`, `0x5CF9B`, `0x5CD82`, and `0x5CE80`;
the offline all-assets importer emits their space-preserving rows at
`cinematics/tarislar-tomb-credits.json`, retaining `0x80` row breaks and
`0xFF` panel terminators. The named host asset records the source row data,
origin `(64,16)`, and white ink from palette line 3, entry 1. The entry thunks
at `0x5C472` through `0x5C4C2` pass name-table addresses `0x2110`, `0x2110`,
`0x2110`, `0x2310`, and `0x2290` with `D1=$E000`; the final two start four and
three rows lower. Lua owns the five-page order and yields typed credit-page
requests to the host presenter. Debug and Release CTest
`shadowrun_ghidra_conversion_qt_field_tarislar_tomb_credits_presenter_qa`
captures each panel and writes `actual.bmp` for the first. After the final
controller input, the page remains visible through the six `0x53126` service
iterations (18 VBlanks), fades over 17 two-VBlank `0x131C` passes, and stays
black for the outer loop's repeated 34-VBlank wait. The typed host then
restarts the existing attract loop. Debug/Release QA checks all 86 VBlanks
and captures the fade. These are internal captures, not an independent
Genesis visual reference. Lua now retains encounter actors' source slots and
initial combat facts, and its typed six-slot query reads live service flags
and physical meters for the `0x5D29C` eligibility decision. Debug/Release
script-runtime, world-activity, and random-encounter QA pass. Lua validates
the six source addresses as typed actor slots 4..9 (`FF0400` is slot 3). The
field render-pass tail now advances the Scene-55 Lua completion state and
invokes the query during the source frame-3 window. The world-frame callback
projects slot 3's `+0x4C` meter into the `0x1DD94` mode gate and arms the
sequence on the mode-2-to-3 transition. When the phase timer expires with no
eligible actor, the host queues a typed 60-frame continuation for `0x5BAC4`.
This live trigger has not had a full-route CTest or CLI `actual.bmp`; the
existing five-panel QA enters through a direct presenter seam. The typed
continuation now opens the ROM-backed Licourtrix warning and one-response
graph, which ends at an explicit boundary before the D7=0x03E8 DBRA
presentation. The pre-credit audit now traces its initial two-plane scene,
the elapsed-0x190 Plane-B skyline, the elapsed-0x26C 38-column Plane-A reveal,
and palette changes; the offline importer emits inspection images for those
stages, and `--all-assets` now extracts three named background/overlay layers
plus the 19 source-timed sprite lists. The timed presentation, pre-panel fade,
and typed runtime presenter remain unimplemented (see the
[pre-credit source audit](tarislar-tomb-prelude-source-audit.md)). The complete
`FFF11E` visual/effect record is not reproduced. The host synchronizes
`FFE186` between its coordinator and world-frame state at field-frame and
render-pass boundaries. The hill remains partial.
The event-0 projected interaction has no
scene-54/mode-3 map write. See the
[`Tarislar route audit`](visual-audits/puyallup-tarislar-apartments.md).

Current map-dialogue work: scene-2 record 85/event 4 now opens the Lua entry
page and, after A, resumes its numeric scene-13 route. The separate entry
portrait `0x121EBC` matches frame 001200; profile `0x121E80` matches
Gunderson in frame 001500. The initial root, information pages, topic selector,
dynamic question selector, Johnson answer/follow-up, conditional Vigore and
Errascoe branches, the FFFC27 bit-3 alternate Vigore response/follow-up,
alternate root, and Decker contact purchase/failure route are bound through
shared Lua content. ROM action `0x4D` deducts 1,000 nuyen
through the host wallet service; exact-1,000 and 999-nuyen cases, repeat
contact selection, and full-screen confirmation/failure panels have Debug and
Release QA. The field-entry route continues on the same runtime instance
through the production Lua interaction and wallet service: it buys the
contact from a QA-seeded 1,000 nuyen, then rejects the repeat purchase at zero.
The separate dialogue-runtime test verifies that 999 remains unchanged.
Twenty-three full-screen dialogue states and fourteen field-entry captures
match between Debug and Release; the latter were inspected as complete
320x224 screens. Question screens were compared against longplay frames
002100, 002200, 002550, and 002850. The independent longplay has no alternate
Vigore branch or Decker purchase sequence, so those new screens have host
regression evidence only. The field entry route verifies ordinary collision,
modal dismissal, transition, and a clean scene-13 redraw.
This hill remains partial: Debug/Release QA now walks into scene-13 event 30
from the clear adjoining lane with ordinary Up input and opens Gunderson's Lua
menu. Debug/Release ordinary Down QA now enters the Scene-13 south exit after
record 85/event 4 and restores Scene 2 at the saved entry probe facing out of
the door. Both builds save matching full 320x224 captures and route.tsv. The
FFFC27 bit-3 alternate Vigore response and follow-up are covered by the
dialogue-runtime CTest and matching Debug/Release captures; independent Genesis
visuals remain open. Work/quest branches, remaining actor population,
audio/timing, and independent Decker-branch visuals also remain open. See
[`Jump House entry and Gunderson audit`](visual-audits/jump-house-entry-and-gunderson.md).

Scene 14's lower event-0 rectangles (records 7 and 8) now use the shared Lua
source-activity return route for the null event table. Each Debug/Release QA
case resolves Scene-1 record 73's event-0 row through Lua, seeds its saved door
probe from that collision record, then enters one south exit with ordinary
Down. It asserts return to Scene 1 at the saved door probe facing outward.
Each writes full-screen approach, contact, and return BMPs plus route.tsv.
Scene-14 event 30 now checks
FFFC1E bit 16 before its normal Johnson root; the set branch clears context 3
through the Lua run registry and shows the ROM's blank-profile, no-choice
booth-vacated page. Debug/Release ordinary-Up QA writes matching full-screen
captures under `qt_runtime/{debug,release}/field-wanderer-vacated-entry-qa/`.
The separate `0x20AFC` conditional-credit gate selects the context-3
conditional-opening root, covered by Lua route QA. The work-offer outcome path
remains open; see the
[`Wanderer audit`](visual-audits/wanderer-scene14.md).

Scene 15 record 12/event 30 (`0x1D5D3E`) now has a map entity and Lua
link to the shared unnamed old-man profile. The source branch presents
five text segments without choices and contains no nuyen or reputation
check; it runs after the map loads. Debug/Release ordinary-Up QA starts
outside the rectangle, captures contact/dialogue/the full 60-frame return,
and returns to Scene 15. All eight 320x224 BMPs match across builds by
SHA-256 and were inspected at full size. Scene 15 is reused by multiple
routes, so this event does not identify a specific named site. No
independent Genesis contact frame is identified.

Scene 12 record 11/event 30 now routes by FFFC1E bit 10: clear presents the
bodyguard page; set opens Caleb Brightmore through the shared Lua Johnson
system. Debug/Release physical-input tests enter with ordinary Up through both
FFFC1E bit-10 branches: the clear gate shows the bodyguard rejection without
changing visit flags or counters; the set gate checks Caleb's two
first-meeting pages, three-choice root, FFFC27 contact/visit bits, and +2
source-byte triplet. Both write full 320x224 captures including `actual.bmp`.
Caleb's information/contact outcomes and context-4 run settlement remain open,
and no independent Genesis encounter frame is available. See the
[`Icarus Descending audit`](visual-audits/icarus-descending-scene12.md).

The user-reported intro, field, HUD, menu, inventory, and sound defects are
tracked as independent goals in
[`visual-defect-goals.md`](visual-defect-goals.md). Reported status and
independently verified status stay separate.

The pre-fix reproduction and the complete post-fix 6,782-frame Release
intro-to-field review are recorded in
[the historical baseline](visual-audits/full-intro-capture-2026-10-06.md)
and [the corrected full capture](visual-audits/full-intro-capture-2026-10-06-pan-fix.md).
The Gator Shaman pan fix and nine-question review are in
[gator-shaman-head-pan.md](visual-audits/gator-shaman-head-pan.md).

The Attributes/Skills grid has two distinct Karma pages: the in-game command
menu is view-only, and Hotel Options opens the separate spending child. Their
routes, input rules, lower detail line, and redraw behavior are reviewed
separately in
[karma-status-and-hotel-upgrade.md](visual-audits/karma-status-and-hotel-upgrade.md).
The [profile and upgrade-boundary audit](visual-audits/attributes-skills-profile-permutation-audit-2026-10-23.md)
checks every displayed stat/skill for the three Joshua presets and ten named
recruited templates, all race ceilings, class gates, upgrade costs, and the full
controller-byte matrix. Its Qt frames match the traced record mapping, but the
supplied Genesis video has no matching Attributes/Skills screen.
The Hotel-specific [profile and preview audit](visual-audits/hotel-upgrade-profile-permutation-audit-2026-10-23.md)
physically opens the spending child for all thirteen raw runner profiles,
checks all 260 cursor/profile states, the thirty race-stat ceilings, zero and
rank-12 boundaries, and an insufficient/decline/confirm transaction for every
profile. Its 398 Debug/Release full-screen frames match by SHA-256. The later
field integration permutation also covers the captured four-frame A hold:
the ROM grid edge at `0xD31E` calls the prompt at `0xD426`; held A frames do not
create a second edge in `0x195C`, so a distinct A after release is required to
purchase. Debug/Release pass with 492 matching profile BMPs and a matching
manifest. Keep the visual hill partial: the user-supplied
`decompilation/ghidra_conversion/build/research/au/e8_karma_spend/` shows Human/Shaman, Body 2, Karma 20, the default-Yes
prompt, and the held A sequence, but lacks ROM/emulator identity and does not
show a later release-and-press or purchase result. Its frame-9040 Body 2 to 3,
cost-3 display matches the ROM Human/Shaman preset at `0x72B1A` and the native
profile permutation; the host fixture has Karma 0 while the capture has 20, so
this is a value-level check. The Mage Only/insufficient
message shares pixels with the selected description in the host capture.

Magic has two user-type-specific presentations per the user's visual
reference: magic users get the yellow advanced breakdown, while non-magic
users get the Defenses screen with two bars. ROM `0xFBA6` is traced, and the
Debug/Release fallback QA now uses the unmodified Samurai class-2 record;
the independent longplay supports the Shaman spell page; a one-second search of
sections 08-41 found no matching Magic Defense frame. Treat that as bounded
negative evidence, not proof of absence. Class/race presets select different attributes, spell tables, and limits; displayed Magic and Atk/Def strips are calculated from the relevant runner, party, posture, focus, and action fields. See the [Magic profile and input permutation audit](visual-audits/magic-spell-action-permutation-audit-2026-10-23.md) and the [Magic split audit](visual-audits/magic-user-screen-split.md).

Matrix node-entry status text is separated and positioned by the ROM draw
addresses: node title at `$C418`, file class at `$C39A`, alert band at `$C51E`,
and difficulty at `$C52C`. Lua publishes the three status values independently;
the Debug route capture is
`decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/matrix-node-action-route-qa/node-entry-screen.bmp`.
The original CPU fixture was not comparable to the longplay action frame, and
the node seed was being dropped by the Lua route. That handoff now selects
System 2 node 1 from the ROM table; Debug/Release QA writes
`system-2-dead-projects-datastore.bmp` with the matching Datastore / Dead
Projects / Green-3 state and its three ROM actions. I inspected the complete
320x224 output beside Section 21 frame `004620`; panel placement, rows, green
grid, and labels align. The selected row marker also uses the ROM's right-arrow
font tile `$5FF`, written as `$85FF` by `0xC376`, with its security palette
line extracted from `0xE2D52`; the prior ASCII caret is gone. Debug and Release
captures are pixel-identical, and both focused CTests pass. This is a
compressed-video visual cross-check, not a raw-pixel golden. The hill remains
partial. See the
[Matrix node status layout audit](visual-audits/matrix-node-status-layout.md).

Scene 1/event 1 has entity-owned Lua tasks for the Tarislar City Inn's indexed
stay/Tabatha service and generic field prompt. The room, Leave, and Tabatha
branches use a JSON graph; successful payment yields the typed `hotel.options`
service outcome. Debug/Release ordinary-input QA reaches record 68 and checks
the service gate, Crash/Leave/Tabatha menu, exact 30-nuyen paid boundary,
29-nuyen insufficient boundary, decline, same-map returns, and the paid Hotel
status/options sequence. Four more physical routes cover the two Caleb/Errascoe
topic orders crossed with Recruit/Thanks, through attack, Renraku aftermath,
Tabatha re-entry, David's 30-frame Holopix, and same-map return. The 101 route
BMPs match across Debug/Release; selected full 320x224 states were inspected.
ROM text records were regenerated into `assets/dialogue/script-text.json`.
Command `0x14` checks that its Notebook/referral stores are resolved before the
runner-byte service can mutate Karma, avoiding partial acceptance state on an
incomplete save. The post-attack commands switch profiles and prompt roots;
they do not establish the attack's combat outcome. There is no independent
Genesis capture for the room-payment or post-attack variants. The supplied
longplay confirms the intro, manager's three-choice menu, Tabatha's blank
vid-phone pages, and her apartment prompt. The combat outcome and independent
payment visuals remain open, so the hill remains partial.
See the [City Inn route audit](visual-audits/tarislar-city-inn-scene1-event1.md).

| Cluster | Hill | ROM | Status | Defending test | Dump |
|---|---|---|---|---|---|
| later | Matrix Storage screen | `0x18D1A` / `0x18F5E` / `0x18DE0` / `0x18F2A` | defended | `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` | Debug/Release route and Lua tests cover root, reverse file list, Start return, erase prompt/default, No cycling, Yes deletion, remaining files, and empty message; six full 320x224 captures per build match by SHA-256. This defends the traced Storage behavior and layout; no independent Genesis Storage frame was identified. See [Matrix Storage audit](visual-audits/matrix-storage-screen.md) |
| 0 Process | Visual dump CLI | — | partial | Archived legacy-native CLI; no active Ghidra CTest for this general capability | No root-project-wide dump is retained; the Release Qt executable dumps only supported scenes |
| 0 Process | Leftover-clear (startup + field menu) | `0x59092` / `0x59172` / `0x10F50` | defended | `shadowrun_leftover_clear` | leftover-clear/*.bmp |
| 1 Boot | Pre-title logos | `0x5D746` after reset `0x0236` | partial | `shadowrun_boot_sequence`; `shadowrun_ghidra_conversion_qt_boot_fasa_qa` | `decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/boot-fasa-qa/actual.bmp` plus shock/strobe checkpoints (source-batch lightning lifetime, ROM-cadenced seven-step figure fade, FASA 34-VBlank fades and 121-VBlank hold, palette-line-2 logo, clean title handoff); whole pre-title hill remains partial |
| 2 Title | Natural no-input attract-loop replay | `0x1A9E` / `0x075E` / `0x131C` / `0x53126` / `0x5D746` | defended | `shadowrun_ghidra_conversion_qt_title_idle_restart_qa` | Debug/Release `qt_runtime/title-idle-restart-qa/actual.bmp` proves a clean replay boot; QA checks the 3,601-update title countdown, 18-VBlank caller service, second 17-pass/34-VBlank fade, recurring source shock request, and pixel-identical fresh/replay boot and title. No matching no-input replay appears in the supplied independent longplay; command-0x14 resource and duration remain open. Nine-question audit: [title-idle-attract-loop.md](visual-audits/title-idle-attract-loop.md) |
| 2 Title | Layered title loop | `0x1A9E` / `0x1E4E` / `0x1C6A` / `0x26D0` / `0x2634` / `0x1872` / `0x0F3C` | partial | `shadowrun_ghidra_conversion`; `shadowrun_ghidra_conversion_qt_startup_qa`; `shadowrun_ghidra_conversion_qt_boot_fasa_qa` | `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/startup-qa/actual.bmp`; initial title hold dumps `qt_runtime/boot-fasa-qa/pre-title-black-frame-1.bmp`, `pre-title-black-frame-2.bmp`, `title-black-start.bmp`, `title-black-frame-1.bmp`, `title-black-frame-45.bmp`, `title-black-frame-90.bmp`, and `title-logo-first-column.bmp`. Debug/Release boot QA verifies the two post-FASA handoff frames and all 90 title-delay updates are full-screen black, then checks the first logo column on update 91; full Release sequence shows frames 540-650 black and 651 first visible mark. Five prompt phase captures, scroll offsets, flame retirement, delayed circuit arrival, and title audio are covered. The full 1,680-frame Section 01 prompt scan finds a 17/5 initial and 34 steady 27/5 visible/blank sample cadence, consistent with the ROM schedule at 30 fps; see the [title prompt blink audit](visual-audits/title-prompt-blink.md). Debug/Release full-screen QA logs all 19 0x5D requests followed by the 0x00 theme request; process loopback verifies resource 93 and resource 0 playback. All 134 title/startup whole-screen captures match between Debug and Release and were reviewed. The independent section-01 reveal sequence supports the order. Circuit color/texture, full-plane wrap, exact Start input phase, and other title fidelity remain open. See [boot black interval audit](visual-audits/boot-title-black-interval.md), [arrival audit](visual-audits/title-circuit-arrival.md), [title audio audit](visual-audits/title-logo-audio.md), and [title circuit audit](visual-audits/title-circuit-palette.md) |
| 2 Title | Start edge fade and Start Options entry | `0x1A9E` / `0x131C` / `0x1342` / `0x075E` / `0x53126` / `0x59092` | partial | `shadowrun_ghidra_conversion_qt_startup_qa` | `qt_runtime/startup-qa/title-exit-fade-start.bmp`, all 17 `title-exit-fade-pass-*.bmp` frames, `title-mode-entry-vblank-01..18`, and `title-exit-destination-start-options.bmp`; Debug/Release QA checks Start-only exit, 34 fade VBlanks, 17 palette passes, 18 black service VBlanks, default New Game selection, and clean full-frame replacement. Independent section 01 frames 001637-001643 and 001670-001674 support fade/black/options order; exact input phase remains open. Shell-to-text sequence is defended separately; its exact 60-Hz duration is not measurable from the clip. Eight-question audit: [title-start-transition.md](visual-audits/title-start-transition.md) |
| 2 Title | Start Options shell-to-text reveal | `0x59092` / `0x10B5A` / `0x10918` / `0x10952` / `0x1095E` / `0x0C3E6` / `0x0C0DA` / `0x0C376` / `0x590EA` | defended | `shadowrun_ghidra_conversion_qt_startup_qa` | Debug/Release full-screen QA asserts the clean shell phase, New Game selection and cursor on reveal, and a clean shell/menu redraw after empty Load return. CLI `actual.bmp`: `decompilation/ghidra_conversion/build/conversion/qt/debug/visual-audits/start-options-reveal/shell/` and `/menu/` (Release equivalents match); both are 320x224. Independent section 01 frames 001670-001671 show shell only, then 001672-001674 show heading, choices, and selector. ROM `0x1095E` supplies an explicit wait before menu tile transfers; the 30-fps capture cannot establish exact 60-Hz display duration. Eight-question audit: [title-start-transition.md](visual-audits/title-start-transition.md) |
| 3 Startup | Start Options | `0x59092` / `0x10B5A` | partial | `test_startup_behavior_and_rendering` | `--scene start-options` (CRT frame + layout `'['/'\\'` title bar, triangle cursor) |
| 3 Startup | Save/Load empty slot | `0x59172` | partial | leftover-clear + startup test | `--scene save-load-empty` |
| 3 Startup | Archetype menu and starter profile transfer | `0x59092` / `0x59248` / `0x59308` / `0x42B0` | partial | `shadowrun_ghidra_conversion_qt_startup_integration_qa`; `shadowrun_ghidra_conversion_qt_startup_qa` | Debug/Release QA inspects the three source-backed portrait/label states and integration copies each full 0x100-byte record from 0x7291A. All 256 controller-byte masks are exercised from each row: Up-before-Down, direction-before-confirm, wrapped preview refresh, A/C/B confirmation, and ignored Start/unused bits. Independent section 02 frame 000060 supports Gator/Shaman portrait, labels, cursor, and shell; no matching Samurai/Decker selection frames are supplied, so the hill remains partial. See the [profile and input audit](visual-audits/archetype-profile-permutation-audit-2026-10-23.md) |
| 3 Startup | Intro stages | `0x59C5E` / `0x5AF62` / `0x59D7C` / `0x5AA96` / `0x5AD1A` | partial | `shadowrun_ghidra_startup_integration`; `shadowrun_ghidra_conversion_qt_opening_wilderness_pan_qa`; `shadowrun_ghidra_conversion_qt_opening_trivid_layer_qa`; `shadowrun_ghidra_conversion_qt_opening_death_layer_qa` | Separate Qt conversion reaches field handoff and emits Seattle, wilderness, tri-vid, airport, and cab-map layers. The 6,782-frame Release route after the terminal death-layer correction was reviewed from boot through 120 stable field frames; all 18 contact sheets and full-size death/handoff frames were inspected. Debug/Release QA covers the Gator Shaman pan, cockpit priority, and left actor-plate clear. Independent section 02 frames 000480/000510/000570/000630 support the head/body relation; 000840/000860/000880/000900/000930/000940 support cockpit order; 000730-000780 support the death-pose clear. OPEN-02/03/04/05/06 are resolved for their reported defects. OPEN-01 is resolved for the reported late start: visible normal-key Release process-loopback matches stage 1 at opening entry, silence through the stage-2 interval, and stage 3 at its entry; the exact paired 0x11 ROM/Z80 trace is silent, and FF 16 stage-boundary stops are wired. The 90-second ROM probe found no within-stage restart. The supplied Genesis capture does not isolate stage-2 silence or exact VBlank timing. Terminal request 0x2E is a verified silent/no-op slot; OPEN-07 is closed as a non-defect, and TAXI-01 missing top/bottom edges are resolved; map-plate extent remains open. See the [taxi outline audit](visual-audits/taxi-window-outline.md), [death-layer audit](visual-audits/michael-death-layer-clear.md), [cockpit audit](visual-audits/cockpit-trivid-layer-priority.md), [full intro capture](visual-audits/full-intro-capture-2026-10-06-open03-clear.md), [propeller audit](visual-audits/airport-rotor-and-aircraft-tail.md), and [Gator Shaman pan audit](visual-audits/gator-shaman-head-pan.md) |
| 3 Startup | Wilderness ambush palette hit and layer cleanup | 0x5A0B2 / 0x5A124 / 0x5A16E / 0x5A1BE / 0x5A252 / 0x5A2D0 / 0x5A328 / 0x5A338 / 0x5A350 | partial | shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa; shadowrun_ghidra_conversion_qt_opening_death_layer_qa | Debug/Release named full-screen checkpoints cover paired magic phases and stop, shared head/body palette fade, 9-frame cadence, six 8-frame timed effects, stage-12 black head submission/removal, center $0826 clear, and right $03B8 map clear. The final Release run contains 6,782 consecutive frames from title to map handoff and first field dialogue; all are represented in 106 chronological whole-screen sheets. All 535 pixel-changing baseline/fix frames were reviewed in full-screen comparisons, and 48 phase timing changes in separate sheets. Independent section 02 frames 000640-000648 show the center blast without a square; frames 000630-000720 support hit/fade/blast/smoke/clear order. OPEN-09 and OPEN-10 are resolved; source-audio timing, exact VBlank alignment, and other intro defects remain open. See the head-flash audit (visual-audits/indian-attacker-head-flash.md), spell-stop audit (visual-audits/ambush-magic-effect-stop.md), and explosion-layer audit (visual-audits/ambush-explosion-border.md) |
| 3 Startup | Wilderness death-pose layer clear | `0x59F8E` / `0x5B32C` / `0x5AA96` | partial | `shadowrun_ghidra_conversion_qt_title_intro_overlay_qa`; `shadowrun_ghidra_conversion_qt_opening_death_layer_qa` | `qt_runtime/opening-death-layer-qa/actual.bmp` plus full-screen checkpoints from frames 688 through tri-vid entry 1382. US-ROM `0x59FBE` fills the 17x16 left actor plate at `$0600` with `$07FF` before the 20x13 death image; the typed callback clears the persistent left layers. Debug/Release tests check callback order, transition pixels, the full remaining pose lifetime, and scene replacement. Independent section 02 frames 000730-000780 support the cleared pose and later projectile motion. Reported visual defect OPEN-03 is resolved; command `0x2E` selects a verified silent slot, closing OPEN-07 as a non-defect. See [the nine-question audit](visual-audits/michael-death-layer-clear.md) |
| 3 Startup | Ambush source choreography | `0x5A1EA` / `0x5A252` / `0x5A59C` / `0x5A8DE` | partial | `shadowrun_native_port` | `visual-regression/intro-hill/intro-1240.bmp`, `intro-1552.bmp`, `intro-2512.bmp` |
| 3 Startup | Airport helicopter overlays | `0x5B08E` / `0x5B100` / `0x5B1B6` | partial | `shadowrun_native_port`; `shadowrun_ghidra_conversion_qt_opening_rotor_qa` | `visual-regression/intro-hill/intro-3000.bmp`, `intro-3400.bmp`; Qt Debug/Release `qt_runtime/opening-rotor-qa/actual.bmp` and `rotor-phase-0.bmp` through `rotor-phase-6.bmp` (whole `320x224` screens), with six-state cadence, 36-frame repeat, and plane-body/rotor-strip pixel checks. Independent section 02 frames 001065/001068/001071/001074/001077/001080 show the animated rear propeller. The 4,982-frame full-intro run was reviewed at 10 fps; all 219 full-screen frames in the active rotor window were inspected consecutively. `OPEN-04` is resolved in [the propeller audit](visual-audits/airport-rotor-and-aircraft-tail.md). |
| 3 Startup | Airport landing movement and sprite selection | `0x5AD1A` / `0x5B198` / `0x5B1B6` / `0x5B270` | partial | `shadowrun_ghidra_conversion_qt_title_intro_late_qa`; `shadowrun_ghidra_conversion_qt_opening_landing_qa` | Qt Debug/Release `qt_runtime/opening-landing-qa/actual.bmp` plus 20 full-screen event/threshold checkpoints; verifies the 0x226 lead-in, seven source records and computed-Y alternate-art window. All 21 images are 320x224 and byte-identical across configurations. Independent section 02 frames 001320/001380/001410/001440 support base/base/alternate/base art and the sampled positions; 30-fps capture does not establish exact VBlank phase. See [airport landing audit](visual-audits/airport-landing-descriptor.md). |
| 3 Startup | Cab destination zoom | `0x13726` / `0xD4464` / `0xD4536` | partial | `shadowrun_native_port`; `shadowrun_ghidra_conversion_qt_startup_qa` | Existing `visual-regression/cab-hill/cab-map-0.bmp` through `cab-map-89.bmp`; Qt Release `archive/qa-artifacts/visual-review/taxi-outline-cli/actual.bmp`; full Release cab-map frames 4680-4861 | The eight cursor tiles and five destination zoom phases are drawn and covered by Debug/Release QA. TAXI-01 missing top/bottom edges is resolved against independent section 02 frames; map-plate extent and exact video-to-VBlank alignment remain open, so the full cab-map hill is partial. See [taxi outline audit](visual-audits/taxi-window-outline.md) |
| 4 Field | Walk / collide | `0x075E` / `0x03084` / `0xB442` / `0xB6DE` / `0xBABC` / `0xBBE4` / `0xC052` / `0xC062` / `0x600E` / `0x5FDC` / `0xE7C8` | partial | `shadowrun_native_port;shadowrun_ghidra_field_session_source_assets;shadowrun_gameplay_script_runtime_qa;shadowrun_ghidra_conversion_qt_field_movement_qa;shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa;shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa;shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa;shadowrun_ghidra_conversion_qt_field_scene2_motel_street_collision_qa` | `--field-movement-qa <dir>` writes 55 whole-screen idle, eight-heading, cadence, and release frames. ROM 0xB6DE selects C062 after accepted movement; 0xB442 selects C052 after reset; C0B2 remains a finite behavior transition. Lua now owns player heading slew, collision-route policy, fallback choice, and C052/C062 selector; C++ provides terrain sampling, route facts, map-event recovery, world-position commit, and drawing. The 48-frame Scene-2 motel route covers approach, held wall contact, diagonal escape, south retreat, and idle. Debug/Release route BMPs match by SHA-256 and all were reviewed full-screen; B442 facing uses the selected C052 list from +0x2E while +0x36 remains unchanged. The independent section-03 longplay supports layout but has no controller log. This hill and the user-reported stuck-obstacle defect remain partial/open; exact Genesis movement behavior, broad terrain recovery, movement audio, and remaining physical routes are unverified |
| 4 Field | Field map foreground priority and actor occlusion | `0x03074` / `0x3D80` / `0x481E` | partial | `shadowrun_ghidra_conversion_qt_field_map_composition_qa`; `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`; `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa` | `qt_runtime/field-scene26-event1-entry-qa/actual.bmp` and `qt_runtime/scene26-event1-terminal-qa/actual.bmp` use the production composition path. The importer preserves the ROM-selected high-priority plane pixels as a sparse transparent layer; all 56 map/overlay pairs are checked for matching dimensions and exact source RGB at every opaque overlay pixel. Scene-26 composition QA confirms a real priority pixel covers an actor and a transparent pixel leaves it visible. The 614-frame whole-screen route was reviewed in eight chronological sheets, including the map, HUD, terminal, menu, transitions, and return. User-reported table/shadow depth inversion is corrected in the Qt map renderer. Independent Genesis scene-26 overlap evidence and exact object-bound validation are still missing, so keep partial. See [field map priority audit](visual-audits/field-map-priority-composition.md) |
| 4 Field | Map event route policy | `0xB7B8` / `0x1F5CA` / `0x1F65A` / `0x1FA78` / `0x172CDC` | partial | `shadowrun_ghidra_conversion_qt_field_script_route_qa`; `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`; `shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa`; `shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_entry_gate_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_c0_exit_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa`; `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`; `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa`; `shadowrun_ghidra_conversion_qt_scene26_maglock_descriptor_route_qa`; `shadowrun_ghidra_conversion_qt_field_redmond_vigore_search_qa`; `shadowrun_ghidra_conversion_qt_field_wanderer_south_exit_record_7_qa`; `shadowrun_ghidra_conversion_qt_field_wanderer_south_exit_record_8_qa` | Typed native policy prioritizes Stoker dialogue and named host services, then uses the active map event table. Scene-5 events 2-5 select Lua-owned store IDs and ROM welcome/package text through 0x1FA78. Events 5 and 3 now open source-backed Weapon's World mode-0/result -1 and Merlin's Lore mode-5/result 0 Lua catalogs after their prompts. QA covers both categories, the first weapon offer, the spell progression training gate, the MicroTronics no-deck gate, and returns. Wire-Masters now opens its mode-7/result-1 Lua cyberware selector and completes typed upgrade transactions; MicroTronics deck-present actions remain open. Debug/Release QA writes the four prompt frames under `qt_runtime/field-scene5-store-service-entry-qa/{wire_masters,merlins_lore,microtronics,weapons_world}/actual.bmp` and Weapon's World category/offer/return at `qt_runtime/field-scene5-store-service-entry-qa/weapons_world/{menu-category,first-weapon-offer,field-return}/actual.bmp` plus Merlin's Lore category/progression/return at `qt_runtime/field-scene5-store-service-entry-qa/merlins_lore/{menu-category,spell-training-gate,field-return}/actual.bmp` and MicroTronics store-closed/return frames; these host captures do not establish Genesis visual fidelity. See the [Weapon's World mode-0 shop review](visual-audits/weapons-world-mode0-shop.md) and [Merlin's Lore mode-5 shop review](visual-audits/merlins-lore-mode5-shop.md), and [MicroTronics no-deck gate audit](visual-audits/microtronics-no-deck-gate.md). The paired scene-2 C0 doorway unconditionally selects scene 0x14; its distinct FFFC2C bit-26 scene-entry action is consumed and timed through final scene re-entry. The separate C0/event-0 exit QA covers return to scene 2. Other routes include C1/event 1 -> scene 0x1A resource activity. All 83 scene-2 route frames and 500 scene-26 descriptor-route frames were reviewed; the 529 scene-26 Debug/Release CTest artifacts match by relative path and SHA-256. That route reaches the event-29 elevator, record-117 A2, the second record-110 0x82 maglock, and the D8 event-0x18 Security Terminal. The QA-only live-Joshua passkey verifies both map-counter updates, two 0x4B requests, and ordinary-input traversal to the terminal prompt. It seeds the passkey and begins in scene 26; it does not establish story-driven access or Genesis visual fidelity. Scene-2 event 1 is behaviorally covered from the New Game spawn, but its sustained-east screenshots show the runner covered by the opaque HUD column; exact Genesis comparison is missing. Independent section 03 supports Stoker order; independent Ares references support prompt/shop. Scene-2 event 7 now has a physical Up-input route from the clear lane south of record 97, three Lua dialogue pages from ROM records 0x117193/0x117266/0x117314, the Landmark Map and party-Karma award, one-shot cleanup, and a clean same-scene return. Its QA seeds the E7 prerequisite. The ROM bit-26 writers at 0x22018/0x22072 are traced, but the Harlequin-to-Gunderson lead dialogue and menu route are not connected; exact Rat's Nest Genesis visuals and event audio are unresolved. The site-map overlay identifies record 97 as the strong Rat's Nest match; record 96 remains a separate event-7 rectangle. The 72 selected Debug/Release images match by SHA-256 and were reviewed as whole screens. See [the seven-question event audit](visual-audits/scene2-redmond-vigore-search.md). Other map records, general terrain recovery, reverse routes, event-1 runner clipping, and event audio remain open. See [field map route audit](visual-audits/field-map-script-routing.md), [scene-2 event-1 audit](visual-audits/scene2-event1-scene26-route.md), and [scene-26 Security Terminal audit](visual-audits/security-terminal-physical-entry.md) |
| 4 Field | Event-29 elevator groups, floor selector, and map transition | `0x58D94` / `0x58DE0` / `0x58DB2` / `0x16A8E` / `0x2BD2` / `0x131C` / `0x12F6` / `0x1342` | partial | `shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene25_group_roundtrip_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene24_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene23_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene25_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene28_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene29_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene30_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene31_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene32_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene33_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene34_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene36_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene37_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene38_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene39_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene40_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene41_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene42_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene43_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene34_group_roundtrip_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene40_group_roundtrip_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene43_group_roundtrip_qa`; `shadowrun_ghidra_field_elevator_selector_runtime`; `shadowrun_ghidra_field_session_source_assets` | ROM-derived groups feed `building.resolve_field_location_entry` and `building.resolve_elevator_destination`; Lua selects the entry action, floor, and destination while C++ retains selector timing, fades, and scene loading. Debug and Release script and route CTests cover inclusive groups 21-25, 26-29, 30-34, 35-40, and 41-43. Focused Debug and Release route QA includes physical event-29 entry in scenes 28 and 36 after neutral updates let the source activity timeout expire. Physical group-route chains enter every member scene; endpoints 25, 29, 34, 40, and 43 are verified at indices 4, 3, 4, 5, and 2. Scene-28 and scene-36 approaches wait for the source activity timeout before ordinary Up input. Keep this hill partial; ordinary resource entry, saved-floor restoration, exact CRAM fade, route audio, and independent map visuals remain open. See [field elevator selector audit](visual-audits/field-elevator-selector.md) |
| 4 Field | Projected-interaction fade timer | `0xB7B8` / `0x16396` / `0x095E` | partial | `shadowrun_ghidra_conversion_qt_field_transition_qa` | `--field-transition-qa <dir>` writes initial, fade frames 1/8/16/17, and `actual.bmp`. Debug/Release QA checks even-count palette cadence, signed expiry after 17 frames, full-screen fade including HUD/player, clean black output, and the pending `FFDF3E` request; 0x16396 has no direct sound call. The Qt presentation uses a uniform black overlay for the source palette steps, so it verifies timing and screen replacement but not exact CRAM color arithmetic. The question-by-question visual audit is [field-transition-fade.md](visual-audits/field-transition-fade.md). See the separate world-activity hill for the downstream scene selection and re-entry path |
| 4 Field | World-activity resource scene and Security Terminal floor result | `0x16E30` / `0x167AE` / `0x16B7C` / `0x03074` / `0x2BD2` / `0x2F32` / `0x20EC2` | partial | `shadowrun_ghidra_conversion_qt_field_world_activity_qa`; `shadowrun_ghidra_conversion_qt_security_terminal_qa`; `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa`; `shadowrun_ghidra_conversion_qt_field_fuchi_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene25_group_roundtrip_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene24_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_elevator_scene23_matrix_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene25_matrix_entry_qa`; `shadowrun_ghidra_field_session_source_assets`; `shadowrun_ghidra_world_activity_routine` | `qt_runtime/field-world-activity-qa/actual.bmp` is the selected third-floor result; companion dumps cover initial scene, resource-scene entry/idle frames, prompt, and return. ROM `0x167AE` treats `FFFBF9=2` as table index 2 and selects scene `0x1A`; this is distinct from scene-2 collision event 2, which remains Stoker dialogue. Its resource bound is 3, typed cursor is `+0x14 = 0x18` and `+0x16 = 0`, and deterministic source PRNG callbacks select `FFE12C = 2` / `FFE185 = 3`; the one default corporate guard in Scene 26 consumes a source PRNG advance during actor initialization before the resource selector. Typed scene preparation loads the named map. `0x16E30` snapshots the typed source-scene/runner presentation before preparation; the resource return callback restores it and `0x2F32`/`0x2BD2` rebuild the named source map. Debug and Release route QA follows the normal scene-2 event-1 collision into scene 26, reaches the physical terminal result, returns to Scene 26, then crosses record 126 by normal Up input to open the Lua Main Menu; Down selects Exit and A restores a clean Scene-2 redraw. The adjacent starting cell is test-seeded. The 674 full-screen BMPs were reviewed in eleven chronological sheets and Debug/Release outputs match by SHA-256. The Exit menu stays pixel-identical for 30 neutral frames and requests no new audio. A second Debug/Release physical-input CTest follows Scene 26 record 126 through Cyberspace to Matrix system 24/node 22 and saves full 320x224 menu and deck captures; the settled deck capture was inspected beside independent Section 15 frame 004220. The Scene-28 deck frame was reviewed beside that reference for panel, icon, text, and lower-slot placement; Scene-29 through Scene-34 and Scene-36 through Scene-43 deck BMPs have the same hash in both builds. This is a layout review, not a pixel-equality claim. A third Debug/Release CTest follows gated Fuchi entry into Scene 21, seeds the adjacent cell and crosses record 121 with ordinary Up, then enters system 22/node 13 through Cyberspace. The Fuchi menu and deck captures match between builds. The Scene-25 elevator-group CTest completes the five-floor source-group route back to Scene 22, then seeds its adjacent cell and crosses event-31 record 136 with ordinary Up and enters the ROM-seeded system 22/node 13. A separate Debug/Release CTest continues from Scene 22 to Scene 24 through the elevator selector, seeds its adjacent cell, crosses record 136, and checks the distinct system 22/node 8 target. Another follows the elevator to Scene 23 and seeds its adjacent cell, crosses record 159, and reaches system 22/node 6. A descriptor-checkpoint CTest starts on Scene 25, seeds the cell south of record 124, and uses ordinary Up to enter system 22/node 15. These routes have matching menu and deck captures across builds. Lua covers all target rows 21-43, while physical direct-entry routes cover Scenes 21, 22, 23, 24, 25, 26, 28-34, and 36-43; 2 Lua target rows (Scenes 27 and 35) have no event-31 collision rectangles in the generated ROM profile, so a direct physical trigger path has not been established. Exact resource-menu cursor art and independent facility-menu frames remain open. Modal dismissal now has a 60-frame host fade; QA captures midpoint and completion, checks no stale screen layer, and checks held-A input does not leak into firing. This host duration is not a Genesis timing claim. The dedicated dispatcher QA injects its resolved byte; the joined event-1 route separately carries floor index 0 to the physical terminal result. The eight-question audit is [world-activity-security-terminal-floor.md](visual-audits/world-activity-security-terminal-floor.md). No matching independent Genesis terminal/floor frames were identified; exact scene animation cadence and the persistent event `0x30` audio stop lifetime remain open |
| 4 Field | Scene-2 event 1 through scene-26 Security Terminal result | `0x17314C` / `0x172CDC` / `0x0D57A0` / `0x54222` / `0x167AE` / `0xB7B8` / `0x20EC2` | partial | `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa` | One live runtime follows New Game scene-2 record 93 (C1/event 1), row 1 to scene 0x1A, preserves selected floor index 0, crosses the scene-26 physical map route to D8 event 0x18, and shows the first-floor result after ordinary A. Passkey item, Electronics skill, and PRNG are explicit QA fixtures. Debug/Release each produce 546 full-screen BMPs; all 557 test artifacts match by SHA-256. The 500 scene-26 frames were reviewed in ten chronological sheets; the scene-2 runner/HUD overlap remains open and no exact Genesis route capture is identified. Audio event 0x30 is checked as a request only; its playback and stop lifetime remain unresolved. See [end-to-end route audit](visual-audits/scene26-event1-terminal-route.md) |
| 4 Field | Three-slot party HUD portraits and equipment | `0x3404` / `0x35D4` / `0x35F0` / `0x3602` | partial | `shadowrun_ghidra_field_hud`; `shadowrun_ghidra_conversion_qt_field_hud_qa`; `shadowrun_ghidra_field_party_roster_runtime` | `--field-hud-qa` writes `party-equipped/actual.bmp`, `party-active-winston/actual.bmp`, and `party-ricky-unarmed/actual.bmp`. All fixed slots read their own typed records; portraits use +0x2C palette lines 1/2/3; ordinary and unarmed items use 0x3602 from +0x57/+0x56/+0x9B, and +0x90 selects the unarmed table. Ricky QA starts with a one-round partial magazine, applies typed same-item unequip, verifies the resulting nonzero +0x4E percentage is below 64, and checks that the unarmed `0x36FC` display plan draws `00` while preserving the percentage and clearing the prior icon. Field HUD panels are static between record refreshes; no sound call is traced. Debug/Release graphics, typed HUD, field-action, and Qt QA pass; all 12 HUD full-screen outputs match by SHA-256 and were reviewed. The full 262-image Release player-action set was reviewed in 14 chronological sheets. Independent section 04 frames 000900/000906/000912 and section 06 frames 000001/000015/000030 support Gator face line 1; the eight-question portrait audit is [hud-portrait-palette.md](visual-audits/hud-portrait-palette.md). Recruited-party visual coverage remains open |
| 4 Field | HUD ammo capacity and display | `0x39F2` / `0x38FA` / `0x36FC` / `0x125FA` / `0x121E0` | partial | `shadowrun_ghidra_field_hud`; `shadowrun_ghidra_conversion_qt_field_hud_qa`; `shadowrun_ghidra_field_hud_ammo_permutation`; `shadowrun_ghidra_conversion_qt_field_action_qa`; `shadowrun_ghidra_inventory_action_conversion`; `shadowrun_graphics`; `shadowrun_graphics_catalog`; `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa` | Gator template item 2 starts at `+0x4E=32`; field refresh clamps it to item capacity 12. The source sweep compares all 12 weapon capacities and all 31 two-tile counter rows to raw ROM, and covers 46,080 firearm updates, inactive/unarmed states, all known non-ammo item codes, SMG three-round service, and grenade phase quantities/depletion. A source Human Samurai record with six Frag Grenades renders the item and `06` HUD counter in `party-grenade-equipped/actual.bmp`; its Debug/Release hashes match. Independent section 04 frame 000900 supports an active Streetline `05` counter and empty companion rows. Inventory-action QA proves item code 3 capacity 11 transfers `4/11 -> 37% -> 4/11`; the Qt HUD shows `00` for an unequipped record while preserving its partial-magazine percentage. The reported 32/37/4 transfer still lacks a matching independent capture, so the hill remains partial. See [HUD ammo/capacity audit](visual-audits/hud-ammo-capacity-refresh.md) and [counter permutation audit](visual-audits/field-hud-ammo-counter-permutation-audit-2026-10-23.md). The profile combat-bar QA now exhausts all 12 old-weapon capacities, all legal old-round counts, and all 12 target capacities for each of the 13 ROM race/class records (29,952 transitions); it also exercises target codes 1..12 through switch, unequip, re-equip, HUD glyph, and Atk/Def bars. A full scan of independent sections 06/07 (2,700/3,600 frames) found Armor Clothing and Stim Patch/Medkit First Aid changes but no weapon switch/reload or post-switch Atk/Def sequence, so the hill remains partial. See [weapon profile permutation audit](visual-audits/field-weapon-selection-profile-permutation-audit-2026-10-23.md). |
| 5 Dialog | Field electromagnetic-lock / Security Terminal prompts | mode-2 event table `0x172CDC` -> `0x20E48` / `0x20E7A` / `0x20E92` / `0x20EC2`; callbacks `0x21616` / `0x21692`; shared prompt `0x1F3C4` | partial | `shadowrun_ghidra_conversion_qt_field_interaction_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_passkey_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_insufficient_passkey_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_no_skill_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_alert_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa`; `shadowrun_ghidra_conversion_qt_security_terminal_qa`; `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa`; `shadowrun_ghidra_field_interaction_runtime`; `shadowrun_ghidra_audio_playback` | Ordinary scene-20 routes reach record 117 (`0x1D9724`) and cover Walk Away, no-item grenade, passkey quality 2 success, quality 1 insufficiency, Samurai no-skill Electronics, and Decker positive-skill Electronics. The no-kit Decker partial result changes current selection 2->1 without sound/map mutation; a fresh physical contact reloads 2 at ROM `0xB994`. QA-seeded item `0x15` plus a source-seeded roll then reaches Electronics success and requests `0x4B`; `0x2BD2` changes map cell 483 from `0x82` to `0x81` after A dismisses the result. The test kit is not gameplay acquisition. All seven active scene-20 route directories contain 352 Debug/Release BMPs with 82 distinct images across the set. The six routes before the alert contain 302 BMPs and 81 distinct images; the alert route contains 50 BMPs and 37 distinct frames within that route. The alert timeout reaches the entity-target selection boundary, but its actor/effect remains unresolved. Exact independent maglock visuals, item acquisition, other passkey qualities, grenade consumption/security, and the Security Terminal world-activity result remain open. Passkey sound `0x4B` resolves to resource 075 (2.2167 s); alert cue `0x30` has unresolved stop lifetime. Hill remains partial |
| 5 Dialog | Shared NPC dialogue panel projection | `0x211E8` / `0x2401C..0x2410C` / `0x21226` / `0x1F65A` / `0x1F246` / `0x1F3C4` / `0x1F5A8` / `0xC168` / `0x10984` / `0x10918` / `0xF4AA0..0xF4B2C` / `0x121FA0` / `0x122420` | partial | `shadowrun_ghidra_dialogue_panel`, `shadowrun_ghidra_field_dialogue_runtime`, `shadowrun_ghidra_conversion_qt_field_dialogue_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_arrival_manager_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_room_paid_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_room_insufficient_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_debt_followup_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa`, `shadowrun_ghidra_conversion_qt_field_dialogue_debt_payment_insufficient_qa`, `shadowrun_gameplay_script_runtime_qa` | `--field-dialogue-dump manager-package <bmp>`, `debt <bmp>`, `jump-house <bmp>`, `arrival <bmp>`, `arrival-manager <bmp>`, `room-paid <bmp>`, `room-insufficient <bmp>`, `debt-followup <bmp>`, `referral-followup <bmp>`, `debt-payment-insufficient <bmp>`; Debug/Release CTests write named `actual.bmp` images under `qt_runtime/field-dialogue-qa/`. Shared frame, source 27-count/26-visible text wrapping, direct A/B/C markers, continuation and stale-control clearing are checked across source records and independent section 03 examples. The typed mode-2/event-2 tuple selects the Stoker arrival record; command 0xE9 uses the bounded sampler and values 0..79 open the two-option F4AB4 manager record. Follow-up commands 0x30/0x31 set FFFC30 bit 1 and FFFC27 bit 0, respectively; Debug/Release QA checks the debt prompt, Jump House referral, updated Mr. Johnsons Notebook row, and the newly unlocked Tips entry. Independent section 03 frames 000600-000602 show the debt prompt, and 000690/000750/000780/000840 show the referral. The A-payment low-funds route writes dialogue, Notebook, and Tips `actual.bmp` checkpoints; the exact A-to-referral transition is not in the capture. Room command 0x06 charges exactly 20 nuyen and selects the towel message; 19 nuyen is unchanged and selects low-creds feedback. Dismissing the paid message enters Hotel status/options/checkout; the 20-nuyen and insufficient-funds boundaries have Debug/Release route coverage. The 0x53AB6 indexed trigger gate and three-choice F4AFC root are now source-traced and represented by the JSON graph. Restored-save wallet/story facts, the 80..99 effect at 0x1E1FC, source RNG seed, FFF0AF binding, physical repeat-entry movement, other speakers, an independent paid-handoff frame, unresolved audio/Hotel behavior remain open. Exact independent Genesis frames for the two-option manager and room-payment results are not identified, so the hill stays partial |
| 4 Field | Council Island Hospital Options and exit confirmation | `0x1FA02` / `0xC928` / `0xCA2E` / `0xCA16` / `0xCE7C` | partial | `shadowrun_ghidra_conversion_qt_field_hospital_options_qa`; `shadowrun_ghidra_conversion_qt_field_hospital_leave_confirmation_qa`; `shadowrun_ghidra_conversion_qt_field_hospital_medical_attention_qa`; `shadowrun_ghidra_conversion_qt_field_hospital_physical_entry_qa`; `shadowrun_ghidra_field_dialogue_runtime`; `shadowrun_gameplay_script_runtime_qa` | Debug/Release full-screen QA writes `qt_runtime/field-hospital-qa/options/actual.bmp`, `options/runner-action-return.bmp`, `qt_runtime/field-hospital-qa/leave-confirmation/actual.bmp`, and `qt_runtime/field-hospital-qa/medical-attention/actual.bmp`; physical-entry QA writes full-frame captures for spawn, doorway contact, the nurse prompt, Hospital Options, both No-default and Yes-selected leave prompts, the Start no-op, the No return to Options, the Yes handoff, and the completed field return, plus `route.tsv`. All captures are complete 320x224 frames. Debug/Release leave-confirmation and final field-return BMPs match by SHA-256 and were inspected at original resolution. Scene-4 mode/event 4/4 opens the service prompt on the field map; Services enters the three-row menu, Cyberware uses the typed Cyberware view, and Exit opens Yes/No with No selected. Start is ignored; No returns to Hospital Options; Yes restores the exact saved collision-probe lead point, faces 180 degrees from the entry heading, and completes the 131-frame same-map return. The physical QA measured entry `(27346,14428)` and return `(27346,14778)` in source coordinates. B calls the Lua `field.mode_action` host service, dispatches 0x391A, and resumes the same Hospital Options row; Start remains in the wait loop. Medical Attention (0xCA2E) uses a Lua-owned, ROM-priced four-tier recovery task with wallet/party route tests. No independent Genesis hospital frame was identified, so the hill remains partial. See [hospital options audit](visual-audits/council-island-hospital-options.md) |
| 6 Menu | Hotel status, options, and checkout after paid Stoker room | 0xCF38 / 0xCFFE / 0xEF08 / 0xC376 / 0xDA6A / 0xE660 | partial | shadowrun_ghidra_hotel_options_menu; shadowrun_gameplay_script_runtime_qa; shadowrun_ghidra_conversion_qt_field_hotel_qa; room-paid dialogue QA; shadowrun_graphics_test | `--field-hotel-qa <dir>` follows ordinary New Game Up/A/B input from scene-2 record 91 through the manager, room payment, Hotel status/options, Karma spending, and checkout. It writes 414 consecutive full-screen frames from initial spawn through the post-checkout Start open/close sequence, `physical-route.tsv`, and 64 named interaction checkpoints (478 BMPs total per configuration). The return fades the checkout page to black, holds black from frame 0235 through frame 0317, reveals the field beginning at frame 0318 and fully by frame 0321, then opens the command overview and returns cleanly. It verifies the returned display-service gate is clear and pixel-checks 27 black command-menu load frames. Lua supplies and the typed native route validates the Hotel menu choice IDs and ROM-derived text. ROM input/cursor, wallet mutation (20 to 0), status service, default-No and checked-out confirmation, full-screen clearing and return are checked. Status/options and checkout advance E660 once per wait-loop VBlank; checkout leaves its text and selection fixed while the lower bezel animates. Hotel status/options use palette line three; checkout uses line two. The separate Karma child is defended in its own row. Hotel RNG seed and independent Hotel captures remain open; see the [nine-question route audit](visual-audits/stoker-room-to-hotel-karma-route.md) |
| 6 Menu | In-game Start-menu Karma status page (Attributes/Skills) | 0x10D30 / 0x11740 / 0xD01C / 0xD27C / 0xD2DA / 0xD302 / 0xD33A / 0xD4A0 / 0xD5DC / 0xD82A | partial | shadowrun_ghidra_attributes_skills_menu_runtime; shadowrun_ghidra_cyberware_detail_conversion; shadowrun_ghidra_conversion_qt_field_attributes_skills_qa; shadowrun_ghidra_conversion_qt_field_profile_attributes_skills_permutation_qa; shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa | `--field-attributes-skills-dump <dir>` writes the view-only `actual.bmp`, one full 320x224 image for each of the 20 selected cells, idle frame 30, populated/empty cyberware details, and A/B/Start returns. ROM 0xD01C sets FFF116=-1; row focus does not open the page, the `0x4000` selected label/value uses palette line 2, and A/B/Start leave without mutation. Typed D5DC presentation includes both Essence values and conditional Body/Quickness/Strength cyberware modifiers. Tests verify old-row clearing, all selected cells, clean full-screen return, and source-specific detail mapping. The new profile permutation checks all 13 ROM-derived race/class headers and 260 numeric cells on the Qt view-only screen; the Debug/Release sets of 13 BMPs have identical hashes. Hotel spending remains a separate D024 route; pixel QA compares identical runner fixtures. The command A action requests event 0x24 and the child page stays static for 30 frames. No matching independent Genesis capture is present, so this defends conversion regressions but does not prove Genesis visual fidelity |
| 6 Menu | Hotel Karma upgrade/spending page | 0xCF38 / 0xC14C8 / 0xD024 / 0xD33A / 0xD3EA / 0xD53C / 0xD4A0 / 0xD82A | partial | shadowrun_ghidra_conversion_qt_field_hotel_qa; shadowrun_ghidra_attributes_skills_menu_runtime; shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa | `--field-hotel-qa <dir>` writes `karma-spend/actual.bmp`, frame-30, Yes/No prompt, applied-upgrade, and return-options frames. Hotel option 0 enters at 0xD024 and opens this spending page with next-value/cost details; it shares the Attributes/Skills grid but is a different screen and route from the in-game Start-menu Karma status page. Typed Body/Karma state, cost, confirm/decline, parent E660 suspension, silent child input, full-screen replacement, clean Hotel return, and pixel distinction from the Start-menu status page are checked. No matching independent Genesis capture is present, so this defends conversion regressions but does not prove Genesis visual fidelity |
| 6 Menu | Command overview pixels and return | `0x10D30` / `0x10F50` / `0x110BC` / `0x114A0` / `0x11592` / `0x2BD2` | partial (same-source composite, not a golden) | `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_field_command_overview_bridge`; `shadowrun_ghidra_conversion_qt_field_command_qa`; `shadowrun_ghidra_conversion_qt_field_attributes_skills_qa`; `shadowrun_ghidra_conversion_qt_field_hotel_qa` | `--field-command-dump <bmp>` plus `-return-field.bmp`; the Hotel QA also checks the checkout return, a cleared Start gate, 27 reported black frames at `checkout-start-black/frame-01..27.bmp`, the revealed command menu at `checkout-start-command-overview/actual.bmp`, and a clean Start return at `checkout-start-return/actual.bmp`. The standalone command QA checks 30 static frames and the ordinary `0x2BD2` refresh against the pre-menu field. The seven root labels remain Lua-bound. Lua route now owns Command/Posture state and input; Attributes/Skills view-only is also Lua-routed; other children remain explicit C++ legacy adapters. Embedded-runtime route CTest and Qt command/Attributes/posture QA cover navigation and clean return. The dated visual audit is in [command overview audit](visual-audits/command-overview-root-lua-2026-10-09.md). Independent Genesis command/posture frames and route-specific audio output remain open |
| 6 Menu | Posture focus, five states, and attack/defense bars | `0x119CE` / `0x11ABE` / `0x11AE4` / `0x12CEE` / `0x12EAE` | partial | `shadowrun_ghidra_conversion_qt_field_posture_qa`; `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`; `shadowrun_ghidra_conversion_qt_field_spell_combat_bar_qa`; `shadowrun_ghidra_equipment_combat_bar`; `shadowrun_ghidra_equipment_skill_merge_permutation`; `shadowrun_ghidra_audio_playback` | `qt_runtime/field-posture-qa.bmp` plus `field-posture-qa-frames/` and `qt_runtime/field-spell-combat-bar-qa/actual.bmp`; Debug/Release QA tests Up/Down on pages 2..6, a 30-frame static hold, all five postures and clamps, both combat strips, sound-event cadence, Down+horizontal priority, and Start return. The combat conversion covers 13 source runner profiles at five postures and all 260 legal spell/force/posture combinations; core upper-range expectations cover Defense 22/23/24/25 against all five race ceilings; the Qt profile-screen CTest walks all 13 source records through posture 0..4, checks 65 full frames against expected values and named bar assets, verifies both clamps and clean controller returns, then drives 13 real weapon-Equip actions and checks each refreshed bar formula/pixels plus stable field returns; it checks Attack 24/index 23, Troll Defense 25/index 24, and source-maximum Orc Shaman/Bear Totem Mana Storm after physical selection of Forces 1..4 and Full Offense; D1F5E Power labels Meager/Minor/Very Low/Low and live +0x9D Force match; D4EA0 +0x25 Damage row and 0x10660 residual-driven None/bar presentation are checked alongside formula-derived Attacks 11..14/rows 10..13 against named assets; writes four Force-panel frames and four completed-command frames; the 104 base BMP captures, three original upper-range frames, and seven added Force-sweep frames match Debug/Release hashes; the separate action-skill CTest checks all 256 action bytes, every four-skill rank tuple 0..12 plus boundary rank 13, and 15 merge boundaries (147,517,440 cases); the Gator frame checks Mana Zap, Damage None, Power Very Low, Atk 6/bar 5 and Def 5/bar 4. Down silently restores each command frame pixel-identically; one Start from posture exits the full menu and returns to the exact field frame. Independent section-06 frames 000015/000020 show Gator Mana Zap Force 3/Neutral in Set Force & Posture; frame 002550 shows Power Very Low, Damage None, and nearest bar rows. These compressed captures corroborate layout only, not Mana Storm or Force 1/2/4; exact Genesis pixels and route-specific output loopback remain open. The reviews are [posture-menu-focus-and-bars.md](visual-audits/posture-menu-focus-and-bars.md) and [equipment combat-bar profile audit](visual-audits/equipment-combat-bar-profile-permutation-audit-2026-10-23.md); the hill remains partial while other callers, field combat/AI consumers, and route transitions remain open |
| 6 Menu | Inventory action row/equipment/discard/automatic-use/Trade | `0x11740` / `0x118C6` / `0x12148` / `0x12168` / `0x121C0` / `0x121E0` / `0x12634` / `0x1275C` / `0x12836` / `0x129FE` | partial (same-source composite, not a golden) | `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_conversion_qt_field_action_qa`; `shadowrun_ghidra_conversion_qt_field_automatic_use_qa`; `shadowrun_ghidra_conversion_qt_field_inventory_trade_qa`; `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa` | The Lua script-menu runtime has an item-action path registered through `field_init.lua`; Qt First Aid QA exercises a separate C++ FieldSceneRuntime controller/helper. Their First Aid state is not unified: Qt currently reads active-runner skills[8] and commits target state directly. Typed party records, active slots, item capacity, shared RNG, and screen drawing remain host services. The route CTest covers weapon equip/capacity conversion, equipped-item transfer and cleanup, default-No discard, B/C-confirmed First Aid target choices and effects plus return; full-screen QA confirms A. Debug/Release also pass the five focused full-frame suites for First Aid, automatic-use, Trade, Clips, and command return. Their 49 BMP captures match across configurations; complete First Aid, Auto-Use, Trade, and Clips screens were inspected, including child-to-parent redraws. The earlier field-action image suite covers 55 screen states for weapon capacity, reload, discard, armor, and gear. The profile combat-bar CTest now physically selects all seven armor rows for all 13 ROM profiles (91 cases), checks source clear/equip/repeat routes and unchanged Atk/Def pixels, and writes 539 armor BMPs per Debug/Release run with matching hashes; section 06 frame 002560 supports selected-armor placement but not the transition. A fresh New Game has only Joshua active, and normal recruitment/save restoration does not yet activate a second Qt Trade recipient. Exact equip/unequip, ammo-transfer, Trade, and automatic-use Genesis frames remain unavailable; Trade visual status and route-specific output loopback remain open. See [the nine-question inventory review](visual-audits/item-selector-and-unequip-review.md). The item-detail number renderer at `0x12A8C` now resolves 40 named values from `0x6AC46`; `0x334C` uploads their glyphs from `0x6A5BE` to VRAM `$DAC0`. Debug/Release selects 43 occupied values across all 13 source profiles, while independent section 06/07 frames show Joshua Medkit uses 06/05/04/03/02/01 across First Aid, target, result, and Inventory screens; this remains one character sequence. The inventory hill remains partial because the wider equip and ammo-transfer transitions lack matching Genesis frames; see [the item detail-value phase audit](visual-audits/field-item-detail-value-phase-audit-2026-10-23.md). The same profile CTest now checks all 12 old weapon capacities, every legal old round count, and all 12 target capacities across the 13 ROM race/class records (29,952 transitions). It also drives actual source starter weapons through target selection, equip, same-cell unequip/re-equip, Start return, ammo counter pixels, and code-dependent Atk/Def bars; 117 Debug/Release full-frame hashes match. Section 06 frame 002560 supports inventory placement only, not these weapon transitions, so the hill remains partial; see [weapon profile permutation audit](visual-audits/field-weapon-selection-profile-permutation-audit-2026-10-23.md). |
| 6 Menu | First Aid target/effect/result | `0x12298` / `0x1235C` / `0x123FA` / `0x12402` / `0x1253E` | partial | `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_conversion_qt_field_first_aid_qa`; `shadowrun_ghidra_first_aid_conversion`; `shadowrun_ghidra_field_party_roster_runtime` | `--field-first-aid-dump <output-dir>` writes `actual.bmp` and target/idle/CANCEL/cancel-return/medkit-result/Inventory-return checkpoints; Qt FieldSceneRuntime currently owns a parallel C++ First Aid controller/effect route; `menu/screens/item_action.lua` contains the corresponding Lua route. The Qt route passes active-runner skills[8] and live target state to the typed helper. Unifying them under one Lua-owned state remains open. Debug and Release captures match and the complete states were inspected. The C++ helper now restricts treatment to item codes 0x10..0x12 (Medkit, Stim Patch, Trauma Patch); 0x13 follows auto-use. Focused Debug/Release unit CTests pass; Debug/Release physical Qt QA drives all 13 raw-ROM profiles through Medkit success/full-target guard plus Stim Patch and Trauma Patch success/full-side guard routes. It checks profile Body/Biotech, dice or zero-dice paths, meters/status/continuation/phase, source result text, inventory detail, HUD, and clean return. It writes 143 profile BMPs (11 per profile) with matching names and hashes; the 13-row Medkit manifest and 52-row patch manifest also match across configurations. Source patch cell/count is retained where present (18 route rows); otherwise QA seeds a patch at the selected Medkit cell with phase 2; see [First Aid patch permutation audit](visual-audits/first-aid-patch-profile-permutation-audit-2026-10-23.md). Independent section-06 frame 002563 supports selected Stim Patch layout at `uses 04`; the Gator source template starts at 06, so that count is a different live state. Troll Samurai uses legal Body 11, physical 8, and difficulty 2 in the Medkit route. The helper matrix independently covers 436,320 treatment cases and 1,313 Medkit full-target guards; phase 2-to-1-to-clear remains covered. ROM `0x123FA` redraws the selected target's 4x4 panel at `C928`; its small portrait crosses the posture strip in both Qt and Genesis, closing `INVENTORY-02` as a non-defect. Longplay frames 002610/002630, 002611/002640, and 002615/002699 show target, result, and return; host RNG seed, Qt speaker-output capture, same-archetype portrait comparison, and recruited-runner presentation remain open. See [the visual audit](visual-analysis/qt-visual-audit.md#applied-review-first-aid-target-result-and-return) |
| 6 Menu | Clips selector, reload, trade, distribution, and result wait | `0x11B58` / `0x11B98` / `0x11CCA` / `0x11D06` / `0x11DD2` / `0x11E7E` / `0x11F7A` / `0x11FDE` / `0x12062` / `0x1209A` / `0x120E4` / `0x127B0` / `0x127FC` / `0x11C9C` / `0x11928` | partial (internal composite; independent Genesis capture unavailable) | `shadowrun_ghidra_conversion_qt_field_clips_qa`; `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_field_clips_menu_runtime`; `shadowrun_ghidra_field_menu_text_catalog` | `--field-clips-dump <output-dir>` writes 25 named full-screen states plus `actual.bmp` and `trade-actual.bmp` aliases (27 BMPs). Lua owns selector, reload prompt, Trade, distribution, ammo mutation decisions, and message wait; C++ supplies typed party/ammo/capacity services and mirrors screen state for Qt. Debug/Release QA covers all selector positions, Cancel/Function Aborted, message acknowledgement, all Trade focus states, recipient cycling/wrap and transfer, default-No/Yes reload, and exact parent restoration. Route QA captures no-recipient, zero-reserve, and signed-negative-reserve aborts; route tests also check byte-wrapped distribution total and quotient writes to all three records. All 27 Debug frames were reviewed; Debug/Release names and SHA-256 hashes match. The supplied longplay has no matching Clips sequence, so visual fidelity, loaded-save field composition, and exact route audio loopback remain open. See [the nine-question content review](visual-audits/clips-lua-content.md) |
| 6 Menu | Ares Weapon Emporium controller/transactions | `0x1FA78` / `0x56AC0` / `0x56DC4` / `0x5794E` / `0x56F4A` / `0x57B8C` / `0x586CA` / `0x130C0` | partial | `shadowrun_ghidra_shop_menu_runtime`; `shadowrun_ghidra_field_mode_text_runtime`; `shadowrun_ghidra_conversion_qt_field_mode_text_qa`; `shadowrun_ghidra_conversion_qt_field_shop_qa`; `shadowrun_ghidra_field_menu_text_catalog`; `shadowrun_ghidra_audio_playback` | The mode-2/event-11 route passes through the generic 30-frame Ares prompt. Typed transactions cover insufficient-funds feedback, default-No cancellation, ordinary inventory purchase, E-coded clips, sale quote/refund/removal, equipped-weapon sale cleanup of +0x56/+0x57/+0x4E with HUD refresh, and protected Totems while retaining the selected group/row. One Lua catalog covers all 50 item records, with twelve firearm templates and seven D4D50 armor templates nested on their items. It binds firearm capacities and Damage/Power profiles; Ares shows its five firearm capacities and `uses` counts for Scatter and Concussion, separate from active-runner HUD rounds. Debug/Release QA writes 32 matching whole-screen `actual.bmp` checkpoints to `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-shop-ares-template-qa/`; all Debug checkpoints were inspected at 320x224. The Stim Patch confirmation was checked against independent longplay frame 5992; no independent firearm/grenade detail frame exists. Shared audio QA decodes event 0x25's resource 037 and verifies default-output start and 2.133 s duration. The hill remains partial because restored-save wallet data, Windows output-loopback capture, and independent Genesis detail visuals remain open |
| 6 Menu | Wire-Masters mode-7 cyberware upgrades | `0x1FA78` / `0x56AC0` / `0x56DC4` / `0x587DA` / `0xD177C` | defended | `shadowrun_ghidra_conversion_qt_field_wire_masters_upgrade_qa`; `shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa`; `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_shop_menu_runtime` | ROM result 1 uploads the eight cyberware icons from `0xCF97C` to VRAM `0x89A0`; Lua owns eight offers and the typed purchase path applies cyberware/Essence/wallet changes. Debug/Release tests cover cursor input, all visible icon slots, confirmation, purchase, rejection cases, and return. Eleven full 320x224 frames match across configurations and were inspected. No independent Genesis Wire-Masters frame is present in the supplied longplay, so exact visual fidelity remains unclaimed. See [the Wire-Masters audit](visual-audits/wire-masters-mode7-cyberware.md) |
| 6 Menu | Cyberware flags/name page | `0x10F50` / `0x11950` / `0x12C36` / `0x11900` / `0xD94` / `0x11928` | partial | `shadowrun_ghidra_conversion_qt_field_cyberware_populated_qa`; `shadowrun_ghidra_conversion_qt_field_cyberware_empty_qa`; `shadowrun_ghidra_cyberware_menu`; `shadowrun_ghidra_gameplay_script_menu_route` | `--field-cyberware-dump <file>` writes full-screen `command-page.bmp`, `actual.bmp`, `held.bmp`, `returned-command-page.bmp`, and `reopened.bmp`; page 4 A opens the typed +0x90 bit list, and Lua owns the wait for a new high-byte edge and return. Empty/nonempty branches are checked, while 30 held frames remain static. Debug/Release QA confirms portrait, inventory, posture, tab, and right HUD preservation plus pixel-identical return/reopen. Independent Genesis Cyberware visual evidence and mutation flows remain open. See `visual-audits/cyberware-shared-panel.md` |
| 6 Menu | Magic fallback, spell panel, and force/posture equipment view | `0xFBA6` / `0x10260` / `0x10276` / `0x102D4` / `0x10016` / `0x10722` / `0xFF5A` / `0x3404` / `0x3602` / `0x1055E` / `0x105E6` / `0x1088C` / `0x11190` | partial | `shadowrun_ghidra_conversion_qt_field_magic_qa`; `shadowrun_ghidra_conversion_qt_field_magic_spell_qa`; `shadowrun_ghidra_conversion_qt_field_magic_defense_profile_qa`; `shadowrun_ghidra_magic_menu_conversion`; `shadowrun_ghidra_field_hud`; `shadowrun_graphics` | `qt_runtime/field-magic-nonmagic-qa/` exercises the unmodified Samurai startup record (class 2, +0x7F=0): parent, clean two-bar Magical Defense view, 30-frame static hold, return, reopen, and return. QA compares all pixels outside the center panel with the parent frame and checks old model/weapon/Atk/Def content is cleared. ROM 0x11900/0x11928 redraw the shared posture/action row, which remains visible. `qt_runtime/field-magic-defense-profiles/` adds 24 live screens for all eight non-caster source profiles with zero, one, or two other caster records; it stores parent, Defense, 30-frame hold, and return captures for each case, plus the Samurai/no-caster `actual.bmp`. Raw ROM records are checked against the individually named assets, and ROM `0x1047E`/`0x55F02`/`0x1055E` independently derive party bonuses, Body/Willpower totals, and bar rows. These are internal integration captures and do not establish synthetic-party recruitment or Genesis pixels. `qt_runtime/field-magic-spell-qa/` uses the Gator Shaman source record (class 1, +0x7F=6): typed spell-table bounds use ROM DBF last-index semantics; selected header and lower icons use all 3x3 glyph tiles; source Success/Drain/Defense rows and force/posture labels replace the command-center panel. A opens the `Set Force & Posture.` subview; typed input changes force/posture; B publishes the spell and refreshes its right-HUD icon from six D4EA0 tiles with the 0x3602 palette. QA checks the four-phase selector at seven-frame intervals, force/posture bounds, silent action loop, right-HUD asset, return/reopen, and no stale description. Independent section 06 frames 000001/000002/000005 support browsing; 000015/000020 show action selection and 000030 shows the equipped HUD icon. No matching independent non-magic Defense frame was found; a one-second candidate scan of sections 08-41 yielded only dialogue/databank false positives. The locator cannot prove absence. Opening event 0x24 maps to `menu-024.flac` (2.484 s metadata); no additional action sound call was traced, and route-level output playback is not measured. Typed spell target/effect handling remains open. The five caster source presets (four races, both caster classes), all 21 table entries and their per-spell force caps, and all 256 controller-byte masks in each Magic loop now have internal QA coverage. The audit traces Success, both Magic Defense strips, the distinct Damage displays, and parent Atk/Def bars to runner stats, active party casters, posture, focuses/phases, totem, spell table, and action metadata. Race-specific attribute ceilings and preset values remain profile-dependent even where the formula has no direct race branch. These additional profiles are ROM-backed internal cases, not independent visual captures. The Magic Defense permutation CTest adds 228,488 ordered party inputs across all 13 raw runner presets, both readiness states, all Focus positions and overlapping Focus pairs, and both Defense strip presentation plans. It checks numeric inputs and row selection; it does not add independent Genesis frames or prove synthetic party combinations are recruitable. A separate field-runtime CTest renders 24 source-profile/caster-count cases through page 5 and the fallback action, checking both numeric strips, named bar pixels, intact HUD, a static hold, record preservation, and return to page 5. Full-screen state screenshots were reviewed; the Qt dumper saves transition checkpoints rather than a continuous clip. See [the nine-question Magic audit](visual-audits/magic-user-screen-split.md) and [the Magic Defense party audit](visual-audits/magic-defense-party-composition-audit-2026-10-23.md) |
| 6 Menu | Pocket Secretary / Notebook / Tips & Clues / Dismiss Runner | `0xC2A6` / `0xC8C8` / `0xC448` / `0x59798` / `0xF236` / `0xF2E2` / `0x1E72C` / `0xF33E` / `0xF312` / `0xF9A4` / `0xE660` | partial | `shadowrun_ghidra_conversion_qt_field_secretary_qa`; `shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa`; `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_pocket_secretary_menu` | `--field-secretary-dump <dir>` covers source row order, Group Items FFFC22 empty/populated branches, Cyberdeck no-deck generic MESSAGE with 30 full-screen E660 wait frames and same-row return, and the installed-deck `0x18150` route through the typed deck screen. Debug/Release parent/deck/return captures match by SHA-256; the deck frame was inspected against independent Section 15 frame `004220`. Notebook state/coordinates/no-run wait includes a 30-frame zero-edge hold and every nonzero controller mask 0x01..0xFF; each edge returns to row 0 and clears stale text. Positive Current Run opens screen 41 from the accepted active offer; Lua builds the body and owns any-edge return, and Qt paints all seven canonical detail types. The fixture rows span Johnson contexts 0-4; Recovery uses initializer 9 and its two-site target pool. The Ghoul frame checks the minimum $10 row at Negotiation 0, bound 1, roll 0. Exact font-cell checks include Matrix Expert's grouped $6,700 from base $6,100, Negotiation 12, bound 121, roll 120, with the Aztechnology conditional system line. The video-matched Courier page uses the Section 03 Ares-to-Jackal route and negotiated $40 to $55. Debug/Release QA saves all seven actual.bmp pages and their row-0 returns, holds the Courier page for 30 frames, and checks all 255 positive-page input masks. These are internal captures; an independent positive-page Genesis frame and acceptance-to-save coverage for every source row/type remain open. Lua registry QA now validates source-row context, initializer, progress, and negotiated-payment invariants across 1,040 row/rank/roll states. Dismiss fixed Abort at y=64, active names from y=80, compact-marker removal, current-runner replacement, source-derived candidate portraits, and stale-panel clearing across entry/return are also covered. Root audio QA checks one page-entry event 0x24 and no sound through four accepted Down moves and wrap; ROM 0xC2A6 makes no navigation sound request. See [the root audio audit](visual-audits/pocket-secretary-navigation-audio.md). Notebook F236 C now calls 0x391A, requests event 0x25, and resumes the same selected row; Debug/Release route QA checks event and full-screen continuity. Event 0x25 maps to resource 037 (2.134 seconds including release tail); shared audio QA verifies decoded playback, output state, and duration, but route loopback remains open. Tips checks all 96 named strings, three LSB-first flags, clamped page selection, source text/counters, C mode action, A/B/Start returns, and clearing parent buttons/selector before child draw. Stoker command 0x30 now sets FFFC30 bit 1 and exposes the next clue; command 0x31 sets FFFC27 bit 0 and reveals the Mr. Johnsons row. F236/F312/C448/Tips advance E660 once per VBlank; Qt draws its persistent random 1x2 effect into twelve lower-bezel positions. Dismiss Runner entry and return preserve existing E660 cells, with route QA checking cell/frame continuity. Full-screen review of 30 consecutive wait frames found each pixel delta confined to one ROM destination pair; text, cursor, controls, and portrait stayed fixed. Debug/Release hashes match for all 128 BMP captures. The five-row root IDs, title/separator/label keys, and Notebook option keys now load from `pocket_secretary.lua`; the ROM cursor position is tested at x=72, to the left of text at x=88. All 128 full-screen captures were reviewed; Debug/Release hashes match. The fixed selector-to-art correction and per-VBlank effect are in [the lower-bezel audit](visual-audits/pocket-secretary-lower-bezel-animation.md). See [the Lua content review](visual-audits/pocket-secretary-lua-content.md). Other story/restored-save flags, other child handlers, and independent Genesis Notebook/Tips frames remain open; see [Current Run no-run input audit](visual-audits/pocket-secretary-current-run-no-run-input-audit-2026-10-23.md) |
| 6 Menu | Notebook: Mr. Johnsons child page | `0xF416` / `0xF404` / `0xF312` / `0xC38C` / `0x391A` / `0xE660` | partial | `shadowrun_ghidra_conversion_qt_field_secretary_qa`; `shadowrun_ghidra_pocket_secretary_menu`; `shadowrun_ghidra_field_menu_text_catalog` | Full-screen Debug/Release dumps cover the FFFC27-filtered five-name list, A/B detail text, C event 0x25, each detail return, cursor wraps, and Start return. Every frame of the 30-VBlank detail hold is saved; the text/selection region stays fixed while E660 changes lower-bezel pixels. `notebook.mr-johnson.*` text comes from ROM in the named asset catalog. All-five QA state FFFC27=0x1F is diagnostic; only bit 0 has a verified story writer, and no matching independent Genesis frame has been found. See [the page audit](visual-audits/notebook-mr-johnsons-page.md) |
| 6 Menu | Notebook: Contacts records | `0xF840` / `0xF5F4` / `0xF312` / `0xC66AC` / `0xC66E0` / `0xC661D` / `0xC662B` | partial | `shadowrun_ghidra_conversion_qt_field_secretary_qa`; `shadowrun_ghidra_field_menu_text_catalog` | ROM pointer tables provide all 13 names and descriptions. Lua owns `FFFC2C` filtering, page construction, cursor, A/B detail wait, C action, and Start return. Debug/Release full-frame captures cover both pages, all child return clearing, and exactly eight records without a paging row. Ten-record fixture mask `0x1C7F` is diagnostic. The Contacts flag writers and independent Genesis frames remain open |
| 6 Menu | Notebook: Shadowrunners records | `0xF6DC` / `0xF5F4` / `0xF312` / `0xC665C` / `0xC6684` / `0xC661D` / `0xC662B` | partial | `shadowrun_ghidra_conversion_qt_field_secretary_qa`; `shadowrun_ghidra_field_menu_text_catalog` | ROM pointer tables provide all 10 names and descriptions. Lua owns `FFFC2A` filtering, page construction, cursor, A/B detail wait, C action, and Start return. Debug/Release full-frame captures cover both pages and clean list/detail/parent replacement using diagnostic mask `0x03FF`. Story writers and independent Genesis frames remain open |
| later | Matrix | `0x1817E` / `0x18450` / `0x18776` / `0x26A00` / `0x54662` / `0x54B34` / `0x54BD2` / `0x54C5E` / `0x55058` / `0x5559E` | partial | `shadowrun_ghidra_gameplay_script_menu_route`; `shadowrun_ghidra_matrix_lua_rules`; `shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa`; `shadowrun_ghidra_conversion_qt_field_matrix_traversal_entry_qa` | `matrix-map-route-qa/actual.bmp`, `qt_runtime/field-matrix-traversal-entry-qa/{matrix-analyze-action-phase-3,matrix-analyze-action-complete,matrix-medic-action-result,matrix-degrade-action-result,matrix-model0-green4-system-access,matrix-pull-plug-prompt,matrix-pull-plug-fade-start,matrix-pull-plug-cross-full,matrix-pull-plug-cross-swept,matrix-pull-plug-center-dot,matrix-pull-plug-center-sparkle,matrix-pull-plug-center-dot-return,matrix-pull-plug-transition-black,matrix-pull-plug-handoff-black,matrix-pull-plug-field-return-input-held,matrix-pull-plug-field-return}.bmp`, `matrix-node-action-route-qa/actual.bmp`, `matrix-node-action-route-qa/system-alert-message.bmp`, `matrix-node-action-route-qa/system-overload-fade-start.bmp`, `matrix-node-action-route-qa/system-overload-cross-full.bmp`, `matrix-node-action-route-qa/system-overload-message.bmp`, `transfer-search-first-dot.bmp`, `transfer-download-cancelled.bmp`, `transfer-download-complete.bmp`, `transfer-no-result.bmp`, `transfer-no-storage.bmp`, `transfer-objective-preview.bmp`, `transfer-objective-complete.bmp`, `transfer-upload-complete.bmp`, `turn-off-node-message.bmp`, `erase-objective-message.bmp`, `erase-file-message.bmp`, `action-failed-message.bmp`, `cancel-alert-message.bmp`, `lockout-message.bmp`, `shutdown-countdown-{3,2,1,0}.bmp`, `shutdown-logout-fade-start.bmp`, and `shutdown-logout-cross-full.bmp`, `field-matrix-cyber-info-qa/{cyberdeck,pull-plug}.bmp` cover host route states. Transfer Data uses the full 70-name ROM table, 20-frame dot cadence plus 9-frame pause, Int/Computer/Sensor check, Yes/No prompt, typed deck write, and objective upload/download branches. The 0x55058 handler reads FFFC34/38 from the actual Notebook tip words, FFFC1E from world progress, FFFC22 group items, FFFC27 Mr. Johnsons bits, and optional wallet facts through Lua's typed world-state transaction; unrestored facts remain absent. Systems 19 class 0 and 23 class 12 are supported only through their confirmed FFFC27 bit-3 branches; paths needing FFFC26, FFFBFA/FB, or extra FFFC22 bits stay gated. Prompt results return with Transfer Data still selected; sequential-frame QA verifies a message fully replaces the prior Yes/No panel. Debug/Release route and Lua-rule tests pass. The field System Search QA follows the source 11 permutation rounds and 20-VBlank jack-in into the Cyberdeck at 0x181F8. ROM 0x181F8 uploads D78AA through 0x177CC, then 0x182A6 calls 0x17708 to write the D8316 footer stream at Plane-B $EB00. The source-composed footer already contains the silver portrait at (16,184). The port had incorrectly painted an unrelated field-menu runner preview over it; that overlay is removed. Cyberdeck QA now requires all pixels in the 32x32 portrait cell to match the corresponding source-composed background region. The full 320x224 Debug/Release Cyberdeck and Pull Plug captures use independent Section 15 frames 004220 and 005400 and were reopened beside both longplay references; hashes match between builds. Medic rating and hardware I/O are fixture choices because their values are not visible. Video compression prevents raw-pixel golden comparison. These captures do not defend the complete Cyberdeck hill; Matrix traversal and animated palette phase are also still open. Matrix captures were reviewed as full frames, including six map labels at separate ROM rows. The accepted type-6 run state is projected from and committed to Lua run_registry state, including its ROM FFFBF0 target-system id; entry at 0xE110 activates it only in the assigned system, and host save data preserves it. No live producer for this type-6 contract is wired yet, and security/maglock changes are not yet consumed by doors/guards or saved. The 0x54A8E random alert now runs on every Lua node-view rebuild, reads field alarm for initial status, and presents the passive/active ROM notice and sounds with source waits; overload follows the five-step 10-frame fade and 54-frame transition cadence before its notice, waits 30 frames plus input, and exits. Crash System, overload, and Pull Plug use one Lua-owned 64-VBlank logout clock: five two-VBlank fade steps, sound 0x29 on VBlank 10, then 54 VBlanks of line sweep, center dot, sparkle, dot, and black tail. Full-frame Pull Plug QA captures the prompt, fade, both cross bounds, center dot, sparkle, returning dot, black tail, VBlank-63 hold, and field return at VBlank 64; it verifies the held confirm stays blocked and the host adds no second 60-frame fade. Debug/Release route and field QA pass. The native phase masks match independent Section 15 frames 005420/005427/005430/005435/005440/005445 at thresholded white-pixel IoU 0.998/0.992/1.000/0.957/1.000/1.000. Remaining node actions, live Matrix actor/IC cadence, source scene graphics, and other field-resume/death outcomes remain open. Independent section-15 frames 004800 and 006600 show the runner, faceted node, and perspective grid. Field jack-in opens the source Cyberdeck; the flat 14x10 topology map remains separate. A partial static node-action viewport now consumes typed Lua state and named runner, model, HUD, footer, and world-effect assets. Focused Debug/Release QA and Lua submenu checks pass. The old frame-2050 replay is withdrawn as Matrix route evidence; it paused at Debug Pick System. The System 2 SPU traversal-entry CTest now follows the 61-VBlank setup and 239-VBlank compact entry, checks the typed SPU Green-3/Data Routing/Ice Suppressed/Medic-1 state, compares eleven model bounds with independent Section 21 frames 004730 through 004830, and checks the wide phase-four runner in the full 320x224 capture. The same Debug/Release QA routes System 1/node 0 to the Green-4 System Access model and compares its settled green bounds and silhouette with Section 15 frame 004440; the different active program means this is a model-crop check, not a full-frame golden. The CyberInfo CTest separately checks the extracted green HUD-frame markers on a settled Orange-4 route. Debug and Release both pass, and the full-size captures were opened beside the longplay frames. These checks establish one calibrated entry route and the corrected HUD/runner layers; they are not pixel-identical full-frame goldens. Other model profiles, other program effects, full actor/render cadence, and other field-return/death paths remain open. See [Transfer Data audit](visual-audits/matrix-transfer-data.md), [Matrix random-alert audit](visual-audits/matrix-random-alerts.md), and [Matrix traversal viewport audit](visual-audits/matrix-traversal-viewport.md). |
| later | Combat empty-magazine player action and C runner selection | `0x391A` / `0xA8EA` / `0xAAD2` / `0xACAE` / `0xABFC` / `0x14226` / `0x36FC` | partial | `shadowrun_ghidra_runner_switch`; `shadowrun_ghidra_entity_animation_player_action`; `shadowrun_ghidra_conversion_qt_field_player_action_qa` | `--field-player-action-dump <dir>` writes 262 full-screen images: 177 action/menu/runner frames (the existing 171 route frames plus six item-2 fire checkpoints) and 85 first-weapon frames (before/shot, 52 delay frames, 30 idle-hold frames, final hold). With no other eligible runner, normal C input keeps Joshua active and requests event 0x25. A separate QA route installs source-template Ricky in slot 1, selects him with ordinary C input, verifies the source selection marker moves, then reloads his active item 1 with ordinary held A. Another ordinary input route equips item 2 through Start/Right/A/Start, reloads its own capacity 12 from the shared reserve, waits out the reload delay, then fires it: rounds go 12->11, reserve stays at 1, event 0x1C plays, and regular selector 0 shows frame 0 at shot, frames 1/2, completion on render update 3, then retirement before render update 4. Item 2 presence, its initially empty magazine, and Ricky's roster installation are QA setup; story recruitment is not implemented. Debug/Release outputs match by SHA-256; all 262 BMPs match. The 103 delay and 30 return frames are pixel-identical, and the full menu/runner route was reviewed for stale or overdrawn controls. The first-weapon route's 85-image firing sequence shows primary movement/terrain retirement as well as Lua finite-list animation; linked actor hit/status/damage remains open because field actor records are not imported. No independent Genesis reload/C-switch/firing capture or route audio loopback exists. Lua now requests combat only from held A with a valid firearm and ready source delay; the live route repeats after the 51-frame gate whether stationary or moving. The C042 pose returns to neutral after expiry. Debug full-screen QA covers both movement states; Release verification and independent Genesis firing frames remain open. Source target/fade subcalls and complete hit/damage remain open |
| 4 Field | Player weapon action and spawn-cycle boundary | `0xA8EA` / `0xACAE` / `0xB39A` / `0x38AC` / `0x14D6A` / `0x1514E` / `0x14E0A` / `0x14F50` / `0x15ABA` / `0x5B1E` | partial | `shadowrun_ghidra_conversion_qt_field_player_action_qa`; `shadowrun_ghidra_collision_state`; `shadowrun_ghidra_entity_combat`; `shadowrun_ghidra_entity_transition`; `shadowrun_ghidra_audio_playback` | For a nonnegative equipped item slot, held-A uses the typed weapon route: an empty magazine reloads, otherwise the spawn cycle consumes one loaded round, requests source sound, sets the C042 pose, and retains primary/secondary records in source slots 9..14. Negative-slot D4EA0 spell actions are classified separately and bypass reload/ammo handling. Lua models spell dispatch for four script-runtime test profiles: after delay, projection, action-code, and line-probe gates it records a C082 frame-0 pose and mode-0 delay. The Lua route now resolves spell trials through the typed 0x1033E boundary, reserves a source effect slot, and runs the target render handler from 0x15830 for the four encounter spell profiles; spell actions do not use ammunition. The field host connects the per-probe 0x14D8E terrain/collision callback. Scene 1 sum-2 selects the ROM Lone Star pair, including lonemage_k0a_spell01; full-screen phase QA reviews named C082/effect assets and keeps the HUD unchanged. Code-5 sequential fan-out and the code-2 FFE186 callback gate are implemented in Lua and covered by script QA; independent Genesis effect/audio comparison remains open. Nonnegative NPC item codes report `item_gated` or `item_effect` on that refresh and do not enter `0x14C7C` or `0x55C76`. `0x1514E` and `0x15646` are not called. Each field behavior update advances the primary through four fixed-point probes against the loaded scene collision map; current E7C8 boundary flags drive recovery, terrain contact selects the finite stopped tail, and the next behavior pass observes Lua's prior-render completion marker and retires each finite record. Lua owns primary/secondary finite-list animation on the 0x4618 render heartbeat; the secondary stays at its spawn position. Rendering follows 0x5B1E's stable descending effective-depth order, with source flags 0x10/0x40 forcing depth zero. QA writes 270 full-screen action/menu/runner/equip/reload/return images; Debug and Release each pass collision-state plus field-action/player-action CTests, and all 270 BMPs match SHA-256. Full-screen images were reviewed across the existing 22 route contact sheets plus nine fresh full action QA sheets (four of which show the weapon-fire folder); these include single-tap, held-stationary, held-moving, cooldown, and neutral-return states. The field, HUD, and menu cleanup remain stable. NPC encounter records and B target selection are now present, but the full actor pool, target recovery/child allocation, independently verified attack outcomes, and combat return remain open. Event 0x3C requests omitted silent resource 060 with no audible duration. No matching independent Genesis firing capture was found. The secondary `0xAC4E` effect does not travel; this hill remains partial, not defended. See [weapon spawn/action audit](visual-audits/weapon-spawn-ammo-consumption.md) |
| 4 Field | NPC spell actions and target effects | `0x13FFE` / `0x14D8E` / `0x1033E` / `0x15830` / `0x490A` / `0x4FE2` / `0x5152` / `0x521E` / `0x52BE` / `0x5322` / `0x55B78` | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_spell_effect_qa`; `shadowrun_ghidra_audio_playback` | `qt_runtime/field-spell-effect-qa/actual.bmp` plus all 25 source phase captures. Lua target-owned clocks complete on render heartbeats 12/31/62/31 for action codes 1/2/5/9. Codes 1/2/9 use the delayed result; code 5 scans records at 0x1597E and advances sequential effects to active, unlocked targets with +0xDE==5. Code 2 skips 0x55B78 only when live FFE186 is 3 and caster +0x10 bit 3 is clear. Script QA checks both branches, exact range exclusions, active-runner ID aliasing, fan-out order, and slot ownership. All four ROM event IDs route to decimal resources 50/52/55/53 and decode/play in Debug/Release. The full 320x224 phase set preserves the HUD byte-for-byte and shows a bounded playfield overlay. Independent Genesis cast pixels/audio and the 0x4964 setup callback remain open; hill is partial, not defended. See [spell effect audit](visual-audits/actor-spell-effect.md) |
| later | Qt audio effect playback | `0x530DE` / table-2 entries 16, 30, 37, 50, 52, 53, 55, 74, 75, and 92; menu event `0x24` | partial | `shadowrun_ghidra_audio_playback`; `shadowrun_ghidra_conversion_qt_field_interaction_qa`; `shadowrun_ghidra_conversion_qt_field_shop_qa`; `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` | `audio-playback-verification.md`; Debug/Release playback checks for events 0x16/0x1E/0x24/0x25/0x32/0x34/0x35/0x37/0x4A/0x4B/0x5C, decoded audio, default output, durations, the 60-request one-shot/coalescing and new-sequence replay check, independent longplay correlation, and fresh Release process-loopback capture of one complete boot `0x5C` cue; boot `0x16` is separately routed as FF 12 stop-resource-22; Ares `actual.bmp`; repeated independent longplay waveform matches support resources 024/037. The new combat sound resources lack an independent longplay match. Background music/loop points, other source events, persistent 0x30 stop lifetime and output-loopback capture for other routes remain open |

## Added transition route

| Hill | ROM entry and behavior | Status | Test and dump |
|---|---|---|---|
| 4 Field | Johnson type-4/type-5 target entry at `0x1F046` / `0x1F106`; target tuple dispatch at `0x1EEDC` / `0xF3E56` | partial: Lua run modules show their source-backed entry passages only when the active offer target matches the current map/event; the field host resumes the existing area-event destination after the modal closes. Physical QA reaches Little Chiba with ordinary movement, checks both run passages, and verifies scene 26 loads. Test fixtures seed map 2/event 1 because the standard ROM target pools do not generate this tuple. Run objective completion and independent Genesis visual comparison remain open. | `shadowrun_ghidra_conversion_qt_field_scene26_johnson_run_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene26_package_recovery_entry_qa`; full-screen dialogue and `actual.bmp` captures in Debug/Release `qt_runtime/field-scene26-{johnson-run,package-recovery}-entry-qa/`; [Little Chiba route audit](visual-audits/scene2-event1-scene26-route.md) |
| 6 Menu | Scripted Ares stock catalog; source records `0x1AAC9E` / `0x1AAD8F` / `0x1AADD7`, category dispatch `0x56DC4` | partial: `shop.lua` now supplies the six ordered categories and three stock lists. The typed runtime validates category IDs and eight-slot limits; Lua group order drives selector focus. The live Ares entry route opens the script-backed stock. Selection input, drawing, purchase/sale flow, and wallet/inventory transactions remain native | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_shop_menu_runtime`; `shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa`; 14 full-screen images per Debug/Release, byte-identical; independent references `ares-introduction-5983.png` and `ares-shop-5985.png` |
| 4 Field | Lua runtime and gameplay content; ROM behavior inputs `0x59C5E` / `0x1F65A`; pinned Lua 5.4.9 | partial: `game.state` reads and writes ROM-backed story facts through native accessors; the Stoker graph gate reads FFFC1E bit 0 and no longer invents opening/debt/package save flags. `game.location` remains read-only native context. `michael_package.lua` registers named host effects for the JSON conversation graph; JSON owns the indexed arrival/manager/debt/package and signed field-exit pages, choices, gates, and typed outcomes. `dialogue_graph.lua` is shared across locations; generic `motel.lua` owns reusable non-quest service choices. `field_interactions.lua` supplies ordered Maglock and Security Terminal action IDs plus outcome-to-ROM-text keys; typed native logic still validates passkeys, skills, and items and executes those actions. Script menus use a generic panel projection and generic choice roles. `game.services.call(domain, action)` yields allowlisted room/debt operations to native wallet/world/runner services and resumes with result IDs. Other field menus, other building dialogue, inventory/item operations, actor staging/animation, audio, and save restoration remain native or unconnected. The New Game trigger runs `field_events.new_game_opening_walk` as a Lua coroutine; its `game.actor.walk_heading("player", 0)` request is yielded to the native field controller and resumed on completion. C++ still owns movement execution, animation timing, scene transitions, payment and inventory transactions, presenter drawing/input, and non-motel menus. `game.ui.say(event)` and `game.ui.menu(menu, choices)` resolve speaker/text profile from native location context; the the indexed Stoker task runs the JSON arrival page and three-choice manager graph; its room/debt branches use typed host transactions and its confirmed same-map exits return `continue_in_place`. Only Stoker is currently bound as a portrait-bearing speaker; the JSON graph resolves his arrival, leave, room, debt, referral, and handoff text keys. Item operations, audio, actor placement/destination movement/animation, and save restoration remain unimplemented. All 134 startup, 16 Stoker, and 252 Hotel Debug/Release BMPs match by SHA-256; every Stoker route frame was reopened at 320x224. Exact Hotel pixels remain unverified without an independent Hotel capture. | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_startup_qa`; `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`; `shadowrun_ghidra_conversion_qt_field_hotel_qa`; [`gameplay-scripting.md`](gameplay-scripting.md); `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/{startup-qa,field-stoker-collision-to-dialogue-qa,field-hotel-qa}/` |
| 4 Field | Lua map-event policy and coordinate rules | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_world_content_tsv_loader_qa`; `shadowrun_ghidra_conversion_qt_field_map_event_system_qa`; `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa` | `field_events.lua` chooses collision actions and `building.lua` chooses location routes; native code validates and applies their typed results. Scene-entry and actor-within-coordinate rules support required state flags and once-per-visit/session policies. The scene-2 New Game intro handoff is the first source-profiled content rule: its exact spawn point, state gate, and repeat policy are authored in `gameplay/world/areas/scene-02-triggers.tsv` and loaded directly from TSV into Lua's runtime data table. Its full-screen route remains under `qt_runtime/field-stoker-collision-to-dialogue-qa/actual.bmp` plus the named checkpoints. All 16 current route screens were reviewed; Debug and Release artifacts match. Other authored area events and unresolved ROM record handlers remain open. See [the field-map profile](field-map-trigger-profile.md) and [route audit](visual-audits/field-map-script-routing.md). |
| Scene-2 Stoker opening auto-walk to shared arrival dialogue | `0x0BF2` / `0x59C5E` / `0x59CFE` / `0x07F0` / `0x075E` / `0xACAE` / `0xB574` / `0xBADE` / `0xB7B8` / `0x16396` / `0x1F5CA` / `0x1F65A` / `0x1F246` / `0xF4AA0` / `0x2BD2`; New Game sets `FFE18B`, then heading-zero movement reaches record 91 (`0xC2`) with no controller input; raw `0xFFC2`/event 2 selects Stoker, and A/B/A returns to the field | defended for normal and schedule-skipped New Game field handoffs, the one-shot zero-input auto-walk, and arrival route; Start in stage 2 preserves its second do-while pass; the airport loop clears its stop marker and continues through the cab route; the shared motel Leave selection also runs its farewell page through Lua and returns to the field. Independent section 03 confirms doorway/fade/panel appearance but does not log input or event state; sampled blank-panel timing and dialogue audio remain open | `shadowrun_ghidra_conversion_qt_startup_integration_qa`; `shadowrun_ghidra_conversion_qt_opening_start_skip_qa`; `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`; `build-ghidra-conversion-qt-{debug,release}/qt_runtime/opening-start-skip-qa/{actual.bmp,stoker-arrival.bmp}`; `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-stoker-collision-to-dialogue-qa/actual.bmp` plus 15 other full-frame checkpoints; nine-question audit: [motel-doorway-scene-entry.md](visual-audits/motel-doorway-scene-entry.md) |
| 4 Field | Dialogue dismissal transition and input isolation | `0x2BD2` plus the typed modal-return adapter | partial | `shadowrun_ghidra_conversion_qt_field_dialogue_qa`; `shadowrun_ghidra_conversion_qt_field_interaction_qa`; `shadowrun_ghidra_conversion_qt_field_world_activity_qa`; `shadowrun_ghidra_conversion_qt_security_terminal_physical_entry_qa` | `build-ghidra-conversion-qt-{debug,release}/qt_runtime/field-dialogue-qa/manager-package/modal-return/{transition-start.bmp,transition-mid.bmp,transition-complete-held.bmp,fresh-press-shot.bmp}` | ROM `0x2BD2` refreshes the field immediately. At the user's request, the Qt presenter now adds a 30-frame fade-in after dialogue/interaction return, suspends field behavior for the fade, and consumes held inputs until release; ordinary Start-menu return is unchanged. QA checks all 30 frames, no ammunition or shot-sound change while A is held, no action on release, one `0x1C` action on a fresh A press, and a clean full-frame redraw. Debug/Release selected CTests pass; the game code's source transition duration is not being claimed from this host choice. Independent Genesis timing evidence remains unavailable, so this hill is partial. See [the transition audit](visual-audits/motel-doorway-scene-entry.md#dialogue-dismissal-transition-and-a-input-isolation-2026-10-10) |
| Scene-2 Ares doorway to generic prompt and weapon shop | `0x075E` / `0xB7B8` / `0x16396` / `0x1FA78` / `0x1F246` / `0x1F3C4` / `0x56AC0`; record 92 (`0xCB`) publishes raw `0xFFCB`/event 11; ordinary Right/Up reaches the prompt, A enters shop, Start returns | partial; `field_mode.lua` selects the ROM's zero/positive/negative Ares narrative and logo from read-only field context; `shop.lua` supplies title, ordered category headings, stock, clips text, and transaction-message keys; `items.lua` maps all 50 source item IDs and 32 detail rows to ROM-generated Lua text. Debug/Release full-screen route QA covers route, categories, and transaction states. Other buildings and entry modes remain open | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_field_mode_text_runtime`; `shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa`; `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-ares-collision-shop-qa/`; 14 full-screen captures and eight-question audit: [ares-physical-entry.md](visual-audits/ares-physical-entry.md) |
| Scene-2 C0 doorway to scene-20 maglock prompt, passkey branches, Electronics trials, no-item result, and Walk Away | `0x075E` / `0xB7B8` / `0x16396` / `0x172CDC` / `0x1D9292` / `0x20E48` / `0x20E7A` / `0x20E92` / `0x20EC2` / `0x2BD2` / `0x21628` / `0x21692`; C0 records 89/90 select scene `0x14`; record 117 selects `0xFF82`/event 2 | partial; ordinary New Game input enters scene 20 and reaches the lock. A opens choices; C walks away. B with no item `0x0D` shows the no-item result through 30 frames and returns. QA quality-2 passkey succeeds; quality 1 against selection 2 retains insufficiency choices. Samurai no-skill Electronics returns a static no-choice result. Decker skill 1/no kit produces one partial success (2->1); the next physical contact resets selection to 2 at ROM `0xB994`. A QA-seeded kit and PRNG produce two of three successes; the result requests `0x4B`, and A return changes map index 483 from `0x82` to `0x81` via `0x2BD2`. Seven active route directories contain 352 matching Debug/Release BMPs. The six routes before the alert contain 302 BMPs and 81 distinct images; the alert route contains 50 BMPs with 37 distinct images within that route. The downstream entity-target actor/effect remains open. Item acquisition, other passkey qualities, grenade consumption/security, and exact independent Genesis visuals remain open | `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_passkey_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_insufficient_passkey_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_no_skill_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_alert_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa`; 45 physical-entry, 50 no-grenade, 48 passkey-success, 47 insufficient-passkey, 50 no-skill, 50 alert, and 62 Electronics-trials BMP captures; all 352 Debug/Release BMPs match by SHA-256; eight-question audit: [scene20-maglock-physical-entry.md](visual-audits/scene20-maglock-physical-entry.md)
| 4 Field | Profile-selected actor populations and Lua-owned behavior heartbeat | `0x43B6` / `0x4466` / `0x44BC` / `0x075E` / `0xA8EA` / `0x13874` / `0x13904` / `0x13D0C` / `0xB330` / `0x4618` / `0x5FDC` / `0x600E` | partial; each active extracted NPC profile receives a behavior heartbeat per field service and a render heartbeat per actor pass; Lua owns per-instance entry delay and C0B2 cadence. The controlled player receives both phases; Lua resolves ROM D-pad/script headings, applies the `0xB530` `0x0800` heading slew, and owns the C052/C062 animation clock. Lua resolves player collision routes/fallbacks and returns C052/C062 selectors; C++ supplies collision and map-event services and applies accepted positions. Connected weapon primary/secondary records now receive Lua behavior/render heartbeats. Lua owns projectile movement, retry/terminal policy, and secondary finite-list retirement; native callbacks supply terrain/entity samples and apply state/audio. General NPC collision policy is not ported. New Game bootstrap leaves the scene-2 profile list inactive per the ROM lifecycle; player weapon projectiles now receive Lua behavior, while the full 16-slot actor loop remains incomplete | `shadowrun_ghidra_conversion_qt_field_action_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_player_action_qa`; `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_movement_qa`; `shadowrun_graphics_test`; `shadowrun_ghidra_conversion_qt_field_employee_recruitment_initializer5_fuchi_scene0_pedestrian_qa`; `shadowrun_ghidra_conversion_qt_field_employee_recruitment_initializer5_fuchi_scene3_pedestrian_qa`; `shadowrun_ghidra_conversion_qt_field_employee_recruitment_initializer5_mitsuhama_scene3_pedestrian_qa` | The shared source actor census includes default-list rows across the extracted map set. The field runtime now loads source-supported kind-5 corp guards and kind-11 pedestrians for their mapped scenes and activates only the current scene. The viewer also displays conditional candidates and kind-8 yakuza / kind-15 Thon with named art. The A800 field selector now publishes its A8AE story actor and can convert randomized candidate rows through the ordinary actor path; the Scene-3/event-3 route now passes Debug/Release ordinary-input QA through Scene 34; after QA-seeded quality-2 passkey success, ordinary input crosses the unlocked record-90 cell and reaches the live A8AE actor; Debug/Release QA verifies the `0xECCC` dialogue and state path. The accepted initializer-5 four-site pool now has Debug/Release physical-entry coverage for all generated target pairs `(0,4)`, `(0,5)`, `(3,2)`, and `(3,3)`. These tests cover Scene-21 choices 1 and 2 plus Scene-30 choices 1 and 2; the Scene-3 routes also assert live effect-index suppression where `FFE12C=3` and `FFF0C8=0`. Continuous street travel remains open. The Scene-34 route crosses record 92 and record 90; Electronics and insufficient-quality lock branches and general NPC movement/collision remain open. C++ `FieldActorBehaviorState` was removed; `actor/heartbeat.lua` carries the instance state and emits behavior/render snapshots. Current Debug/Release runs produced 55 movement and 45 scene-20 full-screen BMPs per build; all paired images match by SHA-256 and every frame was visually reviewed. The scene-20 route now checks the mode-1 post-entry no-spawn branch at behavior ticks 61 and 82; those captures and the remaining route frames were reviewed as full screens. Longplay section 03 frames 4440-4469 corroborate the scene-2 street, HUD scale, and moving runner, but do not log the exact QA input. General NPC movement/collision, target acquisition, weapon use, damage, remaining live actor slots, and exact independent Genesis fidelity remain open. See [the actor heartbeat source audit](actor-ai-heartbeat-source-audit.md), [movement visual audit](visual-audits/field-movement-animation.md), and [actor population audit](visual-audits/field-actor-population-and-combat-handoff.md) |
| 4 Field | ROM random-world encounter creatures and target selection | `0x1E166` / `0x59986` / `0x1E216` / `0x1E25A` / `0x44BC` / `0x1438C` / `0x14446` / `0x14492` / `0x1DC4C` / `0x5C7E` / `0xB330` / `0xB574` / `0xB6E4` / `0xB1DC` / `0xC0A2` / `0x55C76` / `0x4618` / `0x5FDC` / `0x600E` / `0x14C7C` / `0xA8EA` / `0xABFC` / `0xD7EA` / `0x1A2CC` / `0x4E86` / `0x14D8E` / `0x14FF8` / `0x15A4C` / `0x15A6C` | partial; source scene rows allocate live encounter actors, use C0B2 entry art and delays, request their `0xB05E` entry cues, then run Lua target/movement policy on the ROM service cadence and B574 displacement table. The `0x13D0C` path now reads item/spell `+0x27` motion profiles, consumes the six-update runner re-link delay on its 21-pass timer, chooses direct versus lateral endpoints, carries endpoint distance into `+0xC8` step selection, and applies the `0x01E0` close gate to typed host geometry. `0x14C7C` now has source-backed target acceptance and route-stop state: generic and Gargoyle/Wendigo targets use the strict `<0x200` live-coordinate gate; Hell Hounds use the `<0x300` + `+0xDC` effect gate and `0x300..0x5FF` heading band; Gargoyle frame 2 holds route mode with `+0x1A=0x1000`. Accepted targets return no movement step. Lua starts the source C0A2 action pose and requests 0x4C/0x55; accepted linked headings additionally request 0x4D/0x4E by FFF9F8 bit 7, now mapped to resources 77/78 (2.116 seconds each). Lua script QA covers both FFF9F8 cue variants, and field QA requires 0x4C followed by 0x4D/0x4E. Lua runs the source 55C76 three-pass reaction and applies its physical/mental meter result to live party or world-actor records. A combat meter commit queues the damaged world actor's next Lua response: nonzero physical meter selects directional C072, waits eight actor-service passes, and requests B03E; zero selects directional C0B2, reserves +0x10.4, and requests B05E. Offline import exports directional hurt art/profile. Debug/Release Scene 2 QA drives a catalog Warhawk through a live projectile collision, requires the world-meter mutation, then captures both C072 frames and the kind-8 B03E request after eight behavior services. The fixture moves the spawned Weenr onto the projectile route; it verifies the connected hit-to-render path, not natural approach. The eight integrated hit-response images (four C072 and four zero-meter C0B2) are Debug/Release hash-identical and full-screen reviewed; map/HUD remain clean, C0B2 advances through both frames and holds, and B target cycling skips the active downed record. Audible playback/duration for these B03E/B05E requests is not independently verified. The player-side 0xB1DC hit-spark request now follows changed 0x55C76 meter results; focused C++ and Lua QA cover the kind-1 frames and 20 render-pass lifetime, ending in 0xAC4E retirement. The shared 0xE44D8 object art is imported as named BMPs. A plausible independent visual candidate appears in longplay section 25 frames 001133-001143: the red/white burst matches stages 1-3 at frames 001135-001140, but reticle overlap and 30-fps sampling prevent an exact comparison. Origin and actor/hit association remain open; see [player hit-spark audit](visual-audits/player-hit-spark.md). Target recovery and independently verified attack outcomes remain open. C062 walk and finite C0A2 action frames animate on the Lua render heartbeat. Lua handles the B rising-edge normal/special candidate scan at `0x1DC4C`; the host publishes the selected actor and `+0x11.4` latch. Lua drives the `+0xA6` shrink, and four action-code groups use the ROM's 8x8 corner tiles, palette lines, and flips as alpha-preserving PNGs. All four marker corners are pixel-checked in a full-frame Scene 2 dump. The automatic world-frame selector remains native. B selection remains silent. Ordinary target-action cue 0x4C plays resource 76 (2.234 seconds); after heading acceptance, 0x4D/0x4E select resources 77/78 (2.116 seconds each) by FFF9F8 bit 7; Hell Hound cue 0x55 remains silent on slot 85. At 1,200 Scene 2 service passes, both natural-spawn actors remain offscreen; their long approach is not independently corroborated. Hell Hound close effect now uses the ROM `+0xDC` delay, shared effect-slot allocation, three seven-render-pass phases, named scene-palette art, and silent source path; diagnostic Scene 0 captures check phase release and HUD preservation; exact map/effect depth remains unverified. Lua gate blocker/cadence/chance QA and a live Scene 2 production tick now pass, including chance and selector draws, two Weenr list records, source seed consumption, and delayed entry-cue requests; the per-actor spell/item gate now calls the typed 27-probe `0x14D8E` service with ROM placement tables and fresh per-sample `FFDFE2`; the current reviewed encounter rows contain no spell-action actor and this field QA has not invoked the callback. C082 art, `0x1033E` dice, `0x15830` effects/audio, and an independent spell-cast capture remain open. Enabled dialogue handlers, the alternate `+0x9C` marker source, full actor pool, independent `0x55C76` outcomes, and other target recovery and child-allocation paths remain open. Scene 2 sum-8 Weenr item code 1 now reaches the live `0x1514E` spawn cycle: Lua opens the close-range `+0xB0.4` action window, ammo changes 40->39, event `0x1C` is requested once, and the linked record heading selects C042. Debug/Release item-action QA checks all three source rows and full-screen captures. Other NPC item codes/effect branches, projectile target hit/status/damage, and an independently matched Genesis pose remain open; see `visual-audits/npc-item-action-rom-integration.md`. Script-runtime QA drives the 19 catalog profile/action pairs and checks each published decision. Post-down recovery/removal outside the traced hit path, natural creature approach, and independent Genesis hit evidence remain open The alarm alternate list at 0x1E20A now follows D0=0 into the screen-edge candidate builder at 0x1E302; the typed policy is covered by script-runtime QA, while a dedicated live alarm-override spawn QA remains open. | `shadowrun_gameplay_script_runtime_qa` (Debug/Release); `shadowrun_ghidra_entity_animation_spawn_probe` (Debug/Release); `shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` (Debug/Release); `shadowrun_ghidra_conversion_qt_field_hound_close_effect_qa`; `shadowrun_ghidra_conversion_qt_field_npc_item_action_qa` (Debug/Release) | `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-random-encounter-spawn-qa/` writes 14 encounter-route full-screen BMPs, five production-random-dispatch BMPs, and eight integrated hit-response BMPs, including the 1,200-pass frame, two walk frames, both C0A2 action frames, and shrinking/settled target markers. Independent section-06 combat frames 2190/2220 and 2430/2460 corroborate the separated yellow/red brackets, but do not identify their actor action bytes. The NPC item-action QA writes ten full-screen images, `actual.bmp`, and per-frame action/boundary traces under `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-npc-item-action-qa/`. The Hound effect QA writes `actual.bmp` plus entry, target, phase, and release frames under `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-hound-close-effect-qa/`. All 14 random-encounter and 8 Hound full-screen frames were reopened after the target-motion update; every Debug/Release SHA-256 pair matches, the HUD stays intact, and the shrinking marker clips its lower corners at the viewport edge. All eight integrated C072/C0B2 response captures were reopened at 320x224; Debug/Release image hashes match, map/HUD composition remains clean, section-04 longplay frames 294-315 show field combat movement and target brackets but do not identify this actor action or pose. See [creature targeting visual audit](visual-audits/field-random-encounter-creature-targeting.md), [random-event selector audit](field-random-event-selector.md), and [actor heartbeat audit](actor-ai-heartbeat-source-audit.md) and [spell/item line-probe review](visual-audits/actor-spell-item-line-probe.md) |
| 4 Field | Lua-owned weapon projectile behavior | `0x14E0A` / `0x14E32` / `0x14F26` / `0x14F50` / `0x14F86` / `0xAC4E` / `0x4618` / `0x5FDC` / `0x600E` | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_player_action_qa` | `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-player-action-qa/actual.bmp`; `weapon-fire/projectile-animation.tsv` | Lua owns primary four-probe movement, retry/terminal flags, outside-bound retirement, event `0x3C`, the stopped `+0x41` countdown, and secondary finite-list retirement. Typed native services sample terrain and active entities, then apply the returned record/audio state. Lua now calculates the primary projectile attack seed from live typed stats and its weapon template, resolves collisions using the D4540 item effect/difficulty, armor, and physical meter, and returns a typed target update; Qt validates the actor identity and writes the live party delay or world actor meter. Direct deterministic runtime QA verifies a Warhawk seed/hit and resulting meter. The generic player-action fixture has no live target records; separate Scene 2 encounter QA creates the ROM Weenr pair, lets actor 0 approach, B-selects it, and verifies one live Warhawk collision, meter change, and C072 hurt response. This connects one controlled collision-to-meter route. Linked status profiles, meter-zero/death/recovery, broader natural combat cases, and independent Genesis firing evidence remain open. Internal full-screen captures are not Genesis visual goldens. |
| 4 Field | Lua-owned field-location entry routes | `0x1F5CA` / `0x1F65A` / `0x167AE` / `0x173124` / `0x173138` / `0x17314C` / `0x172CDC` / `0x0D57A0` / `0x54222` / `0x17D9B6` / `0x17D625` / `0x17D9C0` / `0x17D620` / `0x17D9D4` / `0x17D623` / `0x17D9E8` / `0x17D626` / `0x17D9FC` / `0x17D629` / `0x17DA06` / `0x18ABBE` | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`; `shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_c0_exit_qa`; `shadowrun_ghidra_conversion_qt_field_gates_undersound_qa`; `shadowrun_ghidra_conversion_qt_field_movement_qa`; `shadowrun_ghidra_conversion_qt_field_crime_mall_ilene_gate_qa`; `shadowrun_ghidra_conversion_qt_field_riannons_magic_shop_entry_qa`; `shadowrun_ghidra_conversion_qt_field_dr_bobs_clinic_entry_qa`; `shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa` | `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-stoker-collision-to-dialogue-qa/actual.bmp`; `field-ares-collision-shop-qa/actual.bmp`; `field-scene20-physical-entry-qa/actual.bmp`; `field-scene20-c0-exit-qa/actual.bmp`; `field-gates-undersound-qa/actual.bmp`  `qt_runtime/{debug,release}/field-crime-mall-ilene-gate-qa/{ilene-not-met,ilene-met}/actual.bmp`; `qt_runtime/{debug,release}/field-riannons-magic-shop-entry-qa/{ilene-not-met,ilene-met}/actual.bmp`; `qt_runtime/{debug,release}/field-dr-bobs-clinic-entry-qa/{ilene-not-met,ilene-met}/actual.bmp`; `qt_runtime/{debug,release}/field-gang-contract-target-entry-qa/{active-puyallup-gang-contract,active-contract-targets-another-area,no-active-contract,active-eye-fivers-contract,eye-fivers-contract-targets-another-site,no-active-eye-fivers-contract}/actual.bmp`; qt_runtime/{debug,release}/field-gang-contract-target-entry-qa/{active-puyallup-gang-contract,active-eye-fivers-contract}/{event-contact.bmp,gang-contract-entry-dialogue.bmp}; route.tsv | `building.lua` selects the ROM-backed Stoker and Ares overrides and reads generic destination bytes from each active map's extracted event table; Lua classifies each ROM event-table target using the `0x167AE` field/resource/scripted/halted ranges and returns a typed `WorldActivityMode`; the Qt executor consumes that mode. Scene-2 C0 selects field scene 20, C1 selects resource scene 26, and Stoker remains a same-map motel interaction. Entity interaction JSON records also provide typed return-anchor/facing policy; the physical Scene-20 C0 exit QA checks source-probe point 0 (3884,10700) and the away-facing route. ROM supports the anchor/camera, while independent Genesis confirmation of the heading is still missing. Gates Undersound event 3 also uses the shared Lua entity route: Debug/Release physical QA reaches record 45, runs the story-gated Ogami page, and returns to scene 0. Scene-1 record 69 reaches Scene 13 from event-table row 5 at zero funds with no entry dialogue; ILENE_MET bit 19 gates only the post-load script-3 actor, confirmed in dedicated Debug/Release physical-input QA. Riannon record 72/event 3 also loads shared Scene 13 at zero funds without entry dialogue, with bit 19 gating the same actor; the dedicated Debug/Release route QA records this as a site crosswalk, not proof of a shop catalog. Dr. Bob record 74/event 6 also loads shared Scene 13 at zero funds with no entry dialogue; bit 19 gates the same actor, and route QA does not establish a clinic menu. Puyallup records 76/77/event 9 and Penumbra record 100/event 4 normally route to Scene 13, but their exact gang-contract targets open the ROM passage and route to Scene 15 with the selected actor groups; Debug/Release QA checks each match plus wrong-target/no-run fallbacks. Record 70 reaches Scene 15 from row 4; bit 29 gates only its post-load actor, also checked in both builds. Other direct collision continuations remain native; map names and reverse doors without ROM routes remain unresolved. See [field-map route audit](visual-audits/field-map-script-routing.md) and [ROM trigger profile](field-map-trigger-profile.md). |
| 4 Field | Scene-1/event-1 City Inn paid stay, Tabatha, and prompt routes | `0x53A78` / `0x53B16` / `0x1F65A` / `0x218BC` / `0x21862` / `0x22112` / `0x22124` / `0x219A4` / `0x2497C` | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_field_dialogue_runtime`; City Inn entry/payment CTests; four `...city_inn_tabatha_{caleb,errascoe}_{recruit,thanks}_qa` CTests | Debug/Release `qt_runtime/field-tarislar-city-inn-{qa,paid,insufficient,declined}-qa/*.bmp`; Debug/Release `qt_runtime/field-tarislar-city-inn-tabatha-{caleb-recruit,caleb-thanks,errascoe-recruit,errascoe-thanks}-qa/{actual.bmp,route.tsv,*.bmp}` | Entity data selects the indexed service for scene/event 1/1 with FFFC1E bits 0/1 set/clear and alternate result zero; the default link retains scene 53. Physical QA reaches record 68 and covers all manager choices, 30-paid/29-insufficient/30-declined wallet outcomes, both Tabatha topic orders crossed with Recruit/Thanks, the source 30-frame Holopix wait, and same-map return. Command 0x14 preflights FFFC30/FFFC27 before its Karma update. 101 route BMPs match across Debug/Release; the independent longplay confirms intro/menu/phone/apartment states, not payment or post-attack branches. The attack actor/combat handoff remains unresolved. See [City Inn route audit](visual-audits/tarislar-city-inn-scene1-event1.md). |
| 4 Field | Penumbra Wylie's Gala Inn scene-3/event-1 room service | `0x18ABB4` / `0x18A6FC` / `0x1F77C` / `0x249FA` / `0x1F816` | partial | `shadowrun_gameplay_script_runtime_qa`; `wylie_gala_inn_tests`; `shadowrun_ghidra_field_dialogue_runtime`; `shadowrun_ghidra_conversion_qt_field_wylie_gala_inn_qa` | Debug/Release `qt_runtime/field-wylie-gala-inn-qa/{actual.bmp,room-payment.bmp,room-paid.bmp,hotel-status.bmp,hotel-options.bmp}` | Ordinary Up reaches record 99/event 1. The JSON graph preserves inline entry/room prompts, signed package-result pages, the FF018F bit-2 free-stay gate, 50-nuyen payment, and typed `open_service hotel.options` / `continue_in_place` outcomes. Lua and C++ tests cover paid, insufficient, free, Leave, unknown state, both signed pages, and explicit boundaries. Debug/Release physical QA completes the paid route through the Hotel Options screen; all full 320x224 captures were inspected and match across configurations. No independent Genesis capture is identified, and other building routes remain open. See [the Wylie's Gala Inn route audit](visual-audits/wylie-gala-inn-scene3-event1.md). |
| 4 Field | Council Island Inn Scene-4/event-0 room service | `0x15BDCE` / `0x1F7CE` / `0x1F246` / `0x21458` / `0xCF38` | defended | `council_island_inn_tests`; seven `shadowrun_ghidra_conversion_qt_field_council_island_inn_*_qa` CTests | Debug/Release `qt_runtime/field-council-island-inn-qa/<route>/{actual.bmp,doorway-contact.bmp,spawn.bmp}` plus route checkpoints | Ordinary Up reaches record 92/event 0. Lua branches on signed FFF0D5, presents the two ROM text roots or the zero-result Crash/Leave and 40-nuyen Pay/Leave pages, and uses the typed `building.charge_40` service. Seven routes cover both signed results, both B/Leave choices, 39/40/100-nuyen balances, and same-map Hotel Status/Options after payment. Dialogue and Hotel pixel-region checks pass in Debug/Release; all 37 corresponding BMPs match by SHA-256. No independent Genesis capture is identified, so Genesis pixel fidelity remains open. See [the Council Island Inn route audit](visual-audits/council-island-inn-scene4-event0.md). |
| 4 Field | Renraku Offices scene-5/event-6 gated entry to activity 35 | `0x067C36` / `0x54076` / `0x110A2A` / `0x1195CF` | partial | `shadowrun_gameplay_script_runtime_qa`; `renraku_offices_tests`; `shadowrun_ghidra_conversion_qt_field_renraku_offices_qa` | Debug/Release `qt_runtime/field-renraku-offices-qa/{actual.bmp,activity-35.bmp,activity-exit-approach.bmp,activity-exit-contact.bmp,source-field-return.bmp}` | Lua applies the story-bit and alternate-result gate, displays the extracted page with its Renraku portrait, then resumes activity 35 after A. Ordinary Up reaches ROM record 46; Debug/Release tests check the MORE prompt, full-frame capture, destination, and ROM-selected resource index 5/marker 0. The typed activity-return snapshot is checked at the ROM probe lead with the opposite heading; Debug/Release ordinary Down QA now enters activity-35 record 121/event 0 and returns to Scene 5 at the saved probe lead with heading 0x8000; it seeds an adjacent activity cell, and independent Genesis interior/exit visuals remain open. See [the Renraku route audit](visual-audits/renraku-offices-scene5-event6.md). |
| 4 Field | Penumbra Fuchi scene-3/event-2 gated entry to activity 21 | `0x18ABA0` / `0x54152` / `0x11B7F4` / `0x11C213` | partial | `shadowrun_gameplay_script_runtime_qa`; `fuchi_offices_tests`; `shadowrun_ghidra_conversion_qt_field_fuchi_offices_qa` | Debug/Release `qt_runtime/field-fuchi-offices-qa/{actual.bmp,activity-21.bmp,activity-exit-approach.bmp,activity-exit-contact.bmp,source-field-return.bmp}` | Lua applies story bits 25/14 and alternate-result 0, presents the extracted entry page and Fuchi logo portrait, then resumes activity 21 after A. Ordinary Up reaches record 97/event 2; full-frame Debug/Release captures were inspected and assert resource index 2/marker 3. The typed activity-return snapshot is checked at the ROM probe lead with the opposite heading; Debug/Release ordinary Down QA now enters activity-21 record 102/event 0 and returns to Scene 3 at the saved probe lead with heading 0x8000; it seeds an adjacent activity cell, and independent Genesis interior/exit visuals remain open. See [the Fuchi route audit](visual-audits/fuchi-offices-scene3-event2.md). |
| 4 Field | Penumbra Lone Star scene-3/event-6 entry to activity 41 | `0x18AB96` / `0x18A6FC` / `0xB7B8` / `0x1651E` / `0x1F5CA` | partial | `shadowrun_ghidra_conversion_qt_field_penumbra_lone_star_entry_qa` | Debug/Release `qt_runtime/field-penumbra-lone-star-entry-qa/{doorway-contact.bmp,actual.bmp}` | Ordinary Up reaches record 96/event 6 and loads numeric resource-activity scene 41 through Lua's generic map route. The ROM trace has no site-specific title-service dialogue or story gate for this event. Full 320x224 contact and destination frames were inspected. The scene-41 return path and independent Genesis visuals remain open. See [the Lone Star route audit](visual-audits/lone-star-security-scene3-event6.md). |
| 4 Field | Scene-6/event-0 Owlfeather gated services and scene-13 fallback | `0x14C108` / `0x14B8EC` / `0x53B78` / `0x53BCC` / `0x53CEA` / `0x53C50` / `0xF4B68` | partial | `shadowrun_ghidra_conversion_qt_field_owlfeather_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_search_followup_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_return_followup_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_closed_gate_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_opening_land_choice_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_opening_council_choice_qa`; `shadowrun_ghidra_conversion_qt_field_owlfeather_ilene_gate_script_qa`; `salish_lodge_tests` | Debug/Release produced 39 full `320x224` captures under `qt_runtime/field-owlfeather-{physical-entry,search-followup,return-followup,closed-gate,opening-land-choice,opening-council-choice}-qa/`; matching BMP SHA-256 values are identical; route facts are in each `route.tsv`. The bit-19 route writes `field-owlfeather-ilene-gate-script-qa/{spawn,doorway-contact,actual}.bmp` and `world-script.tsv` | Ordinary Up reaches record 193. Service 2 checks bits 1/2; C selects the brother topic, applies command `0x17`, and sets bit 2. The Lands choice opens its topic page; Council opens its introduction and three-choice topic page. Thanks from either information branch shows both farewell pages, returns to Scene 6, and leaves bit 2 clear. Service 3 checks bits 2/3; on Scene 6/event 0, Later shows both farewell pages and returns to Scene 6 with bit 3 clear. Its separate Scene-3/event-3 branch opens 0xF4BB4, then sets FFE180; actor contact through 0xA8AE/0xECCC reaches 0x53C90 and sets ARAGORN_FOUND bit 3. The dedicated Lua graph and prioritized interaction link are covered by Debug/Release ordinary-input QA that walks through the unlocked record-90 door to A8AE, then advances all three dialogue pages and verifies source-slot retirement. Service 4 checks bits 3/4; the returned-brother sequence and Medicine Lodge referral finish through command `0x19` and the trigger tail, then return to Scene 6. With bit 1 clear, the indexed service is bypassed and event-table row 0 enters scene 13 directly, without a modal fade. With FFFC1E bit 19 set and both compact runner markers clear, the physical route selects script index 3. ROM-derived actor descriptors now flow through Lua into typed C++ state, queue until the next actor-service pass, and publish with the extracted position, profile, source flags, combat facts, and actor/effect slots. Debug/Release QA checks the published actor at `(0x1100, 0x0300)` with profile `lone11_k0a_item03` and writes the full-frame `actual.bmp`. Other script populations remain bounded by available extracted profile art. Full-frame captures were inspected at original size. The shared JSON graph now contains the Ranger/village/Sinsearach/Council loop and all three service endings; GameScriptRuntime tests cover representative paths. Exhaustive button-edge QA, other service outcomes, Scene-13 departure, and independent Genesis dialogue visuals remain open. See [the Owlfeather route audit](visual-audits/owlfeather-scene6-event0.md). |
| 4 Field | Scene-6/event-1 Red Buffalo Woman gated gift service | `0x14C112` / `0x54322` / `0xF4DB8` / `0xF4DCC` / `0x2196C` | partial | `shadowrun_gameplay_script_runtime_qa`; `salish_lodge_tests`; `shadowrun_ghidra_conversion_qt_field_red_buffalo_physical_entry_qa`; `shadowrun_ghidra_conversion_qt_field_red_buffalo_no_gift_more_qa` | Debug/Release `qt_runtime/field-red-buffalo-physical-entry-qa/{actual.bmp,gift-response.bmp,same-map-return.bmp,route.tsv}` and `field-red-buffalo-no-gift-more-qa/{no-gift-prompt.bmp,more-response.bmp,harlequin-meeting.bmp,michael-topic.bmp,thon-topic.bmp,thon-explanation.bmp,thon-curse.bmp,renraku-topic.bmp,renraku-explanation.bmp,renraku-update.bmp,same-map-return.bmp}` | Ordinary Up reaches record 194; service 18 uses bits 17/18 to select the gift or no-gift branch and suppresses the event-table scene-13 route. The gift path sets story bit 18, sets/clears Notebook tip bits 5/6, updates effect bits 29/30, and returns to Scene 6. Lua tests distinguish the ROM no-business leave text from the More penthouse/Harlequin response using nodes `0xF5888`/`0xF58A4`. The no-gift More physical route follows node `0xF58C0` through the Michael, Thon, and Renraku topic choices to the New Game no-Ito update, then returns to Scene 6 with bit 18 clear and effect bit 7 set. The shared `harlequin.main_conversation` JSON graph runs in both locations; Debug/Release GameScriptRuntime QA covers Ito and no-Ito paths plus the map handoff. Full 320x224 Debug/Release gift and no-gift frames were inspected and match by SHA-256. Independent Genesis dialogue capture remains open. See [the Red Buffalo route audit](visual-audits/red-buffalo-scene6-event1.md). |
| 4 Field | Scene-2/event-0 to Scene-20 Ilene prerequisite and Lady Gillian/Frosty dialogue roots | `0x543A2` / `0xEB92` / `0xECCC` / `0xED92` / `0x54402` / `0x21CE4` / `0x21FD0` | partial | `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_scene20_ilene_prerequisite_qa`; `shadowrun_ghidra_entity_proximity_collision` | Debug/Release `qt_runtime/field-scene20-ilene-prerequisite-qa/actual.bmp`, `ilene-contact.bmp` | Lua route preparation implements service 19's bit-18/19, FFFC34/38 gates and transient FFE182 write. Debug/Release ordinary-input QA reaches the paired scene-2 doorway, verifies the callback runs once, and enters scene 20 with the marker set. When FFE180 is clear, 0xA800 selects pending list 0xA8CC: template 0x172CA spawns at (0x0E80,0x0700) with ordered overrides ending +0x9E=0x08. The actor publishes in slot 3 before the default list (first default slot 4). Debug/Release physical QA checks position/profile. Scene-20 default actor service now applies the source-ordered 0xEB92 later-slot scan plus live runner record; its bit-3 pair callback requires partner +0x10 bit 1 and own +0xB0 bit 7 clear. 0xED92 consumes FFE182 and selects 0x54402. Lua activity execution awards +2 Karma per runner, sets story bit 19, updates FFFC30/34/38, and opens all three paragraphs from group 0x115ECC. Debug/Release QA checks out-of-range retention, contact, exact text/state results, +0xB0 one-shot behavior, dialogue lockout, and writes both full-frame captures. The separate Scene-34 A8AE world-effect contact path now dispatches through Lua and has Debug/Release UI QA; other actor-pair/world-activity handlers remain partial. Separately, Scene-13's bit-19 actor gate and Lua descriptor materialization have ordinary-input Debug/Release QA; that test seeds bit 19 and does not verify the Ilene activity. Command 0x34 selects Lady Gillian root F57E4 under source flags; command 0x44 sets bit 30 and opens Frosty root F57F8. Their live field-interaction caller and runtime wiring remain open. This hill stays partial. |
| 4 Field | Lua run-contract offers and success paths | `0x1E69C` / `0x1EEDC` / `0x1EF2C` / `0x1F1FC` / `0x1F00A` / `0x20A12` / `0x20990` / `0x559BC` / `0x215DA` / `0x1F086` | partial | `shadowrun_ghidra_matrix_lua_rules` (Debug/Release offer data and Matrix-generation checks); `shadowrun_ghidra_run_actor_defeat` (Debug/Release Ghoul area and payout QA); pure-Lua `run_offer_payment_profile_tests.lua` (40 rows, ranks 0-12, both roll endpoints); `shadowrun_ghidra_decimal_glyph` (Debug/Release 6,450 and 6,700 grouped-glyph cases); `shadowrun_ghidra_conversion_qt_field_scene2_gang_contract_entry_qa` (Release physical route); `shadowrun_ghidra_conversion_qt_field_scene2_gang_contract_entry_qa`; `shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa`; `shadowrun_ghidra_conversion_qt_field_big_rhino_entry_qa`; `shadowrun_ghidra_conversion_qt_field_wanderer_vacated_entry_qa` | `qt_runtime/{debug,release}/field-scene2-gang-contract-entry-qa/{approach,event9-contact,gang-contract-entry-dialogue,actual}.bmp`; `route.tsv`; `qt_runtime/{debug,release}/field-gang-contract-target-entry-qa/{active-puyallup-gang-contract,active-contract-targets-another-area,no-active-contract,active-eye-fivers-contract,eye-fivers-contract-targets-another-site,no-active-eye-fivers-contract}/{actual.bmp,route.tsv}`; qt_runtime/{debug,release}/field-gang-contract-target-entry-qa/{active-puyallup-gang-contract,active-eye-fivers-contract}/{event-contact.bmp,gang-contract-entry-dialogue.bmp}; `qt_runtime/{debug,release}/field-big-rhino-entry-qa/{first-booth-entry,johnson-root,actual}.bmp; qt_runtime/{debug,release}/field-wanderer-vacated-entry-qa/{spawn,event-contact,actual}.bmp; route.tsv` | All five ROM context tables now select typed Lua offers for canonical run types 0–6; the Matrix objective selector uses the ROM difficulty bands and candidate data. The connected New Game offer uses context 0 and the shared Wanderer Johnson interaction uses context 3. Scene 11 event 30 now routes to Mortimer Reed; Scene 14 event 30 applies the FFFC1E bit-16 departure gate, clears matching context 3, and shows the blank-profile no-choice page, with Debug/Release physical-entry QA; its conditional-credit root is script-tested; his work callback uses context 2 and its extracted opening text. Club Penumbra scene-3/event-0 now opens the named Lua doorman graph, which models the Reputation-4 gate, 75-nuyen fee, rejection pages, and distinct Mako story conditions. The story-qualified branch queues the typed list, shows both Mako introduction pages, and resumes to Scene 17, where the host publishes the four extracted actors; Lua dispatches the FFE104 callback after the four eligible transitions, updates heat/Karma/story/Strike Team state, and presents the Mako result page. Debug/Release ordinary-input Qt QA covers nine routes: the qualifying callback through Notebook tip 67, the alternate field-mode bit-21 lead, the bit-11 defeated state, both lead facts clear, bit 10 clear, Reputation-3 rejection, 74-nuyen insufficient funds, and the Later choice at both prompts. All identified Mako gate outcomes are covered; callback timeout, black Scene-17 destination captures, and independent Genesis comparison remain open. Scene-17/event-30 resolves to the separate shared one-page old-man dialogue through Lua entity data. Context 1 and context 4 run outcomes, non-Ghoul context-2 settlement, and Caleb's information/contact branch remain open. Big Rhino information/contact is implemented and Debug/Release tested. The new Debug/Release ordinary-input CTest walks into event 30 and captures the first page and root; independent first-entry/question comparison remains open. Caleb's accessible first-meeting route is linked and script-tested; Debug/Release physical-input QA captures the pages and root. Caleb's information/contact branch and run outcomes remain open. Package delivery remains connected. Ghoul offer text now follows its ROM-selected area; Debug/Release run-actor QA checks context-0 Redmond and all four Mortimer/context-2 area rolls, defeat counting, and matching Jump House-context payout/result dialogue. Pure-Lua payment QA checks all 40 rows, ranks 0-12, and both roll endpoints; maximum amount is 6,450 nuyen across ROM presets and 6,700 at rank 12, displayed as `6,700` by grouped decimal formatter `0xC216` at `$C322`; Debug/Release glyph conversion checks cover 6,450/6,700, and Current Run pixels now have Debug/Release QA for all seven canonical detail types, including the $10 minimum and grouped Matrix Expert $6,700 maximum; those native captures are internal regression artifacts, and independent positive-page Genesis evidence remains open. Independent Section 03 frame `003300` shows the New Game offer 40 -> 55, consistent with Joshua Human Shaman Negotiation 3; this verifies the Johnson offer path only. Escort spawn/state, gang fight results (the exact-target entry passage from 0xFE0DE is Lua-routed; Debug/Release physical QA verifies scene-2/event-9 plus Puyallup scene-1/event-9, including the Puyallup area-1/event-9 and Eye-Fivers area-3/event-4 matching-target passages and four-actor Scene-15 groups, plus wrong-target and no-run Scene-13 fallbacks), recovery/recruitment callbacks, Matrix return settlement, and full-screen run-result review remain open. Keep partial until each outcome path is traced, tested, and captured. See [the ROM run-success audit](visual-audits/run-contract-success-paths.md), [Big Rhino audit](visual-audits/big-rhino-scene11.md), and [the package route visual audit](visual-audits/package-run-origin-delivery.md). |

| 4 Field | AK-97/HK227-S/Mach-22 three-ray burst lifecycle | `0x1514E` / `0x152FA` / `0x14E0A` / `0x14E32` / `0x14F1C` / `0x14F86` / `0x150E8` / `0x15E56` / `0x15E16` / `0x55640` / `0x4618` / `0x5FDC` / `0x600E` | partial | `shadowrun_ghidra_entity_transition`; `shadowrun_gameplay_script_runtime_qa`; `shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` (Debug/Release) | `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-random-encounter-spawn-qa/actual.bmp`; `hit-response/first-burst-ray.bmp`; `hit-response/burst-followup-1.bmp`; `hit-response/burst-followup-2.bmp`; `hit-response/burst-settled.bmp` | ROM `0x152FA` seeds the special primary counter to 2 and saves the spawn origin; Lua restarts the live primary twice, preserves sub-coordinate fractions, rotates headings initial/−`0x400`/+`0x400`, recalculates the attack result, and derives source movement offsets. The first integrated test failed with no follow-ups because the typed special-spawn helper omitted `+0x41 = 2`; that omission is fixed and asserted by the entity-transition test. Debug/Release field QA checks three rounds consumed from one A action, both follow-up counter values, exactly one source-selected firing event, two full-screen follow-up captures, and clean retirement. Nine full-screen Warhawk/burst frames were reopened; the field and HUD remain intact. Longplay sections 04, 06, and 25 contain combat frames but do not isolate this weapon's three-ray sequence; its exact art/cadence has no independent Genesis match. Audio request count is checked, natural playback duration is not. See [the seven-question burst review](visual-audits/ak97-three-round-burst.md). |

Scene-`0x14`'s default actor profile contains eight rows, but the ordinary ROM
allocator at `0x1A342` provides seven records (slots 3 through 9) and
`0x4466` ends the list when allocation fails. The physical-entry QA now checks
that seven rows activate and the eighth stays inactive. This covers the
default-only route; pending-list allocation and the complete 16-record actor
lifecycle remain partial.

Independent Section 41 frames `002140`-`002170` match the mode-5 map and show a
walker following the westbound segment of Scene 5's default pedestrian route
at `0x679F0` / `0x67A30`. The capture supports that route match but does not
expose an actor-record ID. It is a default-list walker, separate from the
`0xA800` randomized choices. Debug/Release `field_pedestrian_scenes_qa` now
asserts Scene-5 order 1's `(0x4B80,0x2200)` start and movement toward waypoint
`(0x4B80,0x3980)`, and saves matching `scene-5-actual.bmp` captures. See the
[conditional pedestrian runtime
audit](visual-audits/conditional-pedestrian-runtime-gap-2026-10-26.md).

Independent Section-9 frames 1800-1818 provide a second route-level candidate:
a small sprite matching the extracted pedestrian walk frames moves west from
about `(720,543)` to `(698,544)`, along Scene 2 source order 0's ROM segment
`(840,544)->(640,544)` at `0x172D58`. The actor-record ID remains unverified;
the later Lone Star dialogue is not linked to this sprite. Section-37 frames
2340/2400 map to Scene 3 but their two visible figures do not match the
default pedestrian route direction. A six-frame Scene-1 recheck also rejects
the center figure that briefly overlaps source order 2's `x=1408` segment: its
map X moves from about 1411 to 1327 while two runner portraits remain visible.
The actor identity is not proven. All capture reviews are recorded in the
[pedestrian audit](visual-audits/conditional-pedestrian-runtime-gap-2026-10-26.md).

The separate Debug/Release
`shadowrun_ghidra_conversion_qt_field_region_map_pedestrians_qa` now checks
all three default pedestrians after QA scene reentry on each of the six
regional maps. It verifies their ROM-derived positions, profile, slots 3-5,
visible sprites, and movement, then saves six full-screen actual BMPs and a
route trace. All paired captures
match by SHA-256. The test complements the independent Scene-5 route match;
the screenshots themselves remain internal regression evidence. Exact patrol
cadence across every waypoint and collision behavior remain open.

The `0xA800` gated pedestrian branch and all 69 ROM candidate positions for
scenes `0x15..0x2B` are profiled in the actor population audit and extracted
from the US ROM into a checked named TSV. The standalone map viewer now shows
all three source candidates per scene with pedestrian art and the exact
selection gate, distinguished from active/default-list records. Direct tracing
of `0x0D3A` shows the bound-2 random path produces only choices 1 and 2. A
cross-reference of all `FFE183` writes finds only the sample store at `0xA87A`
and clears at `0x2A82`, `0xC684`, and `0x1E45E`; choice 3 is therefore an
unreachable source-table row after field-mode initialization. The `FFFBED=5`
gate compares the post-`F3DC6` value: raw initializers 5, 11, and 12 become 5,
while Package Recovery initializers 9 and 10 become 4.
All 69 rows now load into Lua and return through a typed host candidate lookup;
`shadowrun_gameplay_script_runtime_qa` checks their scene/choice ordering and
source fields in Debug and Release. `World.resolve_conditional_actor_selection`
now expresses the A800 branch order in Lua behind a typed C++ fact boundary.
Its QA covers A8AE/A8CC precedence, initializer normalization for 5/11/12 and
rejection of 9/10, word-width FFDF40 mode rejection, lazy bound-2 sampling,
retained choice after late-gate failures, unequal/equal 16-bit FFE12C/FFF0C8
comparisons, and selection of reachable choices 1 and 2 while retaining all
three table rows. Lua owns the choice; the field runtime calls its reset at the
modeled `0x2A32`/`0x2A44` and type-5 `0xC684` clears. The `0x1E45E` clear is
connected through `0x0A92` to the typed 13-record reset at `0x1E43C` and is
covered by focused Debug/Release QA. The field runtime now calls the selector
with the accepted offer's raw FFFBED byte, FFDF40, both effect markers, the
boolean zero/nonzero FFFC3C gate, and typed FFE12C/FFF0C8 words. Resource
initialization captures the selected index, and elevator
handoffs retain the current floor word across child-scene loads. The A8AE list
is represented by a typed world-activity actor row and publishes through the
ordinary slot-ordered actor path. Debug/Release `field_pedestrian_scenes_qa` verifies
the A8AE actor in Scene 34 at (0x1D00,0x4100), with the pedestrian profile in
slot 3, and writes `scene-34-story-contact-actual.bmp`. The selector bridge
also converts a chosen candidate row into a live pedestrian descriptor using
the extracted template profile and typed final spawn flags. ROM `0x44BC`
applies the list's byte overrides in order; the offline candidate extractor
records the final `+0x9E` value as `source_spawn_state_flags`, so runtime code
does not parse ROM offset strings. Debug/Release script QA
accepts Employee Recruitment through Johnson's work menu for raw initializers
5 and 11 at Matchstick and 12 at Caleb's booth, then verifies each normalized
A800 choice through the typed request scan.
Debug/Release field QA now performs the Johnson contact and Work choice with
ordinary field input in the same `FieldSceneRuntime` that accepts the offer
and enters the candidate resource scene. Scene 18 starts at its source player
record and walks Up, Left, Right, then Up into record 9/event 30; Scene 12
uses Caleb's documented `(16,80)` entry checkpoint and ordinary Up into event
30. B selects Work and A advances the source opening and generated offer text.
The tests check the Julius and Caleb source text, including Caleb's ROM
spelling `"If your half as good"`, then confirm the accepted raw initializer
remains in that runtime. Only the offer-selection random seed is set; later
negotiation and target draws run through their normal services. Scene 22
choices 1 and 2 use accepted initializers 11 and 12; all other cases use 5.
The QA checks one visible random pedestrian in slot 3 at each of the 46
reachable ROM positions and writes
`scene-<id>-choice-<n>[-initializer-<id>]-actual.bmp`. The candidate BMPs
match between Debug and Release, and the manifest records the initializer
for every candidate image. It also checks the scene's complete default guard
population from `default-actor-lists.tsv`: the A800 actor uses slot 3, and all
source-ordered guards remain active in slots 4 onward. This follows ROM
`0x43B6` list order and `0x1A342`'s slots 3-9. The manifest records each guard
count and slot list. The route also saves root, work-opening, and
generated-offer BMPs for initializers 5, 11, and 12 under
`qt_runtime/field-pedestrian-scenes-qa/johnson-offer-*`; these remain internal
runtime captures. The 46 candidate entries still use resource-scene reentry;
the matrix checks physical Johnson offer acceptance but does not travel to all
46 candidate scenes. Independent Section 36 longplay frames register to the
shared Scene-32/33 Mitsuhama map and show multiple sprites. One frame-8340
sprite matches kind-11 pedestrian art; the central mover in other samples
matches kind-3 runner art better. The previous six-point single-figure route
interpretation was retracted; the pedestrian-looking sprite's record and
producer remain unconfirmed. See
the [Section 36 visual audit](visual-audits/conditional-pedestrian-runtime-gap-2026-10-26.md#section-36-mitsuhama-map-and-moving-sprite-review).
A separate Scene-2 event-1 route now reaches Scene 26 through collision record
93 (`0x17314C`) and event-table row 1 (`0x172CDD`). Its Debug/Release QA
retains the accepted Employee Recruitment dialogue and source initializer 5,
then forces only the target tuple to map 2/event 1 because generated offers
do not produce that pair. At entry, A800 publishes choice 1 from list
`0x1EDCB0` at `(0x2000,0x0500)` in slot 3; the actor is off-screen at the
entry camera. The `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`
test and candidate BMP/TSV match across configurations.
This adds one source-routed candidate entry, not contact proof; the candidate
contact route now has a separate focused CTest. It follows the normal Scene-2
event-1 route and verifies that choice 1 is retained, then establishes the
documented Scene-26 static-profile QA checkpoint `(0x0A00,0x0500)`. The test
crosses both physical passkey gates (`0xA2 -> 0xA1`, `0x82 -> 0x81`) and walks
163 ordinary input frames to the candidate at `(0x2000,0x0500)`. Contact opens
the no-options Mortimer Reed page, sets actor `+0xB0.7`, retains the typed
FFE110 transfer, and dismisses with A while leaving the actor active. The QA
sets the ROM `+0x11.UNSEEN` guard for its long route to block unrelated random
encounters; it restores the original runner flags afterward. Debug/Release
CTest passes, and all 34 output files match by SHA-256. I inspected the
full-screen contact and field-return BMPs; they are internal captures, not an
independent Genesis comparison. The run does not prove continuous travel from
the normal Scene-26 resource-entry position to this candidate, normal offer
generation for the selected map/event pair, or the automatic `0x1E43C` request
route. Those paths and independent Genesis comparison remain open.

Scene-26 A800 choice 2 now has its own physical-contact CTest. It follows the
Scene-2 event-1 entry, selects list `0x1EDCCE` at `(0x2D00,0x2300)`, crosses
the additional record-120 A2 gate at `0x1D9E1E`, and verifies its map cell
`0x5E0` changes to A1 before reaching the Mortimer Reed contact page. Debug and
Release choice-1, choice-2, and pedestrian-scenes tests pass; 19 selected
captures and traces match across configurations. This route still uses the
static-profile Scene-26 checkpoint and the `+0x11.UNSEEN` encounter fixture.
Normal entry-to-candidate travel, automatic `0x1E43C` continuation, and
independent Genesis comparison remain open, so the A800 hill stays partial.

A separate Mitsuhama route drives the lift to Scene 34
for the A8AE story actor. The transition now installs the
destination mode before actor population,
matching the `0x167AE` to `0xA800` source order. The same
coordinator also has a separate higher-priority
static pedestrian list at `0xA8AE` (template `0x1D6988`, position
`0x1D00,0x4100`, ordered overrides `+0x9E=0x20,+0x2E=0x80,+0x9E=0x08`) when
`FFE180 != 0` and active effect index `FFF0C8 == 4`. The elevator route maps
this index to Scene 34, its fifth child. Service 3 at `0x53BCC`
sets `FFE180` at `0x53C3C` on its Scene-3/event-3 branch. Caller `0x75E`
continues after `0x1F5CA` clears the negative `FFF0CC` marker; its event row
loads Scene 30. The inclusive child group at `0x1DB004` is `[30,31,32,33,34]`,
so lift index 4 reaches Scene 34, the fifth destination in the Mitsuhama text
route. Contact dispatches `0x53C90`, sets `ARAGORN_FOUND` bit 3, and opens
`0xF4BBC`. The Scene-3/event-3 producer now has a Lua entry graph and gated
interaction link. Debug/Release
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_entry_qa` walks
record 95, checks the source text and profile alias, accepts with A, reaches
Scene 30 through event-table row 3, walks the entry corridor to the lift with
ordinary input, rides to Scene 34, and verifies the A8AE actor. It writes
`qt_runtime/field-mitsuhama-after-hours-entry-qa/{actual.bmp,scene-30-entry.bmp,route.tsv}`
and contact-page BMPs. The Scene-34 route probe writes
`qt_runtime/field-mitsuhama-after-hours-entry-qa/scene-34-contact-route-probe/{start.bmp,end.bmp,scene-34-maglock-introduction.bmp,scene-34-maglock-passkey-{action,result,wait-15,wait-30}.bmp,scene-34-after-passkey-return.bmp,route.tsv,passkey-action.tsv,passkey-map-delta.tsv,terrain-collision-map.tsv}`.
The route manifest records runtime arrival `(0x1F00,0x4900)` and validates
the three party positions from ordinary resource-entry placement at ROM
`0x1DAFE0`: `(0x1F00,0x4900)`, `(0x1F00,0x4B00)`, and `(0x1F00,0x4D00)`.
The ROM record also sets camera `(0x1600,0x4400)`; after transition/follow
updates the manifest records `(0x1600,0x43E0)`. The standalone elevator QA
uses a Scene-30 static-profile checkpoint `(0x3A00,0x0600)` and does not
establish this route; the Mitsuhama entry QA separately performs the walk
from the ordinary entry to event 29. QA also asserts `FFE180=1`, `FFE182=0`, active index
`FFF0C8=0`, selected index `FFE12C=4`, and no A8AE actor in Scene 30. That
matches the `0xA800` active-index gate; the elevator must advance the active
index to 4 before the actor is published in Scene 34.
The entry test seeds FFFC30 bit 12 and required FFFC1E bits; Lua route QA
covers both FFFC38 tip alternatives and closed-gate cases. After the QA-seeded
quality-2 passkey opens record 90, ordinary input crosses its unlocked cell,
skirts the row-24/row-27 obstruction on the west lane, and returns east on row
31 to reach A8AE. At contact the player is at `(0x1D26,0x40CE)`, within the
ROM `+/-0x40` window of the actor at `(0x1D00,0x4100)`. Debug/Release QA
verifies FFE180 is consumed, FFFC1E bit 3 is set, FFFC30 bits 10-12 are
cleared and bit 13 set, FFFC38 bits 4-5 are cleared, the actor's `+0xB0` bit 7
is set, and A presses advance all three `0xF4BBC` pages with profile `0x121E08`.
After the dialogue returns, `0x53C90` calls `0x1DED8`, which scans active actor
records and invokes `0x1E514` for records with the escort bit; that effect
clears the escort and active bits. Debug/Release QA verifies Aragorn's source
slot leaves the live list, the activity count drops by one while other records
remain active, and saves the closed Scene-34 image.
The accepted initializer 5/11/12 offer-to-request paths and the `0x1E45E`
FFE183 clear now pass focused Debug/Release script QA. Field QA covers all 46
candidate scene/choice pairs by walking to Johnson's work menu, accepting the
offer in the same field runtime, and re-entering the candidate resource scene;
Scene 22 choices 1/2 use initializers 11/12 and the remaining cases use 5.
Each candidate occupies slot 3 before its exact source default-guard list,
which remains active in slots 4 onward. This verifies the in-world
Johnson-to-offer segment and pending/default actor allocation. The per-choice
candidate matrix still uses resource-scene reentry, so it does not exercise
physical travel to every candidate map. Both index-mismatch cases start Scene
30 at the ROM profile checkpoint `(0x3A00,0x0600)` from a staging entry, with
selected index 4 and active index 0. Ordinary Up reaches event 29 records 82/83
at `0x1DB032`; the selector uses group `0x1DB004` `[30,31,32,33,34]`. Ordinary
Up selects index 4, A runs the source-timed lift for 17x8 frames per floor,
and Start completes the physical trip to Scene 34. Choice 1 uses source seed
`0x0000` and destination seed `0x8000`, publishing list `0x1EDE18` at
`(0x1800,0x0F00)`. Choice 2 uses source seed `0x8000` and destination seed
`0x0000`, publishing list `0x1EDE36` at `(0x0380,0x4880)`. Both use profile
`pedestrian_k0b_none00` in slot 3. The focused CTest writes the lift selector,
floor selection/arrival, Scene-34 entry, centered actor BMPs, and route TSVs.
It passes in Debug and Release; all 16 generated artifacts (14 BMPs and two
TSVs) match by SHA-256. I inspected the selector and both centered actor frames; they are
internal captures, not independent Genesis references. These routes start
from a staged profile checkpoint and do not exercise the accepted Johnson
offer through the lift. The FFFC3C pending-spawn case still retains choice 2
from Scene 30 and publishes it in Scene 34 through QA resource entry after the
pending marker clears. That covers A800's nonzero gate only. ROM `0xED60`
stores the tagged FFE110 template in FFFC3C, and `0x286E` uses it for the
`0x1E4AC` player-position spawn if no active/effect-ready record exists. The
typed transfer descriptor and FFFC3C producer/cleanup now run in Qt; Debug and
Release Scene-34 QA check the pedestrian template while A8AE dialogue is open
and its clear through `0x1E514`. Lua accepts 24-bit candidate list offsets and
leaves generic A800 contact on the shared activity path. The `0x286E` callback
now scans the typed seven actor records that can hold `+0xB0.7`, publishes the
`0x1E4AC` fallback at Joshua x+`0x80`/y through Lua, and suppresses a duplicate
on the next guard pass. The Scene-34 pedestrian QA captures
`scene-34-player-spawn-fallback-actual.bmp` and passes in Debug/Release. This is
an internal capture. Debug/Release `field_pedestrian_scenes_qa` now also walks
to Scene 30 A800 choice 2, list `0x1EDDDC`, at `(0x0900,0x0B00)` using ordinary
field input from the live runner position retained by the Johnson-offer
resource-entry case. After 59 host modal-release frames, 141 input frames reach
contact at runner `(0x08DA,0x0AD8)`. The contact sets actor `+0xB0` bit 7,
retains FFFC3C with the typed FFE110 pedestrian transfer, and finds the
existing effect instead of duplicating it. ROM `0xED60` installs callback
`0x1F146` for this generic contact. Lua now routes the reachable A800 row to
`run_employee_recruitment.present_random_contact`; the typed dialogue host
shows the ROM text at `0xFE1F7` with Mortimer Reed's `0x121FF4` portrait. The
no-options page waits for the source input interval, accepts A, returns to the
visible field, and keeps the actor active for the later request scan. Debug and
Release QA capture the page and the restored field as full-screen images. The
same QA now covers Scene 38 choice 1 from list `0x1EE0E8`, whose position record
at `0x1EE100` is `(0x0D00,0x0500)`. After the same 59 neutral entry frames,
55 ordinary Left/Down frames reach contact at runner `(0x0CD6,0x04DE)` and actor
`(0x0D00,0x0500)`. Its source choice 1 presents the generic
contact page and dismisses it with B while retaining the actor and FFFC3C
transfer; Scene 30 choice 2 uses A. The shared `0x1F3C4` no-options wait has a
separate Debug/Release input sweep for A/B/C/Start and all four D-pad events
after its 31-frame host lockout; neutral and low-byte input leave the page open.
Both physical cases keep FFE183 through the page and consume it only at the
`0x1E45E` QA seam. All 96 generated artifacts match between Debug and Release
by SHA-256. The bounded ROM census finds only a direct `FFDF42` read at
`0x0A68` and clear at `0x0A9E`; dynamic aliases remain unresolved. The typed
host callback at `0x1F16A` now dispatches on the next eligible coordinator
pass, keeps its `FFE11C` countdown separate from `FFE166`, retires the success
actor records, and opens Lua-owned success/failure result pages. Debug/Release
`shadowrun_ghidra_world_activity_routine`,
`shadowrun_gameplay_script_runtime_qa`, and
`shadowrun_ghidra_conversion_qt_field_world_activity_qa` cover the typed
request, both callback outcomes, the MORE continuation, run-offer state, and
the target-gated FFFBFB cap. The field QA writes full-screen `actual.bmp`
captures for both success pages and cancellation, plus the clean return field;
Debug/Release hashes match and the frames were inspected. These are internal
regression images, not independent Genesis comparisons. The direct `FFDF42`
request producer remains unknown, and the result QA seeds the scan outcome
after the A800/contact trigger. The automatic physical route and a route-matched
Genesis comparison remain open, so this callback remains partial. Separately,
the `FFDF3E`/`FFDF40` transition path to `0x1E43C` is traced and mode-2 gate
reentry is covered by the coordinator test. The body of `0x1F16A` is decoded as
actor-found Wasp pickup pages or a no-record contract-cancellation page, and
its typed host result callback and Lua pages now have internal frame captures.
The physical A800 routes still use
the `0x1E45E` QA seam and do not claim automatic activity continuation. These
captures are internal regression artifacts, not independent Genesis
comparisons; other physical A800 candidates and the direct request producer
remain open.
See the
`conditional-pedestrian-runtime-gap-2026-10-26.md` audit. The independent
Section-41 longplay now provides route-level matches for the default
pedestrian paths in Scenes 0 and 5; the exact actor row and route cadence stay
unidentified without actor-record telemetry. The ordinary-input Scene-34 route
reaches event 2 at record 90 after crossing record 92 and
opens the generic maglock introduction. A QA-seeded quality-2 passkey reaches
the success page after the source 10-frame choice gate, requests sound `0x4B`,
holds for the 30-frame result wait, and changes only record 90's collision-map
cell from `0x82` to `0x81` after A acknowledgement. The seeded passkey remains
in the live runner inventory. A second Debug/Release route reaches the same
Scene-34 record with a QA-seeded quality-1 passkey. Against selection 2, it
shows the three-choice insufficient-quality page, holds it across the source
input wait, and uses ordinary C to Walk Away. The item quality, selection,
collision map (`0x82`), and action-audio count stay unchanged; the returned
full-screen frame matches its field-only rendering. The new CTest writes 17
BMPs with matching Debug/Release SHA-256 hashes. These are internal captures,
not independent Genesis visuals, and the passkey is a QA fixture. Scene-34
Electronics outcomes and independent Genesis identification remain open, so
this hill remains partial. The separate Scene-20 interaction audit covers
Samurai/Decker Electronics outcomes, but does not substitute for this scene's
remaining routes. The route also exposed the delayed player hurt cue `0x40`
after the runner returned to walking animation; the C++ heartbeat validator
now accepts that source-delayed cue on the runner's behavior pass, and this QA
requires a clean heartbeat result.

The Stoker and field-object ROM dialogue strings have moved from separate TSV
catalogs into `assets/dialogue/script-text.json`. The offline importer writes
that keyed Lua table; native presenters resolve it through `GameScriptRuntime`,
while retaining their shared page, focus, and input behavior. The importer
also emits command labels, service titles, shop headings, and transaction
messages consumed by their existing typed presenters. This is a content
migration only: it does not claim that all title, inventory, or field menu
behavior is script-driven.

The 7x9 portrait stream follows the separate `BGT` repeated-tile and `BGE`
stepped-tile counts from ROM `0x1728`; the upper branch reads a fresh delta
for each emitted tile. A source-format CTest guards those counts and deltas,
and the Vid-Phone visual CTest checks the dialogue panel and portrait cleanup
for Henry and Quinton's service route. These captures use a black panel canvas;
they do not verify panel cleanup over a live map. The Stoker portrait remains matched to independent section 03 frames
000082, 000301, and 000601. The first arrival and three-choice package offer
now run through the indexed title
service and typed map-event route; the C debt question and B Jump House
referral are connected. Typed state resolves unpaid repeat visits to the
offer and both completed flags to ordinary motel service. The live New Game
route now exercises command `0x05` with a 250-nuyen QA wallet seed, applies the
source wallet/story/runner mutations, shows the package handoff, and returns to
the field. Debug/Release CTests cover the first physical entry, payment,
dialogue cleanup, and typed repeat/ordinary phases. The dialogue hill remains
partial because restored-save wallet/story state, physical repeat-entry
movement, and an independent paid-handoff frame remain open. See [the Stoker
scene audit](visual-audits/motel-doorway-scene-entry.md).

The Security Terminal floor-result hill above remains separate: that QA still
injects a resolved activity event after collision. Physical event `0x18` now
has partial scene-26 coverage from a restored static-profile checkpoint: ROM records 124/125
at `0x1D9E46` contain adjacent `0xD8` rectangles; ordinary input follows the
collision map around the `0x7E` wall, passes the two-negative-sample gate, and
opens the typed `building_security_terminal` prompt. That route seeds a
passkey at the two preceding maglocks. A separate source-coordinate QA opens
both terminal choices and returns to a clean field frame. No independent
Genesis terminal capture was found, so this is conversion regression
coverage, not visual-fidelity proof.
The eight-question audit is
[`visual-audits/security-terminal-physical-entry.md`](visual-audits/security-terminal-physical-entry.md).
The unrelated scene-2 record-2 route at ROM `0x172DBE` still produces event 0
and remains a rejected Stoker candidate; see
[`visual-audits/stoker-collision-candidate.md`](visual-audits/stoker-collision-candidate.md).

## Creature movement heading correction

Creature encounter QA exposed an inverted native branch in the ROM heading
measurement at `0x15DB0`: basis bit 6 skips `EXG D4,D5`, and only a clear bit
performs the exchange. The helper now follows that branch, with
`shadowrun_ghidra_movement_metric` checking all eight compass headings. The
Scene 2 actors now turn through the runner's area in the 480-pass trace, but
remain outside the viewport; the natural approach and encounter arrival are
still partial. The world-actor `+0x10.3` route also now consumes the typed
`+0x4D` posture used by ROM `0x13D84/0x13E58`; Debug/Release script-runtime QA
checks all nine posture values across the three range bands. Scene 2
production captures use flags `0x25`, so they do not exercise this branch. The
creature hill status and screenshot
review are in
[`field-random-encounter-creature-targeting.md`](visual-audits/field-random-encounter-creature-targeting.md).

Rejected (must not return):

- Extra Joshua archetypes as switchable party members.
- Mode `0x1A` (security interior) as the motel doorway.
- Generic “empty building” interiors.
- Six full-screen title frames as the claimed title sequence.

## Pocket Secretary root route (2026-10-15)

The `0xC2A6`, `0xC8C8` Group Items, and `0xC448` Dismiss Runner screens are
registered in the field Lua route. Lua owns root row wrap, A-only dispatch,
silent Up/Down navigation, C-button leader reselect, and Start return. Group
Items reads the typed FFFC22 fact, builds visible item IDs from bits 14 down to
0, and waits for a fresh controller edge. Dismiss Runner selection and
stable-party-slot dispatch are also Lua-owned. The shared E660 frame effect
keeps its 12-cell state and per-frame three-draw cadence in Lua; C++ supplies
ROM RNG and projects the resulting state into the Qt presenter. Notebook,
Tips & Clues, and Mr. Johnsons now also run through the Lua route: Lua filters
rows from typed ROM facts, owns selection, detail/no-run waits, C actions, and
return paths; C++ mirrors the route snapshot only for painting. Contacts and
Shadowrunners now have Lua-owned filtered lists, source paging, detail waits,
and typed Qt projections. Current Run dispatch at 0x1E72C is ROM-traced to Lua active_offer and now projects the positive page through a typed view. Debug/Release QA captures all seven canonical detail types and checks their source-backed headers, payments, bodies, and returns. The Lua matrix covers all 13 initializers, 24 Matrix objectives, 12,090 route pairs, and 145,080 Bodyguard route/client combinations. The positive-page inputs are covered through all 255 masks on the video-matched Courier record; restored-save invariants and independent positive-page Genesis evidence remain open. Cyberdeck, Save/Load,
and Debug remain explicit adapter/source boundaries. The hidden row remains unavailable
because the active field context does not import FFF101. Group Items remains
partial: normal FFFC22 acquisition and restored-save state are not wired.

The route CTest passes in Debug and Release. Field Secretary Qt QA passes in
both configurations and writes 95 full-screen BMP captures. Route checks cover
both E660 random branches, Group Items descending-bit rows and fresh-edge
dismissal, the no-run message, filtered Tips pages, all five diagnostic
Mr. Johnson details, C and Start actions, and the Dismiss stable-slot action.
The 95-frame Debug/Release BMP sets have matching paths and hashes. The
Contacts/Shadowrunners list, detail, paging, exact-eight, and return captures
were reviewed full-screen; child panels replace parent content and return
cleanly. A seeded frame now checks ROM E660 selector 0 -> CE136 yellow at the
first C13E8 destination; the full fixed-family selector-to-asset order is
recorded in the lower-bezel audit. This is internal regression evidence; an
independent Genesis capture is still unavailable, so the hill remains partial.

## Quest-gated building entry (partial, 2026-10-19)

The shared Lua entity-link resolver now selects ROM-gated entry tasks for
Renraku Offices (scene 5/event 6) and Penumbra Fuchi (scene 3/event 2). The
tasks use extracted text and profile assets, preserve event-table destinations
35 and 21, and require their source story bits plus alternate result zero.
Gates Undersound (scene 0/event 3) now has a one-shot, same-map title service
that updates the source FFFC30 bits after Ogami's departure text. Lua tests and
the Debug/Release script-runtime CTest cover all three route gates and the
Renraku/Fuchi extracted text keys. A new Debug/Release physical QA reaches
Gates Undersound record 45/event 3 with ordinary Up input, checks service 15's
gated page and same-map return, and writes full-frame `actual.bmp` captures.
Renraku now has Debug/Release physical-input QA from scene-5 record 46/event 6:
the MORE page advances into activity 35, and the full-screen `actual.bmp` and
`activity-35.bmp` were inspected. Fuchi now has Debug/Release physical-input
QA from scene-3 record 97/event 2; its MORE page advances into activity 21.
Both full-screen entry and activity captures were inspected. The entry routes now carry the ROM's FFE184 index-5/marker-0 and FFE187 index-2/marker-3 resource contexts from their Lua links into the typed activity initializer, and Debug/Release physical QA asserts both results. Debug/Release physical Down QA reaches Renraku activity-35 record 121/event 0 and Fuchi activity-21 record 102/event 0; both restore the source field at the ROM probe lead with heading 0x8000. The exit QA seeds an adjacent activity cell. Independent Genesis captures are unavailable for both interiors/exits, so these routes remain partial. Continue from the ROM-backed site route manifests rather than treating guide names as destination proof. See [the Gates Undersound audit](visual-audits/gates-undersound-title-service.md),
[the Renraku route audit](visual-audits/renraku-offices-scene5-event6.md), and
[the Fuchi route audit](visual-audits/fuchi-offices-scene3-event2.md).

## Entity route data migration (partial, 2026-10-22)

The currently modeled route set has since moved into entity-owned
`interactions.json`: 58 ordinary event-table continuations, eight conditional
map overrides, 13 dialogue routes, and eight field services across 70 entities.
Lua resolves those stable interaction IDs and checks ordinary map destinations
against the imported ROM event table; the C++ TSV loader no longer reads legacy
entity `links.tsv` routes. The shared mechanism is covered by Debug/Release
route/runtime tests, including ambiguity, unknown-condition, mismatched-source,
and source-default cases. Keep the hill partial: JSON coverage does not imply
that each destination's shop, entry dialogue, quest gate, activity behavior, or
return has been source-traced and physically exercised. In Scene 5, four guide
storefront triggers share event-table byte 0x23 but ROM 0x1FA78 intercepts them
as four named same-map field services with distinct mode/result values and
entry/package prompts. Debug/Release QA covers all four prompts. Weapon's
World now opens its Lua mode-0 catalog after the prompt, shows the first offer,
and returns to the field; the catalog rows come from the mode-0 tables selected
at 0x56DC4. Merlin's Lore opens its mode-5 catalog, but its purchase actions
remain partial. Wire-Masters and the deck-present MicroTronics menu still stop
at explicit boundaries pending their mode-specific action traces. MicroTronics
has a separately verified no-deck path: result 2 tests `FF018F` bit 0 and, when it
is clear, displays `STORE CLOSED!` from ROM `0x1AA3B4`, waits for input, then
restores field presentation. Debug/Release QA covers this gate and return; a
deck-present MicroTronics menu remains unresolved. The captures are host
regression artifacts, not Genesis visual proof. The site manifest records the
verified event-table pointer and row bytes. See the
[MicroTronics no-deck gate audit](visual-audits/microtronics-no-deck-gate.md),
[field-map route audit](visual-audits/field-map-script-routing.md), and
[the dialogue/location content model](dialogue-location-content-model.md).

## Council Island Medicine Lodge service screen (partial, 2026-10-20)

Scene 4 record 88 (`0x15BDA6`, event 3) reaches the ROM mode/event 4/3
handler at `0x1FCF2`; that handler keeps the player on scene 4 and opens the
Running Stream dialogue. The default and quest-bit-4 prompt roots are
`0x111876` and `0x111892`. Lua now owns the exact three choices, the bit-4
third-choice change, the Walking Bear response, the farewell, and command
`0xF0`'s runner-information bit-4 effect. The regular Debug/Release
physical-entry CTest starts south of record 88, walks Up to event 3, selects
Store with the source B-button choice, and opens the mode-3/result-0 menu on the
same scene. It writes full 320x224 offer, category, spell-detail, purchase
confirmation, purchased-item, purchased-focus, save-restored, and field-return
frames. Lua and native script-runtime tests
cover the link,
option labels, flag change, signed package handoff/payment branches, and
store/wisdom handoffs. Shop category and item selection now live in `shop.lua`;
C++ consumes the Lua snapshot and applies typed transactions. The physical QA
buys the first spell with an eligible runner and verifies the selected table row
advances from force 1 to 2 while funds fall from 10000 to 8000. The progression service
reads the runner's live spell table, selected by its +0xA0 table index. The
ordinary magic-item and focus purchases now pass through the physical menu,
including Yes confirmation, ROM-indexed costs, inventory writes, and save
round-trip preservation. Funds move 10000 -> 8000 for the spell, then 7500
after the 500-nuyen magic item, then 3500 after the 4000-nuyen focus.

The hill remains partial. The five 34-byte ROM progression rows at `0x166AD8`
initialize the shared `FFF9FA` working ledger, and mode-3 purchases now update
the selected typed table. The source menu uses each offer byte directly as the
`D4EA0` spell-row index. QA round-trips the purchased row through the host save
snapshot, restoring a lowered copy and then the saved force, inventory, and
wallet. Debug/Release physical-entry QA also selects the quest-gated wise-one
choice, confirms its left-door direction text, and verifies profile `0x1668A`
applies at `(0x5B00, 0x0880)` while staying on scene 4. The signed roots now
show their ROM package handoff and payment text through Lua, then return to the
same field map. Command `0x1F` publishes the
eight-word scene-entry profile at `0x1668A`; Lua owns the quest-gated dialogue
choice, and the field runtime queues the typed profile for the next full scene
entry. This replaces the previous false handler boundary. The internal
full-frame captures were inspected, but no independent Genesis frame of this
service screen has been identified, so visual fidelity remains open.
See [the Medicine Lodge route audit](visual-audits/council-medicine-lodge-entry.md).

## Council Island Spirit Eyes ritual (partial, 2026-10-21)

Scene-4 record 96 (`0x15BDF6`, event 27) routes through an entity-owned link
to the `salish.spirit_eyes.ritual` JSON graph. It follows the ROM first-contact
chain from `0xF7204` through command `0x15`, the Spirit Eyes description, both
one-choice questions, their response page, and the first horn request. The
bit-5 write happens after the opening page input. Later graph stages model
missing/present horn, pelt, and scale branches and their ordered item, story,
FFFC30, and FFFC40 effects. Scale completion records the `0xF51A8` page and a
typed `play_cinematic` result for command `0x4A`/`0x5D172`. A new
`cinematics.json` catalog records its four ordered ROM text pages, controller
input contract, resource-profile ID, named art alias, text layout, and
source-context return policy. The profile art is extracted to an individually
named 320x224 BMP, and Lua yields each page to the typed full-screen presenter;
both art aliases are now registered in `assets/resources.tsv`. Debug/Release
physical QA checks each exact page text key and unchanged art region while
advancing Sinsearach with C, Down, and Start; Spirit Eyes uses B, Left, Up,
and Start. All seven page captures match byte-for-byte across configurations.
The final typed outcome returns through the source-context modal path. The
bit-9 repeat route is limited to the single text-only page consumed by the
shared prompt runner; the adjacent four pointers are consumed by the separate
cinematic sequence.

Debug/Release script-runtime QA covers the graph branches and typed cinematic
result/catalog binding. Debug/Release physical-input QA reaches record 96 with ordinary Up,
walks the complete first-contact prompt chain, and saves full 320x224 frames
for every prompt and the clean Scene-4 return. C++ persists the FFFC40 ritual
markers as typed field state. Scene-47 record 62 has a JSON-backed Licourtrix
graph covering the pelt-first and scale-second repeat pages, the profile/mode
gate, both answers and the repeated-lie loop, and the post-dialogue story/item
grant. Debug/Release runtime tests cover each branch and the page order. The
ROM `0x2CE0` mode-table predecessor from context `0x14`/event 8 to context
`0x13`/Scene 47 and the `0x53EE2` activity handoff is covered by Debug/Release
field-mode tests. Seeded Debug/Release Qt QA runs the coordinator and scene
entry, verifies Scene 47, spawn `0x5E80,0x1300`, and five Lua-selected actors,
then opens the separate Renraku text-only page. QA covers the 60-frame activity
handoff, portrait/text panel, early A rejection, 30-frame any-button wait, and
typed in-place return; it saves `entry.bmp` and the complete 320x224 prompt
frame as `actual.bmp`. This is an internal capture, not an independent Genesis
comparison. The upstream context producer and physical record-62 input remain
unresolved. The host also does not present the ritual cinematic, and no
independent Genesis Licourtrix dialogue frame was identified. Keep this hill
partial. See
[the Spirit Eyes audit](visual-audits/council-spirit-eyes.md).

## Boris Greenhouse Scene-2/event-6 indexed service (partial, 2026-10-23)

ROM record 87 at `0x173110` has event 6 and a raw event-table value of Scene
26. Indexed title service 10 at `0x53F8C` loads profile `0x121F58`, runs the
dialogue runner `0x1F246`, and returns D6=-1, so the player stays on Scene 2.
The entity's `interactions.json` route gives this service priority over the
raw event-table row; the destination remains owned by the imported map data.
Debug/Release physical-input QA approaches the imported 32×32 trigger from the
south with ordinary Up, confirms the Boris portrait and opening choices, pays
the $100 story offer, and verifies FFFC26 plus the wallet survive save restore.
A second focused route seeds the already-paid information byte and the known
topic facts, then uses ordinary input to select David and Brother?. It checks
the capped `FFFBFB` increment, resolves the named `0x1D6FD0` list, spawns both
Strike Team actors at map-derived candidates, displays the extracted response
text at `0xF95E2` with the shared MORE prompt at `0xFE5BF`, and returns to Scene
2 with both actors active. The response event is now bound in Boris's speaker
profile. Eight full 320×224 captures per build match byte-for-byte; the new
`actual.bmp` is the field return with the spawned actors. These are internal
regression frames, not independent Genesis visual proof. The hill remains
partial: combat behavior and post-combat return are not defended. Command
`0x2D` now covers the source-priced Indian Lands Visa purchase and same-map
return. Command `0x2E` checks the first free runner slot before charging
10,000, stores item `0x14` at rating 2, and returns to the purchase menu. Its
Debug/Release route QA covers exact-balance success, full inventory with zero
funds, insufficient funds without inventory mutation, and save restore. The
`0x2F` Mach 22 command at `0x21C72` is now converted through the same typed
runner-item purchase service. It checks for a free slot, charges the ROM's
1,200-nuyen price, stores item `0x0A` with phase byte 0, and returns to the
purchase menu. Debug/Release QA walks the firearms menu, captures the settled
field after return, and checks the debit, first-free-slot placement, cleared phase byte,
and save restore. The Lua suite covers the full-inventory and insufficient-
funds responses. Other deeper service outcomes, combat completion, and
independent Genesis dialogue frames remain open.
See [the Boris Greenhouse event-6 audit](visual-audits/boris-greenhouse-scene2-event6.md).

## Mako encounter through prompt command `0x60` (partial, 2026-10-23)

The prompt dispatcher at `0x1F57C` maps command `0x60` through table
`0x12CCB4` to entry `0x225D2`. Its gates require Michael's alias learned,
Mako not yet defeated, a successful 75-nuyen debit, and either the Mako lead
or a separate FFFC38 bit-3 fact. On the qualifying path it sets pending list
`0x1E628`, selects mode resources at `0x122000`, and opens the ordered Mako
introduction descriptors `0xF4CBC`/`0xF4CD0`. The handler marks successful
payment at `FFF0D6`, clears D6, and the shared exit helper `0x1FED2` preserves
the clear result; the Scene-3/event-0 finalizer therefore resumes event-table
row 0 to field scene 17. The common field-frame service at `0x02900` reaches
`0x43B6`, which drains the four actor rows without a scene test and installs
callback `0x53FFC`. After the four `FFE177`-counted actor behavior transitions,
the callback records Mako defeated and opens his result page. `0x1DBAA`
separately arms the `FFDF86` activity counter. The extractor indexes those rows
under Scene 47, but that table context does not redirect this route.

The new entity link and Lua graph model the doorman prompt, Reputation
threshold, 75-nuyen fee, rejection pages, Mako gates, and both introduction
pages. The Mako branch queues the extracted actor list before the pages; Scene
17 resolves the four rows from the Scene-47 table context and publishes them
through the existing typed actor path. Lua behavior-completion edges decrement
a separate four-transition counter when `+0x9E` bit 6 is set and `+0x10` bit 3
is clear. At zero the host resets actor service links and invokes the Lua
callback graph, which updates numeric heat, party Karma, story flags, the
Strike Team progression counter, and presents the ROM result page. The new
`shadowrun_ghidra_conversion_qt_field_penumbra_mako_qa` physically enters
Scene-3 record 101 with ordinary Up, checks the 75-nuyen debit and qualifying
story gates, verifies all four pending-list actors, then checks the four
transition callbacks, source effects, result page, and field return. It writes
`actual.bmp` and `route.tsv` under
`build/conversion/qt/{debug,release}/qt_runtime/field-penumbra-mako-qa/`. The route also
exposed and fixed the kind-5 actor asset key and coroutine-local Lua `require`
lookup. Companion Debug/Release tests cover Reputation-3 rejection, 74-nuyen insufficient funds, paid entry with story bit 10 clear, the bit-11 defeated state, both lead facts clear, the alternate field-mode bit-21 lead, and the Later choice at both prompts; each writes `actual.bmp` and `route.tsv`. The callback route tests both Notebook tip 67 and field-mode bit 21 as independent lead sources. The Scene-17 return frame reached through the Mako route is black in the current host renderer, so that QA establishes route state rather than destination graphics. Keep partial: callback timeout and an independent Genesis capture remain unverified. Older restored saves with unavailable heat, FFFC30, or progression data still receive a source boundary.

Scene 17/event 30 is a separate generic one-page contact. ROM record 10 at
`0x1D63B4` carries raw `0x019E` over pixels `(0,32)..(31,63)` and dispatches
through the default `0x20990` branch to text `0xF3FBC` with no choice pointer.
The focused Debug/Release
`shadowrun_ghidra_conversion_qt_field_scene17_unidentified_contact_qa` route
checks the descriptor spawn, then approaches from `(16,80)` with ordinary Up,
opens the unnamed profile and no-choice passage, and dismisses it with A. It
checks the 60-frame return and visible host field frame, and writes full-frame
captures plus `route.tsv` under
`build/conversion/qt/{debug,release}/qt_runtime/field-scene17-unidentified-contact-qa/`.
This direct route renders the local Scene-17 frame; it does not resolve the
Mako-route black return or supply an independent Genesis comparison, so the
contact remains partial.

The Scene-10 northwest restaurant threshold is another direct event-30 contact.
Record 11 at `0x1D618C` carries raw `0x019E` over `(0,32)..(31,63)` and reaches
the same no-choice old-man profile through `0x20990`/`0x20C78`. Its focused
Debug/Release `shadowrun_ghidra_conversion_qt_field_scene10_northwest_contact_qa`
route checks the descriptor spawn, ordinary Up from a seeded south approach,
the dialogue page, A dismissal, and the visible same-scene return. Its eight
full-frame BMPs and `route.tsv` appear under
`build/conversion/qt/{debug,release}/qt_runtime/field-scene10-northwest-contact-qa/`
and match byte-for-byte across builds. No independent Genesis contact frame
has been found, so the route remains partial.

## Wire-Masters mode-7 cyberware selector and purchases (defended, 2026-10-22)

Scene-5/event 2 enters Wire-Masters mode 7/result 1. The result-1 setup in
`0x56AC0` uploads the icon patterns from `0xCF97C` to VRAM `0x89A0`; the eight
three-by-two icon grids come from `0xD177C`. `0x56DC4` routes the selected row
into the equipment-upgrade action. Lua owns the eight offers and labels, while
the typed transaction path applies the cyberware flag, Essence cost, and wallet
change.

Debug/Release QA covers selector input, all eight rendered icon slots, the
default-No confirmation, a completed Datajack purchase, already-owned,
Spurs-conflict, insufficient-Essence, insufficient-funds, and field-return
states. The eleven complete 320x224 captures match between configurations and
were inspected. No independent Genesis Wire-Masters frame appears in the
supplied longplay; this hill defends the traced behavior and host presentation
regression, without claiming exact visual fidelity. See [the Wire-Masters
audit](visual-audits/wire-masters-mode7-cyberware.md).

## Sinsearach village gate mode-9/event-0 dialogue (partial, 2026-10-23)

Scene-9 record 230 at `0x1B578E` crosses the village gate. In mode 9/event 0,
`0x1FD80` selects the guard's hostile/friendly root from story bits 18/20 and
Group Item bit 5, then returns D6=-1 rather than taking the generic scene-13
event-table route. The Lua dialogue graph connects the Elders choice through
command `0x34`, Lady Gillian's bit-9 Frosty referral, and command `0x44`'s
bit-30 effect and opening pages. It applies command `0x49`'s FFFC34 bit-4
clear and bit-5 set, plus a runner source-byte triplet `+3`. ROM helper
`0x5D130` writes FFF0BE=9, selects the three-page table at `0x0F5870` with
resources at `0x1FE690`, and calls the shared story-page routine `0x5D1F6`.
The catalog records those pages, named profile art, source text layout, and
typed `play_cinematic` outcome. The ROM art is extracted offline to a named
BMP; Lua owns page order, and the Qt presenter waits for controller input
between source text pages before returning to the same field context. QA
also proves that the A press which entered the cinematic, its held sample,
and its release leave the first page pixel-stable. It then advances with C,
Down, and Start. The controller-wait CTest sweeps all 255 nonzero port-byte
masks through ROM `0x195C` edge sampling and `0x0D94`'s high-byte event gate;
held samples and releases do not retrigger, while a fresh press does. The
physical QA also checks each page's first source-text glyph at the ROM-derived
text cell and preserves the extracted art below the text. The QA CLI writes the
first story page to `actual.bmp` and all three page captures in Debug and
Release. A one-frame-per-second search of the full supplied longplay found no
close match for this ROM art, so independent visual comparison remains open.
Debug/Release CTest `shadowrun_ghidra_conversion_qt_field_sinsearach_gate_physical_entry_qa`
uses ordinary Up input to reach record 230, drives the full Elders/Frosty
route through all three pages, checks the same-context return and source
effects, and writes full-frame `actual.bmp` captures. Debug/Release
`shadowrun_ghidra_conversion_qt_field_spirit_eyes_physical_entry_qa` reaches
record 96, drives all four ritual pages, and captures the page and clean
return. The gate's separate command roots are also modeled: `0x39` presents
the gate-return plus fare prompt, while `0x3A` presents the fare prompt
directly. Joshua's Yes/No replies and the guard's No reply use ROM text.
Command `0x38` subtracts 1,000 nuyen only when funds cover the fare, selects
the short-wallet page at `0x11B51D` otherwise, and selects the paid-flight
page at `0x11B450` when paid. The paid path calls mode initialization with
scene 6. That sets the direct scene-entry marker and default-profile flags;
the gate finalizer then re-enters the selected mode through the field loop,
without taking Scene 9's generic event-table row. Scene 6's profile pointer is
`0x14B890`; its three default field anchors are `(0x1310, 0x0980)`,
`(0x1220, 0x08C0)`, and `(0x1400, 0x08C0)`. The source text calls this a
highway flight, but the numeric destination is the only ROM-backed scene
identity established here. Lua owns the wallet action; no ROM-address
interpreter or second wallet state was added.

The first profile pair is the controlled runner's arrival: mode initializer
`0x136DE` sets FFFC0E=1 and FFE10E=1 for Scene 6; `0x03074` therefore skips
the pending-position override and copies the default profile; `0x2B34` then
copies its first position pair into runner record FF0100/FF0104. The ROM words
at `0x14B8D2` are `(0x1310, 0x0980)`. The existing host route selects that
first profile record. Debug/Release physical QA now starts with exactly 1,000
nuyen, reaches record 230 through ordinary Up, selects Later and Yes, checks
the debit to zero and paid-flight page, then completes the typed fade and
loads Scene 6 at `(0x1310, 0x0980)`. It writes the offer, Joshua response,
paid-flight page, and full-frame `actual.bmp` under
`build/conversion/qt/{debug,release}/qt_runtime/field-sinsearach-gate-paid-physical-entry-qa/`.
The existing fare physical QA covers 999 nuyen and confirms the short-wallet
page leaves the balance unchanged; its `actual.bmp` is the result page.
Debug/Release `shadowrun_gameplay_script_runtime_qa` now passes the typed
same-scene outcomes after No and insufficient funds, exact-funds debit, and
typed Scene-6 outcome assertions. The gate, fare, paid-fare, Spirit Eyes, and
script-runtime CTests pass in both configurations. Debug/Release paid-route
captures match byte-for-byte. The gate flag permutation audit exhausts all 128
combinations of story bits 13/17/18/19/20/30 and Group Item bit 5 through the
opening root and Elders handoff. It verifies 96 hostile, 16 friendly, and 16
Frosty-option roots, and that command 0x34 exposes Lady Gillian only in the
one combination satisfying all seven ROM conditions. These are flag-space
selector cases; the test does not claim every combination is reachable in a
campaign save. Three new Debug/Release physical-entry cases now capture the
question/answer loop, both question topics, the Elders fallback, Frosty's
unavailable-page self-loop and both exits, and the no-fare response through a
settled Scene-9 return. They assert source-derived A/B/C choice labels and
write full 320x224 frames. All 27 Debug captures match their Release hashes;
the question/answer, Elders, Frosty-left, fare-refusal, and final return frames
were inspected at native resolution. GameScriptRuntime still covers the
remaining graph choice edges and 128 source-flag selector combinations.
The hostile root now has a fourth ordinary-input Debug/Release route: story
bit 18 is clear, the full-frame prompt checks the distinct `0x11B5A0` text and
ordered Questions/Elders/Later labels, then the route traverses Questions and
the Elders/busy fallback to command `0x39`. Its eleven full-frame captures
match byte-for-byte across configurations, and `actual.bmp` is the hostile
prompt. I inspected that 320x224 capture at native resolution. The hill stays
partial because no independent Genesis encounter frame has been found. See the
[gate choice audit](visual-audits/sinsearach-gate-choice-permutation-audit-2026-10-23.md)
and [story-page source audit](cinematic-story-pages-source-audit.md).

Party wipe now follows the source `0xC64C` run-failure gates and one-page
messages for bodyguard, package delivery/recovery, and employee recruitment
before the shared `0xC716` recovery. Lua selects DocWagon scene 0 at
`(0x6200, 0x2D00)` when FFFC1E bit 0 is set, otherwise Little Chiba scene 2 at
`(0x6680, 0x2300)`; a focused Qt CTest checks both routes and writes
`qt_runtime/field-party-wipe-qa/{docwagon,actual}.bmp`. The timer at FFE166 is
not restored by `0x594E0`; whether the save writer copies it remains [H]. The
hill stays partial because `0x1DED8` scans world-actor high-effect slots and
the full `0x391A` runner-resource refresh remains outside the player-side
runtime path. No independent Genesis wipe frame has been claimed. See the
[party-wipe recovery audit](party-wipe-recovery.md).

## Empty field-alarm region profiles (partial, 2026-10-26)

ROM alarm profiles for Scenes 39, 40, and 43 declare `region_count=0` and have
no corresponding rows in `field-alarm-regions.tsv`. The typed profile reader
now treats a missing region table as empty only for those zero-count profiles;
populated profiles must match their declared count. Debug/Release
`shadowrun_gameplay_script_runtime_qa` checks populated Scene 21 and the three
empty profiles. Field alarm and random-encounter QA also pass in both builds.
This verifies profile loading and runtime continuation, not every alarm
outcome or an independent Genesis capture.

## Scene-40 default corp-guard route (partial, 2026-10-26)

ROM `0x44BC` initializes each default-list actor's route pointer and cursor;
`0x142AE` selects the waypoint branch through actor status bit `+0x10.5`.
Lua now enables route behavior for default-list actors with extracted waypoint
data, regardless of sprite kind. Debug/Release
`shadowrun_ghidra_conversion_qt_field_pedestrian_scenes_qa` checks Scene 40
source order 1, the kind-5 guard in slot 4, route `0x1DDE48`, start
`(0x2300,0x2580)`, and next point `(0x2A80,0x2580)`. The manifest records
progress at frame 34 and position `(0x236C,0x2526)`; start and moved full-frame
captures are retained under the build's `qt_runtime/field-pedestrian-scenes-qa`
directory. These are internal runtime captures, not a Genesis visual match.
The hill remains partial while other default-list routes, collision behavior,
and general NPC targeting remain open.

## Scene-20 default corp-guard routes (partial, 2026-10-27)

ROM list `0x1D91DA` loads five corp guards before pedestrians. Scene-20
Debug/Release `shadowrun_ghidra_conversion_qt_field_pedestrian_scenes_qa`
checks that the seven-record pool activates the first five guards and first
two pedestrians in source order, leaving the eighth row inactive. The first
three guard route lists at `0x1D9248`, `0x1D9252`, and `0x1D925C` advance
toward their second coordinates at frames 30, 35, and 44. QA saves start and
moved full-frame captures plus a route manifest under
`qt_runtime/field-pedestrian-scenes-qa`. Debug/Release outputs match each
other; they are internal runtime captures, not a Genesis visual match. Other
default-list routes, collision behavior, and general NPC targeting remain
open.

## Scene-34 record-90 Electronics no-skill branch (partial, 2026-10-27)

The physical Scene-3/30/34 route now covers ordinary New Game Samurai input at
record 90, event 2. ROM `0x21692` scans active runner Electronics bytes at
`+0x8A` and returns the no-skill page before rolling when the party value is
zero; the selected Scene-34 template is the ROM Samurai at `0x7291A`. Debug
and Release `shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_electronics_no_skill_qa`
checks the Electronics/Frag/Walk prompt, no-skill page and 30-frame hold,
selection 2, active-party inventory, locked map cell `0x82`, unchanged action
audio, and clean field return. Its 16 Debug/Release BMPs match by SHA-256, and
the choices, result, and return images were inspected at 320x224. They are
internal captures, not independent Genesis evidence. A second physical route
uses the ordinary Decker preset (template 1/class 3, Electronics 1), without
a kit or passkey. ROM `0x0DBA` with action-boundary seed `0x0006` yields one
face-5 success against target 5; the result reduces selection 2 to 1 without
changing the map or requesting sound. Its 16 Debug/Release BMPs match by
SHA-256. The physical Decker seed-0 route now reaches the zero-success result
and `0x1E04A`. Its QA suppresses the separate `0x1DFC4` region at
`0x1DB576` while walking to record 90, then verifies the fresh `0x960`
alarm timer, sound `0x30`, latch, reinforcement boundary, and all three
actors from list `0x1DB518` at their source-list positions. The new
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_electronics_kit_success_qa`
uses the ordinary New Game Decker (template 1/class 3, Electronics 1) and adds
only a QA fixture of ROM item `0x15`. ROM `0x21692` selects the three-die,
target-3 path; source seed `0x0004` yields faces 4, 2, and 4, meeting the
two-success requirement. It displays the success text at `0x11CD3A`, sets
selection `0xFF`, and requests sound `0x4B`. The result holds for 30 frames
with record 90 still locked at `0x82`; ordinary A commits the single map-cell
change to `0x81` while preserving the kit. All 16 BMP and five TSV artifacts
match between Debug and Release by SHA-256. The inspected prompt, choices,
result, and field return are internal runtime captures, not independent
Genesis evidence. Normal Electronic Kit acquisition and independent visuals
remain open; the hill stays partial.
See the
[conditional pedestrian and Scene-34 runtime audit](visual-audits/conditional-pedestrian-runtime-gap-2026-10-26.md).
