# Gameplay scripting boundary

## Direction

Use embedded Lua for story flow and gameplay orchestration. Lua is a good fit
for a native C++ host because the host can call script functions and register
its own C-facing functions. The runtime currently pins Lua 5.4.9 and compiles
the vendored upstream source into the game. The script package contains
readable `.lua` files; it does not use version-specific bytecode.

Dialogue text, ordered choices, condition expressions, branch edges, and
typed terminal outcomes belong in versioned JSON catalogs. Lua owns the
active graph session and registers small named quest actions; those actions
can yield typed requests to native wallet, inventory, travel, and presentation
services. A cutscene can remain a sequential Lua function for actor movement,
animation, and sound operations.

The Lua-facing surface is one global `game` facade. Scripts read global native
context directly instead of threading location and quest facts through every
callback. The connected surface today is:

```lua
local DialogueGraph = require("dialogue_graph")
return DialogueGraph.run("stoker_motel.indexed_service", registered_actions)
```

`DialogueGraph.run` loads a JSON graph, evaluates named conditions against
typed `game.state`, `game.location`, and `game.field` facts, presents pages and
choices through the shared host adapter, and returns a typed result. Unknown
facts and unconverted source edges stop at explicit boundaries. `game.state`
reads and writes ROM-backed facts through native accessors; dialogue flow is
not duplicated in native code. Standalone script QA may use an in-memory fact
store. `game.location` is read-only native context, including type, identity,
text profile, and optional speaker. `game.field` is read-only field-mode
context, including mode, event code, and the signed alternate result used by
source prompts. The proxies are not duplicated Lua tables or raw pointers
into Qt objects.

The next capabilities should follow the same shape, added only as they connect
to real native systems:

```lua
-- Future item operations use native inventory rules.
game.items.add(item_id, count)
game.items.remove(item_id, count)

-- Future cutscene operations update Lua actor state and await host services.
game.actor.place("stoker", 164, 92)
game.actor.move_to("stoker", 172, 92)
game.actor.animate("stoker", "talk")
game.audio.play("motel.door_open")
```

Movement and animation waits should suspend the Lua coroutine and resume it
when the host reports collision or playback completion. Audio should be
callable from the same sequential flow; whether it blocks or overlaps is a
property of that operation. The game update loop remains the only clock, and
Qt presents Lua-owned actor state and forwards input.

The facade exposes named capabilities, not raw Qt widgets, renderers, actor
objects, inventory containers, or save records. Lua owns actor behavior,
movement intent, animation state, and story sequencing. Native host services
provide terrain queries and accepted-position commits, audio output, and
rendering; they do not choose an actor's next behavior or animation. New
operations can be added one at a time behind this interface.

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
presenter advances the page. The JSON speaker catalog binds Stoker's
`arrival`, `street-ambush`, `package-question`, `leave`, `room.paid`,
`room.insufficient`, `package-handoff`, and `jump-house` events to explicit
text keys and portrait metadata. The JSON menu catalog binds each menu ID and
stable choice ID to its text key. The legacy prompt conversion remains for
focused resource/dump QA; production dialogue graphs carry event and menu IDs
directly. `game.ui.menu(menu_id, choice_ids)` still carries stable IDs and
location context to the shared presenter. The presenter owns layout and source
A/B/C mapping, then resumes Lua with the selected stable ID. C++ does not infer
keys from profile/menu/choice strings.
`game.services.call(domain, action)` yields a namespaced request to an
allowlisted native service and resumes Lua with a stable result ID. The Stoker
JSON graphs call it for the arrival roll, room payment, debt offer, debt
payment, and Jump House referral. JSON chooses the ordered menus and branches
on stable paid/insufficient/service result IDs; Lua registers named host
effects and C++ applies typed wallet, source-world, and runner-record
transactions. The indexed service and signed field-exit selector are separate
graph roots. Each graph operation uses a 100,000-instruction limit on initial
invocation and resume. Runtime graph catalogs are cached by immutable content
package and validate a selected graph once, so an interaction's instruction
budget is not spent revalidating unrelated contacts and locations.

`gameplay/dialogue/conversations/vid_phone_services.json` owns Captain
Quinton's first/repeat call split, Services/Goodbye choices, Hollywood and
Weller information routes, Firepower purchases, and Sharkey hiring result
branches. `conditions.json` selects first versus repeat call from the ROM
call-history fact; `contacts.json` links the contact to the graph. The Lua
module `vid_phone/capt_quinton_jaymes.lua` registers named host effects for the
call-history write and typed wallet/inventory/contact transactions.

Alesandro's service tree is the second Vid-Phone graph in the same JSON
catalog. His `contacts.json` record keeps the shared call-history/menu flow in
the common caller, then selects the service graph. The Lua action module only
forwards the two passkey purchases and Macklemann contact purchase to typed
host transactions; JSON owns the information pages, result menus, and edges.

Sharkey's service tree is the third Vid-Phone graph. JSON owns the two fare
offers, Smart Goggles choice, insufficient-funds and inventory outcomes, and
the separate Scene 6 and Scene 9 travel terminals. `vid_phone/sharkey.lua`
only translates named graph actions to the shared typed service API, using
`data/vid_phone_passages.lua` for the ROM fare and destination facts.

Kipp's service tree is the fourth Vid-Phone graph. It stores his Matrix lesson,
program and cyberdeck offers, transaction outcomes, service loops, and call
terminal in JSON. `vid_phone/kipp_david.lua` only translates the three named
purchase actions; the shared deck service retains the ROM's prices, storage
check, program ratings, and Excalibur fields.

Wilma Temmenhoff's service tree is the fifth graph. JSON owns the credstick
introduction, Matrix pages, program offer, transaction results, and the
different Services/Goodbye return destinations. Her Lua module only registers
the Rebound and Degrade transactions; the typed host service retains the ROM
price and rating-3 storage/write behavior.

Sgt. Macklemann's service tree is the sixth graph. JSON owns the Security and
Avoid Detection lessons, their Services/Goodbye loops, and the Hollywood badge
offer/result pages. The Lua adapter registers the single badge transaction;
the host service retains the 5,000-nuyen charge and Hollywood gate flag write.

Gregory Wilns' service tree is the seventh graph. JSON owns the paranormal
creature lesson, Merlin's Lore discount offer, Power Focus offer, result pages,
and return/farewell routes. His Lua adapter registers the two typed purchase
actions; host services retain eligibility, affordability, inventory, feature
flag, and item rules.

Max's graph is the tenth service graph. It owns first/repeat call history,
the corporate-badge offer, hardware categories, four purchases, and each
result/return edge. `vid_phone/max.lua` registers only the call-history write
and named transactions. The typed service applies wallet/inventory checks and
the badge flag.

Professor Jefferson's graph is the eleventh. It owns the three-page paranormal
lesson, Riannon's discount, Gregory's introduction, and all return/farewell
choices. `vid_phone/professor_jefferson.lua` registers the history write and
two named transactions. The ROM's insufficient-funds text pointer targets
invalid data, so those results stay at a `source_boundary` instead of adding
invented copy.

The eleven contacts with traced service behavior use JSON graphs. Faradouchi
and Agira have separate call-scoped graphs for first/repeat dialogue, their
opposing-faction rejection, the active runner's Reputation-5 gate, and the
accept/decline pledge. ROM commands `0x7D` and `0x7F` write the contact-history
and corresponding Mafia/Yakuza discount bits only after acceptance. Their
graphs return directly to the caller; no Services menu is invented for them.
Longplay breakdowns reviewed so far contain no complete matching call sequence,
so internal tests cover behavior while visual fidelity remains open.

The Ares Weapon Emporium stock is declared in `gameplay/scripts/shop.lua`.
The script groups offers under stable category names; `GameScriptRuntime`
validates the bounded catalog, and the native shop controller consumes those
typed offers. Ordered service labels and action headings are consumed from the
same module without a duplicate ID list in `FieldSceneRuntime`; script tests
check their ROM order. The renderer, category cursor, purchase/sale
confirmation, and wallet/inventory rules remain native. The interaction loop
and other shop menus are still to be migrated.

`gameplay/scripts/field_mode.lua` selects the Ares mode-text record from
read-only `game.field` context. It owns the zero-result store introduction,
negative-result package handoff, positive-result package payment, and the
matching portrait asset key. The shared native mode-text prompt owns the
source-derived title-only display, 30-frame wait, and controller return; it no
longer chooses among Ares paragraphs or hardcodes the Ares logo.

`gameplay/scripts/field_events.lua` evaluates each accepted field collision
from named event-code enums and typed scene/mode/story facts. It returns a
named action; C++ validates the result and executes the corresponding movement,
dialogue, terrain, or activity service. `gameplay/scripts/building.lua` is the
shared location-route script. The native map loader supplies the active scene,
mode, event, and ROM-imported event-table bytes; Lua selects the direct
scene-2 Stoker/Ares routes or the active event-table destination. For example,
scene-2 event 0 selects numeric scene 20 and event 1 selects numeric scene 26
from the source table. C++ loads the typed destination. Unknown place names and
reverse doors without source destinations remain unresolved. Floor-selector
destination groups and several downstream activity results still have native
route logic and remain to be moved behind Lua scripts.

ROM-derived Stoker and field-interaction dialogue strings are imported into
`assets/dialogue/script-text.json` as a keyed Lua text table. The shared dialogue
and field-interaction controllers resolve those keys through
`GameScriptRuntime`; they no longer load separate TSV catalogs. The file is a
content resource generated by the explicit ROM asset-import step, while menu
layout, focus, dialogue paging, and input remain in the generic native
presenters. Story modules select the content and choices. The same ROM import
now includes all ten recruited-runner biography streams and the converted
Ricky, Winston Marrs, and Trent topic prompts and response pages. The ten
recruited-runner display names also come from the ROM template-name table.
`dialogue.lua` groups runner names, biographies, topic choices, and response
pages by stable runner and topic IDs. The shared panel projection resolves
those IDs through Lua while source-derived transition state remains typed
native C++. The projection is covered by conversion QA, but the runner
conversation still has no live field/roster entry route.
General cutscene actor placement, destination movement, scripted animation,
item operations, and save restoration are not connected yet. The scene-list
NPC entry behavior has its own connected Lua policy described below. The
runner has no `io`, `os`, `debug`, or package-loader access.

## Content organization

Legacy Lua menu catalogs return ordered stable IDs, optional title keys, and
ordered label keys; the runtime validates their sizes and does not build label
keys in C++. Ordered ID-only lists use the separate `string_list` API, so
command IDs cannot accidentally be treated as display menus. New dialogue
conversations use versioned JSON for text bindings, ordered choices,
conditions, graph edges, and outcomes, with Lua registering named effects.
Pocket Secretary and other legacy menus remain Lua catalogs until migrated.
Dynamic building menus pass stable IDs through `game.ui.menu`; their
presentation adapter resolves explicit content keys.

Startup Lua maps archetype IDs to their extracted portrait assets. Dialogue
Lua maps speaker IDs to their portrait aliases, and both feed the shared
presenter. The archetype selection screen currently has no visible stat rows:
starting attributes continue to come from the selected extracted ROM record,
rather than a second Lua copy.

- `gameplay/scripts/field_events.lua` contains source-traced map trigger
  declarations and the New Game scene handler. Trigger detection and repeat
  policy belong to the native event runner; a matched event starts a Lua
  coroutine, which requests heading-zero player movement through the shared
  actor service and waits for the native operation to finish.
- `gameplay/scripts/field_interactions.lua` declares the ordered choices for
  the shared Maglock and Security Terminal prompts. The native interaction
  controller maps these stable choice IDs to source commands and retains the
  passkey, Electronics, grenade, alarm, and map-state rules. Its text-key map
  binds those outcomes to the shared ROM-derived dialogue entries, so native
  result handling does not carry a second set of content aliases. All three
  menus return their exact ordered ROM label keys directly; the runtime does
  not compose the label keys from the choice IDs.
- `gameplay/scripts/field_mode.lua` selects the mode-text paragraph and
  portrait key from the signed field-mode/event context exposed by
  `game.field`. Native code keeps only the shared prompt's sentinel timing and
  input behavior.
- `gameplay/scripts/motel.lua` retains a reusable manager/room helper for
  legacy callers and non-quest motel flows. Stoker's production path uses its
  JSON conversation graph instead of the helper's optional quest-choice hook.
- `gameplay/scripts/dialogue.lua` owns field-speaker display names, named text
  profiles, portrait aliases, and explicit event/menu text bindings. Speaker
  display properties resolve from the active location; menu copy resolves
  from the location content profile, separately from whether a portrait is
  present. The generic Qt presenter receives resolved keys and does not infer
  text IDs from menu names or choice IDs.
- `gameplay/scripts/startup.lua` maps the three ROM archetype IDs to the
  extracted portraits shown by New Game. The menu's choice and cursor remain
  typed native state; archetype starting values still come from the matching
  ROM-derived record asset.
- `gameplay/scripts/michael_package.lua` registers named host effects for the
  Stoker conversation; `gameplay/dialogue/conversations/stoker_motel.json`
  owns page order, choices, conditions, result branches, and typed terminals.
  The indexed trigger root is gated by source facts from `0x53AB6`; the signed
  field-exit root models the positive/negative prompt and the E9 arrival
  command. Neither root uses `opening_dialogue_seen`, `michael_debt_paid`, or
  `michael_package_collected`. The room and debt commands remain typed native
  transactions, while JSON selects their confirmed outcomes. The unresolved
  80..99 encounter effect remains an explicit source boundary.
- `gameplay/scripts/shop.lua` declares all 12 service-title IDs and all 32
  service action-heading IDs in ROM order, with explicit per-row text keys
  returned from each Lua menu function. Its Ares catalog connects the Ares
  service title, six ordered categories and their heading IDs, available stock,
  universal clips text, and all nine transaction/detail text keys to the native
  shop controller. The controller still owns rendering, focus, input cadence,
  and wallet/inventory changes.
- `gameplay/scripts/items.lua` maps each source item ID to its ROM-derived
  display name and category keys, maps the 32 source detail rows used by field
  item/shop panels, and declares the four inventory-action IDs in source order.
  It also declares semantic IDs and explicit ordered text keys for inventory/
  First Aid copy, 13 weapon power grades, and 14 armor label lines.
  `FieldSceneRuntime` validates the action list and dispatches Equip/Use,
  Discard, and Trade through the selected stable ID; the text catalog resolves
  the UI and display copy from the Lua keys without duplicate runtime lists.
  QA checks key order and ROM copy. Item codes, cursor state, and inventory
  behavior remain typed native state.
- `gameplay/scripts/attributes_skills.lua` declares stable IDs and explicit
  ordered text keys for the 20 Attributes/Skills labels and five upgrade
  messages. Script QA checks every ID/key/ROM-text tuple, and field catalog QA
  checks the rendered catalog order. Skill values, cursor movement, upgrade
  eligibility, and karma spending remain native state.
- `gameplay/scripts/cyberware.lua` declares stable IDs and explicit ordered
  text keys for the Cyberware title, separator, empty state, and 15 implant
  names. Script and field-catalog QA check each ID/key/value in source order;
  native field state controls which implants are owned and how that page is
  presented.
- `gameplay/scripts/magic.lua` declares the 14 spell IDs in the ROM
  `D4EA0` name-table order, with explicit ordered Lua keys for every spell
  name and description. It also declares seven explicit keys for the shared
  Magic panel labels and headings. The field text catalog reads these menus
  from Lua and binds the ROM copy back to the typed presenter; script and
  catalog QA check every ID, key, value, and source ordering. The typed host spell catalog provides source facts, while Lua owns spell
  selection, force/posture input, and the seven-frame cursor cadence. Actor spell-effect timing and target fan-out live
  in `actor/actions.lua` and `actor/heartbeat.lua`; C++ forwards scene/record facts and presents its phase
  output. The non-magic
  `Magical Defense` fallback redraws the center content panel before its title and bars, so old
  status and item details do not remain underneath.
- `gameplay/scripts/hotel.lua` declares stable choice IDs for the Hotel
  Options and checkout cursor menus, including their explicit title and
  ordered choice-label keys. Its `text_keys()` map binds the separator,
  medical-attention suffix, and recovery-message rows. `FieldSceneRuntime`
  loads these bindings through `GameScriptRuntime`, validates choice order,
  and resolves the ROM copy into `HotelMenuRuntime`. Its typed C++ state still
  owns cursor input, layout, the E660 animation, status service, page
  transitions, and the separate Hotel Karma spending screen.
- `gameplay/scripts/pocket_secretary.lua` declares the five stable IDs for the
  field Pocket Secretary root menu and all five possible Notebook rows in ROM
  order. The root and Notebook definitions return explicit title and ordered
  label keys; the root separator has its own text key. It also names the
  dismiss choices, cyberdeck warning lines, grouped item rows, empty-run
  message, and Mr. Johnson contact/detail entries. Notebook Tips metadata and
  all 96 source-ordered entries are exposed as three validated 32-entry
  content pages. Script QA checks IDs, labels, and keys against the
  ROM-derived catalog. `FieldSceneRuntime` consumes the root and Notebook
  definitions directly and uses their IDs when dispatching the selected row;
  the text catalog resolves the keyed ROM labels from
  `assets/dialogue/script-text.json`. Lua now owns Notebook row gates, input,
  Tips flag filtering/page selection, Mr. Johnsons referral filtering/detail
  waits, C actions, returns, and E660 frame state. C++ supplies typed story
  facts and mirrors the route snapshot into the Qt painter state. The positive
  current-run record path, Contacts, Shadowrunners, restored-save fact import,
  Save/Load, and the extended six-row caller remain explicit boundaries.
- `gameplay/scripts/command_overview.lua` declares the seven stable IDs for the
  in-game command root in ROM order, including the two distinct Inventory
  columns, with each menu row paired to its explicit Lua text key. It also
  declares the metatype, class, and five posture labels used by its runner
  summary and selectable posture bar. Both the menu presenter and field text
  catalog consume these ID lists directly from Lua, while script QA checks ROM
  order and text. Typed native command state remains the sole owner of page
  selection, focus, rendering, posture effects, and action dispatch.
- `gameplay/scripts/clips.lua` declares the top-level Reload/Trade/Distribute/
  Cancel choices and the nested Increase/Decrease/Trade/Recipient choices.
  It also declares stable IDs for the Clips panel labels, reload/trade prompts,
  and status messages. Each definition returns explicit ordered text keys with
  its IDs, and the field menu catalog resolves the extracted copy from those
  Lua keys; QA checks key and source-text order against the ROM. The separate
  `menu/screens/clips.lua` and `menu/rules/clips.lua` modules own cursor,
  reload, trade, distribution, ammunition decisions, and result waits. C++
  supplies a typed three-runner snapshot, active slot, weapon capacity query,
  and ammo-record writes; the Qt painter mirrors Lua's route state.
- `gameplay/scripts/ui.lua` declares the shared Yes/No confirmation menu and
  shared field-stat caption IDs with explicit ordered text keys. The field
  text catalog resolves those keys without a duplicate label list in C++;
  script and catalog QA assert each ID/key pair, source order, and ROM copy.
  The Ares shop and inventory discard paths use the selected stable ID when
  applying the result. Cursor timing, default-No state, sound, and item/wallet
  effects remain native.
- `gameplay/scripts/startup.lua` declares the ordered New Game/Load Game,
  Save Slot, and archetype choices, with explicit optional title and ordered
  label keys returned from those Lua menu functions. The title-screen
  presenter validates each list against its typed native menu state and
  resolves the labels from the shared Lua text table. The same module binds
  semantic text IDs for the title, separators, archetype headings, and
  save/load feedback. The startup renderer keeps source-derived row placement;
  input cadence and menu transitions remain in C++. It also maps 22 typed
  opening-story caption IDs to Lua text keys. The offline importer reads those
  separate `0xFF`-terminated ROM records at `0x5B7D8..0x5BA4B`; the native intro
  schedule still chooses their timing, row, and palette.
Actor ownership target: the source has a behavior service pass at ROM `0xA8EA`
and a separate actor render pass at `0x4618`; Lua should receive one heartbeat
for each active ROM record in each corresponding pass. Lua packages own actor
AI, movement state and decisions, animation state, sound requests, and
scripted placement. C++ provides narrow host services for collision queries,
accepted-position commit, asset rendering, and audio playback. The current
runtime sends full behavior/render heartbeats to the controlled player and
extracted NPC profiles, and behavior/render heartbeats to connected weapon
projectile records; it does not yet enumerate all 16 live records. Lua owns the
player's target-heading selection/slew, collision-route decisions, and
animation cadence, the extracted NPC entry animations, finite weapon-list
animation, and projectile movement/recovery policy. For projectiles, native
services return terrain samples and the typed active-entity scan result; C++
applies Lua's record snapshot, draws named assets, and plays requested audio.
Map-event recovery and world-position commit remain native services; the
building route reads the named event-table facts and selects the destination in
Lua. The current field scene has no live collision candidates, so the
linked hit/status/damage continuation remains an explicit boundary. Most NPC
behavior and the full 16-record population remain unconnected.

- `gameplay/scripts/actor/targeting.lua` resolves Lua AI packages from a stable
  source-template profile ID plus the ROM's item/spell/none action class and
  code. Every active profile actor has a stable instance ID and receives a
  Lua behavior heartbeat on each field behavior-service pass and a render
  heartbeat on the actor render pass. Per-actor mutable state is
  stored in the shared `game.actors[actor_id]` registry, so other gameplay
  scripts can inspect or coordinate actor state without copying it through
  callback arguments. Lua owns entry delay, transition completion timer,
  direction, animation frame, and cadence; the Qt renderer consumes the
  returned snapshot without advancing a second clock. The player receives a
  behavior heartbeat each field service pass, where Lua maps the ROM's
  normalized D-pad table or a scripted override to a target heading, applies
  the `0xB530` `0x0800`-step slew, probes candidates through
  `game.movement.collision`, and chooses the accepted route/fallback. The
  `game.movement.route` service returns source marker and fallback data after
  the preceding terrain sample; `game.movement.recover_collision` invokes the
  existing map-event recovery boundary for preserved negative results and
  returns both the movement result and the updated `+0xB0` route-flags byte.
  Lua merges ROM `0xB7B8`'s one-shot event retry bit from that result into its
  movement state, while keeping the other route bits in the script snapshot.
  This retains the event latch through modal returns until `0xAC5E` observes
  clear terrain. A
  separate render heartbeat owns the player's C062/C052 animation clock and
returns the pose and C052/C062 selector. On a blocked reset, Lua takes the
visible idle facing from C052 selected via current heading `+0x2E`, because
ROM `0xB442` leaves `+0x36` unchanged. C++ applies Lua's selected coordinates
and selector to the field world model and keeps native collision/event state
synchronized.
  In the traced lists,
  `corp_guard_k05_item06` binds item 6, Max-Power (Heavy Pistol), and
  `pedestrian_k0b_none00` has no active item action. Each package selects its
  initial animation and requests its source cue; the audio host executes the
  request. This entry/idle branch is connected only for the current scene-2
  and scene-0x14 profile definitions, with scene 2 still inactive during New
  Game bootstrap. Player route-key derivation, negative-result policy,
  aligned fallback, diagonal alternate, movement flags, and blocked reset now
  run in Lua. Terrain sampling, route-table reads, map-event recovery, and the
  final host-world coordinate write remain native services. General NPC
  movement/collision policy and all live source actor slots are unfinished.
  The `weapon_projectile` package receives behavior and render heartbeats for
  each connected primary/secondary record. Lua advances finite descriptors at
  the ROM `0x5FDC`/`0x600E` cadence, runs the primary's four terrain/retry/entity
  probes, selects the stopped tail and `0x3C` audio request, decrements the
  stopped `+0x41` timer, and retires finite secondary records. The host exposes
  terrain and entity-scan callbacks, then applies the typed record snapshot.
  The active target list remains empty; linked actor/status/damage resolution
  and the positive `0x14F86` follow-up remain explicit boundaries.
  Temporary random-world encounter actors now receive Lua target selection,
  movement policy, entry/walk animation state, and B-selection marker state.
  `field_encounters.lua` also owns the `0x1E166` blocker, `0x3FF` cadence,
  probability, and active-sequence decisions; the host consumes source RNG
  only after Lua requests it. Enabled dialogue handlers and the end-to-end
  production encounter dispatch capture are still open.
  The script does not yet own default NPC movement/target selection, general
  combat choices, weapon fire, hits, or damage. The ROM call order,
  actor timers, target/movement hand-off, and current gaps are documented in
  [the actor heartbeat source audit](actor-ai-heartbeat-source-audit.md).
- `assets/dialogue/script-text.json` is the ROM-derived shared text table for
  opening-story captions, Stoker dialogue, field object interactions, Ares
  mode-event paragraphs, Hotel menu/status text,
  Pocket Secretary root and Notebook title/separator/labels,
  command-overview labels and shared confirmation labels,
  Magic spell names and descriptions by stable spell ID, field-service titles
  and 32 fixed-width shop action headings by stable Lua IDs, field-menu labels
  and feedback, all 96 Notebook Tips,
  startup and save/load copy, shop category headings and transaction copy,
  item names/categories/details, Clips prompts/status copy by stable IDs, and
  the ten runner names and biographies plus the currently converted Ricky,
  Winston Marrs, and Trent topic trees. The ROM importer checks the runner
  names and text-row counts before writing the catalog. Native presenters
  resolve those keys through the Lua runtime and validate the full field-menu
  key set at bootstrap. The
  generated `menu/field/service-routes.tsv` now contains only numeric service
  destinations used as routing data.
- Future `quests/` and `scenes/` modules should call the same `game` facade.
  Menus and dialogue use the generic Qt presenters, and Lua supplies the
  available stable choice ids and story text keys.

Current slices connect the New Game trigger, native-backed story facts,
Stoker's JSON graph roots, Hotel menu definitions, room/debt host transactions,
arrival/farewell/result dialogue, and the Qt presenters. The opening story
captions now resolve through the same Lua text resource while their
source-derived schedule and native rendering remain in the intro controller.
Field interaction, Ares mode-event paragraphs, Hotel text,
startup menu text/choices, Pocket Secretary root choices/labels,
command-overview root choices/labels, Clips selector choices/labels and panel
text keys, Pocket Secretary root and Notebook title/separator/label keys, Notebook option
IDs/labels, shared confirmation choices/labels, Magic spell
IDs/names/descriptions, field-object prompt choices, and shop
service/action-heading IDs are Lua content. Stoker's dialogue copy and choices
are JSON graph data, with Lua executing the graph and registering host actions.
Runner biographies and the
converted Ricky/Winston/Trent topic trees are grouped in `dialogue.lua`; the
shared panel resolves them through Lua and QA captures their screens. Their
conversation path still needs integration with the live roster/field UI.
Script dialogue menus carry
their text profile and optional speaker into one generic panel projection;
menu choices use generic presentation roles. Hotel cursor-menu choices come
from a shared Lua module while the native Hotel presenter draws the source
layout and applies input rules. The Pocket Secretary cursor and child pages
remain native. Wallet and quest effects remain native service
implementations. Ares shop stock is script data, but its selector, transaction
loop, and native UI still run in C++. Other building dialogue, remaining
title/field presentation, item operations, inventory screens, general cutscene
actor placement and animation, audio playback from Lua, and save restoration
remain to be migrated or connected. The source-profiled NPC entry policy is
the first actor package used by the live field update loop.

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
  `export/SCRATCHPAD.md`, section
  `The startup distinction is explicit in the conversion source`.
- Physical Stoker route:
  `docs/ghidra/visual-audits/motel-doorway-scene-entry.md`.
- Lua's embedding API, version history, and MIT license are documented by the
  [official Lua manual](https://www.lua.org/manual/5.4/manual.html),
  [version history](https://www.lua.org/versions.html), and
  [license](https://www.lua.org/license.html).
