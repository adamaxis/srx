# #30b B-button policy audit (US ROM, `port-fixes`)

This audit covers the 41 Lua `E.MenuScreen` routes registered by
`menu/field_init.lua`, plus nearby controller-driven menu roots that do not
currently use `menu.route`. Genesis has no universal B-cancel mapping. The
selected port policy is therefore **ROM by default**, with an optional
`ALWAYS_CANCEL` policy that changes B only in the port.

`FFDD22` separates new button edges from held state at `0x195C`; the menu loop
wait helper `0x0D94` consumes the high-byte edge mask. B is `0x1000`, A is
`0x4000`, and Start is `0x8000` [C]. For the ROM column, the address names the
source input branch/routine. For the port column, headless measurements are
from the production Lua route and its Lua/Qt QA, not from reading the handler
alone.

| Menu(s) | ROM B action and address | Port default behavior (headless measured) | `ALWAYS_CANCEL` |
|---|---|---|---|
| Command/start menu (`COMMAND`) | Confirm through the A selection path: `0x10EC8 -> 0x10E4E/0x11740` [C] | B opens the selected item-action child in one route frame; `menu_tests.lua` [C] | Pop to parent, or close at root |
| Posture focus (`POSTURE`) | B has no branch; Down returns to Command and Start exits: `0x119CE -> 0x11A9A` [C] | B leaves posture focus open; route handler returns no transition in one frame [C] | Pop to parent, or close at root |
| Inventory action (`ITEM_ACTION`) | B confirms like A: `0x11852..0x118A0` [C] | B selects the focused action in one frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Discard confirmation (`DISCARD_CONFIRM`) | A/B/C confirm the current Yes/No selection: `0x12694..0x12758` [C] | B follows the selected confirmation branch; source-backed route suite [C] | Pop to parent, or close at root |
| Inventory transfer (`INVENTORY_TRADE`) | A/B/C confirm the source transfer row: `0x127B0..0x1281E` [C] | B is handled by the transfer selector as confirm; route suite [C] | Pop to parent, or close at root |
| First Aid (`FIRST_AID`) | A/B/C choose the selected target: `0x1235C..0x1253C` [C] | B is handled as target confirmation; source-backed route suite [C] | Pop to parent, or close at root |
| Automatic item use (`AUTOMATIC_USE`) | A/B use the selected item; source dispatch at `0x12148..0x121C0` [C] | B follows the item-use branch; source-backed route suite [C] | Pop to parent, or close at root |
| Clips (`CLIPS`) | A/B/C activate the selector; Cancel is a row, not a B action: `0x11BC6..0x12062` [C] | B activates the selected action in one frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Reload confirmation (`RELOAD_CONFIRM`) | B/C/Start are ignored; A confirms: `0x11D58..0x11D84` [C] | B leaves the prompt open; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Clip transfer (`CLIP_TRADE`) | A/B/C operate the selected action; Start aborts: `0x11EA4..0x120E4` [C] | B follows the selected trade action; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Attributes/Skills/Karma (`ATTRIBUTES_SKILLS`) | B/A enter the selected upgrade or leave the view-only page: `0xD01C`, `0xD33A..0xD4A0` [C] | B follows the existing upgrade/return branch; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Upgrade confirmation (`UPGRADE_PROMPT`) | A/B/C confirm the Yes/No choice: `0xD53C..0xD5DA` [C] | B follows the selected prompt result; route handler [H], no dedicated baseline assertion yet | Pop to parent, or close at root |
| Cyberware page (`CYBERWARE`) | Any fresh edge returns to Command: `0x11950 -> 0x11928` [C] | B returns to Command in one route frame; `menu_tests.lua` [C] | Pop to parent, or close at root |
| Magical Defense wait page (`MAGICAL_DEFENSE`) | Display-then-edge wait: `0xFBA6` [C] | B dismisses the page in one route frame; route handler [C] | Pop to parent, or close at root |
| Magic spell page (`MAGIC`) | B enters the same equipment/action path as A: `0xFD72 -> 0xFD8E -> 0xFF5A` [C] | B opens Magic Force in one route frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Magic Force (`MAGIC_FORCE`) | A/B/C finish Force selection: `0x10016..0x10172` [C] | B returns from Force to its parent in one route frame; source-backed route suite [C] | Pop to parent, or close at root |
| Magic posture (`MAGIC_POSTURE`) | A/B/C finish posture adjustment; Up returns to Force: `0x10722..0x107CE` [C] | B returns from posture adjustment in one route frame; source-backed route suite [C] | Pop to parent, or close at root |
| Pocket Secretary (`POCKET_SECRETARY`) | No B branch; A dispatches, C switches runner, Start exits: `0xC2A6..0xC368` [C] | B leaves the root page unchanged; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Notebook (`NOTEBOOK`) | B dispatches the selected row; Start returns: `0xF236` [C] | B opens the selected row in one route frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Notebook Tips (`NOTEBOOK_TIPS`) | A/B/Start return to Notebook: `0xF236` child path [C] | B returns to Notebook in one route frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Mr. Johnsons (`MR_JOHNSONS`) | B opens the selected detail: `0xF5C2 -> 0xF592` (`0xF416` builds list) [C] | B opens detail in one route frame; field-secretary QA and `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Notebook contacts (`NOTEBOOK_CONTACTS`) | B selects the highlighted record/detail branch in the notebook record page: `0xF236` / `0x1E72C` [C] | B follows the selected record branch in one frame; direct route result [H] | Pop to parent, or close at root |
| Notebook shadowrunners (`NOTEBOOK_SHADOWRUNNERS`) | B selects the highlighted runner/detail branch: `0xF236` / `0x1E72C` [C] | B follows the selected record branch in one frame; direct route result [H] | Pop to parent, or close at root |
| Notebook Current Run (`NOTEBOOK_CURRENT_RUN`) | Any fresh button edge returns from the read page: `0xF312` [C] | B returns to Notebook in one route frame; route handler [C] | Pop to parent, or close at root |
| Group Items (`GROUP_ITEMS`) | Any fresh edge returns from the list: `0xC8C8` [C] | B returns to Pocket Secretary in one route frame; route handler [C] | Pop to parent, or close at root |
| Dismiss Runner (`DISMISS_RUNNER`) | A/B/C confirm selected runner/Abort: `0xC448..0xC4FA` [C] | B follows the selected dismissal/Abort branch; route suite [H], no dedicated baseline assertion yet | Pop to parent, or close at root |
| Save/Load action (`SAVE_LOAD`) | A/B/C select Save or Load: `0xEFB4` [C] | B opens the selected slot page in one route frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Save/Load slot (`SAVE_SLOT`) | A/B/C confirm the selected slot: `0xF188 -> 0x59360` [C] | B calls the selected slot action; route handler [H], no dedicated B assertion yet | Pop to parent, or close at root |
| Cyberdeck (`CYBERDECK`) | B chooses Balanced mode (`FFF0B7=0`): `0x1822C -> 0x18630` [C] | B enters the selected deck page with Balanced mode in one frame; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Pull Plug (`PULL_PLUG`) | A/B/C confirm the Yes/No choice: `0xE438A -> 0xE387E` [C] | B follows the selected prompt result; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Deck program slot (`DECK_PROGRAM`) | B's slot/action behavior is in the `0x19238` program loop [C] | B is consumed by the source-shaped slot handler; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Deck Cyber Info (`DECK_CYBERINFO`) | B branch within `0x18AA6` is [H] | Existing route waits/dispatches B; exact baseline result [H] | Pop to parent, or close at root |
| Deck Storage (`DECK_STORAGE`) | A-only detail action; B is ignored: `0x18D1A` [C] | B leaves the view page unchanged; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Deck Map (`DECK_MAP`) | Map return/dispatch at `0x19018`; exact B branch [H] | B leaves the page open in current tests; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Matrix node action (`MATRIX_NODE_ACTION`) | Node-action loop at `0x54662`; B action by selected node mode [H] | Route suite measures B as delivered to the active node-action state; specific source outcome [H] | Pop to parent, or close at root |
| Matrix entry (`MATRIX_ENTRY`) | Shared entry at `0xE47A -> 0x181F8`; B branch [H] | B reaches the deck/session route; `submenu_tests.lua` [C] | Pop to parent, or close at root |
| Matrix search (`MATRIX_SEARCH`) | Search loop at `0xE1E6`; exact B branch [H] | B is consumed by the active search route; specific source outcome [H] | Pop to parent, or close at root |
| Taxi selector (`TAXI_SELECTOR`) | A/B choose the selected destination; C switches runner and Start exits: `0x135FC..0x13662`, `0x10AF0` [C] | Headless selector tests cover movement and the Start path; B destination selection [H] | Pop to parent, or close at root |
| Message/wait pages (`MESSAGE`) | The caller supplies the source wait mask; B is accepted only when included in that mask [C, per caller] | The Lua mask controls whether B dismisses; default measured in message routes [H] | Pop to parent, or close at root |
| Debug menu/system picker (`DEBUG_MENU`, `DEBUG_PICK_SYSTEM`) | A dispatches; C switches runner; Start returns. No B dispatch: `0x599DA`, `0x59A9A` [C] | B leaves the selection unchanged; route handler [H], no dedicated baseline assertion yet | Pop to parent, or close at root |

The table names all 41 `E.MenuScreen` IDs; related screens share rows only
where they share the same source dispatch. The existing headless baseline is
`menu_tests.lua` 60/60 and `submenu_tests.lua` 268/268. [C] This baseline
confirms the five original routes and the additional B branches marked with
named tests above. Rows marked [H] still need a focused behavior measurement
before their ROM-versus-port default result can be called confirmed.

The non-route roots below are present in the project but do not pass input
through `menu.route` today. Their source B behavior is recorded so the setting
work can make its boundary explicit; their integration into the common route
policy is not yet verified.

| Non-route menu | ROM B action and address | Port default behavior (headless measured) | `ALWAYS_CANCEL` |
|---|---|---|---|
| Startup Options selector (New Game / Load Game) | B dispatches the selected row at `0x590EA..0x59170` [C] | The startup menu conversion test measures B entering New Game on row 0 and Load Game on row 1 [C] | [H] The title/startup selector is outside the in-game Lua route; cancellation back to title is not wired |
| Shops | A/B share the selected-action branch; Start leaves the shop: `0x56AC0` [C] | Headless production Lua `Shop.advance_menu(E.Pad.B)` on `weapons_world`: `event=no_change`, panel 0, selected action 0, still open after one call [C] | [H] Shop uses `Shop.advance_menu`, outside `menu.route`; ALWAYS_CANCEL is not connected |
| Hotel Options / Check Out | Options has no B branch (`0xCF98`); Check Out A/B/C confirm at `0xDAF0` [C] | `hotel_options_input_permutation_test` measures B no-op on Options and confirmation on Check Out [C] | [H] Hotel Lua controller is outside `menu.route` |
| Hospital Options | Outer hospital loop routes B/action through `0xC928 -> 0x391A` [C] | Hospital input currently comes from `game.ui.cursor_menu`; all-choice B result not measured [H] | [H] Hospital UI input is outside `menu.route` |
| Field dialogue choices | Source dialogue list dispatch is caller-specific; examples include `0xF5C2` and `0xF236` [C] | Selected prompt's B action depends on each script menu's input map; a complete headless family measurement is still [H] | [H] Field dialogue is outside `menu.route`; ALWAYS_CANCEL is not connected |

## Conclusion

The ROM-accurate default remains unchanged. The Lua route policy is applied
before a screen handler sees B, so the same behavior covers confirm, no-op,
wait, inventory, and Matrix route children without per-screen remaps. Headless
route tests exercise ROM forwarding and ALWAYS_CANCEL at root and child depth
for every one of the 41 `E.MenuScreen` IDs. [C] The startup selector, shops,
hotel, hospital, and dialogue controllers remain outside `menu.route`; the
setting is not wired to them and both policy modes have not been exercised in
those controllers. Their source/default behavior rows remain separate above
and #30b is not ready for an all-menus coverage claim.
