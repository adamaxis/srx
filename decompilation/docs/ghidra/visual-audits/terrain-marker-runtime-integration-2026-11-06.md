# Projected terrain marker runtime connection

**Status:** partial. The typed field runtime now applies the selected
projected-terrain cell as a read-only collision-query override. It does not
rebuild the full field collision map at runtime, and it does not yet publish
the `FFFBED == 4` gate into the marker selector.

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
floor, or world-effect state changes. Runtime does not copy the source map or
send marker bytes through Lua.

The existing headless
`shadowrun_ghidra_conversion_qt_field_world_activity_qa` exercises Scene 26's
imported collision map with selector inputs `FFE12C == FFF0C8 == 2`, ordinal 3,
and `FFE187 != 0`. It checks the fourth source marker through the live query,
checks every other marker remains unchanged, verifies `FFE180` suppresses the
override, and compares the complete imported map after all queries. The same
test writes the scene's existing full-screen
`actual.bmp`; the marker itself is collision state, so the screenshot cannot
prove its behavior.

Debug and Release headless CTests pass for the Scene-26 runtime case, the
terrain-map selector, and the world-activity selector-state conversion test.

The test uses a QA-seeded selector combination, not an ordinary-input route
that produces `FFE187`. The handler-index gate `FFFBED == 4`, complete
`FFDF8A` collision-map rebuild behavior, and an independent Genesis collision
or gameplay capture remain open. The `playtest_2026-10-25.md` and
`progress_notes.md` paths named by the older handoff are unavailable in this
checkout; this note and `HILLS.md` record the current progress instead.
