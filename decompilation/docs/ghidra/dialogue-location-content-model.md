# Dialogue and location interaction content model

Status: design, source inventory, and staged implementation. The generated
text catalog, speaker/menu/contact catalogs, and location condition catalog
are JSON-backed. All 87 current entity link rows now reference named JSON
conditions where needed. The conversation graph engine, typed terminal
outcome bridge, and entity-owned JSON interaction catalogs remain pending.
This does not claim every listed route is implemented or visually verified.

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
| 0 | `0x53AB6` | Stoker | Prompt roots vary with field/run context and signed alternate result; page, A/B/C choice, payment, job, and later-referral paths. |
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
| Continue in place | Gates, Red Buffalo, Owlfeather services | Close the interaction and keep the current scene/event context. Do not follow a raw event-table destination suppressed by the source handler. |
| Resume pending route | Fuchi and Renraku entry pages | Close the page, then continue the already selected event-table route into the source activity. |
| Return through the saved entrance | Hotel/building checkout and activity exit | Restore the saved origin scene, camera/door return point, and source-derived heading policy. |
| Enter a named field scene | Sharkey travel; the Tarislar Tomb offer | Begin the typed scene transition with its destination and arrival state. |
| Enter an activity route | Office entries and other event-table destinations | Use the pending source route and its resource/activity entry setup. |
| Open a host service | Hotel Options, Hospital Options, shops, taxi, Matrix | Hand off to a separately owned service screen and carry an explicit return policy. This is a yield/handoff, not an implicit dialogue return. |
| Play a cinematic/sequence | Extensible outcome only | The type may be supported, but no surveyed building/phone dialogue terminal has been proven to play a movie. Do not assign one without a ROM trace. |
| Stop at a source boundary | Tabatha's unconverted encounter commands; unresolved phone facts | Diagnostic state outside the normal game outcome union; retain source command/provenance and do not pretend the route completed. |

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
presentation states; it does not expose every unchosen ROM branch.

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

Seven contacts currently have Lua service modules: Captain Quinton, Alesandro,
Sharkey, Kipp, Wilma, Macklemann, and Gregory. Their source result permutations
include:

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
`dialogue.lua` currently holds the 13-contact catalog, speaker profiles, and
many menu-label profiles while seven separate modules hold service menus and
branch flow. The JSON model will put shared labels and graph structure with
the content, while named Lua actions/services retain state mutation policy.

## Proposed JSON content layout

```text
gameplay/scripts/content/dialogue/
  catalog.json                 # speaker IDs, portraits, common UI profiles
  conditions.json              # named, composable condition expressions
  conversations/
    stoker.json
    tarislar_city_inn.json
    ...
  vid_phone_contacts.json      # 13 ordered records, call-history facts,
                               # first/repeat graph IDs, service graph IDs
gameplay/world/entities/<id>/
  interactions.json            # source event routes, conditions,
                               # linked dialogue and route outcomes
decompilation/assets/dialogue/
  script-text.json             # generated ROM text catalog; source strings
                               # and page delimiters remain extractor-owned
```

The ROM extractor remains the owner of exact imported strings. It should emit
`script-text.json` with the same stable keys and retained page/line separators
as today's `script-text.json`; authored graph files refer to those keys instead
of copying text. This preserves text provenance, avoids two sources for one
sentence, and allows a later localization catalog to use the same graph.

Location interaction JSON replaces the nested route/task columns in
`links.tsv`; map geometry, collision records, and event-table destinations
remain in the existing map/source-data pipeline. A route may explicitly say
`resume_source_route`, `suppress_source_route`, `enter_scene`, or
`return_to_origin`. That records the source distinction without duplicating a
map event table or embedding ROM address arithmetic in runtime code.

### Condition language

Conditions are a small typed expression tree, not Lua strings or a custom
semicolon mini-language. The evaluator supports `all`, `any`, `not`, and
typed fact comparisons (`eq`, `ne`, `lt`, `lte`, `gt`, `gte`, `in`). Facts use
semantic names such as `story.motel_bill_paid`, `phone.contact_available`,
`run.active_type`, `field.alternate_result`, `party.active_runner.body`, and
`inventory.has_item`. A fact registry maps those names to Lua-owned game state
or a narrow typed host query. Source bits/addresses belong in evidence metadata,
not in condition evaluation.

Evaluation is tri-state: `true`, `false`, or `unknown`. `unknown` means a
required save field or source boundary has not been implemented. Route
resolution must expose that boundary rather than treating unknown as false and
falling through to a different building destination.

Example:

```json
{
  "schemaVersion": 1,
  "conditions": {
    "tarislar_inn.paid_stay_service": {
      "all": [
        { "fact": "field.scene_id", "op": "eq", "value": 1 },
        { "fact": "field.event_code", "op": "eq", "value": 1 },
        { "fact": "story.motel_bill_paid", "op": "eq", "value": true },
        { "fact": "story.tarislar_asked_shale", "op": "eq", "value": false },
        { "fact": "field.alternate_result", "op": "eq", "value": 0 }
      ]
    }
  }
}
```

### Conversation graph language

Each graph has a stable ID, entry node, default speaker/presentation profile,
nodes, and ROM provenance. Nodes are limited to reusable forms:

- `page`: text key, optional portrait/speaker override, continuation input,
  and next node.
- `choice`: ordered choices with stable IDs, label keys, optional `when`,
  input contract (`buttons_abc`, `cursor_list`, or `yes_no`), and edges.
- `branch`: ordered condition cases and a required explicit fallback.
- `action`: ordered registered Lua effects or a yielded typed host service.
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

## Required validation and migration order

1. Add a JSON schema version and validator for unique graph/node/choice IDs,
   valid references, explicit branch fallback, valid typed outcomes, allowed
   input contracts, and source metadata on converted routes.
2. Convert the generated ROM text catalog to JSON and update its consumers;
   do not edit extracted text by hand.
3. Move speaker profiles, menu labels, and the 13-contact metadata out of
   `dialogue.lua` into JSON while preserving current keys and behavior.
4. Implement the reusable graph/condition engine and typed outcome bridge.
   Remove string parsing such as `matrix_entry_<ids>` and per-screen result ID
   switches as each outcome family is converted.
5. Convert simple, distinct route outcomes first: one-page same-map service,
   entry page that resumes an activity route, saved-origin return, and a
   typed service handoff. Then migrate conditional building routes and phone
   transactions/travel using the same graph engine.
6. Convert each location's existing link variants to entity-owned JSON with
   structured conditions and explicit source-route policy. Never infer an
   interior or destination from a building's name.
7. For each defended hill, test all source-relevant gates and branch outcomes,
   test unknown facts as boundaries, exercise physical input where applicable,
   and save/review a complete `actual.bmp`. Keep the status partial wherever
   independent longplay evidence or source work is still missing.

The first JSON graph migration should be a vertical slice that exercises more
than one terminal kind (for example an in-place service and an entry page that
resumes the source route). That validates the outcome contract before moving
larger quest trees such as Stoker, the Salish dialogues, and the contact
services. Existing incomplete encounter commands, upstream quest producers,
and unverified visual branches remain explicit work, not filled-in defaults.

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
boundary are covered by focused Lua and C++ tests; full conversation-graph and
typed terminal-outcome behavior remains the next implementation slice.
