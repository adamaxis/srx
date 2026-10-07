# Scene-26 Event-29 Floor Selector and Map Transition

Date: 2026-10-07

**Status: partial.** The scene-26 event-29 selector and its selected-scene
handoff are source-traced, implemented as typed native state, and covered by
Debug/Release QA with full consecutive frame captures. The destination map
identity is still unnamed. The Qt fade follows the ROM's timing but uses a
black overlay rather than reproducing its per-color CRAM arithmetic, and the
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
| 21, 22, 23, 24, 25 | 21, 22, 23, 24 | Not yet covered |
| 26, 27, 28, 29 | 26, 27, 28 | Scenes 26, 27, and 28 covered; scene 29 open |
| 30, 31, 32, 33, 34 | 30, 31, 32, 33 | Not yet covered |
| 35, 36, 37, 38, 39, 40 | 35, 36, 37, 38, 39 | Not yet covered |
| 41, 42, 43 | 41, 42 | Not yet covered |

These are scene IDs only; this census does not identify the places. Scenes
25, 29, 34, 40, and 43 have the event-29 trigger but are not members of their
listed destination group. On the normal resource-scene entry path,
`0x167AE` clears `FFF0C8` for scene IDs `0x15..0x2B` (its branch at `0x16950`
and clear at `0x16964`), which supports member index 0 for these non-member
entry scenes. The Qt runtime's default matches that source path. Entry through
save restoration and physical-input QA for those scenes remain open. The
physical return-loop QA covers only scenes 26, 27, and 28 within the 26/27/28
group.

Up and Down change the selected floor and show the corresponding active arrow
for eight VBlanks. A starts the 17-frame floor-change animation, with each
animation frame held for eight VBlanks. When the selected floor differs from
the current floor, the ROM path at `0x58DB2` chooses the destination scene from
the group and prepares it through `0x16A8E`. The scene transition runs 17
palette steps toward black, changes scenes, then runs 17 steps toward the
destination palette. Each palette step waits two VBlanks through `0x1342`;
`0x131C` and `0x12F6` provide the fade directions. The native
`ElevatorSelectorRuntime` and `ElevatorSceneTransitionRuntime` preserve these
states and timings. The selector's A/B/Start behavior and the event-29 field
collision are driven through the normal Qt controller path.

This is not the Stoker Motel route. The separately traced Stoker interaction
is scene 2 collision `0xC2`, mode 2/event 2, which opens the generic Stoker
dialogue. This selector is reached from scene 26 event 29. The ROM evidence
examined here gives the selected map IDs, not a named building. A ROM text
string at `0xFE169` mentions an office building, but no traced reference links
that string to this selector or its scene group. Neither Stokers nor an office
building is assigned as the location in this audit.

## Full-screen capture and evidence

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
route and all 28 sheets for the second route were reviewed as whole screens;
transition and selector checkpoints were inspected at full size. These native
captures establish route behavior and clean screen replacement, not
independent pixel fidelity.

Independent visual reference: `video analysis/long_gameplay_video.mp4`,
approximately 8833-8837 seconds. Around 8833-8834 seconds, the selector shows
current floor 2 and selected floor 3; around 8835.7-8837.2 seconds, the clip
shows the fade and a map after the transition. This supports the selector
labels and screen order, but not exact VBlank cadence or pixel-level color
matching. The reference has two active runners while the fresh QA state has
one, so party-slot differences are state differences rather than a claimed
renderer match.

### Nine-question visual review

1. **Does it look correct? — Partial.** The selector's full-screen layout,
   current/selected floor labels, and screen order agree with the independent
   longplay samples. Exact color arithmetic and every animation frame do not
   have independent pixel-matched references.
2. **Does the visible look match behavior? — Yes.** QA enters via normal Up
   input into collision `0xDD`, records event 29, and checks current floor,
   selected floor, active arrow feedback, the A animation, and destination
   scene. The ROM state and draw reads for `FFF0C8`/`FFF0CA` are documented
   above.
3. **Are the visuals themselves correct? — Partial.** The extracted selector
   background, labels, arrows, portrait, and HUD layers are drawn in source
   order; full frames were reviewed. The independent clip supports the
   selector state but does not supply a pixel-for-pixel golden for the entire
   selector or fade. The black-overlay fade is a timed native approximation.
4. **Should anything animate? — Yes.** Arrow feedback lasts eight VBlanks;
   A starts 17 elevator animation frames at eight VBlanks per frame. A changed
   floor starts the two 17-step palette fades, with two VBlanks per step.
   Neutral holds and unchanged-floor A do not start the floor animation.
5. **Does it have sound? — Yes, with a playback-evidence limit.** B requests
   source audio event `0x25`. The associated audio resource test reports
   2.134 seconds including its release tail. This route verifies the event
   request; exact Windows output loopback for this route has not been captured.
   No sound call is traced for Up/Down.
6. **Are the other visible parts correct? — Partial.** Review included the
   whole 320x224 view, selector art, character panel, floor labels, scene map,
   and HUD across every transition. The captures show clean composition and no
   leftover controls; the return-route maps have no matching independent
   Genesis reference. Party state differs from the independent clip as noted
   above.
7. **Do multiple frames support the interpretation? — Yes.** The two QA
   captures cover idle, navigation, feedback retirement, floor animation,
   fade-out, black handoff, fade-in, and stable destinations. The longplay
   supplies separate selector and post-transition examples but no controller
   log. The two return routes have internal multi-frame evidence only.
8. **Does the new UI replace the old UI cleanly? — Yes in the captured routes.**
   The selector paints its full-screen background and layers instead of
   painting over field controls. Each black handoff clears the selector; each
   destination replaces it with its own field composition. QA confirms no
   dialogue, shop, or prior modal remains open.
9. **Was the whole time window reviewed? — Yes.** All 459 consecutive frames
   from the first route and all 553 from the return routes were captured and
   reviewed through whole-screen contact sheets; selector and transition
   checkpoints were also inspected at full size. The independent video
   timestamps are visual anchors only, not ROM timing measurements.

## Verification and remaining work

The defending Debug/Release tests are
`shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa` and
`shadowrun_ghidra_field_elevator_selector_runtime`. The route QA checks source
collision records, event 29, state transitions, event `0x25`, clean screen
replacement, and rendered pixel regions. The physical-entry test checks the
scene-26 outbound transition and the scene-28-to-27-to-26 return loop; it writes
`qt_runtime/field-elevator-physical-entry-qa/scene26-event29-elevator/actual.bmp`
and the `floor-group-roundtrip/actual.bmp` and consecutive `timeline/` captures.
Debug and Release outputs are compared by frame name and SHA-256.

Remaining: exact CRAM color arithmetic for this field transition, route-level
audio loopback and measured output, a reference capture for all selector
animation frames, and a source-backed name for scene 28. Do not use this route
as evidence that Stokers has an interior.
