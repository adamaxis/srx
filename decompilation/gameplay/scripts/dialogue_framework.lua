-- One opening boundary for field dialogue tasks. Dialogue graphs and scripts
-- continue to own their pages and choices; this module owns the typed origin
-- and the route-selected close contract shared by the field host.
local E = require("enums")

local DialogueFramework = {}

-- These defaults describe current port routes. Source-specific route data
-- overrides them; uncertain transition choices remain [H] until measured.
local SOURCE_DEFAULTS = {
    [E.DialogueSource.BUILDING_ENTRY] = {
        scene_outcome = E.FieldLocationSceneOutcome.SCENE_CHANGE,
        exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
        transition = E.DialogueTransition.FADE,
    }, -- Entry dispatcher 0x1651E [C]; ordinary fade timing remains [H].
    [E.DialogueSource.BUILDING_SERVICE] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Same-scene callback resumes at 0xB34/0xB52/0x2BD2 [C].
    [E.DialogueSource.BUILDING_EXIT] = {
        scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
        exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
        transition = E.DialogueTransition.FADE,
        return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
        return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
        music_return_policy = E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
    }, -- Return anchor/facing: 0xB834 -> 0xBC36/0xBD34, 0x09B8 [C].
    [E.DialogueSource.FIELD_NPC_CONTACT] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Actor dispatch 0xEB92 -> 0xECCC [C]; route timing is explicit below.
    [E.DialogueSource.FIELD_EVENT] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    },
    [E.DialogueSource.FIELD_ACTIVITY] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Route return data can select an activity destination.
    [E.DialogueSource.RANDOM_ENCOUNTER] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Each event route owns its source lock and return effects.
    [E.DialogueSource.SCRIPTED_ENCOUNTER] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Each scripted route owns its source lock and return effects.
    [E.DialogueSource.VID_PHONE] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    }, -- Vid-Phone entry at 0xE4C8 [C]; fade choice remains [H].
    [E.DialogueSource.RECRUITED_RUNNER] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Recruited-runner dispatch 0x1F58A -> 0x211E8 [C].
    [E.DialogueSource.RUN_TARGET] = {
        scene_outcome = E.FieldLocationSceneOutcome.SCENE_CHANGE,
        exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
        transition = E.DialogueTransition.FADE,
    }, -- Active run-target route `0x1F086` [C].
    [E.DialogueSource.STORY_ACTIVITY] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Story contact callbacks 0x1F146 -> 0x53C90/0x54402 [C].
    [E.DialogueSource.STORY_REWARD] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Safe handler 0x540D6 [C].
    [E.DialogueSource.PARTY_RECOVERY] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Recovery path 0xC64C; task outcome selects its continuation [C].
    [E.DialogueSource.FIELD_MENU] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
    }, -- Event-31 field terminal path 0xB7B8 -> 0x0DB26 [C].
    [E.DialogueSource.TAXI] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    }, -- Taxi transition remains source-route data [H].
    [E.DialogueSource.CINEMATIC] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    }, -- The current Qt modal return path fades after a completed task [C].
}

-- Route differences are data. Keep source behavior inherited unless a route
-- has a traced same-scene, scene-change, fade, music, or lock contract.
local ROUTE_OVERRIDES = {
    ["field_generic_contacts.present"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
        text_delay_frames = E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    },
    ["ilene.complete_scene_20_activity"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
        text_delay_frames = E.DialogueTextDelay.ILENE_FIRST_TEXT_FRAME_24,
    },
    ["salish_owlfeather.present_mitsuhama_contact"] = {
        text_delay_frames = E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    },
    ["run_employee_recruitment.present_random_contact"] = {
        text_delay_frames =
            E.DialogueTextDelay.PEDESTRIAN_FIRST_TEXT_FRAME_22,
    },
    ["michael_package.present_field_exit_prompt"] = {
        encounter_source = E.DialogueSource.RANDOM_ENCOUNTER,
        scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
        exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
        transition = E.DialogueTransition.FADE,
        return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
        return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
        music_return_policy =
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
        exit_outcome_type = "continue_in_place",
    }, -- 0x1F65A selects the exit prompt; 0x249C4 may spawn its ambush.
    ["field_random_events.present_hidden_camera"] = {
        transition = E.DialogueTransition.FADE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
    }, -- Source return includes the longplay black interval [C: section 11].
    ["field_random_events.present_haggard_man"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.FADE,
        transition_frames = E.DialogueTransitionFrames.HAGGARD_BLACK_INTERVAL,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
    }, -- Same-map return is visible at longplay frames 1224-1290 [C].
    ["salish_spirit_eyes.present"] = {
        transition = E.DialogueTransition.FADE,
    }, -- Preserve the native modal close around the typed source-context result.
    ["salish_spirit_eyes.present_renraku_ambush"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        input_lock_frames = E.DialogueInputLockFrames.WORLD_ACTIVITY,
    }, -- 0x2CE0 -> 0x53EE2 returns in place after the 60-frame activity [C].
    ["field_activities.resource_scene_main_menu"] = {
        exit_behavior = E.DialogueExitBehavior.RETURN_TO_ACTIVITY_SOURCE,
        transition = E.DialogueTransition.FADE,
        music_return_policy =
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
        exit_result_id = "return_from_world_activity",
    }, -- The source resource-scene exit restores its saved field scene.
    ["council_island_hospital.present_options"] = {
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Native Hospital Options continues the field-service menu.
    ["council_island_hospital.present_field_exit_prompt"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
        exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
        transition = E.DialogueTransition.FADE,
        return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
        return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
        music_return_policy =
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    }, -- The no-service field prompt closes through its saved door [C].
    ["council_island_hospital.present_leave_confirmation"] = {
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- The native menu owns its 131-frame Yes return timeline.
    ["council_island_hospital.present_medical_attention"] = {
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Medical attention returns to the same native service menu.
    ["tarislar_tomb_credits.present"] = {
        transition = E.DialogueTransition.NONE,
    }, -- The title return owns this closed credits page separately.
}

local NON_EXIT_TASK_RESULTS = {
    ["hotel-options"] = true,
    ["hospital-options"] = true,
    hospital_leave_confirmation_requested = true,
    hospital_cyberware_requested = true,
    hospital_medical_attention_requested = true,
    medicine_lodge_store_requested = true,
    medicine_lodge_wise_one_followup = true,
    taxi_selector_requested = true,
    cyberspace_not_converted = true,
    matrix_system_search_requested = true,
    matrix_entry_current = true,
}

local function valid_member(value, enum)
    for _, candidate in pairs(enum) do
        if value == candidate then return true end
    end
    return false
end

local function merge(target, values)
    if values == nil then return target end
    assert(type(values) == "table", "dialogue route data must be a table")
    for key, value in pairs(values) do
        assert(key == "scene_outcome" or key == "exit_behavior" or
               key == "transition" or key == "transition_frames" or
               key == "return_anchor" or
               key == "return_facing" or key == "music_return_policy" or
               key == "encounter_source" or
               key == "input_lock" or key == "input_lock_frames" or
               key == "scene_change_result_id" or
               key == "exit_result_id" or key == "exit_outcome_type" or
               key == "text_delay_frames",
            "dialogue route data has an unsupported field: " .. tostring(key))
        target[key] = value
    end
    return target
end

function DialogueFramework.describe(source, route_id, route_data)
    local defaults = SOURCE_DEFAULTS[source]
    assert(defaults ~= nil, "dialogue opening needs a typed E.DialogueSource")
    assert(type(route_id) == "string" and route_id:match("^[%w_.%-]+$") ~= nil,
        "dialogue opening needs a stable route id")
    local descriptor = {}
    for key, value in pairs(defaults) do descriptor[key] = value end
    merge(descriptor, route_data)
    if route_id == "michael_package.stoker_interaction" and
       descriptor.scene_outcome ==
           E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN then
        descriptor.exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN
        descriptor.transition = E.DialogueTransition.FADE
        descriptor.return_anchor =
            E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD
        descriptor.return_facing =
            E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY
        descriptor.music_return_policy =
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE
        descriptor.input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C
        descriptor.input_lock_frames =
            E.DialogueInputLockFrames.WORLD_ACTIVITY
    end
    if source == E.DialogueSource.BUILDING_SERVICE and
       descriptor.return_anchor ==
           E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD then
        descriptor.scene_outcome =
            E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN
    end
    if route_data ~= nil and route_data.exit_behavior == nil then
        if descriptor.scene_outcome ==
           E.FieldLocationSceneOutcome.SAME_SCENE then
            descriptor.exit_behavior = E.DialogueExitBehavior.STAY
        elseif descriptor.scene_outcome ==
               E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN then
            descriptor.exit_behavior =
                E.DialogueExitBehavior.SOURCE_DOOR_RETURN
        elseif descriptor.scene_outcome ==
               E.FieldLocationSceneOutcome.SCENE_CHANGE then
            descriptor.exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE
        end
    end
    merge(descriptor, ROUTE_OVERRIDES[route_id])
    descriptor.input_lock = descriptor.input_lock or E.DialogueInputLock.NONE
    descriptor.input_lock_frames = descriptor.input_lock_frames or
        (descriptor.input_lock == E.DialogueInputLock.NONE
            and E.DialogueInputLockFrames.NONE
            or E.DialogueInputLockFrames.WORLD_ACTIVITY)
    descriptor.text_delay_frames = descriptor.text_delay_frames or
        E.DialogueTextDelay.NONE
    descriptor.transition_frames = descriptor.transition_frames or
        (descriptor.transition == E.DialogueTransition.FADE
            and E.DialogueTransitionFrames.MODAL_RETURN
            or E.DialogueTransitionFrames.NONE)
    descriptor.exit_result_id = descriptor.exit_result_id or ""
    descriptor.exit_outcome_type = descriptor.exit_outcome_type or ""
    descriptor.return_anchor = descriptor.return_anchor or
        E.FieldLocationReturnAnchor.NONE
    descriptor.return_facing = descriptor.return_facing or
        E.FieldLocationReturnFacing.PRESERVE_ENTRY
    descriptor.music_return_policy = descriptor.music_return_policy or
        E.FieldLocationMusicReturnPolicy.NONE
    assert(valid_member(descriptor.scene_outcome, E.FieldLocationSceneOutcome),
        "dialogue descriptor has an invalid field scene outcome")
    assert(descriptor.encounter_source == nil or
           valid_member(descriptor.encounter_source, E.DialogueSource),
        "dialogue descriptor has an invalid encounter source")
    assert(valid_member(descriptor.exit_behavior, E.DialogueExitBehavior),
        "dialogue descriptor has an invalid exit behavior")
    assert(type(descriptor.exit_result_id) == "string" and
           (descriptor.exit_result_id == "" or
            descriptor.exit_result_id:match("^[%w_.%-]+$") ~= nil),
        "dialogue descriptor has an invalid exit result id")
    assert(type(descriptor.exit_outcome_type) == "string" and
           (descriptor.exit_outcome_type == "" or
            descriptor.exit_outcome_type:match("^[%w_.%-]+$") ~= nil),
        "dialogue descriptor has an invalid exit outcome type")
    assert(valid_member(descriptor.transition, E.DialogueTransition),
        "dialogue descriptor has an invalid transition")
    assert(type(descriptor.transition_frames) == "number" and
           descriptor.transition_frames % 1 == 0 and
           descriptor.transition_frames >= 0 and
           descriptor.transition_frames <= 255 and
           ((descriptor.transition == E.DialogueTransition.NONE and
             descriptor.transition_frames == E.DialogueTransitionFrames.NONE) or
            (descriptor.transition == E.DialogueTransition.FADE and
             descriptor.transition_frames > 0)),
        "dialogue descriptor has invalid transition frames")
    assert(descriptor.return_anchor == nil or
           valid_member(descriptor.return_anchor, E.FieldLocationReturnAnchor),
        "dialogue descriptor has an invalid return anchor")
    assert(descriptor.return_facing == nil or
           valid_member(descriptor.return_facing, E.FieldLocationReturnFacing),
        "dialogue descriptor has an invalid return facing")
    assert(descriptor.music_return_policy == nil or
           valid_member(descriptor.music_return_policy,
                        E.FieldLocationMusicReturnPolicy),
        "dialogue descriptor has an invalid music return policy")
    assert(descriptor.input_lock == nil or
           valid_member(descriptor.input_lock, E.DialogueInputLock),
        "dialogue descriptor has an invalid input lock")
    assert(type(descriptor.input_lock_frames) == "number" and
           descriptor.input_lock_frames % 1 == 0 and
           descriptor.input_lock_frames >= 0 and
           descriptor.input_lock_frames <= 255 and
           ((descriptor.input_lock == E.DialogueInputLock.NONE and
             descriptor.input_lock_frames == E.DialogueInputLockFrames.NONE) or
            (descriptor.input_lock ~= E.DialogueInputLock.NONE and
             descriptor.input_lock_frames > 0)),
        "dialogue descriptor has invalid input-lock frames")
    assert(valid_member(descriptor.text_delay_frames, E.DialogueTextDelay),
        "dialogue descriptor has an invalid first-text delay")
    assert(descriptor.exit_behavior ~=
               E.DialogueExitBehavior.SOURCE_DOOR_RETURN or
           (descriptor.return_anchor ==
                E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD and
            valid_member(descriptor.return_facing,
                         E.FieldLocationReturnFacing)),
        "source-door return needs a named anchor and facing")
    descriptor.source = source
    descriptor.route_id = route_id
    return descriptor
end

local function run_exit_primitive(action)
    assert(type(game.services) == "table" and
           type(game.services.call) == "function",
        "dialogue exit executor needs the host primitive service")
    local result = game.services.call("dialogue_exit", action)
    assert(result == "applied",
        "dialogue exit primitive was not applied: " .. action)
end

-- Lua chooses exit effects from the descriptor. Native code implements only
-- the individual operations requested here.
function DialogueFramework.execute_exit(descriptor, task_result)
    assert(type(descriptor) == "table",
        "dialogue exit executor needs a descriptor")
    -- A source boundary leaves the dialogue open for investigation. Other
    -- typed task results can return control to a caller after this dialogue
    -- has completed, so the active descriptor still owns its close effects.
    if type(task_result) == "table" and type(task_result.type) == "string" then
        local outcome_type = task_result.type
        if outcome_type == "source_boundary" then
            return
        end
        local returns_to_source_context =
            outcome_type == "play_cinematic" and
            task_result.returnPolicy == "source_context"
        if outcome_type ~= "continue_in_place" and
           outcome_type ~= "return_to_caller" and
           outcome_type ~= "resume_source_route" and
           outcome_type ~= "return_to_origin" and
           not returns_to_source_context then
            return
        end
    elseif type(task_result) == "string" then
        if NON_EXIT_TASK_RESULTS[task_result] or
           task_result:match("^matrix_entry_") ~= nil then
            return
        end
    end
    local execute_scene_change = false
    if descriptor.exit_behavior == E.DialogueExitBehavior.SCENE_CHANGE then
        local is_source_route_resume = type(task_result) == "table" and
            task_result.type == "resume_source_route"
        local has_selected_result =
            descriptor.scene_change_result_id ~= nil and
            descriptor.scene_change_result_id ~= ""
        local is_selected_result =
            has_selected_result and
            task_result == descriptor.scene_change_result_id
        execute_scene_change = is_source_route_resume or
            ((not has_selected_result or is_selected_result) and
             type(task_result) ~= "table")
    end
    if descriptor.exit_outcome_type ~= nil and
       descriptor.exit_outcome_type ~= "" and
       (type(task_result) ~= "table" or
        task_result.type ~= descriptor.exit_outcome_type) then
        return
    end
    if descriptor.exit_result_id ~= nil and
       descriptor.exit_result_id ~= "" and
       task_result ~= descriptor.exit_result_id then
        return
    end
    if descriptor.exit_behavior == E.DialogueExitBehavior.STAY then
        run_exit_primitive("stay")
    elseif descriptor.exit_behavior ==
           E.DialogueExitBehavior.SOURCE_DOOR_RETURN then
        run_exit_primitive("source_door_return_" ..
            descriptor.return_anchor .. "_" .. descriptor.return_facing)
    elseif descriptor.exit_behavior == E.DialogueExitBehavior.SCENE_CHANGE then
        if execute_scene_change then
            run_exit_primitive("scene_change")
        end
    elseif descriptor.exit_behavior ==
           E.DialogueExitBehavior.RETURN_TO_ACTIVITY_SOURCE then
        run_exit_primitive("return_to_activity_source")
    elseif descriptor.exit_behavior ~= E.DialogueExitBehavior.NONE then
        error("dialogue exit executor received an invalid behavior")
    end

    if descriptor.transition == E.DialogueTransition.FADE then
        local frames = descriptor.transition_frames
        local action = frames == E.DialogueTransitionFrames.MODAL_RETURN
            and "fade" or ("fade_" .. frames)
        run_exit_primitive(action)
    elseif descriptor.transition ~= E.DialogueTransition.NONE then
        error("dialogue exit executor received an invalid transition")
    end

    if descriptor.music_return_policy ==
       E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE then
        run_exit_primitive("resume_scene_profile")
    elseif descriptor.music_return_policy ~=
           E.FieldLocationMusicReturnPolicy.NONE then
        error("dialogue exit executor received an invalid music policy")
    end

    if descriptor.input_lock ==
       E.DialogueInputLock.WORLD_ACTIVITY_FFE11C then
        run_exit_primitive("lock_" .. descriptor.input_lock_frames)
    elseif descriptor.input_lock ~= E.DialogueInputLock.NONE then
        error("dialogue exit executor received an invalid input lock")
    end
end

function DialogueFramework.open(source, module_name, function_name,
                                scene_outcome, exit_behavior, transition,
                                return_anchor, return_facing,
                                music_return_policy, input_lock,
                                text_delay_frames, input_lock_frames,
                                scene_change_result_id, transition_frames)
    assert(type(module_name) == "string" and
           module_name:match("^[%a_][%w_]*$") ~= nil and
           type(function_name) == "string" and
           function_name:match("^[%a_][%w_]*$") ~= nil,
        "dialogue opening needs a Lua task target")
    local route_id = module_name .. "." .. function_name
    local route_data = {}
    for key, value in pairs({
        scene_outcome = scene_outcome,
        exit_behavior = exit_behavior,
        transition = transition,
        transition_frames = transition_frames,
        return_anchor = return_anchor,
        return_facing = return_facing,
        music_return_policy = music_return_policy,
        input_lock = input_lock,
        input_lock_frames = input_lock_frames,
        text_delay_frames = text_delay_frames,
        scene_change_result_id = scene_change_result_id,
    }) do
        if value ~= "" then route_data[key] = value end
    end
    local module = require(module_name)
    local handler = module[function_name]
    if type(handler) ~= "function" then
        error("dialogue task is missing: " .. route_id, 2)
    end
    -- `game` is the host facade and has no dialogue state machine of its own.
    -- Keep the framework's small contract record in the Lua-owned table.
    game.dialogue = game.dialogue or {}
    local dialogue = game.dialogue
    assert(type(dialogue) == "table", "game dialogue framework state is invalid")
    local previous = dialogue.active
    local resolved_route_data = {}
    for key, value in pairs(route_data) do
        resolved_route_data[key] = value
    end
    local route_data_by_id = type(dialogue.route_data) == "table" and
        dialogue.route_data[route_id] or nil
    if route_data_by_id ~= nil then
        for key, value in pairs(route_data_by_id) do
            resolved_route_data[key] = value
        end
    end
    local descriptor = DialogueFramework.describe(
        source, route_id, resolved_route_data)
    dialogue.open_count = (dialogue.open_count or 0) + 1
    dialogue.active = descriptor
    local ok, result = pcall(function()
        local task_result = handler()
        local is_boundary = type(task_result) == "table" and
            task_result.type == "source_boundary"
        if not is_boundary then
            DialogueFramework.execute_exit(descriptor, task_result)
        end
        return task_result
    end)
    dialogue.last_opening = descriptor
    dialogue.active = previous
    if not ok then error(result, 0) end
    return result
end

function DialogueFramework.source_defaults()
    local result = {}
    for source, descriptor in pairs(SOURCE_DEFAULTS) do
        local copy = {}
        for key, value in pairs(descriptor) do copy[key] = value end
        result[source] = copy
    end
    return result
end

return DialogueFramework
