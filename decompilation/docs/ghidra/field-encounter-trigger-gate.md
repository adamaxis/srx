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

## Puyallup abandoned-building encounter audit

Puyallup scene 1 has three separate abandoned-building routes: collision
records `0x17D998`, `0x17D9F2`, and `0x17DA10` resolve through the event table
to scenes 50, 51, and 52 respectively. The native route rows are in
`gameplay/scripts/data/world_links_data.lua` at lines 47, 56, and 59. This
distinction matters: testing a bootstrapped scene does not exercise the door
entry state.

The scene-profile bases are `0x1E8D3C`, `0x1E911C`, and `0x1E94DC`; word 41
(byte offset `0x52`) is `0xC000` at ROM addresses `0x1E8D8E`, `0x1E916E`, and
`0x1E952E` [C]. With the strict unsigned comparison at `0x1E1BC..0x1E1CA`,
that threshold accepts 75% of source random words. The ROM increments
`FFDF88` in `wait_for_vblank` at `0x0BD2` and checks its low ten bits at
`0x1E1A8..0x1E1B6`; one opportunity is scheduled every 1024 counter ticks.
At 60 ticks per second that is about 17.1 seconds [H: native clock mapping and
host cadence].

The Debug and Release CTest
`shadowrun_ghidra_conversion_qt_abandoned_building_encounter_qa` enters all
three routes with movement input. In scene 52 it reaches phase 10 without
setting the clock (`counter=1034` after 973 `advance_frame()` calls), then a
reproducible test seed on that due update reaches the production actor-spawn
path (five actors). Separate controlled dispatch cases still cover the
actor-bearing lists in scenes 50-53. This confirms that the native physical
routes and scheduled encounter path work for a successful sample. It does
not prove an arbitrary live RNG sequence will spawn an encounter on the first
opportunity, and the native clock's mapping to ROM `FFDF88` remains [H].

When the alarm is active, `0x1E1DA` selects the two-entry list at `0x1E20A`;
both entries reference template `0x1D6A1C`. `0x1E1E0` selects screen-edge
placement through `0x1E302`; ordinary outcomes use the scene's semantic event
content and map-derived placement path at `0x1E40C`. The shared spawn at
`0x1E25A` applies the common encounter actor setup.

This boundary keeps source addresses and arithmetic as provenance in the
native adapter and documentation. Lua applies the field encounter rules to
named state and normalized probabilities.
