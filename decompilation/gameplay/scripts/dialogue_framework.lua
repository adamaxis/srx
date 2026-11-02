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
        transition = E.DialogueTransition.NONE,
    }, -- Same-scene callback resumes at 0xB34/0xB52/0x2BD2 [C].
    [E.DialogueSource.BUILDING_EXIT] = {
        scene_outcome = E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
        exit_behavior = E.DialogueExitBehavior.SOURCE_DOOR_RETURN,
        transition = E.DialogueTransition.FADE,
        return_anchor = E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
        return_facing = E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
        music_return_policy = E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    }, -- Return anchor/facing: 0xB834 -> 0xBC36/0xBD34, 0x09B8 [C].
    [E.DialogueSource.FIELD_NPC_CONTACT] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Actor dispatch 0xEB92 -> 0xECCC [C]; route timing is explicit below.
    [E.DialogueSource.FIELD_EVENT] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    },
    [E.DialogueSource.FIELD_ACTIVITY] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Route return data can select an activity destination.
    [E.DialogueSource.RANDOM_ENCOUNTER] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Scene/event-specific encounter exit remains route data [H].
    [E.DialogueSource.SCRIPTED_ENCOUNTER] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Encounter transition remains route data [H].
    [E.DialogueSource.VID_PHONE] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    }, -- Vid-Phone entry at 0xE4C8 [C]; fade choice remains [H].
    [E.DialogueSource.RECRUITED_RUNNER] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Recruited-runner dispatch 0x1F58A -> 0x211E8 [C].
    [E.DialogueSource.RUN_TARGET] = {
        scene_outcome = E.FieldLocationSceneOutcome.SCENE_CHANGE,
        exit_behavior = E.DialogueExitBehavior.SCENE_CHANGE,
        transition = E.DialogueTransition.FADE,
    }, -- Active run-target route `0x1F086` [C].
    [E.DialogueSource.STORY_ACTIVITY] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Story contact callbacks 0x1F146 -> 0x53C90/0x54402 [C].
    [E.DialogueSource.STORY_REWARD] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Safe handler 0x540D6 [C].
    [E.DialogueSource.PARTY_RECOVERY] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Recovery path 0xC64C; task outcome selects its continuation [C].
    [E.DialogueSource.FIELD_MENU] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    }, -- Event-31 field terminal path 0xB7B8 -> 0x0DB26 [C].
    [E.DialogueSource.TAXI] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.FADE,
    }, -- Taxi transition remains source-route data [H].
    [E.DialogueSource.CINEMATIC] = {
        scene_outcome = E.FieldLocationSceneOutcome.NONE,
        exit_behavior = E.DialogueExitBehavior.NONE,
        transition = E.DialogueTransition.NONE,
    }, -- Scene-55 task entry linkage remains [H].
}

-- Route differences are data. Keep source behavior inherited unless a route
-- has a traced same-scene, scene-change, fade, music, or lock contract.
local ROUTE_OVERRIDES = {
    ["field_generic_contacts.present"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        text_delay_frames = E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    },
    ["ilene.complete_scene_20_activity"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
        input_lock = E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
        text_delay_frames = E.DialogueTextDelay.ILENE_FIRST_TEXT_FRAME_24,
    },
    ["salish_owlfeather.present_mitsuhama_contact"] = {
        text_delay_frames = E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23,
    },
    ["run_employee_recruitment.present_random_contact"] = {
        text_delay_frames =
            E.DialogueTextDelay.PEDESTRIAN_FIRST_TEXT_FRAME_22,
    },
    ["michael_package.stoker_interaction"] = {
        scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
        exit_behavior = E.DialogueExitBehavior.STAY,
        transition = E.DialogueTransition.NONE,
    },
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
               key == "transition" or key == "return_anchor" or
               key == "return_facing" or key == "music_return_policy" or
               key == "input_lock" or key == "text_delay_frames",
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
    merge(descriptor, ROUTE_OVERRIDES[route_id])
    merge(descriptor, route_data)
    descriptor.input_lock = descriptor.input_lock or E.DialogueInputLock.NONE
    descriptor.text_delay_frames = descriptor.text_delay_frames or
        E.DialogueTextDelay.NONE
    descriptor.return_anchor = descriptor.return_anchor or
        E.FieldLocationReturnAnchor.NONE
    descriptor.return_facing = descriptor.return_facing or
        E.FieldLocationReturnFacing.PRESERVE_ENTRY
    descriptor.music_return_policy = descriptor.music_return_policy or
        E.FieldLocationMusicReturnPolicy.NONE
    assert(valid_member(descriptor.scene_outcome, E.FieldLocationSceneOutcome),
        "dialogue descriptor has an invalid field scene outcome")
    assert(valid_member(descriptor.exit_behavior, E.DialogueExitBehavior),
        "dialogue descriptor has an invalid exit behavior")
    assert(valid_member(descriptor.transition, E.DialogueTransition),
        "dialogue descriptor has an invalid transition")
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
    assert(valid_member(descriptor.text_delay_frames, E.DialogueTextDelay),
        "dialogue descriptor has an invalid first-text delay")
    descriptor.source = source
    descriptor.route_id = route_id
    return descriptor
end

function DialogueFramework.open(source, module_name, function_name,
                                scene_outcome, exit_behavior, transition,
                                return_anchor, return_facing,
                                music_return_policy, input_lock,
                                text_delay_frames)
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
        return_anchor = return_anchor,
        return_facing = return_facing,
        music_return_policy = music_return_policy,
        input_lock = input_lock,
        text_delay_frames = text_delay_frames,
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
    local ok, result = pcall(handler)
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
