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
per VBlank. After the final 60-frame zero, it runs the five-step fade at two
VBlanks per step, then holds the black transition for the remaining 54 frames
before logging out. The host uses a full-black frame during that final display
handoff because the original transition artwork is not ported. The panel
rebuild clears the previous actions and selector before drawing the shutdown
screen, and the fade overlay also covers the special countdown renderer.

Debug and Release `shadowrun_ghidra_gameplay_script_menu_route` CTests check
that each value remains visible for 60 frames and save:

- `matrix-node-action-route-qa/shutdown-countdown-3.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-2.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-1.bmp`
- `matrix-node-action-route-qa/shutdown-countdown-0.bmp`
- `matrix-node-action-route-qa/shutdown-logout-fade-start.bmp`
- `matrix-node-action-route-qa/shutdown-logout-fade-black.bmp`

The four countdown frames and both transition frames were reviewed. The
message rows remain fixed while the digit changes; the final fade covers the
countdown and leaves the frame fully black. No old button or selector pixels
remain. These are internal Qt route artifacts, not independent Genesis
captures.

## Remaining integration

The route now reads and commits `ScriptMatrixWorldState` through typed Lua host
bindings. Its test supplies a known active run (`state=-1`, `objective=-1`,
completion bit 0 set) and verifies the completion writes. The production
runtime still has no active-run provider. The field runtime stores the
committed flags, but security/maglock writes are not yet consumed by guard or
door behavior or included in save data; only alarm-off actor `+0x9E` updates
are currently connected. Countdown, fade, and logout timing are connected;
the host’s black hold is a presentation substitute and the source Matrix
logout artwork remains open. The Crash System hill remains `partial`.
