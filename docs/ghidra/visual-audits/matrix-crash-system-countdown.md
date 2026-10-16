# Matrix Crash System countdown

Audit date: 2026-10-16

## ROM trace

Primary source: `input/shadowrun_genesis_us.bin`, entry `0x54B34`.

- The CPU action is entered only after the shared Matrix action gate at
  `0x548A0` succeeds.
- The run is marked complete only when signed `FFF0E3 < 0` and signed
  `FFFBF2 < 0`: clear `FFFC28` bit 0, set `FFF0E3 = 1`, and set `FFFC1C`
  bit 7. If either condition fails, this completion write is skipped.
- Every path calls alarm-off `0x1E09A`, sets field security `FFE170 = FF`,
  and sets maglocks `FFE17F = FF`.
- It redraws the node grid, renders text stream `0x166A44` at VDP address
  `$C518`, then prints the value 3 at `$C722`. The `0xC24` wait uses `D5=$3C`
  (60 frames); the loop updates the digit 3, 2, 1, 0, waiting 60 frames on
  each value.
- It then calls fade `0x18776` and Matrix logout `0x26A00`.

The render positions use the 64-column name-table pitch (`$80` bytes per row):
`$C518` is tile `(12,10)` or pixel `(96,80)`, and `$C722` is tile `(17,14)`
or pixel `(136,112)`. The latter lands in the gap between the two hyphens in
`IN - - SECONDS`. The two `0x80` breaks in the text stream preserve one blank
row between its three text lines.

## Qt route check

The live debug-menu route enters system 0, follows its first link to CPU, runs
Crash System through the shared action gate, and advances the countdown once
per VBlank. It returns from the menu after the final 60-frame zero. The panel
rebuild clears the previous actions and selector before drawing the shutdown
screen.

Debug and Release `shadowrun_ghidra_gameplay_script_menu_route` CTests check
that each value remains visible for 60 frames and save:

- `matrix-node-action-route-qa/shutdown-countdown-3.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-2.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-1.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-0.bmp`

All four Release frames were reviewed. The message rows and panel remain fixed;
only the digit changes. No old button or selector pixels remain. These are
internal Qt route artifacts, not independent Genesis captures.

## Remaining integration

The current debug route supplies an explicit no-active-run context because the
live Matrix game state is not yet connected to this menu route. The route
therefore verifies the message, timing, and return path, but does not exercise
an active run's completion conditions or persist the alarm/security/maglock
flags into the field world. The native fade service and source Matrix logout
presentation also remain open. The Crash System hill remains `partial`.
