# Common dialogue framework (#37)

## Scope and ROM mapping

The US ROM has several dialogue dispatch paths. The common layer below records
which source opened a Lua dialogue task and its typed close contract; it does
not replace those source dispatchers with one ROM-shaped runtime.

| Port entry family | ROM source path | Evidence / certainty |
|---|---|---|
| Field movement events, including event 31 and Vid-Phone | `0xB7B8`; event 31 continues through `0xB86E -> 0x0DB26`, Vid-Phone via `0xE4C8` | [C] US ROM. |
| Building entry, service, and exit selection | `0x1651E`; mode/service dispatch includes `0x53A78`, `0x1F046/0x1F106`, and exit prompt selector `0x1F65A` | [C] US ROM. Route result remains selected by the current typed field route. |
| Actor-pair contact, generic bit 4 | `0xEB92 -> 0xECCC`; bit 4 callback `0x21174`; text/choice lookup `0xFE640` | [C] US ROM. |
| Actor-pair contact, bit 3 story handlers | `0x1F146 -> 0x53C90/0x54402` | [C] US ROM. The two current contact examples include Aragorn and Ilene. |
| Shared dialogue/activity input lock | `0x1E2E0/0x1E2EC` set or gate `FFE11C=60`; service pass decrements at `0x1DD94` | [C] US ROM. Runtime measurements for the current Scene 10/11/19/20 samples are in `/workspace/shadowrun/out/progress_notes.md`; this change does not claim ROM-correct dialogue timing. |
| Active run-target entry | `0x1F086` active-run area/event check before its selected event-table destination | [C] US ROM. This is separate from ordinary NPC contact. |
| Recruited-runner conversation | `0x1F58A` indexed command dispatch to `0x211E8`; its typed C++ conversion handles runner-specific prompt/choice state | [C] US ROM. The conversion currently has no production caller in the Qt runtime; the Lua `RECRUITED_RUNNER` source default is ready for its future task entry. |
| Stoker arrival and ambush | `0x1F65A` prompt selection; `0x249C4` samples the arrival result and can call the ordered actor effect at `0x1D6E2A` | [C] US ROM. |
| Shared door return and outward facing | `0xB834 -> 0xBC36/0xBD34`; facing-away write at `0x09B8` | [C] US ROM. Port route data preserves the measured outward-facing exits. |
| Party recovery and tomb/cinematic entries | Recovery is represented by the current `0xC64C` recovery path; the Scene 55/tomb host entry still lacks a fully traced dialogue-dispatch link | Recovery [C]; exact tomb dispatch linkage [H]. No address is inferred for the tomb task. |

## Descriptor and ownership

`gameplay/scripts/dialogue_framework.lua` is the sole Lua task-opening
boundary. `E.DialogueSource` in `gameplay/scripts/enums.lua` identifies the
source; the native `ScriptDialogueSource` mirror requires the Qt caller to
choose one explicitly. `GameScriptRuntime::start_dialogue_task` enters the
framework and forwards any selected `FieldScriptRoute` data. The framework
merges, in order, source defaults, named route overrides, and the supplied
route data, then validates every outcome against the existing typed enums.

The descriptor fields are:

| Field | Lua type | Meaning |
|---|---|---|
| `source` | `E.DialogueSource` | Building entry/service/exit, field contact/event/activity, random or scripted encounter, Vid-Phone, run target, story/reward/recovery, field menu, taxi, or cinematic. |
| `route_id` | stable Lua module/function ID | Identifies the selected task without embedding a ROM address in runtime state. |
| `scene_outcome` | existing `E.FieldLocationSceneOutcome` | `SAME_SCENE`, `SOURCE_DOOR_RETURN`, `SCENE_CHANGE`, or `NONE`. |
| `exit_behavior` | `E.DialogueExitBehavior` | `STAY`, `SOURCE_DOOR_RETURN`, `SCENE_CHANGE`, or `NONE`. |
| `transition` | `E.DialogueTransition` | `FADE` or `NONE`; later fade timing can be added here as route data. |
| `return_anchor`, `return_facing`, `music_return_policy` | existing `E.FieldLocationReturnAnchor`, `E.FieldLocationReturnFacing`, and `E.FieldLocationMusicReturnPolicy` | Return placement, outward-facing or preserved heading, and scene-profile music resume. |
| `input_lock` | `E.DialogueInputLock` | Names the shared `FFE11C` lock when the selected route uses it. |

`game.dialogue.active` remains set for the entire Lua task, including every
yielded page or menu. The host copies it into `ScriptTaskStep` and
`FieldDialogueRuntime::opening_descriptor()`. Lua task bodies own the page,
choice, state, and outcome flow; `DialogueGraph.run` (`dialogue_graph.lua:647`)
and direct `game.ui.*` calls are page/choice operations inside an already
opened task. Qt continues to present yielded operations and execute the
existing typed field/modal transition. The opening descriptor is the shared
policy record; the host remains responsible for applying scene placement,
input, fade, and audio services.

Source defaults and route exceptions live in `dialogue_framework.lua`, not in
per-call close branches:

| Source/route | Descriptor policy |
|---|---|
| Building entry | Scene change, fade. A route may refine the outcome. |
| Building service | Same scene, stay, no transition by default. |
| Building exit | Source-door return, fade, collision-probe lead, face away from entry, resume scene-profile music. Route data may refine this. |
| Field NPC contact | Same scene, stay, no transition; the shared activity input lock is declared by the generic-contact and Ilene route overrides. |
| Field event/activity and story/reward | No transition unless route data selects a scene result; ordinary field activity defaults to staying in the source scene. |
| Random/scripted encounter | No scene change or transition unless route data says otherwise. |
| Vid-Phone and taxi | Their currently selected source transition is recorded as fade; task outcome still selects the existing destination/return. |
| Party recovery and cinematic | No built-in field scene change or fade; recovery/cinematic outcome owns the continuation. |
| `field_generic_contacts.present`, `ilene.complete_scene_20_activity` | Same-scene stay with `E.DialogueInputLock.WORLD_ACTIVITY_FFE11C`. |
| `michael_package.stoker_interaction` | Same-scene stay. |

Field-route `scene_outcome`, return anchor, facing, and music policy are passed
to Lua as descriptor route data from `field_scene_runtime_internal.hpp`.
This preserves the existing `world_links.lua` / `building.lua` route census,
including `SAME_SCENE` and explicit `SOURCE_DOOR_RETURN` exceptions.

## Entry inventory

The rows below inventory 29 current C++ task-opening call sites. This includes
production entries and QA-only route helpers; each enters
`FieldDialogueRuntime::open_gameplay_task` or its typed Stoker wrapper, which
reaches `GameScriptRuntime::start_dialogue_task` and the Lua framework. The
production paths account for 24 sites; five rows marked QA are test fixtures.
For variable task names, the table names the runtime-selected route family.

| C++ entry point (file:line) | Lua task / purpose | Source descriptor | ROM mapping |
|---|---|---|---|
| `qt_runtime/src/field/field_scene_runtime.cpp:1641` | Selected active run-target task | `RUN_TARGET` | Active run target check `0x1F086` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:1737` | Indexed title service task | `BUILDING_SERVICE` + route data | `0x53A78`, title service selectors `0x1F046/0x1F106` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:1780` | Area-event pre-entry dialogue | `BUILDING_ENTRY` + route data | `0x1651E` area/event destination dispatch [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:1827` | `redmond_barrens.run_vigore_search` | `FIELD_EVENT` | Scene 2/service-22 branch through `0x53A78` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:1872` | Data-selected same-map actor dialogue, including `field_events.present_location_dialogue` | `FIELD_NPC_CONTACT` + route data | Event-30 continuation `0xB7B8 -> 0x20990`; actor pair path `0xEB92 -> 0xECCC` where applicable [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:1902` | Data-selected field-exit prompt | `BUILDING_EXIT` + route data | `0x1F65A`; shared return uses `0xB834 -> 0xBC36/0xBD34`, facing `0x09B8` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:5529` | `story_activity_routes.complete_<name>` | `STORY_ACTIVITY` | Bit-3 story route family `0x1F146 -> 0x53C90/0x54402` [C]; task-specific address remains route-dependent [H]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:5705` | `field_generic_contacts.present` | `FIELD_NPC_CONTACT` | Bit-4 path `0xEB92 -> 0xECCC -> 0x21174 -> 0xFE640`; lock `FFE11C` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:5848` | Aragorn, Ilene, or random employee contact task | `FIELD_NPC_CONTACT` or `RANDOM_ENCOUNTER` | Bit-3 handler `0x1F146 -> 0x53C90/0x54402`; random pedestrian has its separate `0x1F146` callback [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8680` | `salish_spirit_eyes.present_renraku_ambush` | `SCRIPTED_ENCOUNTER` | Renraku mode-table prompt service `0x2CE0 -> 0x53EE2 -> 0x1F246` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8731` | Extraction success/failure result | `STORY_ACTIVITY` | Activity result dispatcher is present in the port; exact ROM task entry for each current result is not identified here [H]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8784` | Fuchi cyberheart safe reward | `STORY_REWARD` | Safe handler `0x540D6` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8823` | Scene 55 tomb antechamber arrival | `CINEMATIC` | Scene 55 arrival dispatch linkage to this presentation task remains [H]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8854` | Scene 55 ending warning | `CINEMATIC` | Scene 55 ending dispatch linkage remains [H]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8910` | Ordinary field terminal | `FIELD_MENU` | Event 31 `0xB7B8 -> 0xB86E -> 0x0DB26` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:8929` | Resource-scene main menu | `FIELD_MENU` | Event 31 resource branch at `0x0DB26` [C]. |
| `qt_runtime/src/field/field_scene_runtime.cpp:10191` | Taxi cabbie dialogue | `TAXI` | Field travel/ride entry; exact source dialogue dispatch address is not cross-linked in the current port notes [H]. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:286` | QA ordinary field-terminal route | `FIELD_MENU` | Same event-31 terminal branch `0x0DB26` [C]. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:310` | QA Spirit Eyes ritual route | `SCRIPTED_ENCOUNTER` | Ritual graph is tied to the Spirit Eyes site; exact interaction entry remains [H]. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:321` | QA tomb-credits route | `CINEMATIC` | Scene 55 tomb ending presentation; exact source entry remains [H]. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:860` | QA Vid-Phone route | `VID_PHONE` | `0xB7B8 -> 0xB86E -> 0x0DB26 -> 0xE4C8` [C]. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:955` | Shared field-location route helper | Caller-supplied typed source + `FieldScriptRoute` data | Source dispatch is selected by event-30 or event-25/27 caller; route data retains `building.lua`/`world_links.lua` outcome [C]. |
| `qt_runtime/src/field/field_scene_runtime_transitions.cpp:1385` | Council Island Hospital menu/service task | `BUILDING_SERVICE` | Hospital/service dispatch belongs to the mode/service path `0x53A78` [C]; each medical operation has its own source routine. |
| `qt_runtime/src/field/field_scene_runtime_player_actions.cpp:405` | Party-wipe failure page | `PARTY_RECOVERY` | `0xC64C` party-wipe recovery [C]. |
| `qt_runtime/src/field/field_scene_runtime_player_actions.cpp:594` | Party wake page | `PARTY_RECOVERY` | `0xC64C` recovery continuation and destination selection [C]. |
| `qt_runtime/src/field/field_scene_runtime_world_effects.cpp:511` | Hidden-camera prompt | `RANDOM_ENCOUNTER` | Random encounter event dispatch [H]; current handler identity is typed but its exact ROM call edge is not linked in the port notes. |
| `qt_runtime/src/dialogue/field_dialogue_runtime.cpp:150` | Stoker field-exit prompt | `BUILDING_EXIT` | `0x1F65A`; Stoker arrival branch `0x249C4` [C]. |
| `qt_runtime/src/dialogue/field_dialogue_runtime.cpp:179` | Stoker interaction task | `BUILDING_SERVICE` | `0x53A78` / `0x1F246` indexed service [C]. |
| `qt_runtime/src/dialogue/field_dialogue_runtime.cpp:276` | QA Stoker manager interaction fixture | `BUILDING_SERVICE` | Same indexed Stoker service path [C]. |

The Qt QA-only `open_stoker_prompt_for_dump` adapter
(`field_dialogue_runtime.cpp:157-169`) also exposes legacy
`StokerMotelPromptId` panel fixtures. Their non-manager text/menu assembly is
in `field_dialogue_runtime.cpp:251-348`, with test-only follow-up hooks in
`field_dialogue_runtime.cpp:1325-1466`. They are exercised by the direct panel
and visual QA helpers, not by the active game route: production Stoker entry
and exit tasks use `michael_package.lua:93-100` through the common framework.
The manager interaction entry at `field_dialogue_runtime.cpp:276` is reached
only through that QA fixture; the production Stoker interaction entry is at
`field_dialogue_runtime.cpp:179`. The QA fixture is retained to preserve
existing panel assertions and is not a second production dialogue task entry.

The separate recruited-runner prompt/choice conversion in
`src/integration/recruited_runner_conversation_conversion.cpp` is exercised
by the C++ conversion and panel tests, but it has no call site in
`qt_runtime/src`. Its ROM entry is `0x1F58A -> 0x211E8`; when the active Qt
runtime wires that task, it must use `E.DialogueSource.RECRUITED_RUNNER` and
the common task opener. It is not counted among the current production Qt
dialogue openings.

`field_events.new_game_opening_walk` is the only remaining raw C++ task start
(`field_scene_runtime_transitions.cpp:826`). It requests the player walk after
scene-2 entry and does not present dialogue (`field_events.lua:116`); the only
scene-entry map event in the active data is recorded in
`gameplay/world/areas/scene-02-triggers.tsv`. The `field_events.present_location_dialogue`
handler is opened through the typed event-30 route above.

Lua task bodies and page renderers are distributed by content. Representative
entry functions (file:line) are `field_events.lua:123`,
`field_generic_contacts.lua:7`, `ilene.lua:39`, `salish_owlfeather.lua:27-32`,
`redmond_barrens.lua:20`, `salish_spirit_eyes.lua:9-18`,
`michael_package.lua:93-100`, `field_activities.lua:175,338,362`,
`council_island_hospital.lua:150,172,200,252`, `party_down.lua:271,303`,
`field_world_activity_results.lua:29,44`, `fuchi_offices.lua:10,24`,
`tarislar_apartments.lua:19,55,74,91,109,113`,
`tarislar_tomb_credits.lua:8,12`, and `taxi_cabbie.lua:105`. Their
`DialogueGraph.run` or `game.ui.*` calls yield pages/choices inside the task;
they do not start another host task.

## Exit inventory and behavior preservation

| Port exit path (file:line) | Role | ROM/source mapping |
|---|---|---|
| `qt_runtime/src/dialogue/field_dialogue_runtime.cpp:385` | Resolve Lua `DialogueOutcome` and close the active task view | Typed outcome boundary; route-specific source dispatch above. |
| `qt_runtime/src/dialogue/field_dialogue_runtime.cpp:860-1469` | Advance pages/choices, resume the active coroutine, and close or return a typed outcome | Page/choice loop and source task result; task remains on `game.dialogue.active` while yielded. |
| `qt_runtime/src/field/field_scene_runtime.cpp:13594-13780` | Consume completed task outcome/result and continue its current field route | Field controller owns applying the outcome; no replacement state machine added. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:1112-1267` | Dispatch typed dialogue outcome and return/transition policy | `continue_in_place`, service return, source route resume, or scene handoff. |
| `qt_runtime/src/field/field_scene_runtime_transitions.cpp:558,767` | Advance scene transition after modal and start modal return transition | Existing modal timing and scene transition path. |
| `gameplay/scripts/building.lua:316-349` | Select route scene outcome and attach return policy | `E.FieldLocationSceneOutcome`; same-scene service and scene-entry branches [C]. |
| `gameplay/scripts/world_links.lua:117-178,228-255` | Choose same-scene stay versus source-door return, anchor, facing, and music policy | `0xB834 -> 0xBC36/0xBD34`; away-facing write `0x09B8`; music resume through existing scene profile path [C]. |
| `gameplay/scripts/dialogue_framework.lua:176-230` | Hold the active typed source/route descriptor through the full task and restore nested state on completion/error | New common opening lifecycle; no gameplay close expectation changed. |

Intentional outward-facing exits (`world_links.exit_bearing`) and the
`SAME_SCENE` return rule remain intact. The current 32-route, 30-exit, and
30-contact census expectations were not altered. Measured first-frame timing
differences found during Step 1 are documented in the progress notes; this
framework task does not claim ROM-correct timing.

## Follow-on hooks

| Follow-on item | Extension point |
|---|---|
| #28b-4 | Add route data to the existing `E.FieldLocationSceneOutcome`/anchor/facing/music descriptor for source-door return, `SAME_SCENE` stay, or a route-specific exception. The framework already merges route data without another enum family. |
| #30 B cancel | Extend the Lua descriptor's input policy/result fields and route data; keep the cancel outcome in the source task rather than a second Qt menu state machine. |
| #31 shop input | Add shop input contract and return route to the descriptor data; preserve source-sized choice lists and let Lua own choices. |
| #32 fades | Extend `E.DialogueTransition` or route data with the source-derived fade mode/timing; Qt executes the selected presentation transition. |

## Focused regression

`gameplay/tests_lua/dialogue_framework_tests.lua` checks that every
`E.DialogueSource` has one policy, route values override source defaults, a
descriptor stays active across a yielded page and clears after completion,
and C++ Qt task starts cannot bypass `FieldDialogueRuntime` except for the
known non-dialogue New Game walk. `run_lua_suites.sh` runs this test, so the
existing `shadowrun_gameplay_lua_suites` CTest registration includes it.
