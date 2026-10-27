# Wire-Masters mode-7 cyberware selector and purchases

## ROM trace

Scene-5/event 2 selects Wire-Masters through `0x1FA78`, then enters the
field-mode service with mode 7/result 1. `0x56AC0` prepares the shared field
service presentation through `0x10BC4` and `0x11048`. Its result-1 branch
also copies `0x5E0` bytes from ROM `0xCF97C` to VRAM `0x89A0`; this is the
cyberware illustration upload. The eight six-tile icon grids begin at
`0xD177C` and name tiles `0x44E` through `0x47B`. The mode palette uses the
first line at `0xD29D6` and the field palette tail at `0x18AC56`.

The selector uses the primary action cursor. ROM controller masks `0x0400`
and `0x0800` decrement and increment that cursor through `0x58C54`; masks
`0x0100` and `0x0200` address the panel cursor, which result 1 does not
advance. `0x56DC4` dispatches the selected action into the equipment-upgrade
path.

The eight mode-7 offers are datajack, cyber-eyes, hand razors, Spurs,
Smartlink, muscle replacement, dermal plating, and wired reflexes. Prices
come from the 32-bit rows at `0x1AB060`; essence thresholds are selected from
the byte table at `0x1AA8A0`. Installed cyberware flags live at runner record
`+0x90`; the source progression and current essence are read from that same
record. Spurs make hand razors unnecessary.

## Implementation and checks

`gameplay/scripts/shop.lua` owns the eight offers, labels, prices, and source
thresholds. The typed equipment-upgrade conversion applies the selected
cyberware bit, essence cost, and wallet change. The shared Lua menu protocol
handles selector input and transaction outcomes. Qt presents the selected
icon, name, description, price, current/preview essence, and eight ROM icons.
The offline graphics extractor uses the raw `0xCF97C` upload and `0xD177C`
icon grids to write individually named RGBA assets; the runtime does not
decode ROM graphics.

The Debug and Release CTests
`shadowrun_ghidra_conversion_qt_field_wire_masters_upgrade_qa`,
`shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa`,
`shadowrun_gameplay_script_runtime_qa`, and
`shadowrun_ghidra_shop_menu_runtime` pass. The dedicated Wire-Masters QA
checks the eight icon slots and selected icon against the empty service
background, moves the selector, checks the default-No confirmation, buys a
datajack, and exercises owned-item, Spurs-conflict, insufficient-essence,
insufficient-funds, and field-return states. All eleven named full-frame
captures have matching Debug/Release SHA-256 hashes; the complete 320x224
frames were inspected.

## Dermal Plating tier correction (2026-10-26)

The shop purchase path in `0x571B2` maps category 6 to flag bits 9 through 11.
At `0x5725C`, it starts with bit 9, advances to bit 10 or bit 11 when the
previous tier is installed, and stops once bit 11 is present. The conversion
previously initialized category 6 at bit 6, which is the Muscle Replacement
range, so a Dermal Plating purchase wrote the wrong runner-record flag. The
typed resolver now starts at bit 9 and preserves the source tier replacement
sequence.

`shadowrun_ghidra_field_equipment_upgrade` now checks purchases through bits
9, 10, and 11 and verifies the terminal already-owned result. Debug and
Release focused CTest runs pass. The dedicated Qt Wire-Masters interaction
still buys only a datajack, so the Dermal Plating cursor-to-confirmation UI
path and its displayed essence changes remain open for a separate runtime QA
case. This unit coverage establishes flag progression, not visual fidelity.

No matching independent Genesis Wire-Masters frame was identified in the
supplied longplay. The captures are host regression output, not an independent
visual golden; exact visual fidelity remains unclaimed.
