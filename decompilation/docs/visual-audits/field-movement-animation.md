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
`0xB6DE/0xC062/0x600E` provenance. The Qt runtime consumes those files directly
and keeps selector, frame, accumulator, period, direction, position, and
walking/idle state typed in `FieldSceneRuntime`. C0B2 transition assets remain
in their separate legacy family.

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

`shadowrun_graphics` and `shadowrun_graphics_catalog` pass in Debug and Release.
Their checks verify the eight C062 directional selectors, marker `4`,
auxiliary `0x0180`, four extracted frames per direction, and separate C0B2
transition markers/assets. The Qt movement test checks the live right-held
path; the other seven headings are covered by selector and extracted-frame
checks, not by seven distinct live movement routes.

Blocked movement is checked in the existing scene-2 wall route of
`shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa`. After
normal held-Down input reaches the source wall row, the test requires no
movement, frame zero, and a C052 directional selector. Debug and Release both
pass, and their 22 route frames match by SHA-256. All 22 Debug frames were
reviewed at full-screen size; the wall and bypass are visible, the actor/HUD
remain present, and no dialogue or stale menu appears. This route still rejects
the nearby record-2 object as Stoker's trigger; it does not identify the real
Stoker property.

## Eight-question visual review

1. **Does it look correct?** The corrected Qt frames show the runner upright
   through held movement. The old fallen final pose no longer appears.
2. **Does the look match behavior?** The held-right route selects C062
   selector `0x28`; release returns to the right-facing C052 selector. Position,
   heading, selector, and frame are checked by the movement QA. All eight
   selector mappings are checked; only right-facing movement is exercised as
   a live field route in this QA.
3. **Are the visuals themselves correct?** The sprite frames are decoded from
   the source descriptors selected by the ROM movement continuation. Whole
   screen review confirms actor, map, and HUD are composited together. Exact
   actor placement against an identical game scene remains unverified.
4. **Is it animated?** Yes. The positive four-pose list loops. The source
   `0x280 / 0x1000` accumulator advances at six- or seven-update intervals,
   depending on starting phase. The 35-update QA observes all four frames and
   a wrap. Accepted movement uses C062; released input selects static C052
   idle. The separate C0B2 transition is finite. At the scene-2 wall, held
   movement is rejected and the C052 selector/frame-zero state is checked.
5. **Is there a sound?** No direct sound request appears in the traced
   heading/vector/collision path. The wider footstep/audio dispatch is not
   resolved; cue and duration remain unknown.
6. **Are other screen areas correct?** All saved images are whole 320x224
   frames. The map, HUD chrome, portrait, item slots, and ammo remain present.
   The portrait-face palette concern remains open under HUD-01, and the visible
   ammo/weapon counts are not evaluated here (HUD-02). No menu or dialogue is
   opened.
7. **Do multiple frames support this interpretation?** Thirteen Qt checkpoints
   show idle, repeated movement, loop wrap, and release. Independent section 04
   frames 000900, 000903, 000906, 000909, 000912, 000915, 000918, and 000921
   show the original actor in multiple field positions/poses. The video has no
   synchronized controller or selector log and does not show this same map, so
   it supports the normal movement-pose family, not exact timing or placement.
8. **Was old UI cleared?** This movement route does not replace a UI page.
   Each checkpoint is drawn as a fresh full frame; review found no old button,
   panel, or sprite underneath the active pose.

## Outcome labels

- **Code changed:** Qt uses source-selected C062 assets/timing for ordinary
  movement and C052 on idle/reset; C0B2 is retained as a separate transition.
- **Behavior implemented:** accepted D-pad movement shows a looping stride;
  release/rejected movement returns to its directional idle pose.
- **Internally checked:** Debug and Release field-movement QA and graphics
  extraction CTests pass; the full-screen Qt outputs match by SHA-256.
- **Independently checked:** section 04 frames support ordinary moving field
  poses. No matching controller/selector capture exists for exact correlation.
- **Reported defect status:** the C062 correction is implemented; right-held
  movement loops through its four poses, and a normal-input wall collision
  returns to C052 idle. Independent section 04 frames support a moving runner,
  but they do not match this map or expose selector/input state. Other live
  headings and exact visual alignment remain open. Movement audio and broader
  collision/building defects are separate goals.
