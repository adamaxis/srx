# #30b B-button policy audit (US ROM, `port-fixes`)

This audit covers the 42 field-menu route screens registered by
`menu/field_init.lua`, plus controller-driven prompts now adapted through the
same route. Genesis has no universal B-cancel mapping. The
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

The table names the 42 original field `E.MenuScreen` IDs; related screens share rows only
where they share the same source dispatch. The existing headless baseline is
`menu_tests.lua` 60/60 and `submenu_tests.lua` 268/268. [C] This baseline
confirms the five original routes and the additional B branches marked with
named tests above. Rows marked [H] still need a focused behavior measurement
before their ROM-versus-port default result can be called confirmed.

The following callers use `menu.route` through registered external-screen
adapters. Their ROM-mode default actions remain screen-specific; the optional
B policy runs before each handler.

| Non-route menu | ROM B action and address | Port default behavior (headless measured) | `ALWAYS_CANCEL` |
|---|---|---|---|
| Startup Options selector (New Game / Load Game) | B dispatches the selected row: row 0 enters New Game, row 1 opens Load Game, `0x59154..0x5916E` [C] | Headless startup B check measures row 0 -> New Game/archetype and row 1 -> Load Game; `generic_menu_input_tests.lua` confirms the Lua adapter forwards B [measured] | `generic_menu_input_tests.lua` measures route `EXIT` at root and `POP` at child depth; Qt maps root cancel to `title_ready` [measured route; host branch wired] |
| Shops | A/B share the selected-action branch; Start leaves the shop: `0x56AC0` [C] | Headless production Lua across all five catalogs: root B is a no-op; action-panel B dispatches the selected action [measured] | `shop_route_tests.lua` measures root/action-panel exit and ALWAYS_CANCEL preemption of transactions [measured] |
| Hotel Options / Check Out | Options handles Up/Down/A/C and has no B branch (`0xCF98..0xCFF0`); Check Out accepts A/B/C at `0xDAF0..0xDB10` after cursor movement at `0xDABE..0xDAEE` [C] | Headless `hotel_options_input_permutation_test`: all 256 masks from both initial rows on each screen; B is ignored on Options, while Check Out B confirms the selected row (initial No stays; Yes checks out) [measured] | `generic_menu_input_tests.lua` and production Hotel Lua wrappers measure root B -> close event and child B -> return-to-Options event [measured] |
| Hospital Options and child prompts | `0xC928..0xCA0A` has no B branch; C calls `0x391A` at `0xC9EA..0xC9FC`. Medical Attention B returns to Options from handler `0xCA2E`; Exit B confirms the selected Yes/No row in handler `0xCE7C` (`0xCF02` accepts A/B/C) [C] | Headless Hospital Options QA: B (`port_byte=0x10`) with Cyberware selected leaves the prompt on row 1; C (`port_byte=0x20`) calls the field mode action and returns to row 1. `field_dialogue_runtime` measures medical B back and leave B confirming the initial No row [measured] | Headless `field_dialogue_runtime`: ALWAYS_CANCEL B closes Hospital Options to field at root and returns from leave confirmation to Hospital Options at child depth [measured] |
| Scripted field-dialogue choices | B dispatch depends on the caller: `run_mode_text_prompt` tests A/B/C in `0x1F3C4..0x1F58C`; e.g. `0xF5C2..0xF5CE` uses B to select a detail row [C]. Vid-Phone A/B activates the contact row at `0xF90C/0xF972`; Start precedes the ignored C path at `0xF980` [C] | Headless `field_dialogue_runtime`: B selects option 2 in its scripted room-choice case; cursor-choice menus map B to the caller's back result. Vid-Phone C leaves its contact page open [measured] | `menu.route` returns root `EXIT` or child `POP` before the Lua menu action. `dialogue_graph_tests.lua` verifies button and cursor cancel edges; `generic_menu_input_tests.lua` covers ROM and ALWAYS_CANCEL actions for button, cursor, hosted, and Vid-Phone screens [measured] |

## Conclusion

The ROM-mode behavior remains unchanged except for Hospital Options B, whose
headless pre-fix action contradicted the no-B-branch ROM loop at `0xC928..0xCA0A`.
The Lua route applies policy before an external screen's source-mode action.
Headless route tests cover ROM forwarding and ALWAYS_CANCEL root/child results
for all 52 registered screens. [C] Startup Options, Hotel Options/Check Out,
Hospital Options/children, and scripted dialogue choices now use the same Lua
route; Shops were already migrated in #31.
