# #30 Menu B-button audit (partial; US ROM, port-fixes)

Step 1 only. No product code changed. Input bits [C]: `0x195C` builds edge/held `FFDD22`; B `0x1000` (`btst #0xC`), A `0x4000`, Start `0x8000`. These are input masks, not a shared cancel policy.

| Menu | ROM B action | ROM address | Port behavior (headless) | Match |
|---|---|---|---|---|
| Command (start) menu | Confirm: branches to the A selection handler | `0x10EC8 -> 0x10E4E / 0x11740` | B selects the item-action child in 1 route update | yes [C] |
| Magic spell page | Confirm: same equipment-selection path as A | `0xFD72 -> 0xFD8E -> 0xFF5A` | B opens MAGIC_FORCE in 1 update | yes [C] |
| Pocket Secretary root | No B branch (A `0xC32E`, C `0xC352`, Start `0xC368`) | `0xC2A6` | B leaves screen unchanged | yes [C] |
| Notebook | Row dispatch (only Start returns from parent) | `0xF236` | B dispatches Tips in 1 update; Tips B returns in 1 | yes [C] |
| Mr. Johnsons list | Opens detail | `0xF5C2 -> 0xF592` (list built at `0xF416`) | B opens details (entries 1, 3) | yes [C] |
| All other menus | not audited | - | - | open [H] |

Finding: Genesis has no universal B-cancel. The playtest reports (B confirms on the start menu, B doesn't cancel on the spell page) describe ROM behavior. A generic "B cancels" remap would diverge from the ROM. Needs a user decision [H].
