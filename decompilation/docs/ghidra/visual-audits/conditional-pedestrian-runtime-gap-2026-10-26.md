# Conditional pedestrian runtime gap audit

**Status:** `FieldSceneRuntime` now calls the Lua `0xA800` selector with the
raw active run initializer, FFDF40, FFE180/FFE182, FFFC3C, and typed FFE12C /
FFF0C8 indices. It queues and publishes the selected A8AE story actor, and it
can turn a randomized candidate row into an ordinary actor descriptor. The
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
`scene-34-contact-route-probe/`. The physical movement stops at this lock;
the passkey branch is now exercised with a QA-seeded live-runner item `0x14`,
quality 2, and ordinary A after the prompt's source 10-frame choice gate. The
route checks the result page and `0x4B` sound through its 30-frame wait, then
acknowledges it with A and verifies the runner returns to Scene 34 with only
record 90's collision-map cell changing from `0x82` to `0x81`; the seeded item
and quality remain in the runner inventory. Debug/Release captures include the
passkey action/result/wait pages, field return, `passkey-action.tsv`, and
`passkey-map-delta.tsv`; the BMPs have matching hashes. This covers the
quality-2 success branch only. Electronics, insufficient passkey quality, and
the remaining route to Aragorn have not been exercised. The A8AE dialogue QA
still injects proximity to test `0xECCC`, the
Lua `0x53C90` graph, and the three `0xF4BBC` pages. Other live marker
producers and the random-candidate path through an accepted initializer-5
offer remain open. Section 41 has no route-matched visual check.

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

### Meaning of the `FFFBED == 5` gate

The comparison at `0xA84C` is confirmed. The writer at `0x1E6D6` copies the
first byte of the selected six-byte offer record in `0xF3EBC` to `FFFBED`; the
record is chosen from context `FFF0AE`. This is the raw selected offer
initializer ID (`source_initializer` in `data/johnson_run_offers.lua`), not a
separate normalized run-type byte. `0x1E71C` indexes the initializer table at
`0xF3DC6` with it. The `0xA800` branch compares the byte exactly to 5.

Lua maps initializer IDs 5, 11, and 12 to canonical
`RunType.EMPLOYEE_RECRUITMENT`, but the `0xA800` predicate admits only ID 5.
The only non-reset byte assignment to `FFFBED` is at `0x1E6D6`; `0x1E68C`
resets the containing word to `0xFFFF`. No source assignment normalizes IDs
11 or 12 to 5. This gate covers the
base Employee Recruitment offer initializer only; it does not cover the
two-site or Renraku variants just because they share a canonical Lua run type.
Candidate metadata preserves the raw initializer gate.

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

## Active runtime boundary

`FieldSceneRuntime::initialize_default_field_actors` now calls
`GameScriptRuntime::resolve_conditional_actor_selection` after the explicit
route-owned lists. It passes the active offer's raw initializer, the current
FFDF40 mode, FFE180/FFE182, FFFC3C, and separate optional 16-bit FFE12C and
FFF0C8 values. The one-shot FFE183 choice remains Lua-owned. The random
callback uses the field PRNG's bound sampler, and Lua calls it only after the
marker, initializer, and mode gates pass.

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
Scene-1 and Scene-3 default pedestrians, then seeds the high-priority story
marker and floor index 4. It verifies one A8AE actor at `(0x1D00,0x4100)` in
actor slot 3, checks the profile and draw visibility, and writes
`qt_runtime/field-pedestrian-scenes-qa/scene-34-story-contact-actual.bmp` in
each build. The Debug and Release BMPs have the same SHA-256. This test
exercises the selector-to-actor projection and internal rendering; it does
not establish that the real story route sets those facts correctly.
Gameplay-script QA still covers raw initializer 5 versus variants 11/12,
both reachable rolls, late-gate choice retention, and unequal/equal 16-bit
effect indices.

`world_effect_active` still initializes false, but the Scene-3/event-3 entry
graph sets it true after the `0xF4BB4` page. The ordinary-input route now walks
record 95, enters Scene 30, uses the lift to reach Scene 34 at active index 4,
and verifies the A8AE actor at `(0x1D00,0x4100)`. The A8AE contact path is now
connected: the field runtime forwards the source list and selected ECCC handler
to Lua, which chooses the contact graph only for list `0xA8AE` with FFE180
priority. The graph consumes FFE180, sets FFFC1E bit 3, clears FFFC30 bits
10-12 and sets bit 13, clears FFFC38 bits 4-5, then presents the three records
from `0xF4BBC` with portrait profile `0x121E08`. Debug/Release QA injects player
proximity at the extracted actor coordinates, advances all three pages with A,
checks the flags and actor one-shot bit, and writes
`scene-34-aragorn-contact-{actual,reply,outcome}.bmp`; the opening-page images
have matching SHA-256 hashes. The approach injection is a test seam, not proof
of a traversable route to the actor. Ordinary input now reaches the record-90
event-2 maglock after crossing record 92; neither passage through that lock nor
the remaining walk to A8AE is established. The `0x1E45E` FFE183 clear is
connected at the `0x0A92` request callback: Lua returns and clears the choice
atomically, and the callback passes that byte into the typed 13-record
`0x1E43C` scan.

Gameplay-script QA reaches Matchstick's accepted Employee Recruitment offer
through Johnson's work menu, checks raw initializer 5, samples a reachable
choice, consumes it once, and starts the typed request. Field pedestrian QA
seeds that accepted source row, enters Scene 21 with the equal-index and
no-pending-spawn gates, and verifies one visible slot-3 candidate at a ROM
choice-1 or choice-2 position; it writes
`scene-21-random-pedestrian-actual.bmp`. The independent offer path and the
field projection path are separate checks. No independent Genesis capture
identifies this candidate, Scene 34, the A8AE actor, or the contact conversation;
the BMPs remain internal runtime regression evidence only.

## Independent capture review

The independent longplay dump in
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_41_source_10620-10815/`
contains these inspected frames:

| Frame | Capture time | Observation |
|---|---|---|
| `frame_002100.png` | `02:58:10.000` | Blue and brown field sprites are visible. |
| `frame_002110.png` | `02:58:10.333` | Another sprite is visible at the right edge while the camera follows the party. |
| `frame_002120.png` | `02:58:10.667` | The party and surrounding sprites have shifted on screen with the camera. |
| `frame_002140.png` | `02:58:11.333` | Several sprites remain in view during movement. |
| `frame_002150.png` | `02:58:11.667` | The sprites overlap near the center of the field. |

At original resolution, frame `002100` shows a small brown standing sprite on
the blue map with another sprite at left and the party HUD at right. Its broad
silhouette and palette resemble the extracted
`pedestrian_k0b_none00` idle art, but the on-screen sprite is too small for a
unique match. The capture contains no scene ID, run-type value, resource index,
or actor-record identity; the surrounding sprites may also be party members.
The sequence cannot establish that the brown sprite came from one of the
`0xA800` lists. Keep it as a visual lead until a route-matched state trace or
actor-record capture identifies the candidate. The same contact sheet shows
Tarislar City Inn dialogue later in Section 41; it does not show the
Mitsuhama after-hours page or identify Scene 30/34. The new
`field-mitsuhama-after-hours-entry-qa/actual.bmp`, `scene-30-entry.bmp`, and
`scene-34-aragorn-contact-{actual,reply,outcome}.bmp` are internal regression
captures only. I inspected the entry and contact pages: their extracted text,
MORE prompts, 56x72 portraits, and three-page sequence are visible; Debug and
Release contact opening images match byte-for-byte. Neither capture is an
independent Genesis visual match.

## Next source-backed slice

The post-page effect is now traced: `0x53C90` calls `0x1DED8`, which scans
`FF0400..FF0A00` and dispatches active actor records with `+0xB0` bit 7 to
`0x1E514`; that handler clears the escort and active bits. The Lua action queues
this source-record cleanup, and the host consumes it before the modal return
fade. Debug/Release QA checks one retirement and saves a closed Scene-34 BMP.
This verifies record removal; it does not claim a walking animation or path.

Next, cover the record-90 Electronics outcome and insufficient-passkey branch,
then continue the Scene-34 route to A8AE without coordinate injection. Cover
the Electronics results with the live runner's class, skill, inventory, and
source RNG inputs. Exercise
the random branch through an accepted raw initializer-5 offer, including offer
variants 11/12, player-spawn pending, index mismatch, and choice persistence
across a child-scene transition. Finally, match a real capture to the selected
scene and live actor record; the current internal BMPs are regression
artifacts, not an independent visual oracle. The pedestrian population hill
remains partial.
