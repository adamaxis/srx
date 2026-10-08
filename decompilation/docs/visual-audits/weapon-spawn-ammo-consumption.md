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

The paired secondary record selects one of sixteen direction lists from
`0x630C`: selectors `0x00..0x1C` are the regular three-frame (`0x8003`) family,
and `0x20..0x3C` are the special eleven-frame (`0x800B`) family. Action codes
8..10 select the special family; the other supported codes use the regular
family. Thus the ROM ties the projectile family to action class and heading,
while the sound table is selected per action code/status. The source does not
provide a unique projectile clip for every gun.

The two spawned records have different roles. The primary keeps the
`0x14E0A` behavior continuation and direction-derived movement offsets from
`0x150E8`; its recovery loop probes terrain and scans active entity records
through `0x15ABA`. On contact, `0x14E0A` enters the linked-record
action/status path. The secondary is copied to the source spawn position,
receives the directional sprite-list selector, and uses `0xAC4E` as its
continuation. That continuation only checks the finite-list end marker and
clears the active bit. It does not advance world position. The current
three-frame Qt animation is the secondary spawn effect, not a traveling
projectile.

All sixteen direction lists use the same raw object-pattern block at ROM
`0xE44D8`, copied by the field setup path through `0x0F5C` to VRAM `0xB000`
(0xD00 bytes). The offline importer now follows that raw-copy path; it had
initially treated the block as a `0x1530` stream and exported transparent
frames. Sixteen one-pixel transparent assets in the special family are source
lists with no sprites, not missing texture data.

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
service resolves every table result to a named resource. Debug/Release
playback checks measure 0x1C/0x1D/0x3F/0x51 at 2.483/2.817/2.150/2.567 seconds;
0x1E/0x4A are checked at 2.283/2.267 seconds.

`FieldSceneRuntime::service_player_weapon_action()` runs from the ordinary
field-frame callback and calls `process_entity_spawn_cycle` using the decoded
runner, equipped action, heading, status byte, and loaded ammo. The decoded
Joshua archetype already has `+0x10=0x0F`, including the action bit; firing QA
checks that source state rather than forcing it. The typed spawn result selects
the direction/action-class asset list, submits event `0x1C`, and adds the
secondary record's source-projected position and animation to the field
presenter. Named sprite frames load from the asset catalog at bootstrap and
draw in the actual Qt field scene. The live test uses item 1 with six rounds,
fires through ordinary A input, then checks the typed ammo service (6->5),
unchanged shared reserve (2), C042 pose/delay, selector 0, and HUD count. The
Qt application routes the audio callback to playback; field QA uses an event
spy, while the separate playback CTest checks the mapped resource and duration.

This connects a visible timed secondary-spawn effect to the field loop, not
the complete 16-record entity/combat lifecycle. The native presenter keeps
paired records outside the authoritative shared slot table and currently
limits visible pair instances to the source's slots 9..14. It draws the
secondary at its source placement and advances its finite sprite list. It does
not retain the primary record, run its `0x14E0A` movement/recovery probes,
scan actual field entities, or apply the linked-record action/status result.
Those remain explicit boundaries. The secondary effect itself has no travel
behavior in the traced source path.

## Nine-question full-screen review

1. **Does it look correct?** The actor changes from idle to a directly
   extracted directional C042 frame. The named secondary-effect frames contain
   the ROM's source pixels and appear in full 320x224 captures. No exact
   matching Genesis firing pose was found in the reviewed longplay, so visual
   equivalence remains unconfirmed.
2. **Does the look reflect the state?** QA verifies the archetype's source
   action flags, ordinary A input, item 1 ammo 6->5, unchanged shared reserve
   (2), attack selector `0x40`, regular projectile selector 0, event `0x1C`,
   and visible ammo 05. Its animation trace records count, selector, and frame
   for all 52 action-delay updates.
3. **Are the visuals correct?** Eight direction-specific regular lists and
   eight special lists come from the decoded ROM pointers and raw object atlas.
   Full-screen captures show the second spawn-effect frame and its removal while
   the map, HUD, and controls remain in the same field composition. Independent
   firing pixels remain unavailable.
4. **Is it animated, and when?** C042 is one static actor frame. The decoded
   action delay begins from the source template's `+0xDC=0` readiness and holds
   for 52 updates until `+0xDC=-1`; 30 additional idle frames remain stable.
   The regular selector 0 advances 0→1→2 on the `0x1000`/`0x270F` timer and
   retires through the high-bit list marker on the third field update after
   spawn. This is the secondary spawn effect's animation; the primary
   collision/action record's cadence remains unimplemented in Qt. Another shot
   remains gated until movement returns to idle/walk.
5. **What sound plays and how long?** Item 1 requests event `0x1C`, resource
   028, a one-shot measured at 2.483 s. Holding A during the action delay does
   not submit repeated events; releasing A does not stop the one-shot. Playback
   state and duration are checked separately by the shared Debug/Release audio
   CTest. Live field-route loopback and independent Genesis audio correlation
   were unavailable.
6. **Were other screen regions checked?** QA writes the before frame, shot,
   all 52 delay images, 30 idle-hold images, and final alias. It requires a
   whole-screen pixel change for the visible spawn-effect frame and its removal,
   then pixel stability for every remaining delay and hold frame. The HUD count
   is checked against its named asset; there is no stale button, menu, or
   overlay.
7. **Do multiple frames support this interpretation?** Multiple complete
   native frames establish the state and timing sequence. Independent section
   06/07 footage was inspected, but it contains no identified weapon shot or
   matching attack pose. Internal frames do not establish Genesis equivalence.
8. **Does the page replace old UI cleanly?** The action opens no page and draws
   no controls. The C042 image replaces the actor's idle/walk image in the
   existing field composition; the projectile is removed after its finite list.
9. **Was the whole time window reviewed?** QA starts at a full-screen loaded
   idle frame, captures the A action, every source-delay update, and 30
   post-delay idle frames. It does not cover movement cancellation, projectile
   travel/impact, or combat return; those source/gameplay paths remain open.

## Verification and status

`shadowrun_ghidra_conversion_qt_field_player_action_qa` saves 85 BMPs in its
weapon-fire folder: before/shot, 52 action-delay frames, 30 idle-hold frames,
and the final hold alias. `projectile-animation.tsv` records count, selector,
frame, and whole-screen change for every delay update. It verifies the visible
second frame, finite-list removal, stable cleared pose, HUD ammo, source flags,
timer progression, and one event callback. All 85 Debug and Release captures
match by SHA-256. The graphics importer test verifies visible pixels in the
regular list's first two frames; entity animation and transition tests pass in
Debug and Release. The audio playback test separately checks mapped resources
and measured durations. No matching independent Genesis firing frame or audio
correlation has been found.

The player A action, ammo update, source sound request/playback route, C042
pose, and finite secondary spawn-effect presentation are connected and
internally verified. Shared entity-slot ownership, the primary record's
movement/recovery probes, actual entity collision, linked-record action/status
handling, full combat return, and independent matching Genesis firing/audio
evidence remain partial/open. The secondary effect has no travel behavior in
the traced source path. The combat hill is not defended.
