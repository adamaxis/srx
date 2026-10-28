# Field taxi selector input audit

**Status:** the ROM-backed single-direction controls and destination confirms
are now covered for all seven starting regions and all seven destinations.
Combined-button ordering, the C-button field action, and independent gameplay
frames with logged controller input remain open.

## ROM trace

At `0x134D2`, the game initializes the cursor from the current region through
the byte table at `0xD4456`. `0x13726` draws the selected row through the
cursor-to-region table at `0xD445D`. After the outline draw, the selector loop
waits for VBlank, latches and samples the controller at `0x195C`, then checks
the held low byte of `FFDD22`:

- `0x1360C` / `0x13618`: Up (`0x01`) or Right (`0x08`) decrements the row,
  wrapping row 0 to row 6.
- `0x1362E` / `0x1364A`: Left (`0x04`) or Down (`0x02`) increments the row,
  wrapping row 6 to row 0.
- A (`0x40`) or B (`0x10`) exits through `0x20714`.
- C (`0x20`) calls `0x391A`; Start (`0x80`) calls `0x10AF0` before the
  selected-mode handoff.

The extracted tables in `data/taxi_destinations.lua` map cursor rows to
`[6,4,2,5,0,3,1]`, and regions back to initial rows `[4,6,2,5,1,3,0]`.
The source capture's frames `001729`-`001735` and `001800` show the cursor and
destination captions in different states. They do not identify which physical
button caused a change.

## Mismatch corrected

`menu/screens/taxi_selector.lua` previously consumed edge bits and grouped
Left with Up and Right with Down. That made Right from Redmond advance to
Renraku, while ROM `0x1360C` decrements to Council Island. The field loop reads
held bits after drawing, so the Lua screen now uses the held low-byte masks and
the ROM's direction groups. A, B, and Start also use their held masks.

## Validation and remaining scope

The embedded `gameplay_script_menu_route` CTest exercises all 49
current-region/destination pairs and confirms each destination with A, B, and
Start. It also checks all four D-pad aliases, wraparound, and that a released
edge without a held state does not move the cursor. The field taxi screenshot
remains a host-rendered regression capture; the independent video confirms
selected states but has no controller log. The separate cab-map plate extent
discrepancy remains open.

ROM calls C-button handler `0x391A` from this loop. It selects a field entity,
plays sound `0x25`, and redraws actor/HUD state; that branch is not represented
by the taxi Lua screen yet. Simultaneous decrement/increment inputs and
direction-plus-confirm ordering also need a separate source-driven pass before
this screen can be called fully audited.
