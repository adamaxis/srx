-- Lua owns per-actor behavior and presentation state. The host calls a
-- behavior heartbeat during 0xA8EA and a render heartbeat during 0x4618;
-- drawing consumes each returned snapshot without advancing it again.
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

local function approach_heading(current, target)
    -- ROM 0xB530 moves a 16-bit fixed-point heading in 0x0800-unit steps.
    local delta = (current - target) & 0xffff
    if delta == 0 then
        return current
    end
    local signed_delta = delta
    if (delta & 0x8000) ~= 0 then
        signed_delta = delta - 0x10000
    end
    if signed_delta < 0 then
        if delta < 0xf800 then
            return (current + 0x0800) & 0xffff
        end
    elseif delta > 0x07ff then
        return (current - 0x0800) & 0xffff
    end
    return target
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
        movement_current_heading = profile.controls_movement
            and context.initial_heading or nil,
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
        animation_accumulator = state.animation_accumulator,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        audio_event = audio_event,
        weapon_item_id = state.weapon_item_id,
        movement_heading = state.movement_heading,
        movement_current_heading = state.movement_current_heading,
    }
end

local function advance_entry_animation(state)
    if state.animation_id ~= "entry" or
       state.transition_frame_list_finished then
        return
    end
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

local function advance_player_animation(state, context)
    if type(context.animation_accumulator_override) == "number" then
        state.animation_accumulator =
            context.animation_accumulator_override
    end
    if type(context.animation_direction_override) == "number" then
        state.direction = context.animation_direction_override & 0x7
    else
        state.direction = direction_from_heading(context.initial_heading)
    end

    if context.animation_mode == "attack" then
        state.animation_id = "attack"
        state.animation_frame = 0
        state.animation_accumulator = 0
        return
    end

    local accumulated = state.animation_accumulator + 0x0280
    local frame_advanced = accumulated >= 0x1000
    if frame_advanced then
        accumulated = accumulated - 0x1000
    end
    state.animation_accumulator = accumulated
    if context.animation_mode == "walking" then
        state.animation_id = "walk"
        if frame_advanced then
            state.animation_frame = (state.animation_frame + 1) % 4
        end
    else
        state.animation_id = "idle"
        state.animation_frame = 0
    end
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
       context.entry_delay > 15 or
       (context.phase ~= "behavior" and context.phase ~= "render") then
        return nil
    end

    local state = states[context.actor_id]
    if context.spawn or state == nil then
        state = initial_state(context, profile)
        states[context.actor_id] = state
        if profile.controls_movement then
            if context.phase == "behavior" then
                state.movement_heading =
                    requested_movement_heading(context, profile)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                end
            else
                advance_player_animation(state, context)
            end
        end
        return snapshot(state, "hold", nil)
    end
    if not same_actor(state, context) then
        return nil
    end

    local action = "hold"
    local audio_event
    if profile.controls_movement then
        if context.phase == "render" then
            state.movement_heading = nil
            advance_player_animation(state, context)
        else
            if type(context.movement_state_heading_override) == "number" then
                state.movement_current_heading =
                    context.movement_state_heading_override & 0xffff
            end
            state.movement_heading =
                requested_movement_heading(context, profile)
            if state.movement_heading ~= nil then
                state.movement_current_heading = approach_heading(
                    state.movement_current_heading, state.movement_heading)
            end
        end
        return snapshot(state, action, nil)
    end

    if context.phase == "render" then
        advance_entry_animation(state)
        return snapshot(state, action, nil)
    end
    state.movement_heading = nil

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

    return snapshot(state, action, audio_event)
end

return ActorAI
