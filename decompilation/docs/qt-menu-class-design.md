# Qt menu class design

This design gives each screen a small controller and presenter while keeping
gameplay transitions in the typed conversion/session layer. It applies to the
Ghidra-conversion Qt executable and is the migration plan for its current
`FieldSceneRuntime` menu responsibilities.

## Ownership

- The focused Qt window forwards each controller edge once.
- A screen controller forwards input edges to that screen's typed conversion
  calls and returns a view or typed route result. It owns host timing and
  presentation inputs where the source requires them.
- The conversion/session layer owns menu selections, inventory and currency
  mutations, world state, and source event transitions.
- A presenter draws one resolved view from individually named assets. It
  clears or replaces the screen region owned by that view before drawing new
  controls. A shared frame remains visible only when the source keeps it across
  the child transition.
- A route coordinator tracks the active screen and parent return route. It
  leaves each screen's source cursor and behavior with that screen's typed
  state.

```mermaid
flowchart TD
    W[Focused Qt window] --> R[Menu route coordinator]
    R --> C[Screen controller]
    C --> S[Typed conversion and session state]
    C --> V[Resolved screen view]
    V --> P[Screen presenter]
    P --> A[Named assets]
    C --> R
```

## Screen map

| Screen | Qt class boundary | Existing authoritative state or behavior | Separation to make |
|---|---|---|---|
| New Game / Start Options | `StartOptionsMenuController` and `StartOptionsMenuPresenter` | `StartOptionsMenuState` and `startup_menu_conversion` | Move key handling and drawing from the window into the screen pair. |
| Save / Load | `SaveLoadMenuController` and `SaveLoadMenuPresenter` | `SaveLoadMenuState`; save validation and restore conversions | Keep slot selection and load result typed; connect real save storage through the session layer. |
| Joshua archetype | `ArchetypeMenuController` and `ArchetypeMenuPresenter` | `JoshuaArchetypeMenuState` and startup integration | Bind the three source records and selected portrait to a dedicated view. |
| In-game Start command overview | `FieldCommandMenuController` and `FieldCommandMenuPresenter` | `FieldCommandOverviewBridge`, `CommandOverviewState`, and menu service conversions | Move the command overview's page rendering and child dispatch out of `FieldSceneRuntime`. |
| Inventory grid | `InventoryMenuController` and `InventoryMenuPresenter` | Active runner record, inventory records, and item catalog | Present selection/equipment from the shared runner record. Preserve the source's one-pixel item outline and redraw the whole owned panel on entry/return. |
| Inventory action row | `InventoryActionMenuController` and `InventoryActionMenuPresenter` | `InventoryActionMenuState` and item-action conversion functions | Keep equip, discard, automatic use, First Aid, and Trade as separate typed routes. |
| Trade | `InventoryTradeMenuController` and `InventoryTradeMenuPresenter` | `InventoryTradeState` and typed roster records | Own recipient scan, full-recipient handling, quantity/confirmation, and transfer result. |
| First Aid | `FirstAidMenuController` and `FirstAidMenuPresenter` | `FirstAidUserState`, target state, and First Aid conversions | Own eligible-target selection and the result wait as a child screen, returning through a typed result. |
| Clips / reload | `ClipsMenuController` and `ClipsMenuPresenter` | `FieldClipsMenuRuntime`, `ClipTradeState`, and item capacity | Extract the current paint/input work from `FieldSceneRuntime`; preserve reload and clip-trade results. |
| Attributes / Skills | `AttributesSkillsMenuController` and `AttributesSkillsMenuPresenter` | `AttributesSkillsMenuState` and progression conversions | Read the active record and route confirmed upgrades through typed mutation functions. |
| Cyberware | `CyberwareMenuController` and `CyberwareMenuPresenter` | `CyberwareMenuModel` | Keep the current installed-bit/name view read-only until source purchase/install paths are traced. |
| Magic | `MagicMenuController` and `MagicMenuPresenter` | `MagicMenuLoopState`, `MagicForceState`, and Magic conversions | Move spell/force selection and return behavior out of `FieldSceneRuntime`; keep the defense fallback as its own source-selected branch. |
| Pocket Secretary | `PocketSecretaryMenuController` and a presenter per child page | `PocketSecretaryMenuState`, `NotebookMenuState`, and `DismissRunnerMenuState` | Give the parent, Notebook/Tips, and Dismiss Runner distinct views and route results. Rebuild the panel before a child page replaces parent controls. |
| Dialogue | `FieldDialogueEventResolver` plus `DialoguePanelPresenter` | `DialoguePanelView`, `FieldExitPromptState`, and source speaker/event records | Resolve speaker properties, trigger event, text, portrait, options, and frame mode before the generic presenter draws. Use event-specific providers for motel, building, and random street encounters. |
| Shop | `ShopMenuController`, `ShopTransactionService`, and `ShopMenuPresenter` | Shop entry, stock, and transaction source records are pending | Trace the shop's mode/event, stock and price tables, buy/sell rules, input loop, and return path before defining the final state fields or enabling transactions. |

`GhidraConversionWindow` remains the Qt window and focused-input endpoint. The
current `FieldSceneRuntime` is the field composition host; its embedded menu
state and `paint_command_overview*` / `advance_command_overview*` methods mark
the extraction seams. `FieldCommandOverviewBridge`, `FieldClipsMenuRuntime`,
and the newly separated `DialoguePanelPresenter` show the intended shape.

## Route and drawing contract

Each controller reports one of: remain on this screen, open a source-selected
child, replace this screen with a source-selected destination, or return to
its parent. The coordinator preserves the parent's typed state while a child
is open. The destination comes from the traced event and source table.

Each presenter owns its screen rectangle. On entry or replacement it redraws
that rectangle from its named background/frame and current view, then draws
only the current controls. A child that shares the parent's frame follows the
source's clear/write order and removes old buttons, text, selectors, and item
graphics before drawing its own. Animation advances from a controller tick
only when a source counter or event requires it. Sound requests go through the
audio service with a source-derived event and measured or explicitly pending
duration.

## Shop boundary

The current source audit identifies the Ares Weapon Emporium entrance as a
separate building/event path and leaves its shop behavior open. The
`FieldAction*Purchase` conversions cover field action progression, cyberware,
resource, and quantity purchases. The general shop inventory, buy/sell
interface, prices, and transaction rules remain open. The shop controller will
consume catalog and stock data after its entry path and tables have been
traced; buying an item and equipping it remain separate state changes.

## Verification for each screen

Every screen slice answers the visual audit questions in
`visual-analysis/qt-visual-audit.md`: appearance, behavioral selection state,
source layout and visual attributes, animation cadence, sound and duration,
related screen content, multiple independent frames, and removal of prior UI.
Each implemented screen also gets a source trace, typed-state route coverage,
a focused CTest, and a CLI image dump. Independent gameplay frames remain the
appearance reference; internal assets and dumps check the conversion path.

## Migration order

1. Extract the in-game command overview and its inventory, First Aid, Trade,
   Magic, Cyberware, Pocket Secretary, and Clips children from
   `FieldSceneRuntime`, one screen slice at a time.
2. Move Start Options, Save/Load, and Archetype handling out of the Qt window
   into their screen controllers and presenters.
3. Separate dialogue event resolution from the generic panel. Connect each
   provider from its speaker properties and field/building/encounter event.
4. Trace the Ares/shop entry and transaction tables, then define and connect
   `ShopMenuController`.

The dialogue frame/presenter slice is committed as `18ae07ba42` and remains
partial while live speaker/event resolution and other event branches are open.
