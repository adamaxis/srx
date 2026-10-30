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

When the alarm is active, `0x1E1DA` selects the two-entry list at `0x1E20A`;
both entries reference template `0x1D6A1C`. `0x1E1E0` selects screen-edge
placement through `0x1E302`; ordinary outcomes use the scene's semantic event
content and map-derived placement path at `0x1E40C`. The shared spawn at
`0x1E25A` applies the common encounter actor setup.

This boundary keeps source addresses and arithmetic as provenance in the
native adapter and documentation. Lua applies the field encounter rules to
named state and normalized probabilities.
