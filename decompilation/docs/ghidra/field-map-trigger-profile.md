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

I added an offline audit utility, [`profile_field_map_triggers.py`](../../ghidra_conversion/tools/rom_research/profile_field_map_triggers.py),
which reads the ROM and verifies the named extracted assets before writing the
inventories. Run it from the repository root:

```powershell
python decompilation/ghidra_conversion/tools/rom_research/profile_field_map_triggers.py
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
Event IDs 2-5 enter the shared maglock branch only in Scene 20 or resource
activity mode (scene IDs 21-43); ordinary field scenes publish those IDs to
their event tables. Timeout and pending-descriptor state can also replace the
selected value before scene preparation. `0x167AE` classifies the generic
table value as follows:

| Table byte | ROM meaning |
|---|---|
| `0x00..0x14` | Field scene ID. Scene `0x14` has a separate FFFC2C bit-26 scene-entry action that clears the bit and schedules a timeout when set; it does not block or replace the destination. Scene `0x0D` checks FFFC1E bit 19 and scene `0x0F` checks bit 29 before selecting their additional world-script path; the field map is prepared before those checks. |
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
| Scene 1, record 67 at `0x17D9A2`, raw `0x00C7`, event 7 | Event-table row 7 at `0x17D620` defaults to `0x0D`; indexed title service 21 at `0x5447E` runs before generic route selection | Title service 21 checks FFF0D5 and FFFC1E story bits before the generic event route. First authorized entry requires bits 30/21 = 1/0, marks bit 21, and calls `0x59C34` with D0=2 for +2 Karma; this conversation does not set FFF0CC and returns to scene 1. Service 21 repeat first presents greeting FC111; bits 16/28/26/27 and FFFC22 item bits 8/7 gate its questions, Ito response, map-information, and tomb branches. Its Yes command `0xE4` calls `0x24954`, sets FFF0CC, and selects scene 54 only when bits 30, 21, 16, 28, 26, and 27 and alternate result 0 match. Pending descriptors can override to scene 15. Marker 2 remains a separate scene-55 Tomb route. | Lua entity links keep the first-authorized conversation on scene 1, add the repeat-information task, and add a separate tomb-offer override gated by bits 30/21/16/28/26/27 and alternate result 0. Its Yes task follows the three ROM travel pages and resumes scene 54; No follows the preparation and Harlequin pages and returns to scene 1. The guarded path also returns to scene 1. Debug/Release physical-input CTests exercise the first visit, guard, repeat information, and tomb routes and save whole-screen actual.bmp files; script QA checks both tomb choices. The upstream Frosty choice, pending-descriptor override, and scene-54 exit/return remain open. See [the Tarislar route audit](visual-audits/puyallup-tarislar-apartments.md). |
| Scene 1, record 69 at `0x17D9B6`, raw `0x00C5`, event 5 | Event-table row 5 at `0x17D625` is `0x0D`; `0x167AE` tests FFFC1E bit 19 after Scene 13 preparation | Scene 13 loads with zero funds and no entry dialogue. Bit 19 (`ILENE_MET`) gates only world-script index 3 and its actor, `lone11_k0a_item03` at `(4352,768)`; it does not gate admission. The Puyallup map marker and ROM contract-location index corroborate the site name, but do not prove a shop flow. | `shadowrun_ghidra_conversion_qt_field_crime_mall_ilene_gate_qa` reaches record 69 by ordinary Up input in Debug and Release. Both story states save full-screen `actual.bmp`; bit 19 clear selects no script, and set selects script 3 with its single actor. Contact and destination captures, route logs, zero funds, and no dialogue are checked. Independent Genesis comparison and any later shop interaction remain open. |
| Scene 1, record 70 at `0x17D9C0`, raw `0x00C4`, event 4 | Event-table row 4 at `0x17D620` is `0x0F`; `0x167AE` prepares Scene 15 before testing FFFC1E bit 29 | The field transition is not gated by nuyen or reputation. Bit 29 is `HEART_DELIVERED`; it gates only Scene 15 script index 5, whose one actor is suppressed when either compact runner marker is 10. The external Puyallup map's 100-nuyen/6-Reputation note is not expressed by this ROM route. | Debug/Release ordinary-Up QA enters Scene 15 with zero funds and no dialogue. With bit 29 clear there is no script actor; with it set Lua publishes the ROM kind-3 actor `lone11_k03_item03` at `(896,2688)`. The contact/exit/return behavior and independent Genesis frames remain open. |
| Scene 1, record 72 at `0x17D9D4`, raw `0x00C3`, event 3 | Event-table row 3 at `0x17D623` is `0x0D`; `0x167AE` tests FFFC1E bit 19 after Scene 13 preparation | The Puyallup map names the site Riannon's Magic Shop; the ROM route itself only selects shared Scene 13. Entry loads at zero funds with no dialogue. Bit 19 (`ILENE_MET`) gates only world-script index 3 and `lone11_k0a_item03` at `(4352,768)`, not admission. | `shadowrun_ghidra_conversion_qt_field_riannons_magic_shop_entry_qa` reaches record 72 by ordinary Up input in Debug and Release. Both bit states save full-screen `actual.bmp`; contact and destination captures verify the route, no charge/dialogue, and actor gate. The existing Lua service title does not establish a shop catalog or transaction; independent Genesis comparison and the shop interaction remain open. |
| Scene 1, record 74 at `0x17D9E8`, raw `0x00C6`, event 6 | Event-table row 6 at `0x17D626` is `0x0D`; `0x167AE` tests FFFC1E bit 19 after Scene 13 preparation | The Puyallup map and ROM contract-location index cross-reference the site as Dr. Bob's Quickstitch Clinic. The ROM route selects shared Scene 13; entry loads at zero funds without dialogue. Bit 19 gates only world-script index 3 and its actor, not admission. | `shadowrun_ghidra_conversion_qt_field_dr_bobs_clinic_entry_qa` reaches record 74 by ordinary Up input in Debug and Release. Both bit states save full-screen `actual.bmp`; captures and route logs verify the scene route, no charge/dialogue, and actor gate. The Lua service-title key does not establish a clinic menu or transaction; independent Genesis comparison and the actual clinic flow remain open. |
| Scene 1, record 76 at `0x17D9FC`, raw `0x00C9`, event 9 | Event-table row 9 at `0x17D629` is `0x0D`; the active-run dispatcher at `0x1F086` checks the current area/event against the gang-contract target | The Puyallup map and contract-location index identify the Ork Gang site. Without an exact active gang-contract target, event 9 loads shared Scene 13; bit 19 gates only its post-load actor. With target area 1/event 9, ROM `0x1F086` shows entry text `0xFE0DE`, then queues actor list `0x1D6D4A`/profile `0x122054` and overrides the generic destination with Scene 15. This is a run-target branch, not an unconditional door gate. | `shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa` reaches record 76 by ordinary Up in Debug and Release. The matching target shows the entry passage and enters Scene 15 with four actors; no-run at record 76 and a different-area active target at record 77 both follow row 9 to Scene 13. Full-screen captures and route logs are hash-identical across builds. Later fight outcome/reward and independent Genesis comparison remain open. The same CTest now covers the Eye-Fivers target and its wrong-target/no-run fallbacks. |
| Scene 3, record 100 at `0x18ABBE`, raw `0x00C4`, event 4 | Event-table row 4 at `0x18A6FC` is `0x0D`; ROM `0x1F086` checks the active gang-contract area/event target | An exact area-3/event-4 match identifies the Eye-Fivers contract target, shows text `0xFE0DE`, queues actor list `0x1D6D72`/profile `0x122018`, and overrides the generic Scene-13 route with Scene 15. A different active target and no-run state follow row 4 to Scene 13. | `shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa` reaches record 100 by ordinary Up in Debug and Release. Exact match publishes four actors after the passage; mismatch and no-run cases fall back. Full-frame artifacts are hash-identical across builds. Fight results, reward, and independent Genesis comparison remain open. |
| Scene 1, record 77 at `0x17DA06`, raw `0x00C9`, event 9 | Shares event-table row 9 at `0x17D629`; the same exact active-run area/event comparison applies | This adjacent ROM rectangle is the second interaction at the Ork Gang site. An active contract targeting another area does not select the gang passage; ordinary routing enters Scene 13. An exact Puyallup target match uses the record-76 branch above. | The dedicated Debug/Release route QA physically enters record 77 with a gang contract targeting area 2/event 9 and verifies no entry passage, then loads Scene 13 from row 9. Record 76 separately verifies the exact Puyallup match and no-run cases. The alternate rectangle's matching-target branch and independent Genesis comparison remain open. |
| Scene 2, records 89/90 at `0x173124` / `0x17312E`, raw `0x00C0`, event 0 | Scene 2 event-table row 0 at `0x172CDC` is `0x14`; Lua route preparation for service 19 at `0x543A2` runs before the route | The event still selects numeric scene 20. When story bits 18/19 are 1/0, `FFF0D5` is zero, `FFFC34` bit 0 is clear, and `FFFC38` bit 12 is set, service 19 arms transient `FFE182` without consuming the event. The active Lua route policy implements these gates and leaves the typed scene-20 destination live. The marker is consumed at `0xED92` only from the `0xEB92` actor-pair scan through `0xECCC`: a pair member needs `+0x9E` bit 3, its one-shot `+0xB0` bit 7 must be clear, and the partner needs `+0x10` bit 1. When `FFE180` is clear, `0xA800` selects pending list `0xA8CC`: template `0x172CA` (`lone11`, kind 10/action 3) spawns at `(0x0E80,0x0700)`, and ordered overrides end with `+0x9E=0x08` (bit 3). The separate eight default guards/pedestrians have no such overrides. If reached, activity `0x54402` adds 2 Karma to each runner, sets story bit 19, updates `FFFC30/34/38`, and opens dialogue group `0x115ECC`. The extracted text at `0x115F10` explicitly names Ilene. In `0x167AE`, scene entry separately tests FFFC2C bit 26 at `0x1689A`: if set, it clears the bit and calls `0x1DBAA`; this does not change scene 20. | Debug/Release physical QA reaches the scene-2 doorway by ordinary input, sets `FFE182` through Lua with the exact service gates, and confirms scene 20 still loads; `actual.bmp` and a route trace are saved. Existing QA also covers ordinary entry with the bit clear and seeded bit-26 consumption with the timeout retained through final scene re-entry. The ROM-generated actor catalog records the A8CC pending list and its FFE180/FFE182 gates. Active scene initialization selects and publishes the actor in slot 3 before defaults; Debug/Release route QA checks its source position/profile and confirms defaults begin at slot 4. Actor-pair eligibility and conditional activity execution remain open. |
| Scene 2, record 85 at `0x1730FC`, raw `0x00C4`, event 4 | Ordinary field mode publishes event 4 to event-table row 4 at `0x172CE0` (scene `0x0D`); it does not enter the conditional-maglock branch. Indexed title service 22 selects entry descriptor `0x110D1E -> 0x116C12` at `0x20484..0x204C6` when `FFF0D5` is clear. | Mike's marker 1 labels Jump House and Gunderson. Frame 001200 independently shows the location description and its portrait before the scene-13 map. The ROM stores numeric scene and trigger values, not the building name. | `building.lua` opens the generic Lua entry task. The route retains the typed scene-13 destination until ordinary A dismisses the description. Debug CTest `shadowrun_ghidra_conversion_qt_field_jump_house_entry_qa` starts at `(0x1D00,0x0A00)`, enters record 85 with normal Up movement, and captures contact, entry, transition start/midpoint, and a clean scene-13 `actual.bmp`. This is controlled ordinary-input route QA, not a complete New Game playthrough. Sound, destination actor placement, Release QA, and independent timing remain open. See [Jump House entry and Gunderson audit](visual-audits/jump-house-entry-and-gunderson.md). |
| Scene 2, record 91 at `0x173138`, raw `0x00C2`, event 2 | Story route override, despite table row 2 being `0x1A` | The completed New Game opening leaves `FFE18B` set; after field warmup, the normal movement continuation heads to this collision without controller input. The route opens Stoker arrival dialogue in scene 2. Hotel/karma interactions remain in scene 2; this is not a separate Stoker interior. Later entry phases depend on `FFFC1E` and the typed debt/package story state. | `building.lua` selects the same-map Stoker dialogue route; indexed title service and shared motel/quest scripts resolve arrival, manager offer, debt/payment handoff, and ordinary-motel state. Debug/Release QA connects New Game arrival through payment/handoff and clean return using a 250-nuyen test wallet. Typed story tests cover the unpaid repeat offer and completed-story ordinary motel. Restored-save story state and a physical repeat-entry route remain open. |
| Scene 15, record 12 at `0x1D5D3E`, raw `0x019E`, event 30 | Direct collision continuation `0xB7B8 -> 0x20990`; Scene 15 falls through to `0x20C78` | The 32x32-unit rectangle covers map pixels X `288..319`, Y `32..63`. The fallback loads portrait `0x121E5C` and prompt `0xF3FAC` / text `0xF3FBC`; its first-choice pointer is `0xFFFFFFFF`, so the contact has no choices. Scene 15 is reused by multiple incoming routes; this event does not identify a named building or add an entry gate. The default branch contains no nuyen or reputation check. | `scene15_unnamed_old_man_contact` links the record to the shared Lua old-man dialogue profile. Debug/Release ordinary-Up QA from an outside approach opens the five-segment passage, then returns to Scene 15 after the 60-frame fade. Eight 320x224 frames and route logs match by SHA-256 across builds. This is a ROM-backed contact interaction, not an automatic entry prompt; no independent Genesis frame is identified. |
| Scene 19, record 12 at `0x1D5F50`, raw `0x019E`, event 30 | `0xB7B8` -> `0x20990`; Scene 19 misses the special scene cases and reaches `0x20C78` | The fallback loads portrait profile `0x121E5C` and prompt record `0xF3FAC`. Its title pointer `0xF3FBC` is the unnamed old-man narrative. The first-choice pointer is `0xFFFFFFFF`; `0x1F3C4` waits for controller input and returns command `0xFB` without a menu. The map rectangle is pixels X `0..31`, Y `32..63`; descriptor fallback spawn is `(208,204)`, outside it. This is a separate contact event, not proof of an automatic map-entry scene. | `field_events.lua` presents the location's Lua speaker/event context through the shared dialogue runtime; `dialogue.lua` binds the unnamed speaker, ROM text, and portrait. The map viewer shows the interaction, route, and content preview. A scene-19 interaction-region entity carries a Lua link to the shared location-dialogue handler. Debug and Release physical-input QA confirms that descriptor spawn `(208,204)` opens no automatic passage, ordinary Up from the south approach reaches record 12, and the Lua handler presents the ROM portrait/text with no choices before returning to Scene 19 after the 60-frame fade. All eight full-screen captures match across builds and were reviewed at original resolution. Debug/Release physical-input QA now walks ordinary Up from the Scene-2 south approach into event 5. A source-route boundary records the Lua-selected Scene-19 destination; the transition loads the descriptor fallback spawn (208,204) with no entry dialogue. This confirms the ordinary event-table route only; other incoming overrides and an independent Genesis passage frame remain open. |
| Scene 2, record 92 at `0x173142`, raw `0x00CB`, event 11 | Service route override, despite table row 11 being `0x33` | Opens the Ares service prompt; A opens the shop. | Connected to field-mode service and shop; physical scene-2 route has QA coverage. |
| Scene 2, record 93 at `0x17314C`, raw `0x00C1`, event 1 | Scene 2 event-table row 1 at `0x172CDD` is `0x1A` | Selects resource-activity scene 26. The coordinate crosswalk matches Redmond map marker 8, Little Chiba; ROM contract area-2 location index 1 is also named Little Chiba. The numeric scene byte itself does not say “office.” | `building.lua` selects ROM row 1 and the world-activity coordinator enters scene 26. The end-to-end terminal path uses explicit QA fixtures for passkey, skill, and PRNG. |
| Scene 2, record 97 at `0x173174`, raw `0x00C7`, event 7 | Indexed title service 22 at `0x545B6` runs before generic route selection; event-table row 7 at `0x172CE3` is `0x1A`, but the special service returns in Scene 2 | Mike's Redmond map marker 9 Rat's Nest aligns with this rectangle, a secondary name crosswalk rather than a ROM-stored label. The service requires scene 2/event 7, alternate result 0, `FFFC34` bit 13 set, and consumed bit 10 clear. It consumes the prerequisite, marks bit 10 and the Landmark Map group item, awards +2 Karma to each runner record, and displays ROM text records `0x117193`, `0x117266`, and `0x117314`. Source `D6=-1` suppresses the generic transition. Menu action `0xE7` / handler `0x249A4` sets the bit-13 prerequisite after the conditional `FFFC1E` bit-26 question. ROM writers `0x22018` and `0x22072` for bit 26 are identified from `FFFC22` dialogue conditions; the live Harlequin choice/menu route remains unconnected. | Lua handles the three pages; `FieldSceneRuntime` binds prerequisite/one-shot/item/Karma facts and applies the ROM award on script completion. Debug/Release QA uses ordinary Up input from `(0x1B00,0x4600)`, but seeds bit 13. It checks the pages, award, one-shot flags, 60-frame host return, and whole-screen replacement. The end-to-end lead remains partial and no independent Rat's Nest Genesis capture is identified. Record 96 shares the service but is not the strong marker-9 geometry match. See [the seven-question visual audit](visual-audits/scene2-redmond-vigore-search.md). |
| Scene 2, record 96 at `0x17316A`, raw `0x00C7`, event 7 | Same event-table row and indexed title service as record 97 | Bounds `x=0x02E0..0x02FF`, `y=0x04A0..0x04BF` align away from Rat's Nest marker 9 on the Mike's map overlay; ROM still sends it through the same event-7 service. | Shares the Lua and typed native handler. Its specific named location and physical approach remain unresolved. |
| Scene 13, records 9/10 at `0x1B3CE4` / `0x1B3CEE`, raw `0x0080`, event 0 | Profile `0x1B3C06 + 0x4E` is a null event-table pointer; the generic event-0 selector at `0x167AE` reads ROM byte `0x000000 = 0`, selecting field scene 0 | Two rectangles at cells `(2,7)` and `(3,7)` are at the south map edge, near the doorway in the extracted scene image. This is a static scene-0 destination; it does not identify scene 0 as the prior caller map or establish a door-relative spawn/facing rule. | The South-edge field transition entity has an authored Lua `area_event_override` link. The resolver consumes this one-time source-derived destination without emulating null-pointer arithmetic. Debug/Release ordinary-Down QA enters scene 0 and saves full 320x224 frames with matching SHA-256 values. No dialogue or quest/run gate is selected by this event-0 branch. |
| Scene 13, records 11/12 at `0x1B3CF8` / `0x1B3D02`, raw `0x019E`, event 30 | Direct continuation `0xB7B8 -> 0x20990`; scene-13 branch chooses mode row 0 from `0x12CC50` and initial prompt `0x105114` | Two event-30 rectangles are at cells `(0,1)` and `(1,1)`. Profile `0x121E80` decodes to a 56x72 portrait matching Gunderson in frame `section_03/frame_001500`. The root labels point to `0x104051`, `0x103A3A`, and `0x104FEB`. Information descriptor `0x10517C` routes questions to builder `0x22F90` and contacts through `0x1051EC` to offer descriptor `0x105200`. Action `0x4D` at `0x22136` debits 1,000 nuyen via `0x21A4E`, returning paid descriptor `0x1052A0` or low-funds descriptor `0x1051D0`; FFFC2C bit 4 is not a unique contact-acquired flag because New Game seeds it at `0x59C48`. | The scene-13 booth link selects `johnson.interaction` through the shared Lua location-route resolver; Lua owns portrait/menu copy, questions, contact purchase/failure, and repeat contact route. Debug/Release dialogue QA captures twenty full-screen states, including the 999-nuyen boundary. The scene-2/event-4 route continues on the same runtime through the production wallet service for a 1,000-nuyen purchase and unfunded repeat, with fourteen matching full-screen Debug/Release captures. The contact branch lacks an independent Genesis frame. Ordinary movement into event 30 is not yet QA-covered. The FFFC27 alternate Vigore response, work contract/quest, sound, and independent contact visuals remain open. Event-0 exit now has a Lua route; physical QA remains open. See [Jump House entry and Gunderson audit](visual-audits/jump-house-entry-and-gunderson.md). |
| Scene 20, record 117 at `0x1D9724`, raw `0x0082`, event 2 | `field_events.lua` selects the maglock action under the scene/world-mode gate | Electromagnetic-lock prompt. A successful passkey/electronics path changes collision cell 483 from `0x82` to `0x81` after result acknowledgement. | Lua selects the action; `FieldInteractionRuntime` executes it. Several physical branches have Debug/Release route coverage. Story acquisition and exact independent Genesis prompt/result captures remain open. |
| Scene 26, record 117 at `0x1D9E00`, raw `0x00A2`, event 2; record 110 at `0x1D9DBA`, raw `0x0082`, event 2 | Same conditional electromagnetic-lock continuation | Successful passkey action updates the sampled passage cells (`0xA2 -> 0xA1` and `0x82 -> 0x81`), after which ordinary movement crosses the lanes. | Connected to the shared maglock handling; physical route QA covers both gates with QA-seeded passkey state. |
| Scene 26, records 124/125 at `0x1D9E46` / `0x1D9E50`, raw `0x00D8`, event `0x18` | Direct `0x20EC2` branch | Security Terminal prompt. Its outcome is a separate world-activity result path, not an elevator and not the scene-2 event-1 destination. | Prompt and joined physical route are connected. Source result handling has focused QA; story item/skill state and independent Genesis result capture remain open. |
| Scenes 21-25, 26-29, 30-34, 35-40, and 41-43; event `0x1D` (`0xDD` raw) | `field_events.lua` selects the event-29 floor-selector action; `building.lua` resolves entry action, entry floor, and selected child scene from the extracted source groups | Groups are `[21,22,23,24]`, `[26,27,28]`, `[30,31,32,33]`, `[35,36,37,38,39]`, and `[41,42]`. Trigger scenes 25, 29, 34, 40, and 43 are not members of their child groups. Destinations remain numeric scene IDs. | Lua owns the elevator entry decision and destination lookup; the typed native selector owns input state/timing and the host loads the chosen scene. Debug and Release script tests and successful group routes are listed in [the elevator audit](visual-audits/field-elevator-selector.md). Physical approach probes currently fail in scenes 28 and 36; ordinary resource entry and saved-floor restoration remain open. |
| Scene 4, record 96 at `0x15BDF6`, raw `0x00DB`, event `0x1B` | `field_events.lua` selects the event-27 world-activity continuation | Source selects the event-27 continuation; the table row is bypassed. Its complete downstream behavior is not established. | Lua selects the typed continuation; Qt has no corresponding pending field-scene handler binding yet. |
| Scene 47, record 62 at `0x1D54B0`, raw `0x00B9`, event `0x19` | If `FFFC10 == 0x13`, direct `0x53EFA`; otherwise rewrite the collision result to `0x00A8`/event 8 | The route depends on field-transition context. The raw event-table row 25 is not a fixed destination; the rewrite path continues as event 8. | Typed dispatcher branch exists. This scene/context route is not physically exercised in the current Qt field runtime. |

### Newly connected quest-gated building interactions

Two previously numeric-only building destinations now use the existing
entity-link entry-task path. Scene 5 record 46 (`0x067C36`, event 6) invokes
title service 12 (`0x54076`) only with story bit 16 set, bit 12 clear, and
alternate result zero. It loads root `0x110A2A` and portrait profile
`0x121E2C`, then leaves D6 at zero so event-table row 6 still selects activity
35. Lua displays the extracted `0x1195CF` opening and returns the declared
`enter_scene_35` result.

Scene 3 record 97 (`0x18ABA0`, event 2) invokes title service 14
(`0x54152`) only with story bit 25 set, bit 14 clear, and alternate result
zero. It loads root `0x11B7F4` and portrait profile `0x122090`, then leaves D6
at zero so event-table row 2 still selects activity 21. Lua displays the
extracted `0x11C213` Fuchi entry page and returns `enter_scene_21`. Lua and
route-resolver tests cover both gate variants. Renraku now also has Debug and
Release physical-input QA: ordinary Up reaches record 46, the extracted page
shows the correct portrait and MORE prompt, and A resumes activity 35. The
full-screen captures are under `field-renraku-offices-qa/`; the activity-35
capture confirms the route resumes after the page. Fuchi now has Debug and
Release physical-input QA too: ordinary Up reaches record 97/event 2, the
gated Fuchi page shows its extracted logo portrait and MORE prompt, and A
resumes activity 21. Its full-screen captures are under
`field-fuchi-offices-qa/`. FFE184/FFE187 effects and the eventual building
return routes remain open; independent Genesis captures are unavailable for
both pages. See [the Renraku route audit](visual-audits/renraku-offices-scene5-event6.md)
and [the Fuchi route audit](visual-audits/fuchi-offices-scene3-event2.md).

Scene 0 record 45 (`0x0D57A0`, event 3) invokes title service 15
(`0x54222`) only when story bit 11 is set, bit 15 is clear, and alternate
result is zero. Its root `0x1111DE` uses portrait profile `0x121FC4`; the
first visible option pointer is `0xFFFFFFFF`, so the shared prompt returns
after the single `0x113D46` Ogami-departure page. Lua then clears FFFC30 bit
26, sets bit 27, marks trigger 15, and returns to scene 0, suppressing the
generic activity-21 event route. Debug/Release physical QA starts at
`(0x3A00,0x0A00)`, enters record 45 with ordinary Up input, checks the page and
one-shot state, and captures `actual.bmp` plus the restored scene-0 frame. See
[the Gates Undersound audit](visual-audits/gates-undersound-title-service.md).

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
   Lua `StoryFlags` now maps command `0x44` to a direct bit-30 write, and its
   test checks the command-table pointer and handler opcodes. The live caller
   and route remain unconnected. The Spirit Eyes branch at `0x53E84` checks
   Group Item bit 2 and, at `0x53E98`, consumes the scale before setting
   prerequisite story bit 9 at `0x53EA8` and opening `0xF4C04`. Scene-47
   handler `0x53F50` sets story bit 8 and Group Item bit 2 after the
   Licourtrix conversation; Lua now models both ends of that item handoff.
4. `FFF0D5` is not a gate: when nonzero the service chooses a separate text
   path at `0x15FF20` or `0x15FF28` based on its sign. When it is zero and bit
   30 is clear, service 21 selects the armed-guard copy beginning at
   `0xFBCC7`, followed by the challenge at `0xFBD5A` and leave narration at
   `0xFBD94`. With bit 30 set and bit 21 clear, it sets bit 21 through
   `0x2196C`, adds 2 to each of the three `FF018D`/`FF028D`/`FF038D` +0x8D Karma
   bytes through `0x59C34`, and selects first-entry copy beginning at
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
and the entry dialogue has no physical Qt QA. Keep the route partial. The
guard/repeat variants, event-specific `FFE162` state, upstream Frosty
action-44 route, scene 54 activity/exit/return, and physical full-screen
replacement and clean-redraw QA remain open.

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

The scene-entry registry reads map-event records directly from
`gameplay/world/areas/scene-NN-triggers.tsv`; the runtime loader places those
records in Lua's in-memory world-content table. Each rule names a scene, trigger
kind, optional inclusive coordinate scope, required typed state flags, an
action, a Lua handler, and a repeat policy (`every_match`, once per scene
visit, or once per session). Handler functions remain in `field_events.lua`.
The scene-2 New Game handoff is the first such row: the scene descriptor's
spawn `(0x3CE0,0x2D00)` and the completed-intro fact start the normal
heading-zero movement once. The restored-save path does not publish the
intro-complete flag. The native registry tracks trigger lifetime and dispatches
the Lua handler; the map viewer displays the exact source scope in its
containing 32-pixel cell.

Scene 20 record 110 at `0x1D96DE` is raw `0x00C0`, event 0, bounds
`x=0x0580..0x059F`, `y=0x0020..0x003F`; profile event-table pointer
`0x1D91C8` is null. ROM `0xB7B8` sends the zero event through `0x1651E`; the
zero-event return sets carry, after which `0xB7B8` starts its `0x10`-update
field transition. There is no event-table destination byte for this record.
The source context path snapshots 49 scene words at `0x16E30`, copies actor
X/Y anchors at `0x2FA4`, and restores scene state and anchors in
`0x2F32`. This supports the host's return to the saved Scene 2 doorway
contact. The Scene-20 transition reuses the live player record rather than
running the initial entity reset at `0xA7B4`, so heading 0 persists in this
route; other approach headings have not been exercised.

Debug/Release CTest `shadowrun_ghidra_conversion_qt_field_scene20_c0_exit_qa`
enters Scene 20 through Scene 2's physical row-0 route, then approaches record
110 with ordinary input: eight Left updates clear the neighboring record 109
`0x7F` wall, followed by held Up. The first C0 terrain sample is at
`(1421,69)`; the next Up/service update dispatches event 0 once and begins the
transition. The full transition takes 17 updates, and the host returns to
Scene 2 at fixed position `(3944,10350)`, heading 0. The `0x20` retry bit
stays set until held Down produces a clear-terrain sample at update 15; after
release, fresh Right input remains in Scene 2 and does not dispatch again.
All 45 full-screen BMPs were reviewed; Debug/Release hashes match. The
screenshots retain the HUD and clean field redraw, with no stale panel.
Scene 20's visible guards match default ROM actor records 3/4 at
`0x1D9266`/`0x1D926C` in the list at `0x1D91DA`. Audio count does not increase
across the contact/return sequence. No independent longplay frame of this exit
has been identified. Thus the current no-retrigger behavior and ordinary path
to record 110 are source-matched and covered; exact ROM facing remains open.

## Resource activity event-0 returns

Resource scenes 21-43 use the same generic event-0 continuation when a
collision record publishes event 0. With `FFDF40=2`, `0xB7B8` calls `0x1651E`;
the zero-event path returns carry and starts the normal 16-count field
transition. The destination is the saved source field context: `0x16E30`
captures it on activity entry and `0x2F32` restores the scene and actor
anchors on return. No event-table byte names a fixed destination. The Lua
field-route policy now returns the typed `source_activity_return` action for
event 0 in these null-table activity scenes; the field coordinator applies the
saved context only after that route is selected.

Debug/Release CTest `shadowrun_ghidra_conversion_qt_field_renraku_offices_qa`
enters Scene-35 record 121 at `0x1DC8AE`; the matching Fuchi route
`shadowrun_ghidra_conversion_qt_field_fuchi_offices_qa` enters Scene-21 record
102 at `0x1D7962`. Both records are `0x0080` event-0 collision cells. Each QA
seeds the adjacent cell and enters the ROM cell with ordinary Down input. The
Lua selects `source_activity_return`, then the shared transition restores the
captured source scene at its collision-probe lead and heading `0x8000`;
Debug/Release route traces and full 320x224 captures match. These runs
establish those two exits, not every resource-scene return record. Independent
Genesis interior/exit captures are unavailable.

## Event-table values in maps 0-9

Only scene profiles 0-9 carry event tables. The following lists show the raw
bytes referenced by event-bearing rectangles. Event codes 2-5 take the
conditional maglock branch only in Scene 20 or resource-activity mode
(scenes 21-43). Ordinary field scenes 0-19 publish those codes to their event
tables, except where a scene-specific service handles the event first.
Codes `0x18..0x1F` take direct branches, with event `0x19` rewriting to event
8 when its context check fails. Scene-2 events 2 and 11 and scene-4 events 3
and 4 also have source-specific service routes. For other generic fallthroughs
the byte is consumed by the field/world-activity route.
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

### Scene 6: Wilderness Area 1 crosswalk

The ROM profile at `0x14B890` has six event-bearing collision records. Records
193/194 (events 0/1) and 190/191/192 (events 8/9/10) use event-table rows
0/1/8/9/10 at `0x14B8EC`; every one contains `0x0D`, selecting field map 13.
The per-record bounds align with Mike's RPG Center Wilderness Area 1 markers 2
through 6: David Owlfeather's lodge, Red Buffalo Woman's lodge, Cave 1, the
unidentified cave, and Cave 2. Those names are a guide crosswalk; the ROM
contains the numeric target and does not name any of those destinations.

Field map 13 is reused by the separate Scene-2 Jump House route: Scene-2 event
4 targets map 13 and map 13's event-30 records start the Gunderson interaction.
That reuse means the Scene-6 table result alone does not establish a unique
room, speaker, interior script, or return route. `0x167AE` checks
`FFFC1E` bit 19 before selecting Scene 13's additional world script; the flag's
quest meaning and that script's content are not linked to these five Scene-6
triggers. For record 193/event 0, indexed title service 2 at `0x53B78` checks
`FFFC1E` bits 1/2, opens Owlfeather's `0xF4B68` first-contact dialogue, and
consumes the event before the default scene-13 route. Physical-input QA confirms
the opening dialogue and the C-choice brother page; it does not cover the
closed gate, the other opening choices, or the complete return. See the
[Owlfeather route audit](visual-audits/owlfeather-scene6-event0.md).

For record 194/event 1, indexed title service 18 at `0x54322` also suppresses
the event-table scene-13 route. It selects the gift page at `0xF4DB8` when
`FFFC1E` bit 17 is set and bit 18 is clear; otherwise it follows the no-gift
root at `0xF4DCC`. The gift completion sets trigger bit 18 through `0x2196C`,
sets/clears Notebook tip bits 5/6, changes field effect bits 29/30, and returns
to Scene 6. Debug/Release physical QA reaches the record by ordinary movement
and checks the gated dialogue, state writes, and completed same-map return.
Other Scene-6 records, physical post-Ito/map-item coverage through this entry,
and an independent Genesis dialogue capture remain open. See the
[Red Buffalo route audit](visual-audits/red-buffalo-scene6-event1.md).

Scene-6 record 189 at `0x14C0E0` is event `0x1F`; it bypasses the event table
and sets the direct continuation at `0x0DB26`. In ordinary field mode
(`FFDF40 == 0`), this routine displays the shared Main Menu from text at
`0xC16A5` and dispatches its four choices through `0xC1668`: Call Taxi
(`0x134D2`), Vid-Phone (`0xE4C8`), Cyberspace (`0xDEE8`), and Exit (`0xE65A`).
Vid-Phone checks the contact bitfield and shows the ROM's no-contacts message
when empty. The Lua call list filters all 13 contact records from the ROM
contact bitfield, pages the rows, resolves each first/repeat dialogue and
portrait, and retains per-contact call-history state. ROM-rooted service
trees are implemented for Capt. Quinton, Alesandro, Sharkey, Kipp David, Wilma Temmenhoff, and Sgt. Macklemann,
including their wallet, inventory, contact-flag, deck-record, and
map-destination effects. The menu
is rooted at `0x1096F4`, with Firepower and Contacts nodes at `0x109750` and
`0x109780`. Other contact-specific service paths remain unresolved.
Cyberspace checks deck,
MPCP, and datajack facts before showing Enter Passcode / System Search / Exit;
known passcodes enter their selected system, while System Search follows the
ROM's eleven permutation rounds and random system choice. The Matrix entry
script waits 20 frames, builds the typed session, and enters the deck/program
loop at ROM `0x181F8`/`0x1822C`. The node-action route represents the separate
world-mode path at `0x54662`, not the initial jack-in screen. `FFDF40` nonzero
still follows the separate scene-specific branch.

For the nonzero branch, the ROM switch at `0x0DBF8` uses the current resource
scene (`FFFBF8`) to seed Matrix node `FFF118` and system `FFFBBC`. The shared
`0x0DE5E` Main Menu offers Cyberspace and Exit; selecting Cyberspace at
`0x0E40C` passes the seeded target directly to the 20-frame Matrix entry at
`0x181F8`, without the ordinary field terminal's Passcode/System Search menu.
Lua now owns this recovered map in `field_activities.lua` and returns a target
that the host forwards to `matrix_entry.lua`:

| Resource scene | System | Node |
|---:|---:|---:|
| 21 | `0x16` | `0x0D` |
| 22 | `0x16` | `0x0D` |
| 23 | `0x16` | `0x06` |
| 24 | `0x16` | `0x08` |
| 25 | `0x16` | `0x0F` |
| 26 | `0x18` | `0x16` |
| 27 | `0x18` | `0x03` |
| 28 | `0x18` | `0x09` |
| 29 | `0x18` | `0x1A` |
| 30 | `0x14` | `0x07` |
| 31 | `0x14` | `0x02` |
| 32 | `0x14` | `0x09` |
| 33 | `0x14` | `0x18` |
| 34 | `0x14` | `0x0E` |
| 35 | `0x15` | `0x03` |
| 36 | `0x15` | `0x04` |
| 37 | `0x15` | `0x08` |
| 38 | `0x15` | `0x13` |
| 39 | `0x15` | `0x14` |
| 40 | `0x15` | `0x12` |
| 41 | `0x17` | `0x15` |
| 42 | `0x17` | `0x09` |
| 43 | `0x17` | `0x0B` |

The E40C gate checks are represented by the shared deck, nonzero MPCP, and
active-runner datajack facts. Their source comparisons are `FF018F` bit 0,
`FFFBBE` nonzero, and bit 0 of the active runner record at `FFFC04 + 0x90`;
the runtime's field facts read the live deck record and cyberware bit rather
than duplicating those RAM addresses. Missing prerequisite data remains a
source boundary. Physical-input QA now confirms Scene 21 record 121 reaches
Matrix system 22/node 13, and Scene 26 record 126 reaches system 24/node 22.
The other 21 target rows have Lua mapping coverage but still need their own
physical routes before claiming them independently defended.

`FieldSceneRuntime` now connects the normal event-31 continuation to the Lua
four-choice task. Call Taxi uses `0xD4456` / `0xD445D` for the seven region and
cursor mappings, draws the extracted cab-map/cursor assets, and fades into the
selected field scene after confirmation. The taxi route QA checks the settled
screen, clean return on the current region, and a completed Downtown scene-0
arrival after fade-in. Vid-Phone page/call screens and Matrix script screens
have focused route tests and 320x224 dumps; the phone runtime checks both
Sharkey passage destinations and item outcomes. Separate ordinary-input QA
physically reaches the Scene-0 terminal-to-Vid-Phone route, the Scene-21
record-121 resource-scene Matrix route, and the Scene-26 record-126
resource-scene Matrix route. The resource routes check that the Matrix session
keeps its ROM-seeded system/node target after the source entry wait. These
checks do not certify the full Matrix visual system, the other 21
resource-scene event-31 routes, or contact-specific service transactions. The
map-viewer preview remains a readable content and ordering aid, not
pixel-fidelity proof.
The scene-6 default actor list is empty. The guide's named people therefore
remain site cross-references until their actor service and gated placements
are source-traced; they are not populated as scenery residents. See the
[Wilderness Area 1 map](https://mikesrpgcenter.com/shadowrun/maps/wild1.html)
for the secondary coordinate/name crosswalk and
`gameplay/world/areas/scene-06-site-routes.tsv` for the clickable route rows.

### Scenes 7 and 8: wilderness route crosswalk

The extracted Scene 7 image visually matches Mike's RPG Center Wilderness Area
3 map, and Scene 8 matches Wilderness Area 2. This is a guide crosswalk, not a
ROM scene-name lookup. The [Wilderness Area 3 guide](https://mikesrpgcenter.com/shadowrun/maps/wild3.html)
labels marker 1 Cave 2, marker 2 Cave 3, and marker 3 Stark. Scene 7 records
207 and 208 (events 8 and 9) align with markers 1 and 2; record 206 (event 0)
aligns with the longhouse at marker 3. All three route bytes select shared
field map 13. Record 209 (event 10) is a separate east-edge trigger with no
named guide marker. The map's default actor list is empty; the guide's Stark
label does not authorize synthesizing an NPC or assigning entry dialogue.

The [Wilderness Area 2 guide](https://mikesrpgcenter.com/shadowrun/maps/wild2.html)
labels Scene 8's four edge routes as Cave 1, Cave 1, a cave with no listed
point of interest, and Cave 3. Scene 8 records 198–201 (events 8–11) align
with those four map markers in order. Every table row selects field map 13;
the guide wording does not establish that the destination interior is empty,
or identify its room or return point. The default actor list is also empty.
The fixed red-crested, blue-crested, and winged-post sprites are extracted
from the scene's priority plane and annotated as scenery props; the timber
bridge artwork visually matches the Scene 6 bridge entity. No actor or
interaction is inferred from these static images. See
`gameplay/world/areas/scene-07-site-routes.tsv`,
`gameplay/world/areas/scene-08-site-routes.tsv`, and
`gameplay/world/areas/scene-08.tsv` for the route and entity placements.

The [Wilderness Area 4 guide](https://mikesrpgcenter.com/shadowrun/maps/wild4.html)
matches the extracted Scene 9 map. Event 0 at `0x1B578E` crosses the open
village gate aligned with guide marker 3, Sinsearach. Events 8, 9, and 10
(`0x1B5798`, `0x1B57A2`, `0x1B57AC`) align with guide markers 4, 1, and 2.
The four table bytes at `0x1B4E76` select field scene 13. The subsequent
`0x167AE` selector returns before choosing a path when FFFC1E bit 19 is clear;
when set, scene-index 3 resolves runner value 6 and compares marker 7 against
the primary and secondary runner markers. A match retains field scene 13;
otherwise script index 3 selects the list at `0x1706C`. Its one record uses
template `0x172CA` and position data at `0x17082`: actor name `lone11`, kind
10, palette line 1, item action 3, position `(0x1100, 0x0300)`, heading
`0xC000`, and ordered overrides `+0x2E=0xC0`, `+0x9E=0x10`. The extracted
`world-activity-actors.tsv` preserves that row; Lua exposes the selected list
through `World.world_activity_actors_for_script(13, 3)`. The field runtime queues
the Lua descriptors and publishes the selected actor during the actor-service
pass. Debug/Release physical-input QA reaches scene 13 with bit 19 set and both
runner markers clear, then checks script index 3, the actor profile and
position, and a full-frame capture. The actor gate and materialization are
connected; that test seeds bit 19 and does not exercise the path that sets it
or the actor's dialogue/contact behavior. Bit 19's Ilene connection is now
source-backed: Scene-2/event-0
service 19 arms `FFE182` after the gift prerequisite, and the follow-up
Scene-20 activity at `0x54402` opens a text page that names Ilene while
setting bit 19. This still does not establish the Scene-13 list's dialogue,
whether its actor is Ilene, or a distinct cave interior. Scene 9's default actor list is empty. The feathered gate
monument, fence panels, and two visually matching timber bridges are authored
as scenery entities in `gameplay/world/areas/scene-09.tsv`; static rocks and
trees are not classified as people.

## Converted code and runtime wiring

| ROM/source operation | Converted C++ function or runtime owner |
|---|---|
| `0x3232` scene-profile table | [`field_scene_profile_table_entry`](../../src/field/scene_profile_conversion.cpp) and `kFieldSceneProfileTable` in the matching header. Runtime source assets are loaded by [`load_field_session_source_assets`](../../src/integration/field_session_source_assets_conversion.cpp). |
| `0x0EE10` collision rasterizer | [`rebuild_terrain_collision_map`](../../src/field/terrain_map_conversion.cpp) |
| `0xE7C8` terrain sampler and `0xB77E` directional probe | [`evaluate_terrain_collision`](../../src/field/terrain_probe_conversion.cpp) and [`probe_entity_directional_collision`](../../src/entities/movement_probe_conversion.cpp) |
| `0xB7B8` event dispatcher | [`advance_entity_movement_and_mode_transition`](../../src/entities/movement_mode_transition_conversion.cpp); typed continuation enum is in `movement_mode_transition_conversion.hpp` |
| `0x1651E` projected world interaction | [`apply_world_map_effect`](../../src/world/world_scene_entity_conversion.cpp) |
| `0x167AE` table byte to field/resource/scripted activity | [`resolve_world_activity_scene_id`](../../src/world/world_activity_routine_conversion.cpp) and [`run_world_activity_routine`](../../src/world/world_activity_routine_conversion.cpp) |
| `0x1F5CA` exit and event-route orchestration | [`run_field_exit_orchestrator`](../../src/field/field_exit_orchestrator_conversion.cpp); collision policy in [`field_events.lua`](../../gameplay/scripts/field_events.lua), location route policy in [`building.lua`](../../gameplay/scripts/building.lua), adapted by `GameScriptRuntime` |
| `0x20E48` maglock / `0x20EC2` Security Terminal | [`FieldInteractionRuntime`](../../ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp), reached through the field scene controller |
| `0x58D94` floor selector / `0x58DB2` handoff | [`ElevatorSelectorRuntime`](../../ghidra_conversion/qt_runtime/src/field/elevator_selector_runtime.cpp) and [`FieldSceneRuntime`](../../ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp) |
| Live Qt route selection and world-activity callback | [`FieldSceneRuntime::run_field_exit_orchestrator`](../../ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp) and the coordinator callback setup in that file |

The Ghidra output is a cross-check, not the runtime implementation:
[`fn_0000B7B8.cpp`](../../export/ghidra-c/fn_0000B7B8.cpp),
[`fn_000167AE.cpp`](../../export/ghidra-c/fn_000167AE.cpp), and
[`fn_0001F5CA.cpp`](../../export/ghidra-c/fn_0001F5CA.cpp). The native runtime
uses typed scene IDs, collision rectangles, event records, and continuation
enums rather than ROM address interpretation.

## Remaining route gaps

- The final TSV column marks which exact records have focused physical-input
  coverage. The remaining coordinates are profiled, but their physical
  reachability is not thereby proven.
- `FieldSceneRuntime::record_field_activity_start` connects maglock,
  Security Terminal, events 25 and 27, event 29, the ordinary event-31
  terminal, and the Scene-26 event-31 resource main menu. Scene-4 record 96
  (`0x15BDF6`, event 27) now routes through its entity-owned Spirit Eyes Lua
  link; ordinary-Up Debug/Release QA checks the raw `0xFFDB` dispatch and the
  first-contact/horn-request screens. This continuation starts its activity
  without publishing a new `FFFBF9` mode-event byte, matching the early ROM
  branch. Scene-47 event 25 now has a Lua route for the context-`0x13` direct
  continuation, but the contextual physical-input route and the complete
  conversation graph remain untested.
  Scene 21 record 121 (`0x1D7A20`, x `0x00E0..0x00FF`, y `0x00C0..0x00DF`,
  event `0x1F`) now has a physical-input route after gated Fuchi entry: Up
  from the adjacent southern cell opens Lua's resource `Main Menu`, and
  Cyberspace enters system 22/node 13. Scene 26 record 126
  (`0x1D9E5A`, x/y `0x0200..0x021F`, event `0x1F`) has a
  physical-input QA route: Up from the adjacent cell opens Lua's resource
  `Main Menu`; Down selects `Exit`; A restores the captured Scene-2 field
  state. A second ordinary-input QA chooses Cyberspace and checks the Matrix
  session snapshot receives system 24/node 22. Together, these routes physically defend
  the direct Matrix entry paths in Scenes 21 and 26; the full 21–43 node/system table has Lua
  mapping coverage. The normal-mode
  four-choice task and taxi/phone/Matrix handlers are connected in Qt, with physical-input coverage for Scene-0 Vid-Phone and the resource-scene Cyberspace choices. Other scene-specific event-31 paths, events 26, 28, and
  30, and the ordinary terminal cursor graphic remain unresolved. The final
  item path in the Spirit Eyes ritual now has a traced producer and consumer:
  Scene-47 handler `0x53F50` sets Group Item bit 2, and Scene-4 handler
  `0x53E98` consumes it before setting story bit 9.
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
