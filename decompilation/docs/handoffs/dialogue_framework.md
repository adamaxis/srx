# Common dialogue framework (#37)

## Scope and ROM mapping

The US ROM has several dialogue dispatch paths. The common layer below records
which source opened a Lua dialogue task and lets its descriptor drive terminal
exit effects. It does not replace those source dispatchers with one
ROM-shaped runtime.

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
| `exit_behavior` | `E.DialogueExitBehavior` | `STAY`, `SOURCE_DOOR_RETURN`, `SCENE_CHANGE`, `RETURN_TO_ACTIVITY_SOURCE`, or `NONE`. |
| `transition`, `transition_frames` | `E.DialogueTransition` and `E.DialogueTransitionFrames` | Selects `FADE` or `NONE` and the descriptor-owned fade duration in host updates. The measured Haggard interval uses 83 frames; ordinary modal returns keep the named 60-frame default. |
| `return_anchor`, `return_facing`, `music_return_policy` | existing `E.FieldLocationReturnAnchor`, `E.FieldLocationReturnFacing`, and `E.FieldLocationMusicReturnPolicy` | Return placement, outward-facing or preserved heading, and scene-profile music resume. |
| `input_lock`, `input_lock_frames` | `E.DialogueInputLock` and `E.DialogueInputLockFrames` | Names the shared `FFE11C` lock and its exact frame count. |
| `exit_result_id`, `exit_outcome_type` | stable Lua result string / outcome type | Limits exit effects to a selected terminal branch. |
| `encounter_source` | `E.DialogueSource` | Identifies an encounter nested inside another task, such as Stoker's random arrival roll inside a building-exit task. |

`game.dialogue.active` remains set for the entire Lua task, including every
yielded page or menu. The host copies it into `ScriptTaskStep` and
`FieldDialogueRuntime::opening_descriptor()`. Lua task bodies own the page,
choice, state, and outcome flow; `DialogueGraph.run` and direct `game.ui.*`
calls are page/choice operations inside an already opened task.

`DialogueFramework.execute_exit` runs when the task reaches its terminal
result. It reads the descriptor and requests each selected effect through
the narrow `dialogue_exit.*` host primitive service: stay,
source-door return with anchor/facing, scene change, return to an activity's
saved source, fade, scene-profile music resume, and an input lock with an
exact frame count. A source boundary has no exit effects. Optional
`exit_outcome_type` and `exit_result_id` fields defer those effects when a
task opens a nested service or selects another route. Qt implements each
requested state/placement/transition/audio primitive; it no longer selects
exit policy or runs a second dialogue-close path.

Source defaults and route exceptions live in `dialogue_framework.lua`, not in
per-call close branches:

| Source/route | Descriptor policy |
|---|---|
| Building entry | Scene change, fade. A route may refine the outcome. |
| Building service | Same-scene stay with the descriptor's current 60-frame modal-return fade. |
| Building exit | Source-door return, fade, collision-probe lead, face away from entry, resume scene-profile music, and `FFE11C=60`. Route data may refine this. |
| Field NPC contact | Same-scene stay with the modal-return fade; the shared activity input lock is declared by the generic-contact and Ilene route overrides. |
| Field event/activity and story/reward | Field events keep their current context; activity and story/reward routes default to same-scene stay. Their selected descriptors apply the modal-return fade. |
| Random/scripted encounter | No scene change or transition unless route data says otherwise. |
| Vid-Phone and taxi | Their currently selected source transition is recorded as fade; task outcome still selects the existing destination/return. |
| Party recovery | No built-in scene change or fade; the task result selects its continuation. |
| Cinematic | A `play_cinematic` result with `returnPolicy=source_context` runs its opening descriptor after Lua presents the final page, before the host consumes the typed return. |
| `field_generic_contacts.present`, `ilene.complete_scene_20_activity` | Same-scene stay with `E.DialogueInputLock.WORLD_ACTIVITY_FFE11C`. |
| `michael_package.stoker_interaction` | Same-context use keeps `STAY`; when typed route data selects the Stoker source-door exception, the descriptor selects the shared outside placement, modal-return fade, scene-profile music, and `FFE11C=60` (`0x173138 -> 0x1F65A -> 0xBC36`). Other #28c same-scene routes remain `STAY`. |
| `michael_package.present_package_handoff` | Same-context terminal page with a modal-return fade; the enclosing Stoker interaction descriptor applies its selected field return. |
| Haggard/escaped-patient encounter (`field_random_events.present_haggard_man`) | Random encounter; same-scene stay, 83-frame black/fade return, no profile music request, and `FFE11C=60` [C]. The Help branch is now a production Lua task. |
| `salish_spirit_eyes.present` | The `source_context` cinematic outcome executes this route's same-scene fade after the graph presents its story pages. |
| `salish_spirit_eyes.present_renraku_ambush` | Scripted encounter; same-scene stay, no exit fade or profile music request, and `FFE11C=60` [C]. |
| `michael_package.present_field_exit_prompt` | Building-exit task with a nested `RANDOM_ENCOUNTER` result from `0x249C4`; source-door return, fade, scene-profile music, and `FFE11C=60` run only for `continue_in_place` [C addresses; parity caveat below]. |
| `field_activities.resource_scene_main_menu` | Restore the saved field scene and apply the descriptor-selected fade/music only for `return_from_world_activity`. |

Field-route `scene_outcome`, return anchor, facing, and music policy are passed
to Lua as descriptor route data from `field_scene_runtime_internal.hpp`.
This preserves the existing `world_links.lua` / `building.lua` route census,
including `SAME_SCENE` and explicit `SOURCE_DOOR_RETURN` exceptions.

## #37b encounter measurements and removed paths

The framework drives exits from the descriptor. `SOURCE_DOOR_RETURN` carries
`COLLISION_PROBE_LEAD` plus `FACE_AWAY_FROM_ENTRY` (ROM
`0xB834 -> 0xBC36/0xBD34`, heading `0x09B8`); `STAY` invalidates any stale
door-return point; `SCENE_CHANGE` resumes the selected field route; and
`RETURN_TO_ACTIVITY_SOURCE` restores the saved field scene. Transition,
music, and lock effects then execute in descriptor order. The native
primitive validates anchor/facing and lock range before updating typed host
state. A completed `return_to_caller` is consumed by the presenter only after
the Lua executor has issued the descriptor's primitive operations. A
source-context cinematic is a deferred dialogue exit: its descriptor runs
after the graph has yielded all of its pages.

| Encounter/route | ROM behavior and evidence | Headless measurement |
|---|---|---|
| Doctor restraining the patient (`field_random_events.present_haggard_man`) | `0x20FA2 -> 0x23BB4 -> 0x23CF2 -> 0x121188`; same Scene 2 position, no door placement or profile resume, `FFE11C=60`. Forced GPGX `/workspace/shadowrun/emu/doors/r95.state` measured the Help result, `+50` nuyen, 60 lock decrements, and 83 consecutive black frames [C]. Longplay section-11 frames 1224–1261 are documented as 38 consecutive black frames; the referenced frame dump is absent in this checkout, so the input/result identity remains [H]. | `shadowrun_ghidra_conversion_qt_field_hidden_camera_qa` drives the production Lua Help branch and measures same-scene stay at `(15584,11520)`, 83 fade updates, 60 lock countdown decrements, and `music_delta=0`. The forced ROM and port positions are fixture-specific; the matching behavior is the unchanged position. |
| Renraku strike team (`salish_spirit_eyes.present_renraku_ambush`) | `0x2CE0 -> 0x53EE2 -> 0x1F246`; the deferred page continues in Scene 47 without source-door placement or a fade; `0x1E2EC` sets the shared `FFE11C=60` lock [C]. A route-matched GPGX exit capture is not available in this checkout [H]. | `shadowrun_ghidra_conversion_qt_field_mode_13_entry_qa` measures a 60-frame activity wait, 30-frame text-input wait, same position, no exit fade, no exit profile-music request, and 60 lock countdown decrements. |
| Stoker paid package interaction (`michael_package.stoker_interaction`) | Same-context dialogue result; the Stoker source-route caller selects the outside exit at `0x173138 -> 0x1F65A -> 0xBC36/0xBD34`, heading `0x09B8` [C]. The route-matched GPGX terminal-effects capture remains unavailable [H]. | `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa` measures the descriptor-selected field return: lead `(15584,11210)`, heading `0x8000`, 60 fade updates, 60 lock countdown decrements, and one profile request for `0x0F`. These are port measurements, not a ROM parity claim. |
| Stoker ambush (`0x249C4`, nested in `michael_package.present_field_exit_prompt`) | `0x249C4` samples 80–99 for the ordered ambush list at `0x1D6E2A`; the enclosing field exit is selected at `0x1F65A`. Its terminal path shares the `0xB834 -> 0xBC36/0xBD34` placement and `0x09B8` outward heading; the Scene-2 music request path includes `0xFFE172`/`0x53126` and `0x2900..0x2916`, with track word `0x0F` [C]. The GPGX forced-state capture `/workspace/shadowrun/emu/doors/jh3.state` measures the three actor spawns, not the terminal exit transition [H]. | The port's shared exit primitives are measured by the Stoker collision QA above, but a headless QA case that runs the rare `0x249C4` exit descriptor is still needed [H]. |

The forced Haggard GPGX and port values match for stay placement, the 83-frame
black interval, and the 60-count lock; its exact music identity remains [H].
Renraku and Stoker have measured port exits and ROM-traced policies, but their
route-matched GPGX exit captures are not available here. The ordinary Stoker
fade is an approximate 60-update host transition, not a ROM timing claim.

| Removed dialogue side-effect path | File:line | Executor-owned operation |
|---|---|---|
| Typed in-place close queried `same_scene_return_policy`, selected placement policy, and started scene/modal refresh. | `qt_runtime/src/field/field_scene_runtime_services.cpp:1160-1215` (before refactor) | Lua `STAY`, transition, music, and lock descriptor fields. |
| Typed source-route close resumed the selected route and started its modal return. | `qt_runtime/src/field/field_scene_runtime_services.cpp:1254-1269` (before refactor) | Lua `SCENE_CHANGE` and transition fields. |
| Typed return-to-origin close began the physical door-return state. | `qt_runtime/src/field/field_scene_runtime_services.cpp:1317-1327` (before refactor) | Lua `SOURCE_DOOR_RETURN`; the outcome handler now only consumes its applied marker. |
| Generic dialogue completion applied door placement, scene refresh, and modal fade. | `qt_runtime/src/field/field_scene_runtime.cpp:13617-13721` (before refactor) | Lua exit executor. |
| Resource-activity dialogue completion restored its saved source and began modal fade. | `qt_runtime/src/field/field_scene_runtime.cpp:13723-13730` (before refactor) | Lua `RETURN_TO_ACTIVITY_SOURCE` and transition fields. |
| Source-scene restoration resumed profile music unconditionally. | `qt_runtime/src/field/field_scene_runtime_transitions.cpp:549` (before refactor) | Descriptor `RESUME_SCENE_PROFILE`; non-dialogue field-exit orchestration retains its separate route. |
| Door-return helper resumed music as part of party placement. | `qt_runtime/src/field/field_scene_runtime_transitions.cpp:1122-1130` (before refactor) | Descriptor `RESUME_SCENE_PROFILE`. |
| Parallel Lua same-scene return-policy helper. | `gameplay/scripts/world_links.lua:162-166` (before refactor) | Descriptor stay/source-door fields. |

Hotel and Hospital native menu checkout/leave results remain separate from the
Lua dialogue exit contract; those non-dialogue menus use
`begin_native_menu_source_entry_return`.

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
| `qt_runtime/src/field/field_scene_runtime.cpp:13594-13865` | Consume completed task outcome/result and continue its current field route | Lua executor applies exit policy; C++ dispatches gameplay-specific outcomes and nested service handoffs. |
| `qt_runtime/src/field/field_scene_runtime_services.cpp:1112-1282` | Dispatch typed dialogue outcomes and consume executor-applied markers | `continue_in_place`, service return, source route resume, or scene handoff. |
| `qt_runtime/src/field/field_scene_runtime_transitions.cpp:558,767` | Advance scene transition after modal and start modal return transition | Existing modal timing and scene transition path. |
| `gameplay/scripts/building.lua:316-349` | Select route scene outcome and attach return policy | `E.FieldLocationSceneOutcome`; same-scene service and scene-entry branches [C]. |
| `gameplay/scripts/world_links.lua:117-178,228-255` | Choose same-scene stay versus source-door return, anchor, facing, and music policy | `0xB834 -> 0xBC36/0xBD34`; away-facing write `0x09B8`; music resume through existing scene profile path [C]. |
| `gameplay/scripts/dialogue_framework.lua:176-230,285-331` | Hold the active typed source/route descriptor and execute its selected exit contract on completion | The Lua executor owns policy; native callbacks supply the individual effects. |

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
