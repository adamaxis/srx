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
Three coroutine operations are now connected to live routes. The first is
`game.actor.walk_heading(actor, heading)`: it accepts the player and headings
0 through 7, yields a typed request to the native field controller, and resumes
with the operation result. `game.ui.say(speaker_id, event_id)` snapshots the
active location text profile into a speaker/event request and suspends until
the native dialogue presenter advances that page. The current connected
speaker is Stoker, whose native profile supplies his name and extracted
portrait; the `stoker.motel` text profile and `arrival` event resolve the
source title and A-button continuation. `game.ui.menu(profile, choice_ids)`
yields a menu definition to the same presenter, then resumes with the stable
ID selected by the player. Shared `motel.lua` builds the ordinary motel options
and collects optional quest hooks; the Stoker route supplies `stoker.motel` as
the location's text profile. The A/B/C input follows the source menu's button
mapping, and validated native handlers still perform the resulting actions.
The New Game first-visit script now says the arrival event, waits for A, then
calls the shared motel manager. These are the first connected dialogue and
menu operations, not a general dialogue/window toolkit. All three operations
use a 100,000-instruction limit on initial invocation and each resume. Actor
placement, destination movement, scripted animation, `game.items`,
`game.audio`, additional speaker/event profiles, and durable save backing
remain unimplemented. The runner has no `io`, `os`, `debug`, or package-loader
access.

## Content organization

- `gameplay/scripts/field_events.lua` contains source-traced map trigger
  declarations and the New Game scene handler. Trigger detection and repeat
  policy belong to the native event runner; a matched event starts a Lua
  coroutine, which requests heading-zero player movement through the shared
  actor service and waits for the native operation to finish.
- `gameplay/scripts/motel.lua` contains shared motel behavior, independent of
  Stoker's identity. It adds the base choices and calls the optional
  `motel_manager_choices()` hook on loaded quest modules in stable module-name
  order. `manager_interaction()` presents that list through the yielded
  `game.ui.menu` service and returns the selected stable choice ID.
- `gameplay/scripts/michael_package.lua` contains the Michael quest hook and
  its first-visit, repeat, and completed branches. Its motel-choice hook checks
  `game.location`, so the quest option appears at Stoker and does not leak into
  another motel. Its first-visit interaction calls `game.ui.say("stoker",
  "arrival")`, then continues into the shared motel manager after A. Other
  quests can add the same choice hook without changing `motel.lua`.
- Future `quests/` and `scenes/` modules should call the same `game` facade.
  Menus and dialogue use the generic Qt presenters, and Lua supplies the
  available stable choice ids and story text keys.

The current vertical slice connects the New Game scene-entry trigger, its
Lua-owned handler and yieldable movement request, the Michael package entry
branch, and the shared motel manager menu to the Qt field and dialogue
runtime. Lua supplies the first-visit speaker/event sequence, then the live
manager choice list, and receives the player's stable selected ID through one
suspended task. Native C++ resolves speaker properties and text assets, draws
the panel, forwards A/B/C input, and still owns movement execution, collision,
payment, inventory effects, animation timing, and most other map/menu behavior.
The remaining examples above describe incremental services to add, not
existing adapters.

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
