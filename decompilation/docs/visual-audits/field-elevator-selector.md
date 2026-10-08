# Scene-26 Event-29 Floor Selector and Map Transition

Date: 2026-10-07

**Status: partial.** The scene-26 event-29 selector and its selected-scene
handoff are source-traced, implemented as typed native state, and covered by
Debug/Release QA with full consecutive frame captures. Physical-input routes
cover non-member trigger scenes 25, 29, 34, 40, and 43. The scene-25 route
continues through scenes 22, 23, 24, and 21, physically entering every member
trigger in group 21/22/23/24. The scene-34 route continues through
31->32->33->30->31, physically entering every member trigger in group
30/31/32/33. The scene-40 route continues through scenes 36, 37, 38, 39, and
35, physically entering every member trigger in group 35/36/37/38/39. The
scene-43 route similarly enters both member triggers in group 41/42. These
routes do not establish ordinary world-event entry into their starting scenes
or saved-session floor restoration. The destination map identities are still
unnamed. The Qt fade follows the ROM's timing but uses a black overlay rather
than reproducing its per-color CRAM arithmetic, and route-specific audio
output has not been captured with loopback.
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
| 21, 22, 23, 24, 25 | 21, 22, 23, 24 | Scene 25 chains through 22, 23, 24, 21, and 22, physically testing all four member triggers |
| 26, 27, 28, 29 | 26, 27, 28 | Member loop covers 26/27/28; scene 29 has a descriptor-spawn physical-input route |
| 30, 31, 32, 33, 34 | 30, 31, 32, 33 | Scene 34 chains through 31, 32, 33, 30, and 31, physically testing every member trigger |
| 35, 36, 37, 38, 39, 40 | 35, 36, 37, 38, 39 | Scene 40 chains through scenes 36, 37, 38, 39, 35, and 36, physically testing all five member triggers |
| 41, 42, 43 | 41, 42 | Scene 43 starts a physical-input group round trip through member scenes 42 and 41 |

These are scene IDs only; this census does not identify the places. Scenes
25, 29, 34, 40, and 43 have the event-29 trigger but are not members of their
listed destination group. On the normal resource-scene entry path, `0x167AE`
clears `FFF0C8` for scene IDs `0x15..0x2B` (its branch at `0x16950` and clear
at `0x16964`), which supports member index 0 for these non-member entry scenes.
The scene-25, scene-29, scene-34, scene-40, and scene-43 routes start from fresh
QA restore checkpoints seeded with each scene's descriptor camera and runner
coordinates, then set the scene-derived interaction mode through a test-only
mode setter. They validate physical input through each non-member trigger and
the host's index-0 fallback; they do not exercise the ordinary `0x167AE`
scene-entry path or prove saved-game floor restoration. In the 34-group route,
six held Right updates align the runner with the narrow trigger lane in scenes
31-34; held Up then enters the paired event-29 rectangles. Scene 30's
descriptor spawn is already centered in its lane and reaches its trigger with
Up. The extracted collision records explain why a direct north start fails at
the west edge of the lane. After the initial scene-34 QA checkpoint, the
remaining group entries are actual selector scene transitions that load each
destination descriptor spawn.
Scene 25 starts from its descriptor-spawn checkpoint and chains through
22->23->24->21->22, entering each member-scene event-29 trigger through the
selector and carrying current-floor indices 1, 2, 3, and 0 into the next leg.
Scene 40 starts from its descriptor-spawn checkpoint and chains through
36->37->38->39->35->36, entering each member-scene event-29 trigger through
the selector and carrying current-floor indices 1, 2, 3, 4, and 0 into the
next leg. Scene 43 similarly transitions through member scenes 42 and 41,
then returns to 42. These routes establish member-trigger behavior and
carried-floor state for groups 21-24, 30-33, 35-39, and 41/42. The physical
return-loop QA also covers scenes 26, 27, and 28 within the 26/27/28 group.

### ROM evidence for member triggers 21-24

ROM table entries at `0x3286`, `0x328A`, `0x328E`, and `0x3292` resolve to
profiles `0x1D73E4`, `0x1D7A86`, `0x1D81BA`, and `0x1D7A86`, respectively.
All four descriptors use camera `(0x2800,0x0080)`, runner spawn
`(0x3000,0x0600)`, and destination group `[21,22,23,24]`. Scene 21's paired
event-29 records 100/101 are at `0x1D794E`/`0x1D7958`; scene 22's records
113/114 are at `0x1D8064`/`0x1D806E`; scene 23's records 133/134 are at
`0x1D885C`/`0x1D8866`; scene 24 reuses scene 22's profile and records
113/114. The paired rectangles have X spans `0x2E0..0x2FF` and
`0x300..0x31F`, Y span `0x20..0x3F`, and collision value `0xDD`. Direct ROM
terrain bytes match the extracted named collision records for all four
scenes.

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

### ROM evidence for member triggers 30-33

ROM table entries `0x32AA`, `0x32AE`, `0x32B2`, and `0x32B6` resolve to
profiles `0x1DAEB8`, `0x1DB49C`, `0x1DBA94`, and `0x1DC130`. Their shared
group pointer at `0x1DB004` lists `[30,31,32,33]`. Scene 30's descriptor
spawns at `(0x3A00,0x0600)`; scenes 31-33 spawn at `(0x3800,0x0600)`. All use
camera `(0x2D00,0)`. Scene 30's collision stream at `0x1DB032` contains paired
event-29 records 82/83; scene 31's stream at `0x1DB5F8` contains records 85/86;
scenes 32 and 33 share the stream at `0x1DBC12`, with records 97/98. The
rectangles span X `0x380..0x39F` and `0x3A0..0x3BF`, Y `0x20..0x3F`, and carry
`0xDD`. Direct ROM profile, group, and collision bytes match the extracted
named assets.

For scenes 31-34, the descriptor spawn is at the west edge of the 64-pixel
trigger lane. The terrain records place solid `0x7F` geometry immediately west
and east of the lane; the northbound probe at the descriptor X is blocked.
Six ordinary Right updates put the runner's probes inside the open lane before
Up reaches event 29. Scene 30 spawns at X `0x3A0`, already centered in the lane,
and its normal Up approach reaches the same paired trigger. The Debug/Release
route `34->31->32->33->30->31` tests all four member triggers through actual
scene handoffs after starting from the scene-34 descriptor checkpoint.

### ROM evidence for member triggers 35-39

Direct ROM profile and terrain-record reads were compared byte-for-byte with
the extracted named collision records. Scene 35 uses table entry `0x32BE`,
profile `0x1DC248`, and trigger records 117/118 at `0x1DC886`/`0x1DC890`.
Scene 36 uses entry `0x32C2`, profile `0x1DC9A8`, and records 109/110 at
`0x1DCF98`/`0x1DCFA2`; scene 37's entry `0x32C6` points to that same profile
and records. Scene 38 uses entry `0x32CA`, profile `0x1DD100`, and records
112/113 at `0x1DD6D6`/`0x1DD6E0`. Scene 39 uses entry `0x32CE`, profile
`0x1DD866`, and records 58/59 at `0x1DDC20`/`0x1DDC2A`. Profiles 35-38 use
the paired `0xDD` rectangles at X `0x260..0x27F` and `0x280..0x29F`, Y
`0x120..0x13F`; scene 39 uses X `0x140..0x15F` and `0x160..0x17F`, Y
`0x20..0x3F`. The five profiles share destination group `[35,36,37,38,39]`.
Scenes 35-38 spawn at `(0x2800,0x1580)` with camera `(0x1F00,0x0B80)`;
scene 39 spawns at `(0x1600,0x0500)` with camera `(0x0C80,0x0300)`.

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

The scene-43 route checks profile table entry `0x32DE` (profile `0x1DEE62`),
group `[41,42]`, descriptor spawn `(0x0500, 0x2300)`, and paired event-29
records 76/77 at `0x1DECD2`/`0x1DECDC`; their X spans are `0x20..0x3F` and
`0x40..0x5F`, and their Y span is `0x1E0..0x1FF`. Ordinary Up input enters
the selector from that checkpoint. QA requests B's sound event `0x25`, selects
scene 42, checks the 17-frame lift animation and Start transition, then checks
the black handoff, scene-42 descriptor spawn, HUD, and idle stability after the
camera's normal lookahead alignment. Its complete capture has 335 consecutive
320x224 frames. All 349 Debug and Release artifacts match by SHA-256. The
timeline was reviewed in nine chronological whole-screen sheets, and the
selector, black handoff, destination entry, and settled destination were
inspected at full size. No independent Genesis capture has been tied to scenes
43 or 42, so the map composition remains visually unconfirmed.

The group round trip continues from that native scene-43 handoff instead of
resetting the scene or injecting another event. Scene 42 profile entry `0x32DA`
points to `0x1DE83E`; its group is `[41,42]`, spawn is `(0x0500,0x2300)`, and
records 76/77 carry the event-29 trigger with Y `0x1E0..0x1FF`. The runtime
arrives on scene 42 with current-floor index 1. Down selects scene 41, whose
profile entry `0x32D6` points to `0x1DE234`; its group is `[41,42]`, spawn is
the same, and records 79/80 carry the paired event-29 trigger with Y
`0x1C0..0x1DF`. From scene 41, Up selects scene 42 and the route returns
through the normal fade and resource-scene handoff. Source selector setup starts
the lift at animation frame 16 for upward travel and frame 0 for downward
travel; QA checks the direction-specific initial frame on each leg. The three
legs contain 335, 335, and 318 consecutive frames (988 total). Twenty-five
chronological whole-screen contact sheets and full-size selector, animation,
handoff, entry, and destination checkpoints were reviewed. All 1,030 files
match between Debug and Release by relative path and SHA-256. No independent
Genesis image has been tied to scenes 41, 42, or 43.

The scene-25 group route starts from descriptor spawn `(0x3000,0x0600)` and
enters records 102/103 through ordinary Up input. Its chained legs are
`25->22->23->24->21->22`, with timeline lengths 332, 309, 309, 599, and 309
frames (1,858 total). The first leg visits all four destinations and ends on
scene 22 at carried floor index 1. The next two legs travel upward one floor
each; the scene-24 leg descends three floors and checks the current-floor digit
at each 136-VBlank stop (displayed 4->3->2->1); the final leg returns upward
from scene 21 to 22. Each leg requests B audio event `0x25` once. The route
checks the event-29 records, selected group scene, selector state, direction-
specific lift start, visible rail-frame changes, arrival digit, black handoff,
destination entry, and stable destination. All 1,947 Debug and Release
artifacts match by relative path and SHA-256. The 1,858 consecutive frames
were reviewed in 48 chronological full-screen contact sheets, and selector,
floor-stop, black-handoff, destination-entry, and settled frames were inspected
at full size. No independent Genesis capture has been tied to scenes 21-25, so
the evidence establishes source-driven behavior and clean screen replacement,
not independent map-pixel fidelity.

The scene-40 group route starts from descriptor spawn `(0x1600,0x0500)` and
enters the paired event-29 rectangles through ordinary Up input. Its six
selector legs are `40->36->37->38->39->35->36`, with timeline lengths 340,
321, 297, 332, 721, and 321 frames (2,332 total). The first leg selects all
five floors; subsequent legs physically enter each member trigger and carry
the floor index from the previous handoff. The 39-to-35 ride descends four
floors: each 17-frame sequence is held for eight VBlanks, the current-floor
digit updates at each 136-VBlank stop, and the selected-floor digit stays put.
Each leg requests B audio event `0x25` once. The event's decoded resource 037
is 2.134 seconds including its release tail; these route tests check the event
request but do not capture Windows output loopback.

The 17 extracted selector frames visibly move the thin rail marker at x=32;
the source timing advances one animation frame every eight VBlanks and begins
at frame 16 upward or frame 0 downward. Route QA checks the rendered rail
pixels on each frame change and the current-floor digit at every stop, in
addition to the typed state and exact travel duration. Sixty-three chronological
whole-screen sheets covered all 2,332 frames. Full-size selector, source-map,
black-handoff, destination-entry, and settled-map checkpoints were inspected
across the route. All 2,443 Debug and Release artifacts match by relative path
and SHA-256. No independent Genesis captures have been tied to scenes 35-40,
so this route establishes source-driven behavior and clean screen replacement,
not independent map-pixel fidelity.

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
   pixel-matched references. The 21-24, 30-33, 35-39, and 41/42 member routes show
   consistent group labels and expected scene replacement, but those numeric
   maps lack independent visual references.
2. **Does the visible look match behavior? - Yes for the tested routes.** QA
   enters through ordinary D-pad input into collision `0xDD` and checks event
   29, every destination in each tested group, current and selected floor,
   active arrow feedback, A animation, and destination scene. Scenes 25/29/34/
   40/43 start from fresh QA checkpoints, so those starting floors do not
   verify normal-entry reset or save restoration. Scene 25 reaches scenes 22,
   23, 24, 21, and 22 at carried floor indices 1, 2, 3, and 0; scene 24's
   three-floor descent updates the current-floor digit at each stop. Scene 40
   then reaches scenes 36, 37, 38, 39, 35, and 36 at carried floor indices
   1, 2, 3, 4, 0, and 1;
   each next member trigger is reached with Up from its destination descriptor
   spawn. Scene 43 similarly reaches scene 42 at index 1 and scene 41 at index
   0. In the 30-33 group, scene 34 starts the chain; scenes 31, 32, and 33
   require six Right updates before Up can enter their lane, while scene 30's
   spawn is already centered and uses Up.
3. **Are the visuals themselves correct? - Partial.** The extracted selector
   background, labels, arrows, portrait, and HUD layers are drawn in source
   order, and whole frames were reviewed. Independent longplay supports a
   selector state but does not provide pixel-matched goldens for the full
   selector, routes, or fade. Scenes 21-25, 30-34, and 35-43 have no
   independent map references.
   The black-overlay fade is a timed native approximation.
4. **Should anything animate? - Yes.** Arrow feedback lasts eight VBlanks; A
   moves the lift for 17 animation frames at eight VBlanks per frame. Start on
   the reached floor begins the two 17-step palette fades, with two VBlanks
   per step. Neutral holds do not start animation. ROM state sets the starting
   animation frame by travel direction: frame 16 upward and frame 0 downward;
   all scene-25, scene-34, scene-40, and scene-43 group-route legs check the
   direction-specific start frame. The scene-25 and scene-40 routes also
   check every visible rail-frame change and the 136-VBlank interval between
   floors. Scene 24's downward trip checks the displayed floors 3, 2, and 1 at
   the source-timed stops. Scene 33's descent checks the intermediate floor
   stops through scene 30. Destination camera follow
   continues after the fade until its source-derived lookahead target is
   reached; scene 22 settles in three updates, while scene 42's longer movement
   is captured and checked rather than mistaken for idle animation.
5. **Does it have sound? - Yes, with a playback-evidence limit.** B requests
   source audio event `0x25` once per route leg. The associated audio resource
   test maps it to resource 037 and reports 2.134 seconds including its
   release tail. These routes verify the event request; exact Windows output
   loopback for these routes has not been
   captured. No sound call is traced for Up/Down.
6. **Are the other visible parts correct? - Partial.** Review covered the
   whole 320x224 view, selector art, character panel, floor labels, scene map,
   and HUD through selection, transitions, and destination holds. Captures
   show clean composition and no leftover controls. The scene-25 and scene-40
   group routes' full frames show the party HUD retained on the selector and
   field controls cleared. The 30-34 route shows the same result. Each member
   leg repeats the full replacement: selector, full-black
   handoff, new map entry, then a stable destination with no stale controls.
   The 30-34 route has the same clean replacement. Independent map references
   are unavailable for these numeric scenes.
7. **Do multiple frames support the interpretation? - Yes.** Consecutive QA
   captures cover idle, group destinations, navigation, feedback retirement,
   direction-specific lift animation, fade-out, black handoff, fade-in, and
   stable destinations. The 1,858-frame 25/22/23/24/21 route was reviewed in
   48 chronological full-screen sheets, with selector, floor-stop, and
   transition checkpoints inspected at full size. The 30-34 route adds 1,856
   frames across 48 chronological full-screen sheets. The 2,332-frame
   40/36/37/38/39/35 route was reviewed in 63 sheets, and the 988-frame
   43/42/41 route in 25 sheets. Longplay supplies separate selector and post-
   transition examples but no controller log. Member routes therefore have
   internal multi-frame evidence only.
8. **Does the new UI replace the old UI cleanly? - Yes in the captured routes.**
   The selector replaces the field view with its full-screen elevator layout;
   the party HUD remains visible as part of that screen. Each black handoff
   clears the selector, and each destination replaces it with its field
   composition. QA confirms no field controls, dialogue, shop, or prior modal
   remain layered underneath.
9. **Was the whole time window reviewed? - Yes.** The 459-frame first route,
   553-frame return route, and non-member routes from scenes 25, 29, 34, 40,
   and 43 were captured consecutively and reviewed in chronologically indexed
   whole-screen sheets. The scene-25 group route adds 1,858 frames across 48
   sheets, the scene-34 group route adds 1,856 frames across 48, the scene-40
   route adds 2,332 frames across 63, and the scene-43 route adds 988 frames
   across 25. Selector, floor stops, black handoff, destination entry, and
   settled screens were also inspected at full size. Independent video
   timestamps are visual anchors, not ROM timing measurements.

## Verification and remaining work

The defending Debug/Release tests are
`shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene25_group_roundtrip_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene34_group_roundtrip_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene40_group_roundtrip_qa`,
`shadowrun_ghidra_conversion_qt_field_elevator_scene43_group_roundtrip_qa`, and
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
330 timeline frames; `qt_runtime/field-elevator-scene40-entry-qa/` has 340 frames;
`qt_runtime/field-elevator-scene25-group-roundtrip-qa/` has five chained
timelines of 332, 309, 309, 599, and 309 frames for scenes 25, 22, 23, 24,
and 21; `qt_runtime/field-elevator-scene40-group-roundtrip-qa/` has six chained
timelines of 340, 321, 297, 332, 721, and 321 frames; and
`qt_runtime/field-elevator-scene43-group-roundtrip-qa/` has three chained
timelines of 335, 335, and 318 frames for scenes 43, 42, and 41 respectively;
`qt_runtime/field-elevator-scene34-group-roundtrip-qa/` has five chained
timelines of 330, 309, 309, 599, and 309 frames for scenes 34, 31, 32, 33,
and 30 respectively. Debug and Release outputs are compared by frame name and
SHA-256. Scene-25,
scene-29, scene-34, scene-40, all 1,947 scene-25 group round-trip artifacts,
all 2,443 scene-40 group round-trip artifacts, and all 1,030 scene-43 group
round-trip artifacts match exactly across builds. All 1,945 scene-34 group
round-trip artifacts also match exactly. The broader
physical-entry directory also matches after excluding its
manually copied `longplay-*` reference frames, which are present only in the
Debug output.

Remaining: ordinary resource-entry and save-restore floor-state paths; exact
CRAM color arithmetic for this field transition; route-level audio loopback and
measured output; independent map references for scenes 21-25 and 29-40 and
independent selector animation frames; and source-backed map names. Do not use
this route as evidence that Stokers
has an interior.
