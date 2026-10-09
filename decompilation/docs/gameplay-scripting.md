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

`game.state` reads and writes the C++ story-fact store through a boolean
property proxy. `game.location` is a read-only proxy over native location
context, including type, identity, text profile, and optional speaker. The
proxies are not duplicated Lua tables or raw pointers into Qt objects.

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
Only story facts connected to the field session currently reach durable game
state; adding the general save adapter remains separate work.

Three coroutine operations are connected to live routes. The first is
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
with the stable selected ID. The source A/B/C mapping is preserved. The generic
motel script handles the shared menu and farewell; the Michael quest
contributes its optional package choice. The native route still validates and
executes package payment and other unfinished actions. Each operation uses a
100,000-instruction limit on initial invocation and resume.
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
  module-name order. `manager_interaction()` presents the menu, runs the
  location's optional `leave` dialogue when selected, then returns the stable
  choice ID for native handling.
- `gameplay/scripts/michael_package.lua` contains the Michael quest hook and
  its first-visit, repeat, and completed branches. Its motel-choice hook checks
  `game.location`, so the quest option appears at Stoker and does not leak into
  another motel. Its first-visit interaction uses the speaker supplied by the
  location context, says the `arrival` event, then continues into the shared
  motel manager after A. Other quests can add the same choice hook without
  adding per-building scripts or changing `motel.lua`.
- Future `quests/` and `scenes/` modules should call the same `game` facade.
  Menus and dialogue use the generic Qt presenters, and Lua supplies the
  available stable choice ids and story text keys.

The current slice connects the New Game trigger, native-backed story facts,
the Michael quest branch, shared motel menu, arrival/farewell dialogue, and the
Qt presenters. Lua sequences the first-visit arrival and the Leave response;
the native presenter draws panels and applies source input rules. Lua returns
stable IDs for actions whose native transaction has not yet moved behind a
game service. Movement execution, collision, payment, inventory effects,
animation timing, and most map/menu behavior remain native. The future API
examples above are design targets, not live adapters.

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
- New Game opening state and field handoff:
  `decompilation/export/SCRATCHPAD.md`, section
  `The startup distinction is explicit in the conversion source`.
- Physical Stoker route:
  `decompilation/docs/visual-audits/motel-doorway-scene-entry.md`.
- Lua's embedding API, version history, and MIT license are documented by the
  [official Lua manual](https://www.lua.org/manual/5.4/manual.html),
  [version history](https://www.lua.org/versions.html), and
  [license](https://www.lua.org/license.html).
