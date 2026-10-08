# Player weapon action and spawn-cycle boundary

## ROM path and connected runtime slice

US ROM `0xA8EA` advances the active field entity record. For a record with the
source action bit (`+0x10`, bit 1), `0xACAE` checks held A (`FFDD22`, bit 6),
requires the animation-window value `(+0x16 & 0x7FF) <= 0x3C`, and routes
weapon action codes 1..12 to `0x1514E`. That routine checks its behavior gate,
then calls `0x38AC` before either paired entity slot allocation. It initializes
the primary at `0x14FF8`, copies the secondary at `0x15278`, chooses one of the
`0x62EC`/`0x62FC` animation tables, and submits its selected sound event at
`0x15238`. The caller then enters `0xB39A`, which chooses the directional
C042 attack pose, applies the source action delay, and resets animation state.
The C042 pose is a static one-frame list held during that delay; it is not a
multi-frame weapon swing.

`0x38AC` returns inactive when `+0x4E` rounds or `+0x57` action code is zero.
For record type `+0x3E <= 2`, action codes 8..10 send `-3` to the typed asset
meter service; other codes send `-1`. Higher types decrement `+0x4E` directly.
A nonzero result continues even when that action consumes the final round. The
first slot allocation follows this ammo service at `0x151B6`, so the ROM may
consume ammunition even if a slot cannot be allocated. The typed
`process_entity_spawn_cycle` preserves the ordering and exposes allocation and
spawn-preparation boundaries as native callbacks.

The sound words at `0x15318`/`0x15336` are event IDs, not projectile resources.
The ordinary table selects `0x1C` for codes 1..4, `0x51` for 5..7, `0x1E` for
8..10, and `0x1D` for 11..12. The status-bit alternate table selects `0x3F`
for codes 1..7 and 11..12, and `0x4A` for codes 8..10. The Qt audio alias
service now resolves every table result to a named resource. Debug/Release
playback checks measure 0x1C/0x1D/0x3F/0x51 at 2.483/2.817/2.150/2.567 seconds;
0x1E/0x4A are checked at 2.283/2.267 seconds.

`FieldSceneRuntime::service_player_weapon_action()` now runs from the ordinary
field-frame callback. The source Joshua archetype already has `+0x10=0x0F`,
including the action bit; the firing QA verifies those source flags and does
not force them. It uses the decoded archetype's initial `+0xDC=0`, ordinary A
input to reach the ready state, a loaded item-1 fixture (6 rounds), then checks
the live typed ammo service (6->5), source status-selected event `0x1C`, the
C042 directional player pose, its delay, and the HUD count. The Qt application
routes the audio callback to the playback service; this field QA uses an event
spy, while the separate playback CTest proves decoding, playback state, and
measured duration.

This is an integrated player-action slice, not the full ROM spawn cycle. The
live Qt action does not yet call `process_entity_spawn_cycle`, allocate the
paired records, or render and advance their `0x8003`/`0x800B` descriptor family.
It therefore has no projectile movement, target acquisition, collision, hit
effect, or damage application. Do not call it complete combat or a visible
projectile.

## Nine-question full-screen review

1. **Does it look correct?** The player actor changes from the idle sprite to
   one directly extracted directional C042 frame. All images are complete
   320x224 screens. No exact matching Genesis firing pose was found in the
   reviewed longplay, so the attack image is source-backed but independently
   unconfirmed.
2. **Does the look reflect the state?** The QA verifies the archetype's source
   action flags, ordinary A input, item 1 ammo 6->5, unchanged shared reserve
   (2), attack selector `0x40`, event `0x1C`, and visible ammo 05. The pre-A,
   shot, every timer frame, and held-state traces record the associated typed
   values.
3. **Are the visuals correct?** The eight directional assets are extracted
   from C042, with source palette and orientation attributes retained; the
   renderer replaces the player image rather than painting a pose over the
   idle/walk sprite. The whole screen was checked for changes to map, HUD, and
   controls. Independent firing pixels remain unavailable.
4. **Is it animated, and when?** C042 is one static frame. The decoded action
   delay begins from the source template's `+0xDC=0` readiness, then holds the
   pose for all 52 updates until `+0xDC=-1`; 30 more idle frames remain stable.
   The source action window blocks another shot until movement returns the
   actor to an idle/walk selector. Projectile cadence and lifetime remain open
   because its descriptor list is not yet connected.
5. **What sound plays and how long?** Item 1 requests event `0x1C`, resource
   028, a one-shot measured at 2.483 s. Holding A during the action delay does
   not submit repeated events; releasing A does not stop the already submitted
   one-shot. Playback state and duration are checked separately by the shared
   Debug/Release audio CTest. Live field-route loopback and independent Genesis
   audio correlation were unavailable.
6. **Were other screen regions checked?** The QA writes the before frame, shot,
   every one of the 52 delay images, 30 stable idle-hold images, and a final
   alias. Each delay and hold frame is required to match the shot's entire
   320x224 image. The HUD count is checked against its extracted named asset;
   there is no stale button, menu, or overlay.
7. **Do multiple frames support this interpretation?** Multiple complete
   frames establish the native state and timing sequence. Independent section
   06/07 footage was inspected, but it contains no identified weapon shot or
   matching attack pose. These internal frames do not establish Genesis
   visual equivalence.
8. **Does the page replace old UI cleanly?** This action opens no page and
   draws no controls. The C042 image replaces the actor's idle/walk image in
   the existing field composition, with no stacked actor sprite or stale UI.
9. **Was the whole time window reviewed?** The QA starts at the full-screen
   loaded idle frame, captures the A action, all 52 source-delay updates, and
   30 post-delay idle frames. It does not claim to cover movement cancellation,
   projectile creation/impact, or combat return; those source/gameplay paths
   remain open.

## Verification and status

`shadowrun_ghidra_conversion_qt_field_player_action_qa` saves 85 BMPs in its
weapon-fire folder: before/shot, 52 action-delay frames, 30 idle-hold frames,
and the final hold alias. It verifies per-frame whole-screen stability, HUD
ammo, source flags, selector, timer progression, and one event callback. Debug
and Release captures match by SHA-256. `shadowrun_ghidra_audio_playback`
separately checks mapped assets and measured durations. The intro/menu sound
and screen systems are unaffected by this field change.

The player A action, ammo update, source sound selection/playback route, and
C042 pose are connected and internally verified. The paired entity spawn,
projectile descriptor decoding/rendering, target/collision/hit/damage path,
full combat route, and independent matching Genesis firing/audio evidence
remain partial/open. The combat hill is not defended.
