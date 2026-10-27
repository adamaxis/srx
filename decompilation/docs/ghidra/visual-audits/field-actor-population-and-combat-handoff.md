# Field actor population and combat handoff audit

**Status:** ROM population, initial behavior entry, direction, animation,
action/item codes, sound-request paths, and the seven-record ordinary actor
allocation limit are traced for profile-selected actors in scenes 2 and
`0x14`. Lua selects a package by stable source-template profile ID and the
ROM-seeded item/spell/none action class and code; native C++ presents its
finite entry animation. In the scene-`0x14` default-list QA route, only the
first seven of eight rows become active. New Game bootstrap does not activate
the scene-2 profile list. The scene-`0x14` mode-1 post-entry completion branch
is now source-traced and checked for a no-spawn result; general post-entry
behavior, pending-list occupancy, the paired effect-slot pool, actor movement,
interaction, collision targets, hits, status, and damage are not implemented.
`LOADOUT-01` remains partial.

## Source path

`0x2CE0` (`advance_field_mode_transition`) maps the current event and transition
context through `0x2E72`. If the selected mode is `0x13` and bit 8 of
`FFFC1E` is clear, it starts world activity with descriptor `0x53EE2`, places
the pending actor-list choice `0x1E552` in `FFE120`, and schedules the source
timeout through `0x1DBAA`. Mode `0x13` maps through `0x2E42` to scene `0x2F`.
The existing typed conversion test records one concrete input: `FFFBF9=8`,
`FFFC10=0x14` selects scene `0x2F` and transition coordinates
`0x5E80,0x1300`.

On the world-frame path, `0x43B6` drains the pending `FFE120` list through
`0x4466`, then drains the default list. For list pointers `0x1E628` and
`0x1E5C4`, it installs actor-list outcome callbacks and separately arms the
activity timeout through `0x1DBAA`. `0x4466` allocates a
world entity and effect slot for each entry; bit 31 of the template pointer
marks a root record. `0x44BC` copies the named template data, installs its
position, and applies optional byte-offset/value overrides. It copies
template byte `+9` to live entity byte `+0x57`. For ordinary item actions,
`0x14D6A` treats `+0x57` as a one-based D4540 item/action code when `+0x56` is
nonnegative; a negative `+0x56` selects the separate D4EA0 spell table. The
guard template has `+8=0`, `+9=6`, which resolves to item 6, Max-Power (Heavy
Pistol). The pedestrian template has `+8=0x7F`, `+9=0`, so it starts with no
active item action. This is a ROM-derived loadout input, not a guess from the
actor's appearance.

The decoded ROM records are:

| List | Records | Source positions |
|---|---|---|
| `0x1E552` | Five `eye5r 1` templates, type `7`, rooted | `5980,0E80`; `5F00,1B80`; `6000,1E80`; `4F00,1200`; `5B00,0980` |
| `0x1E5C4` | One Strike Team, two bodyguards, one pedestrian | `1100,0480`; `1280,0480`; `1100,0580`; `1280,0580` |
| `0x1E628` | Three Strike Team templates and one corp guard | `0F00,0500`; `1080,0500`; `0200,0500`; `0300,0780` |

The names and positions above come from the ROM template/list records, not
guesses based on a screenshot. Keep two fields distinct: `0x44BC` copies the
template's first word (for example, `7` for the `eye5r 1` records) to live
entity `+0x0E`; `0x481E` selects a sprite-source list from the separate type
word at projection slot `FF164C+0x02`. The converted `0x481E` selector maps
projection types 0 through 11 to source `0x5F78`, but the live population to
projection-slot assignment has not been traced. The Qt default-list drawing
path described below uses the actor kind's own `0x600E` state frame and does
not claim that actor records map one-to-one to the ten `FF164C` projection
slots.

## Do not confuse with scene pedestrian choices

`0x5BA98` indexes the table at `0x1EDB88` from scene `FFDDC4` and the
one-based selection counter `FFE183`. Its rows cover scenes `0x15..0x2B` and
provide three single-pedestrian position lists. `0xA800` only selects one
through its run-type, world-mode, resource-index, and pending-spawn gates.
Those entries are not the
three actor groups above and must not be inserted as generic enemies to make
the weapon appear to hit something.

### `0xA800` branch precedence and side effects

Raw ROM `0xA800` (`disassembly.s:0000A800..0000A8AC`) resolves these branches
in order:

| First matching branch | Additional condition | Pending list result | State side effect |
|---|---|---|---|
| `FFE180 != 0` | `FFF0C8 == 4` | Static list `0xA8AE` | Writes its header to `FFE110`, pointer to `FFE124` |
| `FFE180 != 0` | `FFF0C8 != 4` | None; returns without considering `FFE182` | None |
| `FFE180 == 0`, `FFE182 != 0` | None | Static list `0xA8CC` | Writes its header to `FFE110`, pointer to `FFE124` |
| Both effect markers clear | `FFFBED != 5` or `FFDF40 != 2` | None | Does not sample `0x0D3A` |
| Both effect markers clear; offer-initializer/mode gates pass | `FFE183 == 0` | No list decision yet | Samples bound 2, adds one, stores choice 1 or 2 in `FFE183` |
| Random branch reached | `FFE12C != FFF0C8` | None | The sampled `FFE183` remains stored |
| Random branch reached and indices equal | `FFFC3C != 0` | None | The sampled `FFE183` remains stored |
| Run/mode gates pass, indices equal, `FFFC3C == 0` | `FFE183` selects a valid scene row | `0x5BA98` returns the scene/choice list | Writes list header to `FFE110`, pointer to `FFE124` |

This ordering distinguishes “no list” from “no random work”: resource-index
mismatch and a pending player-spawn request suppress the list only after the
one-time choice has been stored. The static `FFE180` and `FFE182` branches
precede and bypass the offer-initializer, mode, random-choice, resource-index, and
player-spawn checks. The candidate row's `activation_gate` records this branch
ordering; it is source documentation, not runtime logic.

The random-pedestrian branch in `0xA800` requires selected offer initializer
byte `FFFBED` value 5,
world mode `FFDF40` value 2, word `FFE12C` equal to word `FFF0C8`, and dword
`FFFC3C` equal to zero. When `FFE183` is zero, it calls `0x0D3A` with bound 2,
then adds one to the result. The ROM routine at `0x0D3A` multiplies the
16-bit PRNG value by the bound and returns the high word (`MULU`, `SWAP`,
`EXT.L`); with bound 2 this is only 0 or 1. A fresh random choice is therefore
1 or 2, not 1 through 3. The table at `0x1EDB88` contains a third pointer for
each scene, but the traced random branch cannot select it. The only non-clear
write to `FFE183` is the bound-2 sample plus one at `0xA87A`; its other writes
are clears at `0x2A82`, `0xC684`, and `0x1E45E`. Choice 3 has no ROM writer and
is unreachable after field-mode initialization; retain it as a table row, not
as a possible random outcome.
`0x5BA98` then indexes `0x1EDB88` by `scene_id - 0x15` and that choice minus
one. The selected pending list is placed in `FFE124`; `0x43B6` drains it
before the `FFE120` list and scene default list. This order matters because
each spawned row consumes an ordinary actor slot before later lists run.

The literal `FFFBED=5` gate is confirmed, but it compares the value after
initializer dispatch. ROM `0x1E69C` copies the selected offer-record byte from
`0xF3EBC[FFF0AE]` to `FFFBED`, then `0x1E71C` calls `F3DC6[FFFBED]`. Raw
initializer 5 uses `0x1E810` and remains 5. Employee Recruitment variants 11
and 12 use `0x1E81C` and `0x1E7D0`; their fallback branches write 5 at
`0x1E848` and `0x1E804`. Package Recovery variants 9 and 10 use those same
handlers but write 4 at `0x1E83C` and `0x1E7F8`. Lua retains the raw offer
initializer and now derives this effective FFFBED value before the A800 gate:
5/11/12 pass, while 9/10 resolve to 4 and fail.

For the `0x15..0x2B` scene-entry path, ROM `0x167AE` sets `FFDF40=2`, clears
the 16-bit active effect index `FFF0C8`, reads the random-index bound from the
active scene-resource record, and samples the 16-bit selected effect index
`FFE12C`. Thus the equality gate first compares that random choice with the
initial active index, zero. The `0xA800` code stores the random
pedestrian choice in `FFE183` before testing this equality and `FFFC3C`; a
failed later gate retains the selected choice through the `0xA800` return.
Higher-priority `FFE180` and `FFE182` branches run before the pedestrian gate
and select their own static lists (the `FFE180` branch additionally requires
`FFF0C8=4`).

The higher-priority `FFE180` list at `0xA8AE` is also a pedestrian source. Its
ROM entry begins `80 1D 69 88 00 00 A8 C6 00 00 00 02 9E 20 2E 80 9E 08`:
the tagged template resolves to `0x1D6988`, the position data is at `0xA8C6`
(`x=0x1D00`, `y=0x4100`), and its ordered overrides are
`+0x9E=0x20`, `+0x2E=0x80`, `+0x9E=0x08`. It is selected only when
`FFE180 != 0` and the active effect index `FFF0C8` is 4; this return precedes
the `FFE182` list and the random pedestrian branch. This is a separate static
list from the 69 scene/choice rows in `0x1EDB88`; it has no scene ID in this
branch. `world-activity-actors.tsv` now carries the A8AE actor in Scene 34,
and the field runtime passes active and selected index facts through Lua before
publishing its extracted template, position, overrides, and slot-3 record.

The `0x167AE` scene-entry path clears `FFF0C8` on resource entry, while the
elevator selector can later change that active index as it selects a child
scene. A direct `FFE180` producer exists:
service 3 at `0x53BCC` writes the marker at `0x53C3C` on its Scene-3/event-3
branch when FFFC1E bit 2 is set, bit 3 is clear, `FFF0D5` is zero, and FFFC30
bit 12 or FFFC38 bit 4/5 is set. It opens root `0xF4BB4`, then sets `FFE180`,
clears D6, and sets `FFF0CC=0xFF`. The same service's separate Scene-6/event-0
branch at `0x53C50` opens the Owlfeather repeat root `0xF4B84`.

The negative `FFF0CC` aborts the remaining indexed-service stages inside
`0x1F5CA`; the caller `0x75E` continues. With D6 clear and `FFFBF9=3`, it
calls `0x167AE`, whose event-table row selects resource Scene 30. Its profile
references group `0x1DB004`, whose header is inclusive maximum index 4 and
whose children are `[30,31,32,33,34]`. Lift index 4 selects Scene 34. The
actor descriptor's final `+0x9E=0x08` write satisfies the special-contact gate
in `0xECCC`; while `FFE180` is set, that routine clears it and dispatches
`0x53C90`, which sets story bit 3 (`ARAGORN_FOUND`) and opens root `0xF4BBC`
with portrait `0x121E08`. The `0xF4BB4` text identifies the after-hours
Mitsuhama route, and Notebook tip 70 identifies Aragorn's destination as its
fifth floor. Their joined route resolves the numeric destination as Scene 34.
Debug/Release QA now walks the Scene-3 after-hours route to Scene 30, traverses
to the Scene-30 elevator, selects index 4, and verifies Scene 34 and the A8AE
actor. The same Debug/Release QA crosses the record-90 door after the
quality-2 passkey branch and reaches A8AE by ordinary input. It verifies FFE180
consumption, story and mode flags, the actor one-shot bit, three `0xF4BBC`
pages, and profile `0x121E08`. After the last page, `0x53C90` calls `0x1DED8`;
that scan dispatches active records with `+0xB0` bit 7 to `0x1E514`, which
clears the escort and active bits. Debug/Release QA checks Aragorn's source
slot leaves the live list, the activity count drops by one while other records
remain active, and saves a closed Scene-34 image.
Debug/Release route QA restores the ROM lift placement `(0x3800,0x0600)`,
crosses Scene-34 record 92 (`0x1DB990`, event 1), and reaches the event-2
maglock at record 90 (`0x1DB97C`), opening the generic `0x20E48` door prompt.
The route seeds a QA-only quality-2 Maglock Passkey (`0x14`) in the live runner
and presses ordinary A after the source 10-frame choice gate. It checks the
success page and `0x4B` sound, holds the result for 30 source frames,
acknowledges with A, and verifies that Scene 34 returns with only record 90's
map cell changing `0x82 -> 0x81`; the passkey remains in inventory.
Debug/Release BMPs and action/map-delta traces match. This tests only the
quality-2 passkey success branch. Electronics and insufficient quality remain
open. No matching independent gameplay capture is available. The route also
exposed the delayed player hurt cue `0x40` after the runner returned to walking
animation; the C++ heartbeat validator now accepts that source-delayed cue on
the runner's behavior pass, and this QA requires a clean heartbeat result. The
special transition through
`0xC64C` and `0x2A32`/`0x2A44` clears `FFE180`; the ordinary resource entry and
elevator handoff preserve it until contact. The static list still has no scene
ID in `0xA800`.

The bounds come from the raw `0x3232` profile table: `0x167AE` follows each
profile's `+0x58` resource-bundle pointer, then the bundle's first longword to
the bound word sampled by `0x0D3A`. The scene groups are:

| Scene IDs | Bound for `FFE12C` | Values that pass the initial `FFF0C8=0` comparison |
|---|---:|---|
| `0x15..0x19` | 4 | `0` |
| `0x1A..0x1D` | 3 | `0` |
| `0x1E..0x22` | 4 | `0` |
| `0x23..0x28` | 5 | `0` |
| `0x29..0x2B` | 2 | `0` |

The sample range is therefore `0..bound-1`, and the pedestrian branch only
passes this gate when the selected resource index is zero. This is a source
state condition, not a fixed percentage of visits; the deterministic PRNG
seed and preceding calls decide whether a given scene entry passes.

The source reset points found so far are field-mode initialization at
`0x2A32`/`0x2A44`, request
consumption at `0x1E43C`, and the type-5 mode-exit path at `0xC684` when
`FFFC1C` bit 0 is set and bit 7 is clear. The top-level transition path at
`0x0A92` calls `0x1E43C` when `FFDF42` is pending; that routine clears
`FFE183`, scans actor records, and may start the `0x1F16A` world activity.
These transitions matter because `FFE183` is shared with the world-activity
request path; it cannot be treated as a fresh per-scene random roll. Lua's
`World.resolve_conditional_actor_selection` implements the A800 gate order and
retains FFE183 between calls. The field scene installs the destination
FFDF40 mode before actor population, matching the `0x167AE` resource-entry
state consumed by A800. It passes the raw offer initializer, effect markers,
FFFC3C, and separate 16-bit active/selected indices into Lua, then publishes a
selected candidate through the ordinary actor-slot allocator. The `0x0A92`
callback now calls Lua's atomic FFE183 consume before the typed `0x1E43C` scan.
Lua's other resets remain connected to the modeled `0x2A32`/`0x2A44` and
type-5 `0xC684` paths. Debug/Release script QA follows accepted Johnson offers
with initializers 5, 11, and 12 through A800 and the typed request scan. Field
QA walks to the Johnson contact, selects Work, accepts the generated offer,
and retains that state in the same `FieldSceneRuntime` through candidate
resource entry. Across all 46 reachable scene/choice pairs it verifies the
candidate in slot 3 and the exact default-list corp-guard count, profiles, and
slots 4 onward. Those expected counts come from
`assets/field/entities/default-actor-lists.tsv`; ROM `0x43B6` drains the A800
list before the default list, and `0x1A342` allocates the seven ordinary actor
records. Scene 22 choices 1/2 use initializers 11/12; the other cases use 5.
This covers the in-world Johnson-to-offer path and candidate/default actor
allocation. The 46 scene entries remain resource reentries, so travel to each
candidate scene is not covered by that matrix. Independent Genesis
identification remains open; the physical Scene-34 lift-to-A8AE route has its
own QA.

The ROM candidate extractor validates the ordered writes consumed by
`0x44BC` and emits the final `+0x9E` byte as typed
`source_spawn_state_flags`. The raw `ordered_overrides` string remains in the
offline TSV for source review and map inspection; `world_content.lua` and the
field runtime consume the typed flag and no longer parse that offset string.
Debug/Release script-runtime QA checks the typed flag on all 69 source rows,
and `extract_field_actor_candidates.py --check` verifies the table against the
US ROM.

The same Debug/Release QA now exercises both reachable A800 random choices
through Scene-30 event 29 and the physical lift to Scene 34. It starts from the
ROM profile checkpoint `(0x3A00,0x0600)`, with `FFE12C=4` / `FFF0C8=0` so each
choice is retained before the index gate. Source seeds `0x0000` and `0x8000`
select choices 1 and 2; each destination uses the opposing seed to verify the
stored choice survives the late gate. The `[30,31,32,33,34]` group at
`0x1DB004` advances to floor index 4 through ordinary Up/A/Start input, and
Scene 34 publishes the matching candidate list, source position, pedestrian
profile, and slot 3. The six route images and TSVs per choice plus the two
deferred captures match across Debug and Release. A separate `FFFC3C` case
sets the pending-spawn condition, samples choice 2 with equal index 0, and
publishes it through QA resource entry after the marker clears. These checks
cover both A800 random outcomes on the Scene-30-to-34 route; they do not drive
the accepted office offer through the lift or physically visit every
candidate map. The QA consumes `FFE183` through the typed `0x1E45E` callback
after publication.

The FFFC3C source trace is now explicit: `0xED60` copies the selected tagged
template from FFE110 into the marker on eligible contact; `0x286E` scans for
an active record with `+0xB0` bit 7 and otherwise passes that template to
`0x1E4AC`, which allocates and places the actor at Joshua `x+0x80` / `y`.
Every A800 candidate row points to root-tagged template `0x801D6988`. Qt now
retains a typed transfer descriptor, publishes typed marker availability to
pair collision, sets FFFC3C after eligible contact, and clears it with the
`0x1E514` escort cleanup. The physical Scene-34 A8AE QA checks the pending
pedestrian profile during dialogue and its clear afterward; the 24-bit Lua
contact route also accepts an A800 candidate list offset as generic activity.
The live `0x286E` guard now projects the source-eligible actor records,
detects an existing `+0xB0` bit-7 escort, and publishes the retained typed
FFE110 actor template at Joshua's `x+0x80/y` when no escort remains. It uses
the ordinary Lua actor heartbeat and actor/effect-slot allocation. Debug and
Release `field_pedestrian_scenes_qa` now physically walks Scene 30 choice 2,
list `0x1EDDDC` at `(0x0900,0x0B00)`, from the live runner position retained
through the accepted Johnson offer's resource-scene reentry. After the host
modal-return input release, ordinary Left/Down reaches `(0x08DA,0x0AD8)` in
141 field frames. Contact sets `+0xB0.7`, retains FFFC3C with the typed FFE110
transfer, and the player-spawn guard finds the existing actor without
duplication.
ROM `0xED60` installs callback `0x1F146` for the generic contact. The callback
sets FFFC1C bit 0 and presents text at `0xFE1F7` with portrait profile
`0x121FF4`; its no-options branch waits `0x1E` VBlanks before accepting a
controller event. Lua now presents that page through Mortimer Reed's registered
speaker profile. Debug/Release QA verifies the text, portrait, A dismissal,
restored Scene-30 field, actor retention, and FFFC3C transfer. The page and
post-dismissal BMPs plus route/waypoint TSVs match between builds. The later
request remains unconnected: the host main loop still needs FFDF42 before
`0x1E43C`, and its actor scan/`0x1F16A` continuation are open. QA checks and
consumes choice 2 at the `0x1E45E` seam without claiming automatic activity
continuation. The candidate matrix still enters scenes through QA resource
reentry; physical travel to the other candidates and independent Genesis proof
remain open. See the dedicated trace in
`conditional-pedestrian-runtime-gap-2026-10-26.md`; this lifecycle remains
partial.

Independent capture `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_41_source_10620-10815/frame_002100.png`
contains two small figures in the field view. A grayscale
`TM_CCOEFF_NORMED` match of the left 260x224 pixels against the extracted map
assets selects `mode-5.bmp` at offset `(895,800)` with score `0.9336`; the next
map result is `mode-2.bmp` at `0.5236`. This identifies the map resource as
Scene 5, but not either figure's source actor record.

A masked BGR `TM_CCORR_NORMED` sweep of the two visible sprite crops against
the extracted actor idle/walk frames ranks the left blue figure as
`actor-kind-9-palette-1-walk-dir-6-frame-0.bmp` (score `0.9447`); the best
kind-11 pedestrian template for that crop scores `0.5919`. The center brown
figure ranks as `actor-kind-3-palette-3-walk-dir-6-frame-0.bmp` (`0.8909`),
while its best kind-11 pedestrian template scores `0.7074`. The approximate
crop windows are `(0,82)..(34,132)` and `(88,92)..(126,139)` respectively.
This visual ranking favors other actor art over the kind-11 pedestrian, but
it does not identify either actor record, party membership, or the visible
RAM state; treat the classification as **[H]** rather than proof that Scene
5's pedestrians are absent.

**[C]** Scene 5's default list at `0x67994` contains three kind-11 pedestrian
rows using template `0x1D6988` and position data beginning at `0x679F0`.
Source `0x1684C` sets world mode 1 for scenes below `0x15`; the random
pedestrian branch at `0xA800` requires world mode 2 and indexes its candidate
table only for scenes `0x15..0x2B`. The screenshot therefore does not support
the claim that this figure came from the random candidate branch. It is
consistent with Scene 5's default pedestrian population, but the frame does
not reveal FFE180/FFE182, the actor record, or which visible figure is Joshua.
The 69 A800 positions remain candidate inputs pending a route-matched capture
from one of their source scenes and visible gate state.

All 69 choices contain one root-tagged `0x1D6988` pedestrian template (kind
11, palette line 3, no initial item/spell action), one position, and the same
ordered record overrides: `+0x9E=0x20`, `+0x2E=<heading byte>`, then
`+0x9E=0x08`. Preserve both writes to `+0x9E`; reducing them to an unordered
set would change the initialized record. The table below records the ROM
position and `+0x2E` value for each one-of-three choice. These are candidate
spawn inputs, not evidence that every one is live on every visit. Coordinates
and heading bytes in the table are hexadecimal.

| Scene | Choice 1: position / `+0x2E` | Choice 2: position / `+0x2E` | Choice 3: position / `+0x2E` |
|---|---|---|---|
| `0x15` | `2800,0580 / C0` | `4700,2D80 / C0` | `4800,1000 / 80` |
| `0x16` | `0300,0700 / 40` | `2300,2B80 / 40` | `0880,3480 / E0` |
| `0x17` | `0680,0580 / 80` | `1700,4980 / 40` | `4780,1000 / 80` |
| `0x18` | `0300,0700 / 40` | `2300,2B80 / 40` | `0880,3480 / E0` |
| `0x19` | `1780,2C80 / 80` | `3000,2C80 / 80` | `3B80,1000 / 40` |
| `0x1A` | `2000,0500 / A0` | `2D00,2300 / 60` | `3B00,2500 / 00` |
| `0x1B` | `1B00,0A00 / 80` | `4B00,1B80 / 80` | `4900,0F00 / 40` |
| `0x1C` | `1000,2500 / A0` | `2080,1100 / 80` | `4D00,2600 / 40` |
| `0x1D` | `1000,2500 / A0` | `2080,1100 / 80` | `4D00,2600 / 40` |
| `0x1E` | `2300,3400 / C0` | `0900,0B00 / 40` | `0A00,2600 / 20` |
| `0x1F` | `1800,0F00 / C0` | `0380,4880 / 00` | `3A00,3900 / C0` |
| `0x20` | `0500,1F00 / 40` | `2500,4F00 / 00` | `3A00,1500 / A0` |
| `0x21` | `0500,1F00 / 40` | `2500,4F00 / 00` | `3A00,1500 / A0` |
| `0x22` | `1800,0F00 / C0` | `0380,4880 / 00` | `3A00,3900 / C0` |
| `0x23` | `0400,0980 / 40` | `1B00,2880 / 20` | `5580,0600 / 80` |
| `0x24` | `0B00,1080 / 00` | `4F00,2300 / 00` | `1F00,2300 / 20` |
| `0x25` | `0B00,1080 / 00` | `4F00,2300 / 00` | `1F00,2300 / 20` |
| `0x26` | `0D00,0500 / 80` | `1500,3000 / 80` | `4E80,3F00 / 00` |
| `0x27` | `1600,2F00 / A0` | `1180,1400 / 80` | `2900,1B80 / 40` |
| `0x28` | `0300,0700 / 40` | `3380,3900 / C0` | `3000,2500 / 40` |
| `0x29` | `1500,0E80 / 00` | `3A00,4280 / 00` | `3A00,4280 / 00` |
| `0x2A` | `2000,0B00 / C0` | `2900,2C00 / C0` | `0500,3880 / 80` |
| `0x2B` | `2000,0B00 / C0` | `2900,2C00 / C0` | `0500,3880 / 80` |

This table does not supply scene-`0x14`'s physical-entry actors. In the ROM
transition, `0x1684C` sets world mode 1 for scene IDs below `0x15`; the
`0x1688E` scene-`0x14` branch exits before selecting the `0x16FD4` pending
list. The random-pedestrian branch in `0xA800` requires mode 2, and the
mode-entry initialization clears its higher-priority `FFE180`/`FFE182` flags.
Thus the scene-20 route's seven-row cap applies to its default list, while the
`0x15..0x2B` candidates remain unspawned there.

The converted helpers are `prepare_pending_entity_template_list` and
`select_pending_entity_template_list` in
`src/entities/entity_template_conversion.cpp`. The Qt field runtime now passes
the offer initializer, world mode, story markers, typed FFFC3C template
presence, and selected/active effect indices to Lua's A800 selector after
scene-entry setup. It queues the selected pending list before the default list,
matching `0x43B6` allocation order, and publishes the resulting actor through
the shared Lua actor path. Debug/Release selector and scene QA cover the state
gates and selected-list records; they do not yet cover the physical
randomized-candidate contact path or identify a candidate in an independent
Genesis frame. See
`conditional-pedestrian-runtime-gap-2026-10-26.md` for the live gate and spawn
lifecycle trace.

Spawn lists also are not always flat pointer pairs. After each template and
position pointer pair, `0x44BC` checks the following word for override mode
`2`. It then reads byte-offset/value pairs until a zero word. For example, the
scene pedestrian list at `0x1EDECC` contains overrides at record bytes
`0x9E`, `0x2E`, and `0x9E` before its zero terminator. An extractor must
preserve order because later writes can replace an earlier value at the same
offset.

The scene's default template list is a separate population source. Profile
entries at `0x323A` (scene 2, profile `0x172C80`) and `0x3282` (scene `0x14`,
profile `0x1D917A`) supply the long at word offsets 21–22 (`FFDDF0`) consumed
by the final `0x43B6` call to `0x4466`:

| Scene | Profile-selected list | Template | Position list | World position | Ordered byte override |
|---|---:|---:|---:|---:|---|
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D58` | `0x3480,0x2200` | `+0x9E=0x20` |
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D68` | `0x5300,0x3700` | `+0x9E=0x20` |
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D78` | `0x6280,0x1900` | `+0x9E=0x20` |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9248` | `0x0600,0x1100` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9252` | `0x0600,0x2300` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D925C` | `0x0800,0x3500` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9266` | `0x5400,0x0A00` | `+0x2E=0x40` |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D926C` | `0x6080,0x0A00` | `+0x2E=0xC0` |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D922E` | `0x1600,0x0580` | none |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D9238` | `0x3680,0x1800` | none |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D923E` | `0x0B00,0x2A80` | none |

The list sentinel is `0x172D4C` for scene 2 and `0x1D922A` for scene `0x14`.
The optional override marker is a 32-bit value `2`, followed by byte-offset /
byte-value pairs and a zero word; preserve the pair order. The tested alternate
scenes 6, 9, and `0x2C..0x31` have empty default lists. These records prove map
population, but not movement, animation, interaction, or damageability. Do
not treat presence in a map list as proof that an actor is a weapon target.

## Default-actor waypoint data and movement handoff

The position pointers above are route tables as well as initial positions.
`0x44BC` copies the pointer into live `+0xE0`, initializes the waypoint cursor
`+0xE4` to zero from the common record template at `0x7361A`, and copies the
first coordinate pair into the actor's current position. `0x142AE` advances
the cursor by one coordinate pair, reads the next X/Y target, and resets the
cursor to zero when the next X word is zero. A negative first X word selects
the player-target branch instead of a coordinate list; none of these default
route lists uses that sentinel. Scene default-list templates are root-tagged,
so `0x44BC` marks them with behavior bit 5, which is the source gate for the
route-list branch.

For route movement, `0x145A2` derives the target heading and distance, then
installs the `0xB574` continuation. The ordinary continuation steps the
current heading toward its target through `0xB530`, selects a directional
animation list, and calls `0xBADE`. That step calls `0xB77E` for a directional
terrain probe; `0xB7B8` handles blocked probes and map-event continuations,
and `0xBBE4` commits accepted positions. The separate `0x4618` render pass
advances the selected actor animation. Waypoint choice, movement policy, and
the behavior/render split therefore all need to reach Lua; a compositor-only
waypoint table would not move an actor.

The ROM coordinate pairs decoded from the profile-selected position pointers
are:

| Scene | Source order / actor | Position-list offset | Ordered route points (hex X,Y) |
|---|---|---:|---|
| 2 | 0 / pedestrian | `0x172D58` | `3480,2200` -> `2800,2200` -> `2800,3D80` -> `3480,3700` -> `5300,3700` -> `5300,2500` -> `5600,2500` -> `5600,1900` -> `6280,1900` -> `6280,0B80` -> `7300,1380` -> `0700,1380` |
| 2 | 1 / pedestrian | `0x172D68` | `5300,3700` -> `5300,2500` -> `5600,2500` -> `5600,1900` -> `6280,1900` -> `6280,0B80` -> `7300,1380` -> `0700,1380` |
| 2 | 2 / pedestrian | `0x172D78` | `6280,1900` -> `6280,0B80` -> `7300,1380` -> `0700,1380` |
| `0x14` | 0 / corp guard | `0x1D9248` | `0600,1100` -> `4F80,1100` |
| `0x14` | 1 / corp guard | `0x1D9252` | `0600,2300` -> `4F80,2300` |
| `0x14` | 2 / corp guard | `0x1D925C` | `0800,3500` -> `4F80,3500` |
| `0x14` | 3 / corp guard | `0x1D9266` | `5400,0A00` |
| `0x14` | 4 / corp guard | `0x1D926C` | `6080,0A00` |
| `0x14` | 5 / pedestrian | `0x1D922E` | `1600,0580` -> `1600,0A80` |
| `0x14` | 6 / pedestrian | `0x1D9238` | `3680,1800` |
| `0x14` | 7 / pedestrian | `0x1D923E` | `0B00,2A80` -> `0F00,2A80` |

The three scene-2 pointers share tails of one ROM coordinate stream. The
one-point scene-`0x14` lists have no additional destination before the zero
terminator. This is route-data evidence, not proof that every actor walks it
on every visit: the `0x13904` actor-state, target, world-activity, and
collision gates still decide which path runs. Lua resolves the extracted
route by source list and order. Debug/Release
`shadowrun_ghidra_conversion_qt_field_pedestrian_scenes_qa` checks one
non-pedestrian case: Scene 40 guard order 1 follows the ROM route at
`0x1DDE48` from `(0x2300,0x2580)` toward `(0x2A80,0x2580)`. This is an
internal runtime check, not an independent Genesis capture or proof that every
default-list actor moves.

The default list is bounded by the live record allocator. `0x43B6` prepares
pending lists first, then drains `FFE124`, `FFE120`, and finally the default
list at `FFDDF0`. Each `0x4466` entry calls `0x1A342`, which scans seven
ordinary world-actor records and returns carry when all are occupied; the
caller then returns from the current list. It also calls `0x1A2CC` for a
paired effect slot and stops if that allocation fails. With those records
free when the default list starts, the ROM can instantiate only its first
seven rows. The typed Qt projection now assigns those rows the enum identities
`slot_3` through `slot_9`; the eighth scene-`0x14` definition remains source
data but is inactive in that route. The separate `0xA800` pending-list gates
and their earlier occupancy are not represented by this projection, so this
does not claim complete actor-pool allocation for every world activity.

The live template type also selects the initial animation table. `0x44BC`
copies the template's first word into live `+0x0E`; this is kind 5 for the
corp-guard template and kind 11 for the pedestrian template. `0x600E` indexes
the state-pointer table at `0x62AC` with that kind, then uses the live `+0x16`
selector and `+0x1E` frame index. The default record at `0x7361A` initializes
both indices to zero. Kind 5's state-zero list is at `0x827C` and points to
frame descriptor `0x86E0A`; kind 11's is at `0x8D7C` and points to
`0xA7684`. Both state lists have marker 1, so their initial state repeats one
frame. The descriptors carry the actual image geometry and origin: the guard
frame is 24x32 at `(-13,-26)` and the pedestrian frame is 32x32 at
`(-14,-24)`. The template's `+0x02` word selects palette line 2 for guards and
line 3 for pedestrians. `0x44BC` copies that template word to entity byte
`+0x2C`; it is separate from `0x600E`'s selector and frame-index words at
`+0x16` and `+0x1E`. The profile overrides touch `+0x2E` or `+0x9E`, not
those animation words, so the state-zero frame choice is supported at
initialization. Those overrides may affect later live behavior that this
initial transition slice does not implement.

The entry behavior is separate from ordinary directional walking. `0xA8EA`
services the default records before the field camera and render pass;
`0xAC84` sends records with the default behavior flags through `0x13904`.
For the supported default-record path, `0x13904` subtracts the initial
`+0xB6` delay (the low nibble of the source PRNG result, 0 through 15), then
checks the zero `+0x16` animation selector and zero `+0x9C` state. It sets the
entry flag and calls `0xB330`. `0xB330` requests the kind-indexed cue from
`0xB05E`, selects the directional C0B2 list, and resets the animation state.
Kind 5 requests event `0x2E`; kind 11 requests `0x2F`. The audio tables map
these to silent table-2 slots 46 and 47, so the source makes a request but no
audible sample or playback duration is defined for either cue.

The selected C0B2 lists for the used directions are finite two-frame lists
(`0x8002` marker, `0x0200` increment, `0x1000` period). The initial idle
increment contributes a `-16` accumulator carry on the activation frame;
then each transition pose remains for nine field updates. The final pose stays
selected while the 20-count behavior timer is serviced; the next zero check
completes on the 21st update after activation. This covers only the observed
initial transition. `0xAB02` and later action, movement, collision, and damage
branches remain outside the Lua heartbeat slice.

The offline actor table now gives each active record a stable Lua profile ID
built from the source template name, entity kind, action-table class, and
action code. It also classifies the template byte at `+8`: a clear sign bit
selects the D4540 item-action table, a set sign bit selects the D4EA0 spell
table, and code zero means no active action. Those are typed data inputs; the
profile ID contains no runtime ROM address.

The Qt actor service passes that identity, action class/code, instance ID,
initial delay, and source heading to `gameplay/scripts/actor/heartbeat.lua`. Lua
selects the `corp_guard_k05_item06` package for the kind-5 guard and
`pedestrian_k0b_none00` for the kind-11 pedestrian. The guard package binds
item 6 (Max-Power / Heavy Pistol); the pedestrian has no active item action.
Each actor now receives a Lua heartbeat on each field behavior-service pass.
The script owns the entry delay, directional C0B2 choice, two-frame timer,
terminal hold, and cue request; C++ stores only the latest returned pose for
rendering and forwards the cue to the audio host. This replaces the prior
`FieldActorBehaviorState` C++ timer/animation implementation for this entry
branch. Combat targeting, attack decisions, actual NPC weapon firing,
collision outcomes, and damage are not implemented by these profiles yet. Lua
route movement is enabled for default-list records with extracted waypoint
data; other NPC movement and collision branches remain open.

The existing field entity catalog is not these actors: the importer currently
decodes the live Joshua template at `0x7291A` (kind 3). `0x4658` updates the
sixteen entity records and calls `0x481E`; `0x481E` separately refreshes ten
projection slots from `FF164C`. Its `0x5F78` geometry must not be substituted
for the kind-5/kind-11 animation frames or treated as their full sprite art.

## Lua default-actor heartbeat slice

The offline `shadowrun_graphics_extract --all-assets` step writes
`field/entities/default-actors.tsv` by walking the profile-selected lists at
`0x172D1C` and `0x1D91DA`. It preserves list order, world position,
template/position/list provenance, source-derived Lua profile ID, item/spell/
none action class and code, palette line, source heading, and ordered
overrides. It writes individually named idle
images and two C0B2 transition frames for each kind and direction used by
these lists. Scene 2 uses direction
0; scene `0x14` also uses directions 2 and 6 because source-order records 3 and
4 override heading at `+0x2E`. Images preserve signed descriptor origins and
are direct assets consumed by the runtime; no ROM or VDP state is used at
runtime.

The runtime population is never inferred from gameplay-video frames. The
extracted rows are ROM list entries with named `corp_guard` or `pedestrian`
templates, source positions, and initial behavior data; no corpse-specific
template or death-state placement is present in this table. New Game's first
scene-2 entry keeps those three profile rows inactive, as the ROM lifecycle
does. A later scene entry may activate only the rows belonging to that scene.
Until the actor death/drop path is connected, these live profile rows must not
be converted into dead-body scenery or supplemented with actors seen in a
longplay.

New Game's `0x075E` setup enters scene 2 and performs its warmup without the
later `0x02900 -> 0x04388 -> 0x043B6` world-frame list-finalization path. The
Qt bootstrap therefore keeps extracted scene-2 definitions inactive and draws
no profile actors on the opening street. On an actual scene change,
`FieldSceneRuntime` activates that destination's profile list and resets each
Lua instance by stable scene/source-order ID. Each field behavior-service pass
calls Lua once per active profile actor. The compositor sorts actors by
effective Y depth with source order as the stable tie-break, draws the
Lua-selected idle or transition frame, then redraws the shared HUD. The
The scene-`0x14` physical-entry QA now runs against this Lua ownership path.
It captures entry, transition, and post-entry frames, and checks Lua actor
snapshots, audio requests, full-screen UI state, and visible guard pixels at
each named checkpoint. Those internal Debug/Release captures establish
renderer consistency only; they are not an independent Genesis visual golden.

The dedicated C++ `FieldActorBehaviorState` and its test target have been
removed. The broader AI source trace, exact call cadence, animation/render
split, and open movement/collision branches are recorded in
[`actor-ai-heartbeat-source-audit.md`](../actor-ai-heartbeat-source-audit.md).
No new visual correctness claim is made from the old screenshots, and an
independent Genesis match is still unavailable.

## Live-route reachability check

The `0x13` result is a source mode-table result, not an ordinary scene-entry
spawn rule. ROM `0x2CE0` consumes `FFFBF9` as the event column and `FFFC10` as
the transition context. `0x2A94` initializes that context to `3` for mode 9;
`0x136DE` clears it for scene 6; `0x03032` supplies the alternate-scene gate.
The typed conversion test's `FFFBF9=8`, `FFFC10=0x14` pair does select mode
`0x13` and scene `0x2F`, but it is only a table test, not a reproduced player
route.

I checked the intervening transition states against the extracted collision
rectangles. From context 3, event codes `10, 10, 9, 8` can advance the table
through contexts `0x10, 0x12, 0x15, 0x14`, entering scenes `0x2D, 0x30, 0x2D,
0x2F`. A further `0x2CE0` invocation with event code `8` would be required for
the tested mode `0x13` result. Scene `0x2F`'s terrain rectangles contain event
code `9` only, so they do not directly publish `8`. However, `0x2CE0` leaves
`FFFBF9` unchanged; another transition request could reuse the prior value.
That route has not been traced, so this rectangle check does not prove that
mode `0x13` is unreachable.

The Qt runtime still does not provide the generic mode-`0x13` activity path:
its field coordinator callback for `advance_field_mode_transition` is
unbound, its `mode_transition_gate` does not receive the scene classifier
result, and its entity-population callback remains an explicit source
boundary. The coordinator's current `field_mode` value comes from the scene
resource mode, so it cannot stand in for `FFFBF9`. Separately, the two
profile-selected default lists at `0x172D1C` and `0x1D91DA` are extracted as
named typed data and their state-zero and initial-transition images are drawn
by the live Qt field compositor. That presentation does not allocate or update
the ROM's live entity records. Keep special list `0x1E552` separate until an
actual event path selects it.

## Scene-`0x14` post-entry completion branch

The source path after the two-frame transition is not a generic idle callback.
`0xA8EA` reaches `0xAB02` from five completion branches. For these default-list
actors, the initialized service byte at `+0x10` is `0x05`, so bit 3 is clear
and `0xAB02` calls `0x16B98`. The weighted-spawn routine first handles special
kind 7/12/13 cases; for ordinary kinds it admits only world modes 0 and 2.
The scene-`0x14` route is world mode 1 (`0x1684C`), so `0x16B98` returns before
checking projection capacity, selecting a random table row, or creating an
additional scene entity. Control returns to `0xAB02`, which sets service bit 6
on the completing actor. The spawn countdown and deferred-activity path only
run if the spawned record returns `+0x9E` bit 6; this route does not set that
bit. The linked-record propagation and field activity countdown gates are
also not activated by these default records. In particular, completion does
not request the entry sound a second time.

The Qt physical-entry QA advances the source behavior loop through ticks 41
to 82 after the transition checkpoint at 40. At ticks 61 and 82 it confirms
seven active profile actors, two visible guards, seven completed entry
transitions, the guard transition frame still selected, unchanged entry cue
counts, and no dialogue/shop/field prompt overlay. Two full-screen guard
regions are pixel-identical to tick 40. This is evidence that the tested Qt
route remains stable through the traced no-spawn branch; it does not establish
general NPC idle, movement, attack, or independent-ROM visual fidelity.

## Visual verification questions

1. **Does it look correct?** New Game's whole-screen field image contains the
   runner, map, and HUD, without drawing inactive profile actors over it. The
   scene-`0x14` full-screen captures show visible guards at source-projected
   positions, with no opaque crop box around either sprite. The independent
   longplay has supporting field-character examples but does not identify
   this exact scene or pose.
2. **Does the current image reflect behavior?** The trace ties source
   directions 2 and 6 to records 3 and 4. The typed table has five
   `corp_guard_k05_item06` rows (item action 6) and three
   `pedestrian_k0b_none00` rows (no action). The scene-`0x14` default-list
   route has seven available actor records, so Lua receives five guards and
   the first two pedestrians; the final pedestrian row is inactive. QA checks
   slots 3 through 9, seven starts and completions, and the visible guards'
   animation frame at each captured checkpoint.
3. **Are the visuals correct?** Named assets use the traced kind, selector,
   palette, descriptor, frame, and signed origin. The exact live projection
   slot assignment through `0x481E` remains unresolved.
4. **Is it animated, and when?** Each active row has a delay of zero through
   fifteen field updates;
   each of two C0B2 frames lasts nine updates. The final frame
   stays selected through the 21-update behavior check. The inactive eighth
   row gets no behavior or render heartbeat. For this mode-1 route, the
   completed transition's weighted-spawn branch returns without adding an
   actor. Later actor states are still open.
5. **Is there a sound effect, and how long?** Entry requests `0x2E` or `0x2F`
   by actor kind. Both map to silent slots, so neither has audible playback
   duration. Other actor sounds are unresolved.
6. **Are the rest of the screen and state correct?** All 45 current full-screen
   route captures were reviewed at 320x224 across five contact sheets, with
   the two post-entry frames also inspected individually. Map, runner, HUD,
   and prompt/menu state were checked together; Debug and Release BMP hashes
   match for all 45 captures.
7. **Do multiple frames support the interpretation?** The route includes
   entry and frames 8, 30, 40, 61, and 82, with matching typed state
   checkpoints; the actor-package trace records the seven source slots and
   inactive eighth row. Frame 30 differs from entry; frame 40 reaches the
   completed transition, and frames 61 and 82 retain the same guard regions
   and cue counts. No same-scene Genesis visual golden is available.
8. **Are new graphics drawn over stale UI?** Scene loading replaces the map;
   active actors draw in the map's entity pass; the HUD is redrawn afterwards.
   Full-screen routes confirm the previous panel and controls are gone. The
   bootstrap path leaves unspawned rows inactive, rather than painting actors
   over the initial street.

## Next implementation slice

The named default-list data and the initial idle-to-C0B2 transition reach the
Qt field compositor. Lua route patrol also runs for default-list actors with
extracted waypoints; the Scene 40 guard check covers one path through
`0x142AE`. Continue with the other `0x13904` branches, post-transition
`0xAB02`, the live `0x4658` record update, and kind-specific action dispatch.
Keep actor targeting outside combat until collision, hit, damage, and
retirement paths are traced and connected. The hill remains partial; exact
independent Genesis captures for these scene actors are not identified.

## Map-viewer actor population evidence

The standalone map viewer now imports the profile-selected default actor list
for all 56 scene descriptors. ROM `0x3084` copies the scene profile into RAM;
the dword at descriptor offset `+0x2A` becomes `FFDDF0`, which `0x43B6`
consumes as the default list. The offline graphics importer writes one row per
scene to `assets/field/entities/default-actor-lists.tsv`,
including a zero count for empty lists, and expands every list into
`default-actors.tsv` while preserving source order, named template, source
kind/palette, position, and ordered overrides. Populated lists are in scenes
0-5, 20-43, 54, and 55. The census finds pedestrians in scenes 0-5, corp guards
and pedestrians in scene 20, corp guards in scenes 21-43, six Yakuza Weenr in
scene 54, and Thon plus two Yakuza Weenr in scene 55. Those rows describe
profile data; their actual activation still depends on the mode transition,
world activity, and actor-slot allocation paths.

The existing offline importer
`decompilation/ghidra_conversion/tools/rom_research/extract_field_actor_candidates.py` follows the separate
scene pointer table for `0x15..0x2B`, then each scene's three list pointers. It
checks that all 69 entries are one root-tagged kind-11 pedestrian, each list
has one position, the ordered overrides remain `+0x9E=0x20`,
`+0x2E=<heading>`, `+0x9E=0x08`, and the source list terminates after that
record. `--check` re-reads the US ROM and compares the result with
`assets/field/entities/conditional-actor-candidates.tsv`.

The viewer keeps gated candidates separate from default-list rows and authored
spawn marks. It derives the eight-way pose using the `0xB36A` rotate/round
direction operation and uses the corresponding source-extracted actor frame
and origin. Candidate markers are faded and dashed because they are mutually
exclusive, gated spawn inputs rather than proof of active NPCs. Idle sprites
for actor kinds 5, 8, and 11 are extracted from their kind/palette state tables.
Kind 15 (Thon) resolves through a source state program at `0xA77A`, not the
ordinary `0x8002` bitmap descriptor used by the actor-frame importer. The
eight `0xC052` selectors all reach the same one-frame state at `0x7884`, which
points at the seven-record sprite list `0xA77A`. `0x622C`/`0x624C` expand this
as raw VDP sprite records; the actor template supplies palette line 1. On the
distinct marker-2 route to scene 55, the mode transition sets `FFE186=2`;
`0x43B6` then calls `0x5BA4C`, which uploads stream `0x1EBDEE` to VRAM
`0x5920`. That upload covers Thon's referenced pattern tiles `0x429..0x45E`.
The Release graphics importer combines the seven pieces with the ROM palette
and source offsets into `field/entities/actors/thon-idle.bmp`; the scene-55
viewer now uses that named sprite instead of an unresolved marker. The three
known world-activity lists are indexed separately for scene 47; other timed
activity data and event-created actor paths remain source-tracing work. None
of these conditional rows are presented as ambient map NPCs.

The offline importer now also expands each profile actor's complete ordered
12.4 coordinate list into `default-actor-waypoints.tsv`. ROM `0x44BC` stores
the list pointer on the active actor and initializes its cursor; `0x142AE`
advances the cursor and wraps on a zero X word. The viewer's optional **ROM
waypoint paths** layer draws those points in source order, while the actor
sprite itself remains at the first point. These lines describe a ROM route,
not an actor known to be alive or moving on every visit. The full import
produced 379 points for 75 default-list actor records across 32 populated
scene profiles; all 56 profile lists, including empty ones, remain indexed.
One scene-35 guard coordinate is outside that map image and is therefore
retained in the Map Profile census rather than drawn over the wrong map area.
The Release importer completed with 1,668 assets, 21 blank descriptors, and
zero missing assets. Viewer snapshots with route lines were inspected for
scenes 2, 20, and 55. A 56-map viewer census now finds 155 actor marks across
33 scenes with zero missing sprite files; the other 23 maps have no in-bounds
actor rows in the indexed source profiles.

The world-activity importer records all 13 actors from lists `0x1E552`,
`0x1E5C4`, and `0x1E628` with their list group, template name and kind,
palette, action code, position, heading, ordered overrides, and activation
rule. `0x2CE0` queues `0x1E552` when entering mode `0x13` with the recorded
`FFFC1E` bit clear; the other two remain separately labeled pending-list
continuations at `0x43B6`. Kind-7/palette-2 directional frames are extracted
from the source state table and shared by `eye5r 1` and `Strike Team`, whose
templates both identify that render kind. The **Special actor lists** viewer
layer is off by default, and its selector shows one source list at a time by
default. The optional all-groups view is for comparing candidate positions;
it does not imply that the ROM runs all three lists together. Scene 47's
special-list overlay was reviewed in the standalone viewer.
