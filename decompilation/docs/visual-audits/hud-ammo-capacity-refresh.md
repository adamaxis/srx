# Field HUD Ammo Display and Capacity Audit

Date: 2026-10-10
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Source path: `0x39F2` -> `0x38FA` -> `0x36FC`; equip/unequip: `0x121E0` / `0x125FA`.
Native path: `FieldSceneRuntime::field_hud_slot_state()` and typed
`advance_field_hud_ammo_meter()`.

## Source trace

- ROM `0x39F2` refreshes the field asset grid and three HUD panels, then calls
  `0x38FA`. That routine iterates the three runner records and services their
  delay, behavior, and ammo assets. The ammo call enters `0x36FC` with `D1=0`
  during this refresh.
- For an active equipped item code below `0x0D`, `0x36FC` applies the delta to
  `+0x4E`, clamps it to the item's capacity at
  `D4540 + (item_code - 1) * 0x30 + 0x24`, then draws the counter. Equipment
  and status gates use `+0x10`, `+0x56`, and `+0x57`.
- Gator's source template at `0x72B1A` has item code 2 and raw `+0x4E=32`.
  Item row 2 at `0xD4570` has capacity 12. The first field refresh therefore
  changes the live count to 12 before the HUD snapshot is presented.
- Unequip and switch semantics are separate. `0x125FA` stores the equipped
  load as a percentage in `+0x4E`; `0x121E0` applies that percentage to the
  next weapon's capacity. Item code 3 at `0xD45A0` has an 11-round capacity
  (`+0x24` at `0xD45C4`). Four loaded rounds become 37% when unequipped
  (`ceil(4 * 100 / 11)`); re-equipping that 11-round item scales 37% back to
  four rounds. Capacity 12 also maps 37% to four by the source rounding rule.
  This reproduces `4/11 -> 37 -> 4` source arithmetic, but does not identify
  the user's exact weapon or save state. Shared reserve `+0x4F` is tracked in
  `LOADOUT-01`.
- The normal unarmed HUD path does not print the saved percentage. `0x36FC`
  starts with `D2=0`; a clear +0x10 active bit, negative +0x56, or +0x56=`0x7F`
  selects table `0x6AC46` without loading +0x4E into D2. It draws `00` and
  leaves the stored percentage untouched. This is why the visible HUD value
  must be projected from the ROM display path rather than copied directly
  from +0x4E.
- The prior Qt projection passed raw +0x4E to the overlay compositor. When an
  unequipped weapon's retained percentage was below 64, the compositor treated
  it as an ammo-counter asset and drew it as bullets beside the unarmed icon.
  `FieldSceneRuntime::field_hud_slot_state()` now passes the live record facts
  through `advance_field_hud_ammo_meter(..., 0)` and stores that plan's
  `displayed` value as `ammo_display_value`. The stored percentage is not
  changed by this projection.
- There is no sound request in `0x39F2`, `0x38FA`, or `0x36FC`. The counter is
  static between source refreshes and state changes; this path has no separate
  animation counter.

## Nine-question review

1. **Does it look correct? Internally, yes for the reproduced state.** The
   full-screen `party-ricky-unarmed/actual.bmp` shows `00` beside Ricky's
   unarmed/cyberware icon while retaining the portrait, other party slots,
   field scene, and each slot's own visible counter. Independent section 06
   frames `002550`, `002560`, `002580`, `002610`, `002640`, `002699`, and
   section 07 frames `000030`, `000120` show neighboring HUD/inventory states,
   but not this same after-unequip state.
2. **Does the look match behavioral state? Yes for the tested transition.**
   QA starts with a source-valid one-round partial magazine, applies typed
   same-item unequip, and checks that +0x4E remains a nonzero percentage below
   64. The rendered counter remains `00`, matching ROM `0x36FC`'s unarmed
   branch. The old raw-value projection would have selected that percentage's
   nonzero counter asset.
3. **Are the visuals correct? Yes for the whole reviewed frame.** The HUD
   shows the unarmed icon, zero counter, runner portrait, companion panels,
   and field background together. The former weapon image clears. The
   inventory selector is a separate one-pixel 24x16 perimeter with no fill;
   its 76-pixel measurement is documented in
   [`item-selector-and-unequip-review.md`](item-selector-and-unequip-review.md).
4. **Is it animated? No.** `0x36FC` synchronously chooses the counter tiles.
   The counter does not animate on its own; it changes when the source record
   or refresh path changes it.
5. **Is there a sound? No sound call is traced** in the capacity refresh or
   counter drawing routines. Equip/reload action sounds belong to separate
   controller paths and are not implied by this HUD projection.
6. **Are the other visible parts of the screen also correct? Reviewed.** The
   complete 262-image Release field-player-action set was reviewed in 14
   chronological contact sheets, including menu/action returns and the field
   HUD around them. The 12 field-HUD QA images were also checked as complete
   screens. No stale controls, missing party panels, or unrelated drawing
   changes were found in this pass.
7. **Do multiple frames support the interpretation? Internally yes;
   independently only for neighboring states.** The field-HUD captures show
   equipped, active-runner, and unarmed states; the action route covers
   multiple input and hold frames. Independent section 06/07 footage has
   multiple nearby interface states, but does not show the exact weapon
   transfer. Those frames do not prove the reported 32/37/4 sequence.
8. **Is the timing supported? Yes for static counter behavior.** The 30-frame
   equip/unequip holds remain pixel-identical; this code has no counter
   animation cadence.
9. **Does the new UI replace old UI cleanly? Yes.** This fix adds no page,
   button, or panel. It corrects the value drawn into the existing HUD slot.
   QA checks that the old item image is cleared and that the percentage asset
   is not drawn over the unarmed icon or the existing HUD.

## Validation and disposition

The earlier `party-ricky-unarmed/actual.bmp` QA used a fully loaded state
whose retained 100% percentage was outside the counter asset range. That
fixture could not catch a percentage below 64 being mistaken for bullets. The
updated QA sets one loaded round, performs the typed same-item unequip, checks
the resulting nonzero percentage is below 64, installs that live record, and
checks the full-screen image uses the `00` counter asset rather than the
percentage asset. It also verifies the stored value is preserved, no weapon
remains equipped, and the previous item image clears.

- **Code changed:** HUD slot state now carries the rendered `0x36FC D2`
  projection as `ammo_display_value`; the overlay no longer interprets raw
  +0x4E as an always-visible round count.
- **Behavior implemented:** unequipped records retain their stored ammo
  percentage while their HUD displays `00`; equipped weapon counts still use
  the typed source capacity and display rules.
- **Internally checked:** field-HUD CTest proves an unarmed record keeps
  +0x4E=18 while projecting counter zero and the zero-counter asset. Qt QA
  proves the same behavior through a live partial-magazine unequip transition
  and checks the complete `actual.bmp`. Qt Debug and Release field-HUD and
  field-player-action suites pass 2/2 each; the workbench field-HUD CTest
  passes 1/1 in Debug and Release. The 12 Debug and Release field-HUD output
  images match byte-for-byte. The complete 262-image Release player-action
  set was reviewed; it is not claimed hash-matched to Debug here.
- **Independently visually checked:** section 06 frames `002550`, `002560`,
  `002580`, `002610`, `002640`, `002699` and section 07 frames `000030`,
  `000120`. They show whole-screen neighboring states, not the exact
  after-unequip transfer.
- **Disposition:** the stale percentage-as-bullets rendering is fixed in the
  Qt HUD path and covered by a source-valid regression. The user's exact
  visible 37-on-hand route remains unreconciled with ROM's normal unarmed
  `00` branch, so the combined `32/37/4` report stays partial pending a
  matching normal-play state or independent capture.
