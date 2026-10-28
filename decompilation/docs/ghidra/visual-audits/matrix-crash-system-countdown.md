# Matrix Crash System countdown

Audit date: 2026-10-22

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
  `$C518`, then prints the value 3 at `$C722`. The `0xC24` wait uses `D5=$3C`;
  its `DBF` loop performs 61 VBlank waits for each digit 3, 2, 1, and 0.
- It then calls fade `0x18776` and Matrix logout `0x26A00`.

The logout routine uploads five 8x8 patterns from ROM `0x15BE02` to the
center-tile graphics area. The corresponding name words at `0x15BEA2` select
sparkle `0x4B1`, dot `0x4B2`, horizontal line `0x4B3`, vertical line `0x4B4`,
and blank `0x4B5`. It lays the two lines across the view, replaces one edge
tile per VBlank for 14 ticks on both axes and six more on the horizontal axis,
then changes the center tile through dot, sparkle, dot, and blank holds of
11/6/6/11 VBlanks. The initial sparkle stays centered over the crossing lines.

The render positions use the 64-column name-table pitch (`$80` bytes per row):
`$C518` is tile `(12,10)` or pixel `(96,80)`, and `$C722` is tile `(17,14)`
or pixel `(136,112)`. The latter lands in the gap between the two hyphens in
`IN - - SECONDS`. The two `0x80` breaks in the text stream preserve one blank
row between its three text lines.

## Turn Off Node source trace

Primary source: `input/shadowrun_genesis_us.bin`, entry `0x54BD2`.

- It redraws the node-action grid, reads the current node record from
  `FF1100 + FFF118 * 0x14`, and dispatches on that record's `+1` class byte.
- Class `0x27` calls alarm-off and sets `FFE170`; `0x28` sets `FFE170`;
  `0x29` sets `FFE17F`; `0x2A` calls alarm-off.
- All four branches print the ROM text at `0x16684A` (`NODE IS OFFLINE`) at
  `$C61A`, then enter the any-button wait at `0xD94`.
- Alarm-off `0x1E09A` clears `FFE16F` only when set. In that branch it visits
  the seven actor records from `FF0400` in `0x100` steps, setting `+0x9E` bit 2
  for each record whose bit 1 is set. It always sends sound `0x30` through
  stop-resource service `0x530D2`.

The Lua route now reads the node class, applies these class-specific changes,
commits field and actor facts through the typed host bridge, and uses a
separate stop-resource callback. Route QA reaches ROM system 0's class `0x29`
Slave Module and checks the maglock flag and clean `NODE IS OFFLINE` panel.
Rules QA covers all four classes and the conditional actor-flag update. The
field host currently exposes only its active typed actor records; unrecovered
raw records remain a boundary.

## Qt route check

The live debug-menu route enters system 0, follows its first link to CPU, runs
Crash System through the shared action gate, and advances the countdown once
per VBlank. After the final 61-frame zero, it runs the five-step fade at two
VBlanks per step, then paints the ROM's line-art sequence during the 54-frame
display handoff before logging out. Lua publishes the phase and line bounds;
Qt presents the native layout and the extracted center-tile pixels. The panel
rebuild clears the previous actions and selector before drawing the shutdown
screen, and the fade overlay also covers the special countdown renderer.

Debug and Release `shadowrun_ghidra_gameplay_script_menu_route` CTests check
that each value remains visible for 61 frames and save:

- `matrix-node-action-route-qa/shutdown-countdown-3.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-2.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-1.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-0.bmp`
- `matrix-node-action-route-qa/shutdown-logout-fade-start.bmp`
- `matrix-node-action-route-qa/shutdown-logout-cross-full.bmp`

The four countdown frames and transition frame were reviewed. The message rows
remain fixed while the digit changes; the final fade removes the countdown and
reveals the full cross, including its center sparkle. The independent Genesis
sequence in Section 15 frames `005420`, `005427`, `005430`, `005435`, `005440`,
and `005445` shows the same cross sweep, center dot, sparkle, dot, and black
tail. Thresholded white-pixel mask IoU is 0.998 for the full cross, 0.992 for
the collapsed cross, 1.000 for both dot frames, 0.957 for the sparkle, and
1.000 for the black frame. The one-pixel differences in cross/sparkle masks are
consistent with video compression. Full 320x224 Debug/Release QA captures
each native phase and checks the exact ROM tile masks and source-derived line
bounds.

## Remaining integration

The route now reads and commits `ScriptMatrixWorldState` through typed Lua host
bindings. Its test supplies a known active run (`state=-1`, `objective=-1`,
completion bit 0 set) and verifies the completion writes. The production
runtime still has no active-run provider. The field runtime stores the
committed flags, but security/maglock writes are not yet consumed by guard or
door behavior or included in save data; only alarm-off actor `+0x9E` updates
are currently connected. Countdown, fade, and the full Matrix logout artwork
are connected and checked against the independent longplay. The Crash System
hill remains `partial` because the production run provider and field-effect
consumers are still missing.
