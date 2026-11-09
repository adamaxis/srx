# MicroTronics service gate and cyberdeck-model audit

**Status:** Partial. Scene-5 event 4 covers the no-deck gate, the deck-present
Cyberdecks category, and the four typed cyberdeck-stat upgrades. The model and
stat paths have purchase confirmation, wallet debit, deck-field updates,
limit rejection, and return behavior. Four other MicroTronics categories
remain explicit boundaries. The host BMPs are regression artifacts; an
independent Genesis shop capture has not been identified.

## ROM trace

Scene-5 event 4 is intercepted by `0x1FA78`, which selects MicroTronics with
mode 6 and result 2. The shared field-service loop at `0x56AC0` loads the
service presentation and active-runner HUD, then checks result 2 at `0x56BF4`.
If bit 0 of `FF018F` is clear, it draws the `STORE CLOSED!` text at `0x1AA3B4`
to name-table word `0xC316`, waits for any controller event through `0xD94`,
and restores the field assets through `0x334C`, `0x39F2`, and `0x3404` before
returning. With bit 0 set, `0x587DA` draws the mode-6 model availability rows
from `0x1AB196` (the non-mode-6 callers use the adjacent `+0x70` table). At
`0x5898A`, eight 14-byte row markers select the ordinary `0xCE29A` grid or the
blank grid; the model table has three positive rows followed by blank rows.
The selected Cyberdecks action reaches `0x573FC`/`0x57412`. It compares the
current signed model index with the selected row at `0x5742A..0x57446`, checks
the row's 32-bit cost against the wallet at `0x57466..0x57472`, and uses the
shared confirmation at `0x57492..0x574C8`. On acceptance, `0x574CC..0x57518`
debits the cost, replaces MPCP, hardening, load/I/O, response and model index,
and only raises memory/storage when the row's capacity is larger.

The `cyberdeck_upgrades` category reaches `0x5766A`. Cursor 0 checks memory
against signed-word `MPCP * 0x28` at `0x576F4..0x576FC`; cursor 1 checks storage
against signed-word `MPCP * 0x50` at `0x576AE..0x576B6`; cursor 2 checks the
low byte of `MPCP * 0x0A` against signed-byte load-I/O at `0x57702..0x5770A`.
Each of those paths uses `BLE` for its source cap test. Cursor 3 compares the
response byte with 3 at `0x576CE..0x576D4` and branches on the subtraction's
N flag alone (`BMI`), so the native transaction preserves that wrapped flag
behavior instead of substituting a host clamp or normalized byte comparison.
The max messages are the ROM strings at `0x1AA50E` and `0x1AA4DD`. If the
selected stat is below its source limit, the price is read from
`0x1AC8E0[cursor]`; insufficient funds uses `0x1AA587`, and confirmation goes
through `0x57B8C` at `0x5776A`. Acceptance debits funds and increments memory
by 10 (`0x577A4`), storage by 25 (`0x577AC`), load-I/O by 5 (`0x577B4`), or
response by 1 (`0x5779E`). Since the ROM checks the current value before the
increment, it does not clamp an accepted increment to the computed limit.

The three rows in ROM order are model index / price / MPCP / hardening / memory
target / storage target / load-I/O / response: `0 / 5,000 / 4 / 1 / 50 / 100 /
20 / 0`, `1 / 25,000 / 6 / 2 / 100 / 500 / 20 / 1`, and `2 / 60,000 / 8 / 3 /
200 / 500 / 50 / 1`. If the current model is equal or higher, ROM text at
`0x1AA836` reports `Unable to trade "up" to this cyberdeck.`

The host checks the typed `FF018F` counterpart after the MicroTronics welcome
prompt. Lua resolves the message key through the shared field-mode content
selector, and the native field-service presenter shows it over the shared
service background. A fresh game starts with bit 0 clear. Restored saves keep
the fact unknown, so the host reports an explicit boundary instead of
guessing.

For the deck-present path, the same typed `FF018F` fact opens the Lua-owned
mode-6/result-2 catalog. Lua owns category navigation and the selected offer;
the native transaction adapter applies ROM-derived values to the typed
`CyberdeckRecord` and session wallet. The host selector labels model rows with
their model numbers and shows their typed properties. The four stat choices
use semantic host field names; those names are not claimed as ROM text. Neither
layout nor the `0xCE29A` source grid art has been compared with an independent
Genesis capture.

## Route and captures

`shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa` exercises
Scene-5 event 4 from the fresh-game state. It checks the welcome prompt,
`shop.store_closed`, the absence of an open shop menu, and dismissal back to
the same field scene. Debug and Release both pass, and their gate and return
images have matching SHA-256 hashes.

`shadowrun_ghidra_conversion_qt_field_microtronics_deck_upgrade_qa` seeds the
Test Deck, then follows ordinary menu input through the category and offer
panels. It checks declining model 0, purchases models 0/1/2 using both A and B,
wallet totals, exact hardware values, memory/storage preservation, and model-0
rejection after model 2. It also navigates to all four stat offers; checks a
declined and accepted memory purchase, storage-cap rejection, load-I/O and two
response upgrades, response-max rejection, insufficient funds, unchanged
state on declines/rejections, and Start return. Debug and Release CTest cover
these host behaviors. QA writes `actual.bmp` frames for the category, model and
stat offers, confirmations, results, limit/funds messages, and field return
under `qt_runtime/field-microtronics-deck-upgrade-qa/microtronics/`.

Release images:

- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/microtronics/store-closed/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/microtronics/field-return/actual.bmp`

These captures confirm the typed host behavior and regression frames only.
They do not establish Genesis screen fidelity. The remaining MicroTronics
combat, defense, mask/sense, and second cyberdeck-upgrade categories are still
unimplemented.
