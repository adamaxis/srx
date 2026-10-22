# Field map event route planning

Date: 2026-10-08

**Status:** partial. `field_events.lua` selects a named action for every
accepted collision; `building.lua` selects direct scene-2 location routes,
ROM event-table destinations and their typed field/resource/scripted/halted
destination mode, elevator entry groups, and selected floor destinations.
The embedded Lua package also defines the New Game scene-entry action and
Stoker story/menu branches. C++ validates those script results and applies the
typed engine operations. Selector timing, animations, transitions, and
downstream activity results remain native. Some ROM rows still have no
identified destination or reverse route.

## ROM-derived route model

The complete 56-scene/892-trigger source census, per-record coordinates, event-table bytes, numeric destination classes, and C++ cross-references are in the [field-map trigger profile](../field-map-trigger-profile.md) and its generated [scene index](../field-map-scene-profiles.tsv) / [trigger records](../field-map-trigger-records.tsv). This audit keeps the physically exercised routes and their QA status.

The ROM keeps physical collision and its consequence in separate steps. The
field movement path publishes the collision result; `0xB7B8` derives the
event identity from that result and starts the matching interaction. Scene
event tables are separate map data. The source-backed Qt import already keeps
each map's collision rectangles and event table as individually named host
records.

| Source input | Lua policy and native service | Result in the connected path |
|---|---|---|
| Scene 2, collision value `0xC2` at ROM `0x173138`, field mode 2/event 2 | `building.lua` location route, then Lua story/menu package | `0x1F65A` selects the generic Stoker arrival dialogue; Lua resolves first/repeat/ordinary story state and supplies manager choices; native handlers apply selected actions |
| Scene 2, collision value `0xCB` at ROM `0x173142`, field mode 2/event 11 | Ares service script | `0x1FA78` selects the Ares mode-text prompt; A opens the shop and Start returns |
| Scene 2, paired collision values `0xC0` at ROM `0x173124` and `0x17312E`, event 0 | `building.lua` reads the active map's ROM event table | Row 0 in `0x172CDC` unconditionally selects numeric scene `0x14` (scene 20); the identifier does not supply a place-name. `0x167AE` separately checks FFFC2C bit 26 at `0x1689A`; when set, it clears the bit and schedules `0x1DBAA`'s timeout without changing the destination. |
| Scene 20, record 110 at `0x1D96DE`, raw `0x00C0`, event 0 | Scene-`0x14` generic interaction | No event-table destination exists. `0xB7B8` -> `0x1651E` returns carry on zero event and begins the normal field transition; `0x16E30`, `0x2FA4`, and `0x2F32` preserve/restore the source scene context and actor anchors. Debug/Release QA reaches the record with eight ordinary Left updates around adjacent record 109's `0x7F` wall, then held Up; the first raw C0 sample is followed by the zero-event dispatch on the next service update. On return, the route restores the Lua-selected source collision-probe lead anchor at (3884,10700), 350 source position units (21.875 map pixels) beyond the contact at (3884,10350), outside the expanded trigger envelope. scene2-c0-return-source.tsv records the selected bank and positions; the CTest asserts that the runner resumes with the opposite of the entry heading. ROM 0xBC36/0xBD34 and 0x2F32 support the anchor and camera. The opposite-facing choice is the requested route policy; no independent Genesis capture confirms that heading write. Held Down clears the retry latch before fresh Right. |
| Scene 2, collision value `0xC1` at ROM `0x17314C`, event 1 | `building.lua` reads the ROM event table and passes the target to the world-activity service | Row 1 in `0x172CDC` selects numeric scene `0x1A` (scene 26); its resource bundle has bound 3 |
| Scene 26-29 family, paired `0xDD` collision rectangles, event 29 | `building.lua` resolves the source child group and selected destination; native code presents the selector and transition | `0x58D94` calls `0x58DE0`; the typed child scene group contains scenes 26, 27, and 28. Debug/Release physical QA covers scene 26->28 and the later scene-28 trigger after the source activity timeout expires. This is separate from the Stoker scene-2 event-2 dialogue route; map identity remains open |
| Scene 20, collision value `0x82` at ROM `0x1D9724`, event 2 | Building interaction continuation | The electromagnetic-lock handler `0x20E48` opens its generic prompt. Physical QA covers quality-2 passkey success, quality-1 insufficiency, Samurai no-skill result, Decker no-kit partial result, and Decker kit-assisted success. The no-kit roll reduces selection 2->1 without sound/map change; physical recontact reloads selection 2 through `0xB994`. A QA-seeded item `0x15` plus source-seeded three-roll success then requests `0x4B`; `0x2BD2` changes map index 483 from `0x82` to `0x81` only after the result is acknowledged. No-skill/partial/insufficient branches preserve the map and add no action sound. Kit acquisition and other passkey qualities remain open. Seed 0 now follows the ordinary scene-20 zero-roll branch: the alert page is static for 30 frames, selection/inventory/map stay unchanged, and ROM 0x1E04A schedules timeout marker -1 for 0x78 frames without a scene-20 sound request. A field frame hands world_active=0xFFFF to the 0x1DE00 entity-target selector; its downstream actor/effect boundary remains unresolved. Exact independent maglock visuals remain open. |
| Scene 26, collision record 117 (`0xA2`) at ROM `0x1D9E00`, world-mode-2 event 2 | Generic electromagnetic-lock continuation and passkey action | `0xB7B8` reaches `0x20E48`; descriptor-route CTest seeds active runner item `0x14`/quality 2, verifies sound `0x4B`, and checks sampled map index 712 changes `0xA2` to `0xA1` before ordinary Right input crosses the passage. Traversal clears that marker from the active collision buffer |
| Scene 26, collision record 110 (`0x82`) at ROM `0x1D9DBA`, event 2; then `0xD8` records 124/125 at `0x1D9E46`, event `0x18` | Shared maglock continuation followed by typed Security Terminal continuation | The same live passkey changes sampled map index 525 from `0x82` to `0x81`; ordinary input follows the collision grid around the `0x7E` wall into the D8 lane. `0xB7B8` requires two negative samples and selects `0x20EC2`; the descriptor-spawn route opens the Electronics/Walk Away prompt. A separate checkpoint QA remains as focused coverage |
| Any remaining in-range map-table event | Active map's own event table | The selected row is passed to the existing world-activity coordinator; missing/out-of-range tables remain unresolved |

## Scene-20 entry marker and timeout

The Hollywood Correctional doorway is not quest-gated: Scene 2's event-table
row 0 always selects field Scene 20. After the destination profile loads,
`0x167AE` checks FFFC2C bit 26 at `0x1689A`. If set, it clears only that bit
and calls `0x1DBAA` to arm the 120-frame timeout; it does not replace or block
Scene 20. The host now reads FFFC2C separately from FFFC1E, preserves the
timeout across the final `0x2BD2` scene re-entry, and leaves the bit's story
meaning and producer unresolved. Debug/Release CTest
`shadowrun_ghidra_conversion_qt_field_scene20_entry_gate_qa` exercises the
ordinary Scene-2 doorway with bit 26 seeded, verifies the bit is consumed and
the timeout remains armed, and writes a full-screen `actual.bmp`. The existing
physical-entry QA covers the clear-bit case. The image was inspected as a
native render; no independent Genesis capture of this route has been identified.

The first Stoker approach is automatic. `0x59C5E` sets `FFE18B` before the New
Game opening. The field entry at `0x075E` initializes the ordinary scene-2 map
and Joshua at `(0x3CE0, 0x2D00)`. After its 31 entity-service warmup passes,
`0xACAE` sees the still-set activity marker; `FFE18C` is zero, so the handler
sets Joshua's target heading to zero and installs the normal field movement
continuation `0xB574`. That continuation uses the same directional movement,
terrain probes, and collision dispatcher as controller-driven movement. It
reaches scene-2 record 91 (`0xC2`), where `0xB7B8` publishes event 2 and the
ordinary fade/field-exit path selects Stoker through `0x1F5CA`/`0x1F65A`.
After that first exit-orchestrator pass, `0x075E` clears `FFE18B`. The walk is
therefore a one-shot scripted movement on the real map, not a unique cutscene
animation and not a controller-input route. The C0 records at `0x173124` and
`0x17312E` are a separate doorway that selects scene 20; they are not the
Stoker greeting.

Each accepted movement-collision event is evaluated by
`field_events.lua` from a named `FieldCollisionEventCode` and typed ROM-derived
state facts. `GameScriptRuntime` rejects unknown actions, enum names, and
malformed values. `FieldMapEventSystem` records the input and validated Lua
decision; the movement controller executes the selected continuation. After
field-exit orchestration begins, `building.lua` selects the direct Stoker/Ares
routes or reads the destination byte from the active ROM event table. Lua also
classifies that destination into a named `WorldActivityMode` using the ROM's
`0x167AE` boundaries: field scenes 0-20, resource activities 21-43, scripted
activities 44-55, and halted for 56-255. The Qt executor consumes that enum;
it no longer infers the destination class from a raw byte on this route.
Source-internal timeout and descriptor overrides still use their own typed
branches. C++ loads the selected destination and performs the scene transition. The
completed-intro scene-entry event remains in the same script package's
stateful rule registry, scoped to the ROM-derived scene-2 spawn and limited to
once per session. Native C++ still performs movement and scene changes. Lua
resolves event-29 child groups and selected destination IDs; native C++ owns
the selector's timing, lift animation, fades, and map loading. Scene-20 record
110 C0 still has no event-table row or proven reverse destination, as
documented in the ROM-wide profile.

The ROM-derived scene profiles and extracted collision records show four
additional numeric event-29 groups: triggers in scenes 21-25 target the
21-24 group; 30-34 target 30-33; 35-40 target 35-39; and 41-43 target 41-42.
The entry scenes 25, 29, 34, 40, and 43 are not listed as child destinations.
The runtime currently chooses group index 0 for those scenes. Descriptor-spawn
checkpoint QA physically reaches event 29 from scenes 25, 34, and 40. Scene 25
continues through `25->22->23->24->21->22`, physically entering each member
trigger in group 21-24 and carrying the floor index through each handoff. The
scene-34 route continues through `34->31->32->33->30->31`, physically entering
all four member triggers in group 30-33. Scenes 31-33 need six ordinary Right
updates to align with the collision-defined trigger lane before Up reaches
event 29; scene 30 is centered at spawn and uses Up. The scene-40 route
continues through `40->36->37->38->39->35->36`, physically entering every
member trigger in group 35-39 and carrying the selected floor across the
handoffs. Scene 43 starts a chained `43->42->41->42` route that physically
tests both member-scene triggers in group 41-42. Scene 29 has separate trigger
QA for group 26-28. The initial non-member routes use a test-only interaction
mode setter and do not verify the ordinary resource-scene entry path. The
resource setup at `0x167AE` clears current-floor `FFF0C8` for scene IDs
`0x15..0x2B`, supporting index 0 on normal entry; saved-floor restoration
remains open.

In the chained 26->28 and 40->36 routes, ROM `0x1E2E0` starts a 60-field-frame
world-activity timeout. Selector and fade processing do not consume those
field updates. The QA now waits for the remaining timeout before using ordinary
input to enter the destination trigger. The typed movement transition also
reports `world_activity_already_running` when the native activity coordinator
refuses to begin another activity; it does not report that a new selector
opened.

The Lua route policy is `building.lua:resolve_field_location_route()`. It
selects the Stoker dialogue and Ares service directly, then resolves other
valid events against the ROM event table supplied by the active map profile.
The Qt field-exit coordinator consumes that typed result to select the
dialogue event code, service route, or normal area-event continuation. It does
not interpret ROM addresses at runtime.

Unconverted location-to-route bindings remain beside reusable map entities in
`links.tsv`. The eight Ares, Boris, Gates Undersound, Tarislar City Inn,
Owlfeather, and Red Buffalo field-service routes, thirteen entry-dialogue
bindings, and eight conditional map overrides use
`gameplay/world/entities/<entity>/interactions.json`; the package loader puts
them in Lua's versioned content registry, and `world_content.lua` joins them
to the entity instance in the current scene. `building.lua` owns route
selection: direct same-map services require their source mode, JSON routes can
declare named conditions and explicit source-route policy, and ordinary area
links are accepted only when their numeric destination agrees with the active
ROM event-table row. Unlinked events continue through the same imported table
path. This gives scene entities reusable route data without assigning unknown
interiors or treating every collision event as a door.

The initial source-verified links cover Scene 2: Stoker event 2 (same-map
dialogue), Ares event 11 (same-map shop service), Jump House event 4 to scene
13, Jackal's Lantern event 5 to scene 19, Boris' Greenhouse event 6 to numeric
resource scene 26, Ghoul Warehouse event 8 to numeric scripted scene 52,
Hollywood Correctional Facility event 0 to scene 20 when the source table
selects it, and Little Chiba's event 1 to numeric resource scene 26. The
destination bytes are cross-checked against the imported ROM table at runtime.
Scene-2 event 3 also publishes to its event-table row in ordinary field mode;
it does not invoke the conditional maglock. Scene-4 events 3 and 4 have
separate Medicine Lodge and Hospital service routes. The shared conditional
maglock branch for event codes 2-5 applies in Scene 20 and resource-activity
scenes 21-43. Event 7 has its conditional Vigore handler, and unresolved or
guide-only site identities remain on their existing source route. This initial
entity set is not a complete route census. In particular, the ROM census's
892 rows are event-bearing collision rectangles, not 892 independently proven
doors or reversible links.

The reusable route model now supports several source-confirmed outcomes for
one event. Remaining entity `links.tsv` rows use `route_priority`; migrated
entity JSON uses `route.priority`. Both use stable condition IDs that resolve
through `gameplay/dialogue/location_conditions.json`.
Conditions cover story facts, active/current runs, alternate results, and the
building-interaction mode. Missing facts evaluate to `unknown`; an unknown
candidate that could outrank the known route and equal-priority true matches
produce an explicit source boundary instead of silently falling through to
the ROM event table. Entry passages use a separate `entry_condition_id` and
named Lua task/location fields. The host preserves the typed destination
during the modal and consumes it only when Lua returns the declared
`entry_continue_result_id` or typed `entry_continue_outcome_type`. Jump House now uses this shared path: service 22's
clear `FFF0D5` branch opens its ROM-derived Lua passage and then resumes scene
13. Its ordinary-input QA checks the dialogue result and the clean destination
redraw. Lua route tests cover story-fact and run-state variants, priority
ties, unknown-fact boundaries, and entry task selection.

The entity sweep now has 68 explicit Lua link rows in 57 entity files across
scenes 0-7: 57 event-table routes, three Tarislar service-21 overrides, seven
field services, and one same-map dialogue route. Event IDs 2-5 alone do not
identify a maglock route: `0xB7B8` selects the lock prompt only in Scene 20 or
resource-activity mode (scene IDs 21-43). Field scenes 0-19 publish those
events to their map tables. Five
Scene-5 sites select resource-activity destination 35. The four Weapon's
World, Wire-Masters, Merlin's Lore, and Microtronics links remain numeric-only;
Renraku Offices now has a conditional title-service-12 arrival page when
story bit 16 is set, bit 12 is clear, and alternate result is zero. Its JSON
graph returns `resume_source_route`; the host then resumes the already selected
activity-35 destination. Penumbra
Fuchi similarly gets its title-service-14 cyber-heart entry page when bit 25
is set, bit 14 is clear, and alternate result is zero. Both preserve their
ROM event-table destinations (35 and 21) after their respective entry tasks.
Gates Undersound's title service 15 now uses the same link mechanism: after
bit 11 is set and bit 15 is clear, it shows the one-page Ogami departure text,
clears FFFC30 bit 26, sets bit 27, marks bit 15, and returns to scene 0 even
though the generic event-table row points to activity 21.
Another link records Scene 7's longhouse event-0 destination as field scene
13; the guide's Stark label does not establish the room or speaker. Each
ordinary area row is joined to an exact `interactables.tsv` ROM record and its
event-table byte; the route resolver checks that byte before selecting the
authored numeric target. These additions cover only the traced entry pages:
the remaining prompt branches are open. Renraku/Fuchi links now encode the
FFE184 index/marker overrides and their Debug/Release physical-entry QA checks the
resulting resource state. Debug/Release ordinary Down routes now reach
Renraku activity-35 record 121/event 0 and Fuchi activity-21 record 102/event
0. Lua explicitly selects the typed `source_activity_return` route for these
null-table resource exits. The shared resource transition then restores the
source scene at the captured ROM collision-probe lead with heading `0x8000`;
both route traces and all full-screen captures match across builds. These exit checks seed an adjacent
activity cell, and independent Genesis interior/exit visuals remain open. The
four Scene-5 stores still need
their shop catalogs and activity return behavior. Scene-1 Tarislar now has physical QA for its first-authorized
Harlequin conversation, its guarded leave branch, and its gated tomb-departure
Yes route into scene 54. The first conversation returns to scene 1; the tomb
route requires story bits 30, 21, 16, 28, 26, and 27 plus alternate result 0.
The repeat information branch, upstream Frosty choice, pending-descriptor
override, and scene-54 completion/return remain open.

### Puyallup Ork Gang contract target

Scene 1 records 76 and 77 (`0x17D9FC` and `0x17DA06`) are separate event-9
rectangles at the Ork Gang Hideout. Their shared event-table row 9 at
`0x17D629` selects Scene 13 when no exact active run target intercepts the
route. ROM `0x1F086` compares the current area/event pair in FFFBF8/FFFBF9
against the selected gang-contract target in FFFBEE/FFFBF. For Puyallup
area 1/event 9, it queues actor-list `0x1D6D4A` with resource profile
`0x122054` and presents the passage from text `0xFE0DE` (descriptor
`0xFDE26`). The pending descriptors then override the generic Scene-13 route
and select Scene 15. `run_gang_contract.lua` owns the target match, passage
task, and actor-group data; the entity link remains the reusable Scene-13
fallback and records return policy.

`shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa` physically
reaches the Puyallup rectangles with ordinary Up input in Debug and Release.
Record 76 with an exact area-1/event-9 target shows the passage and enters
Scene 15 with four actors. Record 77 with an active area-2/event-9 target and
record 76 with no active contract both follow row 9 to Scene 13 without the
passage. The same test exercises Penumbra Scene 3 record 100/event 4: the exact
area-3/event-4 Eye-Fivers target enters Scene 15 with the third actor group,
while a different target and no active run follow row 4 to Scene 13. All 26
generated files match across builds by SHA-256; the full 320x224 contact,
passage, destination, and fallback captures were inspected. Together with the
existing Scene-2/event-9 route test, all three ROM target tuples are physically
covered. These are host captures, not independent Genesis comparisons. The
gang fight result, reward settlement, and independent visual reference remain
open. See the [Ork Gang target-entry audit](puyallup-ork-gang-contract-entry.md)
and [Eye-Fivers target-entry audit](eye-fivers-gang-contract-entry.md).

The named-route sweep still has source-profiled gaps. Scene-1 Tarislar Garden
Apartments has indexed service 21 and story-bit gates. The first-authorized
conversation, guarded leave path, and gated tomb-departure Yes route are
Lua-backed and physically QA-covered. The repeat information branch, upstream
Frosty action, pending descriptor override, and scene-54 return are not fully
connected. Other map-guide labels often have only a numeric event-table
destination; the guide name does not establish an interior dialogue. The
per-scene route manifests now reflect the `0xB7B8` mode gate instead of
inferring a maglock from event IDs 2-5 alone. See the [Tarislar audit](puyallup-tarislar-apartments.md)
and each `scene-NN-site-routes.tsv` manifest before adding a location row.

The entity-owned `links.tsv` rows now carry return-anchor and return-facing
policy for all 60 traced map-transition variants across scenes 0-7, except
Scene 4/event 6, whose connection to the Medicine Lodge remains unresolved.
For links marked `collision_probe_lead`, the field coordinator captures the
already-expanded 0xBC36/0xBD34 point 0 and its camera before leaving. Activity
returns apply that anchor to the restored source-scene snapshot; the shared
Lua route policy turns the runner away from the entrance. Scene-2 C0 -> scene
20 returns from contact (3884,10350) to (3884,10700). The ROM trace establishes
the position/camera anchor; the heading flip is the requested host policy and
is not claimed as a recovered ROM write. Debug and Release route tests assert
the typed policies, and the physical Scene-2 return tests cover the contact
and restored field view. No independent Genesis capture verifies the return
heading. The 892 event-bearing collision rectangles are not 892 proven doors;
untraced reverse links and the other scene maps remain open.

The scene-2 C1 route is exercised from the New Game spawn with normal
controller input. It walks east through the open southern lane, then north
through record 93's doorway cell, records event 1, and follows row 1 into the
scene-26 resource activity. The source geometry explains why the approach
cannot cross record 47's `0x7F` wall directly. The full trace, screenshots,
and remaining visual issue are recorded in the
[scene-2 event-1 audit](scene2-event1-scene26-route.md).

The end-to-end `shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa`
keeps that same runtime alive after the event-1 handoff, traverses scene 26 to
the physical D8 event-`0x18` trigger, and resolves Electronics through the
existing terminal handler. The entry selects floor index 0 from a bound of 3;
the result displays “first floor” after the test's explicit passkey, skill, and
PRNG fixtures. This joins the normal map event, resource setup, physical map
route, and terminal result without adding a runtime subsystem. The
question-by-question review is in
[the scene-26 event-1 terminal audit](scene26-event1-terminal-route.md).

## Full-screen verification

I reviewed all 83 route BMPs as whole 320x224 frames: 12 Stoker frames, 14
Ares frames, 42 scene-20 frames, and 15 scene-2 event-1 frames. The older
routes include the full map and HUD during movement, captured fade phases,
each panel/shop state, and return screens. The scene-2 event-1 route review
found the runner covered by the opaque HUD column during a long uninterrupted
eastward hold; the route trace still reports its native projection visibility
flag as true. I record this mismatch as unresolved rather than accepted as
visually correct. The
scene-2 fade reaches black and the scene-26 destination replaces the map and
HUD without a dialogue or shop left open. Debug and Release route BMP hashes
match.

The separate scene-26 descriptor route produces 500 sequential 320x224 frames
from spawn through the D8 Security Terminal prompt, covering both passkey doors
and the collision-backed corridor between them. All ten chronological contact
sheets and full-resolution named screens were reviewed; the Debug/Release sets
contain 529 matching CTest artifacts. In these captures the runner remains in
the viewport. The QA's logged projection includes the source `0x80` origin,
which `paint_field_entity()` subtracts before drawing; those raw numbers cannot
be compared directly with the 320x224 image bounds. This corrects an earlier
misreading of the second-lock return frame. Independent scene-26 Genesis
frames are still absent.

The scene-20 passkey route adds 45 named full-screen checkpoints plus
`actual.bmp` through the same physical New Game doorway and maglock, with a
QA-only live passkey. Its success page is static during the source 30-frame
wait, requests `0x4B`, and returns to a field image with no modal or stale
controls. All 51 Debug/Release artifacts match by relative-path SHA-256. The
route verifies passkey use at the physical trigger; it does not verify
story-driven passkey acquisition or independent Genesis visuals for this lock.

The insufficient-quality route follows the same physical path with a quality-1
key against selection 2. Ordinary A retains the insufficiency choices, leaves
the key and collision map unchanged, adds no action sound, and ordinary C
returns to the field. Its choice frame remains pixel-identical after eight
idle frames. It writes 44 named checkpoints plus `actual.bmp`; all 49
Debug/Release artifacts match by relative-path SHA-256. This covers one
low-quality case, not the full passkey quality range.

The physical Samurai route selects Electronics with ordinary A. Its active
runner has source skill byte `+0x8A=0`; ROM `0x21692` returns table `0x11CD2A`
(first string pointer `0x11CA59`) before random checks or an action-sound call.
The generic no-skill Electronics result has no choices, remains pixel-
identical through a 30-frame wait, then ordinary A returns to a clean field.
Inventory, map, and selection remain unchanged.

The ordinary-input Decker trials route then tests both positive-skill outcomes.
The New Game Decker has Electronics 1 and no kit. Seed `0x0006` produces one
success against the no-kit threshold, reducing selection 2->1 and showing the
partial-result page without sound or map mutation. After the source 30-frame
wait and A return, holding Up at the contact edge slides along the lock face
and does not create a new collision. The corrected route uses Down to clear
the contact envelope, Up+Right and Down+Left to navigate the corridor, Left
to realign, then Up to physically contact record 117. That blocked probe
reaches ROM `0xB994`, which reloads the collision event selection as 2. The QA
then inserts kit item `0x15` in the
live Decker inventory as an explicit fixture. Seed `0x0004` yields two
successful rolls from three kit-enabled trials, selecting the success page and
requesting `0x4B`. The map remains `0x82` through the result page and changes to
`0x81` on A return via `0x2BD2`. The kit is test-seeded; its story acquisition
is not connected. The zero-success alert page and timeout handoff are covered; the downstream entity-target selector and exact Genesis maglock
visuals remain open. This route writes 62 full-screen BMPs/67 artifacts, with
43 distinct images; all Debug/Release files match by relative-path SHA-256.

Across the seven physical scene-20 CTests, 352 BMPs are present in the active
route directories, with 82 distinct 320x224 images across those routes. The six
routes before the alert route contain 302 BMPs and 81 distinct images; the
alert route contains 50 BMPs and 37 distinct images. The retry path's corridor,
dialogue, choices, result, and return were reviewed as full-screen frames. All
352 Debug/Release BMP pairs match. The alert timeout reaches the entity-target
selection boundary; its actor/effect remains unresolved. These internal captures
verify complete redraws and adjacent field/HUD state, not Genesis fidelity.

The joined scene-2-to-scene-26 terminal route writes 546 whole-screen BMPs
across the event-1 approach, 500 scene-26 movement/interaction frames, result
holds, and field return. All 557 test artifacts match across Debug and Release.
The separate event-1 screenshots match the earlier route exactly; the new
scene-26 sequence was reviewed again because its animation phases differ from
the descriptor-spawn route. The known scene-2 runner/HUD overlap remains open.

The Ares introduction and category were also compared at full size with the
independent longplay references `ares-introduction-5983.png` and
`ares-shop-5985.png`. The prompt shell, green text, title, and offer layout
agree. The recording uses a different saved runner and wallet, so portrait and
balance differences are not treated as presentation defects. Independent
section 03 frames `000029` through `000048` show the same scene-2 map and HUD
as the event-1 route's spawn; frames `000028` through `000082` also show the
Stoker approach, fade, blank panel, and populated dialogue. They support map
appearance and screen order, not event-1 collision input or the later
eastward path. No matching independent scene-20 maglock screen or scene-26
entry was found; those visuals remain unverified.

## Nine-question review

1. **Does it look correct?** The full frames show the complete field, HUD,
   fade, dialogue, shop, and return screens. Stoker and Ares screens have
   independent visual references; the exact scene-20 event does not.
2. **Does the look reflect behavior?** The route traces include the collision
   values, projected event codes, selected script/table route, screen owner,
   controller action, and return. No event tuple is injected in these three
   normal-input route captures.
3. **Are the visuals correct?** Stoker and Ares screen families agree with
   their independent references. Scene-20 panel pixels remain source-asset and
   internal-renderer checked only.
4. **Is it animated?** Field movement changes pose and position; the source
   fade has 17 updates. Stoker dialogue is input-driven. The Ares title prompt
   holds for 30 frames before input; its before/after captures are identical.
  Scene-20 prompt/result waits follow their separate 10/30-frame paths. The
  event-1 route captures moving poses and a 30-frame static scene-26 hold;
  exact scene-26 ambient animation cadence remains unverified.
5. **Is there sound?** No discrete collision or Stoker dialogue sound request
   was established. The Ares prompt path has no direct sound request; shop
   sounds are covered by the shop route. Event `0x30` on successful Security
   Terminal interaction and its stop lifetime remain unresolved.
6. **Are related screen areas correct?** The full images include map, runner,
   HUD, panels, shop state, and return. The Ares status portrait and wallet
   reflect the New Game fixture; they are not compared to a different save as
   if its state were identical.
7. **Do multiple frames support the interpretation?** Yes: 68 native frames
   cover movement, fades, prompts, shop, and returns; independent Stoker and
   Ares samples support the corresponding screen families. The video has no
   collision log.
8. **Does each new UI replace the old UI cleanly?** Yes in the reviewed routes.
   The full-screen route owner replaces the field view, and return frames
   contain no stale dialog or shop controls.
9. **Was the whole time window reviewed?** Yes for the 83 existing route
   captures: the complete available screenshots include movement, fade,
   prompt/shop, and return stages. The scene-26 event-29 selector now has its
   own 459-frame consecutive capture and nine-question review in the
   [elevator selector audit](field-elevator-selector.md). Independent video
   remains a visual reference, not a ROM cadence measurement.

## Verification and remaining work

The focused route-policy CTest and the Stoker, Ares, scene-20, and scene-26
Security Terminal physical-entry CTests pass in Debug and Release. Their BMP
sets have matching names and SHA-256 hashes across configurations, including
the 23 captures for scene 26. The route-policy test checks direct script
priority, scene-2 row 0, a same-numbered event using another map's row, and
unavailable/out-of-range tables. The scene-26 checkpoint and nine-question
review are in [security-terminal-physical-entry.md](security-terminal-physical-entry.md).
The event-29 selector and selected-scene transition have full-route QA from
scenes 26-29, descriptor-spawn checkpoints 25, 29, 34, 40, and 43, and chained
member-scene routes `25->22->23->24->21->22`,
`40->36->37->38->39->35->36`, and `43->42->41->42`, with a nine-question
review in [field-elevator-selector.md](field-elevator-selector.md). The
1,858-frame scene-25 group route, 1,856-frame scene-34 group route,
2,332-frame scene-40 group route, and 988-frame scene-43 group route match
across Debug/Release by relative-path SHA-256. Their full timelines were
reviewed in 48, 48, 63, and 25 chronological whole-screen sheets, respectively.
Physical trigger paths for group members 30-33 are now covered.

This is a route-planning foundation, not complete map/path coverage. The
scene-2 event-1 physical route is behaviorally covered but visually partial
because of the runner/HUD overlap and missing exact Genesis comparison. Scene
26 now has a 500-frame descriptor-spawn CTest: elevator return, A2 and record
110 maglock prompts, fixture-backed passkey successes, map-cell changes
`0xA2 -> 0xA1` and `0x82 -> 0x81`, passage crossing, and ordinary-input entry
to the D8 event-`0x18` terminal prompt. All sequence frames and named screens
were inspected across ten contact sheets and at full resolution where needed;
the 529 Debug/Release CTest artifacts match by relative-path SHA-256. The QA
seeds the passkey and starts in scene 26, so it does not prove the prior
scene-2 resource entry or story-driven access. Additional doorway/object
records, broad terrain recovery, other building item use, reverse transitions,
and world-entry one-shot conditions still need ROM traces and normal-input
route captures.
