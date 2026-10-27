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
following duplicate-suppression pass. Physical randomized-candidate contact
and an independent Genesis comparison remain open. The
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

One additional tagged record begins at `0x1EE232`, adjacent to the Scene-41
and Scene-42 candidate data. It is not an A800 choice: Scene 41 selects
`[0x1EE1F6, 0x1EE214, 0x1EE214]`; Scene 42 selects
`[0x1EE250, 0x1EE26E, 0x1EE28C]`. A scan found no big-endian literal ROM
longword reference to `0x001EE232`. This establishes that the current
`0x1EDB88` candidate table does not select it, not that every computed or
indirect path to the record has been ruled out.

The other extracted literal pedestrian rows are bounded as follows:
`default-actors.tsv` has 21 rows across scenes 0-5 and 20;
`world-activity-actors.tsv` has the A8AE story actor in Scene 34 and a
pending-list actor at source record `0x1E5FA` in Scene 47; the 857
`dynamic-world-activity-actors.tsv` rows across 56 scenes contain no literal
pedestrian profile, name, or sprite stem. These table counts do not identify
every small sprite in gameplay captures and do not exhaust generated or
computed actor paths. Keep the Section-41 sprite identification open until a
matching scene and live actor record are traced.

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
remains a partial hill because the physical randomized-candidate contact route
and a route-matched Genesis capture are still open.

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
| `frame_001200.png` | `02:57:40.000` | Industrial-looking field map with a blue sprite near the left wall and a brown sprite near the center; neither has a confirmed actor identity. |
| `frame_001500.png` | `02:57:50.000` | Lone Star badge portrait; “Hey, Flick. I think this guy's packing. You got a permit for that illegal weapon?”; choices `Attack!!`, `Talk to them.`, `Run away!`. |
| `frame_001800.png` | `02:58:00.000` | Black transition frame. |
| `frame_002100.png` | `02:58:10.000` | Blue and brown field sprites are visible. |
| `frame_002110.png` | `02:58:10.333` | Another sprite is visible at the right edge while the camera follows the party. |
| `frame_002120.png` | `02:58:10.667` | The party and surrounding sprites have shifted on screen with the camera. |
| `frame_002140.png` | `02:58:11.333` | Several sprites remain in view during movement. |
| `frame_002150.png` | `02:58:11.667` | The sprites overlap near the center of the field. |
| `frame_002400.png` | `02:58:20.000` | Taxi asks for 40 yen to travel to the Puyallup Barrens, with accept and cancel choices. |
| `frame_002700.png` | `02:58:30.000` | Field map with a red/brown sprite near machinery; scene and actor identity remain unknown. |
| `frame_003000.png` | `02:58:40.000` | Tarislar City Inn introduction dialogue and portrait. |

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

The same red/brown figure appears to move through the registered map in nearby
captures. Approximate screen centers and resulting map coordinates are:

| Longplay frame | Map offset | Match | Figure screen center | Approximate map position |
|---|---:|---:|---:|---:|
| `frame_008300.png` | `(262,138)` | `0.97151` | `(115,95)` | `(377,233)` |
| `frame_008340.png` | `(388,161)` | `0.98817` | `(49,61)` | `(437,222)` |
| `frame_008370.png` | `(449,74)` | `0.95782` | `(141,109)` | `(590,183)` |
| `frame_008400.png` | `(546,20)` | `0.98338` | `(117,100)` | `(663,120)` |
| `frame_008430.png` | `(666,39)` | `0.95367` | `(112,100)` | `(778,139)` |
| `frame_008460.png` | `(720,1)` | `0.96946` | `(170,89)` | `(890,90)` |

These nearby-frame offsets use the same broad mask, with the sidebar and center
of the viewport excluded; figure centers are visual estimates from the
independent frames. The frame-8340 focused map mask described above scores
0.97534 at the same offset. A separate BGR `TM_CCORR_NORMED` search, masking
black pixels in the ROM-derived sprite assets, matches the figure crop
`x=32..77, y=30..91` to `pedestrian_k0b_none00` idle/walk direction 2 or 6,
frame 3, at `(39,49)` with score `0.9498`; the next kind-11 sprite match scores
`0.9281`. **[C]** The captured sprite is a close visual match for the extracted
kind-11 pedestrian art. **[H]** This does not identify its actor record or
party membership. Movement also means the current position cannot be compared
directly with an A800 initial position to prove which source path created it.
Keep the A800 identity and pedestrian producer question open; no runtime
behavior or hill status changes from this frame audit.

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
Johnson. The live runner position at Scene-30 entry is `(4906,1156)`. The QA
releases the host modal-close input for 59 neutral frames, then uses ordinary
Left/Down input. Contact occurs on frame 141 at runner `(0x08DA,0x0AD8)` and
actor `(0x0900,0x0B00)`, a per-axis delta of `(-0x26,-0x28)`. The route planner
uses the imported terrain collision model only to choose waypoints; the runner
position, collision response, input handling, and actor contact are processed
by the live runtime.

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
`(4906,1156)`; after 59 neutral modal-release frames, ordinary Left/Down input
reaches the actor in 55 frames at runner `(3286,1246)` and actor `(3328,1280)`.
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

This does not establish that the callback is invoked automatically by the
current host main loop. That loop still requires an FFDF42 request before
calling `0x1E43C`; its later actor scan and `0x1F16A` continuation remain open.
Both physical cases consume the retained choice through the `0x1E45E` test
seam and do not claim activity completion.

All 93 files in the Debug and Release pedestrian QA output directories match
by SHA-256, including the entry, contact, and closed-field BMPs, both route and
waypoint traces, the 46-row candidate matrix, and the Scene-40 guard route.
I inspected
the Scene-38 page and restored-field frames; these are regression artifacts,
not independent Genesis evidence. Scene-entry travel and physical contact for
other A800 candidates, the FFDF42 request producer, the
`0x1E43C`/`0x1F16A` continuation, and a route-matched Genesis comparison remain
open. The A800 hill remains partial.
