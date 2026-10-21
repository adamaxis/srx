# Matrix Storage screen

## ROM trace

Storage entry `0x18D1A` calls the shared right-panel redraw at `0x1897C`,
which preserves the selected Command row, installed-program grid, and footer.
It then writes the panel stream at `0xE3A07` to Plane A `$C126`, reads total
storage from the active deck record at `FFFBBE + 4` into `$C244`, reads
available storage at `0x19558` into `$C2C4`, and writes the option stream at
`0xE38BA` to `$C536`. The text writer `0xC0DA` treats `0x80` as a one-row
advance.

The visible stream rows and their source-derived pixel positions are:

| Text | Pixel origin | Source |
|---|---:|---|
| `Storage` | `(200,16)` | `0xE3A07`, six leading spaces |
| bracket and eight backslashes | `(200,24)` | `0xE3A07` |
| `Total  . . . . . Mp` | `(152,32)` | `0xE3A07` |
| `Available. . . . Mp` | `(152,40)` | `0xE3A07` |
| `DataFiles` | `(192,56)` | `0xE3A07`, five leading spaces after the blank row |
| bracket and nine backslashes | `(192,64)` | `0xE3A07` |
| `View` / `Erase` | `(216,80)` / `(216,96)` | `0xE38BA` |

The cursor base is `$C532`, `(200,80)`, with the default at View. `$C244` and
`$C2C4` are the rightmost digit anchors `(272,32)` and `(272,40)`. The tested
active deck contains 1000 total and 973 available storage units.

View at `0x18F5E` resets the occupied-file counter, then visits records 4, 3,
2, 1, and 0. Occupied records use the fixed 0x14-byte ROM filename at
`0xE3A85 + id*0x14`, the separator stream at `0xE3FFD`, and the size written by
`0x1954A`. The list starts at `$C4A6` (name row `(152,72)`, separator/size row
`(152,80)`) and advances `$100` per occupied record; Start returns to Storage.
With no occupied files, the `0xE3A6C` message is written at `$C52C`, producing
`No datafiles` at `(176,80)` and ` in storage` at `(176,88)`, then returns to
the deck.

Erase starts at the highest occupied slot. The prompt writes its filename at
`$C526`, separator at `$C5A6`, size at `$C5C4`, Yes/No at `$C6B6`, and the
default-Yes cursor at `$C6B2`. No scans downward from the current record and
wraps through the five slots. Yes clears the selected record's type, size, and
value at `0x18F2A`, recalculates capacity, and returns through View; Start
skips the prompt to the file list.

## Native implementation and evidence

Lua owns the option strings, source-positioned panel lines, capacity, file
ordering, prompt selection, and deletion state. C++ publishes typed fields to
the Qt presenter, which retains the parent deck row, grid, and footer. The
`shadowrun_ghidra_gameplay_script_menu_route` suite checks view ordering,
Start/B behavior, erase cycling, deletion, and the empty return. The
`shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` test checks the
source-positioned text rows and exercises all routes, then writes six complete
frames under `qt_runtime/field-matrix-cyber-info-qa/deck-storage*/actual.bmp`.

Debug and Release route and screen CTests passed, along with all pure-Lua
gameplay suites. I inspected all six full Debug frames; each is 320x224, and
all six Release captures have identical SHA-256 hashes. This is source-position
and internal regression evidence. An independent Genesis capture of the
Storage screen was not available for comparison.

## Remaining work

The complete Matrix/Cyberdeck hill remains partial: the Storage route is now
implemented and tested, but independent Genesis comparison for this screen
and broader Matrix traversal/action coverage remain open.
