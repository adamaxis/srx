# Weapon Detail and Magazine Switch Review

Date: 2026-10-08
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Independent visual source: `video analysis/long_gameplay_video.mp4` and its
section 06/07 FFmpeg frames.

## Scope and ROM trace

This review covers the weapon Damage/Power rows in the field inventory and the
loaded-ammo state when switching weapons. It does not implement combat damage
or claim that combat fire/reload is complete.

The ROM's `0x12E9E -> 0x12F50` path reads the selected item's `D4540 + 0x25`
byte as the seven-tile Damage row index from `CF84C`, then writes that
56x8-pixel row at `C52A` (host position `(168,80)`). `0x13000` uses the same
item's `+0x26` selector for the Power label. Those values remain source table
lookups; this pass does not infer numeric damage from the bar art. The separate
seven-tile `CF8F0` table belongs to the residual/Drain path and is not reused
for the item Damage row.

Direct US-ROM bytes for item records 1 through 15 were checked at
`D4540 + (item_code - 1) * 0x30`: `+0x24` capacities are
`6,12,11,25,6,10,15,30,28,24,5,8,6,6,6`; `+0x25` Damage row indexes are
`0,0,0,0,1,1,1,1,1,1,2,2,2,1,1`; `+0x26` Power selectors are
`4,6,6,6,10,9,9,6,7,6,10,9,8,6,10`. These are the table bytes consumed by
the corresponding source paths.

The ammo complaint needs a precise source distinction. Each weapon's capacity
comes from its own `+0x24`. The active runner's `+0x4E` stores loaded rounds.
ROM `0x125FA` reads and clears that same runner byte, converts the loaded
rounds to a percentage using the current weapon capacity, and returns the
percentage to its caller. The unequipped state keeps that normalized value in
runner `+0x4E`; `0x121E0` scales it to the selected weapon's capacity when a
weapon is equipped. The percentage is not stored in each inventory item's
`+0x6E` phase byte. Spare `+0x4F` Universal Gun Clips are one runner-wide
reserve. The US ROM does not keep a separate spare-clip reserve for each
weapon. Existing Qt QA exercises weapon 1 at 3/6, weapon 2 at 6/12, the
shared 50-percent transfer, and reload consumption from the shared reserve.

## Code and verification added

- The item-catalog test checks all 15 source capacities, Damage row indexes,
  and Power selectors against the ROM-derived typed catalog.
- The graphics test checks all 14 extracted CF84C Damage rows and all 14
  CF8F0 Drain rows are 56x8 images.
- Qt field-action QA compares the rendered Damage row against the named
  `damage-bar-N.bmp` selected by the typed item record for item codes 1 and 2.
- Debug and Release Qt field-action QA each produced 54 full-screen BMPs. All
  54 Debug screens were reviewed at full screen size; each corresponding
  Release file has the same SHA-256 hash.

## Live field action QA update

The `shadowrun_ghidra_conversion_qt_field_player_action_qa` CTest drives the
live `FieldSceneRuntime` input path. It reloads item 1 to its ROM capacity of
6, then records every field update while source `+0xDC` counts down to `-1`:
103 full-screen images, each pixel-identical to the post-reload frame. Ordinary
Start, Left, Right, A, and Start input selects and equips item 2 through the
field inventory menu; its next A reloads to 12. The shared `+0x4F` reserve
falls 3->2->1. The existing inventory QA separately drives 3/6 -> 6/12 ->
3/6 percentage transfer.

Held A now reaches the live field action service when the active runner has an
eligible weapon record. The service consumes the typed magazine meter,
selects the regular/alternate event from the equipped item's source status,
submits it to the Qt audio callback, applies the ROM `0xB39A` direction selector
and `+0xDC` delay, and refreshes the HUD. Eight named C042 one-frame assets
replace the player walk image while the attack pose is active. This remains a
partial shot route: the source's paired entity allocation and secondary
`0x8003`/`0x800B` animation format are not yet rendered or advanced, and no
projectile target, collision, hit effect, or damage is applied.

The weapon-fire QA drives ordinary A input in the game loop and saves the
whole 320x224 before, attack, and held frames. It checks item 1 ammo 06->05,
unchanged shared reserve, active player/pose selector 0x40/direction, source
event 0x1C, a live HUD count of 05, and a byte-identical held frame. All three
frames were reviewed full-screen for unrelated field/HUD changes and stale
controls. Independent section 06 and 07 full frames were reviewed; no matching
firing pose or projectile event could be identified. This establishes the Qt
integration and source-backed action state, not Genesis visual equivalence.

The separate C route installs Ricky from the named source save-roster template,
then selects his active record through ordinary controller input; A reloads
that record to 6 rounds and reduces his reserve 3->2. Ricky's roster and item
2's presence are QA fixtures, not claimed story recruitment or inventory
acquisition.

The earlier 171-image route covers 10 action/menu checkpoints, all 103 reload
delay updates, 23 weapon-switch press/release frames, 30 field-return hold
frames, and five C/A runner frames. Debug and Release output hashes match.
The per-image difference and stability assertions cover the delay and return
holds.

## Nine-question visual review

1. **Does it look correct?** The Damage row remains a source-derived 56x8
   asset. The firing route changes the actor to the directional C042 image;
   no matching independent firing frame exists, so that appearance remains
   unconfirmed against Genesis.
2. **Does appearance match behavior?** The displayed item values follow
   `+0x25/+0x26`; the live fire action follows source flags and A input, spends
   one round (06->05), and changes the HUD count. The shared reserve remains
   unchanged until reload.
3. **Are the visuals correct?** Damage/Power and the separate Drain resources
   retain their source lookups. Eight directional C042 frames are individually
   extracted and replace the actor image. Whole-screen internal QA checks the
   unchanged map, other HUD counters, and UI; this is not an independent
   Genesis firing golden.
4. **Is it animated?** Damage/Power are static. C042 is a one-frame pose held
   for all 52 decoded `+0xDC` updates; 30 idle frames after expiry remain
   stable. Movement returns the actor to its walk/idle selection. Paired
   projectile animation and cadence are still unconnected.
5. **What sound plays and how long?** C runner selection requests `0x25`, and
   successful reload requests `0x4B` twice (2.134 s and 2.217 s mapped files).
   Item 1 A action requests `0x1C` / resource 028 (2.483 s measured). Shared
   Debug/Release audio QA verifies playback and durations; live field loopback
   and independent Genesis firing audio remain unavailable.
6. **Were related screen regions checked?** The 171-image route covers
   inventory, detail rows, Clips, equipment, runner selection, field return,
   and full-screen reload delay. The new 85-image fire route checks the full
   field and HUD on every timer/hold frame. The attack pose replaces the actor
   image; it does not cover adjacent controls or add a stale overlay.
7. **Do multiple frames support the interpretation?** The native route stores
   before, shot, every delay, and 30 idle-hold screens. Independent sections
   06/07 were reviewed, but no exact attack pose, ammo consumption, or shot
   sequence was found.
8. **Does the page replace prior UI cleanly?** Fire does not open a page. The
   existing field UI remains stable while the actor image changes; item/menu
   redraw and return behavior remain covered by the separate inventory QA.
9. **Was the affected time window reviewed?** All 52 action-delay updates and
   30 post-delay hold frames are saved as full screens and asserted pixel-equal
   to the attack frame. The 171-frame reload/menu route was also reviewed.
   Projectile/impact and movement-cancel timing remain unverified.

## Status

The source does not define a spare-clip reserve per weapon: `+0x4F` is shared
by the runner, and switching normalizes the active magazine through runner
`+0x4E`. Each weapon still has its own capacity, and that capacity controls
the loaded count after equip/reload. The typed mapping, inventory switch, reload, and initial live A-button ammo/audio/pose route are covered. `LOADOUT-01` remains partial until the paired source entities, projectile movement/rendering, target/collision/hit/damage behavior, and a matching independent firing example are reviewed.
`ITEMS-01` remains partial for combat damage values, shops, and independent
weapon-detail visual evidence.
