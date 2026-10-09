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

The Lua-facing surface should be one global `game` facade, with focused
services such as:

```lua
game.state.opening_dialogue_seen = true
game.items.grant("michael_package")
game.actor.place("stoker", 164, 92)
game.actor.move("player", 164, 92)
game.actor.animate("stoker", "talk")
game.audio.play("motel.door_open")
game.ui.say("stoker.arrival")
local choice = game.ui.menu("motel.manager", {"crash", "leave"})
```

Calls that wait for a movement, animation, window, or sound should suspend the
Lua coroutine and resume it when the native system reports completion. A
nonblocking sound call can return immediately. The game update loop remains
the only clock; Qt continues presenting the native state and forwarding input.

The facade exposes named capabilities, not raw Qt widgets, renderers, actor
objects, inventory containers, or save records. This keeps the boundary clear:
Lua decides the sequence and story branch; native services validate and apply
state changes, move and draw actors, play audio, and save the resulting game
state. New operations can be added one at a time behind this interface.

## State and ownership

`game.state` is the script-visible name for native story state. Scripts can
change a flag directly; the host reads those writes back, validates them, and
applies approved fields to native session state and, when implemented, the
save adapter. Lua globals are not the save system. Service calls such as
granting an item or paying a debt still go through the native inventory or
wallet rules.

The runtime exposes global Lua `game.state` and `game.location` tables, so
scripts read and update session context without threading state arguments
through every callback. These tables are synchronized with typed native state
at call boundaries; they are not live C++ object references, and C++ remains
authoritative. The opening Stoker quest script marks
`opening_dialogue_seen`, and the field controller applies that validated flag.
Other script writes are not yet connected to general session or save state.
The first coroutine service is `game.actor.walk_heading(actor, heading)`: it
currently accepts the player and headings 0 through 7, yields a typed request
to the native field controller, and resumes with the operation result. Both
initial invocation and each resume have a 100,000-instruction limit. Actor
placement, destination movement, scripted animation, `game.items`,
`game.audio`, `game.ui`, and durable save backing remain unimplemented. The
runner has no `io`, `os`, `debug`, or package-loader access.

## Content organization

- `gameplay/scripts/field_events.lua` contains source-traced map trigger
  declarations and the New Game scene handler. Trigger detection and repeat
  policy belong to the native event runner; a matched event starts a Lua
  coroutine, which requests heading-zero player movement through the shared
  actor service and waits for the native operation to finish.
- `gameplay/scripts/motel.lua` contains shared motel behavior, independent of
  Stoker's identity. It adds the base choices and calls the optional
  `motel_manager_choices()` hook on loaded quest modules in stable module-name
  order.
- `gameplay/scripts/michael_package.lua` contains the Michael quest hook and
  its first-visit, repeat, and completed branches. Its motel-choice hook checks
  `game.location`, so the quest option appears at Stoker and does not leak into
  another motel. Other quests can add the same hook without changing `motel.lua`.
- Future `quests/` and `scenes/` modules should call the same `game` facade.
  Menus and dialogue use the generic Qt presenters, and Lua supplies the
  available stable choice ids and story text keys.

The current vertical slice connects the New Game scene-entry trigger, its
Lua-owned handler and yieldable movement request, the Michael package entry
branch, and the shared motel manager choices to the Qt field and dialogue
runtime. Native C++ still owns movement execution, collision, payment,
inventory effects, animation timing, scene presentation, and most other
map/menu behavior. Only the heading-based player movement operation is wired
through the coroutine boundary; the remaining examples above describe
incremental services to add, not existing adapters.

## Source and visual verification

Every event and story branch remains grounded in the ROM profile and checked
against converted Ghidra code. C++ source addresses stay in audit documents as
provenance, not script identifiers. Script QA checks state transitions and
normal-input routes. Visual review checks the full frame during menu/dialogue
entry and exit, selector state, animation timing, sound timing, and whether
old panels are fully cleared before the next interface appears. Internal Qt
captures verify the converted presentation only; they do not establish
Genesis fidelity without an independent game capture.

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
