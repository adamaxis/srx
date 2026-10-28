# Field taxi selector input audit

**Status:** ROM-backed direction edges and A/B destination confirms are
covered for all seven starting regions and destinations. The follow-up trace
also covers direction/confirm and A/B/C/Start priority, the C-button runner
switch callback, and Start's separate exit branch. Independent gameplay frames
with logged controller input remain unavailable.

## ROM trace

At `0x134D2`, the game initializes the cursor from the current region through
the byte table at `0xD4456`. `0x13726` draws the selected row through the
cursor-to-region table at `0xD445D`. After the outline draw, the selector loop
waits for VBlank, latches and samples the controller at `0x195C`. That sampler
writes new button edges to the high byte of `FFDD22` and held state to the low
byte. The selector uses byte-sized `BTST` operations at `0x135FC` onward, so
these checks read the high-byte edge bits:

- `0x135FC` / `0x1360C` / `0x13618`: Up (`0x0100`) or Right (`0x0800`) decrements the row,
  wrapping row 0 to row 6.
- `0x1362E` / `0x1363E` / `0x1364A`: Left (`0x0400`) or Down (`0x0200`) increments the row,
  wrapping row 6 to row 0.
- Direction groups run in order: one Up/Right decrement, then one Left/Down
  increment. Buttons are checked afterward, so direction plus A/B confirms
  the updated row. A (`0x4000`) precedes B (`0x1000`); both enter `0x20714`.
- C (`0x2000`) calls `0x391A`, then the loop also checks Start. Start
  (`0x8000`) takes the separate `0x10AF0` exit path without calling `0x20714`.

The extracted tables in `data/taxi_destinations.lua` map cursor rows to
`[6,4,2,5,0,3,1]`, and regions back to initial rows `[4,6,2,5,1,3,0]`.
The source capture's frames `001729`-`001735` and `001800` show the cursor and
destination captions in different states. They do not identify which physical
button caused a change.

## Mismatch corrected

`menu/screens/taxi_selector.lua` previously grouped Left with Up and Right
with Down. ROM source confirms these byte-sized checks read new-edge bits, not
the low-byte held state. The Lua screen now uses the high-byte masks and the
source direction order. It processes both direction groups before buttons,
routes C through the existing runner-reselect service, and lets Start exit
without opening the A/B cabbie dialogue.

## Validation and remaining scope

The embedded `gameplay_script_menu_route` CTest exercises all 49
current-region/destination pairs and confirms each destination with A and B.
It also sweeps all 256 subsets of the eight controller edge bits, pairing each
fresh-press subset with its held-only mask before release. The sweep checks all
four D-pad aliases, wraparound, direction/confirm order, simultaneous
opposite direction groups, C reselect, Start exit, and combined-button
priority. The field taxi screenshot
remains a host-rendered regression capture; the independent video confirms
selected states but has no controller log. The separate cab-map plate extent
discrepancy remains open.

The C-button action reuses the shared runner-switch service and remains a
host-state check rather than an independently captured Genesis state. The map
plate extent discrepancy and independent gameplay frames with logged inputs
remain open, so visual fidelity remains partial.
