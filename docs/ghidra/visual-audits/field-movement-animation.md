# FIELD-01: Field movement pose and animation audit

## Defect and result

The reported held-D-pad pose looked fallen. The Qt presenter had selected the
`0xC0B2` family for every accepted move. That family is an eight-descriptor
finite behavior transition; its terminal descriptor is held. Ordinary
accepted movement uses a different selector family. The Qt route now uses
`0xC062` for movement and `0xC052` for idle/reset. The held-D-pad fallen pose
is corrected in the current Qt conversion executable.

## ROM trace

The normal field pass at `0x075E` services active entity behavior through
`0xA8EA` and its record continuation. The movement continuation approaches the
controller target heading from `0x19D2` in `0x0800` increments (`0xB530`),
then runs `0xBABC` / `0xBADE` / `0xB77E` through the four-corner terrain probe
at `0xE7C8`. `0xBBE4` commits an accepted position. If the position does not
change, the continuation calls `0xB442` to reset movement state.

After an accepted step, the continuation at `0xB6BA` derives the movement
direction from the committed movement heading (`+0x36`) and writes the
corresponding `0xC062` selector to record `+0x16` at `0xB6D4..0xB6DE`. The
eight selectors are `0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C`; a right
heading (`0x4000`) selects `0x28`. The `0xB442` reset path instead selects a
static directional pose from `0xC052` (`0x00..0x1C`). These paths replace only
the animation selector; they do not clear the frame index or accumulator.

The kind-3 `0x6378` state table resolves C062 selectors to four-descriptor
positive-marker lists. `0x5FDC` increments `+0x1E` when `+0x1A + +0x18` reaches
the period at `+0x1C`; `0x600E` wraps the positive list at its marker and
updates the sprite descriptor consumed by `0x60FA`. For this actor, `0xBABC`
sets `+0xCA` to `0x28`; `0x600E` multiplies it by `0x10`, yielding a `0x280`
increment against `0x1000`. A pose advances on six- or seven-update intervals,
depending on the accumulator phase.

`0xB330` and the immediate branch of `0xAFF2` select C0B2 (`0xE8..0x104`),
whose lists carry marker `0x8008` and auxiliary `0x0200`. Because the marker's
high bit denotes a finite list, `0x600E` sets the selector's high bit and holds
frame 7 at completion. This is not the ordinary held-D-pad cycle.

## Qt implementation and verification

The extractor writes C062 frames as individually named files at
`assets/field/entities/movement-<direction>-<frame>.bmp`, with
`0xB6DE/0xC062/0x600E` provenance. The Qt renderer consumes those files
directly. `actor_ai.lua` owns the player's render-phase frame accumulator,
four-pose cadence, and idle/walk/attack animation selection; it also owns the
default NPCs' C0B2 transition animation cadence. `FieldSceneRuntime` retains
the accepted position and collision state, then applies the movement position
and C052/C062 selector returned by Lua. Player target-heading selection, the
`0xB530` `0x0800` heading slew, collision-route policy, fallback selection,
and movement animation selector now run in Lua. C++ provides terrain samples,
route-table facts, the existing map-event recovery service, the final world
position write, and asset drawing. C0B2 transition assets remain a separate
family.

## Full-screen visual review for the Lua heartbeat slice

The latest Debug and Release QA runs emitted 55 field-movement frames, 43
scene-20 physical-entry frames, and 48 Scene-2 motel-route frames each. Their
corresponding Debug/Release BMPs match byte for byte. I reviewed every frame in
full-screen contact sheets and inspected the motel route's `actual.bmp` at
native size. The movement route keeps the map, runner, portrait, equipment/ammo
slots, and sidebar intact while the camera scrolls; release returns to the
directional idle pose. The scene-20 sequence shows the field, doorway, interior,
maglock dialogue, and return without stale dialogue panels, duplicate controls,
or old button graphics under the new screen.

I also reviewed the independent longplay's 30-fps scene-2 frames 4440-4469
(02:28). They show the same street, HUD scale, and moving runner through the
camera scroll. The video does not log controller input, so it corroborates
placement and sprite appearance but does not prove the exact heading or
per-step cadence of the QA route. The internal captures verify this conversion
path, not Genesis visual fidelity.

## Eight-question visual check

1. **Does it look correct?** The runner remains aligned to the street and
   changes facing with the held direction; the longplay frames corroborate its
   scale and street placement.
2. **Does the look match state?** The movement QA checks the source target and
   current headings, accepted coordinates, walking state, and C052/C062
   selector values; release resets to the corresponding directional idle
   selector.
3. **Are the visuals correct?** The ROM selects C062 for accepted movement and
   C052 for idle/reset. The renderer uses the extracted directional frame files
   for those selectors.
4. **How does it animate?** ROM `0x5FDC` advances on a `0x1000` period from a
   `0x0280` increment. Lua advances the four-pose walk cycle on the corresponding
   six/seven-update cadence; the QA checks all four poses and a loop.
5. **Is there sound?** The traced ordinary movement and selector path from
   `0xB530` through `0xB6DE` contains no movement-specific audio call. Entry
   cues `0x2E`/`0x2F` belong to a separate branch and map to silent slots.
6. **Are neighboring visuals clean?** The complete field and scene-20 frames
   were reviewed, including HUD, portrait, map edges, doorway, interior, and
   dialogue return. No old panel, duplicate control, or stale button remains
   over the next screen.
7. **Are there multiple frames?** Yes: 55 field-movement and 43 scene-20
   checkpoints per Debug/Release build, plus the 30 consecutive longplay frames
   4440-4469. The longplay does not expose input state, so it does not confirm
   the QA route's exact movement timing.
8. **Is old UI cleared?** The field movement and Scene-20 route return use
   complete frames. The field route has no menu/dialogue layer; Scene-20 shows
   the prior dialogue and controls cleared on return, with no duplicate buttons
   beneath the restored field screen.

The `shadowrun_ghidra_conversion_qt_field_movement_qa` route captures idle,
held-right ticks 1, 4, 7, 8, 14, 16, 21, 28, 32, and 35, release, and
`actual.bmp`. It checks all eight C052/C062 selector pairs, the `0x4000`
target/current heading, the source-derived `(600,-200)` displacement at tick
16, the right-facing C062 selector, all four movement poses, at least one loop
wrap over 35 held updates, and return to C052 idle. It also checks the complete
playfield and HUD chrome. Debug and Release pass; all thirteen whole-screen BMPs
have identical SHA-256 hashes between configurations. Every Debug frame was
reviewed at 320x224, including the map, portrait, item/ammo slots, sidebar, and
runner. No menu/dialog overlay or stale control appears.

The same Release QA now drives the other six controller combinations from the
source word table at `0x19D2`: up, down, and four diagonals. Each route starts
from the same scene-2 field state, accepts movement through the live field
update, aligns to its source heading, selects the matching C062 direction,
then returns to C052 on release. `field-movement-headings.tsv` records input,
heading, selector, frame, facing transform, and world-position change. Thirty
additional `320x224` BMPs cover idle, first held update, aligned, held, and
released states for all six directions. All were reviewed as full-screen
captures; map scrolling, actor placement, and the shared HUD remain coherent,
with no dialogue, menu, or stale UI over the field. These six new route captures
were verified in Release; the earlier right/left captures remain byte-matched
between Debug and Release.

`shadowrun_graphics` and `shadowrun_graphics_catalog` pass in Debug and Release.
Their checks verify the eight C062 directional selectors, marker `4`,
auxiliary `0x0180`, four extracted frames per direction, and separate C0B2
transition markers/assets. The Qt movement test now exercises all eight live
headings: right and left have extended cadence routes, while the other six
directions have ordinary-input movement, facing, selector, full-screen, and
release checkpoints.

Blocked movement is checked in the scene-2 route of
`shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa`. After
normal held-Down input reaches the source wall row, the test requires no
movement, frame zero, and a C052 directional selector. The live runtime now
connects this blocked-step branch to typed `reset_entity_movement_state`
(ROM `0xB442`): it clears the blocked bit, restores the desired heading,
selects C052, and re-evaluates collision at the current position. `0xB442`
does not clear the frame index or animation accumulator; the next render pass
resolves the selected one-frame C052 list. The QA then holds diagonal Up/Right
at the wall corner, requires the source reset boundary to fire without
committing either candidate coordinate, releases input, and verifies
target/current heading equality,
directional idle, and frame zero. Debug and Release pass; all 55 route BMPs
match by SHA-256. Every Debug frame was reviewed in whole-screen contact sheets;
the `actual.bmp` was inspected at native size. The wall, bypass, corner reset,
player, and HUD remain visible, and no dialogue, stale
menu, or duplicate control layer appears. This route still rejects the nearby
record-2 object as Stoker's trigger; it does not identify the real Stoker
property.

## Scene-2 motel-street collision route

The separate `shadowrun_ghidra_conversion_qt_field_scene2_motel_street_collision_qa`
starts at the traced New Game position `(0x3CE0, 0x2D00)` and uses ordinary
controller input to reach the motel's west frontage. It then holds Up against
the wall formed by scene-2 ROM collision records 32-38
(`0x172EEA..0x172F26`). The ROM route table at `0xBB86` can choose an open
orthogonal candidate when one side of the probe is clear. Here, Up follows that
fallback for several steps, then settles at the wall while Up remains held;
the final 61 held-input frames have the same position and the C052 idle pose.
Up+Left then moves around the wall edge, and South retreats to open pavement.
The route stays west of scene-2 record 91 (`0x173138`, C2), so this collision
check does not fire the motel doorway event.

The ROM step record keeps a separate `+0x36` animation heading for a diagonal
fallback (`0xBADE`/`0xBB70`). Accepted walking uses that heading to select
C062. The `0xB442` reset instead writes a C052 idle selector from restored
current heading `+0x2E` and leaves `+0x36` untouched. The first Lua movement
port exposed a mismatch at that boundary: the selected C052 list faced south,
but the following render heartbeat reused stale `+0x36` and drew west-facing
assets. Lua now returns the C052/C062 selector directly and keeps idle facing
with the selected C052 list; C++ applies that returned selector. The QA checks
idle facing against `+0x2E`, walking facing against `+0x36`, selector, frame,
and world position on every recorded frame. `motel-street-movement.tsv` logs
input, current/target/animation headings, event, and audio-event count.

Debug and Release each pass this CTest. All 48 whole-screen BMPs match by
SHA-256 and were reviewed, including 19 consecutive held-Up frames and 16
consecutive Up+Left frames; the HUD and field stay present with no stale menu
or dialogue graphics. The route reports zero new audio events; playback
duration is not applicable to this tested path. Independent section-03 longplay frames
28-45 show the same street with the runner at rest, but do not include the
controller sequence or collision state, so they do not verify the wall
behavior.

This is targeted evidence for the location the user identified. It shows the
tested input can be blocked by the motel wall and can leave it through another
heading; it does not reproduce a permanent movement lock. The user's exact
stuck sequence and an independent Genesis collision trace are still missing,
so FIELD-02 remains open.

## Related whole-screen review findings

The same final screenshot pass covered the Stoker collision-to-dialogue,
Ares shop, scene-20 physical-entry, and Security Terminal routes. Their field,
dialogue, and shop frames replace one another cleanly; no stale button or
panel layer remains visible underneath a later screen. The Stoker dialogue
captures still show the portrait asset previously flagged by the user. This
movement slice does not change it, and the portrait defect remains open. In
the Stoker collision frames, a pale cluster under the building overhang stays
fixed relative to the map as the camera moves. The screenshots do not identify
its ROM/map source, so this review does not classify it as a live NPC or add it
to the actor population.

## Eight-question visual review

1. **Does it look correct?** Every Scene-2 motel-route capture shows the runner,
   wall, street, and HUD. The runner stays upright, and the new diagonal
   fallback poses face the selected source direction. The Genesis controller
   sequence for this wall is unavailable, so this is not a visual-fidelity
   claim.
2. **Does the look match behavior?** The per-frame trace checks current and
   target headings, ROM-derived `+0x36` animation heading, direction, C052/C062
   selector, frame, position, event, and audio count. Walking facing follows
   `+0x36`; a blocked reset's idle facing follows the C052 selector from
   `+0x2E`, even when `+0x36` remains stale. Held Up reaches the wall, uses the
   source one-side-clear candidate, then settles; Up+Left and South leave the
   frontage. Scene 2 remains active with event zero throughout.
3. **Are the visuals themselves correct?** Sprite frames are the named
   source-derived assets selected by the Lua movement controller. Whole
   screen review confirms actor, map, and HUD are composited together. Exact
   Genesis pixels and placement for this movement route remain unverified.
4. **Is it animated?** Accepted movement uses the four-frame C062 lists; the
   source `0x280 / 0x1000` accumulator advances at six- or seven-update
   intervals. The full movement QA observes all four poses and a loop wrap.
   Held Up returns to the static C052 selector/frame zero after the wall
   fallback is exhausted. Diagonal fallback updates `+0x36`; Lua selects C062
   from that direction. A blocked reset uses the C052 selector selected from
   `+0x2E`. The separate C0B2 transition remains finite.
5. **Is there a sound?** The route's audio-event count increases by zero over
   169 movement updates. No movement cue or duration is assigned by this
   route; broader footstep/audio dispatch remains unresolved.
6. **Are other screen areas correct?** All 48 images are whole `320x224`
   frames. The map, HUD chrome, portrait, item slots, and ammo remain visible,
   with no additional menu/dialogue layer. HUD palette and ammo-count concerns
   remain separate under HUD-01/HUD-02.
7. **Do multiple frames support this interpretation?** The new route captures
   all 19 frames of the held-Up wall decision and all 16 initial Up+Left
   escape frames, plus approach, long hold, retreat, and idle checkpoints.
   Independent section-03 frames 28-45 show the same street with the runner at
   rest, but have no controller or collision log. They anchor the scene's look,
   not the fallback behavior. Section-04 frames 000900-000921 support ordinary
   moving poses on another map.
8. **Was old UI cleared?** This route does not open a menu or dialog. Each
   checkpoint is a fresh complete field frame; review found no old buttons,
   panels, or interaction graphics left underneath the runner.

## Outcome labels

- **Code changed:** Lua resolves player collision-route policy and returns
  C052/C062 selectors; C++ supplies collision, route, and map-event services,
  then applies the selected position and selector. The `0xB442` idle-facing
  path follows C052 from `+0x2E` while diagonal walking uses `+0x36`; C0B2
  remains a separate transition.
- **Behavior implemented:** accepted D-pad movement shows a looping stride;
  release/rejected movement returns to its directional idle pose.
- **Internally checked:** Debug/Release field-movement, Stoker, Ares, and
  motel-frontage CTests pass; the 48 motel-route BMPs match across Debug and
  Release and were reviewed full-screen.
- **Independently checked:** section 04 frames support ordinary moving field
  poses. No matching controller/selector capture exists for exact correlation.
- **Reported defect status:** ordinary movement, source fallback facing, and
  the tested motel wall route are connected. This route did not reproduce a
  persistent movement lock after changing heading or retreating. Independent
  video frames show the street but not the input sequence; the reported
  collision defect and broader building paths remain open.
