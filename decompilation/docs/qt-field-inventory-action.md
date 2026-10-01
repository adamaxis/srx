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
  scaling is applied when a weapon changes.
- `0x12634`/`0x1275C` enter the source Yes/No discard prompt. No is selected
  initially; possession is cleared only after Yes, with equipped weapon or
  armor state cleared with it. The protected slot-seven item branch remains
  source-backed through `can_discard_selected_item`.
- `0x12836` remains the source trade eligibility gate. The current Joshua-only
  field bootstrap has no source-verified recipient roster, so a valid trade
  stops at an explicit recipient boundary rather than inventing one.

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
final discard mutation. It writes ten 320x224 BMP checkpoints outside the
asset tree. The matching CTest is
`shadowrun_ghidra_conversion_qt_field_action_qa`.

The general contract for this and future interactables is itemized in
`interactable-testing-methodology.md`.

This dump is an executable regression artifact, not an independent Genesis
visual golden. A source-independent longplay comparison is still required
before this hill can be called visually defended.

## Deliberate remaining boundaries

The common action row is now connected, but this does not claim the complete
field menu is finished. The following remain explicit source boundaries in the
Qt conversion:

- First Aid target selection/effect presentation (`0x12298`/`0x1253E`).
- Automatic-use acknowledgement and ordinary item-action handlers
  (`0x121C0` and the later item branches).
- Trade recipient discovery, capacity confirmation, and transfer presentation
  (`0x12836`/`0x129FE`).
- The separate Clips, Attributes/Skills, Cyberware, Magic, and Pocket
  Secretary command routes.

Those gaps are conversion-analysis work, not permission to replace the source
branches with invented Qt behavior.
