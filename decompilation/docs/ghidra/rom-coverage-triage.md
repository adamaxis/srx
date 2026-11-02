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

## Missing bugs

### `0x24A1E` — ROM command `0xEB`, Heavy Combat Armor purchase

The source branch at `0x24A1E` checks for an available inventory slot, charges
30,000 nuyen, and writes item `0x32` at phase 0. The choice text is present at
`0x10D70C`, but the purchase handler and caller path have no port owner. This
is recorded as a missing bug, not implemented as part of B2 triage.

### `0x24A56` — ROM command `0xEC`, Power Focus phase-4 purchase

The source branch at `0x24A56` requires class 0 or 1, an available inventory
slot, and 80,000 nuyen, then writes item `0x18` at phase 4. The choice text is
present at `0x10D70C`, but the purchase handler and caller path have no port
owner. This is recorded as a missing bug, not implemented as part of B2
triage.

Both source traces and the missing-handler finding are also recorded in the
B2 section of `/workspace/shadowrun/out/progress_notes.md`.
