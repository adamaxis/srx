# Conditional pedestrian runtime gap audit

**Status:** `FieldSceneRuntime` calls the Lua `0xA800` selector with the raw
active run initializer, FFDF40, FFE180/FFE182, a derived presence projection of
the typed FFFC3C actor-template marker, and typed FFE12C / FFF0C8 indices. The
selected FFE110 actor descriptor is retained in typed state; eligible bit-3
contact copies that descriptor into FFFC3C, and escort cleanup clears it.
Lua accepts the candidate list's full 24-bit source offset and keeps generic
pedestrian contact on the shared activity path without opening a story task.
Debug/Release Scene-34 QA verifies the pedestrian template is pending while
the A8AE dialogue is open and is cleared by its `0x1E514` cleanup. The runtime
queues and publishes the selected A8AE story actor and randomized candidate
rows through the ordinary actor path. The Qt `0x286E` callback now scans the
typed active/effect state and publishes the `0x1E4AC` fallback through the Lua
actor service at Joshua x+`0x80` / y. Debug/Release QA covers the spawn and the
following duplicate-suppression pass. Debug/Release QA also walks into the
randomized candidates in Scene 30 choice 2 and Scene 38 choice 1 after QA-only
scene reentry. That seam reloads scene resources and actors while retaining
the runner's prior world coordinates; it does not establish a valid source
arrival or travel route. Source-valid travel to those scenes and an independent
Genesis comparison remain open. The
Scene-3/event-3 branch is represented by a Lua entry graph and gated route;
Debug/Release physical QA walks record 95, checks the `0xF4BB4` page and
`0x12209C` portrait, then resumes through the event-table route into Scene 30.
The QA records the resulting runtime runner position as `(0x1F00,0x4900)`.
The ordinary resource-entry placement at ROM `0x1DAFE0` encodes camera
`(0x1600,0x4400)`, then Joshua `(0x1F00,0x4900)` and the two party runners at
`(0x1F00,0x4B00)` and `(0x1F00,0x4D00)`. The QA validates all three runner
placements and records the post-transition camera `(0x1600,0x43E0)` after
camera updates. The 16 ROM bytes are `16 00 44 00 1F 00 49 00 1F 00 4B 00
1F 00 4D 00`. The separate elevator-group QA starts from Scene 30's static
profile checkpoint `(0x3A00,0x0600)`; the Mitsuhama route QA instead uses
ordinary input from the ROM resource-entry placement through the row-33 lane
to event 29 and records its movement trace. After the Scene-30 handoff,
Debug/Release QA observes
`FFE180=1`, `FFE182=0`, active index `FFF0C8=0`, selected index `FFE12C=4`,
and zero A8AE story actors. This is consistent with the `0xA800` gate: its
active-list choice requires `FFF0C8==4`; the selected resource index alone
does not publish the actor. The Debug/Release Mitsuhama QA now walks from the
ordinary Scene-30 resource entry to event 29, uses the live selector and lift
to set active index 4, and confirms Scene 34 and the actor. After the contact
callback QA, a separate ordinary-input route probe restores the ROM lift
placement `(0x3800,0x0600)`, walks around the west wall through collision
record 92 (`0x1DB990`, generic event 1), and reaches record 90 (`0x1DB97C`,
collision `0x0082`, event 2). The resulting `0x20E48` “sturdy door” prompt
uses selection 2. Debug and Release write matching full-screen
`scene-34-maglock-introduction.bmp` captures and a frame-by-frame input,
position, collision, and event trace under
`scene-34-contact-route-probe/`. The ordinary route now continues through the
unlocked record-90 cell, uses the west lane around the row-24/row-27
obstruction, and returns east on row 31 to approach A8AE. The passkey branch
is exercised with a QA-seeded live-runner item `0x14`,
quality 2, and ordinary A after the prompt's source 10-frame choice gate. The
route checks the result page and `0x4B` sound through its 30-frame wait, then
acknowledges it with A and verifies the runner returns to Scene 34 with only
record 90's collision-map cell changing from `0x82` to `0x81`; the seeded item
and quality remain in the runner inventory. Debug/Release captures include the
passkey action/result/wait pages, field return, `passkey-action.tsv`, and
`passkey-map-delta.tsv`; the BMPs have matching hashes. This covers the
quality-2 success branch. The new
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_insufficient_passkey_qa`
replays the route with a QA-seeded quality-1 key. At Scene-34 record 90,
selection 2, ordinary A presents the three-choice insufficient-quality page;
the screen holds through the source input wait, then ordinary C chooses Walk
Away. The test verifies that quality 1, selection 2, collision cell `0x82`,
and action-audio count remain unchanged, and that the returned full-screen
frame equals the field-only image. Debug/Release CTest passes for both the
quality-2 and quality-1 Scene-34 routes; the insufficient branch writes 17
BMPs with matching SHA-256 hashes. I inspected the 320x224 choice and return
frames. They are internal renderer evidence, not an independent Genesis
comparison; the passkey is a QA fixture. The new
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_electronics_no_skill_qa`
uses the ordinary New Game Samurai preset (template `0x7291A`, Electronics
byte `+0x8A=0`) on the same Scene-3/30/34 physical route. At record 90 it opens
the Electronics/Frag/Walk choices, selects Electronics with ordinary A, and
checks the `0x21692` no-skill result through its 30-frame hold and A return.
The test verifies active-party skill and inventory, selection 2, map cell
`0x82`, unchanged action audio, and a clean field-only return. Its 16 Debug and
Release BMPs match by SHA-256; I inspected the full-screen choices, result, and
return frames at 320x224. These are internal runtime captures, not an
independent Genesis comparison. The new
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_electronics_partial_qa`
uses the ordinary New Game Decker (class 3, Electronics 1) without a kit or
passkey. It pins the source PRNG at the Electronics action boundary to seed
`0x0006`; `0x0DBA` yields one face-5 success against target 5, reducing
selection 2 to 1. The partial result remains static for 30 frames, adds no
sound, leaves the lock cell `0x82`, and returns to a clean field with selection
1. Its 16 Debug/Release BMPs match by SHA-256; the prompt, result, and return
frames were inspected at 320x224. They are internal runtime evidence, not an
independent Genesis comparison. The Decker zero-roll path now uses the same
ordinary route with seed 0 at the Electronics action boundary. ROM `0x0DBA`
yields zero successes; `0x21692` selects alert text `0x11CD4A` and calls
`0x1E04A`. The walk crosses the separate Scene-34 coordinate alarm at
`0x1DB576`, so this route sets QA `FFE170` while walking and restores it
immediately after the direct action. ROM `0x1E04A` does not use that region
gate: it arms `FFE11E=0x960`, sets the clear `FFE16F` latch, requests sound
`0x30`, and processes list `0x1DB518` through `0x1E0DC`. Debug/Release
QA verifies three new actors at the coordinates in source-list records
`0x1DB534`, `0x1DB54A`, and `0x1DB55C`, the 30-frame alert hold, and clean
field return. Its 16 BMPs per configuration match by SHA-256; choices,
alert-result, and return images were inspected at 320x224. They remain
internal captures, not independent Genesis evidence. The new
`shadowrun_ghidra_conversion_qt_field_mitsuhama_after_hours_electronics_kit_success_qa`
uses the ordinary New Game Decker profile (class 3, Electronics 1) with only a
QA-seeded Electronic Kit (`0x15`). ROM `0x21692` switches this inventory state
to three trials against target 3; seed `0x0004` yields faces 4, 2, and 4. Two
successes show text `0x11CD3A`, set selection `0xFF`, and request sound `0x4B`.
The result is static for the source 30-frame hold and leaves record 90 locked
at `0x82`; ordinary A commits only that map cell to `0x81`. The kit remains in
the Decker's inventory. All 16 BMP and five TSV artifacts match between Debug
and Release by SHA-256. I inspected the complete prompt, choices, success
page, and clean field return; these remain internal captures, not independent
Genesis evidence. Normal kit acquisition remains open. Ordinary input
reaches the live A8AE actor at
`(0x1D00,0x4100)` within the ROM `+/-0x40` proximity window. A presses advance
all three `0xF4BBC` pages, and the source cleanup retires Aragorn's actor
record. The direct marker setters and `FFE120` producer census are traced
below. Gameplay-script QA now follows accepted Employee Recruitment offers
from their Johnson work menus through the normalized `0xA800` choice and the
typed `0x1E43C` request. Field QA now drives the Johnson offer tasks in a
single `FieldSceneRuntime`, then keeps the accepted offer state live while
entering each candidate resource scene. It covers initializer 5 throughout
the matrix and initializers 11/12 for Scene 22 choices 1/2. Ordinary input
reaches the Johnson contact and offer in the same runtime; the 46-candidate
matrix still uses resource reentry. Focused Debug and Release QA starts Scene
30 at its ROM profile checkpoint `(0x3A00,0x0600)`, reaches event 29 with
ordinary Up, selects floor index 4, rides the lift, and arrives in Scene 34
for both bound-2 choices. Choice 1 uses initial seed `0x0000` and opposing
destination seed `0x8000`; it publishes list `0x1EDE18` at
`(0x1800,0x0F00)`. Choice 2 uses initial seed `0x8000` and opposing
destination seed `0x0000`; it publishes list `0x1EDE36` at `(0x0380,0x4880)`.
Both actors occupy slot 3. The focused CTest passes in Debug and Release, and
all 16 generated artifacts (14 BMPs and two TSVs) match by SHA-256. These routes start from a
staged Scene-30 checkpoint; they do not connect accepted-offer input to the
lift or physically visit every candidate map. The inspected BMPs are internal
renderer evidence; independent Genesis comparison and the Section-41
route-matched visual check remain open.

### Scene-2 event-1 arrival at a Scene-26 A800 candidate

The focused `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`
adds one source-routed destination to the candidate entry checks. Ordinary
field input reaches Scene 2 collision record 93 at ROM `0x17314C`; event 1
uses event-table row 1 at `0x172CDD` to load resource Scene 26. The QA retains
the accepted Employee Recruitment offer and its source dialogue, but its
QA-only fixture overrides the offer target to map 2/event 1 because the normal
generated offer pool does not produce that target pair. Thus the field entry
and destination table are source-routed, while the selected Johnson target is
forced for this case.

At arrival, the QA checks raw source initializer 5, mode `FFDF40=2`, clear
`FFE180`/`FFE182`, clear pending actor marker `FFFC3C`, and equal
`FFE12C`/`FFF0C8`. The bound-2 selector chooses choice 1: list `0x1EDCB0`,
position record `0x1EDCC8`, coordinates `(0x2000,0x0500)`, and actor slot 3.
The candidate is not visible from the entry camera (`visible=0`). Debug and
Release CTests pass, and their candidate TSV and full-frame BMP hashes match.
This verifies the reachable A800 choice and actor allocation after one
ordinary-input ROM event route; it does not verify candidate contact or its
appearance when the runner reaches it. The QA-only target override, contact
route, normal Johnson target generation for this pair, and independent
Genesis comparison remain open.

### Scene-26 A800 choice 2: third passkey gate and physical contact

The separate `shadowrun_ghidra_conversion_qt_field_scene26_a800_choice2_contact_qa`
now follows the normal Scene-2 event-1 entry and selects the second reachable
A800 row. At the event-contact random reset, source seed `5` produces the
bound-2 choice result `1` (choice 2); the following raw advance and bound-3
result `0` satisfy the active-effect gate. The candidate is list `0x1EDCCE`,
position record `0x1EDCE6`, `(0x2D00,0x2300)`, heading `0x6000`, actor slot 3.
The accepted Johnson target is still a QA-only map-2/event-1 override, and the
route still uses the documented static-profile checkpoint `(0x0A00,0x0500)`
with the ROM `+0x11.UNSEEN` encounter guard set for the long walk.

After the two previously audited passkey doors, ordinary D-pad movement
reaches the third A2 lock: Scene-26 collision record 120 at ROM `0x1D9E1E`,
normalized bounds x `0x400..0x41F` and y `0x2E0..0x2FF`. The blocked candidate
at `(0x42B,0x2F8)` samples `(0x41F,0x2F1)` from its upper-left probe, selecting
map index `0x5E0`. The prompt shows the source “sturdy door” text and `<MORE>`.
After the prompt's 10-frame input gate, ordinary A displays the Maglock
Passkey success result and requests sound `0x4B`; the page stays stable for its
30-frame acknowledgement interval. A then returns through the 60-frame fade.
The collision-map diff contains only index `0x5E0`, changing `0xA2` to `0xA1`.

The same field scene then walks to choice 2 in 313 ordinary movement frames.
Contact opens the no-options Mortimer Reed page, sets actor `+0xB0.7`, retains
the typed `FFE110` transfer, and the A dismissal returns to a clean field with
the actor still active. Debug and Release CTests pass for both choice 1 and
choice 2, as does the broader pedestrian-scenes CTest. The 19 selected
candidate, collision-probe, route, and full-screen BMP artifacts match by
SHA-256 across configurations. I inspected the lock prompt, passkey result,
contact page, and clean field return; these are internal Qt captures, not an
independent Genesis comparison. Continuous travel from normal Scene-26 entry,
normal Johnson generation of the map/event pair, the automatic `0x1E43C`
request path, and independent Genesis comparison remain open.

### Scene-22 A800 choice 1 contact

The isolated `shadowrun_ghidra_conversion_qt_field_scene22_a800_initializer11_contact_qa`
starts with raw run initializer 11, which the source normalizer maps to
`FFFBED=5`; the destination mode is `FFDF40=2`. The selected A800 row is list
`0x1EDF26`, position record `0x1EDF3E`, coordinates `(0x0300,0x0700)`, heading
`0x4000`, spawn flags `0x08`, and actor slot 3. The route starts from the QA
resource-reentry checkpoint `(4906,1156)`, not a traced normal Scene-22 entry,
then reaches the candidate with 154 ordinary directional inputs.

Contact opens the source-backed Mortimer Reed page with portrait
`dialogue.portraits.mortimer.reed`, no options, and text “Hey! There you are.
Let’s go. My new employer is waiting.” After the ROM `0x1F3C4` 30-VBlank input
wait, C dismisses the page. The runner returns to Scene 22 while the candidate
actor remains active, its `+0xB0.7` escort flag is set, and the typed
`FFE110` player-spawn transfer is retained. The callback's mode-5 `FFFC1C`
bit-0 state and the selected choice are checked at the `0x1E45E` QA seam; this
does not execute the unresolved `FFDF42` / `0x1E43C` automatic request path.

The fresh-runtime CTest is separate from the 46-case matrix because its
retained contact state otherwise affects a later candidate case. Debug and
Release CTests pass, and all 11 route, candidate, and full-frame artifacts
match by SHA-256. I inspected the entry, contact page, and closed field frames.
These are internal Qt captures, not independent Genesis evidence. The supplied
`au/e8_karma_spend` recording is Scene 2/event 2 and does not show this
Scene-22 contact route; a route-matched Genesis capture remains open.

## ROM path confirmed from the US image

The ROM image at `decompilation/input/shadowrun_genesis_us.bin` is 2 MiB, so
these ROM addresses are also the corresponding file offsets. At `0xA800`, the
raw bytes begin `4A 39 00 FF E1 80`, followed by the branch to `0xA82C`. The
disassembly at `disassembly.s:0000A800..0000A8AC` confirms this precedence:

1. Nonzero `FFE180` selects static list `0xA8AE` only when the 16-bit active
   effect index `FFF0C8 == 4`, then returns without considering `FFE182`.
2. Otherwise, nonzero `FFE182` selects static list `0xA8CC`.
3. With both markers clear, the pedestrian branch requires `FFFBED == 5` and
   `FFDF40 == 2`. If `FFE183` is zero, it samples bound 2 through `0x0D3A`,
   adds one, and stores choice 1 or 2. It then requires
   `FFE12C == FFF0C8` and `FFFC3C == 0` before calling `0x5BA98`.

The source list order is explicit at `0x43B6` (`disassembly.s:000043B6`): it
calls `0xA800`, drains the resulting `FFE124` list, then drains `FFE120` when
present, and finally drains the scene default list. `0x4388` calls `0x43B6`.
The original ROM bytes at `0x43B6` begin `0C 39 00 02 00 FF E1 86 66 00 00
08`, matching the conditional resource load before list preparation. The
earlier pending list therefore consumes an ordinary actor slot before either
later list.

### Higher-priority static pedestrian list at `0xA8AE`

The `FFE180` branch is a second pedestrian path, separate from the
`0x1EDB88` candidate table. The US ROM bytes at `0xA8AE` are
`80 1D 69 88 00 00 A8 C6 00 00 00 02 9E 20 2E 80 9E 08`: the tagged template
is `0x1D6988`, the position record at `0xA8C6` decodes to `(0x1D00,0x4100)`,
and the ordered overrides are `+0x9E=0x20`, `+0x2E=0x80`, `+0x9E=0x08`.
At `0xA800`, this list is selected only when `FFE180 != 0` and the active
effect index `FFF0C8 == 4`. That branch returns before the `FFE182`/Ilene list and
before the random pedestrian checks. The static list has no scene ID in this
selector, so its placement depends on the active resource group and live
`FFF0C8` state rather than a scene ID embedded in this list.

The actor is the kind-11 pedestrian template `0x1D6988`, with palette line 3
and final overrides `+0x9E=0x08` after the intermediate `+0x9E=0x20` and
`+0x2E=0x80` writes. That final interaction bit connects the list to the
proximity path: `0xECCC` checks bit 3 on one actor and bit 1 at `+0x10` on its
partner; when `FFE180` is active it clears the marker, installs handler
`0x53C90`, and begins the world activity. Handler `0x53C90` sets FFFC1E bit 3
(`ARAGORN_FOUND`) and opens root `0xF4BBC` with portrait profile `0x121E08`.
This makes `0xA8AE` a one-time story-contact actor rather than an unconnected
pedestrian candidate.

The ROM-side source trace for the route is connected across the service,
field coordinator, resource, elevator, and contact handlers. Service 3 at
`0x53BCC` sets
`FFE180` on the Scene-3/event-3 branch after opening root `0xF4BB4`. The field
loop calls `0x1F5CA`, which clears the negative `FFF0CC` abort marker and
returns to caller `0x75E`; it does not cancel the caller's next transition.
Because D6 is clear and `FFFBF9=3 < 8`, `0x75E` takes `0xA32` and calls
`0x167AE`. Event-table row `0x18A6FC` selects resource Scene 30. Resource
initialization loads the list header value 4 into `FFF0C6`, leaves the active
`FFE180` marker set, and forces `FFE12C=4`.

The Scene-30 elevator list at `0x1DB004` begins with maximum index 4 and then
contains `[30,31,32,33,34]`. The selector's normal route
`0x58D94 -> 0x58DE0 -> 0x58FF8 -> 0x16A8E` advances `FFF0C8`; index 4 loads
numeric Scene 34. `0xA800` selects the `0xA8AE` pedestrian at this index, and
its final `+0x9E=0x08` override connects it to `0xECCC`. Contact dispatches
`0x53C90`, sets `ARAGORN_FOUND` bit 3, and opens `0xF4BBC`.

This source route joins the `0xF4BB4` text about slipping into Mitsuhama with
the numeric destination Scene 34. Notebook tip 70 says Aragorn is transferred
to Mitsuhama's fifth floor; index 4 is the fifth child in the source list.
Thus Scene 34 is the source-derived fifth-floor destination for this story
route. The building identification comes from joining the dialogue branch,
event-table row, shared group pointer, and elevator index; no independent video
capture has yet been matched to the scene or actor.

The actor data now has both the marker-gated `0xA8CC` row and the fixed story
contact `0xA8AE` row. The field runtime calls the Lua selector with live
marker, mode, pending-spawn, and separate 16-bit active/selected-effect facts.
The `0x167AE` entry path clears `FFF0C8` for scenes `0x15..0x2B`; the elevator
selector later changes that source index as it selects a child scene.

There is one direct `FFE180` producer at `0x53C3C`, in the Scene-3/event-3
branch of indexed service 3 (`0x53BCC`). It requires FFFC1E bit 2 set, bit 3
clear, `FFF0D5 == 0`, and at least one of FFFC30 bit 12 or FFFC38 bits 4/5.
The branch selects mode-resource profile 2 at `0x53C22`, opens root `0xF4BB4`
with portrait profile `0x12209C`, then writes `FFE180=1`, clears D6, and writes
`FFF0CC=0xFF`. The same handler's separate Scene-6/event-0 branch at `0x53C50`
opens the repeat prompt `0xF4B84`; that remains the separate Owlfeather path.
The Scene-3/event-3 route is now represented by the `salish.owlfeather`
`mitsuhama_entry` graph and the Scene-3 interaction link. The graph presents
the extracted `0xF5B00` text with profile `0x12209C`, arms the Lua
`world_effect_active` fact after the page, and resumes the selected source
route. `shadowrun_gameplay_script_runtime_qa` covers both FFFC30/FFFC38 lead
alternatives and the closed story/alternate-result gates. Debug/Release
physical QA seeds FFFC30 bit 12, reaches record 95 with ordinary Up, checks
the page and portrait alias, and accepts through to Scene 30. It does not yet
verify a live lift selection or the Scene-34 contact. Its `route.tsv` records
the runtime Scene-30 runner position `(0x1F00,0x4900)` and validates all three
party placements from ROM `0x1DAFE0`; it also records the post-transition
camera after follow updates. The same manifest records
`FFE180=1`, `FFE182=0`, `FFF0C8=0`, `FFE12C=4`, and no A8AE actor in Scene 30,
matching the `0xA800` active-index gate. The separate Scene-30 elevator QA
begins at the ROM profile checkpoint `(0x3A00,0x0600)`, so the physical path
between those two positions remains untraced and must be followed before
treating the elevator tests as one route.

The `FFF0CC` check is an internal early return from the indexed-service
orchestrator at `0x1F5CA`; caller `0x75E` continues after it returns. The
Scene-3/event-3 route reaches Scene 30 without crossing the special
`0xC64C`/`0x2A32`/`0x2A44` reset path, and the normal resource entry and lift
handoff preserve `FFE180` until the actor-contact handler consumes it. The
Section 41 sprites remain visual context only and do not independently prove
the source-derived route.

The candidate table currently contains 69 rows: three source-table entries for
each scene `0x15..0x2B`. The ROM's only non-clear write to `FFE183` is at
`0xA87A`: it stores the `0x0D3A` bound-2 result plus one. The bounded sampler
returns the high word of a 16-bit PRNG value multiplied by the bound, so the
stored choice is 1 or 2. The other `FFE183` writes found in the ROM disassembly
are clears at `0x2A82`, `0xC684`, and `0x1E45E`; no source path writes 3.
Therefore choice 3 is a source-table row, but is unreachable after the field
mode initializes this byte. The candidate rows are not proof that all three
can spawn.

### Tagged-list and scene-selector census

A byte scan of the 2 MiB US ROM finds 74 occurrences of the tagged pedestrian
template longword `80 1D 69 88`. The extracted conditional-candidate manifest
has 69 scene/choice rows and 50 unique list offsets. Directly reading the
three-pointer arrays selected from `0x1EDB88` confirms all 69 manifest
scene/choice pointers match the ROM.

**[C]** The three tagged references at `0x1D9212`, `0x1D921A`, and
`0x1D9222` are the Scene-20 default-list pedestrian entries, not a separate
spawn path. The mixed list begins at `0x1D91DA` and has eight actors (five
corp guards followed by three pedestrians). Each pedestrian template pointer
is followed by its position-list pointer: `0x1D922E`, `0x1D9238`, or
`0x1D923E`. These three records are already included in the 21 rows of
`default-actors.tsv`.

[C] The tagged-pointer scan now reconciles all 74 US-ROM occurrences: 50
unique selected A800 list offsets, 21 default-list pedestrians, the A8AE
story actor, the `0x1E5FA` pending-list actor, and one unselected record at
`0x1EE232`. The groups are disjoint and every occurrence is classified. The
single unselected record is not selected by the A800 table: ROM `0x5BA98` indexes the
outer scene table at `0x1EDB88` by `FFDDC4 - 0x15`, then indexes that scene's
three choices by `FFE183 - 1`. Scene `0x29` (41) points to choice table
`0x1EDC98`, whose entries are `[0x1EE1F6, 0x1EE214, 0x1EE214]`; Scene `0x2A`
(42) points to `0x1EDCA4`, whose entries are
`[0x1EE250, 0x1EE26E, 0x1EE28C]`. **[C]** The selected list at `0x1EE214`
contains one template/position pair and its overrides, then a zero longword at
`0x1EE228`; `0x4466` tests that next longword and returns on zero. The
position data it references begins at `0x1EE22C` and terminates on zero X at
`0x1EE230`. The adjacent tagged record at `0x1EE232` points to position data
at `0x1EE24A` (`0x0780,0x3980`), but no A800 choice selects it, and a
bytewise scan finds no `1E E2 32`, `00 1E E2 32`, or `80 1E E2 32`
reference. **[H]** Another computed or indirect owner remains possible; this
orphan does not establish a general pedestrian spawner.

The other extracted literal pedestrian rows are bounded as follows:
`default-actors.tsv` has 21 rows across scenes 0-5 and 20;
`world-activity-actors.tsv` has the A8AE story actor in Scene 34 and a
pending-list actor at source record `0x1E5FA` in Scene 47; the 857
`dynamic-world-activity-actors.tsv` rows across 56 scenes contain no literal
pedestrian profile, name, or sprite stem. They do contain three dynamic
`entity_kind=11` encounter records: a Rat Shaman and an Indian Warrior in
Scene 1's random-sum-6 sequence at `0x1D6DDE`, and a Rat Shaman in Scene 5's
random-sum-8 sequence at `0x1D6E7A`. Their ROM templates at `0x1D6BD8` and
`0x1D6C22` select spell/item actions, and their positions are resolved from
open screen-edge candidates. This is a different source population from the
fixed `0x1D6988` pedestrian template and its default route lists. **[C]** The
entity-kind number and a close sprite match identify a visual family, not an
ambient-pedestrian role or a particular live actor record. These table counts
do not identify every small sprite in gameplay captures and do not exhaust
generated or computed actor paths. Keep the Section-41 sprite identification
open until a matching scene and live actor record are traced.

### Other direct `FFE120` producers

A scan of the US ROM for the absolute operand `00 FF E1 20`, followed by
instruction checks at each store, finds these pending-list writes:

| ROM address | Queued pointer | Source result |
|---|---:|---|
| `0x2D2C` | `0x1E552` | Five `eye5r 1` records for the mode-13 entry path. |
| `0x16912`, `0x16928`, `0x16946` | `0x16FD4` scene table result | A gated singleton `lone11` script in scenes `0x0A`, `0x0B`, and `0x0D..0x13`; scene `0x0C` has a null table entry. Scene `0x0A` chooses `0x17018` when `FFFBF9==6`, otherwise `0x17034`. |
| `0x16A52` | `0xFFE132` | Builds a temporary list from the separate `FFE162` descriptor pointer and pairs each descriptor with a local callback. Its 857 extracted dynamic actor rows across 56 scenes contain no literal pedestrian profile, name, or sprite stem. |
| `0x2247C`, `0x224EC` | `0x1E5C4` | Three Strike Team/bodyguard records and one kind-11 pedestrian record. |
| `0x22622` | `0x1E628` | Three Strike Team records and one corp guard. This store is reached by the paid Club Penumbra command-`0x60` branch at `0x225D2`, after its fee and story gates. |

The three `0x16FD4` stores are source-script selection, not a per-map generic
pedestrian table: each selected ROM list contains one `lone11` actor with its
own kind, palette, position, and item-action template. The table covers scene
IDs `0x0A..0x13`; the special Scene-20 branch exits before indexing it. The
`0x1E5C4` pending story list is the only one in this census that adds the
shared `0x1D6988` pedestrian template. The `0x1E628` and `0x1E552` lists do not.

This closes the direct absolute-operand producer census for `FFE120`. It does
not establish another general pedestrian spawner: the only additional generic
pedestrian in these queues is the single actor in `0x1E5C4`, while the separate
`0xA800` scene/choice table remains the broad conditional pedestrian source
for scenes `0x15..0x2B`. Indirect writes through a copied pointer and the
identity of the pedestrian-like sprites in Section 41 remain open.

### `FFE180` / `FFE182` story-marker setters

The same US-ROM absolute-operand scan finds one non-clear direct setter for
each marker. `0x53C3C` writes `FFE180=1` from indexed service 3 after its
Scene-3/event-3, story-bit, alternate-result, and group-item gates. That
marker reaches the fixed A8AE Aragorn-contact list only while
`FFF0C8==4`; the list is a story actor with a generic pedestrian template,
not an ambient walker.

`0x53A78` loads FFFC1E into D1 before dispatching indexed services. Service
19 at `0x543A2` requires FFFC1E bit 18 (`RED_BUFFALO_WOMAN`) set and bit 19
(`ILENE_MET`) clear, `FFFBF8==2`, `FFFBF9==0`, `FFF0D5==0`, FFFC34 bit 0
clear, and FFFC38 bit 12 set. It then writes `FFF0B4=0` and `FFE182=0xFF`
at `0x543F8`. On the Scene-20 path, A800 reaches A8CC only when FFE180 is
clear; that fixed list is one `lone11` actor (kind 10, palette 1, item action
3) at `(0x0E80,0x0700)`, with ordered overrides `+0x9E=0x20`,
`+0x2E=0x80`, `+0x9E=0x08`. It is Ilene's encounter actor, not the shared
`0x1D6988` pedestrian template.

Direct clear instructions found for `FFE180` are at `0x2A64`, `0xED7E`, and
`0x1E43C`; direct clears for `FFE182` are at `0x2A6A`, `0xC652`, `0xED9C`,
`0x1E442`, and `0x22796`. This bounds the direct absolute-operand setter and
clear paths; both marker-selected lists are story-specific and do not explain
a general pedestrian spawner.

### Meaning of the `FFFBED == 5` gate

The comparison at `0xA84C` is confirmed, and it reads the post-dispatch value
rather than the raw offer-record byte. ROM `0x1E69C` copies the selected
initializer from `0xF3EBC[FFF0AE]` to `FFFBED`, then indexes `0xF3DC6` and calls
the selected setup routine at `0x1E71C`. Raw initializer 5 uses `0x1E810` and
retains 5. Employee Recruitment variants 11 and 12 use `0x1E81C` and `0x1E7D0`;
their non-9/10 branches write 5 at `0x1E848` and `0x1E804`. Package Recovery
variants 9 and 10 use those same handlers but write 4 at `0x1E83C` and
`0x1E7F8`. Thus the selector's `FFFBED==5` gate admits raw offer initializers
5, 11, and 12, while rejecting 9 and 10.

The Lua run registry retains the raw `source_initializer`. The conditional
selector now models the `0x1E69C`/`0xF3DC6` normalization before applying the
`0xA800` gate, so both Employee Recruitment variants reach the same branch as
initializer 5, while both Package Recovery variants resolve to 4. Candidate
metadata records this normalization instead of treating `FFFBED` as the
untouched offer-record byte.

### Full `0xA800` selection matrix

| First matching condition | Later predicates consulted | Result |
|---|---|---|
| `FFE180 != 0` and `FFF0C8 == 4` | None | Select fixed story-contact list `0xA8AE`; skip `FFE182` and random pedestrians. |
| `FFE180 != 0` and `FFF0C8 != 4` | None | Select no list; return without checking `FFE182`. |
| `FFE180 == 0` and `FFE182 != 0` | None | Select fixed list `0xA8CC`; skip random pedestrians. |
| Both markers clear; `FFFBED != 5` or `FFDF40 != 2` | None | Select no list and do not sample `FFE183`. |
| Both markers clear; initializer and mode gates pass; `FFE183 == 0` | Sample bound 2 and store 1 or 2 before testing the remaining gates. | The choice persists if a later gate fails. |
| Random branch reaches `FFE12C != FFF0C8` or `FFFC3C != 0` | Choice may already have been sampled and stored. | Select no list; retain the choice. |
| Both markers clear; initializer/mode gates pass; floor words equal; `FFFC3C == 0` | Index the scene table using the stored `FFE183` choice. | Select one candidate list; reachable choices are 1 and 2. |

The Debug and Release `shadowrun_gameplay_script_runtime_qa` now enumerate
13,824 combinations through Lua's decision function. The sweep crosses both
effect markers, raw initializer values `nil`, 0, 4, 5, 9, 10, 11, and 12,
`FFDF40` values 0, 1, 2, and `0x0102`, nine absent/equal/mismatched 16-bit
effect-index pairs, both `FFFC3C` states, retained choices 0/1/2, and both
bound-2 rolls. It asserts branch precedence and list identity, checks that only
normalized employee initializer 5 reaches the random branch, and verifies
whether the choice is sampled, retained after late rejection, or left untouched
by the higher-priority story lists. This is selector-state coverage; the
separate field QA supplies the scene-entry, actor-allocation, and contact paths.

The selected pending list is drained before the scene default list at `0x43B6`,
so a candidate consumes an ordinary actor slot first. Keep all three table rows
for provenance, but treat row 3 as unreachable under the ROM's valid
`FFE183` lifecycle.

### FFFC3C player-position spawn lifecycle

FFFC3C stores the 32-bit FFE110 transfer value, which is a tagged actor
template for the A800 lists; it is not a standalone boolean. At `0xED60`, an
eligible `+0x9E` bit-3 actor contacted by an entity with `+0x10` bit 1 copies
FFE110 into FFFC3C, sets its `+0xB0` bit 7, and starts world activity. All 69
conditional-candidate rows use root-tagged pedestrian template
`0x801D6988`, so the native representation can retain a typed actor-template
descriptor without keeping a ROM pointer.

At `0x286E`, the ROM tests the marker, scans thirteen records from `FF0400`
with stride `0x100`, and returns if any record has `+0x10` bit 0 and `+0xB0`
bit 7 set. If no record matches, it passes the exact marker value to
`0x1E4AC`. That routine allocates an ordinary actor slot and effect slot,
initializes from the selected template, sets `+0xB0` bit 7, retains the marker,
and places the actor at Joshua's `x+0x80` / `y`. The `0x2FA4` entry path also
repositions the first existing active/effect-ready record to that location;
`0x1E514` clears FFFC3C when it clears an actor's `+0xB0` bit 7.

The active Qt runtime retains typed FFE110 actor-template data and a typed
FFFC3C marker. A800 receives a derived presence fact, sufficient for its gate.
The pair-collision model publishes whether the typed transfer template is
available; eligible bit-3 contact copies that descriptor into FFFC3C. Lua
contact routing keeps A8AE/A8CC story dialogue while generic 24-bit A800
contact starts the shared activity path without being rejected as an unmapped
story actor. Scene-34 QA checks the pedestrian profile while dialogue is open
and its clear after cleanup.

The live `0x286E` callback maps typed source actor slots 3..9 into the
thirteen-record guard input. **[C]** ROM `0xEB92` bounds the `0xED60` contact
pair to records `FF0400..FF0A00`; `0x1E4AC` allocates through `0x1A342`, which
searches the same slots 3..9. `0x44BC` initializes from the common record at
`0x7361A`, whose `+0xB0` byte is zero, and the ROM's `+0xB0` bit-7 set
instructions are only `0xED60`/`0xEDD2` and `0x1E4AC`. The remaining guard
entries (slots 10..15) cannot carry this flag through those source paths.
When no flagged actor remains, the runtime clones the retained typed transfer
descriptor, places it at Joshua x+`0x80` / y, sets the typed `+0xB0` bit-7
behavior fact, and publishes it through the ordinary Lua actor heartbeat and
slot/effect allocation. The added Scene-34 QA removes the selected A8AE record
while retaining its source descriptor, checks the exact fallback profile,
position, actor slot, and escort flag, captures
`scene-34-player-spawn-fallback-actual.bmp`, then verifies a second guard pass
finds that record instead of duplicating it. The captures are internal Qt
images, not independent Genesis evidence; the Debug and Release fallback
captures have matching SHA-256
`E95E506ACB226E1F43B3D2665401F06A88008A67B5F8F27C9ECA61CFEBCF9FB0`. This
remains a partial hill because source-valid candidate-scene travel and a
route-matched Genesis capture are still open.

## Active runtime boundary

`FieldSceneRuntime::initialize_default_field_actors` now calls
`GameScriptRuntime::resolve_conditional_actor_selection` after the explicit
route-owned lists. It passes the active offer's raw initializer, the current
FFDF40 mode, FFE180/FFE182, a presence projection of the typed FFFC3C template,
and separate optional 16-bit FFE12C and FFF0C8 values. Lua derives the post-`F3DC6` FFFBED
value before checking the A800 initializer gate. The one-shot FFE183 choice
remains Lua-owned. The random
callback uses the field PRNG's bound sampler, and Lua calls it only after the
marker, normalized-initializer, and mode gates pass.

Resource initialization records the selected FFE12C index and initial FFF0C8
word. Elevator entry and completed floor movement update the active floor
word; those typed values survive the routine-state reset during a child-scene
load. If the selector returns A8CC or A8AE, the runtime resolves the extracted
source-list row and queues it before defaults. A8AE is present in
`world-activity-actors.tsv` with the `0x1D6988` template, `0xA8C6` position
record, ordered overrides, pedestrian profile, and Scene-34 context. For a
random result, the runtime joins the selected candidate row to the matching
kind-11 template facts and applies its ordered spawn-flag overrides before
using the same ordinary actor publisher.

The focused Debug/Release `field_pedestrian_scenes_qa` checks the ordinary
Scene-1 and Scene-3 default pedestrians and the Scene-5 row-1 route. For Scene
5, it asserts the `0x679F0` actor starts at `(0x4B80,0x2200)` and moves closer
to its first waypoint `(0x4B80,0x3980)`, then saves
`qt_runtime/field-pedestrian-scenes-qa/scene-5-actual.bmp`; these Debug/Release
images are pixel-identical. The test then seeds the high-priority story marker
and floor index 4. It verifies one A8AE actor at `(0x1D00,0x4100)` in
actor slot 3, checks the profile and draw visibility, and writes
`qt_runtime/field-pedestrian-scenes-qa/scene-34-story-contact-actual.bmp` in
each build. The Debug and Release BMPs have the same SHA-256. This test
exercises the selector-to-actor projection and internal rendering; it does
not establish that the real story route sets those facts correctly.
Gameplay-script QA drives accepted offers through Johnson's work menu for
F3EBC context 1 zero-based slots 2 and 4 (raw initializers 5 and 11) and
context 4 slot 4 (raw initializer 12). It checks their source negotiation
and target draws, then confirms all three reach the normalized `0xA800`
branch and typed `0x1E43C` request. Selector coverage rejects package-recovery
variants 9/10 after their normalization to 4 and covers both reachable rolls,
late-gate choice retention, and unequal/equal 16-bit effect indices.

`world_effect_active` still initializes false, but the Scene-3/event-3 entry
graph sets it true after the `0xF4BB4` page. The ordinary-input route now walks
record 95, enters Scene 30, uses the lift to reach Scene 34 at active index 4,
and verifies the A8AE actor at `(0x1D00,0x4100)`. The A8AE contact path is now
connected: the field runtime forwards the source list and selected ECCC handler
to Lua, which chooses the contact graph only for list `0xA8AE` with FFE180
priority. The graph consumes FFE180, sets FFFC1E bit 3, clears FFFC30 bits
10-12 and sets bit 13, clears FFFC38 bits 4-5, then presents the three records
from `0xF4BBC` with portrait profile `0x121E08`. Debug/Release QA walks through
the unlocked record-90 cell to the actor, advances all three pages with A,
checks the flags and actor one-shot bit, and writes
`scene-34-aragorn-contact-{actual,reply,outcome}.bmp`; the opening-page images
have matching SHA-256 hashes. At contact, the player is at `(0x1D26,0x40CE)`
and the actor at `(0x1D00,0x4100)`. Cleanup removes Aragorn's source slot and
reduces the active Scene-34 activity count by one while other records remain.
The `0x1E45E` FFE183 clear is
connected at the `0x0A92` request callback: Lua returns and clears the choice
atomically, and the callback passes that byte into the typed 13-record
`0x1E43C` scan.

Gameplay-script QA follows Matchstick's work menu to accepted initializers 5
and 11, and Caleb's Icarus work menu to accepted initializer 12. Field QA now
reaches those same offers through ordinary input in a single
`FieldSceneRuntime`: Scene 18 starts from its source player record and walks
Up, Left, Right, then Up into record 9/event 30; Scene 12 starts at the
documented `(16,80)` checkpoint and walks Up into Caleb's event 30. B selects
Work; A advances the source work page and generated offer text. Julius's and
Caleb's source text are asserted before acceptance. The tests seed the source
offer-selection random draw only, leaving subsequent negotiation and target
draws live. Caleb cases bootstrap the same runtime at Scene 12's source
checkpoint before the contact; the accepted offer then remains in that runtime
through candidate resource entry, with no `run_registry` save/restore.
Scene 22 choices 1 and 2 use initializers 11 and 12; other cases use 5.
All 46 reachable scene/choice pairs write candidate images and record their
initializer. Each case also checks that its candidate takes slot 3 before the
scene's complete source-ordered default guard list, that every guard remains
active in slots 4 onward, and that no extra default row is active. The expected
guard counts come from `assets/field/entities/default-actor-lists.tsv`; the
list drain order is ROM `0x43B6`, with the seven ordinary records allocated by
`0x1A342`. Debug/Release candidate BMP hashes match. Root, work-opening, and
generated-offer captures for all three initializers are also saved under
`qt_runtime/field-pedestrian-scenes-qa/johnson-offer-*`. They are internal
runtime captures, not independent Genesis reference images. This closes the
physical Johnson-to-accepted-offer segment, while these 46 candidate cases
still use resource-scene reentry instead of physically traveling to each
candidate scene. The index-mismatch cases start Scene 30 at the ROM profile
checkpoint `(0x3A00,0x0600)` with `FFE12C=4` and `FFF0C8=0`, then ordinary Up
reaches event 29 records 82/83 at `0x1DB032`. The selector group at `0x1DB004`
is `[30,31,32,33,34]`; ordinary Up selects floor index 4, A runs the
source-timed lift for 17x8 frames per floor, and Start completes the trip.
For choice 1, source seed `0x0000` and destination seed `0x8000` publish list
`0x1EDE18` at `(0x1800,0x0F00)`. For choice 2, source seed `0x8000` and
destination seed `0x0000` publish list `0x1EDE36` at `(0x0380,0x4880)`.
Both use profile `pedestrian_k0b_none00` in slot 3. Focused Debug and Release
CTest pass; all 16 generated artifacts (14 BMPs and two TSVs) match by
SHA-256. I inspected
the selector and both centered actor frames; these are internal captures, not
independent Genesis references. Both routes start from a staged profile
checkpoint and do not carry the accepted offer to the lift. The separate
`FFFC3C` case samples choice 2 while player-spawn is
pending, then publishes that choice in Scene 34 after clearing the pending
flag through resource entry. The other matrix cases also use resource entry.
No independent
Genesis capture identifies this candidate,
Scene 34, the A8AE actor, or the contact conversation;
the BMPs remain internal runtime regression evidence only.

## Independent capture review

The independent longplay dump in
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_41_source_10620-10815/`
contains these inspected frames:

| Frame | Capture time | Observation |
|---|---|---|
| `frame_001200.png` | `02:57:40.000` | Field map matches Scene 0; blue and brown sprites are visible, but their identities are not confirmed. |
| `frame_001500.png` | `02:57:50.000` | Lone Star badge portrait; “Hey, Flick. I think this guy's packing. You got a permit for that illegal weapon?”; choices `Attack!!`, `Talk to them.`, `Run away!`. |
| `frame_001800.png` | `02:58:00.000` | Black transition frame. |
| `frame_002100.png` | `02:58:10.000` | Field map matches Scene 5; blue and brown field sprites are visible. |
| `frame_002110.png` | `02:58:10.333` | Another sprite is visible at the right edge while the camera follows the party. |
| `frame_002120.png` | `02:58:10.667` | Scene 5 map and actor positions shift with the camera. |
| `frame_002140.png` | `02:58:11.333` | Several sprites remain in view during movement. |
| `frame_002150.png` | `02:58:11.667` | The sprites overlap near the center of the field. |
| `frame_002400.png` | `02:58:20.000` | Taxi asks for 40 yen to travel to the Puyallup Barrens, with accept and cancel choices. |
| `frame_002700.png` | `02:58:30.000` | Field map matches Scene 1; the red/brown sprite near machinery is not identified. |
| `frame_003000.png` | `02:58:40.000` | Tarislar City Inn introduction dialogue and portrait. |

I matched the 272x224 gameplay viewport, excluding the right HUD, against all
56 ROM-derived map images in `assets/field/maps`. The highest normalized
correlation scores and map offsets are:

| Frame | Scene/profile | Score | Map offset (pixels) |
|---|---|---:|---:|
| `frame_001200.png` | Scene 0 / `0x0D5456` | 0.9069 | `(678,736)` |
| `frame_002100.png` | Scene 5 / `0x06796A` | 0.9082 | `(895,800)` |
| `frame_002120.png` | Scene 5 / `0x06796A` | 0.8757 | `(955,790)` |
| `frame_002150.png` | Scene 5 / `0x06796A` | 0.8790 | `(1070,782)` |
| `frame_002700.png` | Scene 1 / `0x17D5C4` | 0.9609 | `(1285,708)` |

The map offsets also follow the moving background geometry seen in the
independent frames. This identifies the visible field maps; it does not identify
the other sprites or provide input/event telemetry. The sequence shows Scene 0
before the prompt/black interval, Scene 5 after it, and Scene 1 after the later
taxi prompt. It does not establish which input caused either transition.

The screen in `frame_001500` exactly matches the follow-up prompt record at
`0x120E98`. Its text pointer is `0x11D48C`, whose ROM text reads “Hey, Flick.
I think this guy's packing. You got a permit for that illegal weapon?” Its
choice commands are Attack `0x95`, Talk `0x97`, and Run `0x99`, matching the
three visible labels. The record shares the three resource pointers at
`0x103A9A`, `0x103AA5`, and `0x103AEB` with the initial patrol record at
`0x120E54`, but its Talk command differs: the initial prompt uses `0x96`.

For event `0x20F12`, the initial prompt at `0x120E54` is enabled in scenes 0
and 3 at selector sums 3 and 4. Choosing Talk (`0x96`) enters the shared
compliance handler at `0x23452`. After its special-pass check, `0x23482`
samples a bound-12 value, subtracts heat `FFFBFA`, and compares the result with
the live field `FF017A`. On the failed check, `0x234B4` tests the heat cutoff:
heat 3 or higher returns the suspect-ID prompt at `0x120EB4`; lower heat enters
the item scan at `0x234C8`. The scan starts at `0xFF0100` and can inspect up to
three `0x100`-byte runner blocks. It tests a block's eight item slots only
when bit 0 at `+0x10` is set; a clear bit skips that block's item scan and the
code does not advance to the next block. The item bytes are compared with
`0x08`, `0x09`, `0x0A`, `0x0C`, `0x0D`, `0x14`, `0x31`, or `0x32`. A match
branches to `0x23542`, which returns `0x120E98`; no match returns the release
prompt at `0x120ED0` and lowers heat. The Talk choice `0x97` from the captured prompt enters
`0x2354A`, where the next branch checks weapon-permit bit 11 of `FFFC22`.

This corrects the source path in the terminal-outcomes table: `0x234C8` starts
the inventory scan; it does not load the prompt. Event `0x20F12` is one
confirmed route to this exact follow-up, but the same compliance handler is
shared by other Lone Star events in the table. The capture identifies the
dialogue record and state branch, not which event or scene initiated it.

The direct Scene-41-to-43 patrol event is a different presentation. Handler
`0x2106A` loads prompt `0x1213CC` and bundle `0x122048`; the prompt has one
MORE command `0xD9` to `0x24622`, not the three Attack/Talk/Run commands shown
in `frame_001500`. Its source outcome is “A Lone Star patrol spots you.” /
“Hold it right there!” at selector sum 5. Therefore this capture does not
identify the direct Scene-41-to-43 patrol branch, despite being in the same
video section as the later field frames.

For the three-option prompt family, the traced input branches have different
actor and state outcomes: Attack can start the patrol list and raise heat by
2 (capped at 8), or display the radio-call exit if the actor sequence fails;
Talk can release the player, continue into ID or illegal-weapon/permit checks,
or reach arrest depending on flags, rolls, inventory, and heat; Run raises heat
by 1 (capped at 8) and can produce a detective encounter, a mixed pursuit, or
an escape according to the chase roll. The terminal branches are recorded in
`decompilation/assets/field/world-activity-terminal-outcomes.tsv` under
`0x20F12`. The capture shows the choice screen only; it does not reveal which
input was selected or any resulting branch. `FF017A`, heat, active-runner
inventory, and the permit flag remain live source inputs; the frame does not
identify their values or the runner's class/race profile.

Frames `002140` through `002170` provide a route-level match for the brown
walker. The unobscured 240x58 top strip in those frames matches the extracted
mode-5 map at camera offsets `(1028,782)`, `(1070,782)`, `(1110,782)`, and
`(1148,782)` respectively; the first match scores 0.977 normalized
correlation. After subtracting those offsets and accounting for the sprite's
foot anchor, the walker stays near the map's `y=920` route band and moves west
as the camera advances. Scene 5's default list at `0x679F0`, row 1 at position
record `0x67A30`, starts at `(19328,8704)` and gives waypoints
`(19328,14720)` then `(6656,14720)`: in map pixels, `(1208,544)` to
`(1208,920)` then west to `(416,920)`. The captured westbound segment agrees
with that pedestrian route;
the earlier southbound leg is not visible in this excerpt. It is separate
from the `0xA800` randomized candidate lists. [C] The map and route
match are visible in the independent frames and present in the source data.
[H] The footage has no actor-record telemetry, so the exact live record and
party membership of the other sprites are not directly exposed. This does not
tie the walker causally to the earlier Lone Star prompt in the same section.
The same contact sheet shows
Tarislar City Inn dialogue later in Section 41; it does not show the
Mitsuhama after-hours page or identify Scene 30/34. The new
`field-mitsuhama-after-hours-entry-qa/actual.bmp`, `scene-30-entry.bmp`, and
`scene-34-aragorn-contact-{approach,actual,reply,outcome,closed}.bmp` are
internal regression captures only. I inspected the contact opening, reply,
outcome, and closed field frames: the extracted text, MORE prompts, 56x72
portrait, three-page sequence, and return to the field are visible. All 22
Scene-34 QA BMPs have matching Debug/Release hashes. These images are not an
independent Genesis visual match.

### Scene-0 Downtown pedestrian route review (2026-10-27)

I checked the lower brown walker in the independent Section-41 frames against
the Scene-0 map and default actor routes. Each 272x224 gameplay crop was
registered to `mode-0.bmp` with grayscale normalized cross-correlation; the
walker screen X positions are visual estimates, so the resulting map X values
are approximate:

| Longplay frame | Map offset | Walker screen X | Approximate map X |
|---:|---:|---:|---:|
| `001320` | `(1129,622)` | 262 | 1391 |
| `001330` | `(1167,622)` | 236 | 1403 |
| `001340` | `(1209,622)` | 206 | 1415 |
| `001350` | `(1249,622)` | 178 | 1427 |
| `001360` | `(1287,622)` | 152 | 1439 |
| `001370` | `(1329,622)` | 121 | 1450 |

Across these 50 dumped images the map camera advances east by about 200 map
pixels, while the walker advances east by about 59 pixels in map coordinates;
its road position stays near the `y=800` band. This agrees with the horizontal
`(1304,800)` to `(1600,800)` segment in Scene-0's default list at `0xD5538`.
All three `pedestrian_k0b_none00` route records share that segment: source
orders 0, 1, and 2 use position lists `0xD556C`, `0xD5584`, and `0xD5594`,
respectively. **[C]** The segment is directly present in those ROM-derived
coordinate lists. **[I]** The independently captured lower walker is moving
along that segment, separate from the faster screen-centered figures whose
movement is consistent with the camera-followed party. It is a route-level
match for the ordinary Scene-0 population, separate from the stationary A800
contact candidates.

The frames have no input or actor-record telemetry. The shared route segment
does not identify which of the three source records produced this walker, and
the visual match does not establish its exact frame cadence or collision
state. I also excluded the red/brown figure near center in Scene-1 frame
`002700` from the pedestrian count. It is at approximately `(1411,817)`, near
source-order 2's `(1408,784)` to `(1408,952)` route, but by frame `002730` it
is at approximately `(1353,700)`, off that route. Its screen position stays
near center while the map offset changes from `(1285,708)` to `(1227,588)`,
which is consistent with the tracked party; the first frame alone would have
been a false positive. I checked the intervening Scene-1 frames every ten
captures through frame `002750` against `mode-1.bmp`:

| Frame | Map offset | Match | Approximate center figure map X |
|---:|---:|---:|---:|
| `002700` | `(1285,708)` | 0.97842 | 1411 |
| `002710` | `(1285,670)` | 0.95595 | 1411 |
| `002720` | `(1267,628)` | 0.94525 | 1393 |
| `002730` | `(1227,588)` | 0.92172 | 1353 |
| `002740` | `(1219,550)` | 0.91589 | 1345 |
| `002750` | `(1201,508)` | 0.95255 | 1327 |

Offsets are full 272x224 grayscale `TM_CCORR_NORMED` matches; the figure's
screen center is visually estimated near x=126. The ROM route at position list
`0x17D694` reaches the vertical segment `(1408,784)->(1408,952)`. Although
frame `002700` falls near that segment, the center figure then moves away in
map X while remaining near screen center. Both runner portrait panels are
visible in this sequence. **[I]** Its camera-relative track and movement away
from the source waypoint direction support treating it as party movement, not
as evidence of a pedestrian following this segment; the capture does not
identify the exact live actor record.

## Section 36 Mitsuhama map and moving sprite review

Independent capture: `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_36_source_9120-9420/frame_008340.png`.
The 320x224 viewport registers to both `mode-32.bmp` and `mode-33.bmp` at map
offset `(388,161)`. A grayscale `TM_CCORR_NORMED` comparison of the left
268x224 pixels, masking the sidebar from x=260 and the figure at x=34..63,
y=38..81, scores 0.97534. The two map assets have the same SHA-256
(`2bd7ef348da583943d1f7be8ce278a08f3b7ea7aefab51c2ea41896833ca9687`), so
this image cannot distinguish Scene 32 from Scene 33.

ROM `0x5A62` projects an actor from its world position relative to camera
words `FFDDC6/FFDDC8`, shifts by four, and adds the 128-pixel viewport center.
Using the map registration and the visible figure's foot area gives an
approximate map position `(436,238)` at frame 8340. This is a screen-derived
position, not a live RAM readout. The source rows for either scene put the
three `0xA800` pedestrian choices at `(0x0500,0x1F00)`,
`(0x2500,0x4F00)`, and `(0x3A00,0x1500)` in 16.4 world coordinates, or map
pixels `(80,496)`, `(592,1264)`, and `(928,336)`. The default list at
`0x1DBBE8` has two `corp_guard_k05_item06` rows at `(0x0400,0x2700)`, or
`(64,624)`. These are source initial positions; the frame's different
position does not rule out an actor that has moved since spawn.

The image does not expose the live selector gates. At `0xA800`, a set `FFE180`
story marker wins when its active effect index is 4; otherwise `FFE182` takes
priority over the randomized pedestrian branch. That branch requires
`FFFBED==5` and `FFDF40==2`, records a fresh `FFE183` choice from 1 or 2, then
checks `FFE12C==FFF0C8` and `FFFC3C==0` before publishing. `0x43B6` drains
that pending list before the scene defaults. The source table has a third
candidate row, but a fresh random draw does not select it; a nonzero preset
`FFE183` can still index it. None of these RAM values can be read from this
video frame.

The initial broad match treated a red/brown sprite crop as one moving figure
across these nearby captures. The adjacent-frame review below rejects that
continuity assumption; retain these as per-frame detections only. Their
approximate screen centers and map-coordinate estimates are:

| Longplay frame | Map offset | Match | Detected sprite center | Per-frame map estimate |
|---|---:|---:|---:|---:|
| `frame_008300.png` | `(262,138)` | `0.97151` | `(115,95)` | `(377,233)` |
| `frame_008340.png` | `(388,161)` | `0.98817` | `(49,61)` | `(437,222)` |
| `frame_008370.png` | `(449,74)` | `0.95782` | `(141,109)` | `(590,183)` |
| `frame_008400.png` | `(546,20)` | `0.98338` | `(117,100)` | `(663,120)` |
| `frame_008430.png` | `(666,39)` | `0.95367` | `(112,100)` | `(778,139)` |
| `frame_008460.png` | `(720,1)` | `0.96946` | `(170,89)` | `(890,90)` |

These nearby-frame offsets use the same broad mask, with the sidebar and center
of the viewport excluded; the listed centers are visual estimates. A follow-up
inspection of adjacent full frames `008330` through `008380` shows several
sprites at once. The earlier six-center series does not track one identified
sprite: frame `008340`'s outlier is a different visible body from the central
mover in most other samples. Do not use the derived six-point map series as one
actor's route.

A masked BGR `TM_CCORR_NORMED` search against the ROM-derived idle/walk assets
separates the two visual hits. In frame `008340`, the small sprite at
`(39,49)` matches `pedestrian_k0b_none00` idle direction 2 or 6 at `0.9498`;
the best kind-3 palette-3 runner match at that location is `0.7146`. For the
central mover in the other sampled frames, the best local matches are kind-3
palette-3 runner poses: frame `008300` walk direction 2/frame 2 at `0.8408`,
`008370` walk direction 1/frame 2 at `0.8880`, `008400` walk direction
2/frame 1 at `0.8870`, `008430` idle direction 2 at `0.8944`, and `008460`
walk direction 2/frame 3 at `0.8824`. These are local masked-template scores,
not actor-record identifications; the matching art is extracted from the
kind-3 template at ROM `0x7291A` and the kind-11 template at `0x1D6988`.

**[C]** Frame `008340` contains a sprite visually matching the kind-11
pedestrian art. **[H]** Its source record, party membership, and producer remain
unknown. The central mover's better repeated kind-3 match supports a controlled
runner or party-runner interpretation, but the capture has no RAM or input
telemetry. The source Scene-32/33 default rows are kind-5 guards on the patrol
at `0x1DBBFC`; their route band does not match the small frame-`008340` sprite's
estimated position. The conditional A800 rows for these scenes have only an
initial coordinate before the zero-X terminator, so this sequence does not
show one of those candidates walking its source route. Keep the sprite's
producer unresolved; the capture shows a pedestrian-art match, not a confirmed
ambient-pedestrian spawn or a source-path match. No runtime behavior changed;
the population hill remains partial.

### Adjacent-frame kind-11 movement track (2026-10-28)

A focused recheck follows that small sprite through 17 samples from
`frame_008330.png` to `frame_008362.png`, two capture frames apart. Each 272x224
gameplay crop was aligned to the shared Scene-32/33 map with grayscale
`TM_CCORR_NORMED`; the fixed mask removes the sidebar and the earlier target
area. A masked BGR `TM_CCORR_NORMED` search then compares the left-side actor
area against the extracted idle/walk frames of ROM kind-11 template
`0x1D6988`. Map offsets and template matches stay spatially continuous, with
map scores `0.9377..0.9858` and sprite scores `0.9252..0.9498`:

**[C]** In all 17 samples, one left-side sprite matches the extracted kind-11
pedestrian walk art. **[I]** The repeated pose progression and smooth projected
foot path support a single actor moving east across the sequence. The actor
search uses crop `x=0..109`, `y=25..144` and masks black template pixels; foot
estimates use each matched frame's visible bottom-center.

| Longplay frame | Map offset / score | Sprite origin | Best walk frame / score | Approximate map foot |
|---:|---:|---:|---|---:|
| 008330 | `(368,134)` / 0.9837 | `(22,76)` | dir 2/6, frame 1 / 0.9301 | `(398,233)` |
| 008332 | `(371,137)` / 0.9858 | `(25,73)` | dir 2/6, frame 1 / 0.9387 | `(405,233)` |
| 008334 | `(376,143)` / 0.9851 | `(30,67)` | dir 2/6, frame 2 / 0.9252 | `(416,234)` |
| 008336 | `(378,147)` / 0.9800 | `(34,63)` | dir 2/6, frame 2 / 0.9266 | `(422,234)` |
| 008338 | `(381,153)` / 0.9805 | `(37,57)` | dir 2/6, frame 3 / 0.9476 | `(426,234)` |
| 008340 | `(388,161)` / 0.9750 | `(39,49)` | dir 2/6, frame 3 / 0.9498 | `(435,234)` |
| 008342 | `(396,161)` / 0.9667 | `(36,49)` | dir 2/6, frame 0 / 0.9392 | `(444,234)` |
| 008344 | `(402,161)` / 0.9605 | `(35,49)` | dir 2/6, frame 0 / 0.9412 | `(449,234)` |
| 008346 | `(412,161)` / 0.9481 | `(37,49)` | dir 2/6, frame 1 / 0.9460 | `(458,233)` |
| 008348 | `(420,157)` / 0.9425 | `(36,53)` | dir 2/6, frame 1 / 0.9355 | `(464,233)` |
| 008350 | `(426,154)` / 0.9385 | `(36,56)` | dir 2/6, frame 1 / 0.9303 | `(470,233)` |
| 008352 | `(432,146)` / 0.9377 | `(40,64)` | dir 2/6, frame 2 / 0.9401 | `(482,234)` |
| 008354 | `(432,138)` / 0.9378 | `(48,72)` | dir 2/6, frame 3 / 0.9352 | `(488,234)` |
| 008356 | `(432,132)` / 0.9381 | `(53,78)` | dir 2/6, frame 3 / 0.9428 | `(493,234)` |
| 008358 | `(432,122)` / 0.9411 | `(56,86)` | dir 2/6, frame 3 / 0.9338 | `(496,232)` |
| 008360 | `(432,114)` / 0.9453 | `(53,97)` | dir 2/6, frame 0 / 0.9255 | `(497,235)` |
| 008362 | `(432,108)` / 0.9484 | `(54,103)` | dir 2/6, frame 0 / 0.9276 | `(498,235)` |

The source projection path at `0x5A62` relates map/camera position to sprite
placement. These screen-derived foot estimates move about 100 map pixels in
positive X while staying within Y 232..235. The kind-11 direction-2 and
direction-6 images are pixel-identical, so the capture does not reveal facing.
The search switches to a different, lower-scoring sprite at frame `008364`; the
track stops at `008362` rather than joining that hit to this actor. The older
six-sample central-mover series remains mixed and is not reused as this track.

This establishes that the independent sequence contains a continuously moving
sprite whose art matches the ROM pedestrian template. The measured path band
does not match the Scene-32/33 default guard patrols at map Y 624/1168 or the
A800 candidate initial coordinates `(80,496)`, `(592,1264)`, and `(928,336)`.
Those source records and lists are confirmed by `0x1DBBFC` and the Scene-32/33
A800 rows at `0x1EDE72`, `0x1EDE90`, and `0x1EDEAE`; their position pairs are
`0x1EDE8A`, `0x1EDEA8`, and `0x1EDEC6`. They do not identify this live sprite.
**[H]** Another story or computed actor path, or a later action retargeting a
candidate, remains possible. With no frame-linked RAM or input log, this track
does not prove its actor record or a general pedestrian spawner. It supersedes
the earlier per-frame-only conclusion for this one sprite; the population hill
remains partial.

### Scene-32/33 actor-source inventory

The map asset is shared by Scenes 32 and 33, and the decoded source rows use
the same actor tables in both cases. A ROM check of the known default, alarm,
dynamic-activity, and conditional-candidate lists narrows the path question:

| Source path | ROM data | Relevant actor and position evidence |
|---|---|---|
| Default scene list | `0x1DBBE8`, positions at `0x1DBBFC` | Two kind-5 corp guards share the rectangle `(64,624) -> (64,1168) -> (928,1168) -> (928,624) -> (64,624)`. |
| Scene alarm effect | Scene profiles `0x1DBA94` and `0x1DC130` select list `0x1DBB10` | Four kind-5 corp-guard rows use position lists `0x1DBB34`, `0x1DBB4A`, and `0x1DBB5C`. The latter two cover map bands Y 80..160 and 720..1056; `0x1DBB34` is the default guard rectangle. The associated source checks are at `0x1DFC4` and `0x1E04A`. |
| Dynamic activity selections | Selection cells `0x1D7304` and `0x1D731C`; lists `0x1D6EF8`, `0x1D6EF4`, and `0x1E20A` | The Scene-32/33 decoded rows are kind-5 `guard1` or corp guards; none uses the kind-11 pedestrian template. |
| Conditional A800 list | `0x1EDE72`, `0x1EDE90`, and `0x1EDEAE` | Three kind-11 candidates start at `(80,496)`, `(592,1264)`, and `(928,336)`. Their position lists end after the initial pair. Each ordered override sequence ends with `+0x9E=0x08`. The fresh random branch selects choice 1 or 2; the third table row is not selected by that draw. |

The default and alarm records therefore do not match the observed Y band
232..235, and the dynamic activity rows use guard art. Among these decoded
lists, the A800 rows are the only kind-11 records. ROM `0x44BC` applies each
override pair as a byte write at `0x4608..0x4610` when the record's `+8`
marker is 2. For these candidates the first `+0x9E=0x20` write is later
overwritten by `+0x9E=0x08`. This is distinct from the rendered kind-11 sprite
identity. The normal pedestrian flee branch
at `0x13904` first returns when `+0x9E` bit 3 is set (`0x1393C..0x13946`);
only then does it test pedestrian bit 5 and nonzero `FFDF84` at
`0x1394A..0x1395A`. That latter branch calls `0x13E8C/0x13E9E` to set the flee
target before `0x145A2` moves toward it. The A800 records' final role byte
takes the earlier return, so they cannot flee through this source path. Their
zero-X route terminators also select the initial coordinate again, which is
already reached when no other source action changes the target. Thus none of
the decoded A800 rows explains this moving track through its ordinary route or
actor-service behavior. A later write to the role/target or an unclassified
actor source remains possible; the footage has no live record or input
telemetry.

## Regional default pedestrian population and route coverage

The source scene profiles and extracted area manifests distinguish the six
large region maps from the later building/resource scenes: Scene 0 Downtown,
Scene 1 Puyallup, Scene 2 Barrens, Scene 3 Penumbra, Scene 4 Council Island,
and Scene 5 Renraku. `default-actor-lists.tsv` has three
`pedestrian_k0b_none00` rows in each region map. `default-actor-waypoints.tsv`
resolves their `+0xE0` route pointers. The table below counts runtime
waypoints after the initial spawn coordinate; the generated Lua stores that
coordinate separately as `initial`:

| Scene | Region | Source list | Pedestrian waypoint counts by source order |
|---:|---|---:|---|
| 0 | Downtown | `0xD5538` | 19, 13, 9 |
| 1 | Puyallup | `0x17D660` | 10, 4, 7 |
| 2 | Barrens | `0x172D1C` | 11, 7, 3 |
| 3 | Penumbra | `0x18A73A` | 8, 2, 7 |
| 4 | Council Island | `0x15B998` | 17, 11, 7 |
| 5 | Renraku | `0x679F0` | 8, 5, 2 |

All 18 region-map route lists have nonzero waypoint counts. The selector at
`0xA800` is a different population: direct decoding of all 69 candidate
coordinate lists in scenes `0x15..0x2B` finds the zero-X terminator immediately
after each initial coordinate pair, so none carries a patrol waypoint. This
fits the gated employee-contact path and does not make those rows a general
walker spawner. The ROM's movement path explains why: `0x44BC` copies the
initial `+0xE0` coordinate into both the actor's live position and target
`+0x96/+0x98`. On the tagged-record branch, `0x142AE` checks that target with
`0x144E8`; once it is reached, the routine reads the next `+0xE0` pair. Every
A800 list has a zero-X terminator there, so `0x142AE` clears cursor `+0xE4`
and selects the initial pair again. The selected target is still the actor's
spawn position, so the close-range check reports it reached and does not
install the `0xB574` walk continuation through `0x145A2`. The current Lua
`apply_patrol` path represents the same loop: an empty point list resets the
target to `initial`, while the zero-distance check leaves movement inactive.
These entries are stationary contacts unless another source action changes
their target. The six regional default lists have separate nonempty routes.

The new
`shadowrun_ghidra_conversion_qt_field_region_map_pedestrians_qa` checks each
map via the QA scene-reentry helper in a fresh field runtime. This does not
claim physical travel between the region maps. For every source order it
asserts the extracted spawn coordinate, pedestrian profile, actor slots 3-5,
and visibility. Ordinary field updates must move all three actors. The QA writes
`pedestrian-routes.tsv` plus a full-screen `scene-0` through `scene-5`
`actual.bmp`. The Debug and Release CTests pass, and all six corresponding
screenshots are byte-identical across configurations. I inspected the six
screenshots; each shows the expected region-map artwork and a visible pedestrian
near the source actor route. They are internal runtime captures, not independent
Genesis comparisons. This confirms the source list/model/runtime path covers
all six regional maps. The pure-Lua `ai_think_tests` now advances all 18
regional pedestrian records through every extracted target, the zero-X wrap
back to each spawn pair, and the first target after wrap; the 186 reached-point
transitions pass. That checks waypoint selection and cursor progression at
source-coordinate arrivals, not travel time or collision along each segment.
Exact patrol waypoint cadence, collision behavior for every segment, and actor
identity in the independent captures remain open.

## Next source-backed slice

The post-page effect is now traced: `0x53C90` calls `0x1DED8`, which scans
`FF0400..FF0A00` and dispatches active actor records with `+0xB0` bit 7 to
`0x1E514`; that handler clears the escort and active bits. The Lua action queues
this source-record cleanup, and the host consumes it before the modal return
fade. Debug/Release QA verifies Aragorn's source slot is removed and saves a
closed Scene-34 BMP. The route captures are internal runtime evidence, not an
independent Genesis animation or map comparison.

The Scene-34 Electronics branch matrix now covers Samurai no-skill, ordinary
Decker no-kit partial success, Decker zero-success alert, and Decker kit
success. The physical office-to-accepted-offer segment now passes for Matchstick
and Caleb in the same runtime. The index-mismatch late-gate branch now uses the
physical Scene-30-to-34 lift case; FFFC3C still uses resource entry. The
accepted offer then reaches candidate
resource scenes through QA reentry; one separate late-gate case now covers a
physical Scene-30-to-34 lift trip from the ROM profile checkpoint. The
accepted-offer-to-lift route and travel to the other candidate destinations
remain open. Continue from that source transition and resume
the pedestrian producer census across street scenes, matching candidate
records to independent longplay frames. The current Scene-34 BMPs remain
internal regression evidence and do not identify a live actor in the longplay.
The pedestrian population hill remains partial.

## Scene-30 and Scene-38 A800 physical contact

### Resource-reentry boundary

**[C]** `reenter_field_scene_for_dump` is a QA-only resource reload. For a
different scene it calls `prepare_runtime_field_scene` directly and does not
assign the controlled runner's position. By contrast,
`transition_to_field_destination` installs its requested spawn or the
destination profile's first field record after preparing the map; the ROM
profile reload path is `0x16A8E -> 0x03074 -> 0x03084`, with pending runner
coordinates handled by `0x030CC..0x0312E`. Therefore the Scene-30 and Scene-38
contact walks below begin at the position retained from the preceding QA
context, not a source-proven arrival point in those maps. They verify candidate
contact, callback, text, portrait, and dismissal behavior once the test places
the actor and runner in the same prepared scene. They do not verify map entry,
reachability from a normal route, or that the player would encounter these
actors during ordinary travel. No-route results from the same resource-reentry
setup likewise cannot establish that a candidate is unreachable from the
game's real entry point. The position mismatch is directly visible in the US
ROM profiles: Scene 30 profile `0x1DAEB8` has first field record
`(0x3A00,0x0600)` at `+0x42`, while Scene 38 profile `0x1DD100` has
`(0x0000,0x0080)` there. Both contact QA cases instead carry `(4906,1156)` into
the reloaded map. The zero first records in profiles such as Scenes 22 and 23
also need their incoming route traced before they can be treated as usable
arrival points.

**[C]** The extracted US-ROM candidate row in
`assets/field/entities/conditional-actor-candidates.tsv` selects Scene 30,
choice 2, list `0x1EDDDC`, at `(0x0900,0x0B00)`. The source selector at
`0xA800` samples choices 1 or 2 through `0x0D3A`; field QA seeds the existing
choice-2 input `0x8000` while leaving subsequent gameplay services live.
`0x43B6` drains the pending A800 row before the scene's default guards.

Debug and Release `shadowrun_ghidra_conversion_qt_field_pedestrian_scenes_qa`
now exercises ordinary field contact for that candidate. The setup accepts
the Johnson offer in the same `FieldSceneRuntime` and uses the matrix's
resource-scene reentry; the candidate itself is not reached by walking from
Johnson. After reentry, the carried runner position is `(4906,1156)`. The QA
releases the host modal-close input for 59 neutral frames, then uses ordinary
Left/Down input. Contact occurs on frame 141 at runner `(0x08DA,0x0AD8)` and
actor `(0x0900,0x0B00)`, a per-axis delta of `(-0x26,-0x28)`. The route planner
uses the imported terrain collision model only to choose waypoints; the runner
position, collision response, input handling, and actor contact are processed
by the live runtime. This is the contact-mechanics check described above, not
an arrival route.

**[C]** On contact, the candidate's `+0xB0` bit 7 is set, FFFC3C retains the
typed FFE110 pedestrian transfer, and the player-spawn guard reports an
existing effect rather than creating a duplicate. ROM `0xED60` installs the
generic callback `0x1F146` for this contact. The callback sets FFFC1C bit 0,
loads A0 from `0xFDE3E` and A5 with portrait profile `0x121FF4`, then reaches
the text pointer `0xFE1F7`. Its next longword is the no-options sentinel;
`0x1F3C4` waits `0x1E` VBlanks before the `0xD94` controller-event wait.

Lua routes the reachable candidate row to
`run_employee_recruitment.present_random_contact`, which presents the source
text `Hey! There you are.` followed by `Let's go. My new employer is waiting.`
with the registered Mortimer Reed profile and portrait. The Qt field runtime
accepts A after the source-timed lockout and returns to the visible Scene-30
field. It leaves the candidate record active and keeps the sampled FFE183
choice available for the later request scan. Debug/Release QA checks the text,
portrait, no-options state, close input, actor retention, and FFFC3C transfer.

### Scene-38 choice 1

**[C]** The US-ROM row at `0x1EE0E8` selects template `0x1D6988`, position
record `0x1EE100`, and ordered overrides `+0x9E=0x20`, `+0x2E=0x80`,
`+0x9E=0x08`. Bytes at `0x1EE100` begin `0D 00 05 00`, placing the candidate
at `(0x0D00,0x0500)`. The extracted table identifies it as Scene 38 choice 1
in the same `0xA800` one-of-three group. The QA uses source initializer 5,
matching effect indices, no pending player spawn, and seed zero; ROM `0x0D3A`
therefore selects choice 1. `0x43B6` drains this A800 row before Scene 38's
default list.

Debug and Release `shadowrun_ghidra_conversion_qt_field_pedestrian_scenes_qa`
also drives this candidate through the live runtime. The runner starts at
`(4906,1156)` carried from the prior QA context; after 59 neutral modal-release
frames, ordinary Left/Down input reaches the actor in 55 frames at runner
`(3286,1246)` and actor `(3328,1280)`.
The runtime sets the contact bit, presents the same ROM-backed no-options page
and portrait, accepts B after the source wait, and restores the field with the
actor retained. It checks the FFE110-to-FFFC3C transfer and existing-effect
guard. Scene 30 choice 2 uses A; Scene 38 choice 1 now uses B. This adds
physical input coverage for another scene, the other reachable random result,
and both button dismissals.

### Contact-page frame inspection

I inspected the full 320x224 Scene-30 and Scene-38 contact frames. Both show
the same portrait at the left of the no-options dialogue window and the text
`Hey! There you are.` / `Let's go. My new employer is waiting.` without any
choice rows. The two page BMPs are byte-identical across scenes and across
Debug/Release, SHA-256
`5e3f21af532a86afb2f0da0221b7eb116c2c56cfedc31dfb116b203dbffa8981`.
Their closed-field frames differ, retaining each scene's map and actor/player
positions.

This payload-to-frame match is consistent with ROM `0x1F146` loading text
through `0xFDE3E -> 0xFE1F7`, portrait profile `0x121FF4`, and the following
no-options sentinel before the shared `0x1F3C4` wait. It confirms the host
shows the expected text and portrait consistently for both contacts; it is
not a Genesis pixel comparison. An independent, route-matched capture of this
specific page remains open.

## No-options controller event sweep

**[C]** ROM `0x1F3C4` sends a no-options text record through `0x1F592`, which
waits `0x1E` VBlanks and then calls `0x0D94`. The wait loops until
`FFDD22 & 0xFF00` is nonzero, so A, B, C, Start, and all four directions are
accepted events. The Qt field dialogue runtime uses a 31-frame lockout to
represent the source delay plus the following VBlank sample.

Debug/Release `shadowrun_ghidra_field_dialogue_runtime` sweeps all eight
high-byte event bits against a no-options page, verifies neutral and low-byte
input keep it open, drains all 31 lockout frames, then checks that each event
returns to the field. The physical Scene-38 contact above separately verifies
B on the actual A800 generic-contact page. This verifies the input filter and
host timing; it does not prove automatic FFDF42 activity continuation or an
independent Genesis page comparison.

The controller sweep alone does not establish that an A800 contact automatically
continues into the world-activity result. The ROM has a separate automatic
request path that does not require `FFDF42`: at `0x095E`, the field loop tests
`FFDF3E` and clears it at `0x0970`. When `FFE186` is zero, it tests `FFDF40` at
`0x0992`; a nonzero value branches directly to `0x0A72`, which calls
`0x1E43C` at `0x0A92`, calls `0x2F32` at `0x0A98`, and clears `FFDF42` at
`0x0A9E`. This path bypasses the ordinary `FFDF42` test at `0x0A68`.

The generic A800 contact callback does not itself provide the missing direct
request write. ROM `0xED6A` installs `0x1F146` in `FFDD2C`. The callback bytes
at `0x1F146..0x1F169` set `FFFC1C` bit 0, load the text pointer `0xFDE3E` and
speaker profile `0x121FF4`, clear `FFF10C`, call the shared mode-exit/text
service at `0x1F246`, and return. There is no `FFDF42` operand in that body.
A full-ROM scan finds only two literal `00 FF DF 42` operands: the main-loop
test at `0x0A68` and its clear at `0x0A9E`. This rules out the contact callback
as a direct absolute-address writer; a computed or indirect write elsewhere
remains possible. The displayed contact page and the `0x1E43C` request handler
are therefore still separate steps in the traced path.

The preceding world-frame code also traces one way to raise `FFDF3E`:
`0x0B7B8` calls `0x1651E` at `0x0BA7C`; its carry result initializes the
`FFDF3A`/`FFDF3C` fade at `0x0BA86..0x0BA8E`. The ordinary service calls
`0x16396` at `0x0AC6`; when that fade expires, `0x163D0` sets `FFDF3E` for the
next loop pass. This traces a world-interaction producer for `FFDF3E`; the
subsequent call to `0x1E43C` remains conditional on `FFDF40` being nonzero at
the transition, and no specific A800 candidate is identified as its trigger.

The typed coordinator maps the prepared-scene gate at `0x0992` to its
world-activity request and field-scene initialization callbacks. The
`shadowrun_ghidra_field_runtime_coordinator` CTest now explicitly covers mode 2
with `field_scene_gate=2`, as well as the existing generic gate-1 case. This
covers the coordinator branch. The downstream `0x1F16A` result callback is now
connected through a typed `WorldActivityCallbackKind::extraction_result`
continuation and Lua result tasks; focused tests cover both result branches.
This callback integration does not identify which physical A800 contact starts
the automatic route.

The active host also has no production writer for
`FieldRuntimeCoordinatorState::world_activity_requested`: the member starts
false, `advance_field_runtime_frame` only reads and clears it, and its focused
coordinator test seeds it directly. The same-named Lua actor decision is
handled separately in `FieldSceneRuntime` by scheduling the typed
`0x1DBAA`-equivalent timeout (`FFDF84/FFDF86`); it does not set the coordinator
member. Thus the coordinator branch is tested in isolation, while the live
producer for the ROM `FFDF42` request and an automatic A800 continuation remain
unconnected.
The direct `FFDF42` producer is still unknown, and both physical A800 cases
consume the retained choice through the `0x1E45E` test seam rather than proving
that a pedestrian contact reaches this automatic route.

All 96 files in the Debug and Release pedestrian QA output directories match
by SHA-256, including the entry, contact, and closed-field BMPs, both route and
waypoint traces, the 46-row candidate matrix, and the Scene-40 guard route.
I inspected
the Scene-38 page and restored-field frames; these are regression artifacts,
not independent Genesis evidence. Scene-entry travel and physical contact for
other A800 candidates, the direct FFDF42 producer, and a route-matched Genesis
comparison remain open.

The FFDF3E/FFDF40 coordinator call path is traced above, but the current A800
physical routes do not demonstrate that they trigger it. The A800 hill remains
partial.

The Qt contact path now carries the successful generic-contact callback into
typed field-exit state: ROM `0xA800` selects mode 5 (`FFFBED=5`), and
`0x1F146` sets `FFFC1C` bit 0 while preserving its independent bit-7 mirror.
The accepted mode-5 branch in `finalize_field_mode_exit_transition` clears the
Lua-owned `FFE183` choice before its high-slot cleanup. ROM `0xC69A` calls
`0x1E68C`, whose `move.w #-1,(FFFBEC)` resets the typed `FFFBED` selector to
`0xFF` and whose following byte clear resets the complete typed `FFFC1C` flag.
The native cleanup publishes both resets before its cleanup callback. Its
focused CTest covers accepted cleanup ordering and the rejected bit-7 branch;
physical Scene-30 and
Scene-38 contact QA asserts the published mode and bit 0. Those contact tests
still consume the choice through the `0x1E45E` QA seam: they do not establish
the physical trigger for automatic `0x1E43C` processing or the direct
`FFDF42` producer.

## FFDF42 request-producer census (bounded)

**[C]** A byte scan of the US ROM for the big-endian absolute address operand
`00 FF DF 42` finds two occurrences, at file offsets `0x0A6A` and `0x0AA0`.
The instructions are the `tst.w (0x00FFDF42).l` at ROM `0x0A68` and
`clr.w (0x00FFDF42).l` at `0x0A9E`. The field loop branches around the request
path when the word is zero; when nonzero it calls `0x1E43C` at `0x0A92`, calls
`0x2F32`, clears the request, then restarts the scene loop. The exported call
graph also has only this direct caller for `0x1E43C`.

The adjacent absolute operands were checked for writes that could overlap the
word at `FFDF42`. `FFDF3E` accesses are word-sized; `FFDF40` has word-sized
tests and stores only; no `FFDF41` or `FFDF43` absolute operand occurs. There is
no direct longword store beginning at `FFDF40` that could include the request
word. In the generated Ghidra C, the only `DAT_00ffdf42` references are the
read and clear in `fn_0000075E.cpp`. The nearby exported pointer walks begin at
`FFDF44` for the scene anchor/collision-probe records or at `FFDF96`/`FFDFCA`
for action-candidate arrays; the inspected walks advance from those bases and
do not write backward into `FFDF42`.

This is a bounded result, not proof that the ROM has no direct producer. A
dynamically computed pointer or unresolved indirect call could still alias the
request word; the current export does not identify such a path. The separate
`FFDF3E` + `FFDF40` call into `0x1E43C` is traced above and does not depend on a
nonzero `FFDF42`; the direct request producer remains open. The callback body
and its typed host integration are summarized below.

## ROM callback at `0x1F16A`: extraction result

**[C]** ROM `0x1E43C` installs `0x1F16A` at `0x1E49C` as the pending field
callback before `0x1E2EC` starts the 60-frame activity countdown. Decoding the
US-ROM instructions at `0x1F16A` shows a shared prelude at `0x1F1B0`: when
`FFFBED` is 4 or 5, `FFFBEE` is 5, and `FFFBEF` is 6, it advances `FFFBFB`
and applies the source upper clamp of 8. Other input combinations leave that
counter unchanged.

The callback branches on `FFE181`, which `0x1E43C` sets to zero when its
13-record scan finds an active record with `+0x10` bit 0 and `+0xB0` bit 7;
otherwise the scan leaves it nonzero:

| Callback state | ROM writes and calls | Prompt sequence |
|---|---|---|
| `FFE181 == 0` (matching record found) | Set `FFFC1C` bit 7, then call `0x1DED8`; that routine clears `FFFC3C` and retires its seven actor slots carrying `+0xB0` bit 7 through `0x1E514`. | Descriptor `0xF3D64` presents text `0xF3A91` with the shared MORE label `0xFE5BF`. Selecting it sends descriptor `0xF3D78`, whose no-options page uses text `0xF3B2D`. The two pages describe the Wasp pickup and departure. |
| `FFE181 != 0` (no matching record) | Call `0x1E68C`, which clears `FFFBEC` and `FFFC1C`. | Descriptor `0xF3D80` has the no-options sentinel and presents the cancellation text at `0xF3BE3`. |

Both branches set portrait profile `0x121FF4`, clear `FFF10C`, and enter the
shared mode-exit and prompt path at `0x1F246` / `0x1F3C4`. ROM `0x1F3C4`
confirms that the success branch's command pointer `0xF3D78` loops back as the
next prompt descriptor, while the `0xFFFFFFFF` first-option sentinel takes the
one-page input wait. The text bytes terminate at `0xFF`; `0x80` becomes a line
break, matching the offline field-text reader.

The host connects this callback as a typed pending field-scene continuation.
The request path starts `FFE11C`'s 60-frame timeout separately from the
`FFE166` coordinator gate, then dispatches the callback on the next eligible
field pass rather than waiting for that timeout. On success, the host mirrors
the `FFFC1C` effect-control bit and retires the existing seven-slot actor
records before Lua presents the two pages. On failure, Lua clears the accepted
run offer and presents the cancellation page. The Lua result task advances
`FFFBFB` only for run types 4/5 targeting area/event 5/6, through the shared
source-capped counter helper.

Debug and Release `shadowrun_ghidra_world_activity_routine` and
`shadowrun_gameplay_script_runtime_qa` cover the typed request result, the
success/failure pages, the MORE continuation, active-offer retention/clearing,
and the Renraku-only counter gate at values 7 and 8. The focused
`shadowrun_ghidra_conversion_qt_field_world_activity_qa` now seeds both outcomes
of the 13-record scan, dispatches the typed `0x1F16A` continuation through the
field coordinator, and writes full-screen `actual.bmp` files for success page
one (with MORE), its follow-up page, and the cancellation page, plus
`extraction-return-field.bmp`. Debug and Release hashes match for all four
captures; I inspected the full frames for the Mortimer portrait, ROM-derived
Wasp/cancellation text, page affordance, and return cleanup. These are internal
regression captures, not an independent Genesis comparison. The QA seam starts
after the A800/contact trigger and supplies the scan outcome; it does not prove
that a physical contact reaches `0x1E43C`/`0x1F16A`. The direct `FFDF42`
producer, physical automatic route, and route-matched independent Genesis
capture remain open, so the callback hill remains partial.

## Scene-26 A800 candidate contact route (2026-10-27)

The source-routed Scene-2 event-1 QA selects Scene 26 and verifies choice 1 at
list `0x1EDCB0`, position record `0x1EDCC8`, `(0x2000,0x0500)`, slot 3. The
contact CTest retains that actor, then uses the documented Scene-26 static
profile QA checkpoint `(0x0A00,0x0500)`. It opens both source passkey cells
(`0xA2 -> 0xA1` and `0x82 -> 0x81`) through ordinary input and follows a
collision-map route to the pedestrian. The route trace records 163 movement
frames; at contact the runner is `(0x2008,0x04C4)` and actor is
`(0x2000,0x0500)`.

ROM `0xED60` installs callback `0x1F146` when this generic A800 row is
contacted. Debug/Release `shadowrun_ghidra_conversion_qt_field_scene26_a800_contact_qa`
checks the one-page Mortimer Reed text, portrait `0x121FF4`, zero-option wait,
A dismissal, return to the visible Scene-26 field, actor `+0xB0.7`, and retained
FFE110/FFFC3C transfer state. The test sets the source `+0x11.UNSEEN` gate for
the full travel and contact route so unrelated random encounters do not
interrupt this audit, then restores the original runner flags. This gate is a
QA fixture; it is not evidence that the ordinary profile blocks those events.
All 34 Debug/Release output hashes match. I inspected the full-screen contact
page and returned field; they are internal renderer captures, not independent
Genesis visuals.

The contact-route investigation also exercised the real scene-26 random
outcome rows and found that command IDs and dispatch values were stored as
hexadecimal TSV strings, including the tagged `return:` form for a prompt-loop
return. `field_encounters.resolve` now converts these to typed numeric command
and target values while preserving whether the result is a handler call or a
prompt return. The C++ outcome holds the full 32-bit source command. Debug and
Release `shadowrun_gameplay_script_runtime_qa` verifies the real Scene-26
security-plate row and a Redmond prompt row with a tagged return target; the
world-content loader QA also passes. The UI handlers for selected random
dialogue events remain unconnected, so the contact CTest exercises the source
leader-protected gate to isolate this candidate-contact path.

This route does not prove continuous travel from the normal Scene-26 resource
entry to the candidate: it checkpoints the runner at the static profile before
opening the two gates. Normal generation of the target map/event pair, physical
automatic `0x1E43C` processing after contact, the direct `FFDF42` producer, and
a route-matched Genesis capture remain open.

## Normal Employee Recruitment targets that reach A800 scenes (2026-10-27)

The US ROM bytes at `0xF3E6E` are `00 05 00 04 03 02 03 03`, four ordered
`(area,event)` pairs. Bytes at `0xF3E76` are `01 02 03 06`, the two-site
pool. The source initializer dispatch at `0x1E69C` normalizes Employee
Recruitment variants 11 and 12 to the `FFFBED=5` value consumed by A800;
initializer 11 chooses the two-site pool, while initializer 12's `0x1E7D0`
handler uses the fixed Renraku pair `(5,6)`. The runtime's named table in
`gameplay/scripts/run_support.lua` retains those same values and names.

The field event tables route every pair in these pools to a resource scene in
the A800 range `0x15..0x2B`:

| Source initializer | Target pair | ROM field route | A800 destination |
|---|---|---|---:|
| 5 | `(0,5)` Fuchi | Scene 0 record 46, event-table row 5 at `0x0D54B7` = `0x15` | 21 |
| 5 | `(0,4)` Mitsuhama | Scene 0 record 47, row 4 at `0x0D54B6` = `0x1E` | 30 |
| 5 | `(3,2)` Fuchi | Scene 3 record 97, row 2 at `0x18A6FE` = `0x15` | 21 |
| 5 | `(3,3)` Mitsuhama | Scene 3 event-table row 3 at `0x18A6FF` = `0x1E` | 30 |
| 11 | `(1,2)` Ares | Scene 1 record 71, row 2 at `0x17D622` = `0x1A` | 26 |
| 11 | `(3,6)` Lone Star | Scene 3 record 96, row 6 at `0x18A702` = `0x29` | 41 |
| 12 | `(5,6)` Renraku | Scene 5 record 46, row 6 at `0x0679CC` = `0x23` | 35 |

The target-entry handler compares the active run pair with the current
`FFFBF8/FFFBF9` map/event pair before the event-table destination continues.
ROM `0x1EDB88` supplies an A800 candidate row for each of scenes 21, 26, 30,
35, and 41. This establishes that the ordinary Employee Recruitment pools
can lead into conditional-pedestrian scenes; the remaining A800 mode, story,
index, and effect-marker gates still apply on entry.

The existing deterministic Johnson captures show examples of normal pool
selection: initializer 5 displays `(0,4)` Mitsuhama, initializer 11 displays
`(3,6)` Lone Star, and initializer 12 displays `(5,6)` Renraku in
`johnson-offer-{5,11,12}-generated-offer.bmp`. The route manifest records
selection seeds 2/4/4; those seeds choose the source offer row, while the
negotiation, target, and client draws remain live. Accepting the generated
offer moves that offer, including its target, into the active-run slot in
`run_registry.lua`. These captures are internal Qt artifacts, not Genesis
visual evidence.

The Scene-26 contact route above still forces `(2,1)` (Little Chiba), which is
not in either standard target pool. Two normal initializer-11 targets now have
accepted-offer-to-doorway traces followed by target-map checkpoints into their
conditional resource scenes. The checkpoint boundary means these QAs do not
prove continuous Johnson-to-target street travel in one runtime session.

## Initializer-11 Lone Star target into Scene 41 (2026-10-27)

`shadowrun_ghidra_conversion_qt_field_employee_recruitment_target_pedestrian_qa`
uses the ordinary Johnson work-menu acceptance path for initializer 11 and
reads the generated route from the active run registry. The accepted target is
`(3,6)`, matching the ROM pool bytes at `0xF3E76` and the Scene-3 event route
table. It carries the live run registry, funds, and source random seed into a
fresh Scene-3 runtime initialized at a documented field checkpoint; this is an
explicit QA seam, not a simulation of travel from Johnson to Penumbra.

The Scene-3 source collision record 96 at `0x18AB96` is at
`(0x0700..0x071F, 0x0040..0x005F)` with event code `0xC6`. Ordinary Up input
contacts it and event row 6 at `0x18A702` routes to Scene 41. The target-entry
handler compares the active run target against `FFFBF8/FFFBF9`; the entry page
contains “You create a diversion” and “Finding your quarry and escaping.” The
test dismisses it through ordinary A input after the source 31-frame input
lockout.

Scene 41 allocates one conditional A800 actor from candidate choice 1,
`pedestrian_k0b_none00`, list offset `0x1EE1F6`, position `(0x1500,0x0E80)`,
slot 3, with source flags `0x08`. The capture records natural Scene-41 arrival
separately: the actor is outside the initial viewport, so the final pedestrian
image pans the QA camera to show the already selected actor. This framed image
does not claim the actor is naturally visible on arrival.

The QA emits `target-map-spawn.bmp`, `target-doorway-contact.bmp`,
`employee-target-entry.bmp`, `scene-41-natural-entry.bmp`,
`scene-41-pedestrian-actual.bmp`, three Johnson offer captures, and
`normal-target-route.tsv` under each Debug/Release QA artifact directory. Their
hashes match between configurations. These are internal Qt regression
artifacts, not an independent Genesis comparison. The tested route adds one
normal target pair and verifies the Scene-41 pedestrian spawn. The Ares route
below covers initializer-11 `(1,2)` but its mismatched effect indices correctly
suppress the Scene-26 candidate. The Renraku route below covers initializer-12
`(5,6)`. The three remaining initializer-5 pairs, other A800
mode/story/index/effect permutations, continuous street travel, and
route-matched independent Genesis captures remain open.

## Initializer-11 Ares target into Scene 26 (2026-10-27)

The Ares CTest follows the ordinary Julius work menu and accepts the live
initializer-11 offer with target `(1,2)`. Offer-selection seed 28 selects the
initializer-11 row and leaves negotiation, target, and client draws live. The
accepted run registry, funds, and source RNG state are carried into a fresh
Scene-1 runtime at checkpoint `(0x5100,0x2500)`; continuous travel from the
Johnson office to Puyallup remains outside this QA.

Scene-1 collision record 71 at `0x17D9CA` covers
`(0x0500..0x051F,0x0220..0x023F)` with event `0xC2`. The recorded collision
grid shows the approach cell clear. Four ordinary Up updates reach the trigger;
event-table row 2 at `0x17D622` routes to Scene 26. The active run pair matches
the current Scene-1/event-2 facts, so the same ROM target-entry passage opens.
After its 17-frame page wait, ordinary A is accepted after the 31-frame
lockout.

The Scene-26 resource pass samples A800 candidate choice 2, row
`pedestrian_k0b_none00` at list `0x1EDCCE`, position `(0x2D00,0x2300)`. It
does not publish an actor: the live selected effect index is `FFE12C=1`, while
the active index is `FFF0C8=0`. ROM `0xA800` retains the sampled `FFE183`
choice, then rejects the random candidate branch when those words differ. The
typed runtime also reports no active world effect and no `FFFC3C` transfer
template, matching the no-actor result. The QA asserts the retained candidate
choice and these gate values, and captures the natural Scene-26 arrival with
zero conditional pedestrian actors. This records the source-gated absence; it
does not force a candidate to appear.

Debug and Release pass
`shadowrun_ghidra_conversion_qt_field_employee_recruitment_ares_pedestrian_qa`.
All 11 files under
`qt_runtime/field-employee-recruitment-ares-pedestrian-qa/`, including the
doorway collision grid and route TSV, have matching hashes between builds.
The inspected entry page and no-actor scene are internal Qt captures, not
independent Genesis visuals. Together with the Lone Star route above, both
initializer-11 target pairs have now been generated and contacted. The
initializer-12 `(5,6)` route is recorded below; the three remaining
initializer-5 target pairs, continuous street travel, the other A800 gate
permutations, and route-matched independent Genesis captures remain open.

## Initializer-12 Renraku target into Scene 35 (2026-10-27)

The Renraku CTest accepts the ordinary Icarus work-menu offer from initializer
12. Offer-selection seed 4 chooses its fixed `(5,6)` target; later negotiation,
target, and client RNG draws stay live. The accepted run registry, funds, and
source random seed are carried into a fresh Scene-5 runtime at checkpoint
`(0x2700,0x1000)`. This is the same explicit QA seam used by the two
initializer-11 cases, not continuous street travel from the Johnson office.

Scene-5 collision record 46 at `0x067C36` covers
`(0x0260..0x027F,0x00A0..0x00BF)` with event `0xC6`. The checkpoint is clear;
23 ordinary Up updates contact the doorway, and event-table row 6 at
`0x0679CC` routes to Scene 35. The target-entry passage opens and dismisses
after its 17-frame presentation wait and 31-frame input lockout.

Scene 35 enters the ROM `0xA800` random-pedestrian path: initializer 12 maps to
`FFFBED=5`, and the resource activity uses mode `FFDF40=2`. The selector
samples candidate choice 1, list `0x1EE034`, position `(0x0400,0x0980)`. It
does not publish the row: the live selected index is `FFE12C=4`, while the
active index is `FFF0C8=0`. Both effect markers are clear and `FFFC3C` has no
transfer template, so the unequal indices reject the actor while retaining
the sampled choice. The QA checks that gate-derived no-actor outcome rather
than forcing a pedestrian into Scene 35.

Debug and Release pass
`shadowrun_ghidra_conversion_qt_field_employee_recruitment_renraku_pedestrian_qa`.
All 11 output files, including the natural Scene-35 capture, collision grid,
and route TSV, have matching hashes between configurations. The field and
target screenshots are internal Qt captures; no route-matched independent
Genesis frame is available. The three normal initializer-11/12 target pairs
are now contacted, while the three remaining initializer-5 pairs, remaining
A800 gate-state combinations, continuous street travel, and independent
Genesis comparison remain open.

## Initializer-5 Mitsuhama target into Scene 30 (2026-10-27)

The new `shadowrun_ghidra_conversion_qt_field_employee_recruitment_initializer5_mitsuhama_pedestrian_qa`
accepts the ordinary initializer-5 offer selected with source seed 2. The
generated run target is `(0,4)`, matching the second row in ROM's four-site
pool at `0xF3E6E` and the named `four_site` Lua pool. The QA retains the
accepted run registry, funds, and source random seed while bootstrapping a
fresh Scene-0 runtime at `(0x1100,0x2B00)`; this checkpoint is an explicit test
seam rather than continuous travel from Johnson.

Ordinary Up input contacts Scene-0 collision record 47 at `0x0D57B4`, bounds
`x=0x0100..0x011F`, `y=0x0280..0x029F`, event `0xC4`. Event-table row 4 at
`0x0D54B6` selects resource Scene 30. The target-entry passage appears, then
ordinary A dismisses it after the 17-frame presentation wait and 31-frame
input lockout.

The Scene-30 resource entry normalizes initializer 5 to `FFFBED=5`, uses mode
`FFDF40=2`, and samples choice 2. It selects list `0x1EDDDC` and position
record `0x1EDDF4`, placing `pedestrian_k0b_none00` at `(0x0900,0x0B00)` in
slot 3. The active and selected effect indices are both 0; world-effect,
pending-effect, and player-spawn-pending facts are clear. After the candidate
is selected, the runtime retains its typed actor descriptor for later contact,
so the final diagnostic reports `transfer_template_present=1`. That output
descriptor is separate from the player-spawn-pending input used by the Lua
A800 gate and must not be re-read as the pre-selection FFFC3C condition.

Debug and Release pass this route plus the existing initializer-11 Lone Star
and Ares tests and initializer-12 Renraku test. All 11 output files for the
new route, including eight full-screen BMPs and three TSV manifests, have
matching SHA-256 hashes across configurations. The inspected screenshots are
internal Qt captures; there is no route-matched independent Genesis capture.
The three remaining initializer-5 target pairs are covered below. Continuous
street travel, remaining A800 gate permutations, and independent Genesis
comparison remain open.

## Remaining initializer-5 targets from the four-site pool (2026-10-27)

ROM `0xF3E6E` supplies the initializer-5 four-site target pool
`[(0,5),(0,4),(3,2),(3,3)]`. The existing initializer-5 Mitsuhama route covers
`(0,4)`. Three new physical-entry QAs accept each remaining pair through the
ordinary Johnson work-menu path, carry the accepted run registry, funds, and
source random seed into a fresh source-scene runtime at an explicit checkpoint,
walk into the mapped collision record with ordinary Up, dismiss the target
passage with A, and resume the imported event-table route. The checkpoints are
QA seams; they do not claim continuous street travel from Johnson.

| Generated target | Physical route from ROM trigger manifest | A800 candidate and live outcome |
|---|---|---|
| `(0,5)` Fuchi; seed 539; post-offer seed 23623 | Scene 0 record 46 at `0x0D57AA`, bounds `x=0x06A0..0x06BF`, `y=0x0060..0x007F`, collision/event `0xC5`/5; row 5 at `0x0D54B7` selects Scene 21 (`0x15`). Checkpoint `(0x6B00,0x0900)`. | Choice 1 retains list `0x1EDECC`, position `(0x2800,0x0580)`. `FFF0C8=0`, `FFE12C=3`; the mismatched indices suppress the actor while retaining the sampled choice. |
| `(3,2)` Fuchi; seed 41; post-offer seed 22901 | Scene 3 record 97 at `0x18ABA0`, bounds `x=0x0080..0x009F`, `y=0x0140..0x015F`, collision/event `0xC2`/2; row 2 at `0x18A6FE` selects Scene 21. Checkpoint `(0x0800,0x1700)`. | Choice 2 retains list `0x1EDEEA`, position `(0x4700,0x2D80)`. `FFF0C8=0`, `FFE12C=3`; the mismatched indices suppress the actor while retaining the sampled choice. |
| `(3,3)` Mitsuhama; seed 430; post-offer seed 37546 | Scene 3 record 95 at `0x18AB8C`, bounds `x=0x02C0..0x02DF`, `y=0x00A0..0x00BF`, collision/event `0xC3`/3; row 3 at `0x18A6FF` selects Scene 30 (`0x1E`). Checkpoint `(0x2D00,0x0F00)`. | Choice 1 retains list `0x1EDDBE`, position `(0x2300,0x3400)`. `FFF0C8=0`, `FFE12C=3`; the mismatched indices suppress the actor while retaining the sampled choice. |

In all three routes the `0xA800` activity selector reaches its random branch:
the run normalizes to `FFFBED=5`, `FFDF40=2`, both world-effect markers are
clear, and the player-spawn-pending gate is clear. It samples `FFE183` before
the later effect-index comparison rejects publication. The tests assert the
exact retained choice row and position even when the gate suppresses the actor.
This distinguishes candidate selection from an actor actually entering slot
3. The existing `(0,4)` Scene-30 route covers the complementary live match,
`FFF0C8=FFE12C=0`, and publishes choice 2 from list `0x1EDDDC` at
`(0x0900,0x0B00)`.

All seven normal Recruitment target-route CTests pass in Debug and Release.
Each of the three new routes emits 11 files; names and SHA-256 hashes match
between builds. The route loop waits for the existing host modal-return fade
counter to reach zero before saving its destination frame; the TSV's
`settle_frames=59` is a host presentation count, not a ROM timing claim. The
`begin_field_modal_return_transition` code labels this 60-frame fade-in a host
presentation choice. Full-frame target-entry and destination images were
inspected, including the Scene-30 matched-index actor image. These are internal
Qt captures, not independent Genesis comparisons. The full four-row pool now
has physical target-pair coverage; continuous travel, the remaining A800 gate
permutations, and independent Genesis comparison remain open.

## Section-37 Penumbra population screen (2026-10-28)

I screened the independent longplay frames in
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_37_source_9420-9720/`
against the ROM-derived regional maps. Frames 2340 and 2400 match `mode-3.bmp`
at map offsets `(972,642)` and `(1212,642)`, with normalized grayscale
correlations `0.9581` and `0.9643`. The surrounding sequence also matches
Scene 3 at frame 2280 `(808,662)`, frame 2460 `(1400,766)`, frame 2520
`(1640,616)`, and frame 2550 `(1742,614)`; scores range from `0.8250` to
`0.8816` outside the two strongest matches. These are screen-to-map
registrations, not actor telemetry.

The capture visibly contains a red/brown figure near screen center and a blue
figure near the left edge in the 2340/2400 pair; two portrait panels are also
visible in the HUD. Approximate map positions from the strongest registrations
are `(1084,748)` and `(992,692)` at frame 2340, then `(1309,764)` and
`(1221,694)` at frame 2400. Both figures move east in map coordinates over
that interval. The ROM default list at `0x18A73A` contains three pedestrian
records with route lists `0x18A76E`, `0x18A786`, and `0x18A794`. Their
horizontal route segments near `y=776..784` run west; the other eastbound
segments are near `y=224`. The measured pair therefore does not provide a
route-and-direction match to those default pedestrians.

**[H]** The two field figures are compatible with the two visible party
portraits, but the recording has no actor-slot or input log. This sequence
does not identify an ambient pedestrian, A800 candidate, or generated actor,
and does not establish that Scene 3 lacks other walkers. Keep the Scene-3
independent population comparison open; the source lists remain confirmed,
while this particular footage is not a route-level pedestrian match.

## Section-9 Scene-2 pedestrian route candidate (2026-10-28)

**[C]** The independent longplay frame sequence at
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_09_source_1020-1320/`
shows a small walking sprite separate from the screen-centered runner at
frames 1800 through 1818. A fresh registration of the left 272x224 viewport
against ROM-derived Scene-2 `mode-2.bmp`, using grayscale normalized
cross-correlation (`TM_CCORR_NORMED`), places it at the offsets below. Masked
matching against the extracted `pedestrian_k0b_none00` walk frames yields the
corresponding sprite positions and scores:

| Capture frame | Map offset | Map score | Matched walk frame | Sprite upper-left | Sprite score |
|---:|---:|---:|---|---:|---:|
| 1800 | `(646,440)` | 0.96908 | dir 2/6, frame 1 | `(66,77)` | 0.75577 |
| 1803 | `(646,452)` | 0.96921 | dir 2/6, frame 1 | `(63,65)` | 0.74898 |
| 1806 | `(646,464)` | 0.96804 | dir 2/6, frame 1 | `(60,53)` | 0.73883 |
| 1809 | `(646,476)` | 0.96663 | dir 2/6, frame 3 | `(57,41)` | 0.74813 |
| 1812 | `(646,488)` | 0.96756 | dir 2/6, frame 3 | `(53,29)` | 0.75643 |
| 1815 | `(646,500)` | 0.96285 | dir 2/6, frame 0 | `(49,18)` | 0.75103 |
| 1818 | `(648,512)` | 0.96590 | dir 2/6, frame 0 | `(42,6)` | 0.75267 |

The extracted direction-2 and direction-6 images for frames 0, 1, and 3 are
pixel-identical, so the sprite match does not determine its facing direction.

**[C]** The corresponding registered foot positions move from approximately
map `(720,543)` at frame 1800 to `(698,544)` at frame 1818. The raw US-ROM
bytes at `0x172D1C` identify source-order 0 as template `0x1D6988` and point to
position data `0x172D58`. That data begins with spawn `(0x3480,0x2200)` =
`(840,544)` map pixels, followed by waypoint `(0x2800,0x2200)` = `(640,544)`.
The map offset's Y rises 72 pixels across the sequence while the matched
sprite's screen Y falls by 71 pixels; after the extracted frame's foot offset,
its registered Y remains within about one map pixel of the ROM route. Its map
X moves west along the source segment. The larger central figure is spatially
separate; its actor identity is not inferred here.

The source connection is the ordinary default-list path: `0x44BC` seeds the
actor's live and target positions from the route's initial pair and retains
the route pointer; `0x142AE` loads the next `+0xE0` pair for an actor with the
route flag when its current target is reached; `0x145A2` prepares movement
toward that target, and `0x5A62` projects world coordinates against the camera
for drawing. The route pointer and coordinate pairs above were checked in the
US ROM; the exported disassembly is a control-flow cross-check. The video
sampling supports the position and art match, but does not reveal the live
actor record or source movement/animation counters.

**[H]** This is a strong route-and-art candidate for the ordinary Scene-2
default pedestrian, likely source order 0; the video has no actor-record or
input telemetry, so it cannot prove the live record identity. The later
Lone-Star dialogue in the same section is not linked to this sprite. This
match supports the regional default-list path and says nothing about the
separate A800 selector used by resource scenes 21-43.

### Scene-2 pedestrian source-step timing check (2026-10-28)

**[C]** The matched source-order-0 route at `0x172D1C` starts at
`(0x3480,0x2200)` = `(840,544)` map pixels and selects `(0x2800,0x2200)` =
`(640,544)` as its first waypoint. Its ordered override is `+0x9E=0x20`;
the template word is root-tagged `0x801D6988`. The list spawner at
`0x449A..0x449E` initializes `+0x10` to `0x05` and then sets bit 5 for this
tag, so the live byte is `0x25`; bit 1 remains clear. I checked all 75
default-actor rows with waypoint data against their ROM list words, and all
75 have this root tag. The horizontal distance stored by `0x145A2` in `+0xC8`
is `0xC80`. `0xA8EA` visits each active record once per actor-service pass and
calls its selected `+0x3A` continuation at `0xAAB0`; `0x145A2` installs
`0xB574` there for this route.
`0xB574` approaches the stored heading and reaches `0xBADE` for the movement
step. With `FFDF84=0`, the pedestrian branch selects vector table `0xBE62`
(`0xB5EA..0xB624`); since stored `+0xC8=0xC80` is at least `0x460`,
`0xB6E4` leaves that table selected. Its cardinal components are 10 source
coordinate units, or `10/16 = 0.625` map pixels per service pass. The value
is retained for this leg; `+0xC8` is the distance measured when the waypoint
was selected, not the remaining distance recomputed on every movement pass.

The active runtime carries the route fact as typed
`source_actor_has_waypoint_data`, separate from `source_record_flags`: the
default-actor initializer currently seeds that generic byte as `0x05`, while
the ROM byte for this tagged route is `0x25`. Lua receives the typed route fact
and selects the same 10-unit vector for a pedestrian with `+0x10` bit 1 clear
when the stored route distance is at least `0x460` and `FFDF84` is zero. This
trace found no lost waypoint branch in that path; the generic flag byte is not
a byte-for-byte mirror of the ROM record. `advance_default_field_actors()`
runs from the field entity-service callback, while
`advance_default_field_actor_animations()` runs separately at the render-pass
tail. This keeps waypoint decisions and movement on the behavior heartbeat,
and animation on the render heartbeat.

The render cadence follows a separate ROM clock. Kind 11's looping walk list at
`0x8DE0` has four frames. The `0x44BC` initializer copies base record
`0x7361A`, whose `+0x1A` accumulator is zero and `+0x1C` period is `0x1000`.
After the walk selector is chosen, `0x600E` reads the step value at `+0xCA`;
`0x6092..0x60AA` scales 10 by `0x1C`, writing increment `0x0118` to `+0x18`.
Each `0x4618` render pass calls `0x5FDC`, which adds that increment and
subtracts the period when crossed, so the four-pose list advances after 14 or
15 render passes depending on the carried accumulator. The Lua renderer uses
the same `+0xCA`-derived increment and `0x1000` period.

**[I]** In the independent Section-9 capture, the registered foot moves from
approximately map X 720 to 698 across 18 intervals of the 30-fps dump. If the
ordinary field loop takes one actor-service pass per 60-Hz VBlank and
`FFDF84` remains zero, the source step predicts `18 * 2 * 10/16 = 22.5` pixels
west; the measured displacement is about 22 pixels. This is consistent with
the source step for the visible route. The recording has no RAM or capture
clock telemetry, so it does not establish the live record, `FFDF84` value, or
exact number of source passes represented by each dumped image. The existing
route tests check waypoint selection and wrap; they do not independently assert
this per-pass movement rate. Its best masked sprite matches also change from
walk frame 1 to 3 and then 0 at the sampled times. Six render passes elapse
between 30-fps capture images under the same one-pass-per-VBlank assumption;
the observed pose changes are compatible with a 14/15-pass source animation
period, but the capture does not identify the live accumulator phase. This
does not verify a full animation loop. Other regional routes and
collision-blocked steps remain open.

### Scene-5 pedestrian source-step timing check (2026-10-28)

**[C]** The Scene-5 default list at `0x679F0` has a second tagged record at
`0x67A00`: its template is `0x801D6988` and its position list is `0x67A30`.
The ROM pairs start `(0x4B80,0x2200)` = `(1208,544)` with waypoint
`(0x4B80,0x3980)` = `(1208,920)`, then waypoint `(0x1A00,0x3980)` =
`(416,920)`. On reaching the first waypoint, the westward leg is
`0x3180` source coordinate units. The root tag sets `+0x10` bit 5 after the
base `0x05` initialization, leaving bit 1 clear. With `FFDF84=0`, the source
movement branch therefore selects the 10-unit cardinal vector from `0xBE62`;
the stored `+0xC8=0x3180` is above the `0x460` threshold.

In independent Section-41 longplay frames `002140`, `002150`, `002160`, and
`002170`, the gameplay-map registrations give camera X offsets 1028, 1070,
1110, and 1148. A masked match against the extracted kind-11 walk art places
the same west-facing sprite near screen X 170, 114, 62, and 12. Using the
sprite's eight-pixel foot-center offset gives approximate map X values 1206,
1192, 1180, and 1168. Across 30 intervals at 30 fps, the registered sprite
moves about 38 pixels west. If the field provides one service pass per
60-Hz VBlank and `FFDF84` stays zero, the source step predicts
`30 * 2 * 10/16 = 37.5` pixels west. This second route is consistent with
the same per-pass movement rate found in Scene 2.

**[I]** The match supports the Scene-5 route candidate and the movement-rate
inference; it does not identify the live actor record. The capture has no RAM
or source-clock telemetry, so `FFDF84`, the pass count, and the sprite's exact
source identity remain unconfirmed. The compressed masked pose matches do not
support a new animation-cadence claim.

### Early Section-41 Scene-5 route checkpoint (2026-10-28)

**[C]** I extended the Scene-5 registration earlier in the same independent
longplay sequence. Matching the left 272x224 gameplay viewport against
`mode-5.bmp` with grayscale normalized cross-correlation locates the map at
the offsets below. A masked BGR search in the screen corridor implied by the
ROM route's X=1208 coordinate matches the third, right-hand figure to the
extracted kind-11 walk art:

| Capture frame | Map offset | Map score | Matched template | Sprite upper-left | Sprite score | Approx. map X |
|---:|---:|---:|---|---:|---:|---:|
| 2125 | `(968,782)` | 0.93075 | kind 11, walk dir 4/frame 3 | `(235,102)` | 0.8891 | 1211 |
| 2130 | `(990,782)` | 0.92441 | kind 11, walk dir 4/frame 3 | `(213,102)` | 0.8902 | 1211 |
| 2135 | `(1010,782)` | 0.92494 | kind 11, walk dir 4/frame 0 | `(194,101)` | 0.8766 | 1212 |

The approximate map X uses the established eight-pixel foot-center offset.
Across these ten 30-fps capture intervals, camera X advances 42 pixels while
the sprite shifts left 41 pixels on screen, leaving its registered map X
within four pixels of the route's X=1208 line. The ROM record at `0x67A00`
points to route data `0x67A30`, whose first pair is `(1208,544)` and next
point is `(1208,920)`; the following point turns west to `(416,920)`. The
matching location is consistent with that route's vertical leg or turn. It
precedes the already sampled frames `002140` through `002170`, where the same
route candidate is registered moving west.

**[I]** This adds a route-positioned kind-11 art match before the previously
documented westward samples and is consistent with the source waypoint path.
The footage has no actor-record or RAM telemetry, and this check estimates
only X; it does not confirm the waypoint's Y, the exact turn frame, or the
live source-order identity. Treat the visual continuity and route identity as
a strong candidate, not proof of a specific actor record. It does not change
the source-derived movement-rate or animation-cadence conclusions above.

## A800 contact and FFDF42 request separation (2026-10-28)

**[C]** The generic A800 actor-contact path and the main-loop
`0x1E43C` request are separate ROM transitions. At `0xED3C..0xEDB2`, a
contactable record whose `+0x9E` bit 3 and `+0x10` bit 1 pass the actor/player
checks is marked at `+0xB0` bit 7. `0xED60` copies `FFE110` to `FFFC3C`, and
`0xED6A` selects callback `0x1F146` in `FFDD2C`. The `FFE180` and `FFE182`
story markers can replace that callback with `0x53C90` or `0x54402`; otherwise
`0xEDAC` calls `0x1E2EC`.

`0x1E2EC` writes 60 to the `FFE11C` timeout and copies the selected `FFDD2C`
callback into `FFE104`. The field loop's separate callback dispatcher at
`0x0B26..0x0B5E` invokes `FFE104` on an eligible pass, clears it after return
except for its explicit `0x5BAC4` continuation, and restores the active actor
pointer. In the generic case, `0x1F146` sets `FFFC1C` bit 0 and enters the
shared no-options text path at `0x1F246` / `0x1F3C4`. This explains the
physical candidate-contact prompt covered by the Scene-26 and Scene-22 routes.

The automatic result path remains distinct: the ordinary request branch tests
`FFDF42` at `0x0A68` and calls `0x1E43C` at `0x0A92`; the fade/coordinator
branch checks `FFDF3E` at `0x095E`, then nonzero `FFDF40` at `0x0992`, before
joining at `0x0A72`. No transfer from the A800 contact's `0x1E2EC` callback
setup into either `0x1E43C` caller is present in the traced direct control
flow. The earlier literal-address census still leaves computed or indirect
`FFDF42` writers unresolved. Physical A800 prompt QA therefore defends the
contact presentation and return path, not the separate automatic
`0x1E43C` result callback.
