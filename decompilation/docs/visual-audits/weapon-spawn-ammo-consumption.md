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

Action codes 13..15 take the separate effect-record path at ROM 0x15646 before 0x1514E; that path calls asset-meter routine 0x36FC, not ammo service 0x38AC. The 0x36FC branch for codes 13..15 decrements the equipped inventory phase byte and clears that slot at zero; it does not change weapon ammo rounds. The connected Qt weapon action path likewise accepts only weapon codes 1..12 before reload or ammo handling.

The paired secondary record selects one of sixteen direction lists from
`0x630C`: selectors `0x00..0x1C` are the regular three-frame (`0x8003`) family,
and `0x20..0x3C` are the special eleven-frame (`0x800B`) family. Action codes
8..10 select the special family; the other supported codes use the regular
family. Thus the ROM ties the directional spawn-effect list to action class
and heading, while the sound table is selected per action code/status. The
source does not provide a unique projectile clip for every gun.

The two spawned records have different roles. The primary keeps the
`0x14E0A` behavior continuation and direction-derived movement offsets from
`0x150E8`; its recovery loop probes terrain and scans active entity records
through `0x15ABA`. On contact, `0x14E0A` enters the linked-record
action/status path. The secondary is copied to the source spawn position,
receives the directional sprite-list selector, and uses `0xAC4E` as its
continuation. That continuation only checks the finite-list end marker and
clears the active bit; it does not advance world position. The secondary's
three-frame list is a spawn effect. The primary travels independently through
its source offsets and terrain-recovery behavior.

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
the direction/action-class asset list and submits event `0x1C`. The Qt field
runtime retains the primary and secondary records in source slots 9..14. The
`0xA8EA` behavior pass advances primary movement/recovery, while the `0x4618`
render pass sends both records through Lua's finite-list animation heartbeat.
Lua advances the `0x5FDC` accumulator, resolves the `0x600E` frame/terminal
marker, and returns the named sprite frame; the renderer sorts them with the
player. Named sprite frames load from the asset catalog at bootstrap and draw
in the actual Qt field scene. Primary movement and collision recovery remain
native C++ behavior and are an explicit port boundary.
The live test uses item 1 with six rounds, fires through ordinary A input, then
checks the typed ammo service (6->5), unchanged shared reserve (2), C042
pose/delay, selector 0, and HUD count. The Qt application routes the audio
callback to playback; field QA uses an event spy, while the separate playback
CTest checks the mapped resource and duration.

The same field QA also takes the ordinary Start/Right/A menu route to equip
item 2, reloads it to its own 12-round capacity, waits for the reload delay to
finish, then fires through ordinary A input. It checks 12->11 in item 2's
loaded magazine, unchanged shared reserve (1), sound event `0x1C`, regular
selector 0, and the secondary effect's frame 0->1->2 sequence. `0x600E` marks
the finite list complete during render update 3; the next `0xA8EA` pass retires
it before render update 4. The six full-screen item-2 checkpoints show the
ready state, shot, three in-list/terminal states, and the cleared secondary
effect with no menu controls left over.

The Qt field loop now retains both typed records returned by the ordinary
player-A spawn path. Its weapon allocator uses the source six-slot window
9..14, assigns primary and secondary records to separate free slots, and
releases each slot when its own source continuation ends. Every field update,
the primary follows `0x14E0A`: it advances its extracted animation, makes four
`0x14F50` fixed-point terrain/recovery probes against the currently loaded
scene collision map, publishes the current `0xE7C8` boundary result, commits
accepted movement, and enters the finite stopped tail on terrain contact or
retry exhaustion. The secondary follows `0xAC4E` and only advances its finite
named directional effect at its spawn point. The renderer draws player, primary,
and secondary records in `0x5B1E`'s stable descending effective-depth order;
source flags `0x10`/`0x40` force depth zero.

This is connected movement and terrain retirement, not the complete 16-record
entity/combat lifecycle. The current map importer has no populated live actor
records, so collision search receives no fabricated targets; linked-record
hit/status/damage remains an explicit boundary. The source requests event
`0x3C` on the non-outside terrain terminal path. Its table-2 resource 60 is
omitted from the named audio assets as a silent slot, so there is no host clip
or measured playback duration to reproduce. The secondary effect itself has
no travel behavior in the traced source path.

## Nine-question full-screen review

1. **Does it look correct?** The actor changes from idle to a directly extracted
   directional C042 frame. The primary moves away from the spawn point and stops
   on the scene's terrain; the secondary directional effect stays at its source
   position. No matching Genesis firing scene was identified, so equivalence is
   not established.
2. **Does the look reflect the behavioral state?** QA uses Joshua's decoded
   source action flag and ordinary A input. It verifies item 1 ammo 6->5 with
   reserve 2, then item 2 ammo 12->11 with reserve 1; checks source event `0x1C`,
   C042 selector `0x40`, secondary selector/frame, and records primary position,
   heading offsets, retry count, and retirement. The test does not fabricate an
   enemy hit when the scene supplies no actor records.
3. **Are the visuals correct?** Eight regular and eight special directional
   lists are selected from the decoded ROM pointers and raw object atlas by
   action class and heading. The ROM does not provide a unique projectile clip
   per gun. The primary uses the source movement record and named extracted
   frames; the secondary uses the stationary finite spawn-effect list.
   Independent firing pixels remain unavailable.
4. **Is it animated, and when?** C042 is one static actor pose held during the
   source action delay: 52 field updates, followed by 30 stable idle holds. Lua
   advances regular selector 0 from `0 -> 1 -> 2` on the source
   `0x1000`/`0x270F` cadence. Render update 3 marks the list complete while
   retaining frame 2; the following behavior update retires it before drawing.
   Special `0x800B` selectors have 11 frames. With the same signed `0x270F`
   accumulator and `0x1000` period, the source wraps the word and holds frame 4
   on updates 5-8 and frame 9 on updates 14-17; it marks completion on update
   19 and the following behavior pass retires it. The Lua runtime checks this
   source sequence, but the connected field route has not captured a special
   weapon effect yet.
   Lua performs four source terrain/recovery probes per field update, moves
   while the native terrain service permits it, then selects the finite stopped
   tail on terrain contact. Retry exhaustion or an outside-bounds result stops
   and retires it according to the `FFDF68` marker. The stopped-list countdown
   and secondary finite-list retirement are also Lua policy. There is no live
   actor-hit animation yet.
5. **What sound plays and how long?** Items 1 and 2 request event `0x1C`,
   resource 028, a one-shot measured at 2.483 seconds. Holding A during the delay
   does not repeat it. Terrain terminal event `0x3C` is source-requested but maps
   to omitted silent resource 060, so it has no host clip or audible duration.
   Mapped playback state/duration is checked by the separate Debug/Release audio
   CTest; route loopback and independent Genesis audio correlation remain open.
6. **Were other screen regions checked?** All full-screen output includes the
   map, actor, HUD, selector, and right-side controls. QA captures the before/shot
   frames, all 52 delay updates, 30 idle holds, and the item-2 equip/reload/fire
   return route. The map and HUD remain intact as both records move and retire.
7. **Do multiple frames support this interpretation?** After moving projectile
   behavior into Lua, all 262 Debug and Release screenshots still matched by
   SHA-256. The current full-screen captures were reviewed in nine fresh
   contact sheets of 30 frames each; the before/shot/three-delay/retirement
   frames were also inspected at native size. Independent sections 06/07 were
   reviewed, but they contain no identified player firing example or matching
   pose. Internal native frames prove route consistency, not Genesis visual
   fidelity.
8. **Are new visuals cleanly redrawn?** Firing opens no page and adds no
   controls. The C042 pose replaces the actor's field sprite; projectile images
   follow the source entity draw order with the player. Both slots are released
   when their own continuations end, and no projectile pixels remain in the
   settled captures.
9. **Was the whole time window reviewed?** QA starts from a loaded idle screen,
   captures the action and every delay update, then captures 30 post-delay idle
   frames. It also captures the item-2 route from equip/reload through firing and
   cleanup. Movement cancellation and linked actor hit/damage are not covered.

## Verification and status

`shadowrun_ghidra_conversion_qt_field_player_action_qa` saves 85 BMPs in its
weapon-fire folder: before/shot, 52 action-delay frames, 30 idle-hold frames,
and the final hold alias. `projectile-animation.tsv` records both records'
counts, primary position/origin, offsets, retry count, secondary
selector/frame/completion, and whole-screen change for every delay update. It
verifies primary movement and terrain retirement, the visible secondary frame,
finite-list removal,
stable cleared pose, HUD ammo, source flags, timer progression, and source audio
events. The Lua runtime test now directly checks four ordered probes, terrain
rejection, retry wrap/exhaustion, outside-bound sound/retirement, target
position handoff, stopped-list retirement, and the unresolved linked-action
outcome. The remaining item/action/menu/runner route supplies 177 additional
full-screen captures. The extra item-2 terminal checkpoint brings the total to
262 Debug and Release BMPs; all match by SHA-256. The current Debug screens were
reviewed in nine fresh full-screen contact sheets, and the firing window was
inspected at native size. Qt field action and player-action CTests pass in
Debug and Release. The audio playback test checks mapped resources and
durations. No matching independent Genesis firing frame or audio correlation
has been found.

Ordinary A input now connects the source spawn helper, weapon ammo/HUD update,
mapped shot sound, C042 pose, six-slot weapon allocator, Lua-owned
primary/secondary finite animation and behavior, primary fixed-point movement,
live map-terrain sampling, slot retirement, and the source field depth order.
The source's live actor records have not been imported; target selection,
linked-record hit/status/damage, movement
cancellation, and full combat return remain open. Event `0x3C` has no mapped
audible asset, and independent firing visuals/audio are unavailable. The
secondary effect has no travel behavior in the traced source path. LOADOUT-01
and its combat hill remain partial, not defended.
