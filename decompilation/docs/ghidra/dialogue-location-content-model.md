# Dialogue and location interaction content model

Status: design, source inventory, and staged implementation. The generated
text catalog, speaker/menu/contact catalogs, and location condition catalog
are JSON-backed. All 87 current entity link rows reference named JSON
conditions where needed. The Lua graph engine and typed outcome bridge drive
Gates Undersound's same-map service, the Renraku and Fuchi gated entry pages,
Tarislar City Inn, Wylie's Gala Inn, and the two Stoker entry roots. The field
host dispatches the `hotel.options` and `hospital.options` service IDs.
Captain Quinton's first/repeat call and complete service tree is a JSON graph;
Alesandro's service tree is also JSON-backed. Their Lua modules register named
host actions. Five other contact service trees remain procedural. Remaining
conversation routes and other service outcome
dispatchers are pending; this does not claim every listed route is implemented
or visually verified.

## Design goals

- Keep dialogue copy, speaker/presentation metadata, choices, conditions, and
  location destinations in readable JSON with stable IDs.
- Keep the authoritative active conversation, branch selection, and gameplay
  effects in Lua. JSON describes the graph; it is not executable code.
- Use typed host requests and outcomes. A new building or phone branch should
  select a known outcome in data/Lua instead of requiring another string case
  in the Qt coordinator.
- Preserve source provenance on each route and graph edge. ROM addresses are
  evidence only; runtime data uses semantic IDs and extracted destinations.
- Treat missing source facts and unconverted commands as explicit boundaries,
  not as `false`, a silent default, or a successful return.
- Keep the graph engine and route policy independent of Qt so another host can
  provide the same typed services and presentation.

## Source and longplay findings

### ROM route selection

ROM `0x1F5CA` services a field event in order: mode resources and flags,
world-effect handling, the 23-entry indexed title-service table at
`0x15FD5C`, the event prompt at `0x1F65A`, mode exit, and the generic field
mode service. A handler can consume the event (`D6 < 0`), preserve the pending
event-table destination (`FFF0CC < 0`), or allow the ordinary transition to
continue. Therefore a dialogue ending cannot be represented by one generic
“close and return” result.

The indexed table has 13 active slots and 10 no-op slots. The active slots
known from the US ROM and current route audits are:

| Slot | Source handler | Interaction | Source-backed branch/result shape |
| ---: | --- | --- | --- |
| 0 | `0x53AB6` | Stoker | Three-choice root `0xF4AFC` only when FFFC1E bit 0 is clear, scene/event is 2/2, and signed alternate result is 0. `0x1F65A` separately selects arrival, positive package question, or negative package handoff. Arrival `<MORE>` runs E9: rolls 0..79 select the two-choice manager record `0xF4AB4`; 80..99 enter the unresolved effect sequence. |
| 1 | `0x53B16` | Tarislar City Inn | Requires scene/event 1/1, story bit 0 set, bit 1 clear, alternate result 0. Same-map service; successful 30-nuyen stay opens Hotel Options. |
| 2 | `0x53B78` | Owlfeather first contact | Scene 6/event 0; bit 1 set and bit 2 clear. Three initial choices, with the brother choice applying command `0x17`. |
| 3 | `0x53BCC` | Owlfeather search follow-up | Bit 2 set and bit 3 clear. Topic branches and a later/leave path. |
| 4 | `0x53CEA` | Owlfeather returned-brother follow-up | Bit 3 set and bit 4 clear. Returned-brother and Medicine Lodge referral branches. |
| 5–9 | `0x49A0` | No-op slots | No indexed service branch. |
| 10 | `0x53F8C` | Boris Greenhouse | Scene 2/event 6 service suppresses the raw Scene-26 route; story and item facts gate topics and paid offers. |
| 11 | `0x49A0` | No-op slot | No indexed service branch. |
| 12 | `0x54076` | Renraku Offices | Scene 5/event 6, bits 16/12 and alternate result gate an entry page, then the source activity route continues. |
| 13 | `0x49A0` | No-op slot | No indexed service branch. |
| 14 | `0x54152` | Fuchi Offices | Scene 3/event 2, bits 25/14 and alternate result gate an entry page, then the source activity route continues. |
| 15 | `0x54222` | Gates Undersound | Scene 0/event 3, bits 11/15 and alternate result gate a one-page same-map departure service. |
| 16–17 | `0x49A0` | No-op slots | No indexed service branch. |
| 18 | `0x54322` | Red Buffalo Woman | Scene 6/event 1; bits 17/18 select the gift vs. no-gift Harlequin graph; both suppress Scene 13 and return to Scene 6. |
| 19 | `0x543A2` | Ilene route preparation | Scene-2/event-0 story/effect gate prepares a later Scene-20 entry; it is not a dialogue terminal. |
| 20 | `0x49A0` | No-op slot | No indexed service branch. |
| 21 | `0x5447E` | Tarislar Apartments | Scene 1/event 7; signed alternate result, first/repeat state, conditional information, guarded leave, and gated Tomb offer. |
| 22 | `0x545B6` | Redmond Vigore search | Scene 2/event 7; prerequisite bit 13 and unconsumed bit 10 gate three narrative pages, rewards, and a same-map return. |

The table is one source-ordered service layer, not a complete count of game
dialogues. Other routes include ordinary event-table doors, `0x1F65A` prompt
families, resource-scene services, field terminals, service menus, and run
contracts. Current entity link data contains 87 route rows across 70 entities
(58 area-event rows, 8 area-event overrides, 13 dialogue rows, and 8 field
services). This is the checked-in route catalog size, not proof that the ROM
has only 70 locations.

The source-supported terminal distinctions already required by these routes
are:

| Terminal behavior | Example | Host meaning |
| --- | --- | --- |
| `continue_in_place` | Gates, Red Buffalo, Owlfeather services | Ends dialogue and keeps the current scene/event context. Do not follow a raw event-table destination suppressed by the source handler. |
| `resume_source_route` | Fuchi and Renraku entry pages | Ends the page, then continues the already selected event-table route into the source activity. |
| `return_to_origin` | Hotel/building checkout and activity exit | Ends dialogue and restores the saved origin scene, camera/door return point, and source-derived heading policy. This represents “ends and sends outside” when the saved origin is outdoors. |
| `enter_field_scene` | Sharkey's paid travel offers; the Tarislar Tomb offer | Ends dialogue and enters a named scene after the typed destination is validated against the completed source service, using the named ROM arrival policy. |
| `enter_activity` | Office entries and other event-table destinations | Ends dialogue and enters the pending source route with its resource/activity setup. |
| `open_service` | Hotel Options, Hospital Options, shops, taxi, Matrix | Ends the current graph and hands off to a separately owned service with an explicit return policy. This is a yield/handoff, not an implicit dialogue return. |
| `play_cinematic` | Extensible outcome only | Ends dialogue and plays a named cinematic/sequence. No surveyed building/phone terminal is proven to use it, so do not assign one without a ROM trace. |
| `source_boundary` | Tabatha's unconverted encounter commands; unresolved phone facts | Diagnostic state outside the normal game outcome union; retain source command/provenance and do not pretend the route completed. |

### Prompt and choice shapes

The shared ROM prompt loop (`0x1F246`, `0x1F3C4`) supports more than a
message-plus-choice screen:

- A text page with an A-button continuation such as `< MORE >`.
- A title-only or blank-text page that waits for input.
- A fixed button menu with one, two, or three displayed choices bound to A/B/C.
- A conditionally filtered choice list, where removing an option changes the
  visible button order.
- A cursor list using Up/Down, confirm, cancel/back, and optional paging rows.
- A result page followed by a return-to-services choice, a farewell choice, or
  a terminal handoff.
- A source transaction that yields to a host operation and branches on a
  structured result such as paid/insufficient, purchased/out-of-stock,
  inventory-full, ineligible, or passage-requested.

These are different input contracts. A `MORE` page is not a two-choice menu;
the Vid-Phone contact list is not an A/B/C prompt; and the Hospital Exit
confirmation ignores Start and returns to Options when No is chosen.

### Longplay evidence

The hand-reviewed first 12 minutes and direct 320x224 frame review show the
same panel shell serving separate states: Stoker's opening page at section 03
frame 000082, three-choice manager menu at 000301, payment/debt prompt at
000601, and title-only Jump House referral at 000690. The lower choice area is
replaced between these states. This supports page, menu, and title-only
presentation states; it does not expose every unchosen ROM branch. The 000301
three-choice panel is the indexed trigger-0 graph rooted at `0xF4AFC`. It is
not the two-choice `0xF4AB4` manager panel reached after the arrival page's E9
roll, which has a separate 0..79 route and is covered by a different graph
entry.

Later longplay frames also show dialogue presentation variants: section 05
frame 001440 (Max portrait/text page), section 14 frame 006630 (Henry portrait
with Yes/No rows), and section 23 frame 001770 (A/B services/goodbye choice).
Section 32 frame 001500 shows the Big Rhino/steak portrait asset used by the
Macklemann profile. These frames support the shared panel/portrait/choice
model. There is no matching complete Vid-Phone call sequence in the longplay;
the ROM call descriptors and handler paths, not those isolated frames, define
the contact permutations. Longplay timestamps and holds are visual sequence
anchors, not ROM frame timing.

## Vid-Phone permutation inventory

The ROM list at `0xC66AC`/`0x109682` has 13 ordered contacts. Contact access
and call history are separate facts. The field flow filters unavailable names,
shows up to seven contacts plus Next on page one, then the remaining six plus
Prior on page two. A first call can differ from a repeat call; two contacts
have special first-call/history behavior. Calls may present a one-page or
multi-page first/repeat message, a Services/Goodbye menu, or a contact-specific
service graph.

Seven contacts have source-derived service implementations: Captain Quinton,
Alesandro, Sharkey, Kipp, Wilma, Macklemann, and Gregory. Quinton's complete
first/repeat call and service flow is now a JSON graph; Alesandro's service
flow is a second graph. The other five service flows remain procedural Lua
modules pending migration. The traced result
permutations include:

- Information-only topics and multi-page lessons, with return-to-services or
  disconnect/farewell outcomes.
- Purchases/hiring with insufficient funds, unavailable state, out-of-stock,
  eligibility, storage-full, and success results.
- Item/program/deck grants and contact/history flag writes.
- Sharkey's 3,000-nuyen Scene-6 passage, 5,000-nuyen Scene-9 passage, and
  3,000-nuyen Smart Goggles offer. A successful passage ends the call and
  transitions to the destination; insufficient funds returns to services.
- Result screens that either return to a service menu or end the call, as
  distinct from result screens that start travel.

The remaining six list contacts still have first/repeat text and portraits;
their contact-specific service behavior is not inferred from those facts.
`contacts.json`, `speakers.json`, `menus.json`, `conditions.json`, and
`conversations/*.json` now hold shared content and converted graph structure.
Named Lua actions and typed host services retain state mutation policy. Six
contact modules still own procedural service flow and are explicitly pending.

## JSON content and entity-route layout

```text
decompilation/gameplay/dialogue/
  contacts.json                # 13 ordered records and graph scope bindings
  speakers.json                # speaker IDs, portraits, event text bindings
  menus.json                   # stable choice IDs and ordered label bindings
  conditions.json              # named, composable typed conditions
  conversations/<graph>.json   # pages, choices, branches, effects, outcomes
  location_conditions.json     # source-derived location route conditions
decompilation/gameplay/world/entities/<id>/
  interactions.json            # target entity-owned event/route bindings
decompilation/assets/dialogue/
  script-text.json             # generated ROM text catalog and page breaks
```

The ROM extractor remains the owner of exact imported strings and emits
`script-text.json` with stable keys and retained page/line separators. Authored
graph files refer to those keys through speaker and menu bindings instead of
copying text. This preserves text provenance, avoids two sources for one
sentence, and allows a later localization catalog to use the same graph.

Entity interaction JSON is the target replacement for nested route/task
columns in `links.tsv`; unconverted route rows remain there until source gates
and destinations are traced. Map geometry, collision records, and event-table
destinations remain in the existing map/source-data pipeline. A route may say
`resume_source_route`, `suppress_source_route`, `enter_scene`, or
`return_to_origin`. That records the source distinction without duplicating a
map event table or embedding ROM address arithmetic in runtime code.

### Condition language

Conditions are a small typed expression tree, not Lua strings or a custom
semicolon mini-language. `conditions.json` entries wrap an `expression` plus
ROM provenance. The evaluator supports `all`, `any`, `not`, and typed fact
comparisons (`eq`, `ne`, `lt`, `lte`, `gt`, `gte`, `in`). Facts use semantic
names such as `story.vid_phone_called_0`, `story.motel_bill_paid`,
`run.active_type`, and `field.alternate_result`. A fact provider maps those
names to Lua-owned game state or a narrow typed host query. Source bits and
addresses belong in evidence metadata, not in condition evaluation.

Evaluation is tri-state: `true`, `false`, or `unknown`. `unknown` means a
required save field or source boundary has not been implemented. Route
resolution must expose that boundary rather than treating unknown as false and
falling through to a different building destination.

Example:

```json
{
  "schemaVersion": 1,
  "conditions": {
    "vid_phone.quinton_contact_not_called": {
      "expression": {
        "fact": "story.vid_phone_called_0", "op": "eq", "value": false
      },
      "provenance": {
        "romAddresses": ["0xE5E2", "0x109682", "0x1096B6"],
        "scope": "contact_0_call_history_clear"
      }
    }
  }
}
```

### Conversation graph language

Each graph has a stable ID, entry node, default speaker/presentation profile,
nodes, and ROM provenance. Nodes are limited to reusable forms:

- `page`: speaker event ID, optional speaker/text-profile override, and next
  node. Speaker JSON binds the event to extracted text and portrait metadata.
- `choice`: menu ID, ordered stable choice IDs with graph edges, optional
  condition references, and input contract (`buttons_abc`, `cursor_list`, or
  `yes_no`). Menu JSON owns title and choice-label keys, including inline
  prompt text when the ROM shows copy and choices together.
- `branch`: ordered condition cases and a required explicit fallback.
- `action`: one registered Lua effect or yielded typed host service, with
  explicit result-ID edges when the operation can return multiple outcomes.
- `terminal`: exactly one typed outcome after all ordered effects complete.

Graph node IDs are content IDs, never source addresses. A source command may
appear as provenance on the edge/action that represents it. Effects are named
operations (`apply_dialogue_command`, `charge_building_amount`,
`award_party_karma`, `adjust_runner_triplet`, `set_story_fact`) with typed
parameters; JSON cannot call arbitrary Lua functions. Complex quest policy
stays in named Lua actions and is unit-testable without the presenter.

The `terminal` outcome is a discriminated union, not a free-form result string:

```json
{ "type": "continue_in_place" }
{ "type": "resume_source_route" }
{ "type": "return_to_origin", "returnPoint": "saved_door_entry" }
{ "type": "enter_field_scene", "sceneId": 54, "arrivalId": "tomb_entry" }
{ "type": "enter_activity", "activityId": "resource_scene_21" }
{ "type": "open_service", "serviceId": "hotel.options", "returnPolicy": "origin" }
{ "type": "play_cinematic", "cinematicId": "source_confirmed_id", "returnPolicy": "resume_origin" }
```

`open_service` and other asynchronous operations carry their return policy;
they are not aliases for `continue_in_place`. `play_cinematic` is part of the
extensible type set but remains unused until a dialogue route is source-traced.
An unresolved condition or unconverted source command is a separate diagnostic
boundary result and is never serialized as a successful terminal outcome.

## Runtime object responsibilities

| Object/service | Owns |
| --- | --- |
| `DialogueCatalog` | Immutable JSON definitions, stable-ID lookup, schema/reference validation. No active session state. |
| `LocationInteractionResolver` | Match typed scene/event/context against entity interactions; apply explicit priority; reject ambiguous matches. |
| Lua `DialogueSession` / `DialogueController` | Current graph/node, filtered options, input-to-choice mapping, ordered branch/effect execution, and the authoritative outcome. |
| `ConditionEvaluator` + fact provider | Pure tri-state evaluation over named typed facts. It does not write game state. |
| `ActionRegistry` / host service interface | Run named Lua gameplay effects and yield typed requests for wallet, inventory, party, shop, taxi, phone, activity, and travel operations. |
| `DialogueViewModel` | Immutable current page/menu projection: speaker, portrait ID, text keys/resolved text, visible ordered choices, selected row, input contract. |
| Qt dialogue presenter | Draw the view model and forward focused-window input. It never selects story branches or changes world state. |
| `DialogueOutcomeDispatcher` | Translate a completed typed outcome into existing field/scene/service coordinators; owns saved location-return application. |

Lua remains the one owner of active story/dialogue state. C++ types validate
requests and outcomes and provide host services; Qt remains a presenter. No
second C++ dialogue graph or menu cursor state machine is introduced.

For portable testing, the graph engine receives interfaces for facts, input,
effects, and outcome dispatch. A fake provider can drive every branch without
Qt. The JSON parser is an adapter behind the catalog loader; the graph,
conditions, and outcome types must not depend on Qt JSON classes.

## Validation and migration checkpoints

1. **Implemented:** versioned JSON schema validation checks stable graph,
   node, and choice IDs; references; explicit branch fallbacks; typed
   outcomes; input contracts; and ROM provenance.
2. **Implemented:** the ROM extractor emits the dialogue text catalog as JSON
   and consumers reference stable text keys. Extracted strings remain
   extractor-owned.
3. **Implemented for shared catalogs:** speaker, menu, and 13-contact data are
   JSON-backed. Captain Quinton's full call flow and Alesandro's service flow
   use JSON graphs; five other contact service flows remain procedural Lua.
4. **Implemented with explicit host boundaries:** the reusable Lua graph and
   condition engines and typed `DialogueOutcome` union are active. The host
   dispatches only connected outcome families; return, other activity/service,
   and cinematic transitions still stop at named boundaries. Remove legacy
   result strings as each family is source-traced and migrated.
5. **Ongoing:** migrate one source-traced behavior hill at a time, covering
   same-map services, pending route continuation, return through the saved
   origin, typed service handoffs, conditional building gates, and phone
   transactions/travel. Do not infer an interior or destination from a
   building's name.
6. **Ongoing:** move each entity's traced interaction routes from nested TSV
   route/task columns into entity-owned JSON with structured conditions and
   explicit source-route policy. Keep unconverted routes in the existing
   source-data path until their gates and destinations are known.
7. **For each defended hill:** test all source-relevant gates and outcomes,
   test unknown facts as boundaries, exercise physical input where applicable,
   and save/review a complete `actual.bmp`. Keep status partial wherever
   independent longplay evidence or source work remains missing.

The first JSON graph migration was a vertical slice that exercises more than
one terminal kind. Stoker's indexed service and signed field-exit prompt now
use separate JSON graph roots. The remaining Stoker work is source coverage
for the 80..99 effect callback, saved-game fact restoration, and independent
visual checks. Quinton's call graph and Alesandro's service graph are
converted; five other contact service flows and the Salish dialogues still
need conversion. Existing
incomplete encounter commands, upstream quest producers, and unverified visual
branches remain explicit work, not filled-in defaults.

## Implemented condition slice

`gameplay/dialogue/location_conditions.json` now contains versioned, stable-ID
condition definitions for every nontrivial gate in the 87 current entity link
rows. The corresponding `links.tsv` rows carry `route_condition_id` and
`entry_condition_id` references; semicolon conditions, the separate entry
alternate-result field, and building-interaction mode gates have been removed
from those route records. Existing source addresses and branch evidence stay
with the link row.

`condition_evaluator.lua` evaluates the `all`, `any`, `not`, `eq`, `ne`, `lt`,
`lte`, `gt`, `gte`, and `in` forms against Lua state and typed route context.
It returns `true`, `false`, or `unknown`. Unknown facts and unresolved
same-priority routes become typed `source_boundary` route results, so they do
not select a default event-table destination. Known false conditions still
allow the source default route. The condition evaluator and the source-route
boundary are covered by focused Lua and C++ tests.

## Implemented graph and outcome slice

`dialogue_graph.lua` is a Lua-owned, object-oriented graph runner. Its bounded
catalog validates node and edge references, named condition references,
speaker/text bindings, ROM provenance, input contracts, and terminal payloads.
Its session handles pages, ordered and condition-filtered choices, A/B/C and
yes/no input, cursor-list input, ordered condition branches, registered named
actions, and typed terminal outcomes. The host adapter yields the current
page/menu to the existing presenter and returns controller facts. Effects are
resolved by `dialogue_actions.lua`; JSON cannot call arbitrary Lua code.

The C++ script boundary parses a discriminated `DialogueOutcome` value:
`continue_in_place`, `resume_source_route`, `return_to_origin`,
`enter_field_scene`, `enter_activity`, `open_service`, `play_cinematic`, and a
diagnostic `source_boundary`. Route data can declare the expected typed result
for a pre-entry page. The field presenter dispatches `continue_in_place`,
`resume_source_route`, the Sharkey travel form of `enter_field_scene`, and the
named `hotel.options`/`hospital.options` service screens. Return, activity,
other service-screen, and cinematic outcomes still stop at an explicit host
boundary until their coordinators are source-traced and connected.

Seven non-Stoker graphs and two Stoker roots currently exercise that contract:

- Gates Undersound trigger 15 performs the ordered trigger/page/effect-flag/
  trigger-tail operations and returns `continue_in_place`, suppressing event
  table row 3's resource activity 21.
- Renraku title service 12 performs its trigger/page and returns
  `resume_source_route`; the route link expects that type before loading the
  already selected activity 35. Its old `enter_scene_35` result string has
  been removed from this route.
- Fuchi title service 14 performs its trigger/page and returns
  `resume_source_route`; the route link expects that type before loading the
  already selected activity 21. Its old `enter_scene_21` result string has
  been removed from this route.
- Tarislar City Inn service 1 is a JSON graph with its intro, room/Leave/Tabatha
  choices, 30-nuyen payment, insufficient-funds branch, and same-map return.
  A successful payment yields `open_service` for `hotel.options` with the
  `origin` return policy. Its named Tabatha action remains Lua-owned, and the
  unconverted runner/ambush commands remain explicit source boundaries. The
  graph/payment outcome is covered by the field-dialogue runtime test; the
  FieldScene QA separately checks physical entry/Leave and exercises the
  production hotel dispatcher through a controlled typed-outcome seam, so it
  does not claim one end-to-end paid field transaction.
- Wylie's Gala Inn service 1 is a JSON graph with its signed run-result pages,
  inline entry and room prompts, runner-status free-stay gate, 50-nuyen wallet
  action, and paid/insufficient branches. Successful paid or free paths return
  typed `open_service hotel.options` with `origin`; same-map choices return
  `continue_in_place`. Debug/Release tests cover each graph permutation, and
  physical QA walks to the inn and completes the paid path through Hotel
  Options. Independent Genesis visuals remain unavailable.
- Captain Quinton's Vid-Phone record 0 is a JSON graph with a condition-driven
  first/repeat split, the Services/Goodbye menus, Hollywood/Weller information
  loops, Firepower purchases, and Sharkey hiring. The action adapter records
  call history and submits typed transactions; JSON selects success,
  insufficient-funds, out-of-stock, or explicit source-boundary results. The
  direct Lua suite covers the declared permutations, and Debug/Release
  Vid-Phone QA covers the call panel and ordinary physical entry/return path.
  The reviewed longplay has no matching complete Quinton call sequence, so the
  graph remains partial for independent visual proof.
- Alesandro's Vid-Phone service tree is a JSON graph reached after the shared
  first/repeat call menu. It covers three corporate-security topics (including
  the two-page patrol route), level-3 and level-5 passkey transactions, and
  Macklemann's contact offer. Purchased/hired and out-of-stock results lead to
  Services/Goodbye menus; insufficient funds ends on its page; unavailable or
  unknown service results stop at a source boundary. A direct Lua suite covers
  these permutations, and Debug/Release script/panel QA passes. The longplay
  has a shared Alesandro/Professor portrait reference but no complete matching
  Alesandro call sequence.

- Stoker has two JSON roots so the indexed trigger and signed field-exit
  selector stay separate. The indexed root uses the ROM gate from `0x53AB6`
  (scene/event 2/2, alternate result 0, FFFC1E bit 0 clear), presents the
  three-choice `0xF4AFC` menu, and models the room, debt, package handoff,
  referral, and typed Hotel Options branches. The field-exit root models the
  signed `0x1F65A` prompt variants, the E9 0..79 two-choice manager route, and
  an explicit boundary for the unresolved 80..99 effect callback. Lua tests
  cover the declared branch permutations; Debug CTests exercise both roots
  through the field presenter. Saved-game restoration, the rare effect
  handoff, and independent visuals remain open.

The Lua graph tests cover branch fallback, unknown-condition boundaries,
condition-filtered choices, cursor input, registered action result edges,
pages, inline prompt text, and every declared successful outcome shape. The
C++ runtime test advances the five earlier route graphs through their
presenter and service yields and checks typed outcomes and ordered story
effects. Focused Debug CTests exercise both Stoker roots through the field
dialogue presenter. Debug and Release coverage for all implemented graph
routes remains in progress. Full physical route QA and independent Genesis
comparison remain separate evidence; the reviewed
longplay shows shared panel/menu examples but no matching Gates departure or
Renraku/Fuchi first-entry sequence or Wylie's Gala Inn interaction.
