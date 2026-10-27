# Scene-26 Event-29 Floor Selector and Map Transition

Date: 2026-10-09

**Status: partial.** Lua resolves the source-backed floor list and maps an
explicit floor index to its numeric scene ID. The native selector keeps the
highlighted floor (`FFF0CA`) separate from the lift's current floor (`FFF0C8`).
The first word in each child list is the greatest valid zero-based floor index,
so the imported count is that value plus one. The groups now contain all ROM
destinations: 21-25, 26-29, 30-34, 35-40, and 41-43. This corrects the prior
truncation of each group's final destination and the six-floor group's runtime
limit. Debug and Release asset, selector, script, and route checks pass; exact
CRAM fade arithmetic, saved-floor restoration, route audio loopback, and
scene-matched Genesis map references remain open.

The prior scene-28 and scene-36 follow-up probes started while the previous
world activity was still active. ROM routine `0x1E2E0` starts a 60-field-frame
activity timeout; selector and map-fade updates do not advance the field
service timer. The QA now waits through the remaining timeout with neutral
field frames before it physically approaches the destination trigger. Both
triggers then open through ordinary movement input. This fixes the QA sequence
and the native transition result's reporting of an already-running activity;
it does not establish independent visual fidelity or the immediate-input
experience against a scene-labeled Genesis recording.

Ordinary resource-entry points and saved-session floor restoration remain
open. Route anchors now identify groups 21-25 as Fuchi Industrial Electronics,
30-34 as Mitsuhama Computer Technologies, 35-40 as the Renraku site, and 41-43
as Lone Star Security. Group 26-29 remains unnamed. All 23 event-29 trigger
scenes occur in their respective child lists; scenes 25, 29, 34, 40, and 43
are the final entries at indices 4, 3, 4, 5, and 2. The Qt fade uses a black
overlay rather than the ROM's per-color CRAM arithmetic, and route-specific
audio output has not been captured with loopback.
## Source trace and native behavior

The scene-26 terrain records 102 and 103 carry collision value `0xDD`. The
same paired trigger rectangle occurs in scene 28 records 112/113 and scene 27
records 131/132. The ordinary field collision path publishes event 29 and
enters ROM routine `0x58D94`. That caller saves `FFF0C8`, calls selector
routine `0x58DE0`, then reloads the destination only if `FFF0C8` changed. At
selector entry, `0x58DE0` copies current floor `FFF0C8` to highlighted floor
`FFF0CA`. `0x167AE` loads the child-list header into `FFF0C6`; `0x58DE0` allows
selection through that inclusive maximum, and `0x16A8E` reads the child map at
list offset `2 + 2*FFF0C8`. D-pad input changes only `FFF0CA`. A (`bit 14`) calls `0x58FF8`; its
timed animation advances `FFF0C8` one floor at a time. Start (`bit 15`)
returns from the selector. Therefore D-pad plus Start cannot change floors
without A. The selector's child scene list is imported offline as
`records/field/elevator-scene-groups.bin`; scenes 26-29 share destinations
26-29. Runtime code consumes the typed scene list and named extracted art; it
does not read the ROM or interpret ROM addresses. The native confirmation path
resolves the destination from `current_floor`, even if the player changes the
highlight again after a confirmed lift trip.

The US ROM input order is explicit in `0x58E6C..0x58FA6`. Previous-floor bit 9
is checked before next-floor bit 8, and either direction consumes the poll
before A/B/Start are checked. The direction path sets `D5=8` at `0x58F14` and
calls `0xC24`; that routine waits at `0xBD2`, then executes `DBF D5`, so the
arrow interval contains nine VBlank waits. The selector loop at `0x58E5A`
waits once more before sampling input, making the next poll ten VBlanks after a
directional input. With no direction pressed, A (bit 14) is handled first at
`0x58F74` and branches back to the loop, even when current and selected floors
are equal. B (bit 13) then calls sound routine `0x391A`; Start (bit 15) is
checked after B. Thus A+B+Start handles only A, B+Start requests the sound and
then exits, and direction+A/B/Start handles only the direction for that poll.
The host selector now models the nine-VBlank feedback and this branch order.

### Numeric floor display and action-state audit

The selector has two independent numeric labels. The extracted current-floor
layers use ROM table `0x1B6100` (catalog provenance `0x58FCC`) and index
`FFF0C8`; the selected-floor layers use table `0x1B60E8` (catalog provenance
`0x58FA8`) and index `FFF0CA`. The Qt renderer loads six named layers for each
state and composites each from its own typed index. These labels show the
one-based floor ordinal; the scene ID comes separately from the selected child
list entry at `2 + 2*FFF0C8`.

| Child-scene group | Valid zero-based indices | Numeric labels shown |
|---|---:|---:|
| 21-25 | 0-4 | 1-5 |
| 26-29 | 0-3 | 1-4 |
| 30-34 | 0-4 | 1-5 |
| 35-40 | 0-5 | 1-6 |
| 41-43 | 0-2 | 1-3 |

The scene-40 capture `qt_runtime/field-elevator-scene40-group-roundtrip-qa/selector-destination-39-selected.bmp`
shows current index 5 as `6` while selected index 4 is `5`; child-list index 4
resolves to scene 39. This is an internal Qt capture checked against the ROM
table-derived layers, not an independent Genesis pixel comparison. It also
shows why treating either displayed number as the numeric scene ID would be
wrong.

| User action or state | Source state and display effect | Audit coverage |
|---|---|---|
| Open selector | `FFF0CA` is initialized from `FFF0C8`; both ordinal labels start on the same floor. | ROM trace and normal-input entry QA. |
| Up or Down within the group | Only `FFF0CA` changes; the selected label moves while the current label stays fixed. The active arrow lasts through nine VBlank waits, followed by one VBlank before the next input poll. | ROM `0x58F14`/`0xC24`/`0x58E5A`, selector runtime test, and group-route captures. |
| Direction at either group boundary | The selected index is clamped to the first or inclusive last child; that direction's feedback still runs. | Runtime test covers both six-floor boundaries. |
| A with a different floor selected | A starts the lift animation; `FFF0C8` advances one floor step at a time and the current label follows. Scene loading uses the resulting current index. | ROM `0x58F74`/`0x58FF8`, single-step unit path, and multi-step group-route captures. |
| Start after highlight changes but before A | The selector returns with `FFF0C8` unchanged, so the caller does not load another scene. | Selector runtime test. |
| B | Calls sound routine `0x391A` and leaves the selector open unless Start is also pressed. B+Start requests sound, then exits. | ROM branch order and selector runtime test. Independent Genesis audio playback is still open. |
| Simultaneous directions/buttons | Previous (bit 9) precedes next (bit 8); either direction suppresses other inputs for that poll. With no direction, A precedes B and Start; B precedes Start. | ROM `0x58E6C..0x58FA6` and selector runtime tests cover lower-edge direction combinations, A+B+Start while moving and stationary, and B+Start. |
| Held input | A held direction is sampled again after the nine feedback waits and one outer-loop wait: ten VBlanks between direction polls. Held B is polled on the next VBlank; held A is sampled again after the synchronous lift routine returns. | ROM timing trace and host feedback/animation gates; independent held-input video capture remains open. |

The ordinary doorway entry and an elevator arrival are separate routes. On
floor travel, the native route reloads the selected scene profile and places
the runner at that destination profile's lift landing; source-floor coordinates
are map-local and are not copied into the next floor. The profile coordinates
are close to their event-29 rectangles in groups 21-25, 26-29, 30-34, and
35-40. This does not establish each building's ordinary doorway position.
The `0x3074` conditional override (`FFFC0E`/`FFE10E`) remains a separate
coordinate path and must be traced for each ordinary entry route.

### Source-data trigger census

The offline extraction resolves each scene profile at the ROM table `0x3232`
and the elevator child list through that profile's resource pointer at `+0x58`.
Scanning the extracted terrain records for collision value `0xDD` finds paired
event-29 rectangles in 23 numeric scenes, arranged as follows:

| Trigger scenes | Source child-scene group | Physical route QA |
|---|---|---|
| 21, 22, 23, 24, 25 | 21, 22, 23, 24, 25 | Scene 25 starts at index 4; its route visits the other group maps through event-29 triggers. |
| 26, 27, 28, 29 | 26, 27, 28, 29 | Scene 26 entry and transition to 28 pass; after the active world-activity timeout expires, ordinary input opens scene 28's event-29 selector. Scene 29 is index 3. |
| 30, 31, 32, 33, 34 | 30, 31, 32, 33, 34 | Scene 34 starts at index 4 and chains through scenes 31, 32, 33, and 30. |
| 35, 36, 37, 38, 39, 40 | 35, 36, 37, 38, 39, 40 | Scene 40 starts at index 5; the selector supports six maps. After the activity timeout expires, the scene-36 route visits the remaining group destinations. |
| 41, 42, 43 | 41, 42, 43 | Scene 43 starts at index 2 and chains through scenes 42 and 41. |

The group headers at `0x1D752A`, `0x1D9936`, `0x1DB004`, `0x1DC36E`, and
`0x1DE380` are respectively 4, 3, 4, 5, and 2. `0x167AE` treats each as an
inclusive floor index, while `0x16A8E` reads one more child ID than the header
value. This establishes all final destinations as members, not separate
non-member triggers. The endpoint routes still start from test-only restored
checkpoints; they do not prove ordinary doorway entry or saved-session floor
restoration. A descriptor coordinate is a reproducible harness starting point,
not proof of the route by which the player entered the scene. In the 30-34
group route,
six held Right updates align the runner with the narrow trigger lane in scenes
31-34; held Up then enters the paired event-29 rectangles. Scene 30's
descriptor spawn is already centered in its lane and reaches its trigger with
Up. The extracted collision records explain why a direct north start fails at
the west edge of the lane. After the initial scene-34 QA checkpoint, the
remaining group entries are actual selector scene transitions that place the
runner at the destination profile's lift landing.
Scene 25 starts from its descriptor-spawn checkpoint and chains through
22->23->24->21->22, entering each member-scene event-29 trigger through the
selector and carrying current-floor indices 1, 2, 3, and 0 into the next leg.
The scene-34 route similarly covers member scenes 31, 32, 33, and 30; scene 43
covers 42 and 41. The earlier scene-28 and scene-36 probes ended at `(3280,
1160)` and `(10960, 5264)` because they sent movement before the world-activity
timeout released. The corrected routes record neutral updates until that
source timer reaches zero, then enter each event-29 rectangle with ordinary
input. These captures establish physical trigger behavior after the source
activity gate; they do not establish the numeric maps' real-world names.

### ROM evidence for member triggers 21-25

ROM table entries `0x3286` through `0x3296` resolve to profiles
`0x1D73E4`, `0x1D7A86`, `0x1D81BA`, `0x1D7A86`, and `0x1D8A40`.
All five descriptors share group pointer `0x1D752A`, which has maximum index
4 followed by scene IDs `[21,22,23,24,25]`. Scenes 21-24 use camera
`(0x2800,0x0080)` and runner spawn `(0x3000,0x0600)`; scene 25 is the final
member at floor index 4. Scene 21's paired event-29 records 100/101 are at
`0x1D794E`/`0x1D7958`; scene 22's records
113/114 are at `0x1D8064`/`0x1D806E`; scene 23's records 133/134 are at
`0x1D885C`/`0x1D8866`; scene 24 reuses scene 22's profile and records
113/114. The paired rectangles have X spans `0x2E0..0x2FF` and
`0x300..0x31F`, Y span `0x20..0x3F`, and collision value `0xDD`. Direct ROM
terrain bytes match the extracted named collision records for scenes 21-24.

Up and Down change only the highlighted floor and show the corresponding
active arrow through nine VBlank waits. A moves the lift between current and
highlighted floors; each of 17 animation frames is held for eight VBlanks.
Start returns from the selector. The caller compares `FFF0C8` to the saved
current floor and only then loads the scene for the actual current floor via
`0x16A8E`.
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

### ROM evidence for member triggers 30-34

ROM table entries `0x32AA` through `0x32BA` resolve to profiles
`0x1DAEB8`, `0x1DB49C`, `0x1DBA94`, `0x1DC130`, and `0x1DC1BC`. Their shared
group pointer at `0x1DB004` has maximum index 4 and lists
`[30,31,32,33,34]`; scene 34 is the final destination at index 4. Scene 30's descriptor
spawns at `(0x3A00,0x0600)`; scenes 31-33 spawn at `(0x3800,0x0600)`. All use
camera `(0x2D00,0)`. Scene 30's collision stream at `0x1DB032` contains paired
event-29 records 82/83; scene 31's stream at `0x1DB5F8` contains records 85/86;
scenes 32 and 33 share the stream at `0x1DBC12`, with records 97/98; scene 34
uses profile `0x1DC1BC` and records 85/86 from the `0x1DB5F8` stream. The
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

### ROM evidence for member triggers 35-40

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
`0x20..0x3F`. Scene 40 uses profile `0x1DDD1A` and paired event-29 records
70/71 at `0x1DE112`/`0x1DE11C`. All six profiles share group pointer
`0x1DC36E`, whose maximum index is 5 and whose destinations are
`[35,36,37,38,39,40]`.
Scenes 35-38 spawn at `(0x2800,0x1580)` with camera `(0x1F00,0x0B80)`;
scene 39 spawns at `(0x1600,0x0500)` with camera `(0x0C80,0x0300)`.
Their profile coordinates are close to their paired event-29 elevator
rectangles and define destination lift landings for selector travel. They do
not establish the ordinary entry point for any building; that must come from
the corresponding ROM mode/event transition and its pending coordinate state.

## Full-screen capture and evidence

The scene-25 route checks paired source collision records 102/103 and starts
at descriptor position `(0x3000, 0x0600)`. Ordinary Up enters event 29 with
current and selected floor index 4, exposing all five destinations 21-25.
The route checks B's audio request and nine-VBlank arrow feedback, exercises
floor selection, and follows the selected floor through the 17-frame lift
animation and timed fades. The endpoint is a group member at index 4, not a
separate trigger. The group round trip starts at scene 25, explicitly travels
to scene 22 at index 1, then chains through the other member triggers and
returns to scene 22. Scene 22's camera settles after the fade in three updates
(`camera_y` 96, 64, 32, then 0); the test saves the immediate post-fade frame
separately and starts its stable hold after camera motion stops.

The scene-29 route checks paired source collision records 112/113 and starts
at descriptor position `(0x0A00, 0x0500)`. Its source group is `[26,27,28,29]`;
the selector opens at current and selected floor index 3. QA checks all four
destinations, the full-black handoff, destination profile lift landing and
HUD, and that no modal graphics remain layered over the destination. The
checkpoint is restored at the source profile coordinate, so it verifies the
collision and selector path rather than ordinary building entry.

The scene-34 route checks paired source collision records 85/86, group
`[30,31,32,33,34]`, and descriptor spawn `(0x3800, 0x0600)` with camera
`(0x2D00, 0)`. The selector opens at floor index 4. Six Right updates from the
north-facing spawn align the runner with the narrow trigger lane before Up
enters event 29. The chained route explicitly selects index 1, then visits
scenes 32, 33, 30, and 31 through their own physical triggers. The round trip
records 2,181 frames across 2,272 Debug/Release files, with all paired artifact
hashes matching. The initial Right hold follows native heading-turn behavior;
it does not move the runner through a wall or synthesize a trigger event.

The scene-40 route checks ROM profile table entry `0x32D2` (profile
`0x1DDD1A`), group list `[35,36,37,38,39,40]`, and paired event-29 records
70/71 at `0x1DE112`/`0x1DE11C`. Its descriptor camera is `(0x0C80, 0x0300)`
and profile runner coordinate is `(0x1600, 0x0500)`; its event-29 rectangle is
at X `0x140..0x17F`, Y `0x20..0x3F`, well away from that coordinate in the map.
The profile coordinate is the QA checkpoint, not a verified story-entry
coordinate. The source collision grid leaves a northward lane from the
checkpoint into the `0xDD` rectangles. The selector opens at index 5. The
group round trip explicitly travels to scene 36 at index 1, then enters each
member trigger through ordinary movement input and returns to scene 36. No
independent Genesis capture has been tied to scenes 40 or 36.

The scene-43 route checks profile table entry `0x32DE` (profile `0x1DEE62`),
group `[41,42,43]`, descriptor spawn `(0x0500, 0x2300)`, and paired event-29
records 76/77 at `0x1DECD2`/`0x1DECDC`; their X spans are `0x20..0x3F` and
`0x40..0x5F`, and their Y span is `0x1E0..0x1FF`. Ordinary Up input enters
the selector from that checkpoint at current floor index 2. The chained QA
requests B's sound event `0x25`, selects scene 42 at index 1, checks the
17-frame lift animation and Start transition, then checks
the black handoff, destination profile lift landing, HUD, and idle stability after the
camera's normal lookahead alignment. The three-leg route records 1,060 frames across 1,108 Debug/Release artifacts;
all paired SHA-256 hashes match. The selector, handoff, destination entry, and
settled destination were inspected at full size. No independent Genesis
capture has been tied to scenes 43 or 42, so the map composition remains
visually unconfirmed.

The group round trip continues from the scene-43 handoff instead of resetting
the scene or injecting another event. Scene 42 profile entry `0x32DA` points
to `0x1DE83E`; its group is `[41,42,43]`, spawn is `(0x0500,0x2300)`, and
records 76/77 carry the event-29 trigger. The runtime arrives on scene 42 at
floor index 1. Down selects scene 41, whose profile entry `0x32D6` points to
`0x1DE234`; its group is `[41,42,43]`, spawn is the same, and records 79/80
carry the paired trigger. From scene 41, Up selects scene 42 and returns
through the normal fade and resource-scene handoff. QA checks direction-
specific lift start, selector feedback, destination landing, and clean screen
replacement on all three legs. No independent Genesis image has been tied to
scenes 41, 42, or 43.

The scene-25 group route starts from descriptor spawn `(0x3000,0x0600)` and
enters records 102/103 through ordinary Up input. Its chained legs are
`25->22->23->24->21->22`. The first leg starts at source index 4 and selects
index 1; the next two legs travel upward one floor each, scene 24 descends
three floors, and the final leg returns upward from scene 21 to 22. Each leg
requests B audio event `0x25` once. QA checks event-29 records, destination
index, selector state, direction-specific lift start, visible rail-frame
changes, arrival digit, black handoff, destination entry, and stable
destination. The route records 2,176 frames across 2,270 Debug/Release files;
all paired artifact hashes match. No independent Genesis capture has been tied
to scenes 21-25, so this establishes route behavior and screen replacement,
not independent map-pixel fidelity.

The scene-40 group run starts from descriptor spawn `(0x1600,0x0500)`, enters
event 29 with Up, opens at index 5, and explicitly travels to scene 36 at index
1. After the source activity timeout expires, ordinary Up opens scene 36's
event-29 selector; the chained route covers scenes 36-39 and 35 before
returning to 36. The group route records 2,766 frames across 2,880
Debug/Release files, with all paired artifact hashes matching. The event's
decoded resource 037 is 2.134 seconds including its release tail; route QA
checks the request but does not capture Windows output loopback.

The 17 extracted selector frames visibly move the thin rail marker at x=32;
source timing advances one animation frame every eight VBlanks and begins at
frame 16 upward or frame 0 downward. The scene-40 route checks selector
choices, the lift to scene 36, the fade, destination screen, and physical
member-trigger routes through scenes 36-39 and 35. No independent Genesis
capture has been tied to these numeric maps, so internal route behavior and
screen replacement are verified while map-pixel fidelity remains unknown.

The Debug and Release CLI route QA captures every update through the scene-26
event-29 selector, lift animation, both fades, scene replacement, and the
scene-28 member trigger. The scene-40 route similarly transitions to scene 36,
waits through the remaining activity timeout, and captures its physical
member-trigger chain. The earlier 120-update probes stopped at `(3280,1160)`
and `(10960,5264)` because they did not wait for the source activity gate to
release; those historical outputs are not evidence that the destination
rectangles are unreachable. The corrected timelines include the neutral wait,
ordinary movement input, selector, handoff, and destination frames. The
scene-25, scene-34, and scene-43 routes also exercise their source groups end
to end. These native captures establish internal route behavior and clean
screen replacement, not independent pixel fidelity.

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
   pixel-matched references. The corrected scene-28 and scene-36 routes open
   their selectors and transition to the selected numeric scenes. The scene-36
   field capture has a black band above the map viewport; without an
   independent capture tied to scene 36, I cannot tell whether that is map
   boundary space or a drawing defect.
2. **Does the visible look match behavior? - Partial.** QA
   enters through ordinary D-pad input into collision `0xDD` and checks event
   29, every destination in each tested group, current and selected floor,
   active arrow feedback, A animation, and destination scene. On return from
   scenes 28 and 36, QA observes the active world-activity timeout, waits for
   it to reach zero, then enters the physical trigger with ordinary input.
   Scenes 25/29/34/40/43 start from fresh QA checkpoints, so those starting
   floors do not verify normal-entry reset or save restoration. Scene 25 reaches scenes 22,
   23, 24, 21, and 22 at carried floor indices 1, 2, 3, and 0; scene 24's
   three-floor descent updates the current-floor digit at each stop. Scene 40
   selects scene 36 at carried floor index 1; after the timeout, its member
   route opens scene 36's event-29 selector and visits 37, 38, 39, and 35.
   Scene 43 similarly reaches scene 42 at
   index 1 and scene 41 at index 0. In the 30-34 group, scene 34 starts the
   chain; scenes 31, 32, and 33
   require six Right updates before Up can enter their lane, while scene 30's
   spawn is already centered and uses Up.
3. **Are the visuals themselves correct? - Partial.** The extracted selector
   background, labels, arrows, portrait, and HUD layers are drawn in source
   order, and the full frames show no stale selector or dialog layer. The
   scene-36 capture's upper black band remains unclassified. Independent
   longplay supports a selector state but does not provide pixel-matched
   goldens for the full selector, routes, or fade. These numeric maps have no
   scene-matched independent map reference. The black-overlay fade is a timed
   native approximation.
4. **Should anything animate? - Yes.** Arrow feedback lasts nine VBlank waits; A
   moves the lift for 17 animation frames at eight VBlanks per frame. Start on
   the reached floor begins the two 17-step palette fades, with two VBlanks
   per step. Neutral holds do not start animation. ROM state sets the starting
   animation frame by travel direction: frame 16 upward and frame 0 downward;
   the completed scene-25, scene-34, and scene-43 group-route legs check the
   direction-specific start frame. The scene-25 route also checks every
   visible rail-frame change and the 136-VBlank interval between
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
   show no leftover controls. The scene-36 map's upper black band is still
   unexplained. The scene-25 and scene-34 group routes' full frames show the
   party HUD retained on the selector and field controls cleared. The
   scene-40 route shows the selector replaced by scene 36 after the fade.
7. **Do multiple frames support the interpretation? - Yes internally;
   partial independently.** Consecutive QA captures cover idle, cooldown,
   group destinations, navigation, feedback retirement, direction-specific
   lift animation, fade-out, black handoff, fade-in, and stable destinations.
   The corrected scene-28 and scene-36 routes now enter their triggers. The
   longplay supplies separate selector and post-transition examples but no
   controller log or scene IDs for these maps.
8. **Does the new UI replace the old UI cleanly? - Yes in the captured routes.**
   The selector replaces the field view with its full-screen elevator layout;
   the party HUD remains visible as part of that screen. Each black handoff
   clears the selector, and each destination replaces it with its field
   composition. QA confirms no field controls, dialogue, shop, or prior modal
   remain layered underneath.
9. **Was the whole time window reviewed? - Captured end to end; visual review
   remains partial.** The new Debug and Release runs record the cooldown,
   ordinary trigger approach, selector, animation, both fades, and destination.
   Full-size field, selector, and destination frames were inspected. The
   scene-36 upper black band remains open pending a scene-matched Genesis
   reference. Independent video timestamps are visual anchors, not ROM timing
   measurements.

## Verification and remaining work

The focused Qt Debug and Release runs each pass 12 selector/elevator CTests,
covering selector state and timing, physical event-29 entry, all five source
groups, and the linked Matrix continuations. The source-assets CTest and
gameplay-script QA passed in the preceding focused run; neither code path was
changed during the nine-VBlank correction. The ROM-derived player sprite-origin
check reports 104 rows and zero changes. The two earlier
120-update approach failures were traced to the active source activity timeout;
route QA now waits for it before physical re-entry. These captures verify host
route behavior and cross-build consistency, not independent Genesis pixels.

The Debug and Release physical-entry artifact trees each contain 1,202 files
with matching SHA-256 hashes. The group-route trees contain 2,270 files for
scenes 21-25, 2,272 for 30-34, 2,880 for 35-40, and 1,108 for 41-43; every
paired file matches by SHA-256. Their timelines contain 1,035 frames for
physical entry and continuation, then 2,176, 2,181, 2,766, and 1,060 frames
for the four complete group routes respectively.

Remaining: ordinary resource entry and save-restore floor-state paths; exact
CRAM color arithmetic; route audio loopback; a scene-matched independent map
reference to classify the upper black band in the scene-36 capture; and
source-backed map names. Do not use this route as evidence that Stokers has an
interior.
