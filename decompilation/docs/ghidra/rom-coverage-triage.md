# ROM routine coverage triage

This ledger classifies the 66 addresses in the historical B2 set. The prior
coverage snapshot reported 64 unmapped addresses; `0x1D7A` and `0x5D996` were
already mapped when this triage began, so they are included in the historical
set and marked as port owners. The complete address-by-address ledger is
[rom-coverage-triage.tsv](rom-coverage-triage.tsv).

`port owner found` identifies an active C++ or Lua owner. `replaced-by`
identifies a host service, authored layout, or extracted asset that takes over
the ROM mechanism. These are structural classifications and do not claim
runtime equivalence. `missing bug` is an unimplemented routine with source
behavior identified for a future focused fix.

## Required B2 owners

- `0x3B1E` and `0x3BAC`: the active field-camera helper owns axis-step
  classification and following a fixed overview target in
  [field_camera_conversion.cpp](../../ghidra_conversion/src/field/field_camera_conversion.cpp).
  The source trace and sampled headless measurements are in the B2 entry in
  `progress_notes.md`; they verify the sampled thresholds and steps, not the
  complete camera routine.
- `0x44BC`: actor setup is owned by
  [FieldSceneRuntime](../../ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp),
  with typed template conversion and Lua patrol behavior. The source trace and
  the production spawn-delay measurements are recorded in `progress_notes.md`.
- `0x1728`: the source sprite-stream conversion helper and offline extractor
  evidence are mapped. The game runtime uses named assets directly; this row
  does not claim that the helper is a runtime decoder or that its fixture
  alone proves visual equivalence.

## Purchase handler ports

### `0x24A1E` — ROM command `0xEB`, Heavy Combat Armor purchase

The dispatch-table entry at `0x12D060` reaches `0x24A1E`. There is no class
gate. The handler asks the shared slot scan at `0x22874` before calling wallet
helper `0x21A4E` to charge `0x7530` (30,000) from `FFFBFE`; a full inventory
returns descriptor `0x10D738` (text `0x10DD61`), while insufficient funds
returns `0x10D730` (text `0x10DCD6`: “You cannot buy what you desire with
something you do not have.”). Success returns `0x10D728` (text `0x10DC84`:
“The item you have requested is on its way.” followed by “May the Yakuza
guide you, Joshua-san.”), writes item `0x32` at the selected slot, and clears
its phase byte at slot `+8` (`0x24A4A`/`0x24A50`). The source choice at
`0x104234` reads “I'd like to buy the Combat Armor.” No story flag is written
by this handler. These rules are represented by
`E.BuildingPurchaseOffer.HEAVY_COMBAT_ARMOR` and
`building.purchase_heavy_combat_armor`; the typed host service applies the
slot/funds/item transaction. The choice's caller path remains unproven.

### `0x24A56` — ROM command `0xEC`, Power Focus phase-4 purchase

The dispatch-table entry at `0x12D064` reaches `0x24A56`. After comparing the
active runner class to 2 at `0x24A5C`, the `BMI` at `0x24A62` admits classes 0
and 1 (Mage/Shaman); classes 2 and 3 return `0x110A22` (text `0x1109B1`,
“This little item isn't going to do you any good. Maybe one of your magically
inclined chums could call me back.”). Eligible classes then use the shared
slot scan at `0x22874` before wallet helper `0x21A4E` charges `0x13880`
(80,000) from `FFFBFE`. Full inventory returns `0x10D738`/`0x10DD61` (“I am
so sorry, Joshua-san, but this transaction must wait.”); insufficient funds
returns `0x10D730`/`0x10DCD6` (“You cannot buy what you desire with something
you do not have.”). Success returns
`0x10D728`/`0x10DC84`, writes item `0x18`, and writes phase 4 at the selected
slot's `+8` (`0x24A9A`/`0x24AA0`). The offer record at `0x10DBAD` names Power
Focus and its $80,000 price; the choice at `0x104256` reads “I'd like to buy
the Power Focus.” The available decline choice at `0x105101` reads “Neg on
that, chum.” No story flag is written by this handler. The Lua offer and class
gate are `E.BuildingPurchaseOffer.POWER_FOCUS_PHASE_4` and
`building.purchase_power_focus_phase4`; the typed host service applies the
slot/funds/item transaction. The choice's caller path remains unproven.

Headless `purchase_handlers_tests.lua` covers successful item/phase/wallet
writes, insufficient funds, inventory-full priority, class rejection, and
unchanged unrelated flags. The ROM source trace and before/after registry
measurement are recorded in `/workspace/shadowrun/out/progress_notes.md`.
