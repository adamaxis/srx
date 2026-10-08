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

## Live reload action QA update

The `shadowrun_ghidra_conversion_qt_field_player_action_qa` CTest now also
drives the live `FieldSceneRuntime` A-button action. It starts with item 1
empty and three shared clips, then checks reload to its ROM-defined capacity
of 6 and a reserve count of 2. In the same runtime, QA seeds item 2 as the
selected empty weapon and verifies its ordinary held-A reload reaches its own
12-round capacity and consumes the next shared clip, leaving one. This second
weapon setup is diagnostic state injection; the existing inventory-menu QA
separately drives the ordinary equip route and verifies the source-derived
3/6 -> 6/12 -> 3/6 percentage transfer.

The player-action QA writes seven 320x224 full-screen frames. Debug and Release
outputs are compared by SHA-256, and the entire Debug sequence is reviewed for
the weapon glyph, ammo counter, map, HUD, and stale controls. This evidence
connects the reload action to the field controller/runtime; it does not
connect the broader fire/projectile/target/hit loop or prove an independent
Genesis visual match.

## Nine-question visual review

1. **Does it look correct?** The displayed rows occupy the source-derived
   Damage row position and use a full 56x8 asset. The Qt row is internally
   compared against the named extracted CF84C row. No selected-weapon Damage
   screenshot exists in the independent longplay, so independent placement
   confirmation remains unavailable.
2. **Does appearance match behavior?** Yes for the tested source states. The
   visible row follows the selected item's `+0x25`; Power follows `+0x26`.
   The separate HUD count follows the active weapon's `+0x24` capacity and
   `+0x4E` loaded state. The existing Qt route also verifies that the shared
   `+0x4F` clip count
   decreases only on reload.
3. **Are the visuals correct?** The asset source and mapping are checked
   against CF84C and the D4540 table. The source-defined CF8F0 Drain assets
   remain separate. This is internal source/renderer verification, not an
   independent Genesis golden for a selected weapon.
4. **Is it animated?** No. The item Damage/Power rows are redrawn with the
   menu state and have no animation counter or timed playback.
5. **Is there a sound?** No sound call is present in the item-row draw path.
   Equip confirmation and selector movement use separate events `0x24` and
   `0x25`; the mapped effect files are 2.484 s and 2.134 s respectively.
   This pass did not record Windows loopback for these exact weapon-detail
   transitions, so those are decoded-file durations, not measured audible
   route durations.
6. **Were neighboring screen regions checked?** Yes. All 54 full-screen
   captures were reviewed, including portrait/HUD, item cells, detail rows,
   action buttons, Clips prompt/result, armor replacement, gear, and return.
   No stale buttons or unrelated graphics were left over the new state.
7. **Do multiple frames support the interpretation?** Internal frames cover
   item 1, item 2, equip/unequip, ammo percentage, reload confirmation/result,
   and 30-frame holds. Independent section 06 frames `002550`, `002560`,
   `002580`, `002610`, `002640`, and `002699` show neighboring command and
   inventory states; none shows a selected weapon's Damage row or a weapon
   load transfer. They do not independently validate those details.
8. **Does the page replace prior UI cleanly?** The full-screen route shows
   action rows and Clips child controls replacing their parent panel, then
   returning without stale controls. Equip/unequip and armor/gear toggles
   match fresh full-screen redraws in the existing QA.
9. **Was the affected time window reviewed?** Yes for these static menu
   states: all 54 checkpoints were inspected, and the source QA's 30-frame
   equip/unequip holds are pixel-identical. This is a checkpoint set, not a
   continuous capture of every VBlank.

## Status

The source does not define a spare-clip reserve per weapon: `+0x4F` is shared
by the runner, and switching normalizes the active magazine through runner
`+0x4E`. Each weapon still has its own capacity, and that capacity controls
the loaded count after equip/reload. The typed mapping, natural inventory
switch, and live field reload action are covered; `LOADOUT-01` remains partial
until normal combat fire/projectile behavior and a matching independent
loaded-save example are reviewed.
`ITEMS-01` remains partial for combat damage values, shops, and independent
weapon-detail visual evidence.
