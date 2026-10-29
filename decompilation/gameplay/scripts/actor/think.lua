-- actor/think.lua: source-order dispatch for the 0x13904 behavior timer.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local EntityMotion = require("entity_motion")
local ActorWaypointRoutes = require("data.actor_patrol_routes")
local u8, u16, s8, s16 = I.u8, I.u16, I.s8, I.s16
local AiTuning = E.AiTuning
local ActionKind = R.ActionKind

local function controlled_runner(context)
    local actor_id = context.controlled_party_member_id
    if type(actor_id) ~= "string" or actor_id == "" then
        return nil
    end
    for _, candidate in ipairs(context.ai_target_candidates or {}) do
        if candidate.id == actor_id and
           candidate.world_activity_actor ~= true then
            return candidate
        end
    end
    return nil
end

local function request_world_activity(state)
    state.world_activity_requested = true
end

local function thon_period_path(state, context)
    -- ROM 0x13A4C gates on +0x16 before decrementing shared FFE188 [C].
    if context.animation_blocks_behavior == true then
        return "thon_busy"
    end

    local period = context.source_thon_period_counter
    if type(period) ~= "number" then
        -- FFE188 has no explicit clear or initializer in the ROM trace.
        state.ai_think_gap = "thon_global_period_initial_value_unresolved"
        return "thon_unresolved"
    end

    local decremented = u8(period - 1)
    state.source_thon_period_counter = decremented
    if s8(decremented) >= 0 then
        return "thon_idle"
    end

    -- 0x13A66 reloads before 0x14492 picks from FFDFC8 and 0xAE6E aims [C].
    state.source_thon_period_counter = AiTuning.THON_PERIOD
    local target = R.linked_target(context, state)
    if type(target) ~= "table" or type(target.heading) ~= "number" then
        state.ai_think_gap = "thon_target_link_unavailable"
        return "thon_unresolved"
    end
    -- ROM 0xAE6E exits at its FFE186 == 3 gate before calculating the link
    -- probe or calling 0x1033E. The caller still installs the C082 pose below.
    local line_clear = false
    if context.combat_meter_service_blocked ~= true then
        if type(target.world_x) ~= "number" or
           type(target.world_y) ~= "number" or
           type(context.action_line_clear) ~= "function" then
            state.ai_think_gap = "thon_line_of_sight_unavailable"
            return "thon_unresolved"
        end
        line_clear = context.action_line_clear(
            target.world_x, target.world_y)
        if type(line_clear) ~= "boolean" then
            state.ai_think_gap = "thon_line_of_sight_unavailable"
            return "thon_unresolved"
        end
    end
    if not R.begin_thon_action(state, target, line_clear) then
        state.ai_think_gap = "thon_directional_pose_unavailable"
        return "thon_unresolved"
    end

    -- ROM 0xAE6E calls 0x1033E after its target and collision-window checks;
    -- 0x13A4C then selects AF7C/AFAE, which reloads +0xDC and installs C082.
    -- Keep the pose/delay even when the line probe blocks the spell, but only
    -- resolve dice and reserve a target effect when the probe passes [C].
    local resolution
    state.source_action_dispatch = "spell"
    state.ai_think_audio_event = nil
    if line_clear then
        local combat = game.modules.combat
        if type(combat) == "table" and
           type(combat.resolve_spell_action) == "function" then
            resolution = combat.resolve_spell_action(context, target)
        end
        if type(resolution) == "table" then
            state.spell_action_resolution = resolution
            state.action_result = resolution.action_result
            if type(resolution.combatant) == "table" then
                state.combatant = resolution.combatant
            end
            if type(resolution.audio_event) == "number" then
                state.ai_think_audio_event = resolution.audio_event
            end
        else
            state.ai_think_gap = "thon_spell_resolution_unavailable"
        end
    end
    R.begin_spell_cast(state, context, target, resolution)
    return "thon_action"
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
       state.actor_facts.roles.spotted or
       not state.actor_facts.lifecycle.group_leader or
       type(leader) ~= "table" or type(context.action_line_clear) ~= "function" then
        return
    end

    if context.action_line_clear(leader.world_x, leader.world_y) ~= true then
        return
    end
    if type(state.movement_current_heading) ~= "number" then
        state.ai_think_gap = "guard_heading_unavailable"
        return
    end
    -- 0x15D3C measures guard-to-leader heading; 0x14D8E stores it in FF13AE
    -- when the leader is ACTIVE, otherwise FF13AE keeps the guard's +0x2E [C].
    local facing_reference = leader.actor_facts.lifecycle.active
        and leader.heading or state.movement_current_heading
    if type(facing_reference) ~= "number" then
        state.ai_think_gap = "leader_heading_unavailable"
        return
    end
    local facing_delta = s16(u16(
        state.movement_current_heading - facing_reference))
    -- 0x13FB4 rejects an absolute signed-word delta at or above 0x4000 [C].
    if math.abs(facing_delta) >= AiTuning.QUARTER_TURN then
        return
    end

    if context.scene_mode == "building" and
       not leader.actor_facts.interaction.unseen then
        -- 0x13FCC queues the security message only for a visible leader [C].
        state.field_event_request = E.EventFn.GUARD_SPOTS_YOU
    end
    -- 0x13FF6 sets +0x9E.b0 after the optional building event [C].
    state.actor_facts.roles.spotted = true
end

local function think_gate(state, context)
    local facts = state.actor_facts
    local world_active = context.world_activity_active == true

    if context.entity_kind == E.SpriteType.THON then
        return thon_period_path(state, context)
    end
    -- 0x1392A -> 0x14508 -> 0x1450E uses the controlled runner's record [C].
    if facts.behavior.escort then
        return "follow_leader"
    end
    if facts.roles.quest then
        return "skip"
    end
    if facts.roles.pedestrian and world_active then
        return context.source_projection_active == true and "flee" or "hold"
    end
    if context.source_projection_active == false and
       facts.roles.despawn_offscreen then
        state.ai_deactivate_requested = true
        return "skip"
    end
    if facts.behavior.fleeing then
        return context.source_projection_active == true and "flee" or "hold"
    end

    if context.source_projection_active == false then
        if not world_active then
            return "patrol"
        end
        if context.world_scene_state ~= E.EndingState.NONE and
           not facts.lifecycle.ai_runner and
           context.hold_for_ending_activity == true then
            return "hold"
        end
        return "follow_leader"
    end

    local raises_world_activity = false
    if context.source_projection_active == true and
       facts.roles.hostile then
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
    if context.animation_blocks_behavior ~= true then
        state.watch_heading = u16(leader.heading)
    end
    return "watch"
end

local function select_think_path(state, context)
    state.world_activity_requested = false
    state.field_event_request = nil
    state.ai_deactivate_requested = false
    state.ai_think_gap = nil
    state.think_token_claimed = false
    state.source_thon_period_counter = nil
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
    -- 0x138B8 claims FFE175 before the 0x13904 think gate [C].
    state.behavior_timer = AiTuning.THINK_FRAMES
    state.think_token_claimed = true
    state.ai_think_path = think_gate(state, context)
    return state.ai_think_path
end

local function set_idle(state, context)
    if context.animation_blocks_behavior == true then
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
        set_idle(state, context)
        return
    end
    if not state.actor_facts.behavior.fleeing then
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
        state.actor_facts.behavior.fleeing = true
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

local function apply_watch(state, context)
    if type(state.watch_heading) ~= "number" then
        return
    end
    state.movement_current_heading = u16(state.watch_heading)
    set_idle(state, context)
end

local function actor_spawn_record(context)
    local actor_id = context.actor_id
    if type(actor_id) ~= "string" or actor_id == "" then
        return nil
    end
    return ActorWaypointRoutes[actor_id]
end

local function set_route_target(state, x, y)
    state.ai_route_target_x = u16(x)
    state.ai_route_target_y = u16(y)
end

local function stop_route(state, context)
    -- 0x1460E clears the current walk continuation when the selected point
    -- is within the source arrival metric [C]. Clear it before asking the
    -- presentation state to become idle; the host may still be playing an
    -- action animation and block that presentation change.
    state.movement_active = false
    state.movement_heading = nil
    state.movement_route_distance = nil
    set_idle(state, context)
end

local function measure_route_target(state, context)
    if type(state.ai_route_target_x) ~= "number" or
       type(state.ai_route_target_y) ~= "number" or
       type(context.measure_destination) ~= "function" then
        state.ai_think_gap = "actor_route_destination_measure_unavailable"
        return nil
    end
    local measured = context.measure_destination(
        state.ai_route_target_x, state.ai_route_target_y)
    if type(measured) ~= "table" or
       type(measured.heading) ~= "number" or
       type(measured.distance) ~= "number" or
       type(measured.step_cost) ~= "number" then
        state.ai_think_gap = "actor_route_destination_measure_unavailable"
        return nil
    end
    return measured
end

local function continue_route(state, measured, context)
    if measured.step_cost < AiTuning.ARRIVE_DIST then
        stop_route(state, context)
        return
    end
    -- 0x145A2 returns before installing a new route while +0x16 is busy [C].
    if context.animation_blocks_behavior == true then
        return
    end
    state.movement_heading = u16(measured.heading)
    state.movement_route_distance = u16(measured.distance)
    state.movement_active = true
end

local function initial_route_target(state, context, route)
    if type(state.ai_route_target_x) == "number" and
       type(state.ai_route_target_y) == "number" then
        return true
    end
    -- 0x44BC copies the first +0xE0 pair into the live actor position and
    -- +0x96/+0x98 [C]. The host forwards that current position each pass.
    if type(context.source_world_x) == "number" and
       type(context.source_world_y) == "number" then
        set_route_target(state, context.source_world_x,
                         context.source_world_y)
        return true
    end
    if type(route) == "table" and type(route.initial) == "table" and
       route.no_patrol ~= true and
       type(route.initial.x) == "number" and
       type(route.initial.y) == "number" then
        set_route_target(state, route.initial.x, route.initial.y)
        return true
    end
    state.ai_think_gap = "actor_route_initial_position_unavailable"
    return false
end

local function route_collision_free(state, x, y)
    if type(game) ~= "table" or type(game.movement) ~= "table" or
       type(game.movement.collision) ~= "function" then
        state.ai_think_gap = "formation_collision_query_unavailable"
        return nil
    end
    local collision_code = game.movement.collision(x, y)
    if type(collision_code) ~= "number" then
        state.ai_think_gap = "formation_collision_result_unavailable"
        return nil
    end
    -- 0x14550/0x1455A accept zero or negative collision results [C].
    return collision_code <= AiTuning.FOLLOW_COLLISION_CLEAR_MAX
end

local function apply_follow_destination(state, context)
    local leader = controlled_runner(context)
    local function known_formation_position(position)
        return position == "primary" or position == "secondary" or
               position == "other"
    end
    if type(leader) ~= "table" or type(leader.world_x) ~= "number" or
       type(leader.world_y) ~= "number" or
       type(leader.source_record_heading) ~= "number" or
       not known_formation_position(leader.formation_position) or
       not known_formation_position(context.formation_position) then
        state.ai_think_gap = "formation_leader_record_unavailable"
        return
    end

    local angle = u16(leader.source_record_heading)
    if context.formation_position == "primary" or
       (context.formation_position == "secondary" and
        leader.formation_position == "primary") then
        angle = u16(angle - AiTuning.FOLLOW_ANGLE_STEP)
    else
        angle = u16(angle + AiTuning.FOLLOW_ANGLE_STEP)
    end
    -- 0x14512 stores 0x20 to d3, then 0x14516 overwrites it with 0xE0;
    -- the effective 0x15DCC radius is E.AiTuning.FOLLOW_RADIUS [C].
    local offset = EntityMotion.offset_at_radius(
        angle, AiTuning.FOLLOW_RADIUS)
    if type(offset) ~= "table" or
       type(offset.horizontal) ~= "number" or
       type(offset.vertical) ~= "number" then
        state.ai_think_gap = "formation_heading_vector_unavailable"
        return
    end

    local target_x = u16(leader.world_x + offset.horizontal)
    local target_y = u16(leader.world_y + offset.vertical)
    local first_clear = route_collision_free(state, target_x, target_y)
    if first_clear == nil then
        return
    end
    if not first_clear then
        target_x = u16(leader.world_x - offset.horizontal)
        target_y = u16(leader.world_y - offset.vertical)
        local second_clear = route_collision_free(state, target_x, target_y)
        if second_clear == nil then
            return
        end
        if not second_clear then
            -- 0x1458A falls back to the leader's live XY after both probes [C].
            target_x = u16(leader.world_x)
            target_y = u16(leader.world_y)
        end
    end

    -- 0x14590 writes +0x96/+0x98 before 0x144E8/0x145A2 [C].
    set_route_target(state, target_x, target_y)
    local measured = measure_route_target(state, context)
    if measured ~= nil then
        continue_route(state, measured, context)
    end
end

local function apply_patrol(state, context)
    -- 0x142AE +0x9E.6 transfers directly to 0x1450E [C].
    if state.actor_facts.roles.hostile then
        apply_follow_destination(state, context)
        return
    end

    local route = actor_spawn_record(context)
    if not initial_route_target(state, context, route) then
        return
    end
    -- 0x142C2/0x144E8 tests the current +0x96/+0x98 point first. A distant
    -- point stays selected and is routed by 0x145A2; only an arrived point
    -- reaches the +0x10.5 waypoint/follow decision [C].
    local measured = measure_route_target(state, context)
    if measured == nil then
        return
    end
    if measured.step_cost >= AiTuning.ARRIVE_DIST then
        continue_route(state, measured, context)
        return
    end
    stop_route(state, context)

    local group_leader = type(route) == "table"
        and route.group_leader == true
        or (route == nil and state.actor_facts.lifecycle.group_leader)
    if not group_leader then
        apply_follow_destination(state, context)
        return
    end
    if route == nil then
        state.ai_think_gap = "patrol_waypoint_list_unavailable"
        return
    end
    if route.no_patrol == true then
        apply_follow_destination(state, context)
        return
    end
    if type(route.points) ~= "table" or
       type(route.initial) ~= "table" then
        state.ai_think_gap = "patrol_waypoint_list_unavailable"
        return
    end

    local cursor = state.ai_waypoint_index or AiTuning.PATROL_INITIAL_CURSOR
    if type(cursor) ~= "number" or cursor % AiTuning.PATROL_WAYPOINT_STRIDE ~= 0 or
       cursor < 0 then
        state.ai_think_gap = "patrol_waypoint_index_invalid"
        return
    end
    local next_cursor = cursor + AiTuning.PATROL_WAYPOINT_STRIDE
    local waypoint = route.points[
        next_cursor / AiTuning.PATROL_WAYPOINT_STRIDE]
    if waypoint == nil then
        -- 0x142F6 sees the zero-X terminator, clears +0xE4, and selects the
        -- spawn pair at +0xE0. The next arrival restarts at the first point [C].
        state.ai_waypoint_index = AiTuning.PATROL_INITIAL_CURSOR
        set_route_target(state, route.initial.x, route.initial.y)
        return
    end
    if type(waypoint.x) ~= "number" or type(waypoint.y) ~= "number" then
        state.ai_think_gap = "patrol_waypoint_data_invalid"
        return
    end
    state.ai_waypoint_index = next_cursor
    set_route_target(state, waypoint.x, waypoint.y)
end

local function preserve_offscreen_action_dispatch(state, context)
    if context.source_projection_active ~= false or
       (context.action_kind ~= ActionKind.spell and
        context.action_kind ~= ActionKind.item) then
        return
    end
    -- The existing 0x13FFE action path selects/retains its linked record
    -- before reporting the offscreen result. Keep that action result while
    -- the newly selected patrol/follow destination owns movement [C].
    local target = R.linked_target(context, state)
    if type(target) == "table" then
        state.ai_target_id = target.id
        state.ai_target_heading = target.heading
        state.ai_target_distance = target.distance
        state.ai_target_engaged = false
    end
    if context.action_kind == ActionKind.spell then
        state.source_action_dispatch =
            R.classify_spell_dispatch(state, context, target)
    else
        state.source_action_dispatch =
            R.classify_item_dispatch(state, context, target)
    end
end

local function apply_think_result(state, context, path)
    if path == "wait" or path == "skip" then
        if state.ai_deactivate_requested then
            state.movement_active = false
        end
        return true
    elseif path == "thon_busy" or path == "thon_unresolved" then
        return true
    elseif path == "thon_idle" then
        -- The 0xB45C idle/stand continuation is represented by set_idle [C].
        set_idle(state, context)
        return true
    elseif path == "thon_action" then
        local audio_event = state.ai_think_audio_event
        state.ai_think_audio_event = nil
        return true, audio_event
    elseif path == "flee" then
        apply_flee(state, context)
        return true
    elseif path == "hold" then
        state.watch_heading = nil
        set_idle(state, context)
        return true
    elseif path == "watch" then
        apply_watch(state, context)
        return true
    elseif path == "patrol" then
        apply_patrol(state, context)
        preserve_offscreen_action_dispatch(state, context)
        return true
    elseif path == "follow_leader" then
        -- 0x139B4 and 0x14508 enter 0x1450E without the 0x142AE arrival test [C].
        apply_follow_destination(state, context)
        preserve_offscreen_action_dispatch(state, context)
        return true
    end
    -- Combat remains on the existing target/action path [C].
    return false
end

local function think(state, context)
    local path = select_think_path(state, context)
    return apply_think_result(state, context, path)
end

R.think_gate = think_gate
R.think = think
return R
