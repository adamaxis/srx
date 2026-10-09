# Gameplay scripting boundary

## Direction

Use embedded Lua for story flow and gameplay orchestration. Lua is a good fit
for a native C++ host because the host can call script functions and register
its own C-facing functions. The runtime currently pins Lua 5.4.9 and compiles
the vendored upstream source into the game. The script package contains
readable `.lua` files; it does not use version-specific bytecode.

Scripts should read naturally as gameplay logic. A motel interaction should
use a shared motel routine and ask the quest layer for relevant story hooks.
A cutscene should be a sequential function that places actors, awaits their
movement or animation, plays sounds, and presents dialogue. Item scripts
should call inventory operations. They should not be nested data records that
repeat a choice id, visual role, and native action for every row.

The Lua-facing surface is one global `game` facade. Scripts read global native
context directly instead of threading location and quest facts through every
callback. The connected surface today is:

```lua
game.state.opening_dialogue_seen = true
if game.location.type == "motel" then
    game.ui.say("arrival")
end
local choice = game.ui.menu("manager", {"crash", "leave"})
```

`game.state` reads and writes native story facts through a boolean property
proxy. The live field runtime binds known properties directly to its typed
quest state; Lua sees the current values and writes them back immediately,
without an interaction-level snapshot/copy-back pass. Standalone script QA may
still use an in-memory fact store. `game.location` is a read-only proxy over
native location context, including type, identity, text profile, and optional
speaker. The proxies are not duplicated Lua tables or raw pointers into Qt
objects.

The next capabilities should follow the same shape, added only as they connect
to real native systems:

```lua
-- Future item operations use native inventory rules.
game.items.add(item_id, count)
game.items.remove(item_id, count)

-- Future cutscene operations yield to the native actor/animation systems.
game.actor.place("stoker", 164, 92)
game.actor.move_to("stoker", 172, 92)
game.actor.animate("stoker", "talk")
game.audio.play("motel.door_open")
```

Movement and animation waits should suspend the Lua coroutine and resume it
when the native system reports completion. Audio should be callable from the
same sequential flow; whether it blocks or overlaps is a property of that
operation. The game update loop remains the only clock, and Qt presents native
state and forwards input.

The facade exposes named capabilities, not raw Qt widgets, renderers, actor
objects, inventory containers, or save records. This keeps the boundary clear:
Lua decides the sequence and story branch; native services validate and apply
state changes, move and draw actors, play audio, and save the resulting game
state. New operations can be added one at a time behind this interface.

## State and ownership

`game.state` writes update the native story-fact store immediately. The host
still owns that store and any persistence rules; Lua does not receive the save
record or mutate arbitrary engine objects. Inventory and wallet changes must
go through native operations so their source rules can validate the request.
Only the allowlisted story facts currently bind to the live field session;
save serialization and restoration remain separate work.

Four coroutine operations are connected to live routes. The first is
`game.actor.walk_heading(actor, heading)`: it accepts the player and headings
0 through 7, yields a typed request to the native field controller, and resumes
with the operation result. `game.ui.say(event_id)` resolves the speaker and
text profile from native location context, then suspends until the dialogue
presenter advances the page. The only bound speaker is Stoker; his native
profile supplies his name and portrait, and the `stoker.motel` profile
currently binds `arrival` and `leave`. Arrival has its source A-button
continuation; the title-only farewell closes through the ordinary controller
wait. `game.ui.menu(menu_id, choice_ids)` combines the menu name with the
location text profile, yields choices to the common presenter, and resumes
with the stable selected ID. The source A/B/C mapping is preserved.
`game.services.call(domain, action)` yields a namespaced request to an
allowlisted native service and resumes Lua with a stable result ID. Current
Stoker scripts use it for room payment, debt-offer flags, debt payment, and the
Jump House referral. Lua chooses the room/debt menus, updates its bound quest
facts from the returned outcome, and selects result dialogue; C++ validates
and applies wallet, source-world, and runner-record effects. The Motel module
handles the shared manager and room menu; Michael's quest module contributes
the package option and debt menu. Each operation uses a 100,000-instruction
limit on initial invocation and resume.

The Ares Weapon Emporium stock is declared in `gameplay/scripts/shop.lua`.
The script groups offers under stable category names; `GameScriptRuntime`
validates the bounded catalog, and the native shop controller consumes those
typed offers. The renderer, category cursor, purchase/sale confirmation, and
wallet/inventory rules remain native. This is the first shop content moved out
of the C++ menu implementation; the interaction loop and other shop menus are
still to be migrated.

ROM-derived Stoker and field-interaction dialogue strings are imported into
`assets/dialogue/script-text.lua` as a keyed Lua text table. The shared dialogue
and field-interaction controllers resolve those keys through
`GameScriptRuntime`; they no longer load separate TSV catalogs. The file is a
content resource generated by the explicit ROM asset-import step, while menu
layout, focus, dialogue paging, and input remain in the generic native
presenters. Story modules select the content and choices.
Actor placement, destination movement, scripted animation, item operations,
audio playback, additional speaker/event profiles, and save restoration are
not connected yet. The runner has no `io`, `os`, `debug`, or package-loader
access.

## Content organization

- `gameplay/scripts/field_events.lua` contains source-traced map trigger
  declarations and the New Game scene handler. Trigger detection and repeat
  policy belong to the native event runner; a matched event starts a Lua
  coroutine, which requests heading-zero player movement through the shared
  actor service and waits for the native operation to finish.
- `gameplay/scripts/motel.lua` contains one shared motel interaction, not a
  script for every named building. It adds the base choices and collects the
  optional `motel_manager_choices()` hook from quest modules in stable
  module-name order. `manager_interaction()` handles the shared manager menu,
  room menu, payment service, result dialogue, and Leave dialogue. A successful
  room payment returns the hotel-options result to the native host.
- `gameplay/scripts/michael_package.lua` contains the Michael quest hook and
  its first-visit, repeat, and completed branches. `stoker_interaction()` now
  chooses the arrival page and shared motel manager in Lua; the field route no
  longer converts a Lua phase string into a C++ story-entry enum. Its
  motel-choice hook checks `game.location`, so the quest option appears at
  Stoker and does not leak into another motel. `stoker_interaction()` uses the
  speaker supplied by location context, runs the first-visit `arrival` event,
  enters the shared motel manager, then opens the debt menu and selects
  payment/referral result pages. It writes first-visit and debt/package facts
  through the live native-state proxy after the matching story transition;
  no dialogue adapter copies these facts into a temporary Lua map. Other
  quests can add the same choice hook without adding per-building scripts or
  changing `motel.lua`.
- `gameplay/scripts/shop.lua` is the shared shop catalog. It currently defines
  the ordered Ares categories and stock; shops can reuse those categories
  while native services continue to validate each transaction.
- `assets/dialogue/script-text.lua` is the ROM-imported shared text table for
  the Stoker motel and field object-interaction routes. Their native handlers
  retain stable text keys and resolve the strings through the Lua runtime.
- Future `quests/` and `scenes/` modules should call the same `game` facade.
  Menus and dialogue use the generic Qt presenters, and Lua supplies the
  available stable choice ids and story text keys.

The current slice connects the New Game trigger, native-backed story facts,
the Michael quest branch, shared motel and debt menus, room/debt service calls,
arrival/farewell/result dialogue, and the Qt presenters. Stoker and field
interaction text is now Lua content. Script menus carry
their text profile and optional speaker into one generic panel projection;
menu choices use generic presentation roles. The native presenter draws panels
and applies source input rules. Wallet and quest effects remain native service
implementations. Ares shop stock is script data, but its selector, transaction
loop, and native UI still run in C++. Other building dialogue, title/field
menus, item operations, inventory screens, actor placement/animation, audio
playback, and save restoration remain to be migrated or connected.

## Source and visual verification

Every event and story branch remains grounded in the ROM profile and checked
against converted Ghidra code. C++ source addresses stay in audit documents as
provenance, not script identifiers. Script QA checks state transitions and
normal-input routes. Each visual review asks whether the screen looks right;
whether focus/selection matches the actual behavior; whether the art, layout,
and selector marks are correct; what moves and on which cadence; which sounds
play and for how long; whether adjacent screen regions also remain correct;
whether several frames support the interpretation; and whether the prior UI
was cleared or redrawn before new controls appeared. Internal Qt captures
verify the converted presentation only; they do not establish Genesis fidelity
without an independent game capture.

## Provenance

- ROM scene-2 descriptor/collision profile: `0x172DAA`, with Stoker record 91
  at `0x173138` and the scene event table at `0x172CDC`.
- Ares shop stock: three-byte ROM records at `0x1AAC9E` (weapons), `0x1AAD8F`
  (other items), and `0x1AADD7` (armor); the category dispatch is at `0x56DC4`.
- New Game opening state and field handoff:
  `decompilation/export/SCRATCHPAD.md`, section
  `The startup distinction is explicit in the conversion source`.
- Physical Stoker route:
  `decompilation/docs/visual-audits/motel-doorway-scene-entry.md`.
- Lua's embedding API, version history, and MIT license are documented by the
  [official Lua manual](https://www.lua.org/manual/5.4/manual.html),
  [version history](https://www.lua.org/versions.html), and
  [license](https://www.lua.org/license.html).
