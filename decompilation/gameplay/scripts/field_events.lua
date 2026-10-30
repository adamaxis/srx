-- Map event placement, state gates, actions, and repeat policy are authored in
-- gameplay/world/areas/scene-NN-triggers.tsv and loaded into
-- world_content_data.map_events. This module supplies the handler functions.
-- Event flow is ordinary sequential Lua and movement is a narrow host service.
local Events = {}
local MovementDirection = require("enums").MovementDirection

local event_code = {}
for value = 0, 31 do
    event_code[value] = "event_" .. value
end

local action = {
    project_world_interaction = "project_world_interaction",
    skip_world_interaction = "skip_world_interaction",
    begin_event_31_activity = "begin_event_31_activity",
    begin_event_30_activity = "begin_event_30_activity",
    begin_event_29_activity = "begin_event_29_activity",
    begin_event_28_activity = "begin_event_28_activity",
    begin_event_27_activity = "begin_event_27_activity",
    mark_projected_terrain = "mark_projected_terrain",
    begin_maglock_interaction = "begin_maglock_interaction",
    begin_security_terminal = "begin_security_terminal",
    begin_event_25_activity = "begin_event_25_activity",
    begin_field_mode_exit = "begin_field_mode_exit",
}

local function decision(context, next_action, next_event_code)
    return {
        action = next_action,
        event_code = next_event_code or context.event_code,
        collision_rewrite = "none",
        mode_event_code_published = false,
        projected_terrain_event_published = false,
    }
end

function Events.resolve_collision(context)
    if type(context) ~= "table" or
       type(context.event_code) ~= "string" or
       type(context.scene_id) ~= "number" or
       type(context.interaction_mode) ~= "boolean" or
       type(context.scene_20) ~= "boolean" or
       type(context.transition_context_25) ~= "boolean" or
       type(context.field_mode_exit_requested) ~= "boolean" or
       type(context.interaction_gate_active) ~= "boolean" or
       type(context.activity_sequence_active) ~= "boolean" or
       type(context.terrain_event_suppressed) ~= "boolean" or
       type(context.terrain_marker_publish_allowed) ~= "boolean" or
       (context.maglocks_disabled ~= nil and
        type(context.maglocks_disabled) ~= "boolean") then
        return nil
    end

    local code = context.event_code
    if code == event_code[31] then
        return decision(context, action.begin_event_31_activity)
    elseif code == event_code[30] then
        return decision(context, action.begin_event_30_activity)
    elseif code == event_code[29] then
        if context.activity_sequence_active then
            return decision(context, action.skip_world_interaction)
        end
        return decision(context, action.begin_event_29_activity)
    elseif code == event_code[28] then
        if context.activity_sequence_active then
            return decision(context, action.skip_world_interaction)
        end
        return decision(context, action.begin_event_28_activity)
    elseif code == event_code[27] then
        return decision(context, action.begin_event_27_activity)
    elseif code == event_code[26] then
        local result = decision(context, action.mark_projected_terrain)
        result.projected_terrain_event_published =
            context.terrain_marker_publish_allowed
        return result
    end

    local is_maglock_selection =
        code == event_code[2] or code == event_code[3] or
        code == event_code[4] or code == event_code[5]
    -- ROM 0xB7B8 selects the shared 0x20E48 lock prompt for events 2..5
    -- only while FFE17F is clear. When maglocks are disabled, fall through
    -- to the ordinary world-interaction path below.
    if (context.scene_20 or context.interaction_mode) and
       is_maglock_selection and not context.terrain_event_suppressed and
       context.maglocks_disabled ~= true then
        return decision(context, action.begin_maglock_interaction)
    end

    if code == event_code[24] then
        return decision(context, action.begin_security_terminal)
    end

    local result = decision(context, action.project_world_interaction)
    if code == event_code[25] then
        if context.transition_context_25 then
            result.action = action.begin_event_25_activity
            return result
        end
        result.collision_rewrite = "event_25_generic_world_interaction"
        result.event_code = event_code[8]
    end

    if context.field_mode_exit_requested then
        result.action = action.begin_field_mode_exit
        return result
    end

    result.mode_event_code_published = true
    if context.interaction_mode and not context.scene_20 and
       result.event_code == event_code[0] and
       (context.interaction_gate_active or
        context.activity_sequence_active) then
        result.action = action.skip_world_interaction
        return result
    end

    return result
end

function Events.new_game_opening_walk()
    return game.actor.walk_direction("player", MovementDirection.NORTH)
end

-- Location content is selected in Lua after the native field dispatcher has
-- supplied a typed context. Shared presentation stays generic; the active
-- location's speaker and dialogue profile own the text and choices.
function Events.present_location_dialogue()
    local location = game.location
    if type(location.type) ~= "string" or location.type ~= "building" or
       type(location.id) ~= "string" or
       type(location.speaker_id) ~= "string" or
       type(location.text_profile) ~= "string" or
       type(location.event_id) ~= "string" then
        return "skip"
    end
    game.ui.say(location.event_id)
    return "shown"
end

return Events
