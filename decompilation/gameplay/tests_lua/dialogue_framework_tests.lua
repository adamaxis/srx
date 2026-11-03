local E = require("enums")
local DialogueFramework = require("dialogue_framework")

local function equal(actual, expected, message)
    assert(actual == expected,
        (message or "value mismatch") .. ": expected " .. tostring(expected) ..
        ", got " .. tostring(actual))
end

local defaults = DialogueFramework.source_defaults()
local source_count = 0
for _, source in pairs(E.DialogueSource) do
    source_count = source_count + 1
    assert(defaults[source], "dialogue source is missing a default: " .. source)
end
local default_count = 0
for _ in pairs(defaults) do default_count = default_count + 1 end
equal(default_count, source_count, "every dialogue source has one default")

local contact = DialogueFramework.describe(
    E.DialogueSource.FIELD_NPC_CONTACT, "field_generic_contacts.present")
equal(contact.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
    "NPC contact stays in its scene")
equal(contact.exit_behavior, E.DialogueExitBehavior.STAY,
    "NPC contact has a stay-in-place close")
equal(contact.input_lock, E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    "generic contact declares the shared input lock")
equal(contact.text_delay_frames,
    E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    "generic contact follows its measured first-text frame")

local ilene_contact = DialogueFramework.describe(
    E.DialogueSource.STORY_ACTIVITY,
    "ilene.complete_scene_20_activity")
equal(ilene_contact.text_delay_frames,
    E.DialogueTextDelay.ILENE_FIRST_TEXT_FRAME_24,
    "Ilene contact follows its measured first-text frame")

local pedestrian_contact = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "run_employee_recruitment.present_random_contact")
equal(pedestrian_contact.text_delay_frames,
    E.DialogueTextDelay.PEDESTRIAN_FIRST_TEXT_FRAME_22,
    "random pedestrian contact uses its measured first-text frame")

local aragorn_contact = DialogueFramework.describe(
    E.DialogueSource.STORY_ACTIVITY,
    "salish_owlfeather.present_mitsuhama_contact")
equal(aragorn_contact.text_delay_frames,
    E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    "Aragorn's story contact uses the common measured first-text frame")

local exit = DialogueFramework.describe(
    E.DialogueSource.BUILDING_EXIT, "michael_package.present_field_exit_prompt")
equal(exit.scene_outcome, E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
    "building exit returns through its source door")
equal(exit.return_anchor, E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    "building exit uses the source door anchor")
equal(exit.return_facing,
    E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    "building exit faces away from its entry")
equal(exit.music_return_policy,
    E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    "building exit resumes the scene music profile")

local routed = DialogueFramework.describe(
    E.DialogueSource.FIELD_EVENT, "fixture.stays",
    { scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
      exit_behavior = E.DialogueExitBehavior.STAY,
      transition = E.DialogueTransition.NONE })
equal(routed.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
    "route data can select the same-scene outcome")
local unlocked = DialogueFramework.describe(
    E.DialogueSource.SCRIPTED_ENCOUNTER, "fixture.unlocked")
equal(unlocked.input_lock, E.DialogueInputLock.NONE,
    "generic encounter descriptors do not assume an activity lock")
equal(unlocked.input_lock_frames, E.DialogueInputLockFrames.NONE,
    "encounter locks belong to the traced event route")
equal(unlocked.text_delay_frames, E.DialogueTextDelay.NONE,
    "unrelated dialogues do not inherit contact presentation delay")
local unrelated_field_route = DialogueFramework.describe(
    E.DialogueSource.FIELD_NPC_CONTACT, "fixture.field_service")
equal(unrelated_field_route.text_delay_frames, E.DialogueTextDelay.NONE,
    "unrelated field services do not inherit actor-contact presentation delay")
equal(unlocked.return_anchor, E.FieldLocationReturnAnchor.NONE,
    "a dialogue declares no source-door return by default")
equal(unlocked.music_return_policy, E.FieldLocationMusicReturnPolicy.NONE,
    "a dialogue declares no music resume by default")
local no_lock = DialogueFramework.describe(
    E.DialogueSource.FIELD_EVENT, "fixture.no_lock")
equal(no_lock.input_lock_frames, E.DialogueInputLockFrames.NONE,
    "a dialogue without a lock has zero lock frames")
local haggard = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "field_random_events.present_haggard_man")
equal(haggard.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
    "the doctor result resumes its current field scene")
equal(haggard.exit_behavior, E.DialogueExitBehavior.STAY,
    "the doctor event leaves the player on the current map")
equal(haggard.transition, E.DialogueTransition.FADE,
    "the doctor result uses its measured return fade")
equal(haggard.transition_frames,
    E.DialogueTransitionFrames.HAGGARD_BLACK_INTERVAL,
    "the doctor descriptor carries its measured 83-frame black interval")
equal(haggard.music_return_policy, E.FieldLocationMusicReturnPolicy.NONE,
    "the doctor event does not resume a building profile")
equal(haggard.input_lock_frames,
    E.DialogueInputLockFrames.WORLD_ACTIVITY,
    "the doctor event carries the source world-activity lock")
local renraku = DialogueFramework.describe(
    E.DialogueSource.SCRIPTED_ENCOUNTER,
    "salish_spirit_eyes.present_renraku_ambush")
equal(renraku.exit_behavior, E.DialogueExitBehavior.STAY,
    "the Scene-47 Renraku notice returns in place")
equal(renraku.transition, E.DialogueTransition.NONE,
    "the Scene-47 Renraku notice has no exit fade")
equal(renraku.input_lock_frames,
    E.DialogueInputLockFrames.WORLD_ACTIVITY,
    "the Scene-47 mode activity carries the 60-frame lock")
local stoker_exit = DialogueFramework.describe(
    E.DialogueSource.BUILDING_EXIT,
    "michael_package.present_field_exit_prompt")
equal(stoker_exit.exit_behavior,
    E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    "the Stoker 0x249C4 exit prompt returns through its source door")
equal(stoker_exit.input_lock_frames,
    E.DialogueInputLockFrames.WORLD_ACTIVITY,
    "the Stoker exit descriptor carries its 60-frame lock")
equal(stoker_exit.encounter_source, E.DialogueSource.RANDOM_ENCOUNTER,
    "the Stoker 0x249C4 branch is typed as a random encounter")
equal(stoker_exit.exit_outcome_type, "continue_in_place",
    "the Stoker door exit applies only when its field return completes")
local stoker_interaction = DialogueFramework.describe(
    E.DialogueSource.BUILDING_SERVICE,
    "michael_package.stoker_interaction")
equal(stoker_interaction.exit_behavior,
    E.DialogueExitBehavior.STAY,
    "the indexed Stoker conversation stays when its source context does")
equal(stoker_interaction.return_anchor,
    E.FieldLocationReturnAnchor.NONE,
    "the same-context Stoker conversation has no door placement")
equal(stoker_interaction.return_facing,
    E.FieldLocationReturnFacing.PRESERVE_ENTRY,
    "the same-context Stoker conversation preserves its facing")
equal(stoker_interaction.transition, E.DialogueTransition.FADE,
    "the indexed Stoker interaction selects its fade")
equal(stoker_interaction.music_return_policy,
    E.FieldLocationMusicReturnPolicy.NONE,
    "the same-context Stoker conversation does not restart outside music")
equal(stoker_interaction.input_lock_frames,
    E.DialogueInputLockFrames.NONE,
    "the same-context Stoker conversation does not add an outside lock")
local stoker_field_exit = DialogueFramework.describe(
    E.DialogueSource.BUILDING_SERVICE,
    "michael_package.stoker_interaction", {
        scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
    })
equal(stoker_field_exit.exit_behavior,
    E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    "the Stoker source-door context selects the recorded exception")
equal(stoker_field_exit.input_lock_frames,
    E.DialogueInputLockFrames.WORLD_ACTIVITY,
    "the Stoker source-door context sets FFE11C to 60")
local stoker_package_page = DialogueFramework.describe(
    E.DialogueSource.BUILDING_SERVICE,
    "michael_package.present_package_handoff")
equal(stoker_package_page.exit_behavior,
    E.DialogueExitBehavior.STAY,
    "the package handoff remains in its current context")
equal(stoker_package_page.return_anchor,
    E.FieldLocationReturnAnchor.NONE,
    "the package handoff does not place the party independently")
equal(stoker_package_page.return_facing,
    E.FieldLocationReturnFacing.PRESERVE_ENTRY,
    "the package handoff preserves its caller's facing")
equal(stoker_package_page.music_return_policy,
    E.FieldLocationMusicReturnPolicy.NONE,
    "the package handoff does not restart field music independently")
equal(stoker_package_page.input_lock_frames,
    E.DialogueInputLockFrames.NONE,
    "the package handoff does not add a separate outside lock")
local activity_return = DialogueFramework.describe(
    E.DialogueSource.FIELD_MENU,
    "field_activities.resource_scene_main_menu")
equal(activity_return.exit_behavior,
    E.DialogueExitBehavior.RETURN_TO_ACTIVITY_SOURCE,
    "resource exit selects the executor's saved-scene restore")
equal(activity_return.music_return_policy,
    E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    "resource return declares its music policy")

local prior_game = rawget(_G, "game")
local exit_calls = {}
_G.game = { services = { call = function(domain, action)
    assert(domain == "dialogue_exit",
        "dialogue executor invoked a non-exit service")
    exit_calls[#exit_calls + 1] = action
    return "applied"
end } }
package.preload.dialogue_framework_fixture = function()
    return {
        run = function()
            equal(game.dialogue.active.source,
                E.DialogueSource.RANDOM_ENCOUNTER,
                "opening source is active in the task")
            equal(game.dialogue.active.scene_outcome,
                E.FieldLocationSceneOutcome.SAME_SCENE,
                "opening applies its route outcome")
            local resumed, answer = coroutine.yield("fixture-page")
            assert(resumed)
            equal(answer, "continue", "task resumes through its open call")
            equal(game.dialogue.active.route_id,
                "dialogue_framework_fixture.run",
                "route stays active for every yielded page")
            return "fixture-complete"
        end,
    }
end

local task = coroutine.create(function()
    return DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "dialogue_framework_fixture", "run",
        E.FieldLocationSceneOutcome.SAME_SCENE,
        E.DialogueExitBehavior.STAY,
        E.DialogueTransition.NONE)
end)
local started, page = coroutine.resume(task)
assert(started, page)
equal(page, "fixture-page", "task yields its first page")
equal(game.dialogue.open_count, 1, "framework records the opening")
equal(game.dialogue.active.route_id,
    "dialogue_framework_fixture.run", "framework retains active route")
local resumed, result = coroutine.resume(task, true, "continue")
assert(resumed, result)
equal(result, "fixture-complete", "framework returns the task result")
equal(game.dialogue.active, nil, "framework clears active route on return")
equal(game.dialogue.last_opening.source,
    E.DialogueSource.RANDOM_ENCOUNTER,
    "framework records the completed opening")
equal(#exit_calls, 1, "a completed task executes its exit descriptor")
equal(exit_calls[1], "stay",
    "the route's stay behavior is executed after its task")

local function open_exit_fixture(route_data, task_result)
    exit_calls = {}
    package.loaded.dialogue_framework_fixture = nil
    game.dialogue = { route_data = {
        ["dialogue_framework_fixture.exit"] = route_data,
    } }
    package.preload.dialogue_framework_fixture = function()
        return { exit = function() return task_result or "closed" end }
    end
    local exit_task = coroutine.create(function()
        return DialogueFramework.open(
            E.DialogueSource.FIELD_EVENT,
            "dialogue_framework_fixture", "exit")
    end)
    local opened, result = coroutine.resume(exit_task)
    assert(opened, result)
    equal(result, task_result or "closed",
        "executor preserves the dialogue task result")
    return exit_calls
end

local changed_exit = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.STAY,
    transition = E.DialogueTransition.FADE,
    transition_frames = 17,
    music_return_policy = E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 17,
})
equal(table.concat(changed_exit, ","),
    "stay,fade_17,resume_scene_profile,lock_17",
    "descriptor values control stay, fade, music, and lock frame effects")

local changed_return = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    transition = E.DialogueTransition.NONE,
    return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    music_return_policy = E.FieldLocationMusicReturnPolicy.NONE,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 9,
})
equal(table.concat(changed_return, ","),
    "source_door_return_collision_probe_lead_face_away_from_entry,lock_9",
    "descriptor can select source-door placement without fade or music")

local accepted_scene_change = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
    transition = E.DialogueTransition.FADE,
    scene_change_result_id = "accepted",
}, "accepted")
equal(table.concat(accepted_scene_change, ","), "scene_change,fade",
    "descriptor can select a source route and fade")

local declined_scene_change = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
    transition = E.DialogueTransition.FADE,
    scene_change_result_id = "accepted",
}, "declined")
equal(table.concat(declined_scene_change, ","), "fade",
    "an unselected source-route result skips its scene change but keeps the descriptor fade")

local boundary_exit = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    transition = E.DialogueTransition.FADE,
    return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 60,
}, { type = "source_boundary", reason = "unresolved" })
equal(#boundary_exit, 0,
    "an unresolved source boundary does not apply dialogue exit effects")
local caller_return_exit = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.STAY,
    transition = E.DialogueTransition.FADE,
}, { type = "return_to_caller" })
equal(table.concat(caller_return_exit, ","), "stay,fade",
    "a completed dialogue applies its descriptor before returning to its caller")
local cinematic_return_exit = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.STAY,
    transition = E.DialogueTransition.FADE,
}, {
    type = "play_cinematic",
    cinematicId = "fixture.cinematic",
    returnPolicy = "source_context",
})
equal(table.concat(cinematic_return_exit, ","), "stay,fade",
    "a source-context cinematic applies its dialogue descriptor after the pages")
local service_handoff = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    transition = E.DialogueTransition.FADE,
    return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    exit_outcome_type = "continue_in_place",
}, { type = "open_service", service_id = "hospital.options" })
equal(#service_handoff, 0,
    "opening an in-building service does not trigger the building exit")
local unguarded_service_handoff = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
    transition = E.DialogueTransition.FADE,
    return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    music_return_policy =
        E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 60,
}, { type = "open_service", service_id = "matrix.system_search" })
equal(#unguarded_service_handoff, 0,
    "typed service handoffs never inherit dialogue-exit side effects")
local field_scene_handoff = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
    transition = E.DialogueTransition.FADE,
    music_return_policy =
        E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 60,
}, { type = "enter_field_scene", sceneId = 6,
     arrivalId = "salish_sinsearach_copt" })
equal(#field_scene_handoff, 0,
    "typed field-scene handoffs leave transitions to their receiving route")
local matrix_handoff = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.STAY,
    transition = E.DialogueTransition.FADE,
    input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    input_lock_frames = 60,
}, "matrix_system_search_requested")
equal(#matrix_handoff, 0,
    "field Matrix routes hand off without running dialogue-exit effects")
local activity_exit = open_exit_fixture({
    exit_behavior = E.DialogueExitBehavior.RETURN_TO_ACTIVITY_SOURCE,
    transition = E.DialogueTransition.FADE,
    music_return_policy =
        E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    exit_result_id = "return_from_world_activity",
}, "return_from_world_activity")
equal(table.concat(activity_exit, ","),
    "return_to_activity_source,fade,resume_scene_profile",
    "resource exit applies saved-scene, transition, and music policy")

local function execute_source_route(source, route_id, task_result, route_data)
    exit_calls = {}
    local descriptor = DialogueFramework.describe(source, route_id, route_data)
    DialogueFramework.execute_exit(descriptor, task_result)
    return table.concat(exit_calls, ",")
end

equal(execute_source_route(
          E.DialogueSource.RANDOM_ENCOUNTER,
          "field_random_events.present_haggard_man", "resolved"),
      "stay,fade_83,lock_60",
      "Haggard exit executor runtime requests same-scene 83-frame fade and lock 60")
equal(execute_source_route(
          E.DialogueSource.SCRIPTED_ENCOUNTER,
          "salish_spirit_eyes.present_renraku_ambush",
          { type = "continue_in_place" }),
      "stay,lock_60",
      "Renraku exit executor runtime requests stay and lock 60 only")
equal(execute_source_route(
          E.DialogueSource.RUN_TARGET,
          "run_employee_recruitment.present_random_contact", "resolved"),
      "scene_change,fade",
      "an unfiltered completed run-target task resumes its pending scene route")
equal(execute_source_route(
          E.DialogueSource.RUN_TARGET,
          "run_employee_recruitment.present_random_contact",
          { type = "return_to_caller" }),
      "fade",
      "typed caller handoffs skip scene change and retain the descriptor fade")
equal(execute_source_route(
          E.DialogueSource.BUILDING_EXIT,
          "michael_package.present_field_exit_prompt",
          { type = "continue_in_place" }),
      "source_door_return_collision_probe_lead_face_away_from_entry," ..
      "fade,resume_scene_profile,lock_60",
      "Stoker exit executor runtime requests door return, fade, music, lock")
equal(execute_source_route(
          E.DialogueSource.BUILDING_SERVICE,
          "michael_package.stoker_interaction",
          { type = "continue_in_place" }),
      "stay,fade",
      "Stoker's same-context interaction keeps #28c STAY")
equal(execute_source_route(
          E.DialogueSource.BUILDING_SERVICE,
          "michael_package.stoker_interaction",
          { type = "continue_in_place" }, {
              scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
          }),
      "source_door_return_collision_probe_lead_face_away_from_entry," ..
      "fade,resume_scene_profile,lock_60",
      "Stoker's typed source-door exception applies all effects through the executor")
equal(execute_source_route(
          E.DialogueSource.BUILDING_SERVICE,
          "michael_package.present_package_handoff",
          { type = "continue_in_place" }),
      "stay,fade",
      "the standalone package-handoff page retains its same-context close")

package.preload.dialogue_framework_fixture = nil
_G.game = prior_game

-- Runtime task starts in the Qt host must use the one typed dialogue entry.
-- The remaining direct generic task start is the non-dialogue field event.
local function read_file(path)
    local file = assert(io.open(path, "rb"), "could not read " .. path)
    local contents = file:read("*a")
    file:close()
    return contents
end

local opener_path =
    "decompilation/ghidra_conversion/qt_runtime/src/dialogue/field_dialogue_runtime.cpp"
local opener_source = read_file(opener_path)
assert(opener_source:find("script_runtime_->start_dialogue_task(", 1, true),
    "FieldDialogueRuntime must invoke the Lua dialogue framework")
assert(not opener_source:find("script_runtime_->start_task(", 1, true),
    "FieldDialogueRuntime must not start raw Lua tasks")

local direct_starts = assert(io.popen(
    "rg -n '\\.start_task[[:space:]]*\\(' " ..
    "decompilation/ghidra_conversion/qt_runtime/src", "r"))
local allowed_non_dialogue_start = 0
for line in direct_starts:lines() do
    if line:find(
        "field_scene_runtime_transitions.cpp:", 1, true) and
       line:find('game_scripts_.start_task("field_events", activation.handler)',
                 1, true) then
        allowed_non_dialogue_start = allowed_non_dialogue_start + 1
    else
        error("raw C++ task start bypasses dialogue framework: " .. line)
    end
end
local _, _, command_status = direct_starts:close()
assert(command_status == 0 or command_status == true,
    "could not scan Qt runtime task starts")
equal(allowed_non_dialogue_start, 1,
    "only the known non-dialogue field event uses a raw task start")

local services_source = read_file(
    "decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_services.cpp")
local continue_start = assert(services_source:find(
    "bool FieldSceneRuntime::handle_dialogue_continue_in_place(", 1, true))
local cinematic_start = assert(services_source:find(
    "bool FieldSceneRuntime::handle_dialogue_play_cinematic(",
    continue_start, true))
local continue_body = services_source:sub(continue_start, cinematic_start - 1)
assert(not continue_body:find("apply_field_door_exit_return(", 1, true) and
       not continue_body:find("advance_field_scene_transition_after_modal(", 1, true) and
       not continue_body:find("begin_field_modal_return_transition(", 1, true),
    "typed continue outcomes must not apply placement or transitions")
local resume_start = assert(services_source:find(
    "bool FieldSceneRuntime::handle_dialogue_resume_source_route(",
    cinematic_start, true))
local enter_scene_start = assert(services_source:find(
    "bool FieldSceneRuntime::handle_dialogue_enter_field_scene(",
    resume_start, true))
local resume_body = services_source:sub(resume_start, enter_scene_start - 1)
assert(not resume_body:find("resume_field_mode_entry_route(", 1, true) and
       not resume_body:find("advance_field_scene_transition_after_modal(", 1, true) and
       not resume_body:find("begin_field_modal_return_transition(", 1, true),
    "typed source-route outcomes must not apply scene exits")
local return_to_entry_start = assert(services_source:find(
    "bool FieldSceneRuntime::handle_dialogue_return_to_source_entry(",
    enter_scene_start, true))
local native_menu_return_start = assert(services_source:find(
    "bool FieldSceneRuntime::begin_native_menu_source_entry_return(",
    return_to_entry_start, true))
local return_to_entry_body = services_source:sub(
    return_to_entry_start, native_menu_return_start - 1)
assert(not return_to_entry_body:find("begin_field_door_return(", 1, true) and
       not return_to_entry_body:find("apply_field_door_exit_return(", 1, true),
    "typed return-to-origin outcomes must not place the party outside the executor")
assert(not return_to_entry_body:find(
           "begin_field_modal_return_transition(", 1, true) and
       not return_to_entry_body:find(
           "advance_field_scene_transition_after_modal(", 1, true) and
       not return_to_entry_body:find("scene_music_latch_ = true", 1, true),
    "typed return-to-origin outcomes must not transition or resume music outside the executor")

local runtime_source = read_file(
    "decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp")
local world_effects_source = read_file(
    "decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_world_effects.cpp")
local event_dispatch_start = assert(world_effects_source:find(
    "bool FieldSceneRuntime::present_field_dialogue_event(", 1, true))
local event_dispatch_end = assert(world_effects_source:find(
    "void FieldSceneRuntime::trigger_field_world_effect_sequence(",
    event_dispatch_start, true))
local event_dispatch_body = world_effects_source:sub(
    event_dispatch_start, event_dispatch_end - 1)
for _, side_effect in ipairs({
    "apply_field_door_exit_return(",
    "begin_field_door_return(",
    "advance_field_scene_transition_after_modal(",
    "begin_field_modal_return_transition(",
    "scene_music_latch_ = true",
    "field_world_activity_state_.timeout =",
}) do
    assert(not event_dispatch_body:find(side_effect, 1, true),
        "field encounter dispatch applies dialogue exit side effect outside " ..
        "the Lua executor: " .. side_effect)
end
local returned_start = assert(runtime_source:find(
    "FieldDialogueAdvanceResult::returned_to_field", 1, true))
local returned_end = assert(runtime_source:find(
    "FieldDialogueAdvanceResult::returned_from_world_activity",
    returned_start, true))
local returned_body = runtime_source:sub(returned_start, returned_end - 1)
assert(not returned_body:find("apply_field_door_exit_return(", 1, true) and
       not returned_body:find("advance_field_scene_transition_after_modal(", 1, true) and
       not returned_body:find("begin_field_modal_return_transition(", 1, true),
    "completed dialogue tasks must not apply placement or transitions outside the executor")
local activity_return_start = assert(runtime_source:find(
    "FieldDialogueAdvanceResult::returned_from_world_activity",
    returned_end, true))
local activity_return_end = assert(runtime_source:find(
    "FieldDialogueAdvanceResult::hotel_options_requested",
    activity_return_start, true))
local activity_return_body = runtime_source:sub(
    activity_return_start, activity_return_end - 1)
assert(not activity_return_body:find("restore_world_activity_source_scene(", 1, true) and
       not activity_return_body:find("begin_field_modal_return_transition(", 1, true),
    "resource-scene dialogue returns must be applied by the Lua executor")
local restore_source = read_file(
    "decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp")
local restore_start = assert(restore_source:find(
    "bool FieldSceneRuntime::restore_world_activity_source_scene(", 1, true))
local restore_end = assert(restore_source:find(
    "void FieldSceneRuntime::advance_field_scene_transition_after_modal(",
    restore_start, true))
local restore_body = restore_source:sub(restore_start, restore_end - 1)
assert(not restore_body:find("scene_music_latch_ = true", 1, true),
    "resource source restore must leave music policy to its caller")
assert(not services_source:find("same_scene_return_policy", 1, true),
    "the parallel Lua same-scene exit policy is no longer queried by C++")

print("dialogue framework opening contract passed")
