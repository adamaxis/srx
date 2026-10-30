-- Map event placement, state gates, actions, and repeat policy are authored in
-- gameplay/world/areas/scene-NN-triggers.tsv and loaded into
-- world_content_data.map_events. This module supplies the handler functions.
-- Event flow is ordinary sequential Lua and movement is a narrow host service.
local Events = {}
local MovementDirection = require("enums").MovementDirection

local action = {
    project_world_interaction = "project_world_interaction",
    skip_world_interaction = "skip_world_interaction",
    begin_world_activity = "begin_world_activity",
    mark_projected_terrain = "mark_projected_terrain",
    begin_maglock_interaction = "begin_maglock_interaction",
    begin_security_terminal = "begin_security_terminal",
    begin_special_activity = "begin_special_activity",
    begin_field_mode_exit = "begin_field_mode_exit",
}

local collision_kind = {
    empty_world_interaction = true,
    world_interaction = true,
    lockable_door = true,
    security_terminal = true,
    contextual_activity = true,
    terrain_marker = true,
    world_activity = true,
    activity_transition = true,
    profiled_contact = true,
    field_activity = true,
}

local function decision(next_action, collision_rewrite)
    return {
        action = next_action,
        collision_rewrite = collision_rewrite or "none",
        publishes_world_interaction = false,
        projected_terrain_event_published = false,
    }
end

function Events.resolve_collision(context)
    if type(context) ~= "table" or
       context.event_code ~= nil or context.raw_collision_code ~= nil or
       context.scene_id ~= nil or
       type(context.collision_kind) ~= "string" or
       type(context.correctional_facility) ~= "boolean" or
       type(context.resource_site) ~= "boolean" or
       type(context.special_activity_context) ~= "boolean" or
       type(context.field_mode_exit_requested) ~= "boolean" or
       type(context.interaction_gate_active) ~= "boolean" or
       type(context.activity_sequence_active) ~= "boolean" or
       type(context.terrain_event_suppressed) ~= "boolean" or
       type(context.terrain_marker_publish_allowed) ~= "boolean" or
       (context.maglocks_disabled ~= nil and
        type(context.maglocks_disabled) ~= "boolean") then
        return nil
    end

    local kind = context.collision_kind
    if not collision_kind[kind] then
        return nil
    end
    if kind == "field_activity" or kind == "profiled_contact" or
       kind == "world_activity" then
        return decision(action.begin_world_activity)
    elseif kind == "activity_transition" then
        if context.activity_sequence_active then
            return decision(action.skip_world_interaction)
        end
        return decision(action.begin_world_activity)
    elseif kind == "terrain_marker" then
        local result = decision(action.mark_projected_terrain)
        result.projected_terrain_event_published =
            context.terrain_marker_publish_allowed
        return result
    end

    -- The native boundary only labels a collision a lockable door after its
    -- source scene/mode context selects the shared lock prompt.
    if kind == "lockable_door" and
       not context.terrain_event_suppressed and
       context.maglocks_disabled ~= true then
        return decision(action.begin_maglock_interaction)
    end

    if kind == "security_terminal" then
        return decision(action.begin_security_terminal)
    end

    local result = decision(action.project_world_interaction)
    if kind == "contextual_activity" then
        if context.special_activity_context then
            result.action = action.begin_special_activity
            return result
        end
        result.collision_rewrite = "ordinary_interaction"
    end

    if context.field_mode_exit_requested then
        result.action = action.begin_field_mode_exit
        return result
    end

    result.publishes_world_interaction = true
    if context.resource_site and not context.correctional_facility and
       kind == "empty_world_interaction" and
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
