# Field encounter trigger contract

ROM entry `0x1E166` defines the observable gates for a field encounter. The
native adapter translates its memory reads into game facts before calling
`field_encounters.lua`; Lua never receives the RAM words, byte counts, frame
counter, or PRNG operands.

| Game fact passed to Lua | ROM condition | Native translation |
| --- | --- | --- |
| `display_busy` | `FFDFE8 != 0` blocks (`0x1E166..0x1E16C`) | True when field presentation is occupied. [C] |
| `leader_unseen` | `FFFC04` record `+0x11` bit 7 blocks (`0x1E170..0x1E17C`) | Read the active leader's named `UNSEEN` status flag. [C] |
| `world_activity_locked` | `FFE18B != 0` blocks (`0x1E180..0x1E186`) | True while a field activity owns the world. [C] |
| `alarm_active` | `FFE16F` permits the combat-active exception and selects the alarm outcome (`0x1E18A`, `0x1E1D0`) | Read the current alarm state. [C] |
| `combat_active` | `FFDF84 != 0` blocks only when the alarm is clear (`0x1E194..0x1E19A`) | Read whether field combat is active. [C] |
| `encounter_in_progress` | `FFE177 != 0` blocks (`0x1E19E..0x1E1A4`) | True when an encounter is already occupying field actor slots. [C] |
| `encounter_check_due` | `FFDF88 & 0x03FF == 10` (`0x1E1A8..0x1E1B6`) | The native field clock reports whether this scheduled opportunity is due. Mapping that clock to source cadence remains [H]. |
| `encounter_probability` | Unsigned PRNG result must be below `FFDE18` (`0x1E1BC..0x1E1CA`) | The adapter scales the profile threshold by `1/65536`. [C] |
| `probability_sample` | The same unsigned PRNG result from `0xD44` | Requested lazily by Lua, then scaled by `1/65536`; the strict less-than comparison is unchanged. [C] |

The lazy draw preserves source RNG consumption: blocked and not-due field
passes do not advance the RNG. Scaling both unsigned operands by `1/65536`
preserves the source comparison exactly, including equality at the threshold.

## Abandoned-building encounter audit

The handoff does not identify which building produced the report. ROM
collision and event-table rows identify eight guide-crosswalk building sites
across three source maps and four destination scenes:

| Source map | Guide marker | ROM collision record and event | Event-table result | Destination |
| --- | --- | --- | --- | --- |
| Downtown (scene 0) | 10 | record 50 at `0x0D57D2`, event word `0x00C8` (index 8) | `0x0D54B2[8] = 0x32` | scene 50 |
| Downtown (scene 0) | 11 | record 51 at `0x0D57DC`, event word `0x00C9` (index 9) | `0x0D54B2[9] = 0x33` | scene 51 |
| Puyallup (scene 1) | 1 | record 66 at `0x17D998`, event word `0x00C8` (index 8) | `0x17D620[8] = 0x32` | scene 50 |
| Puyallup (scene 1) | 10 | record 75 at `0x17D9F2`, event word `0x00CA` (index 10) | `0x17D620[10] = 0x33` | scene 51 |
| Puyallup (scene 1) | 12 | record 78 at `0x17DA10`, event word `0x00CB` (index 11) | `0x17D620[11] = 0x34` | scene 52 |
| Penumbra (scene 3) | 7 | record 102 at `0x18ABD2`, event word `0x00C8` (index 8) | `0x18A6FC[8] = 0x34` | scene 52 |
| Penumbra (scene 3) | 4 | records 103/104 at `0x18ABDC`/`0x18ABE6`, event word `0x00C7` (index 7) | `0x18A6FC[7] = 0x33` | scene 51 |
| Penumbra (scene 3) | 8 | record 105 at `0x18ABF0`, event word `0x00C9` (index 9) | `0x18A6FC[9] = 0x35` | scene 53 |

These rows are confirmed by the raw US ROM bytes and the collision-record
inventory. The guide names are location crosswalks; the ROM only supplies the
numeric activity scene. Testing a bootstrapped interior alone would not
exercise the source doorway state.

The scene-profile bases for scenes 50-53 are `0x1E8D3C`, `0x1E911C`,
`0x1E94DC`, and `0x1E96E4`; word 41 (byte offset `0x52`) is `0xC000` at
`0x1E8D8E`, `0x1E916E`, `0x1E952E`, and `0x1E9736` [C]. With the strict
unsigned comparison at `0x1E1BC..0x1E1CA`, that threshold accepts 75% of source
random words. The ROM increments
`FFDF88` in `wait_for_vblank` at `0x0BD2` and checks its low ten bits at
`0x1E1A8..0x1E1B6`; one opportunity is scheduled every 1024 counter ticks.
At 60 ticks per second that is about 17.1 seconds [H: native clock mapping and
host cadence].

The Debug and Release CTest
`shadowrun_ghidra_conversion_qt_abandoned_building_encounter_qa` now walks into
all nine known collision records across those eight sites and checks the
resulting ROM event and destination scene. In the natural Puyallup marker-12
entry to scene 52, it leaves the clock untouched: the field counter reaches
1034 (phase 10) after 973 further field updates. The following live field
update evaluates that due opportunity. With a reproducible seed selected
before that update, the production path rolls `0xA47A < 0xC000` and spawns five
ghoul actors. Separate seeded
production-trigger cases cover the scene-50 Bandersnatch list and ghoul lists
in scenes 51-53. The test writes four directly bootstrapped encounter BMPs,
plus immediate and 120-update captures from the physical Scene-52 entry.
Debug and Release artifacts have matching SHA-256 hashes.

The test seed makes the encounter result reproducible, so it verifies the
live cadence gate and spawn path but does not claim a user's unseeded PRNG will
spawn at the first opportunity. The directly bootstrapped captures have no
actors in the standard render projection and do not establish actor visibility.
The natural-entry images show one actor-sized sprite immediately after the
five-actor spawn, while the image after 120 neutral updates does not clearly
show an actor. At that point five actors remain active and their world
positions have changed; the QA player's projected y-coordinate is 278, below
the 224-pixel view. These internal Qt images do not establish Genesis visual
correctness. The handoff still does not say which site was tested by the user,
and the host clock's mapping to ROM `FFDF88` remains [H].

When the alarm is active, `0x1E1DA` selects the two-entry list at `0x1E20A`;
both entries reference template `0x1D6A1C`. `0x1E1E0` selects screen-edge
placement through `0x1E302`; ordinary outcomes use the scene's semantic event
content and map-derived placement path at `0x1E40C`. The shared spawn at
`0x1E25A` applies the common encounter actor setup.

This boundary keeps source addresses and arithmetic as provenance in the
native adapter and documentation. Lua applies the field encounter rules to
named state and normalized probabilities.
