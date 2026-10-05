# Ghidra-conversion Qt field inventory action

This is the current executable hill for the converted Qt project. It is
separate from `shadowrun_native`, `shadowrun_qt`, and the Genesis bus. The Qt
runtime consumes the direct files under `decompilation/assets/`; it does not
read ROM addresses or emulate VDP/name-table state.

## Source behavior carried into the host

- `0x10D30` supplies the command-overview lifecycle and starts on selector 6.
- `0x11740`/`0x118C6` preserve the two-confirm inventory action loop. The
  first A edge opens the four-choice row; Left/Right wraps source choices 2
  through 5; the second A edge dispatches the selected choice.
- `0x12148` reads the selected two-column/four-row inventory cell from the
  typed runner record. Empty cells do not create a synthetic item.
- `0x12168` and `0x121E0` classify and mutate weapon/gear and armor state.
  Equipment is separate from possession, and the converted ammo-factor
  scaling is applied when a weapon changes. ROM `0x125FA` first saves the
  active weapon's remaining load as a percentage in `+0x4E`; `0x12256` scales
  that percentage to the selected weapon's `+0x24` ammo limit. The first
  division rounds up on any remainder; the second rounds half up. Equipping
  gear keeps the percentage in `+0x4E`, and equipping a later weapon converts
  it back to that weapon's rounds. `+0x4F` remains a runner-wide Universal Gun
  Clips count, rather than a per-weapon stock.
- `0x12634`/`0x1275C` enter the source Yes/No discard prompt. No is selected
  initially; possession is cleared only after Yes, with equipped weapon or
  armor state cleared with it. The protected slot-seven item branch remains
  source-backed through `can_discard_selected_item`.
- `0x12168` routes utility codes `0x13`, `0x14`, `0x16`, and `0x18..0x2B` to
  `0x121C0`. It renders the three D202F lines at C620 (`128,96` in the host),
  waits for any fresh controller edge through `0x0D94`, then returns without
  changing possession or equipment. The selected item outline remains visible
  while the message waits and clears when the source inventory row returns.
- `0x12298`/`0x1235C`/`0x12402`/`0x1253E` connect First Aid through the same
  selected-item route. Typed party flags/names build the target list; D-pad
  wraps through active runners and CANCEL; A alone confirms; the medkit/patch
  handler updates meter, status, item phase, and continuation; the result
  waits for the source event loop before returning to Inventory.
- `0x12836` rejects item codes `0x17..0x2B`; otherwise `0x1288E` scans the
  three-record ring after the active runner, skipping inactive records and the
  source runner. The Qt route consumes `FieldPartyRosterRuntime` records and
  returns without mutation when Joshua is the only active runner.
- `0x1296A` begins each recipient on No. Left/Right toggles Yes/No; confirm on
  No advances to the next active record. A full inventory displays the source
  multiline message and confirmation advances to the next candidate even if
  Yes is selected. Confirm on Yes transfers to the first free slot through
  `0x129FE`, carrying ordinary item phase, clearing armor phase, and removing
  equipped source state when the selected weapon/armor moves.
- The Qt dump installs diagnostic party records to reach full-recipient and
  transfer branches; this fixture does not invent normal-game party members.
  The eligible-recipient branch still lacks the source `0x110E8` recipient HUD
  panel, and no supplied independent capture shows the Trade screen.

The action background and focus blocks are direct extracted assets. The focus
blocks are source name-table blocks and are therefore loaded as opaque images;
they are not treated as transparent sprite overlays. Their source placement is
the reviewed action row at `(120,184)` with focus x positions
`128,152,176,200`.

The action confirmation strings are extracted from the ROM text tables into
`menu.field.text` alongside the existing command/menu records. No new runtime
text is synthesized for this path.

## Executable verification

The release executable has a deterministic diagnostic path:

```text
shadowrun_ghidra_conversion_qt.exe --assets decompilation/assets \
  --field-action-dump <output-dir>
```

It drives the real Qt field scene through Start, the source command selector,
the inventory action row, cyclic selection, cancellation/re-selection,
equip/unequip, combat-bar math, the discard choice, the Yes/No prompt, and the
final discard mutation. Its ammo subpath seeds two ROM-defined guns (item 1
capacity 6, item 2 capacity 12), then checks 3/6 -> 50 percent -> 6/12 and
back to 3/6 through real menu input. The clip reserve stays at 3 throughout.
This is a diagnostic state fixture, not a claim that the normal opening save
contains those two inventory entries. The same route reloads weapon 1 to its
six-round capacity, consumes one of the shared clips, checks the prompt's
No/Yes states, and verifies that Cancel removes all Clips controls before the
command page returns. The dump writes eighteen 320x224 BMP checkpoints
outside the asset tree, including weapon-switch, reload, unequip, and return
frames. The matching CTest is
`shadowrun_ghidra_conversion_qt_field_action_qa`.

The weapon mutation menu has no item-specific animation branch: its state and
panel redraw on the input edge. Each A dispatch requests source event `0x24`
(the mapped 2.484-second command-confirm clip). Clips row movement requests
event `0x25` (mapped to `audio.effects.resource.037`, 2.134 seconds including
its release tail); Clips confirmation requests `0x24`. Shared Qt audio QA
decodes and starts events `0x24`, `0x25`, and `0x4B` on the default output and
checks duration and stop behavior. Windows loopback capture for these exact
inventory actions remains unrecorded. The independent
longplay frames in section 06 (002550, 002560, 002580, 002610, 002640, 002699)
and section 07 (000030, 000120) support the shared inventory shell, item
selection, First Aid result, and return layout across repeated frames. None
shows a weapon-switch ammo transfer, so the percentage conversion is
ROM-derived and runtime-tested but still lacks an independent visual example.

First Aid has a separate end-to-end route and image dump:

```text
shadowrun_ghidra_conversion_qt.exe --assets decompilation/assets \
  --field-archetype samurai --field-first-aid-dump <output-dir>
```

`shadowrun_ghidra_conversion_qt_field_first_aid_qa` checks target/CANCEL,
B/C rejection, Medkit use/result/acknowledgement, source event IDs, record
updates, adjacent shell/HUD, and the selected-item outline. It writes
`actual.bmp` and named checkpoints for target, idle hold, CANCEL, cancellation
return, use result, and Inventory return. It confirms that held A does not
dismiss the result before a fresh controller edge. The test lowers the typed
physical meter to exercise the injured-target path.

Automatic-use items have a separate end-to-end route:

```text
shadowrun_ghidra_conversion_qt.exe --assets decompilation/assets \
  --field-automatic-use-dump <output-dir>
```

`shadowrun_ghidra_conversion_qt_field_automatic_use_qa` checks the extracted
three-line D202F text, item/detail/shell/HUD layers, the thin item outline,
unchanged item/equipment bytes, the source command-confirm event 0x24, a static
30-frame wait, held/released input, and return on a new B edge. It writes
`actual.bmp` and message/idle/held/released/return frames. Event 0x24 maps to the
existing 2.484-second audio file; the wait itself issues no additional sound
request. The supplied longplay shows repeated inventory and First Aid panels,
but no exact automatic-use message, so this scene remains partial pending an
independent capture.

Trade has a separate typed-state route and image dump:

```text
shadowrun_ghidra_conversion_qt.exe --assets decompilation/assets \
  --field-inventory-trade-dump <output-dir>
```

`shadowrun_ghidra_conversion_qt_field_inventory_trade_qa` checks no-recipient
return, active-runner ring order, full-recipient continuation, default No,
left/right choice state, ordinary item and phase transfer, source equipment
cleanup, the action shell, item details, the slim selector, and a 30-frame
static interval. ROM `0x1296A` loops at VBlank and redraws Yes/No only after a
horizontal edge; confirm requests event `0x24`, and horizontal movement
requests `0x25`. Event `0x24` has the existing mapped clip. Event `0x25` has no
verified alias or duration, and the callback assertion does not prove audio
playback. The ROM also invokes `0x110E8` to draw the chosen recipient's small
HUD panel; the current Qt screen has not projected that panel. The six BMPs
are internal regression checkpoints, not independent Genesis comparisons.

The general contract for this and future interactables is itemized in
`interactable-testing-methodology.md`.

This dump is an executable regression artifact, not an independent Genesis
visual golden. A source-independent longplay comparison is still required
before this hill can be called visually defended.

## Deliberate remaining boundaries

The common action row and Trade's typed selection/mutation path are connected,
but the complete field menu is not finished. Trade's recipient HUD panel
(`0x110E8`), independent visual reference, and mapped selector-movement sound
remain open.

The other command families have separate partial routes and are tracked in
`HILLS.md`; this inventory-action audit does not count them as finished.
