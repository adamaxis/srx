# Scene-26 Event-29 Floor Selector and Map Transition

Date: 2026-10-07

**Status: partial.** The scene-26 event-29 selector and its selected-scene
handoff are source-traced, implemented as typed native state, and covered by
Debug/Release QA with full consecutive frame captures. Physical-input routes
cover the non-member trigger scenes 25, 29, 34, and 40. These start from QA restore
checkpoints seeded with each scene's descriptor position and test-only
interaction-mode setup; they validate the corresponding groups 21/22/23/24,
26/27/28, 30/31/32/33, and 35/36/37/38/39. They do not establish ordinary world-event entry
into those scenes or saved-session floor restoration. The destination map
identities are still unnamed. The Qt fade follows the ROM's timing but uses a
black overlay rather than reproducing its per-color CRAM arithmetic, and
route-specific audio output has not been captured with loopback.

## Source trace and native behavior

The scene-26 terrain records 102 and 103 carry collision value `0xDD`. The
same paired trigger rectangle occurs in scene 28 records 112/113 and scene 27
records 131/132. The ordinary field collision path publishes event 29 and
enters ROM routine `0x58D94`, which calls the floor-selector routine at
`0x58DE0`. It reads the current-floor state at `FFF0C8` and the selected-floor
state at `FFF0CA`. The selector's child scene list is imported offline as
`records/field/elevator-scene-groups.bin`; for each of scenes 26, 27, and 28,
the group contains scene IDs 26, 27, and 28. Runtime code consumes the typed
scene list and named extracted art; it does not read the ROM or interpret ROM
addresses.

### Source-data trigger census

The offline extraction resolves each scene profile at the ROM table `0x3232`
and the elevator child list through that profile's resource pointer at `+0x58`.
Scanning the extracted terrain records for collision value `0xDD` finds paired
event-29 rectangles in 23 numeric scenes, arranged as follows:

| Trigger scenes | Source child-scene group | Physical route QA |
|---|---|---|
| 21, 22, 23, 24, 25 | 21, 22, 23, 24 | Scene 25 has a descriptor-spawn physical-input route; member scenes 21-24 remain open |
| 26, 27, 28, 29 | 26, 27, 28 | Member loop covers 26/27/28; scene 29 has a descriptor-spawn physical-input route |
| 30, 31, 32, 33, 34 | 30, 31, 32, 33 | Scene 34 has descriptor-spawn physical-input QA; member scenes 30-33 remain open |
| 35, 36, 37, 38, 39, 40 | 35, 36, 37, 38, 39 | Scene 40 has descriptor-spawn physical-input QA; member scenes 35-39 remain open |
| 41, 42, 43 | 41, 42 | Not yet covered |

These are scene IDs only; this census does not identify the places. Scenes
25, 29, 34, 40, and 43 have the event-29 trigger but are not members of their
listed destination group. On the normal resource-scene entry path, `0x167AE`
clears `FFF0C8` for scene IDs `0x15..0x2B` (its branch at `0x16950` and clear
at `0x16964`), which supports member index 0 for these non-member entry scenes.
The scene-25, scene-29, scene-34, and scene-40 routes start from fresh QA restore
checkpoints seeded with each scene's descriptor camera and runner coordinates,
then set the scene-derived interaction mode through a test-only mode setter.
They validate physical input through each non-member trigger and the host's
index-0 fallback; they do not exercise the ordinary `0x167AE` scene-entry
path or prove saved-game floor restoration. For scene 34, six held Right
updates turn the runner from its north-facing spawn toward the trigger lane;
held Up then enters the paired rectangles at records 85/86. The extracted
collision grid shows the adjacent solid tile that blocks a direct north start.
Scene 43 still lacks physical-input QA, as do member-scene triggers 21-24 and
the member triggers in the other uncovered groups. The physical
return-loop QA covers only scenes 26, 27, and 28 within the 26/27/28 group.

Up and Down change the selected floor and show the corresponding active arrow
for eight VBlanks. A moves the lift between the current and selected floor;
the 17 animation frames are each held for eight VBlanks. Once the lift reaches
the selected floor, Start begins the map transition. The ROM path at `0x58DB2`
chooses the destination scene from the group and prepares it through `0x16A8E`.
The scene transition runs 17 palette steps toward black, changes scenes, then
runs 17 steps toward the destination palette. Each palette step waits two
VBlanks through `0x1342`; `0x131C` and `0x12F6` provide the fade directions.
The native `ElevatorSelectorRuntime` and `ElevatorSceneTransitionRuntime`
preserve these states and timings. The selector's A/B/Start behavior and the
event-29 field collision are driven through the normal Qt controller path.

This is not the Stoker Motel route. The separately traced Stoker interaction
is scene 2 collision `0xC2`, mode 2/event 2, which opens the generic Stoker
dialogue. This selector is reached from scene 26 event 29. The ROM evidence
examined here gives the selected map IDs, not a named building. A ROM text
string at `0xFE169` mentions an office building, but no traced reference links
that string to this selector or its scene group. Neither Stokers nor an office
building is assigned as the location in this audit.

## Full-screen capture and evidence

The scene-25 route checks paired source collision records 102/103 and starts
at descriptor position `(0x3000, 0x0600)`. Ordinary Up input enters event 29
and exposes the four source destinations 21-24 from current floor index 0.
After B requests audio event `0x25`, QA visits all four selections, checks
eight-VBlank arrow feedback, and returns to destination 22. A moves the lift
through 17 animation frames at eight VBlanks each; Start initiates the two
fades and scene change. Its timeline contains 332 consecutive 320x224 frames,
from the descriptor-spawn field through a 30-frame destination hold. Scene
22's camera settles after the fade in three updates (`camera_y` 96, 64, 32,
then 0); the test saves the immediate post-fade frame separately and only
starts its stable hold after camera motion stops.

The scene-29 route checks paired source collision records 112/113 and starts
at descriptor position `(0x0A00, 0x0500)`. Ordinary Up input enters event 29
and exposes destinations 26-28 from current floor index 0. After B requests
event `0x25`, QA visits each destination and returns to 27. A moves the lift
through 17 animation frames at eight VBlanks each; Start initiates the fades
and scene change. Its timeline contains 304 consecutive 320x224 frames,
through a 30-frame destination hold. Both non-member tests check the full-black
handoff, destination descriptor spawn and HUD, and absence of stale modal
graphics. They use restored checkpoints at their source descriptor spawns, so
they establish the collision/UI route rather than map reachability from
another scene.

The scene-34 route checks paired source collision records 85/86, group
`[30,31,32,33]`, and descriptor spawn `(0x3800, 0x0600)` with camera
`(0x2D00, 0)`. Six Right updates from the north-facing spawn align the runner
with the narrow trigger lane; Up enters event 29 and opens at group index 0.
After B requests event `0x25`, QA visits destinations 31, 32, and 33, returns
to 31, plays the 17-step lift animation at eight VBlanks per step, and uses
Start to transition to scene 31. The 330 consecutive 320x224 frames cover the
approach, selector, all floor choices, feedback retirement, animation, both
fades, black handoff, and a 30-frame destination hold. Debug and Release
produce identical hashes for all 349 files in the capture directory. The
initial Right hold follows the native heading-turn behavior; it does not move
the runner through a wall or synthesize a trigger event.

The scene-40 route checks ROM profile table entry `0x32D2` (profile
`0x1DDD1A`), group list `[35,36,37,38,39]`, and paired event-29 records
70/71 at `0x1DE112`/`0x1DE11C`. Its descriptor camera is `(0x0C80, 0x0300)`
and runner spawn is `(0x1600, 0x0500)`. The source collision grid leaves a
northward lane from the spawn into the `0xDD` rectangles. Ordinary Up opens at
index 0; QA visits destinations 36 through 39, returns to 36, plays the
17-step lift animation at eight VBlanks per step, and transitions to scene 36.
The 340 consecutive 320x224 frames cover the field approach, selector,
feedback, all floor choices, animation, both fades, black handoff, and a
30-frame destination hold. Debug and Release produce identical hashes for all
362 files. The full timeline was reviewed in nine chronological whole-screen
contact sheets, with the selector and transition checkpoints inspected at full
size. No independent Genesis capture has been tied to scenes 40 or 36.

The Debug and Release CLI route QA captures every update from before the
scene-26 collision, through selector input and animation, both fades, scene
replacement, and a 30-frame stable scene-28 hold. This first route includes 459
consecutive 320x224 frames in `timeline/`, with state, input, palette step,
alpha, and scene logged in `timeline.tsv`. A second physical-input QA starts
from scene 28, enters its paired event-29 trigger, descends to scene 27, then
repeats through scene 27's trigger to scene 26. Its
`floor-group-roundtrip/timeline/` capture contains 553 consecutive frames,
including both arrow-feedback windows, floor animations, fades, black
handoffs, scene replacements, and stable holds. It also records each scene's
collision rectangles and verifies the destination descriptor spawn. `actual.bmp`
is the final scene-26 field frame for this second route. Named BMP checkpoints
cover both selector states, feedback, animation, fade boundaries, black
handoffs, scene entries, and held destinations. All 23 sheets for the first
route and all 28 sheets for the second route were reviewed as whole screens.
The 332-frame scene-25, 304-frame scene-29, 330-frame scene-34, and 340-frame
scene-40 routes were reviewed in six, five, nine, and nine chronological sheets; transition and selector
checkpoints were inspected at full size. The nested scene-2 event-1 route uses a QA-only source
PRNG seed so its selected resource variant matches across build configurations;
the live runtime still seeds itself. These native captures establish route
behavior and clean screen replacement, not independent pixel fidelity.

Independent visual reference: `video analysis/long_gameplay_video.mp4`,
approximately 8833-8837 seconds. Around 8833-8834 seconds, the selector shows
current floor 2 and selected floor 3; around 8835.7-8837.2 seconds, the clip
shows the fade and a map after the transition. This supports the selector
labels and screen order, but not exact VBlank cadence or pixel-level color
matching. The reference has two active runners while the fresh QA state has
one, so party-slot differences are state differences rather than a claimed
renderer match.

### Nine-question visual review

1. **Does it look correct? - Partial.** The selector layout, current/selected
   floor labels, and screen order agree with the independent longplay samples.
   Exact color arithmetic and every animation frame do not have independent
   pixel-matched references.
2. **Does the visible look match behavior? - Yes for the tested routes.** QA
   enters through ordinary D-pad input into collision `0xDD` and checks event
   29, every destination in each tested group, current and selected floor,
   active arrow feedback, A animation, and destination scene. Scenes 25/29/40
   use Up; scene 34 holds Right for six updates, then Up. Each non-member route
   begins from a fresh QA checkpoint, so its index-0 starting state
   does not verify the ROM's normal-entry reset or save restoration. Scene 34
   requires the native heading turn from held Right before Up reaches its
   trigger lane. Scene 40's index-0 trigger is directly north of its spawn.
3. **Are the visuals themselves correct? - Partial.** The extracted selector
   background, labels, arrows, portrait, and HUD layers are drawn in source
   order, and whole frames were reviewed. The independent clip supports the
   selector state but does not provide a pixel-for-pixel golden for the full
   selector, the scene-25/29/34/40 routes, or the fade. The black-overlay fade is a
   timed native approximation.
4. **Should anything animate? - Yes.** Arrow feedback lasts eight VBlanks; A
   moves the lift for 17 animation frames at eight VBlanks per frame. Start on
   the reached floor begins the two 17-step palette fades, with two VBlanks
   per step. Neutral holds do not start animation. Scene 22's post-fade camera
   alignment is three additional updates and is checked as camera motion, not
   idle animation.
5. **Does it have sound? - Yes, with a playback-evidence limit.** B requests
   source audio event `0x25`. The associated audio resource test reports
   2.134 seconds including its release tail. These routes verify the event
   request; exact Windows output loopback for these routes has not been
   captured. No sound call is traced for Up/Down.
6. **Are the other visible parts correct? - Partial.** Review covered the
   whole 320x224 view, selector art, character panel, floor labels, scene map,
   and HUD through selection, transitions, and destination holds. Captures
   show clean composition and no leftover controls. The new routes do not
   have matching independent Genesis references; the reference clip has two
   active runners while fresh QA states have one.
7. **Do multiple frames support the interpretation? - Yes.** Consecutive QA
   captures cover idle, all tested group destinations, navigation, feedback
   retirement, floor animation, fade-out, black handoff, fade-in, and stable
   destinations. The longplay supplies separate selector and post-transition
   examples but no controller log. Scene 25/29/34/40 and the return routes therefore
   have internal multi-frame evidence only.
8. **Does the new UI replace the old UI cleanly? - Yes in the captured routes.**
   The selector paints a full-screen background and layers instead of painting
   over field controls. Each black handoff clears the selector; each
   destination replaces it with its field composition. QA confirms no
   dialogue, shop, or prior modal remains open.
9. **Was the whole time window reviewed? - Yes.** The 459-frame first route,
   553-frame return route, and the 332-, 304-, 330-, and 340-frame non-member routes were
   captured consecutively and reviewed in chronologically indexed
   whole-screen contact sheets. Selector and transition checkpoints were also
   inspected at full size. Independent video timestamps are visual anchors,
   not ROM timing measurements.

## Verification and remaining work

The defending Debug/Release tests are
`shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa`, and
`shadowrun_ghidra_field_elevator_selector_runtime`. The route QA checks source
collision records, event 29, state transitions, event `0x25`, clean screen
replacement, and rendered pixel regions. The physical-entry tests check the
scene-26 outbound transition, the scene-28-to-27-to-26 return loop, and the
scene-25, scene-29, scene-34, and scene-40 descriptor-spawn checkpoint routes; the
original route writes
`qt_runtime/field-elevator-physical-entry-qa/scene26-event29-elevator/actual.bmp`
plus the `floor-group-roundtrip/actual.bmp` and consecutive `timeline/` captures.
The non-member route dumps are under
`qt_runtime/field-elevator-scene25-entry-qa/` and
`qt_runtime/field-elevator-scene29-entry-qa/`, with 332 and 304 timeline
frames, respectively, and `qt_runtime/field-elevator-scene34-entry-qa/` has
330 timeline frames; `qt_runtime/field-elevator-scene40-entry-qa/` has 340.
Debug and Release outputs are compared by frame name and
SHA-256. Scene-25, scene-29, scene-34, and scene-40 output directories each
match exactly across builds. The broader
physical-entry directory also matches after excluding its
manually copied `longplay-*` reference frames, which are present only in the
Debug output.

Remaining: physical QA for member scenes 21-24, 30-33, 35-39, and 41-42, plus
non-member trigger scene 43; ordinary resource-entry and
save-restore floor-state paths; exact CRAM color arithmetic for this field
transition; route-level audio loopback and measured output; independent
references for scene 25/29 and all selector animation frames; and source-backed
map names. Do not use this route as evidence that Stokers has an interior.
