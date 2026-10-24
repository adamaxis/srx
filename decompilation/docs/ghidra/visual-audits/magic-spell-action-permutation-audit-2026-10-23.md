# Magic Spell Action Permutation Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Pocket Secretary Magic page, spell selection, force/posture setup, metric strips, and class/race presets (`0xFBA6`)

## Evidence

The raw ROM is the behavior source. The relevant field captures are the
independent 320x224 frames in video section 06:

| Capture | State visible |
|---|---|
| `frame_000001.png`, `frame_000002.png` | Rockskin selected |
| `frame_000005.png` | Mana Zap selected |
| `frame_000015.png`, `frame_000020.png` | Set Force & Posture action view |
| `frame_000030.png` | Mana Zap icon in the field HUD after equip |

These frames corroborate the Gator Shaman spell page and action view. They do
not establish every spell table or the Magical Defense fallback. The source
files are under `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/`.

## Presets and class/race coverage

The live record fields used here are resource `+0x7F`, Sorcery `+0x80`,
species/race `+0x9A`, class `+0x9B`, spell-table word `+0xA0`, attributes
`+0x77..+0x7C`, inventory `+0x66..+0x6D`, and item phases `+0x6E..+0x75`.
Species and class labels follow the ROM command-overview text rows. The five
spell-bearing templates cover four races and both caster classes.

| ROM template | Preset | Species / class | Body, Quickness, Strength, Charisma, Intelligence, Willpower | Magic / Sorcery / spell table | Totem |
|---|---|---|---|---|---|
| `0x7291A` | Joshua Samurai | Human 0 / Samurai 2 | 4, 3, 4, 1, 2, 4 | 0 / 0 / 0 | none |
| `0x72A1A` | Joshua Decker | Human 0 / Decker 3 | 2, 5, 2, 2, 4, 3 | 0 / 0 / 0 | none |
| `0x72B1A` | Joshua Gator Shaman | Human 0 / Shaman 1 | 2, 4, 1, 2, 3, 4 | 6 / 3 / 4 | Gator `0x2A` |
| `0x72C1A` | Rat Shaman | Dwarf 2 / Shaman 1 | 4, 2, 3, 1, 3, 4 | 6 / 3 / 0 | Rat `0x29` |
| `0x72D1A` | Troll Samurai | Troll 4 / Samurai 2 | 7, 2, 5, 1, 2, 3 | 0 / 0 / 0 | none |
| `0x72E1A` | Human Mage | Human 0 / Mage 0 | 2, 3, 2, 2, 3, 4 | 6 / 3 / 1 | none |
| `0x72F1A` | Dwarf Decker | Dwarf 2 / Decker 3 | 4, 3, 3, 2, 4, 3 | 0 / 0 / 0 | none |
| `0x7301A` | Bear Shaman | Orc 3 / Shaman 1 | 6, 3, 4, 1, 3, 5 | 6 / 4 / 2 | Bear `0x2B` |
| `0x7311A` | Elf Decker | Elf 1 / Decker 3 | 3, 5, 3, 3, 6, 4 | 0 / 0 / 0 | none |
| `0x7321A` | Human Samurai | Human 0 / Samurai 2 | 5, 5, 6, 3, 5, 4 | 0 / 0 / 0 | none |
| `0x7331A` | Elf Mage | Elf 1 / Mage 0 | 3, 5, 3, 5, 5, 5 | 6 / 5 / 3 | none |
| `0x7341A` | Elf Decker | Elf 1 / Decker 3 | 1, 7, 3, 5, 6, 6 | 0 / 0 / 0 | none |
| `0x7351A` | Human Samurai | Human 0 / Samurai 2 | 5, 6, 5, 3, 6, 5 | 0 / 0 / 0 | none |

The separate ROM race maximum table at `0xC22A6` is indexed by `+0x9A` and
orders attributes as Body, Quickness, Strength, Charisma, Intelligence,
Willpower:

| Race | Maximums |
|---|---|
| Human | 6, 6, 6, 6, 6, 6 |
| Elf | 6, 7, 6, 8, 6, 6 |
| Dwarf | 7, 5, 8, 6, 6, 7 |
| Orc | 9, 6, 8, 5, 5, 6 |
| Troll | 11, 5, 10, 4, 4, 5 |

The Magic formula does not branch directly on race. Race affects the selected
runner's preset attributes and race-specific attribute ceilings. The spell
route additionally depends on class, Magic resource, Sorcery, spell-table
index, inventory, phases, current force, posture, and active party membership.
The QA profile sweep seeds the Magic-relevant fields from the source presets;
its other inventory cells are cleared for the focused menu case, so it is not
a replacement for loading every complete runner record.

## Spell-table presets and force limits

At `0xFBA6`, a record enters the spell page when class `+0x9B < 2` and Magic
resource `+0x7F` is nonzero. Otherwise the two-bar Magical Defense page is
selected. The big-endian word at `+0xA0` selects one of five source tables at
`0x166AD8`; each table stores spell code and that spell's maximum force.
Spell names and row definitions are typed from the source catalog.

| Table | Source caster preset | Entries as `spell: maximum force` |
|---:|---|---|
| 0 | Dwarf Rat Shaman | Stink: 3, Flame Bolt: 1, Invisibility: 1, Rockskin: 2 |
| 1 | Human Mage | Flame Dart: 2, Heal Wounds: 2, Rockskin: 3 |
| 2 | Orc Bear Shaman | Mana Zap: 1, Mana Storm: 4, Sleep: 3, Heal Wounds: 2, Confusion: 2 |
| 3 | Elf Mage | Mana Blast: 3, Hellblast: 3, Super Barrier: 2, Invisibility: 3, Sleep: 4, Heal Wounds: 4 |
| 4 | Human Gator Shaman | Mana Zap: 3, Rockskin: 3, Heal Wounds: 1 |

The five actual table sizes are 3, 4, 5, or 6 entries; none can reach the
second seven-column row. Each selected force is clamped to `1..selected
spell's table maximum` at `0x100A8`. Runner posture is clamped to `0..4` at
`0x107CE`.

## What the displayed numbers mean

The two Magic-page defense strips are calculated from records, not copied from
a fixed runner preset. ROM `0x1047E` scans the three active party records and
includes only class 0 or 1. For each caster, `0x55F02` starts from Sorcery
`+0x80` and adds the phase of the highest-numbered Power Focus in inventory
slots 6 through 0. The source rounds each value to a quarter using integer
division and rounds up when the remainder is at least three. It adds that
party pool to the active runner's Body `+0x77` for Physical Defense and
Willpower `+0x7C` for Mana Defense, with byte wrapping. A value of zero uses
strip row 0; otherwise the displayed row is value minus one.

The left Success strip is also computed. `0x1055E` uses posture-reduced
Sorcery plus Power Focus, caps it at Magic resource `+0x7F`, adds the selected
force, then applies the matching spell focus from slots 6 through 0 and the
slot-7 Rat/Gator/Bear modifier selected by `0xC8AE0`. The spell-focus item is
`0x19 + action index`; its phase and the totem word both affect the displayed
value. The ROM uses a six-wide action-metric lookup (`row*6 + column`) even
though the spell cursor uses a seven-column presentation stride. That
difference is retained in the rule.

Two separate values are shown near the right side of the panel and must not be
confused:

- The top-right `Damage` label/bar comes from the selected spell's `D4EA0
  +0x25` byte in `0x1088C`. A negative byte clears that display; a
  nonnegative byte selects its source row.
- The lower-left `None` or effect strip comes from `0x105E6` calling
  `0x10660`. That calculation combines current force, the spell's `D4EA0
  +0x26` difficulty, posture-adjusted Sorcery and Willpower, source rounding,
  and the decoded `0x166AD4` base effect. Nonpositive residuals display
  `None`; positive residuals choose a Damage row.

This distinction resolved a possible visual mismatch while reviewing
`frame_000030`: its top-right `Damage` label is the `0x1088C` value, while the
lower-left result is the separate `0x105E6` calculation. The current renderer
paints both in those source positions.

The parent command page's Atk/Def bars are another calculation. ROM `0x12CEE`
starts from byte-wrapped `(Quickness + Intelligence + Willpower) / 2` plus the
first Combat Sense phase (`0x55ED6`), applies posture, and selects an
action-specific skill merge (`0x12C74`) for Attack. Defense applies the
opposite posture share, Body, an action-flag or item-`0x27` reduction
(`0x55E62`), and the cyberware bits 9 through 11. This is not the same pair of
strips as Magic Physical/Mana Defense; later bar audits should check these
inputs independently.

## Input permutation trace

The controller edge latch at `0x195C` puts new presses in the high byte of
`FFDD22`. The QA sends each 8-bit controller-port mask as a fresh press and
releases it before the next mask. ROM loop precedence is:

| Loop | Priority and outcome |
|---|---|
| Spell page | Left, Right, Up, Down, any A/B/C, then Start. The first matching direction consumes the poll; a direction plus confirm therefore moves only. The current five source tables make Up/Down no-ops because no second row exists. |
| Force (`0x10016`, wrapper `0xFF5A`) | Right, then Left adjust and continue. Otherwise A/B/C completes. Down without confirm enters posture; Down with confirm completes through the outer `0xFF5A` check. Start is ignored. |
| Posture (`0x10722`, wrapper `0xFF5A`) | Up returns before Right, Left, and A/B/C. Up alone returns to Force; Up plus A/B/C is completed by the outer wrapper's still-latched confirm edge. Right/Left adjust before confirm. Start is ignored. |

The Debug QA now sweeps all 256 possible controller-byte masks in each loop,
checks state and cursor/force/posture results, and covers all five source
caster profiles and all 21 spell entries. The focused Gator route also checks
the A/B/C completions, force and posture boundaries, clean parent return,
cursor cadence, visible source bars, and the selected spell's HUD art.

### Later-longplay search for the non-magic Defense page

The initial section-01 through section-07 review had no Defense fallback
capture. To check later gameplay, I sampled 10,096 frames at one-second
intervals across sections 08-41 of the same longplay dump, covering source time
0720 through 10815 seconds. A green-pixel template made from the host title at
`(120,48)` was used only to rank candidate frames; the strongest exact-position
match scored 0.5395. The highest candidates I opened were Matrix/datastore
menus and NPC/run dialogue (including sections 09, 11-14, 20, 24, 26-27, and
33), not the two-strip Magical Defense page. The locator is not OCR and this
sampling cannot rule out a very brief page or a differently rendered title.
No independent non-magic Defense image was identified in the reviewed
longplay; keep the visual claim partial.

## Disposition

- **Code changed:** Lua Force now accepts C alone as a completion input.
  Lua Posture preserves the ROM's Up-before-confirm behavior and reports the
  latched confirm to the outer `0xFF5A` caller. The Qt QA now exercises all
  256 controller masks in each Magic loop, the five caster spell tables, every
  table entry's force range, and its visible spell icon.
- **Debug verification:** the focused `shadowrun_ghidra_conversion_qt_field_magic_spell_qa`,
  `shadowrun_gameplay_script_runtime_qa`, and
  `shadowrun_ghidra_magic_menu_conversion` tests passed. The profile and input
  sweeps create internal BMP artifacts under
  `build/conversion/qt/debug/qt_runtime/field-magic-spell-qa/`.
- **Independent visual check:** the supplied section-06 captures support the
  Gator spell page, action heading, and equipped HUD icon. They do not cover
  the four other caster-table profiles or Magical Defense, and their 30 fps
  sampling does not establish VBlank cadence. A one-second candidate search
  through sections 08-41 also identified no Defense frame; candidate frames
  were manually opened, but the search is not conclusive absence proof.
- **Still partial:** no independent non-magic Defense frame is available;
  cast-target/effect continuation and Windows audio loopback are outside this
  pass. The internally generated profile frames are implementation checks,
  not Genesis visual goldens.

See also [the Magic screen split audit](magic-user-screen-split.md),
[the visual audit index](../visual-analysis/qt-visual-audit.tsv), and the ROM
entries `0xFBA6`, `0x10016`, `0x100A8`, `0x1047E`, `0x1055E`, `0x105E6`,
`0x10660`, `0x10722`, and `0x1088C` in
`decompiled-function-status.tsv`.
