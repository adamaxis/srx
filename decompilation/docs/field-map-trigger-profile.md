# ROM Profile: Field Map Doors, Collision Triggers, and Routes

Date: 2026-10-08

**Status:** the source inventory is complete for the 56 field-scene profiles
and every high-bit collision rectangle. Native route coverage remains partial:
the document separates proven route meanings from numeric destinations and
unbound continuation handlers.

## Primary source and reproducible inventory

The US ROM is authoritative. This profile starts at its 56-entry scene pointer
table at `0x3232`; each four-byte entry selects a field profile. The profile
stores the collision-record pointer at `+0x26` and the optional 32-byte event
table pointer at `+0x4E`. Each collision record is five big-endian words:
inclusive X start/end, inclusive Y start/end, and collision/event value. The
first word with its sign bit set ends the collision-record list.

I added an offline audit utility, [`profile_field_map_triggers.py`](../tools/profile_field_map_triggers.py),
which reads the ROM and verifies the named extracted assets before writing the
inventories. Run it from the repository root:

```powershell
python decompilation/tools/profile_field_map_triggers.py
```

The run verified all 56 scene pointers and descriptors, all 5,163 collision
record rows (51,630 bytes), and all 10 present event tables against the ROM.
The 56 scene IDs reference 54 unique profile addresses and 50 unique collision
streams. Those streams contain 4,381 unique ROM collision-record addresses.
The event census has 892 scene-reference rows at 699 unique high-bit record
addresses. Scenes 22/24 and 36/37 share full profiles; collision streams are
also shared by scenes 28/29, 31/34, 32/33, and 42/43. The inventory writes:

- [`field-map-scene-profiles.tsv`](field-map-scene-profiles.tsv): every scene's
  table/profile addresses, profile/collision alias groups, sentinel, record
  count, event table address, and per-event counts.
- [`field-map-trigger-records.tsv`](field-map-trigger-records.tsv): one row
  per high-bit collision record, including exact ROM record address, inclusive
  world-coordinate bounds, 32-unit collision-grid cells, raw value, event ID,
  profile/collision alias groups, table byte, dispatcher branch, ROM handler
  addresses, and corresponding converted C++ functions/runtime owners. Its
  final column records a known physical-input QA route or explicitly says no
  focused route is recorded.

The 892 rows are **event-bearing collision rectangles**, not 892 named doors.
Some mark a special terrain cell, an elevator selector, or a generic event
route. A rectangle only activates the interaction path when the movement
collision and dispatcher gates are satisfied. Low-bit records still define
the walls and paths around those triggers; all 5,163 records are preserved in
the named collision assets and indexed by per-scene counts/pointers.

## How the ROM routes a collision

1. `0x3232` selects the scene profile. `0x03074`/`0x03084` prepare the active
   field profile and map layers.
2. `0x0EE10` clears and rebuilds the 64-by-64 collision map. It rasterizes each
   rectangle inclusively in `0x20` coordinate steps. The TSV's grid-cell
   columns use that same 32-unit step; the raw columns preserve the ROM words.
   The map slot is `((((y & 0xFFE0) << 1) + (x >> 5)) & 0x0FFF)`.
3. The terrain sampler at `0xE7C8` and directional entity probe at `0xB77E`
   produce collision results. The `0xB7B8` dispatcher only proceeds for the
   correct entity behavior/mode, an allowed scene or world-mode gate, and at
   least two negative directional probes. It reads the raw collision value
   from `FFDF64` and derives `event_id = raw_value & 0x1F`.
4. `0xB7B8` takes direct branches for several event IDs. Other values are
   published to the field-exit/world-activity path; the current scene's event
   table supplies a byte which `0x167AE` interprets as a field, resource, or
   scripted activity selector. A table byte is therefore not automatically a
   building name or an interior map.

The event table has 32 bytes, but a direct branch can bypass the matching
table byte. The inventory keeps both the raw byte at that index and the
dispatcher decision visible so the byte is not mistaken for a destination.
Timeout and pending-descriptor state can also replace the selected value before
scene preparation. `0x167AE` classifies the generic table value as follows:

| Table byte | ROM meaning |
|---|---|
| `0x00..0x14` | Field scene ID. Scene `0x14` has a separate flag/timeout gate; scenes `0x0D` and `0x0F` also have source flag checks. |
| `0x15..0x2B` | Resource-activity scene ID. Its profile supplies the random-index bound and resource cursor. |
| `0x2C..0x37` | Scripted-activity scene ID. |
| `0x38..0xFF` | Outside the accepted scene ranges; the source routine halts at its range guard. |

The 892 scene-reference rows for event-bearing rectangles are distributed as
follows; counts repeat a ROM record when multiple scene IDs share its profile
or collision stream. Event codes `0x0C..0x0F` and `0x11..0x17` do not occur.

| Event IDs `0x00..0x17` (generic or conditional) | Count | Event IDs `0x18..0x1F` (direct family) | Count |
|---|---:|---|---:|
| `00=55, 01=175, 02=95, 03=71, 04=48, 05=16, 06=6, 07=6, 08=13, 09=13, 0A=6, 0B=3, 10=14` | 521 | `18=40, 19=1, 1A=168, 1B=1, 1C=36, 1D=46, 1E=12, 1F=67` | 371 |

## Interpreted routes

These are routes established by ROM collision records and dispatcher flow.
Scene numbers are kept numeric when the ROM evidence does not identify a
place-name or an interior.

### Secondary map-name cross-check (added 2026-10-10)

Mike's RPG Center's [Redmond Barrens map](mikes-rpg-center-shadowrun-reference-notes.md#redmond-rom-coordinate-cross-check-rats-nest-lead)
provides a useful external label crosswalk. A proportional comparison of its
480-by-320 map image with the scene-2 trigger coordinates places records 91,
92, 93, and 97 near Stoker's Motel, Ares, Little Chiba, and Rat's Nest. This
supports identifying scene 2 as Redmond Barrens, but these names are not
stored in the ROM. The same guide distinguishes Stoker's Motel (marker 6)
from Rat's Nest (marker 9). Its Rat's Nest item prerequisite is a clue to
check against ROM story variables, not a runtime rule by itself. See the
[full site notes](mikes-rpg-center-shadowrun-reference-notes.md) for the
external-source inventory and unresolved questions.

The complete Scene-2 record-by-record crosswalk is in the [Redmond coordinate
audit](mikes-rpg-center-shadowrun-reference-notes.md#redmond-rom-coordinate-cross-check-jump-house-and-rats-nest).
It compares all 13 event-bearing records (85-97) with the guide's numbered
map, while keeping the ROM event-table byte and any special handler separate
from the guide's place-name. Several records select numeric scene 26, which
the ROM classifies as resource activity; none of those bytes says “office.”

| Starting map/record | ROM event and route | Result established by ROM | Current Lua/native connection |
|---|---|---|---|
| Scene 1, record 67 at `0x17D9A2`, raw `0x00C7`, event 7 | Event-table row 7 at `0x17D620` defaults to `0x0D`; indexed title service 21 at `0x5447E` runs before generic route selection | The service handles field mode 1/event 7. `FFF0D5` selects one of two package-text paths; it does not gate the service. With it clear, `FFFC1E` bit 30 and bit 21 select the guarded, first-entry, or repeat passage. First entry sets bit 21 and adds 2 to three actor phase counters. A nonzero `FFF0CC` result sets `FFE186=1`; since event 7 is below 8, the main loop jumps directly to selector `0x167AE` without changing the marker, which selects scene 54. Pending `FFE162` descriptors take precedence and select scene 15. The scene-55/Tomb routine `0x20938` is not reached on this event-7 marker-1 branch. Dispatcher index `0x44` reaches `0x21FD0`, which sets bit 30 and loads profile `0x121F4C` alongside text beginning “You are taken to Frosty”; the upstream choice route remains unconnected. | Not connected in `building.lua` or the live indexed-title service. The trigger record and site crosswalk are source-profiled only; no physical-input QA or `actual.bmp` exists. See [the Tarislar route audit](visual-audits/puyallup-tarislar-apartments.md). |
| Scene 2, records 89/90 at `0x173124` / `0x17312E`, raw `0x00C0`, event 0 | Scene 2 event-table row 0 at `0x172CDC` is `0x14` | Selects numeric field scene 20; entry then encounters the scene-20 interaction geometry. No place-name is inferred from the byte. | `building.lua` selects ROM row 0 and `run_world_activity_routine` loads scene 20; physical New Game route reaches record 117. |
| Scene 2, record 85 at `0x1730FC`, raw `0x00C4`, event 4 | Conditional-maglock dispatcher falls back to event-table row 4 at `0x172CE0` (scene `0x0D`); indexed title service 22 selects entry descriptor `0x110D1E -> 0x116C12` at `0x20484..0x204C6` when `FFF0D5` is clear | Mike's marker 1 labels Jump House and Gunderson. Frame 001200 independently shows the location description and its portrait before the scene-13 map. The ROM stores numeric scene and trigger values, not the building name. | `building.lua` opens the generic Lua entry task. The route retains the typed scene-13 destination until ordinary A dismisses the description. Debug CTest `shadowrun_ghidra_conversion_qt_field_jump_house_entry_qa` starts at `(0x1D00,0x0A00)`, enters record 85 with normal Up movement, and captures contact, entry, transition start/midpoint, and a clean scene-13 `actual.bmp`. This is controlled ordinary-input route QA, not a complete New Game playthrough. Sound, destination actor placement, Release QA, and independent timing remain open. See [Jump House entry and Gunderson audit](visual-audits/jump-house-entry-and-gunderson.md). |
| Scene 2, record 91 at `0x173138`, raw `0x00C2`, event 2 | Story route override, despite table row 2 being `0x1A` | The completed New Game opening leaves `FFE18B` set; after field warmup, the normal movement continuation heads to this collision without controller input. The route opens Stoker arrival dialogue in scene 2. Hotel/karma interactions remain in scene 2; this is not a separate Stoker interior. Later entry phases depend on `FFFC1E` and the typed debt/package story state. | `building.lua` selects the same-map Stoker dialogue route; indexed title service and shared motel/quest scripts resolve arrival, manager offer, debt/payment handoff, and ordinary-motel state. Debug/Release QA connects New Game arrival through payment/handoff and clean return using a 250-nuyen test wallet. Typed story tests cover the unpaid repeat offer and completed-story ordinary motel. Restored-save story state and a physical repeat-entry route remain open. |
| Scene 2, record 92 at `0x173142`, raw `0x00CB`, event 11 | Service route override, despite table row 11 being `0x33` | Opens the Ares service prompt; A opens the shop. | Connected to field-mode service and shop; physical scene-2 route has QA coverage. |
| Scene 2, record 93 at `0x17314C`, raw `0x00C1`, event 1 | Scene 2 event-table row 1 at `0x172CDD` is `0x1A` | Selects resource-activity scene 26. The coordinate crosswalk matches Redmond map marker 8, Little Chiba; ROM contract area-2 location index 1 is also named Little Chiba. The numeric scene byte itself does not say “office.” | `building.lua` selects ROM row 1 and the world-activity coordinator enters scene 26. The end-to-end terminal path uses explicit QA fixtures for passkey, skill, and PRNG. |
| Scene 2, record 97 at `0x173174`, raw `0x00C7`, event 7 | Indexed title service 22 at `0x545B6` runs before generic route selection; event-table row 7 at `0x172CE3` is `0x1A`, but the special service returns in Scene 2 | Mike's Redmond map marker 9 Rat's Nest aligns with this rectangle, a secondary name crosswalk rather than a ROM-stored label. The service requires scene 2/event 7, alternate result 0, `FFFC34` bit 13 set, and consumed bit 10 clear. It consumes the prerequisite, marks bit 10 and the Landmark Map group item, awards +2 Karma to each runner record, and displays ROM text records `0x117193`, `0x117266`, and `0x117314`. Source `D6=-1` suppresses the generic transition. Menu action `0xE7` / handler `0x249A4` sets the bit-13 prerequisite after the conditional `FFFC1E` bit-26 question. ROM writers `0x22018` and `0x22072` for bit 26 are identified from `FFFC22` dialogue conditions; the live Harlequin choice/menu route remains unconnected. | Lua handles the three pages; `FieldSceneRuntime` binds prerequisite/one-shot/item/Karma facts and applies the ROM award on script completion. Debug/Release QA uses ordinary Up input from `(0x1B00,0x4600)`, but seeds bit 13. It checks the pages, award, one-shot flags, 60-frame host return, and whole-screen replacement. The end-to-end lead remains partial and no independent Rat's Nest Genesis capture is identified. Record 96 shares the service but is not the strong marker-9 geometry match. See [the seven-question visual audit](visual-audits/scene2-redmond-vigore-search.md). |
| Scene 2, record 96 at `0x17316A`, raw `0x00C7`, event 7 | Same event-table row and indexed title service as record 97 | Bounds `x=0x02E0..0x02FF`, `y=0x04A0..0x04BF` align away from Rat's Nest marker 9 on the Mike's map overlay; ROM still sends it through the same event-7 service. | Shares the Lua and typed native handler. Its specific named location and physical approach remain unresolved. |
| Scene 13, records 9/10 at `0x1B3CE4` / `0x1B3CEE`, raw `0x0080`, event 0 | No event table; event-0 collision routing | Two rectangles at cells `(2,7)` and `(3,7)` are near the lower-left doorway on the extracted scene image. The independent longplay shows the interior, but the exact source exit/interaction semantics and return destination still need tracing. | No Lua route or focused physical-input QA. Do not assume these rectangles return to scene 2 until the ROM exit path is identified. |
| Scene 13, records 11/12 at `0x1B3CF8` / `0x1B3D02`, raw `0x019E`, event 30 | Direct continuation `0xB7B8 -> 0x20990`; scene-13 branch chooses mode row 0 from `0x12CC50` and initial prompt `0x105114` | Two event-30 rectangles are at cells `(0,1)` and `(1,1)`. Profile `0x121E80` decodes to a 56x72 portrait matching Gunderson in frame `section_03/frame_001500`. The root labels point to `0x104051`, `0x103A3A`, and `0x104FEB`. Information descriptor `0x10517C` routes questions to builder `0x22F90` and contacts through `0x1051EC` to offer descriptor `0x105200`. Action `0x4D` at `0x22136` debits 1,000 nuyen via `0x21A4E`, returning paid descriptor `0x1052A0` or low-funds descriptor `0x1051D0`; FFFC2C bit 4 is not a unique contact-acquired flag because New Game seeds it at `0x59C48`. | `FieldSceneRuntime` routes the continuation through `johnson.interaction`; Lua owns portrait/menu copy, questions, contact purchase/failure, and repeat contact route. Debug/Release dialogue QA captures twenty full-screen states, including the 999-nuyen boundary. The scene-2/event-4 route continues on the same runtime through the production wallet service for a 1,000-nuyen purchase and unfunded repeat, with fourteen matching full-screen Debug/Release captures. The contact branch lacks an independent Genesis frame. Ordinary movement into event 30 is not yet QA-covered. The FFFC27 alternate Vigore response, work contract/quest, sound, and event-0 exit remain open. See [Jump House entry and Gunderson audit](visual-audits/jump-house-entry-and-gunderson.md). |
| Scene 20, record 117 at `0x1D9724`, raw `0x0082`, event 2 | `field_events.lua` selects the maglock action under the scene/world-mode gate | Electromagnetic-lock prompt. A successful passkey/electronics path changes collision cell 483 from `0x82` to `0x81` after result acknowledgement. | Lua selects the action; `FieldInteractionRuntime` executes it. Several physical branches have Debug/Release route coverage. Story acquisition and exact independent Genesis prompt/result captures remain open. |
| Scene 26, record 117 at `0x1D9E00`, raw `0x00A2`, event 2; record 110 at `0x1D9DBA`, raw `0x0082`, event 2 | Same conditional electromagnetic-lock continuation | Successful passkey action updates the sampled passage cells (`0xA2 -> 0xA1` and `0x82 -> 0x81`), after which ordinary movement crosses the lanes. | Connected to the shared maglock handling; physical route QA covers both gates with QA-seeded passkey state. |
| Scene 26, records 124/125 at `0x1D9E46` / `0x1D9E50`, raw `0x00D8`, event `0x18` | Direct `0x20EC2` branch | Security Terminal prompt. Its outcome is a separate world-activity result path, not an elevator and not the scene-2 event-1 destination. | Prompt and joined physical route are connected. Source result handling has focused QA; story item/skill state and independent Genesis result capture remain open. |
| Scenes 21-25, 26-29, 30-34, 35-40, and 41-43; event `0x1D` (`0xDD` raw) | `field_events.lua` selects the event-29 floor-selector action; `building.lua` resolves entry action, entry floor, and selected child scene from the extracted source groups | Groups are `[21,22,23,24]`, `[26,27,28]`, `[30,31,32,33]`, `[35,36,37,38,39]`, and `[41,42]`. Trigger scenes 25, 29, 34, 40, and 43 are not members of their child groups. Destinations remain numeric scene IDs. | Lua owns the elevator entry decision and destination lookup; the typed native selector owns input state/timing and the host loads the chosen scene. Debug and Release script tests and successful group routes are listed in [the elevator audit](visual-audits/field-elevator-selector.md). Physical approach probes currently fail in scenes 28 and 36; ordinary resource entry and saved-floor restoration remain open. |
| Scene 4, record 96 at `0x15BDF6`, raw `0x00DB`, event `0x1B` | `field_events.lua` selects the event-27 world-activity continuation | Source selects the event-27 continuation; the table row is bypassed. Its complete downstream behavior is not established. | Lua selects the typed continuation; Qt has no corresponding pending field-scene handler binding yet. |
| Scene 47, record 62 at `0x1D54B0`, raw `0x00B9`, event `0x19` | If `FFFC10 == 0x13`, direct `0x53EFA`; otherwise rewrite the collision result to `0x00A8`/event 8 | The route depends on field-transition context. The raw event-table row 25 is not a fixed destination; the rewrite path continues as event 8. | Typed dispatcher branch exists. This scene/context route is not physically exercised in the current Qt field runtime. |

### Puyallup / Tarislar Garden Apartments crosswalk

Mike's RPG Center labels Puyallup map marker 2 “Tarislar Garden
Apartments”; its walkthrough says the player visits Frosty after the
Sinsearach trials, then goes to the apartments to meet Harlequin. The ROM
independently supports that story identity: service 21's text records mention
Tarislar, the armed elves, the elevator, and Harlequin. The site is a secondary
map-name and quest-order cross-check, not the source for the trigger or its
conditions. See the [Puyallup map](https://mikesrpgcenter.com/shadowrun/maps/puyallup.html)
and [walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html).

The source path is:

1. Scene 1 collision record 67 at `0x17D9A2` covers grid cell `(23,5)`;
   collision `0x00C7` publishes event 7. Event-table row 7 at `0x17D620`
   contains destination scene `0x0D`.
2. The indexed service loop `0x53A78` calls entry 21, `0x5447E`, before the
   generic event-table route. That service matches field mode 1/event 7.
   `FFF0D5` selects an alternate text chain; it does not gate the service.
3. `FFFC1E` bit 30 is set by dispatcher index `0x44`, handler `0x21FD0`. That
   action loads profile `0x121F4C` and the text chain beginning at `0xF57F8`.
   The first text says “You are taken to Frosty”; this associates the profile
   with Frosty in context, though the extracted profile has no name label. The
   complete Frosty choice graph and its live action-44 route are not connected.
4. `FFF0D5` is not a gate: when nonzero the service chooses a separate text
   path at `0x15FF20` or `0x15FF28` based on its sign. When it is zero and bit
   30 is clear, service 21 selects the armed-guard copy beginning at
   `0xFBCC7`, followed by the challenge at `0xFBD5A` and leave narration at
   `0xFBD94`. With bit 30 set and bit 21 clear, it sets bit 21 through
   `0x2196C`, adds 2 to each of the three `FF018D`/`FF028D`/`FF038D` phase
   counters through `0x59C34`, and selects first-entry copy beginning at
   `0xFBE12`.
5. After the shared prompt runner `0x1F246`, the service tests `FFF0CC`.
   A nonzero marker sets `FFE186=1` and leaves `D6=0`; otherwise it sets
   `D6=-1`. For event 7, the main loop's `FFFBF9 < 8` branch jumps directly to
   selector `0x167AE`; it does not execute the adjacent `FFE186=2` assignment.
   The selector reads event-table row 7 (scene 13), then selects scene 54 from
   marker 1. If `FFE162` contains a pending descriptor list, `0x16A26` publishes
   it and instead selects scene 15, whose field-mode branch also tests
   `FFFC1E` bit 29. With no marker, scene 13 is subject to the selector's bit
   19 gate. Marker 2 is a different path: it selects scene 55 and queues
   `0x20938`, whose graph begins at `0x0FDD1A` with the Tomb of Ellisia
   antechamber. This is not the event-7 marker-1 route. The event-specific
   `FFE162` state and scene-54 completion/return are unresolved.

No screenshot-based scene-name claim should be made from the extracted map
alone. The site map image has not yet been aligned to scene 1's coordinates,
and the entry dialogue has no physical Qt QA. Keep the route source-profiled
but unimplemented until the event-specific `FFE162` state is bound, the
text/action state and actor phase counters are connected, scene 54's entry,
activity, exit, and return are traced, and full-screen replacement and clean
redraw are captured.

### Reported Little Chiba / Redmond elevator transition

The ROM evidence establishes a two-step numeric route, but does not assign
either scene a place-name. In scene 2, record 93 at `0x17314C` publishes
collision `0x00C1`/event 1. Event-table row 1 at `0x172CDD` contains `0x1A`,
which `0x167AE` classifies as resource-activity scene 26. The normal route QA
loads scene 26 at its descriptor spawn `(0x0A00,0x0500)`.

Scene 26's elevator selector is a separate movement trigger: records 102 and
103 at `0x1D9D6A` and `0x1D9D74` combine into event-29 bounds
`x=0x0080..0x00BF`, `y=0x0020..0x003F`. ROM `0xB7B8` requires its movement and
collision gates before dispatching event 29 to `0x58D94`; `building.lua` only
returns `open_elevator_selector` for that event when the extracted scene group
exists. The scene-26 spawn is outside these bounds, so scene entry alone does
not contact this trigger. The current Qt route-entry test holds scene 26 for
30 field updates without opening the selector. Separate elevator QA starts
from a QA-restored scene-26 state and physically approaches the event-29
rectangle; it does not join the normal scene-2 route to that trigger.

Therefore the source does not support an elevator opening automatically as
part of the scene-2-to-26 entry. It also does not identify scene 2 or scene 26
as Redmond or Little Chiba. A joined ordinary-input capture from the user's
reported route is still needed to determine which interaction was taken and
which visible map was being named. Keep unprofiled place names and reverse
routes unresolved.

The event `0x1A` rectangles (168 records, raw `0xDA`) are projected-terrain
markers. `0xB7B8` calls `0x16760` to mark the projected cell and only starts
continuation `0x20C90` if the source marker condition succeeds. Events `0x1C`,
`0x1E`, and `0x1F` select continuations `0x58DB2`, `0x20990`, and `0x0DB26`.
Those are source-proven branches, but their full in-game meaning and connected
Qt handlers are unresolved. Their full per-record locations are in the TSV.

## Lua event system connection

`field_events.lua` owns live collision policy at the field-controller seam.
Every accepted blocked-movement collision passed from the ROM movement path
becomes a typed event with a named `FieldCollisionEventCode`, scene/mode facts,
world coordinates, and source-gate facts. Lua returns a named action such as
project world interaction, start a maglock, mark projected terrain, or enter a
source activity. `GameScriptRuntime` validates the action into
`FieldMapEventAction`; the native movement controller applies that result
through its engine services. `FieldMapEventSystem` records the input and
validated decision for stateful event tracking and QA, but no longer chooses
the collision action.

After the ROM field-exit path begins, `building.lua` owns location route
selection. The native host passes the current scene/mode/event enum and the
extracted 32-byte event table as read-only facts. Lua applies the Stoker and
Ares scene-2 overrides or selects the byte at the active event index. The
typed result carries both the numeric scene/activity target and a named
`WorldActivityMode` selected from the ROM `0x167AE` ranges: field scene,
resource activity, scripted activity, or halted. The world-activity executor
uses Lua's mode for a routed location; source-internal timeout and descriptor
overrides still select their traced mode. The ROM event table remains named
host data and C++ remains the scene/activity executor. Invalid or absent
table routes remain explicit `no_route` results.

The scene-entry registry consumes Lua-authored rules from `field_events.lua`.
Each rule names a scene, optional inclusive coordinate scope, required typed
state flags, an action, and a repeat policy (`every_match`, once per scene
visit, or once per session). The first registered rule is the ROM-backed New
Game opening handoff: scene 2, spawn `(0x3CE0,0x2D00)`, and the completed-intro
marker start the normal heading-zero movement once. The restored-save path
does not publish the intro-complete flag. The native registry tracks trigger
lifetime and dispatches the Lua handler; future scene triggers are authored in
Lua and can use the same collision/movement seam.

One important unresolved record is scene 20 record 110 at `0x1D96DE`: raw
`0x00C0`, event 0, source bounds `x=0x0580..0x059F`,
`y=0x0020..0x003F`. Its profile's
event-table pointer at `0x1D91C8` is null. The generic `0xB7B8` path publishes
event 0 and calls `0x1651E`; the scene-`0x14` branch in `0x1651E` only writes
its special map cells when the incoming event byte is nonzero. The shared
projection/carry path may still run, but this record supplies no map writes or
event-table destination. Therefore no reverse-scene transition is established.
`building.lua` reports the route as `no_route`; the rectangle is not
currently classified as an exit. The new focused CTest covers that unresolved
classification, but there is no physical-input scene-20 C0 QA route yet.

## Event-table values in maps 0-9

Only scene profiles 0-9 carry event tables. The following lists show the raw
bytes referenced by event-bearing rectangles. Codes 2-5 can take the
conditional maglock branch before a table lookup; codes `0x18..0x1F` take
direct branches, with event `0x19` rewriting to event 8 when its context check
fails. Scene-2 event 2 and 11 also have story/service overrides. For other
generic fallthroughs the byte is consumed by the field/world-activity route.
Values are listed as `event=table-byte`; use the per-record TSV to locate
every rectangle and its coordinates.

| Scene | Profile / event table | Referenced event-index bytes |
|---:|---|---|
| 0 | `0x0D5456` / `0x0D54B2` | `00=15, 01=0C, 02=12, 03=15, 04=1E, 05=15, 06=0A, 07=15, 08=32, 09=33, 10=00` |
| 1 | `0x17D5C4` / `0x17D620` | `00=0E, 01=35, 02=1A, 03=0D, 04=0F, 05=0D, 06=0D, 07=0D, 08=32, 09=0D, 0A=33, 0B=34` |
| 2 | `0x172C80` / `0x172CDC` | `00=14, 01=1A, 02=1A, 03=1A, 04=0D, 05=13, 06=1A, 07=1A, 08=34, 09=1A, 0B=33`; event 2 and 11 are scene-2 overrides, so their values do not describe the Stoker/Ares destination. |
| 3 | `0x18A6A8` / `0x18A6FC` | `00=11, 01=0D, 02=15, 03=1E, 04=0D, 05=0B, 06=29, 07=33, 08=34, 09=35, 10=0A` |
| 4 | `0x15B902` / `0x15B95E` | `00=0D, 01=0A, 02=0D, 03=0D, 04=0D, 05=0D, 06=00`; event `1B` is direct. |
| 5 | `0x06796A` / `0x0679C6` | `01=10, 02=23, 03=23, 04=23, 05=23, 06=23, 10=79`; byte `79` is outside the scene ranges and halts at `0x167AE` if reached through the generic route. |
| 6 | `0x14B890` / `0x14B8EC` | `00=0D, 01=0D, 08=0D, 09=0D, 0A=0D` |
| 7 | `0x1B3D0E` / `0x1B3D6A` | `00=0D, 08=0D, 09=0D, 0A=0D` |
| 8 | `0x1B45BC` / `0x1B4618` | `08=0D, 09=0D, 0A=0D, 0B=0D` |
| 9 | `0x1B4E1A` / `0x1B4E76` | `00=0D, 08=0D, 09=0D, 0A=0D` |

Event indices `0x18..0x1F` have bytes in these tables too, but `0xB7B8`
handles those IDs directly. They are retained in each per-trigger row as
diagnostic bytes, marked as bypassed by the dispatcher.

## Converted code and runtime wiring

| ROM/source operation | Converted C++ function or runtime owner |
|---|---|
| `0x3232` scene-profile table | [`field_scene_profile_table_entry`](../ghidra_conversion/src/field/scene_profile_conversion.cpp) and `kFieldSceneProfileTable` in the matching header. Runtime source assets are loaded by [`load_field_session_source_assets`](../ghidra_conversion/src/integration/field_session_source_assets_conversion.cpp). |
| `0x0EE10` collision rasterizer | [`rebuild_terrain_collision_map`](../ghidra_conversion/src/field/terrain_map_conversion.cpp) |
| `0xE7C8` terrain sampler and `0xB77E` directional probe | [`evaluate_terrain_collision`](../ghidra_conversion/src/field/terrain_probe_conversion.cpp) and [`probe_entity_directional_collision`](../ghidra_conversion/src/entities/movement_probe_conversion.cpp) |
| `0xB7B8` event dispatcher | [`advance_entity_movement_and_mode_transition`](../ghidra_conversion/src/entities/movement_mode_transition_conversion.cpp); typed continuation enum is in `movement_mode_transition_conversion.hpp` |
| `0x1651E` projected world interaction | [`apply_world_map_effect`](../ghidra_conversion/src/world/world_scene_entity_conversion.cpp) |
| `0x167AE` table byte to field/resource/scripted activity | [`resolve_world_activity_scene_id`](../ghidra_conversion/src/world/world_activity_routine_conversion.cpp) and [`run_world_activity_routine`](../ghidra_conversion/src/world/world_activity_routine_conversion.cpp) |
| `0x1F5CA` exit and event-route orchestration | [`run_field_exit_orchestrator`](../ghidra_conversion/src/field/field_exit_orchestrator_conversion.cpp); collision policy in [`field_events.lua`](../gameplay/scripts/field_events.lua), location route policy in [`building.lua`](../gameplay/scripts/building.lua), adapted by `GameScriptRuntime` |
| `0x20E48` maglock / `0x20EC2` Security Terminal | [`FieldInteractionRuntime`](../ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp), reached through the field scene controller |
| `0x58D94` floor selector / `0x58DB2` handoff | [`ElevatorSelectorRuntime`](../ghidra_conversion/qt_runtime/src/field/elevator_selector_runtime.cpp) and [`FieldSceneRuntime`](../ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp) |
| Live Qt route selection and world-activity callback | [`FieldSceneRuntime::run_field_exit_orchestrator`](../ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp) and the coordinator callback setup in that file |

The Ghidra output is a cross-check, not the runtime implementation:
[`fn_0000B7B8.cpp`](../export/ghidra-c/fn_0000B7B8.cpp),
[`fn_000167AE.cpp`](../export/ghidra-c/fn_000167AE.cpp), and
[`fn_0001F5CA.cpp`](../export/ghidra-c/fn_0001F5CA.cpp). The native runtime
uses typed scene IDs, collision rectangles, event records, and continuation
enums rather than ROM address interpretation.

## Remaining route gaps

- The final TSV column marks which exact records have focused physical-input
  coverage. The remaining coordinates are profiled, but their physical
  reachability is not thereby proven.
- `FieldSceneRuntime::record_field_activity_start` connects maglock,
  Security Terminal, event 29, and the Scene-26 event-31 resource main menu.
  Scene 26 record 126 (`0x1D9E5A`, x/y `0x0200..0x021F`, event `0x1F`) now has
  a physical-input QA route: Up from the adjacent cell opens Lua's `Main
  Menu`; Down selects `Exit`; A restores the captured Scene-2 field state.
  This is partial coverage of one event-31 record only. Other event-31 map
  records, events 26-28 and 30, context-dependent event 25, the Cyberspace
  branch, and a source-derived cursor graphic remain unresolved.
- Scene 2 event 0 and event 1 have numeric targets `0x14` and `0x1A` in the
  event table. `0x1A` is resource activity according to `0x167AE`; it is not a
  ROM place-name, so this profile does not label it “office.” The source route
  crosswalk and its unresolved named destination are recorded in the
  [Redmond coordinate audit](mikes-rpg-center-shadowrun-reference-notes.md#redmond-rom-coordinate-cross-check-jump-house-and-rats-nest).
- Visual correctness is not established by the trigger data. The map geometry
  and event bytes are ROM-verified; individual route animation, dialog, sound,
  and whole-screen redraw still require route-specific verification against
  source traces and independent gameplay frames where available.

The existing route-level notes remain linked from [field-map-script-routing](visual-audits/field-map-script-routing.md),
[scene-2 event-1 route](visual-audits/scene2-event1-scene26-route.md),
[scene-26 terminal route](visual-audits/scene26-event1-terminal-route.md),
and [the elevator selector audit](visual-audits/field-elevator-selector.md).
