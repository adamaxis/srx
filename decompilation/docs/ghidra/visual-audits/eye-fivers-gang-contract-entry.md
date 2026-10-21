# Eye-Fivers gang-contract entry

**Status:** partial. The Scene 3/event 4 contract-target diversion and the
ordinary Scene-13 fallback are physically covered. The later gang fight,
settlement, and independent Genesis visual comparison remain open.

## ROM route

Penumbra Scene 3 record 100 (`0x18ABBE`) is a distinct `0x00C4` collision
rectangle at x `0x04C0..0x04DF`, y `0x03A0..0x03BF`, publishing event 4. Event
table row 4 at `0x18A6FC` contains `0x0D`, the ordinary field Scene-13 route.
The guide's Penumbra marker 10 and contract-location index 4 cross-reference
this entrance as the Eye-Fivers site; the collision record itself has no
building name.

ROM `0x1F086` compares the current area/event (`FFFBF8` / `FFFBF9`) with the
accepted gang-contract target (`FFFBEE` / `FFFBF`). An exact area-3/event-4
match queues actor list `0x1D6D72` and resource profile `0x122018`, then opens
the entry passage from text `0xFE0DE` using descriptor `0xFDE26`. The pending
actor descriptors override row 4's Scene-13 result and prepare Scene 15. The
reusable entity link keeps Scene 13 as its generic route and records the ROM
collision-probe return policy.

## Physical QA

`shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa` drives
ordinary Up input from south of the ROM rectangle, with zero funds and the
Scene-13 `ILENE_MET` actor gate cleared. It covers:

| Run state | Route result |
| --- | --- |
| Active area-3/event-4 gang contract | Passage, then Scene 15 with four actors |
| Active area-1/event-9 contract | No passage; event-table row 4 loads Scene 13 |
| No active contract | No passage; event-table row 4 loads Scene 13 |

The matching case reaches the event rectangle on frame 22, opens the passage
on frame 39, and reaches Scene 15 after dismissal with four actors. The six
route cases for Penumbra and Puyallup produce 26 files in each build; Debug and
Release SHA-256 hashes match. Full 320x224 contact, passage, destination, and
fallback images were inspected at original resolution. These host captures do
not establish independent Genesis pixel fidelity. The gang encounter and
reward path remain open.
