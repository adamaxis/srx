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

## Native implementation and evidence

Lua owns the option strings, source-positioned panel lines, and live capacity
values in typed Matrix menu state. The Qt presenter keeps the parent deck row,
grid, and footer while drawing only the Storage right panel. The
`shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa` test checks the
typed route state, every panel/option row, capacity, retained program icon,
and full 320x224 frame, then writes
`qt_runtime/field-matrix-cyber-info-qa/deck-storage/actual.bmp`.

Debug and Release CTests passed for the route and screen QA. I inspected both
complete captures; their SHA-256 hashes match. This is source-position and
internal regression evidence. An independent Genesis capture of the Storage
root was not available for comparison.

## Remaining work

The View route must present the reverse-ordered datafile list and wait for
Start before returning to Storage. The Erase route must present the selected
file, size, Yes/No choice, deletion result, and ROM-timed return behavior.
These child flows are not covered by this root-screen patch, so `0x18D1A`
remains partial.
