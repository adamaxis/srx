-- Lua owns per-actor behavior and presentation state. The host calls one
-- heartbeat for each active actor on each field behavior-service pass, then
-- renders the returned animation snapshot without advancing it a second time.
local ActorAI = {}
local states = game.actors

local profiles = {
    ["corp_guard_k05_item06"] = {
        package_id = "corp_security.max_power_heavy_pistol",
        entity_kind = 5,
        action_kind = "item",
        action_code = 6,
        weapon_item_id = 6,
        entry_audio_event = 0x2e,
    },
    ["pedestrian_k0b_none00"] = {
        package_id = "pedestrian.no_active_action",
        entity_kind = 11,
        action_kind = "none",
        action_code = 0,
        entry_audio_event = 0x2f,
    },
    ["field_runner_k03_none00"] = {
        package_id = "runner.field_controlled",
        entity_kind = 3,
        action_kind = "none",
        action_code = 0,
        controls_movement = true,
        entry_animation = false,
    },
}

local function direction_from_heading(heading)
    -- ROM 0xB36A rotates the heading three bits, rounds signed values upward,
    -- then masks to one of the eight C0B2 directional animation lists.
    local value = ((heading << 3) | (heading >> 13)) & 0xffff
    if (value & 0x8000) ~= 0 then
        value = (value + 1) & 0xffff
    end
    return value & 0x7
end

local function source_profile(context)
    local profile = profiles[context.profile_id]
    if profile == nil or
       context.entity_kind ~= profile.entity_kind or
       context.action_kind ~= profile.action_kind or
       context.action_code ~= profile.action_code then
        return nil
    end
    return profile
end

local function initial_state(context, profile)
    return {
        actor_id = context.actor_id,
        profile_id = context.profile_id,
        entity_kind = context.entity_kind,
        action_kind = context.action_kind,
        action_code = context.action_code,
        package_id = profile.package_id,
        weapon_item_id = profile.weapon_item_id,
        direction = direction_from_heading(context.initial_heading),
        entry_delay = context.entry_delay,
        behavior_timer = 0,
        animation_id = "idle",
        animation_frame = 0,
        animation_accumulator = 0,
        transition_started = false,
        transition_frame_list_finished = false,
        transition_complete = false,
    }
end

local function same_actor(state, context)
    return state.profile_id == context.profile_id and
           state.entity_kind == context.entity_kind and
           state.action_kind == context.action_kind and
           state.action_code == context.action_code
end

local function snapshot(state, action, audio_event)
    return {
        actor_id = state.actor_id,
        profile_id = state.profile_id,
        package_id = state.package_id,
        action_kind = state.action_kind,
        action_code = state.action_code,
        action = action,
        animation = state.animation_id,
        direction = state.direction,
        frame = state.animation_frame,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        audio_event = audio_event,
        weapon_item_id = state.weapon_item_id,
        movement_heading = state.movement_heading,
    }
end

local function requested_movement_heading(context, profile)
    if not profile.controls_movement or not context.movement_allowed then
        return nil
    end
    if context.movement_heading_override ~= nil then
        return context.movement_heading_override
    end
    if type(context.controller_nibble) ~= "number" or
       context.controller_nibble < 0 or context.controller_nibble > 15 or
       type(context.controller_heading_table) ~= "table" then
        return nil
    end
    local heading = context.controller_heading_table[
        context.controller_nibble + 1]
    if type(heading) ~= "number" or heading < 0 or heading >= 0xffff then
        return nil
    end
    return heading
end

function ActorAI.heartbeat(context)
    if type(context) ~= "table" then
        return nil
    end
    local profile = source_profile(context)
    if profile == nil or type(context.actor_id) ~= "string" or
       context.actor_id == "" or type(context.initial_heading) ~= "number" or
       type(context.entry_delay) ~= "number" or context.entry_delay < 0 or
       context.entry_delay > 15 then
        return nil
    end

    local state = states[context.actor_id]
    if context.spawn or state == nil then
        state = initial_state(context, profile)
        states[context.actor_id] = state
        state.movement_heading = requested_movement_heading(context, profile)
        return snapshot(state, "hold", nil)
    end
    if not same_actor(state, context) then
        return nil
    end

    local action = "hold"
    local audio_event
    local just_started = false
    state.movement_heading = requested_movement_heading(context, profile)

    if profile.entry_animation == false then
        return snapshot(state, action, nil)
    end

    -- ROM 0x13904 services the actor's signed initial delay before B330.
    if not state.transition_started then
        if state.entry_delay == 0 then
            state.transition_started = true
            state.animation_id = "entry"
            state.animation_frame = 0
            state.animation_accumulator = -16
            state.behavior_timer = 20
            action = "play_entry"
            audio_event = profile.entry_audio_event
            just_started = true
        else
            state.entry_delay = state.entry_delay - 1
        end
    elseif not state.transition_complete then
        if state.behavior_timer == 0 then
            state.behavior_timer = 20
            if state.transition_frame_list_finished then
                state.transition_complete = true
            end
        else
            state.behavior_timer = state.behavior_timer - 1
        end
    end

    -- The two-frame C0B2 list advances at +0x0200 against +0x1000. Lua
    -- carries that cadence; rendering consumes frame without its own clock.
    if state.animation_id == "entry" and not just_started and
       not state.transition_frame_list_finished then
        local accumulated = state.animation_accumulator + 0x0200
        if accumulated >= 0x1000 then
            state.animation_accumulator = accumulated - 0x1000
            if state.animation_frame == 0 then
                state.animation_frame = 1
            else
                state.animation_frame = 1
                state.transition_frame_list_finished = true
            end
        else
            state.animation_accumulator = accumulated
        end
    end

    return snapshot(state, action, audio_event)
end

return ActorAI
