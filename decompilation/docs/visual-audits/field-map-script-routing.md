# Field map event route planning

Date: 2026-10-07

**Status:** partial. This slice adds a typed native route policy for the
collision events already connected to the Qt field controller. It does not
cover every map event or establish a first-entry Stoker cutscene.

## ROM-derived route model

The ROM keeps physical collision and its consequence in separate steps. The
field movement path publishes the collision result; `0xB7B8` derives the
event identity from that result and starts the matching interaction. Scene
event tables are separate map data. The source-backed Qt import already keeps
each map's collision rectangles and event table as individually named host
records.

| Source input | Native route choice | Result in the connected path |
|---|---|---|
| Scene 2, collision value `0xC2` at ROM `0x173138`, field mode 2/event 2 | Stoker dialogue script | `0x1F65A` selects the generic Stoker arrival dialogue; A/B advance the source prompt graph and return to the field |
| Scene 2, collision value `0xCB` at ROM `0x173142`, field mode 2/event 11 | Ares service script | `0x1FA78` selects the Ares mode-text prompt; A opens the shop and Start returns |
| Scene 2, paired collision values `0xC0` at ROM `0x173124` and `0x17312E`, event 0 | Active map's event table | Row 0 in `0x172CDC` selects scene `0x14` (scene 20) |
| Scene 2, collision value `0xC1` at ROM `0x17314C`, event 1 | Active map's event table and world-activity resource selector | Row 1 in `0x172CDC` selects scene `0x1A` (scene 26); its resource bundle has bound 3 |
| Scene 26-29 family, paired `0xDD` collision rectangles, event 29 | Floor selector and selected-scene transition | `0x58D94` calls `0x58DE0`; the typed child scene group contains scenes 26, 27, and 28. Physical QA covers scenes 26-29; destination identity remains open. This is separate from the Stoker scene-2 event-2 dialogue route |
| Scene 20, collision value `0x82` at ROM `0x1D9724`, event 2 | Building interaction continuation | The separate electromagnetic-lock handler `0x20E48` opens its generic prompt |
| Scene 26, collision record 117 (`0xA2`) at ROM `0x1D9E00`, world-mode-2 event 2 | Generic electromagnetic-lock continuation and passkey action | `0xB7B8` reaches `0x20E48`; descriptor-route CTest seeds active runner item `0x14`/quality 2, verifies sound `0x4B`, and checks sampled map index 712 changes `0xA2` to `0xA1` before ordinary Right input crosses the passage. Traversal clears that marker from the active collision buffer |
| Scene 26, collision record 110 (`0x82`) at ROM `0x1D9DBA`, event 2; then `0xD8` records 124/125 at `0x1D9E46`, event `0x18` | Shared maglock continuation followed by typed Security Terminal continuation | The same live passkey changes sampled map index 525 from `0x82` to `0x81`; ordinary input follows the collision grid around the `0x7E` wall into the D8 lane. `0xB7B8` requires two negative samples and selects `0x20EC2`; the descriptor-spawn route opens the Electronics/Walk Away prompt. A separate checkpoint QA remains as focused coverage |
| Any remaining in-range map-table event | Active map's own event table | The selected row is passed to the existing world-activity coordinator; missing/out-of-range tables remain unresolved |

The Stoker prompt is collision-driven after New Game has placed the runner on
scene 2. I found no ROM condition in this traced path that automatically
walks the runner into Stoker's on first map entry. The two adjacent C0 records
are a different doorway and select scene 20; they must not be treated as the
Stoker greeting.

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

The native route policy is `resolve_field_script_route()`. It names the
Stoker dialogue and Ares service directly, then resolves all other valid
events against the current map's event table. The Qt field-exit coordinator
uses that plan to select the dialogue event code, service route, or normal
area-event continuation. It does not interpret ROM addresses at runtime.

The scene-2 C1 route is now exercised from the New Game spawn with normal
controller input. It walks east through the open southern lane, then north
through record 93's doorway cell, records event 1, and follows row 1 into the
scene-26 resource activity. The source geometry explains why the approach
cannot cross record 47's `0x7F` wall directly. The full trace, screenshots,
and remaining visual issue are recorded in the
[scene-2 event-1 audit](scene2-event1-scene26-route.md).

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
