# MicroTronics no-deck gate audit

**Status:** Partial. The Scene-5 event-4 route now follows its welcome prompt
into the ROM's no-deck message and returns to the field on input. The
deck-present catalog and actions remain unresolved. Host screenshots are
regression artifacts; an independent Genesis capture has not been identified.

## ROM trace

Scene-5 event 4 is intercepted by `0x1FA78`, which selects MicroTronics with
mode 6 and result 2. The shared field-service loop at `0x56AC0` loads the
service presentation and active-runner HUD, then checks result 2 at `0x56BF4`.
If bit 0 of `FF018F` is clear, it draws the `STORE CLOSED!` text at `0x1AA3B4`
to name-table word `0xC316`, waits for any controller event through `0xD94`,
and restores the field assets through `0x334C`, `0x39F2`, and `0x3404` before
returning. With bit 0 set, execution continues into the still-unimplemented
MicroTronics action menu.

The host checks the typed `FF018F` counterpart after the MicroTronics welcome
prompt. Lua resolves the message key through the shared field-mode content
selector, and the native field-service presenter shows it over the shared
service background. A fresh game starts with bit 0 clear. Restored saves keep
the fact unknown, so the host reports an explicit boundary instead of
guessing.

## Route and captures

`shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa` exercises
Scene-5 event 4 from the fresh-game state. It checks the welcome prompt,
`shop.store_closed`, the absence of an open shop menu, and dismissal back to
the same field scene. Debug and Release both pass, and their gate and return
images have matching SHA-256 hashes.

Release images:

- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/microtronics/store-closed/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/microtronics/field-return/actual.bmp`

These captures confirm the host path and regression frame only. They do not
establish Genesis screen fidelity. Wire-Masters' result-1 upgrades and the
deck-present MicroTronics result-2 action branches remain open.
