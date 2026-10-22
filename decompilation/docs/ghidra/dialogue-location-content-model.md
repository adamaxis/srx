# Dialogue and location interaction content model

Status: design, source inventory, and staged implementation. The generated
text catalog, speaker/menu/contact catalogs, conversation graphs, and location
condition catalog are JSON-backed. The 86 remaining TSV route rows reference
named JSON conditions where needed; Boris Greenhouse's scripted service route
is now stored in entity-owned interaction JSON. The Lua graph engine and typed outcome
bridge drive Gates Undersound's same-map service, the Renraku and Fuchi gated
entry pages, Tarislar City Inn, Wylie's Gala Inn, and the two Stoker entry
roots. The field host dispatches the `hotel.options` and `hospital.options`
service IDs.
Captain Quinton's first/repeat call and complete service tree is a JSON graph;
Alesandro's, Sharkey's, Kipp's, Wilma's, Macklemann's, and Gregory's service
trees are also JSON-backed. Max's and Professor Jefferson's first/repeat calls
and complete service trees are now JSON graphs as well. Owlfeather's
scene-6/event-0 services 2, 3, and 4 now share a JSON graph with explicit route
conditions and named Lua action adapters. Their Lua modules register named
host actions. Boris Greenhouse's currently modeled Scene-2/event-6 branches
also use the shared JSON graph and typed `continue_in_place` outcome. Other
conversation routes and service outcome dispatchers remain pending; this does
not claim every listed route is implemented or visually verified.

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
- Use small domain objects with explicit ownership: catalogs validate named
  definitions, a dialogue session owns one in-progress graph, condition and
  action registries resolve stable IDs, and the outcome dispatcher hands a
  completed route to a typed host service. Persistent gameplay facts remain
  Lua-owned; transient input and presentation state belongs to its session.
- Treat loaded JSON content as an immutable package revision. The runtime
  caches its catalog by package identity and validates each graph when first
  selected; offline and unit-test construction can validate every graph
  eagerly. Replacing the package table is the reload boundary.

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
| `return_to_caller` | Vid-Phone contact graphs | Ends a nested interaction and gives control back to the owning phone session, which applies its call-completion behavior. It does not mean “stay in this field scene.” |
| `resume_source_route` | Fuchi and Renraku entry pages | Ends the page, then continues the already selected event-table route into the source activity. |
| `return_to_origin` | Hotel/building checkout and activity exit | Ends dialogue and restores the saved origin scene, camera/door return point, and source-derived heading policy. This represents “ends and sends outside” when the saved origin is outdoors. |
| `enter_field_scene` | Sharkey's paid travel offers; the Tarislar Tomb offer | Ends dialogue and enters a named scene after the typed destination is validated against the completed source service, using the named ROM arrival policy. |
| `enter_activity` | No current JSON terminal | Ends a graph and requests a named activity. No activity target is currently registered; map/event-table routes use `resume_source_route` or `enter_field_scene` instead. |
| `open_service` | Tarislar/Wylie/Stoker hotel options; Hospital Options | Ends the current graph and hands off to a separately owned service with an explicit return policy. Hotel and hospital targets are registered; shop, taxi, and Matrix targets still need source-backed bindings. |
| `play_cinematic` | Spirit Eyes command `0x4A` | Ends dialogue and requests a named sequence with an explicit return policy. The ROM has a four-page story sequence after the dialogue; its host presenter is still pending. No surveyed building/phone terminal is proven to use this outcome. |
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

### Keep the interaction dimensions separate

The ROM behavior is a combination of independent decisions. JSON should model
each decision once and connect them with stable IDs; it should not create a new
special-case outcome for every combination.

| Dimension | Source-derived values to model | JSON/runtime owner |
| --- | --- | --- |
| Invocation and gate | Location/entity, scene and event, field mode, signed alternate result, run target/type/result, story flags, items, money, and first/repeat history. A known false gate allows the source default route; an unknown fact or ambiguous match is a boundary. | Entity interaction record plus named condition; `LocationInteractionResolver` and `ConditionEvaluator`. |
| Presentation and input | Text-only/MORE page, title-only wait, one-to-three A/B/C buttons, Yes/No buttons, cursor list with confirm/cancel, contact picker with Next/Prior, timed input lockout, and ordered multi-page sequence. | `page`/`choice` nodes for graph-owned prompts; a named menu/service owns a distinct input contract when it is not the shared prompt loop. The Qt view only presents the resulting view model. |
| Choice availability | Ordered choices can be removed by story or inventory conditions, which changes the physical A/B/C slot assignment. Unknown conditions must stop at a boundary rather than silently hiding an option. | Choice IDs remain stable; Lua filters in source order and assigns visible slots after filtering. |
| Effects and transactions | Ordered story/item/party changes and host operations can report action-specific results such as paid/insufficient, success/unavailable, full storage, ineligible, or passage requested. | A named Lua action or typed host request returns a finite result ID; the graph explicitly maps every source-supported result to the next node. |
| Terminal transition | Keep the current place, resume an already selected source route, restore the saved origin, enter a named location/activity, open a separate service, or play a named cinematic. | One validated terminal tagged union, dispatched through registered host adapters. It is separate from the action result that led to it. |
| Return context | A service may return to its caller, a saved exterior entry, the source route, or a named scene; position and heading follow the selected source entry/return record. | Invocation context carries a stable return-point ID and policy. The host applies the return record; the dialogue graph does not reconstruct coordinates. |

Condition facts keep their source types and owners. `story.*` reads named
boolean progression facts, while `runner.attribute.*` and `runner.skill.*`
read numeric values from the active party record supplied by the host. Missing
runner data stays unknown; a graph does not silently treat it as zero. For the
Faradouchi and Agira calls, ROM `$FF018B` is the active runner's `+0x8B`
Reputation skill (`skills[10]`), and both handlers compare it with 5. This
typed fact keeps a stat gate out of the boolean story-flag store and makes the
same condition usable by later dialogue graphs.

This decomposition covers combinations without flattening them. For example,
a building visit can be gated by a run/story condition, show a page and a
Yes/No choice, charge through a transaction action, branch on the result, and
then return to the saved entrance. The gate, prompt, transaction, and terminal
remain independently readable and testable.

The longplay establishes that these are real presentation states: section 03
frame 000301 shows the three-choice manager panel; section 03 frame 000601
shows a multi-page payment/debt prompt with two choices; section 14 frame
006630 shows a portrait page with Yes/No rows; and section 23 frame 001770
shows a narrative page with Services/Goodbye choices. The same panel shell is
reused across them. These captures show the selected player's path only; ROM
conditions and branch code define the alternatives that the video never
selects.

The current catalog review has 18 graphs. All converted game choice nodes
currently use `buttons_abc`. The Lua session tests also exercise `yes_no`
(exactly two choices) and `cursor_list` (navigation, confirmation, and
cancellation) without pretending those test fixtures are game content. The
Vid-Phone contact picker remains its own paged service loop. Independent game
routes using the two-choice and cursor-list graph contracts still need their
own source-traced conversion and presentation checks.

### Interaction lifecycle

1. Resolve the entity interaction using the current location and source event
   facts. Evaluate gates with three-valued results. A known false gate follows
   the source default; a true gate starts the declared graph or destination;
   unknown data and equal-priority conflicts stop visibly.
2. Capture an immutable invocation context: stable interaction ID, source
   scene/event/mode facts, active run identity when known, and the caller's
   return-point ID. Do not copy coordinates or ROM addresses into dialogue
   definitions.
3. Run the graph in Lua. Pages and choices are presentation requests;
   conditions select edges; named effects mutate story state; typed host
   operations return result IDs. Every action runs once when its node is
   entered, not again when the presenter redraws the page.
4. Return exactly one terminal transition or one diagnostic source boundary.
   The host dispatches a terminal by type and, where applicable, by registered
   stable target ID. A service/cinematic/activity owns its own session and
   follows its declared return policy when it completes.
5. On return, apply the captured source return record. This keeps door offset,
   spawn, and heading policy with the door/arrival data instead of duplicating
   those rules in each conversation.

The Vid-Phone lifecycle is a separate instance of this composition: access and
call-history gates filter the 13 ordered contacts; the contact picker pages
seven names plus Next, then the remaining six plus Prior; selecting a contact
chooses its first/repeat call path; a call can show information, offer
services, execute a transaction, return to services, disconnect, or complete
travel. The contact picker is not encoded as a dialogue A/B/C graph, and a
call-history write belongs to the named contact action that the ROM performs.

### Owlfeather graph conversion

The ROM title services at `0x53B78`, `0x53BCC`, and `0x53CEA` call three
entry adapters that pass `dialogue.entry_id` into the shared
`salish.owlfeather.lodge_services` graph. JSON conditions select each route
from the location, field mode/event, and story-flag facts. The graph contains
the first-contact choices and brother task, the shared Lands/Rangers/villages/
Sinsearach/Council topic loop, the search follow-up's Later branch, and the
returned-brother/referral sequence. It keeps the source effect order: runner
counter adjustment, dialogue command, effect-flag updates, and the service-4
trigger tail. Unsupported runner or command behavior stays an explicit source
boundary. Normal endings use `continue_in_place`, which consumes the indexed
service without following the scene-13 event-table fallback.

Page copy remains in the ROM-generated text catalog. The repeated search
prompt is mapped to `salish.owlfeather.page.search_followup` at `0xF7190`; the
two goodbye pages are mapped to semantic Owlfeather keys instead of the old
misassigned Red Buffalo branch aliases. The extractor preserves existing
catalog order while updating values and placing new keys in their namespace,
so refreshing generated strings does not reorder unrelated content.

Lua graph tests exercise the nested topic loop and each service's distinct
branch shape. Existing physical-entry QA covers the three service gates and
several opening choices with full-frame host dumps. These are internal route
and rendering checks; the reviewed longplay sections contain no matching
Owlfeather sequence, so independent Genesis dialogue visuals remain open.

### Spirit Eyes first-contact and ritual graph

The Scene-4 event-27 graph follows the ROM prompt chain instead of assuming
that first contact immediately consumes an item. `0x53D5A` initializes the
ritual markers, selects fixed trigger 5, and opens `0xF4BD8`. Its first
`<MORE>` dispatches command `0x15`, which sets the active trigger bit and
selects prompt `0xF5150`. The route then presents the Spirit Eyes description,
the one-choice welcome question and response, the one-choice ritual question,
and the text-only first horn request at `0xF51A0`. Tests assert that bit 5 is
set after the first input, not while the opening page is still displayed.

Later contacts select one stage from story flags and item facts:

| Source gate | Inventory result | State change and page |
| --- | --- | --- |
| Bit 6 clear | Group Item bit 4 absent | Show repeat horn request `0xF7ACB`; remain in Scene 4. |
| Bit 6 clear | Group Item bit 4 present | Consume bit 4, set bit 6, update FFFC30/FFFC40, show `0xF7641`. |
| Bit 7 clear | Group Item bit 3 absent | Show repeat pelt request `0xF7B3A`; remain in Scene 4. |
| Bit 7 clear | Group Item bit 3 present | Consume bit 3, set bit 7, update effects, show `0xF771D`. |
| Bit 8 clear | Any scale state | Show missing-scale request `0xF7BC8`. |
| Bit 8 set, bit 9 clear | Group Item bit 2 absent | Show missing-scale request `0xF7BC8`. |
| Bit 8 set, bit 9 clear | Group Item bit 2 present | Consume bit 2, set bit 9, update effects, show completion page `0xF77FE`. |
| Bit 9 set | — | Show text-only page `0xF7C79`, then return in place. |

The completion page's command `0x1A` continues to prompt `0xF51A8` and its
`0xF78A3` page. The next controller event dispatches command `0x4A` to
`0x5D172`, which sets up the full-screen resources at `0x1FE6C2` and passes
the four-page pointer table at `0xF4C20` to the blocking story-page routine
`0x5D1F6`. `cinematics.json` records the stable sequence ID, page order, text
keys, controller-event input contract, and `source_context` return policy;
the graph ends with typed `play_cinematic`. The field dispatcher leaves this
target at an explicit source boundary until its named assets and native
presenter are implemented. On the completed-task repeat route, `0x53ED4`
selects `0xF4C18`. The shared `0x1F246`/`0x1F3C4` runner sees its negative
first-option sentinel, shows only the `0xF7C79` text, and returns. The four
adjacent pointers belong to the separate `0x5D1F6` sequence.

### Scene-47 mode-table Renraku notice

The mode-table activity at `0x2CE0` has a separate deferred handler from the
record-62 Licourtrix conversation. When FFFC10=`0x14`, FFFBF9=8, FFFC0E is
nonzero, and FFFC1E bit 8 is clear, the ROM selects Scene 47 and queues list
`0x1E552` with handler `0x53EE2`. That handler passes the single text-only
descriptor at `0xF4C58` and profile `0x12206C` to the shared
`0x1F246`/`0x1F3C4` prompt loop. The page text is `0xF7D0C`; its negative
first-option sentinel means wait 30 frames, then return on any controller
event. It does not implement the later fight or replace the five queued
actors.

The host represents this as `salish.renraku.cave_ambush` in JSON, with the ROM
text and 56x72 portrait imported as named assets. Its precondition checks the
active Scene-47 location, field mode 47, event 8, and transition context
`0x13`. The page uses the shared `field_mode_text` presentation contract and
ends with typed `continue_in_place`. Debug/Release route QA exercises the
60-frame activity handoff, full-screen page, early-input rejection, 30-frame
wait, and in-place return. The route is injected from traced ROM inputs because
the upstream producer of context `0x14` is not yet recovered; the capture is
internal evidence, with no matching independent longplay frame identified.

### Scene-47 Licourtrix encounter

The record-62 Licourtrix conversation uses the same JSON graph, named text
keys, speaker/menu catalog, and condition evaluator as the other converted
dialogue. ROM order is explicit: trigger 8 is selected first; clear story bit 7
shows the flight page; otherwise set bit 8 shows the empty lair; otherwise the
main graph requires FFFC0E nonzero, FFFC10=`0x13`, and FFFBF9=8. The lie answer
loops on one warning-and-choice node, and the scale reward is written only
after the final page closes. Debug/Release tests cover these permutations.
Scene entry now selects Scene 47, its `0x5E80,0x1300` runner spawn, and the Lua
`mode_13_entry` actor list. The upstream context producer and physical
record-62 input remain incomplete. Reviewed longplay sections contain no
matching Licourtrix encounter, so runtime captures remain internal evidence
rather than an independent Genesis visual comparison.

The source-derived permutation table for this graph is:

| Gate/input | ROM result | Graph result |
| --- | --- | --- |
| FFFC10 is `0x14`, FFFBF9 is 8, FFFC0E is nonzero | `0x2CE0` selects context `0x13`, maps it to Scene `0x2F`, and when FFFC1E bit 8 is clear starts activity with deferred routine `0x53EE2` and pending list `0x1E552`. | Active Qt coordinator selects Scene 47, spawn `0x5E80,0x1300`, and five Lua actors. After the 60-frame activity handoff, `0x53EE2` opens the JSON mode-text page from `0xF7D0C` with profile `0x12206C`; the 30-frame any-button wait ends in `continue_in_place`. The context producer remains unresolved. |
| FFFC10 is not `0x13` | Event 25 is rewritten to event 8 before `0x53EFA`. | The graph is not entered; a direct invocation reports a source boundary. |
| Context `0x13`, any story state | Select story trigger 8 first. | Begin the ordered story checks. |
| Story bit 7 clear, either value of bit 8 | Select the flight-notice record. | Show `serpent_flight`; return in place without writing the scale. |
| Bit 7 set, bit 8 set | Select the empty-lair record. | Show `empty_lair`; return in place without changing the item. |
| Bits 7/8 are set/clear; FFFBF9 is not 8 | The profile mode check fails. | Return in place without opening the conversation or granting the scale. |
| Bits 7/8 are set/clear; FFFBF9 is 8; FFFC0E is zero | The alternate-profile check fails. | Return in place without opening the conversation or granting the scale. |
| Bits 7/8 are set/clear; FFFBF9 is 8; FFFC0E is nonzero | Open the `0xF4C44` graph. | Show three pages, then ask neighborhood/scale. Neighborhood loops on the combined warning-and-choice record; scale advances through the two one-button answers and three closing pages. After the farewell closes, set story bit 8 and Group Item bit 2, then return in place. |

Debug/Release runtime cases cover each distinct gate outcome, the overlapping
bit-7/bit-8 case, the repeated lie loop, the scale answer, every page edge,
the ROM choice labels, and the delayed reward writes. The input contract has no
additional source-defined cancel branch; unresolved host input remains a
boundary instead of inventing one.

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
frame 001440 (Max portrait/text page), section 14 frame 006630 (the Club
Penumbra bouncer's 25-nuyen Yes/No entry prompt), and section 23 frame 001770
(A/B services/goodbye choice).
Section 32 frame 001500 is Big Rhino restaurant dialogue. Its steak portrait
matches the reused art assigned to Macklemann's profile, so it supports that
portrait only, not his Vid-Phone choices or outcomes. The other listed frames
support the shared panel/portrait/choice model. The longplay has no matching
complete Vid-Phone call sequence; ROM call descriptors and handler paths
define the contact permutations. Longplay timestamps and holds are visual
sequence anchors, not ROM frame timing.

## Vid-Phone permutation inventory

The ROM list at `0xC66AC`/`0x109682` has 13 ordered contacts. Contact access
and call history are separate facts. The field flow filters unavailable names,
shows up to seven contacts plus Next on page one, then the remaining six plus
Prior on page two. A first call can differ from a repeat call; two contacts
have special first-call/history behavior. Calls may present a one-page or
multi-page first/repeat message, a Services/Goodbye menu, or a contact-specific
service graph.

Eleven contacts have source-derived service implementations: Captain Quinton,
Alesandro, Sharkey, Kipp, Wilma, Macklemann, Gregory, Henry J. Culver,
Chillicut, Max, and Professor Jefferson. Their converted paths are JSON graphs
with named Lua actions and typed host service calls. The traced result
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
- Max's $6,000 corporate badge, HK227-S, AK-97, and level-4/level-5 passkeys;
  each item offer checks wallet and inventory, then follows its own result
  menu or explicit unavailable boundary.
- Professor Jefferson's three-page paranormal lesson, Riannon discount,
  Gregory introduction, first/repeat history, and success/decline/farewell
  paths. The ROM's insufficient-funds pointer at `0x10F12E` resolves to
  unsupported bytes at `0x0FED41`; those results remain explicit boundaries.

Mr. Faradouchi and Agira Tetsumi now have call-scoped JSON graphs sourced from
ROM handlers `0x22C98/0x22CC8` and `0x22CE8/0x22D18`. The graphs preserve their
first/repeat streams, the opposing-faction rejection, the active runner's
Reputation threshold of 5, and the Yes/No pledge. The call-history bit and
matching FF018F faction discount bit are written only after Yes. Decline,
low Reputation, and rival-faction rejection leave history clear, so the next
attempt still selects the first-call stream. The ROM supplies no Services
menu for these contacts, so none was inferred. Their accepted repeat stream
remains the source-provided acknowledgment text.

`contacts.json`, `speakers.json`, `menus.json`, `conditions.json`, and
`conversations/*.json` hold the shared content and all thirteen contact
interaction graphs: eleven service graphs plus the two faction pledge call
graphs. Lua registers the small flag-writing action; JSON owns page order,
choice IDs, gates, and return-to-caller outcomes. The active runner's
Reputation comes from the typed party binding, while missing runner or restored
save facts remain explicit source boundaries.

## JSON content and entity-route layout

```text
decompilation/gameplay/dialogue/
  contacts.json                # 13 ordered records and graph scope bindings
  speakers.json                # speaker IDs, portraits, event text bindings
  menus.json                   # stable choice IDs and ordered label bindings
  conditions.json              # named, composable typed conditions
  cinematics.json              # named sequence metadata and ordered page keys
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

Entity interaction JSON replaces nested route/task columns in `links.tsv` as
each source-traced entity is migrated. The JSON loader registers
`world/entities/<id>/interactions.json` at
`game.content.entities.<id>.interactions`; `world_content.lua` validates the
document and normalizes its structured source, route, task, and location
context into the Lua route policy's typed input. Its version-1 contract uses
`source.sceneId`/`source.eventCode`, a `route` object with a stable condition
ID, priority, optional destination and source-route policy, named `tasks`, a
shared `locationContext`, optional `entryDialogue`, and ROM `provenance`.
`sourceRoutePolicy` is explicit: `suppress_source_route` must also declare the
expected `sourceTableDestination`, which Lua checks against the active map's
imported event-table row before selecting the interaction. Conditions remain
named expressions in `location_conditions.json`, rather than inline Lua or
string mini-languages. Unconverted route rows remain in `links.tsv` until
their source gates and destinations are traced. Map geometry, collision
records, and event-table destinations remain in the existing map/source-data
pipeline; runtime JSON never duplicates map geometry or ROM address arithmetic.

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
  Outcomes form a tagged union (`continue_in_place`, `return_to_caller`, `resume_source_route`,
  `return_to_origin`, `enter_field_scene`, `enter_activity`, `open_service`,
  or `play_cinematic`) with payload fields specific to the tag. Targeted
  outcomes resolve stable IDs in the corresponding catalog/registry, and
  return policies are checked against the declared target. This lets another
  host register the same outcome handler without route-specific string cases.

Graph node IDs are content IDs, never source addresses. A source command may
appear as provenance on the edge/action that represents it. Effects are named
operations (`apply_dialogue_command`, `charge_building_amount`,
`award_party_karma`, `adjust_runner_triplet`, `set_story_fact`) with typed
parameters; JSON cannot call arbitrary Lua functions. Complex quest policy
stays in named Lua actions and is unit-testable without the presenter.

The `terminal` outcome is a discriminated union, not a free-form result string:

```json
{ "type": "continue_in_place" }
{ "type": "return_to_caller" }
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
`return_to_caller` is consumed only by an owning interaction session; if it
escapes to the field host without a caller, it remains an unregistered outcome
and is reported as a source boundary.
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

Keep the serialized outcome a JSON tagged union, with required and forbidden
payload fields validated for each `type`. Source boundaries are diagnostic
results, never successful graph terminals. Implemented: the C++ bridge converts
the Lua table into a C++17 `std::variant` of outcome payload structs plus a
`SourceBoundaryOutcome` alternative. This makes mismatched payload combinations
unrepresentable to host adapters while preserving the same JSON contract for
Lua and other hosts. The dispatcher registers typed handlers by outcome type
or stable target ID; a missing named target remains unhandled and cannot fall
through to a generic handler. Views do not gain outcome-specific branches.

Action results follow the same rule at their smaller scope. The graph currently
validates each action node's explicit result edges. As host transactions migrate,
each action adapter should declare its allowed result IDs and validate its own
payload; do not create one global list that implies, for example, every
purchase can be `passage_requested` or every payment can be `inventory_full`.

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
   JSON-backed. Eleven contacts have JSON service graphs; Faradouchi and Agira
   have JSON first/repeat and faction-pledge call graphs, including the ROM's
   active-runner Reputation and rival-faction gates.
4. **Implemented with explicit host boundaries:** the reusable Lua graph and
   condition engines and typed `DialogueOutcome` contract are active. The
   portable C++ `DialogueOutcomeDispatcher` registers handlers by outcome type
   or stable target ID. The field host registers in-place continuation, source
   route resumption, ROM-default field-scene travel, hotel and hospital service
   targets, and `return_to_origin` for the saved `source_entry` door record.
   Unknown targets remain visible source boundaries. Activity and cinematic
   transitions still need their source-backed host coordinators. Remove legacy
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
visual checks. The eleven source-derived contact service trees and the two
faction pledge call graphs are JSON-backed. Their state, prompt, and terminal
permutations have automated Lua and native-host coverage; matching independent
longplay frames for the Faradouchi and Agira call sequences remain unavailable,
so their visual status stays partial. The Spirit Eyes
ritual route is JSON-backed through its source-traced Scene-4 prompt chain and staged
item gates. Licourtrix's Scene-47 graph is JSON-backed and tested, and the
seeded mode-table route now reaches its typed Scene-47 entry and Lua actor list.
The context producer, deferred activity routine, ritual cinematic presenter,
physical record-62 route, and unverified visual branches remain explicit work,
not filled-in defaults.

## Implemented condition slice

`gameplay/dialogue/location_conditions.json` now contains versioned, stable-ID
condition definitions for every nontrivial gate in the 86 remaining TSV entity
route rows. Those rows carry `route_condition_id` and `entry_condition_id`
references; entity JSON uses `route.conditionId` and
`entryDialogue.conditionId`. Semicolon conditions, the separate entry
alternate-result field, and building-interaction mode gates have been removed
from route records. Existing source addresses and branch evidence stay with
each route record.

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

The C++ script boundary parses a tagged `DialogueOutcome` value:
`continue_in_place`, `resume_source_route`, `return_to_origin`,
`enter_field_scene`, `enter_activity`, `open_service`, `play_cinematic`, and a
diagnostic `source_boundary`. Route data can declare the expected typed result
for a pre-entry page. The field presenter sends typed outcomes through the
portable `DialogueOutcomeDispatcher`. Type handlers own `continue_in_place`,
`resume_source_route`, and the Sharkey travel form of `enter_field_scene`.
Named handlers own `hotel.options` and `hospital.options`; adding a service
screen registers its stable ID and host coordinator instead of editing a
service-name branch in the presenter. `return_to_origin` can consume the
source-derived saved door point as `returnPoint: "source_entry"`; Hotel and
Hospital checkout now use that same return handler. No migrated dialogue graph
currently emits the outcome. Unsupported activity/cinematic IDs and
unregistered service IDs remain visible source boundaries.

The following non-Stoker graphs and two Stoker roots currently exercise that
contract:

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
- Boris Greenhouse service 10 is a JSON graph for its signed alternate-result
  pages, default/repeat greeting, $100 story offer, $500 information offer,
  ordered Michael/David/Ilene/Thon topic gates, and same-map return. Conditions
  remain named JSON expressions; Lua only registers the source-derived state
  mutations. The Strike Team command `0x2C` remains an explicit source
  boundary, and the unconverted item commands `0x2D`-`0x2F` are not presented
  as implemented paths. Lua graph tests cover payment outcomes, topic priority,
  and missing-fact boundaries. Debug/Release physical route QA covers the
  indexed entry, $100 transaction, typed same-map return, and save restore;
  six full-frame captures per build match. No independent Genesis dialogue
  frame has been identified; keep the route partial.
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
- Henry J. Culver's record at `0x10DE0C` now uses one graph for first/repeat
  call choices, Services, three priced offers, insufficient funds, each state
  write, the return menu after declining, and the source Goodbye page. His
  commands charge 2,500/1,500/8,000; success clears the Lone Star record byte,
  sets FFFC2C bit 27, or sets FFFC22 bit 11 respectively. The state effect is
  applied only after the typed wallet transaction succeeds. Lua permutations,
  script QA, and full-panel image QA cover the declared paths. The reviewed
  longplay contains no complete Henry call, so those internal captures do not
  establish independent Genesis visual fidelity.
- Sgt. Chillicut's record at `0x10E9DE` now keeps the first-call MORE page and
  private-line detail separate from the repeat-call menu. Its three offers
  preserve the ROM's Yes/No/Goodbye buttons and return-to-services behavior;
  successful offers show the service result menu, while failed payments end
  on the source insufficiency page. Commands charge 5,000/3,000/10,000 and
  write the same record, Hollywood-access, and permit facts as Henry. Lua
  permutations cover first/repeat calls, choices, prices, and transaction
  outcomes. No complete Chillicut call appears in the reviewed longplay; its
  route remains visually unverified against an independent Genesis capture.
- Max's record at `0x10E400` branches on contact history, then offers the
  $6,000 corporate badge or the hardware flow. Commands `0x83`-`0x87` charge
  $6,000, $1,500, $2,000, $30,000, and $42,000 for the badge, HK227-S, AK-97,
  and level-4/level-5 passkeys. The C++ service checks wallet and free
  inventory before applying the item or FFFC22 bit-10 badge flag. JSON owns
  offer categories, return menus, success, insufficient-funds, inventory-full,
  and goodbye edges. Lua permutations exercise every command/result pair and
  the host integration test resolves the displayed labels. Section 05 frame
  001440 is a Max portrait/text page, not a phone call; no complete Max call
  sequence appears in the reviewed longplay.
- Professor Jefferson's record at `0x10F0D4` has the three-page paranormal
  lesson, the club offer (command `0x88`, $1,500, FF018F bit 6), and former
  student offer (command `0x89`, $2,000, FFFC2C bit 6, unlocking Gregory).
  JSON keeps each branch's Services/Goodbye choices and transaction outcomes;
  the Lua adapter submits only the two named host requests. The insufficient
  message pointer at `0x10F12E` points to invalid text bytes at `0x0FED41`, so
  both low-funds results stop at a documented source boundary. Section 23
  frame 001770 shows the shared Alesandro/Professor portrait with a generic
  Services/Goodbye panel, not a complete Professor call. Pure-Lua permutation
  and C++ host-integration tests cover the graph; independent call visuals
  remain unavailable.

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
effects. Debug/Release script-runtime QA also traverses the complete known
Spirit Eyes first-contact chain, the three item gates, the typed ritual
activity outcome, and the single completed-task return page. Focused Debug
CTests exercise both Stoker roots through the field dialogue presenter. Full
physical route QA and independent Genesis comparison remain separate
evidence; the reviewed
longplay shows shared panel/menu examples but no matching Gates departure or
Renraku/Fuchi first-entry sequence or Wylie's Gala Inn interaction.
