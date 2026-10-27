# Field encounter trigger facts

ROM entry `0x1E166` reads these gates before its cadence and probability
checks. The `[C]` facts below are checked against
`/workspace/shadowrun/disasm_full.s` at `0x1E166`; `[H]` identifies the host
frame fact used to model the source counter.

| Lua fact | Source | Host state | Evidence |
| --- | --- | --- | --- |
| `display_service_word` | `FFDFE8` word, nonzero blocks | `field_runtime_state_.display_service_word` | `[C]` Existing field runtime state names the display service word and is the state copied into the scene runtime. |
| `leader_protected` | `FFFC04` record `+0x11` bit 7, set blocks | High byte of the active leader record's `flags`, masked with `ActorStatusEx::UNSEEN` | `[C]` `0x1E170` loads the leader record pointer from `FFFC04`; `0x1E176` tests byte `+0x11`, bit 7. |
| `field_activity_lock_active` | `FFE18B` byte, nonzero blocks | `field_runtime_state_.field_activity_lock` | `[C]` `0x1E180` tests this byte. |
| `field_alarm_active` | `FFE16F` byte | `field_matrix_field_state_.alarm`, the Lua field alarm state | `[C]` `0x1E18A` gates the combat-active check and `0x1E1D0` selects the alternate list when set. |
| `combat_active` | `FFDF84` word, nonzero blocks only while alarm is clear | `world_frame_logic_state_.world_active` | `[C]` `0x1E194` tests this word after the clear-alarm branch. |
| `encounter_actor_count` | `FFE177` byte, nonzero blocks | `field_world_activity_timeout_state_.encounter_actor_count` | `[C]` `0x1E19E` tests this byte. |
| `frame_counter` | `FFDF88` word, low 10 bits must equal 10 | `field_world_frame_counter_` | `[H]` Host field-frame counter increments once in the field frame service. It supplies the same cadence input to field actor and alarm services; the source RAM value is not stored as a separate host field. |

Once all gates pass, `0x1E1AE..0x1E1B2` applies mask `0x03FF` and compares
against 10. `0x1E1BC..0x1E1CA` calls `0xD44` and accepts only a draw below
`FFDE18`. With `FFE16F` set, `0x1E1DA` selects the longword list at
`0x1E20A`; its two entries both point to template `0x1D6A1C`, followed by a
zero terminator. `0x1E1E0` clears `D0` before the call to `0x1E216`. That
helper branches on `D0` and sends zero to the screen-edge candidate builder at
`0x1E302`; nonzero selects the map-derived builder at `0x1E40C`. The shared
spawn at `0x1E25A` applies the common encounter actor setup.

The source-derived gate facts and alarm table address live in
`decompilation/gameplay/scripts/enums.lua`; the active Qt runtime only supplies
them to `field_encounters.lua` and consumes its typed outcome.
