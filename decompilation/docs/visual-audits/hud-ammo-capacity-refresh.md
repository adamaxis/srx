# Field HUD Ammo Capacity Refresh Audit

Date: 2026-10-06
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Source path: `0x39F2` -> `0x38FA` -> `0x36FC`.
Native path: `FieldSceneRuntime::refresh_field_hud_runner_slot()` and typed
`advance_field_hud_ammo_meter()`.

## Source trace

- ROM `0x39F2` refreshes the field asset grid and three HUD panels, then calls
  `0x38FA`. That routine iterates the three runner records and calls the
  delay, behavior, and ammo asset services. The ammo call enters `0x36FC` with
  `D1=0` during this refresh.
- When the active record has an equipped item code below `0x0D`, `0x36FC`
  applies the delta to `+0x4E`, clamps it to the item's capacity at
  `D4540 + (item_code - 1) * 0x30 + 0x24`, then draws the counter. The active
  equipment and status gates come from `+0x10`, `+0x56`, and `+0x57`.
- Gator's source template at `0x72B1A` has item code 2 and raw `+0x4E=32`.
  Item row 2 at `0xD4570` has capacity 12. The first field refresh therefore
  changes the live count to 12 before the HUD snapshot is presented.
- Unequip and switch semantics are separate. `0x125FA` stores the equipped
  load as a percentage in `+0x4E`; `0x121E0` applies that percentage to the
  next weapon's capacity. The item-code-3 row at `0xD45A0` has an 11-round
  capacity (`+0x24` at `0xD45C4`). Four loaded rounds become 37% when unequipped
  (`ceil(4 * 100 / 11)`); re-equipping that 11-round item scales 37% back to
  four rounds. Capacity 12 also maps 37% to four by the source rounding rule.
  This makes `4/11 -> 37 -> 4` a fully reproducible source arithmetic path,
  but not proof that it was the user's exact weapon or save state. The shared
  reserve remains `+0x4F` and is tracked in `LOADOUT-01`.
- The normal unarmed field-HUD call does not print the saved percentage. ROM
  `0x36FC` arrives with `D2=0`; a clear +0x10 active bit, negative +0x56, or
  +0x56=`0x7F` selects table `0x6AC46` without loading +0x4E into D2. It draws
  the `00` pair while leaving the stored percentage untouched. Ricky's
  corrected QA state therefore retains +0x4E=100 after 6/6 unequip but shows
  `00` beside the hand. The reported 37-on-hand visual still does not match
  this normal source-style unarmed route and remains unresolved.
- There is no sound request in `0x39F2`, `0x38FA`, or `0x36FC`. The ammo
  counter is static between source refreshes and state changes; this path has
  no separate animation counter.

## Eight-question review

1. **Does it look correct? Partially.** The host now draws 12 for the Gator
   starter record after the source refresh. Independent section 06 frames
   `002550`, `002560`, `002580`, `002610`, `002640`, `002699`, and section 07
   frames `000030`, `000120` were reviewed as whole 320x224 images. They show
   neighboring HUD/inventory states, but no matching weapon load transfer.
   Independent visual confirmation of the complete reported sequence is not
   available.
2. **Does the look match behavioral state? Yes for the traced refresh.** The
   ROM-backed archetype assertion retains raw template value 32; the runtime
   QA checks the live record after refresh against the item's source capacity.
   Gator item code 2 displays 12. After same-item unequip, Ricky retains
   +0x4E=100 but the source unarmed HUD path draws 00. Unequipped percentage
   and equipped rounds are distinct source meanings; the reported visible 37
   is not yet reconciled with the normal unarmed draw path.
3. **Are the visuals correct? Yes for the counter and the reviewed panels.**
   The counter uses named extracted tiles for the clamped value. The related
   full-screen route retains the runner portrait, inventory cells, HUD,
   detail panel, posture strip, and correct item icon. The associated
   inventory outline was separately measured as 76 pixels: a one-pixel
   24x16 perimeter with no fill; see
   [`item-selector-and-unequip-review.md`](item-selector-and-unequip-review.md).
4. **Is it animated? No.** `0x36FC` synchronously updates the value and
   counter tiles. The 30-frame equip/unequip hold captures remain pixel
   identical; ammunition changes only when the source state/update path
   changes it.
5. **Is there a sound? No call traced** in the capacity refresh or counter
   drawing routines. Equip/reload action sounds belong to their separate
   controller paths and are not implied by the HUD refresh.
6. **Are the other visible parts of the screen also correct? Reviewed.** All
   28 current field-action full-screen Debug frames were inspected, including
   equip, cancel, selected/unselected actions, discard, equip/unequip on two
   weapon rows, reload confirmation, empty/reloaded magazine, and Clips
   return. I also inspected the other five older zoom/contact artifacts in
   the Debug output directory. The current selector is one pixel at source
   scale; no stale parent controls or extra black item box appeared in these
   route captures.
7. **Do multiple frames support the interpretation? Internally yes;
   independently only for neighboring states.** The Qt route covers multiple
   input moments across two weapon capacities, unequip percentage, reload,
   and return. Independent section 06/07 frames show multiple item/HUD states
   but do not show the weapon transfer itself. Do not use those adjacent
   frames as proof of the 32/37/4 sequence.
8. **Does new UI replace old UI cleanly? Yes.** This change adds no page or
   button. The inventory action row and Clips child replace the center panel;
   cancel redraws the parent command page pixel-identically. Shared HUD and
   posture remain visible.

## Validation and disposition

The previous `party-ricky-unarmed/actual.bmp` fixture was invalid: it copied
the raw save-roster template after the scene had clamped a different live
record, then forced the unequipped fields while retaining the template's 32
rounds. The fixture now copies Ricky's installed live record after the source
refresh, selects the equipped inventory item, and performs the typed same-item
unequip transition. QA checks the resulting `+0x4E=100`, no equipped item, and
the `00` asset drawn by the source unarmed branch while the stored percentage
remains untouched, plus the cleared prior icon, before saving the full-screen
frame. This repairs the fixture; it does not establish what appeared during
the user's exact route.

- **Code changed:** field HUD refresh now runs the typed zero-delta source
  clamp across the three live records before forming HUD slot state.
- **Behavior implemented:** initial over-capacity values are clamped to each
  active item's source capacity; item-clear writes from the source routine
  are retained.
- **Internally checked:** typed field-HUD CTest asserts 32 -> 12 and the
  correct counter tiles, plus unarmed 37 -> visible 00 with stored 37
  preserved (1/1 in Debug and Release); inventory-action CTest
  checks code-3 capacity 11 and the 4 -> 37 -> 4 unequip/equip round trip;
  Qt field-HUD/action CTests pass 2/2 in Debug and Release; graphics/catalog
  CTests pass 2/2 in Debug and Release. The graphics test asserts the ROM
  template still begins at 32 and item code 3 has capacity 11.
  Twenty-eight current Debug full-screen BMPs were reviewed and have
  byte-identical Release counterparts.
- **Independently visually checked:** section 06 frames `002550`, `002560`,
  `002580`, `002610`, `002640`, `002699` and section 07 frames `000030`,
  `000120`. They show whole-screen context, not the exact transfer.
- **Reported defect resolved:** only the initial 32-round over-capacity host
  display is corrected. The combined `32/37/4` report remains partial until
  normal-play state or matching independent footage identifies the observed
  weapon, capacity, and count meanings.
