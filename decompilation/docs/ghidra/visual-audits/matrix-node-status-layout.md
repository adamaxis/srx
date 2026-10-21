# Matrix node status layout

## Source trace

The active node action loop starts at ROM `0x54662`. Its draw sequence at
`0x547E2` selects the current function record and renders the function title
at name-table address `$C418` (`0x54802`). It draws the action prompt from
`0x166696` at `$C596` (`0x54816`), then calls the status renderer `0x54A28`.

`0x54A28` performs three separate writes:

| Field | ROM data | Name-table address | 64-cell plane pixel origin |
|---|---|---:|---:|
| File/node class | `0xE4060 + node[+1] * 16` | `$C39A` | `(104, 56)` |
| Alert band | `0xE30B6 + node[+5] * 8` | `$C51E` | `(120, 80)` |
| Action difficulty | `node[+6]` | `$C52C` | `(176, 80)` |

The title address `$C418` maps to `(96,64)`. These coordinates use a 64-cell
name-table row of `0x80` bytes: one tile row is 8 pixels. The ROM label tables
contain padding and the alert-band strings include their trailing hyphen, so
Lua retains those exact strings and the number is drawn separately after the
hyphen.

## Change

The prior Qt panel flattened file class, alert band, and difficulty into
`top_text` and drew them together under the panel header. The Lua node route now
publishes a typed status table; the host snapshot carries the three values
separately, and the presenter uses the name-table positions above. The action
choices were moved below the status area so the selected-row background does
not paint over the alert band or difficulty.

## Verification

- Debug CTest: `shadowrun_ghidra_gameplay_script_menu_route` passes.
- Dump: `build/conversion/qt/debug/qt_runtime/matrix-node-action-route-qa/node-entry-screen.bmp`.
- The complete 320x224 dump was inspected. Title, file class, band plus
  difficulty, and the action choices occupy distinct rows; the old compact
  stack under the top-left title is gone.
- The original System 0 CPU fixture was not a valid comparison for the
  longplay. The documented `matrix_node_id` seed was passed through C++ but
  discarded by the Lua screen. The route now forwards that seed and the
  optional IC condition into the typed Matrix session.
- A source-matched System 2, node 1 fixture writes
  `system-2-dead-projects-datastore.bmp`: Datastore, Dead Projects, Green-3,
  and the actions Leave Node / Transfer Data / Erase. This matches the screen
  content in longplay Section 21 frame `004620`. The complete 320x224 host
  frame was inspected beside the independent compressed frame; the panel
  geometry, rows, green grid, and labels align. The selected marker is now the
  ROM right-arrow tile `$5FF`: `0xC0C2` uploads the font beginning at `$5AB`,
  and `0xC376` writes `$85FF` for the selected row. Four offline assets use the
  security-color palette line selected at `0x566D8`; the host restores the
  tile's palette-zero transparency while compositing. This replaces the
  incorrect ASCII `>` marker. Debug and Release captures are pixel-identical,
  and the focused route CTest passes in both configurations. This remains a
  visual cross-check against compressed video, not a raw-pixel golden.

Status: partial. The full Matrix viewport and other node/status combinations
remain open.
