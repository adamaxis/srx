# Projected terrain marker runtime connection

**Status:** partial. The typed field runtime now applies the selected
projected-terrain cell as a read-only collision-query override and maps an
accepted Lua `package_recovery` run to the ROM's `FFFBED == 4` selector gate.
It does not rebuild the full field collision map at runtime.

## ROM trace

**[C]** World service at `0x029BC` calls `0x0EE10`. `0x0EE10` returns unless
`FFDF8A` is set, clears that request, clears the 4096-byte `FF1700` collision
field, rasterizes the field rectangles, and calls `0x0EE86` for its negative
terrain-table entry (`disassembly.s:1654`, `7017-7054`).

**[C]** `0x0EE86` requires field mode `FFDF40 == 2`, either `FFE184 != 0`,
`FFE187 != 0`, or `FFFBED == 4`, equality between selected `FFE12C` and active
`FFF0C8`, and clear `FFE180`. It sign-extends `FFE185`, selects that occurrence
of terrain marker `0xDA`, and replaces it with `0x9A`
(`disassembly.s:7055-7070`). `0x16964-0x169D4` initializes the selected index
and ordinal and applies the `FFE184`, `FFE187`, and `FFE180` overrides.

**[C]** ROM `0x1E69C` writes the selected handler to `FFFBED` and dispatches
through `0xF3DC6`; the Current Run dispatcher at `0x1E72C` uses table `0xF3DFA`,
whose handler-4 entry is package recovery at `0x1E87A`. Lua resolves source
package-recovery offer variants to the named run ID `package_recovery`. The
field runtime uses that accepted run identity as a boolean selector fact and
refreshes the cached cell when a dialogue task returns control to field
movement.

**[C]** Event `0x1A` at `0xB90E-0xB934` distinguishes raw `0xDA` from other
codes: `0xDA` skips setting `FFE178`, while a non-`0xDA` cell can set it when
`FFFC1C.7` is clear. `0x1E43C-0x1E44E` and `0x2A64-0x2A76` clear the effect
selector flags. These facts are the basis for preserving the selector facts
across scene loads and clearing them at those typed runtime transitions.

## Host connection and QA

The host stores the override flags, selected ordinal, active/selected resource
indices, field mode, and the chosen cell as typed state. Its collision lookup
checks the chosen cell after the higher-priority field-barrier overlay and
returns the selected marker behavior without mutating the imported terrain
map. The cell is refreshed when field mode, resource selection, elevator
floor, world-effect state, or a completed dialogue changes its selector inputs.
Runtime does not copy the source map or send marker bytes through Lua.

The headless `shadowrun_ghidra_conversion_qt_field_world_activity_qa` exercises
Scene 26's imported collision map. Its original case checks selector inputs
`FFE12C == FFF0C8 == 2`, ordinal 3, and `FFE187 != 0`, then verifies the live
collision query changes only the selected marker, `FFE180` suppresses it, and
the imported map stays unchanged. An added case seeds an accepted
`package_recovery_two_site_pool` offer, uses mode 2 and matching effect indices,
and verifies only ordinal 3 changes through the run-identity gate. It checks
all marker cells and the unchanged source map, and writes
`qt_runtime/{debug,release}/field-world-activity-qa/package-recovery-marker-actual.bmp`.
The marker is collision state; the dump does not independently prove its
gameplay effect.

Debug and Release headless CTests pass for the Scene-26 runtime case and the
standalone terrain-map selector.

Both selector cases use QA-seeded state. The accepted package-recovery gate is
connected through semantic Lua run state, but an ordinary-input route through
that offer and its collision effect has not been captured here. Complete
`FFDF8A` collision-map rebuild behavior and an independent Genesis collision
or gameplay capture remain open. The `playtest_2026-10-25.md` and
`progress_notes.md` paths named by the older handoff are unavailable in this
checkout; this note and `HILLS.md` record the current progress instead.
