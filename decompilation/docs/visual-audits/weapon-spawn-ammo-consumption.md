# Combat spawn ammo and sound selection

## Scope and source trace

This slice completes the native source model at the field spawn-cycle boundary.
It does not connect the live Qt runner to target selection, projectile rendering,
or hit/damage handling.

- ROM `0x1514E` rejects global mode 3 and action windows where
  `(+0x16 & 0x7FF) > 0x3C`. After the source behavior gate, it calls `0x38AC`
  before asking for either free entity slot.
- ROM `0x38AC` returns inactive when `+0x4E` ammo or `+0x57` action code is
  zero. For record type `+0x3E <= 2`, action codes 8..10 send `-3` to
  `0x36FC`; other codes send `-1`. Higher record types decrement `+0x4E`
  directly by one. A nonzero result continues the spawn cycle, including when
  the meter reaches zero during that action.
- The first slot allocation is at `0x151B6`, after the ammo service. Thus an
  eligible action can consume ammo even when no free slot is available. The
  native model preserves this ordering and represents a missing weapon-service
  callback as unresolved instead of silently allowing a shot.
- The words at `0x15318`/`0x15336` are sound event IDs, not projectile asset
  IDs: `0x15238` submits the selected value to `0x530DE`. For action codes
  8..10 the ordinary table selects `0x1E`; the alternate table selects `0x4A`
  when `+0x56` status bits 1 or 2 are set. The Qt audio service maps those
  events to individually named resources 30 and 74.
- Resource 30 is 100,716 decoded samples at 44.1 kHz, or 2.284 seconds.
  Resource 74 is 99,975 samples at 44.1 kHz, or 2.267 seconds. Both are
  one-shots from the extracted FLAC assets. Their sound identity in a live
  combat scene has not been independently matched to the longplay.

## Verification questions

1. **Does it look correct?** This change has no visible output. It does not
   make the weapon or projectile look correct.
2. **Does the current look reflect the behavioral state?** The native spawn
   state now carries ammo and action code separately. The typed tests verify
   the source meter deltas, inactive-ammo gate, unresolved boundary, and
   ordering before allocation. The Qt runtime does not call this spawn cycle.
3. **Are the visuals correct?** No renderer changed, so there are no new
   combat pixels to compare. The projectile and target visuals remain open.
4. **Is it animated, and when?** The source action is eligible only inside
   the `+0x16` window through `0x3C`; this slice does not create or advance a
   projectile animation. Its source cadence and animation still need a live
   field reconstruction.
5. **What sound plays and how long?** Ordinary action codes 8..10 request
   event `0x1E` / resource 30 for 2.284 seconds; alternate status requests
   `0x4A` / resource 74 for 2.267 seconds. Debug and Release playback QA
   checks alias selection, decoding, playing state, and duration on the
   default output device. The longplay has no identified matching shot/audio
   sample, so this does not establish the Genesis sound in context.
6. **Were related screen areas checked?** The change only alters typed combat
   state and the shared audio alias map. It does not touch the weapon detail
   page, magazine HUD, command menu, or dialogue. No new full-screen scene
   dump exists because there is no live spawn path to capture.
7. **Do multiple independent frames support the interpretation?** No. The
   reviewed neighboring longplay frames do not show a clear weapon firing
   sequence. Internal tests support source ordering and audio decoding, not
   Genesis visual fidelity.
8. **Are new controls drawn over stale UI?** No controls or panels are drawn
   by this slice. The Qt renderer is untouched.

## Current status

`shadowrun_ghidra_entity_combat`, `shadowrun_ghidra_entity_transition`, and
`shadowrun_ghidra_audio_playback` cover the native ammo transaction and
selected sound assets in Debug and Release. The combat hill stays partial:
`FieldSceneRuntime::service_player_weapon_action()` only reaches the empty
magazine action when a prepared behavior bit is injected, and the live Qt field
scene still lacks a connected target/action/spawn/render path. Normal firing,
the projectile animation, hit/damage, and independent Genesis evidence remain
open.
