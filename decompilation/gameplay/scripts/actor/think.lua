-- actor/think.lua: source-order dispatch for the 0x13904 behavior timer.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local AiTuning, ActorFlag, ActorStatus, ActorStatusEx, BehaviorFlag, SceneMode =
    E.AiTuning, E.ActorFlag, E.ActorStatus, E.ActorStatusEx,
    E.BehaviorFlag, E.SceneMode

local function has(flags, bit)
    return (flags & bit) ~= 0
end

local function controlled_runner(context)
    local slot = context.controlled_runner_slot
    if type(slot) ~= "number" then
        return nil
    end
    for _, candidate in ipairs(context.ai_target_candidates or {}) do
        if candidate.source_record_slot == slot and
           candidate.world_activity_actor ~= true then
            return candidate
        end
    end
    return nil
end

local function request_world_activity(state)
    state.world_activity_requested = true
end

local function guard_spots_you(state, context, leader)
    if context.world_activity_active == true then
        return
    end
    if context.source_alarm_active == true then
        -- 0x13F80 tail-jumps to 0x1DBAA when FFE16F is already set [C].
        request_world_activity(state)
        return
    end
    if context.source_alarm_active ~= false or
       has(state.source_spawn_state_flags or 0, ActorFlag.SPOTTED) or
       not has(state.source_record_flags or 0, ActorStatus.GROUP_LEADER) or
       type(leader) ~= "table" or type(context.action_line_clear) ~= "function" then
        return
    end

    if context.action_line_clear(leader.world_x, leader.world_y) ~= true then
        return
    end
    -- FF13AE has no recovered producer in the current host. The ROM-facing
    -- reference and the actor's +0x2E heading are injectable facts [H].
    if type(context.guard_facing_reference) ~= "number" or
       type(state.movement_current_heading) ~= "number" then
        state.ai_think_gap = "guard_facing_reference_unresolved"
        return
    end
    local facing_delta = s16(u16(
        state.movement_current_heading - context.guard_facing_reference))
    if math.abs(facing_delta) >= AiTuning.QUARTER_TURN then
        return
    end

    if context.source_scene_mode == SceneMode.BUILDING and
       not has(leader.interaction_flags or 0, ActorStatusEx.UNSEEN) then
        state.field_event_request = E.EventFn.GUARD_SPOTS_YOU
    end
    -- 0x13FF6 sets +0x9E.b0 after the optional building event [C].
    state.source_spawn_state_flags = u8(
        (state.source_spawn_state_flags or 0) | ActorFlag.SPOTTED)
end

local function think_gate(state, context)
    local spawn_flags = context.source_spawn_state_flags or 0
    local behavior_flags = context.source_behavior_flags or 0
    local record_flags = context.source_record_flags or 0
    local world_active = context.world_activity_active == true

    -- 0x13920's kind-0xF transfer is retained in the existing generic actor
    -- action path, but its 0x13A4C global-period behavior remains unresolved [H].
    if context.entity_kind == E.SpriteType.THON then
        state.ai_think_gap = "thon_period_path_unported"
        return "combat"
    end
    -- 0x1392A -> 0x14508 -> 0x1450E uses a source formation slot; the host has
    -- no typed formation record yet, so the existing targeting path remains [H].
    if has(behavior_flags, BehaviorFlag.ESCORT) then
        return "follow_leader"
    end
    if has(spawn_flags, ActorFlag.QUEST) then
        return "skip"
    end
    if has(spawn_flags, ActorFlag.PEDESTRIAN) and world_active then
        return context.source_projection_active == true and "flee" or "hold"
    end
    if context.source_projection_active == false and
       has(spawn_flags, ActorFlag.DESPAWN_OFFSCREEN) then
        state.ai_deactivate_requested = true
        return "skip"
    end
    if has(behavior_flags, BehaviorFlag.FLEEING) then
        return context.source_projection_active == true and "flee" or "hold"
    end

    if context.source_projection_active == false then
        if not world_active then
            return "patrol"
        end
        if context.world_scene_state ~= E.EndingState.NONE and
           not has(record_flags, ActorStatus.AI_RUNNER) and
           context.source_action_ammo == AiTuning.ENDING_HOLD_ROUNDS then
            return "hold"
        end
        return "follow_leader"
    end

    local raises_world_activity = false
    if context.source_projection_active == true and
       has(spawn_flags, ActorFlag.HOSTILE) then
        -- 0x139BE calls 0x1DBAA before re-reading FFDF84 [C].
        request_world_activity(state)
        raises_world_activity = true
    end
    if world_active or raises_world_activity then
        return "combat"
    end

    local leader = controlled_runner(context)
    guard_spots_you(state, context, leader)
    local posture = context.source_actor_posture
    if type(posture) ~= "number" then
        state.ai_think_gap = "actor_posture_unavailable"
        return "combat"
    end
    if posture <= E.Posture.NEUTRAL then
        return "patrol"
    end
    if posture == E.Posture.FULL_OFFENSE then
        if type(leader) ~= "table" or type(leader.distance) ~= "number" then
            state.ai_think_gap = "controlled_runner_distance_unavailable"
            return "combat"
        end
        if leader.distance < AiTuning.ENGAGE_DIST then
            request_world_activity(state)
            return "combat"
        end
        return "patrol"
    end
    if posture > E.Posture.FULL_OFFENSE then
        return "combat"
    end
    if type(leader) ~= "table" or type(leader.distance) ~= "number" or
       type(leader.heading) ~= "number" then
        state.ai_think_gap = "controlled_runner_watch_facts_unavailable"
        return "combat"
    end
    if leader.distance >= AiTuning.WATCH_DIST then
        return "patrol"
    end
    if (context.source_animation_selector or 0) <= AiTuning.ANIM_BUSY then
        state.watch_heading = u16(leader.heading)
    end
    return "watch"
end

local function advance_think(state, context)
    state.world_activity_requested = false
    state.field_event_request = nil
    state.ai_deactivate_requested = false
    state.ai_think_gap = nil
    if state.behavior_timer > 0 then
        state.behavior_timer = state.behavior_timer - 1
        state.ai_think_path = "wait"
        return "wait"
    end
    if context.source_think_suppressed == true then
        -- The source subq leaves +0xB6 negative and retries next pass [C].
        state.behavior_timer = AiTuning.THINK_RETRY
        state.ai_think_path = "skip"
        return "skip"
    end
    state.behavior_timer = AiTuning.THINK_FRAMES
    state.ai_think_path = think_gate(state, context)
    return state.ai_think_path
end

local function set_idle(state)
    if (state.source_animation_selector or 0) > AiTuning.ANIM_BUSY then
        return
    end
    state.movement_active = false
    state.movement_heading = nil
    state.movement_route_distance = nil
    state.animation_id = "idle"
    state.animation_frame = 0
    state.direction = R.direction_from_heading(
        state.watch_heading or state.movement_current_heading or 0)
end

local function apply_flee(state, context)
    if context.source_projection_active ~= true then
        set_idle(state)
        return
    end
    if not has(state.source_behavior_flags or 0, BehaviorFlag.FLEEING) then
        local camera_x, camera_y = context.source_camera_x,
            context.source_camera_y
        local limit_x, limit_y = context.source_camera_limit_x,
            context.source_camera_limit_y
        if type(camera_x) ~= "number" or type(camera_y) ~= "number" or
           type(limit_x) ~= "number" or type(limit_y) ~= "number" then
            state.ai_think_gap = "flee_camera_bounds_unavailable"
            return
        end
        local anchor_x = u16(camera_x + AiTuning.FLEE_CAMERA_BIAS)
        local anchor_y = u16(camera_y + AiTuning.FLEE_CAMERA_BIAS)
        local offset_x, offset_y = AiTuning.FLEE_OFFSET, AiTuning.FLEE_OFFSET
        if (limit_x >> 1) < anchor_x then offset_x = -offset_x end
        if (limit_y >> 1) < anchor_y then offset_y = -offset_y end
        state.flee_target_x = u16(anchor_x + offset_x)
        state.flee_target_y = u16(anchor_y + offset_y)
        state.source_behavior_flags = u8(
            (state.source_behavior_flags or 0) | BehaviorFlag.FLEEING)
    end
    if type(state.flee_target_x) ~= "number" or
       type(state.flee_target_y) ~= "number" or
       type(context.measure_destination) ~= "function" then
        state.ai_think_gap = "flee_destination_measure_unavailable"
        return
    end
    local measured = context.measure_destination(
        state.flee_target_x, state.flee_target_y)
    if type(measured) ~= "table" or
       type(measured.heading) ~= "number" or
       type(measured.distance) ~= "number" or
       type(measured.step_cost) ~= "number" then
        state.ai_think_gap = "flee_destination_measure_unavailable"
        return
    end
    if measured.step_cost < AiTuning.ARRIVE_DIST then
        state.movement_active = false
        state.movement_heading = nil
        state.movement_route_distance = nil
        return
    end
    state.movement_heading = u16(measured.heading)
    state.movement_route_distance = u16(measured.distance)
    state.movement_active = true
end

local function apply_watch(state)
    if type(state.watch_heading) ~= "number" then
        return
    end
    state.movement_current_heading = u16(state.watch_heading)
    set_idle(state)
end

local function apply_path(state, context, path)
    if path == "wait" or path == "skip" then
        if state.ai_deactivate_requested then
            state.movement_active = false
        end
        return true
    elseif path == "flee" then
        apply_flee(state, context)
        return true
    elseif path == "hold" then
        state.watch_heading = nil
        set_idle(state)
        return true
    elseif path == "watch" then
        apply_watch(state)
        return true
    end
    -- Patrol (0x142AE) and formation follow (0x1450E) are named at the gate,
    -- but their waypoint/slot data is absent. Keep the preexisting AI path [H].
    return false
end

R.think_gate = think_gate
R.advance_think = advance_think
R.apply_think_path = apply_path
return R
