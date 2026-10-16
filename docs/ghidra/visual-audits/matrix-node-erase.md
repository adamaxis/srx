# Matrix datastore Erase

**Status:** partial. The Lua action, typed run-state writeback, message layout,
and return-to-node path are connected. The production host still has no live
active-run provider, and there is no independent Genesis capture for these
messages.

## ROM trace

Primary source: `input/shadowrun_genesis_us.bin`.

- The datastore action dispatch reaches `0x5559E` after the shared node-action
  gate at `0x548A0`.
- `0x5559E` redraws the node, requires `FFF0E3 < 0`, compares the current
  node's class byte with `FFFBF2`, and requires `FFFBF6 == 0` for the objective
  branch.
- The objective branch clears bit 0 of `FFFC28`, sets `FFF0E3 = 1`, sets bit 7
  of `FFFC1C`, and prints the stream at `0x1668CE` to `$C594`.
- Every other case prints the stream at `0x16685A` to `$C594`.
- Both paths jump to `0xD94`, the any-input-edge wait, and then return to the
  current datastore node.
- The exact ROM streams are:
  - `0x1668CE`: `  The recog-code| rapidly finds the|data and zaps it to| electron heaven!!|| Run accomplished!`
  - `0x16685A`: `You find a file that| appears important| and obliterate it|  from memory!!!|| Score one for the|downtrodden hacker!`

## Host route

`matrix_node_action.lua` runs Erase through the same Lua runner gate as the
other node actions. It reads the optional typed Matrix run snapshot, applies
the ROM objective condition, commits the resulting state, and displays the
decoded text. If the host does not yet know an active run, Erase takes the
ordinary text path and does not create or complete a run.

## Verification

`shadowrun_ghidra_gameplay_script_menu_route` covers the objective and ordinary
branches through system 0's ROM datastore nodes, including the any-input return
and the route back to the original slave module. The Lua rules test checks the
same run/objective condition and both ROM text addresses. Debug and Release
artifacts are saved as:

- `build/conversion/qt/{debug,release}/qt_runtime/matrix-node-action-route-qa/erase-objective-message.bmp`
- `build/conversion/qt/{debug,release}/qt_runtime/matrix-node-action-route-qa/erase-file-message.bmp`

The four generated frames were inspected. Each message replaces the datastore
action panel, preserves the ROM blank-line break, and contains no unpainted
previous-screen pixels. These internal Qt artifacts do not establish
independent Genesis visual fidelity.
