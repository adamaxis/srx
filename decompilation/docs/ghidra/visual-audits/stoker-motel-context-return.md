# Stoker motel cancel: field-context return diagnosis

Status: implemented; focused Debug and Release motel route QA pass.

## Reported route and local reproduction

The user reported that at Stoker's Coffin Motel, choosing check in and then the
room menu's “never mind” response opens a dialogue error, places the runner
outside, leaves music silent, and prevents the field pause menu from opening.
The source route is the Scene-2 Stoker service interaction; it does not load a
separate motel interior.

The Qt QA route uses the real New Game controller path: walk Up into Scene-2
collision record 91, advance Stoker's arrival, choose the first manager-menu
option, choose B on the room offer, dismiss Stoker's farewell, and wait for the
field to settle. It writes full-frame BMPs and `state-route.tsv` under the
Debug build's `qt_runtime/field-motel-cancel-qa/` directory. The original QA
reached the farewell without a dialogue error, then failed its return
assertion: Scene 2 and the saved source-door lead `(15584,11210)` returned, but
the camera was `(13312,8688)` rather than `(13312,8656)`. The command overview
and movement checks had not run at that point.

The expanded route trace clarified the music state. At New Game spawn the
request list contains track 15 and the tracked current ID is 15. By the Stoker
arrival page, the route has issued a second fade and no track is active. The
room cancellation now restores saved track 15 after the fade; the request
count changes from 1 to 2 because playback had stopped. It is not a duplicate
request while track 15 is already active. Both builds return the saved lead
and camera, dismiss the dialogue without an error, resume movement, and open
and close the Start command menu after its reveal settles.

## ROM evidence [C]

- Scene-2 collision record 91 is at `0x173138`; the existing doorway audit
  decodes its bounds `(0x3C0..0x3DF, 0x280..0x29F)` and property `0xC2`.
- The mode-exit orchestrator at `0x1F5CA` runs the field-exit selector at
  `0x1F65A`. Its Scene-2/event-2 branch at `0x1F72A..0x1F76C` selects the
  Stoker profile and arrival record `0xF4AA0` before calling the shared
  prompt service `0x1F246`.
- The source return-point builder at `0xBC36` reads a three-position bank from
  `0xBD34`, stores party anchors at `FFDF44..FFDF4E`, and saves camera words
  from `FFDDC6/FFDDC8` to `FFDF50/FFDF52` (`0xBC98..0xBCAC`).
- ROM `0x2F32` restores the saved camera words from `FFDF50/FFDF52` to
  `FFDDC6/FFDDC8` (`0x2F82..0x2F96`), then enters `0x2BD2`. The source
  transition aligns both camera words with `0xFF80` at `0x2BE2/0x2BEA`.
  Therefore the ROM has an explicit camera-rounding step. The user-directed
  native context pop restores the exact pre-dialogue camera after that source
  refresh; the intermediate alignment remains source-derived.
- ROM `0x2900` requests profile music only when `FFE172` is set. A same-scene
  `0x2BD2` refresh does not set that latch by itself. The descriptor requests
  profile-music resume; the host restores the saved track only after the door
  fade has stopped playback, and skips a second request when it remains active.

## Native route and failure boundary

`stoker_motel.indexed_service` in
`decompilation/gameplay/dialogue/conversations/stoker_motel.json` routes the
room-menu `leave` choice to the farewell page and then `continue_in_place`.
`DialogueFramework.execute_exit()` applies this route through multiple native
exit primitives (`stay`/`source_door_return`, `fade`, profile music, and input
lock). The callback bridge is in
`qt_runtime/src/field/field_scene_runtime.cpp`; return placement and camera
follow are in `field_scene_runtime_transitions.cpp` and
`field_scene_runtime_world_effects.cpp`.

The native exit bridge now receives one Lua operation,
`restore_field_context_<behavior>`. The generic push captures the typed scene,
party lead/records, camera-follow state and camera entry coordinates, active
world/input coordinator state, activity counters, the scene's last requested
music ID, attenuation, and Start-menu duck state. The pop restores the scene's
position/camera and input context when the return remains on that scene; it
preserves the return-facing policy and applies the descriptor's transition and
post-return input lock. It restarts the saved field track only when the active
track changed or stopped. A field dialogue fallback at the start of the host
frame covers existing native prompt entry points as well as Lua-started
dialogs.

Debug and Release evidence:
`shadowrun_ghidra_conversion_qt_field_motel_cancel_qa` passes in both builds.
The New Game route records exact saved camera `(13312,8656)`, lead
`(15584,11210)`, track 15 restored after the doorway fade, unchanged funds,
zero pending transition, resumed movement, and Start-menu open/close. The
The `dialogue_framework_tests` case passed inside
`shadowrun_gameplay_lua_suites`; that aggregate still reports the known
unrelated `field_random_events_tests` and `equiv_units` failures. These internal
captures are port evidence, not an independent Genesis comparison.

## Shared-exit census to retain during the fix

`dialogue_framework.lua` defines shared exit families for building entry,
building/service exit, field NPC contact, field event/activity, random and
scripted encounters, vid phones, recruited runners, run targets, story
activity/rewards, recovery, field menus, taxi, and cinematics. Route overrides
add generic contact/Ilene timing and Stoker's source-door return. The host
bridge currently also carries per-dialogue transition flags for source-door
return, scene change, and activity-source restoration. Review these shared
families and the route overrides before removing any primitive; the source
door facing policy is an intentional project choice.

The handoff's `/workspace/out/DIAG.md` is not present in this Windows checkout.
This file is the current equivalent diagnostic record. Its requested status
summary belongs in the current `decompilation/docs/ghidra/visual-audits/`
tree; do not create or overwrite the unavailable `/workspace/out/` directory.
