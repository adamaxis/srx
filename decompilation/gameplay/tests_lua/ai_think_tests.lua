-- Source-order checks for the 0x13904 actor think dispatcher.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
local ActorAI = require("actor.init")
local R = require("actor.registry")
local E = require("enums")
local pass, total = 0, 0

local function check(condition, message)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL: " .. message) end
end

local function context(overrides)
    local c = {
        entity_kind = E.SpriteType.CORP_GUARD,
        source_record_flags = E.ActorStatus.ACTIVE,
        source_spawn_state_flags = 0,
        source_behavior_flags = 0,
        source_projection_active = true,
        source_think_suppressed = false,
        source_alarm_active = false,
        source_scene_mode = E.SceneMode.STREET,
        world_activity_active = false,
        world_scene_state = E.EndingState.NONE,
        source_actor_posture = E.Posture.NEUTRAL,
        source_animation_selector = 0,
        source_action_ammo = 0,
        controlled_runner_slot = 0,
        ai_target_candidates = {
            { id = "party.runner.0", source_record_slot = 0,
              world_activity_actor = false, world_x = 0x2000,
              world_y = 0x2400, heading = 0x1000, distance = 0x300,
              projected_y_nonzero = true,
              service_flags = E.ActorStatus.ACTIVE,
              interaction_flags = 0 },
        },
    }
    for key, value in pairs(overrides or {}) do c[key] = value end
    return c
end

local function state(overrides)
    local s = {
        behavior_timer = 0,
        source_record_flags = E.ActorStatus.ACTIVE,
        source_spawn_state_flags = 0,
        source_behavior_flags = 0,
        source_animation_selector = 0,
        movement_current_heading = 0x1000,
        movement_active = false,
    }
    for key, value in pairs(overrides or {}) do s[key] = value end
    return s
end

local function think_path(actor, facts)
    R.think(actor, facts)
    return actor.ai_think_path
end

local world_due, runner_due = R.should_rebuild_target_lists(
    E.AiTuning.WORLD_TARGET_LIST_PHASE, 0)
check(world_due and not runner_due,
      "0x13874 rebuilds the world list on the 0x0B frame phase")
world_due, runner_due = R.should_rebuild_target_lists(
    E.AiTuning.RUNNER_TARGET_LIST_PHASE, 0)
check(not world_due and runner_due,
      "0x13874 rebuilds the runner list on the 0x0F frame phase")
world_due, runner_due = R.should_rebuild_target_lists(
    E.AiTuning.WORLD_TARGET_LIST_PHASE + E.AiTuning.TARGET_LIST_PHASE_MASK + 1,
    0)
check(world_due and not runner_due,
      "0x13874 phase mask repeats every 64 field frames")
world_due, runner_due = R.should_rebuild_target_lists(
    0, 1)
check(world_due and runner_due,
      "FFDFE8 nonzero rebuilds both target lists")
check(R.should_rebuild_target_lists(-1, false) == nil and
      R.should_rebuild_target_lists(0, -1) == nil,
      "target-list rebuild facts reject invalid typed inputs")

local s, c = state(), context({ source_actor_posture = E.Posture.NEUTRAL })
local due_count = 0
local first_path
for pass_index = 1, E.AiTuning.THINK_FRAMES + 1 do
    local path = think_path(s, c)
    if path ~= "wait" then
        due_count = due_count + 1
        first_path = path
    end
end
check(due_count == 1 and first_path == "patrol",
      "0x13904 runs the first thought, then counts 20 intervening passes")
local second_path = think_path(s, c)
check(second_path == "patrol" and
      s.behavior_timer == E.AiTuning.THINK_FRAMES,
      "0x1391A reloads the named think period after a due pass")

s, c = state(), context({ source_think_suppressed = true })
check(think_path(s, c) == "skip" and
      s.behavior_timer == E.AiTuning.THINK_RETRY,
      "0x1390E suppression keeps the timer negative for next-pass retry")
c.source_think_suppressed = false
check(think_path(s, c) == "patrol" and
      s.behavior_timer == E.AiTuning.THINK_FRAMES,
      "FFE175 clear retries immediately and refreshes +0xB6")

local first_actor, second_actor = state(), state()
local token_context = context({ source_think_suppressed = false })
local first_actor_path = think_path(first_actor, token_context)
token_context.source_think_suppressed = first_actor.think_token_claimed
local second_actor_path = think_path(second_actor, token_context)
check(first_actor_path == "patrol" and first_actor.think_token_claimed and
      second_actor_path == "skip" and second_actor.behavior_timer ==
          E.AiTuning.THINK_RETRY and not second_actor.think_token_claimed,
      "FFE175 lets the first due slot claim the frame and leaves the second at -1")
token_context.source_think_suppressed = false
check(think_path(second_actor, token_context) == "patrol" and
      second_actor.think_token_claimed,
      "the suppressed actor retries and claims the token on the next field frame")

local thon_state = state()
local thon_context = context({
    entity_kind = E.SpriteType.THON,
    source_thon_period_counter = E.AiTuning.THON_PERIOD,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, projected_y_nonzero = true,
          service_flags = E.ActorStatus.ACTIVE, interaction_flags = 0,
          world_x = 0x2000, world_y = 0x2400,
          heading = 0x1000, distance = 0x300 },
    },
    ai_random_bound = function() return 0 end,
    action_line_clear = function() return true end,
})
local thon_skips = 0
for _ = 1, E.AiTuning.THON_PERIOD do
    local path = R.think_gate(thon_state, thon_context)
    if path == "thon_idle" then
        thon_skips = thon_skips + 1
    end
    thon_context.source_thon_period_counter =
        thon_state.source_thon_period_counter
end
local thon_fire_path = R.think_gate(thon_state, thon_context)
check(thon_skips == E.AiTuning.THON_PERIOD and
      thon_fire_path == "thon_action" and
      thon_state.source_thon_period_counter == E.AiTuning.THON_PERIOD and
      thon_state.ai_target_id == "party.runner.0" and
      thon_state.movement_current_heading == 0x1000 and
      thon_state.source_animation_selector > E.AiTuning.ANIM_BUSY,
      "THON decrements the shared period eight times, then aims and selects C082")

local offscreen_runner = {
    id = "party.runner.1",
    world_activity_actor = false,
    projected_y_nonzero = false,
    service_flags = E.ActorStatus.ACTIVE,
}
local visible_runner = {
    id = "party.runner.0",
    world_activity_actor = false,
    projected_y_nonzero = true,
    service_flags = E.ActorStatus.ACTIVE,
}
local runner_pool_context = context({
    ai_target_candidates = { offscreen_runner, visible_runner },
    ai_random_bound = function() return 0 end,
})
local retained_runner_state = state({ linked_target_id = offscreen_runner.id })
local selected_runner = R.linked_target(runner_pool_context,
                                        retained_runner_state)
check(selected_runner == visible_runner and
      retained_runner_state.linked_target_id == visible_runner.id,
      "0x14446 drops an off-screen retained runner and selects the visible +0x0C record [C]")
local only_offscreen_context = context({
    ai_target_candidates = { offscreen_runner },
    ai_random_bound = function() return 0 end,
})
local only_offscreen_state = state({ linked_target_id = offscreen_runner.id })
check(R.linked_target(only_offscreen_context, only_offscreen_state) == nil and
      only_offscreen_state.linked_target_id == nil,
      "0x14446 does not select or retain an off-screen runner [C]")

local strike_context = context({
    entity_kind = E.SpriteType.GHOUL,
    dynamic_world_encounter_actor = true,
    movement_allowed = true,
    initial_heading = 0,
})
local strike_state = state({
    entity_kind = E.SpriteType.GHOUL,
    action_delay = -1,
    movement_current_heading = 0,
})
local strike_target = {
    id = "party.runner.0",
    heading = E.AiTuning.QUARTER_TURN,
    distance = E.AiTuning.CLOSE_STRIKE_RANGE - 1,
}
local strike_in_range = R.resolve_source_target_motion(
    strike_context, strike_state, strike_target)
local strike_audio, strike_heading_audio = R.begin_dynamic_target_action(
    strike_state, strike_context, strike_target)
local strike_result = R.snapshot(strike_state, "hold", strike_audio, nil,
                                 strike_heading_audio)
check(strike_in_range and strike_result.world_activity_requested == true and
      strike_state.world_activity_requested == false,
      "0xB25E strike returns one typed request for 0x1DBAA in range [C]")

local thon_unknown_state = state()
check(R.think_gate(thon_unknown_state,
      context({ entity_kind = E.SpriteType.THON })) == "thon_unresolved" and
      thon_unknown_state.ai_think_gap ==
          "thon_global_period_initial_value_unresolved",
      "THON preserves the missing FFE188 initializer as an explicit gap")

local thon_busy_state = state({
    source_animation_selector = E.AiTuning.ANIM_BUSY + 1,
})
local thon_los_calls = 0
thon_context.source_animation_selector = E.AiTuning.ANIM_BUSY + 1
thon_context.source_thon_period_counter = E.AiTuning.THON_PERIOD
thon_context.action_line_clear = function()
    thon_los_calls = thon_los_calls + 1
    return true
end
check(R.think_gate(thon_busy_state, thon_context) == "thon_busy" and
      thon_busy_state.source_thon_period_counter == nil and
      thon_los_calls == 0,
      "THON with +0x16 above 0x3C leaves the period and action path untouched")

local escort_path = R.think_gate(state({
    source_behavior_flags = E.BehaviorFlag.ESCORT,
}), context({ source_behavior_flags = E.BehaviorFlag.ESCORT }))
check(escort_path == "follow_leader",
      "0x1392A escort branch selects 0x14508 follow")

check(R.think_gate(state({ source_spawn_state_flags = E.ActorFlag.QUEST }),
    context({ source_spawn_state_flags = E.ActorFlag.QUEST })) == "skip",
    "0x1393C +0x9E.b3 returns")

local pedestrian = E.ActorFlag.PEDESTRIAN
check(R.think_gate(state({ source_spawn_state_flags = pedestrian }),
    context({ source_spawn_state_flags = pedestrian,
              world_activity_active = true })) == "flee",
    "0x1394A pedestrian with FFDF84 active enters the flee route")

s = state({ source_spawn_state_flags = E.ActorFlag.DESPAWN_OFFSCREEN })
local despawn_path = R.think_gate(s, context({
    source_spawn_state_flags = E.ActorFlag.DESPAWN_OFFSCREEN,
    source_projection_active = false,
}))
check(despawn_path == "skip" and s.ai_deactivate_requested,
      "0x13962 offscreen +0x9E.b2 calls 0x1E514")

check(R.think_gate(state({ source_behavior_flags = E.BehaviorFlag.FLEEING }),
    context({ source_behavior_flags = E.BehaviorFlag.FLEEING })) == "flee",
    "0x1397A +0xB0.b6 returns to the stored flee route")

check(R.think_gate(state(), context({ source_projection_active = false })) ==
      "patrol", "0x13984 offscreen with FFDF84 clear selects patrol")
check(R.think_gate(state(), context({ source_projection_active = false,
      world_activity_active = true })) == "follow_leader",
      "0x139B4 active offscreen actor follows FFFC04")
check(R.think_gate(state(), context({ source_projection_active = false,
      world_activity_active = true,
      world_scene_state = E.EndingState.FINAL_FIGHT,
      source_action_ammo = E.AiTuning.ENDING_HOLD_ROUNDS })) == "hold",
      "0x13996 ending-state offscreen actor with +0x4E gate enters hold")
check(R.think_gate(state(), context({ source_projection_active = false,
      world_activity_active = true,
      world_scene_state = E.EndingState.FINAL_FIGHT,
      source_action_ammo = E.AiTuning.ENDING_HOLD_ROUNDS,
      source_record_flags = E.ActorStatus.ACTIVE | E.ActorStatus.AI_RUNNER })) ==
      "follow_leader",
      "0x139A0 +0x10.b3 bypasses the ending-state hold branch")

s = state({ source_spawn_state_flags = E.ActorFlag.HOSTILE })
local hostile_path = R.think_gate(s, context({
    source_spawn_state_flags = E.ActorFlag.HOSTILE,
}))
check(hostile_path == "combat" and s.world_activity_requested,
      "0x139BE hostile on-screen branch requests 0x1DBAA then combat")
check(R.think_gate(state(), context({ world_activity_active = true })) ==
      "combat", "0x139CE active FFDF84 takes the existing combat path")

local hold_context = context({ source_projection_active = false,
    world_activity_active = true, world_scene_state = E.EndingState.FINAL_FIGHT,
    source_action_ammo = E.AiTuning.ENDING_HOLD_ROUNDS })
s = state({ source_animation_selector = E.AiTuning.ANIM_BUSY })
check(R.think(s, hold_context) and s.ai_think_path == "hold" and
      s.animation_id == "idle" and not s.movement_active,
      "0x13E40 idle pose sets the held idle continuation")
s = state({ source_animation_selector = E.AiTuning.ANIM_BUSY + 1,
            movement_active = true, animation_id = "action" })
check(R.think(s, hold_context) and s.animation_id == "action" and
      s.movement_active,
      "0x13E40 leaves a busy pose and its current movement alone")

s = state()
c = context({ source_spawn_state_flags = pedestrian,
               world_activity_active = true,
               source_camera_x = 0x1000, source_camera_y = 0x6000,
               source_camera_limit_x = 0x8000,
               source_camera_limit_y = 0x8000 })
local measured_x, measured_y
c.measure_destination = function(x, y)
    measured_x, measured_y = x, y
    return { heading = 0x2000, distance = 0x500, step_cost = 0x300 }
end
local flee_handled = R.think(s, c)
check(flee_handled and s.ai_think_path == "flee" and measured_x == 0x3F80 and
      measured_y == 0x4480 and
      (s.source_behavior_flags & E.BehaviorFlag.FLEEING) ~= 0,
      "0x13E9E stores camera-biased flee coordinates once and 0x145A2 routes")
local stored_x = s.flee_target_x
c.source_camera_x = 0x3000
s.behavior_timer = 0
R.think(s, c)
check(s.flee_target_x == stored_x,
      "0x13E9E does not replace an existing flee destination")

s = state({ source_animation_selector = E.AiTuning.ANIM_BUSY + 1,
            movement_active = true })
check(R.think_gate(s, context({ source_behavior_flags =
      E.BehaviorFlag.FLEEING, source_projection_active = false })) == "hold",
      "0x13E8C offscreen flee route enters 0x13E40")

check(R.think_gate(state(), context({ source_actor_posture = E.Posture.MEDIUM_DEFENSE })) ==
      "patrol", "0x13EF4 posture <= 2 selects 0x142AE")
s = state()
check(R.think_gate(s, context({ source_actor_posture = E.Posture.FULL_OFFENSE,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.ENGAGE_DIST,
          heading = 0x1000, interaction_flags = 0 },
    } })) == "patrol" and not s.world_activity_requested,
    "0x13EF4 posture 4 at ENGAGE_DIST does not engage")
s = state()
check(R.think_gate(s, context({ source_actor_posture = E.Posture.FULL_OFFENSE,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.ENGAGE_DIST - 1,
          heading = 0x1000, interaction_flags = 0 },
    } })) == "combat" and s.world_activity_requested,
    "0x13EF4 posture 4 inside ENGAGE_DIST requests combat")
check(R.think_gate(state(), context({ source_actor_posture =
      E.Posture.FULL_OFFENSE + 1 })) == "combat",
      "0x13F36 posture above 4 selects combat")
check(R.think_gate(state(), context({ source_actor_posture = E.Posture.MEDIUM_OFFENSE,
      ai_target_candidates = {
          { id = "party.runner.0", source_record_slot = 0,
            world_activity_actor = false, distance = E.AiTuning.WATCH_DIST,
            heading = 0x1000, interaction_flags = 0 },
      } })) == "patrol",
      "0x13F52 watch distance at WATCH_DIST returns to patrol")
s = state()
local watch_context = context({
    source_actor_posture = E.Posture.MEDIUM_OFFENSE,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.WATCH_DIST - 1,
          heading = 0x2000, interaction_flags = 0 },
    },
})
check(R.think(s, watch_context) and s.ai_think_path == "watch" and
      s.movement_current_heading == 0x2000,
      "0x13F68 watch branch faces FFFC04")

local function guard_case(overrides, actor_overrides, line_clear)
    local gc = context({
        source_record_flags = E.ActorStatus.ACTIVE | E.ActorStatus.GROUP_LEADER,
        source_actor_posture = E.Posture.NEUTRAL,
        source_alarm_active = false,
        source_scene_mode = E.SceneMode.BUILDING,
        action_line_clear = function() return line_clear ~= false end,
        ai_target_candidates = {
            { id = "party.runner.0", source_record_slot = 0,
              world_activity_actor = false, world_x = 0x2000,
              world_y = 0x2400, distance = 0x300, heading = 0x1000,
              service_flags = E.ActorStatus.ACTIVE,
              interaction_flags = 0 },
        },
    })
    for key, value in pairs(overrides or {}) do gc[key] = value end
    local gs = state({ source_record_flags =
        E.ActorStatus.ACTIVE | E.ActorStatus.GROUP_LEADER,
        movement_current_heading = 0x2000 })
    for key, value in pairs(actor_overrides or {}) do gs[key] = value end
    return gs, gc, R.think_gate(gs, gc)
end

s, c = guard_case()
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) ~= 0 and
      s.field_event_request == E.EventFn.GUARD_SPOTS_YOU,
      "0x13F76 clear line and facing schedules GUARD_SPOTS_YOU in building mode")
s, c = guard_case({ source_scene_mode = E.SceneMode.STREET })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) ~= 0 and
      s.field_event_request == nil,
      "0x13FF6 marks spotted outside building mode without the event")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, heading = 0x1000,
      interaction_flags = E.ActorStatusEx.UNSEEN },
} })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) ~= 0 and
      s.field_event_request == nil,
      "0x13FDE suppresses the event while active runner +0x11.b7 is set")
s, c = guard_case({}, {}, false)
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) == 0,
      "0x13FAC blocked line check does not mark the actor spotted")
s, c = guard_case({ source_alarm_active = true })
check(s.world_activity_requested and
      (s.source_spawn_state_flags & E.ActorFlag.SPOTTED) == 0,
      "0x13F80 FFE16F branch requests 0x1DBAA before spotting")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, heading = 0x7000,
      service_flags = 0, interaction_flags = 0 },
} })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) ~= 0 and
      s.ai_think_gap == nil,
      "0x14D8E keeps the guard heading in FF13AE when leader ACTIVE is clear")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, heading = 0x1000,
      service_flags = E.ActorStatus.ACTIVE, interaction_flags = 0 },
} }, { movement_current_heading = 0x5000 })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) == 0,
      "0x13FC6 rejects a facing difference at the quarter-turn boundary")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, heading = 32769,
      service_flags = E.ActorStatus.ACTIVE, interaction_flags = 0 },
} }, { movement_current_heading = 32767 })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) ~= 0,
      "0x13FB4 wraps the signed heading delta across 0x8000")

check(R.think_gate(state(), context({ source_projection_active = false })) ==
      "patrol", "0x142AE selects the patrol branch [C]")
check(R.think_gate(state({ source_behavior_flags = E.BehaviorFlag.ESCORT }),
      context({ source_behavior_flags = E.BehaviorFlag.ESCORT })) ==
      "follow_leader", "0x1450E formation follow is selected by escort [C]")

local WaypointLists = require("data.actor_waypoint_lists")
local patrol_list_offset, patrol_route
for list_offset, records in pairs(WaypointLists) do
    local candidate = records[0]
    if candidate ~= nil and candidate.initial.x == 1536 and
       candidate.initial.y == 4096 and candidate.points ~= nil and
       #candidate.points == 8 and candidate.points[1].x == 15616 and
       candidate.points[1].y == 4096 then
        patrol_list_offset, patrol_route = list_offset, candidate
        break
    end
end
check(patrol_route ~= nil,
      "generated spawn data contains the hand-traced 0x679F0 patrol route [C]")
if patrol_route ~= nil then
    local route_context = context({
        source_actor_list_offset = patrol_list_offset,
        source_actor_source_order = 0,
        source_actor_slot = 0,
        source_world_x = patrol_route.initial.x,
        source_world_y = patrol_route.initial.y,
    })
    route_context.measure_destination = function()
        return { heading = 0, distance = 0, step_cost = 0 }
    end
    s = state()
    check(R.think(s, route_context) and
          s.ai_route_target_x == 15616 and s.ai_route_target_y == 4096 and
          s.ai_waypoint_index == E.AiTuning.PATROL_WAYPOINT_STRIDE,
          "0x142F4 advances +0xE4 and selects the first patrol pair [C]")

    s.behavior_timer = 0
    route_context.source_world_x = patrol_route.points[1].x
    route_context.source_world_y = patrol_route.points[1].y
    s.ai_route_target_x = patrol_route.points[1].x
    s.ai_route_target_y = patrol_route.points[1].y
    check(R.think(s, route_context) and
          s.ai_route_target_x == 15616 and s.ai_route_target_y == 8704 and
          s.ai_waypoint_index == E.AiTuning.PATROL_WAYPOINT_STRIDE * 2,
          "0x142F4 advances to the next ROM waypoint pair [C]")

    s.behavior_timer = 0
    local last_point = patrol_route.points[#patrol_route.points]
    route_context.source_world_x = last_point.x
    route_context.source_world_y = last_point.y
    s.ai_route_target_x, s.ai_route_target_y = last_point.x, last_point.y
    s.ai_waypoint_index = #patrol_route.points *
                          E.AiTuning.PATROL_WAYPOINT_STRIDE
    check(R.think(s, route_context) and
          s.ai_route_target_x == patrol_route.initial.x and
          s.ai_route_target_y == patrol_route.initial.y and
          s.ai_waypoint_index == 0,
          "0x142F6 clears +0xE4 and wraps to the spawn pair [C]")
end

local redmond_route_records = WaypointLists[0x172D1C]
local redmond_expected = {
    { offset = 0x172D58, x = 10240, y = 8704 },
    { offset = 0x172D68, x = 21248, y = 9472 },
    { offset = 0x172D78, x = 25216, y = 2944 },
}
for source_order = 0, 2 do
    local route = redmond_route_records and redmond_route_records[source_order]
    local expected = redmond_expected[source_order + 1]
    local valid = route ~= nil and route.position_offset == expected.offset and
        route.initial ~= nil and type(route.points) == "table"
    check(valid,
          string.format("scene-2 pedestrian %d has its ROM 0x172D1C route", source_order))
    if valid then
        local route_context = context({
            entity_kind = E.SpriteType.PEDESTRIAN,
            source_record_flags = E.ActorStatus.ACTIVE,
            source_spawn_state_flags = pedestrian,
            source_actor_slot = source_order + 3,
            source_actor_list_offset = 0x172D1C,
            source_actor_source_order = source_order,
            source_world_x = route.initial.x,
            source_world_y = route.initial.y,
        })
        route_context.measure_destination = function()
            return { heading = 0, distance = 0, step_cost = 0 }
        end
        local pedestrian_state = state()
        check(R.think(pedestrian_state, route_context) and
              pedestrian_state.ai_route_target_x == expected.x and
              pedestrian_state.ai_route_target_y == expected.y and
              pedestrian_state.ai_waypoint_index ==
                  E.AiTuning.PATROL_WAYPOINT_STRIDE,
              string.format("0x142F4 selects pedestrian %d's first Redmond waypoint", source_order))
    end
end

local collision = game.movement.collision
game.movement.collision = function() return 0, 0 end
local function formation_case(actor_slot, leader_slot, leader_heading,
                              expected_x, expected_y, description)
    local route_x, route_y
    local formation_context = context({
        controlled_runner_slot = leader_slot,
        source_actor_slot = actor_slot,
        source_behavior_flags = E.BehaviorFlag.ESCORT,
        ai_target_candidates = {
            { id = "party.runner." .. leader_slot,
              source_record_slot = leader_slot,
              world_activity_actor = false,
              world_x = 8192, world_y = 9216,
              source_record_heading = leader_heading,
              heading = 0, distance = 0, interaction_flags = 0 },
        },
    })
    formation_context.measure_destination = function(x, y)
        route_x, route_y = x, y
        return {
            heading = 0,
            distance = E.AiTuning.ARRIVE_DIST + 1,
            step_cost = E.AiTuning.ARRIVE_DIST + 1,
        }
    end
    local follower = state()
    local handled = R.think(follower, formation_context)
    check(handled and follower.ai_route_target_x == expected_x and
          follower.ai_route_target_y == expected_y and
          route_x == expected_x and route_y == expected_y and
          follower.movement_active == true,
          description)
end

formation_case(E.AiTuning.FOLLOW_FIRST_SLOT,
    E.AiTuning.FOLLOW_FIRST_LEADER_SLOT, 0, 8033, 9374,
    "0x1451E slot 0 subtracts 0x6000 from heading 0 and uses radius 0xE0 [C]")
formation_case(E.AiTuning.FOLLOW_SECOND_SLOT,
    E.AiTuning.FOLLOW_FIRST_LEADER_SLOT, 0, 8033, 9374,
    "0x14532 slot 1 subtracts when the leader slot is 0 [C]")
formation_case(E.AiTuning.FOLLOW_SECOND_SLOT,
    E.AiTuning.FOLLOW_SECOND_SLOT, 0, 8350, 9374,
    "0x1452A slot 1 adds 0x6000 when the leader slot is nonzero [C]")
formation_case(E.AiTuning.FOLLOW_FIRST_SCENE_ACTOR_SLOT,
    E.AiTuning.FOLLOW_SECOND_SLOT, 0, 8350, 9374,
    "0x1452A adds 0x6000 for scene actor +0x3E=3 [C]")
formation_case(E.AiTuning.FOLLOW_FIRST_SLOT,
    E.AiTuning.FOLLOW_FIRST_LEADER_SLOT, E.AiTuning.QUARTER_TURN, 8033, 9057,
    "0x1451E rotates a 0x4000 leader heading to the ROM offset [C]")

local collision_calls = 0
game.movement.collision = function()
    collision_calls = collision_calls + 1
    return collision_calls == 1 and E.Collision.BLOCKED or 0, 0
end
local reverse_x, reverse_y
local blocked_context = context({
    controlled_runner_slot = E.AiTuning.FOLLOW_FIRST_LEADER_SLOT,
    source_actor_slot = E.AiTuning.FOLLOW_FIRST_SLOT,
    source_behavior_flags = E.BehaviorFlag.ESCORT,
    ai_target_candidates = {
        { id = "party.runner." .. E.AiTuning.FOLLOW_FIRST_LEADER_SLOT,
          source_record_slot = E.AiTuning.FOLLOW_FIRST_LEADER_SLOT,
          world_activity_actor = false,
          world_x = 8192, world_y = 9216,
          source_record_heading = 0, heading = 0, distance = 0,
          interaction_flags = 0 },
    },
})
blocked_context.measure_destination = function(x, y)
    reverse_x, reverse_y = x, y
    return {
        heading = 0,
        distance = E.AiTuning.ARRIVE_DIST + 1,
        step_cost = E.AiTuning.ARRIVE_DIST + 1,
    }
end
s = state()
R.think(s, blocked_context)
check(collision_calls == 2 and reverse_x == 8351 and reverse_y == 9058 and
      s.ai_route_target_x == reverse_x and s.ai_route_target_y == reverse_y,
      "0x14564 negates a blocked formation offset and probes the reverse [C]")
game.movement.collision = collision

-- ROM 0xB574 and 0xB6BA-0xB6DE truncate dynamic-enemy walk headings for
-- movement and C062 selection even when +0x10.THINK is clear. The player
-- BABC selector remains on its existing rounded path.
local walk_selector_offsets = {}
for direction = 0, 7 do
    walk_selector_offsets[direction + 1] =
        direction * E.ActorAnimation.DIRECTION_SELECTOR_STEP
end

local function movement_fixture(dynamic_actor)
    return {
        dynamic_world_encounter_actor = dynamic_actor,
        entity_kind = dynamic_actor and E.SpriteType.CORP_GUARD
            or E.SpriteType.FIELD_RUNNER,
        source_record_flags = E.ActorStatus.ACTIVE & ~E.ActorStatus.THINK,
        source_spawn_state_flags = dynamic_actor and E.ActorFlag.HOSTILE
            or E.ActorFlag.NONE,
        source_behavior_flags = 0,
        source_projection_active = true,
        world_activity_active = false,
        movement_entity_flags = E.ActorStatus.ACTIVE & ~E.ActorStatus.THINK,
        movement_services_available = true,
        movement_idle_selectors = walk_selector_offsets,
        movement_offsets = {
            { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 },
            { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 },
        },
        movement_state = {
            event_flags = 0,
            collision_flags = 0,
            movement_flags = 0,
            position_x = 0x4000,
            position_y = 0x4000,
            route_flags = 0,
            route_status = 0,
        },
    }
end

local function movement_heading_case(heading, expected_direction,
                                     dynamic_actor, description)
    local movement_context = movement_fixture(dynamic_actor)
    local actor_state = {
        movement_heading = heading,
        movement_current_heading = heading,
    }
    local step = R.advance_entity_movement(movement_context, actor_state)
    local expected_selector = E.ActorPose.WALK +
        E.ActorAnimation.DIRECTION_SELECTOR_STEP * expected_direction
    check(step ~= nil and step.outcome == "accepted" and
          actor_state.direction == expected_direction and
          step.direction_animation == expected_selector and
          R.movement_direction_from_heading(movement_context, heading) ==
              expected_direction,
          description)
end

game.movement.collision = function() return 0, 0 end
movement_heading_case(0xF000, 7, true,
    "enemy B574/C062 truncates 0xF000 with THINK clear to direction 7 / selector 0x3C [C]")
movement_heading_case(0x1000, 0, true,
    "enemy B574/C062 truncates 0x1000 with THINK clear to direction 0 / selector 0x20 [C]")
movement_heading_case(0xF000, 0, false,
    "non-dynamic controller heading keeps the rounded BABC direction [C]")

local heartbeat_actor_id = "playtest13_dynamic_hellhound"
local heartbeat_target = {
    id = "party.runner.0",
    world_activity_actor = false,
    projected_y_nonzero = true,
    service_flags = E.ActorStatus.ACTIVE,
    interaction_flags = 0,
    heading = 0xF000,
    distance = E.AiTuning.HELL_HOUND_PURSUIT_RANGE,
}
local heartbeat_context = movement_fixture(true)
heartbeat_context.actor_id = heartbeat_actor_id
heartbeat_context.profile_id = "hell_hound_k0c_none00"
heartbeat_context.entity_kind = E.SpriteType.HELL_HOUND
heartbeat_context.action_kind = "none"
heartbeat_context.action_code = E.ItemId.NONE
heartbeat_context.phase = "behavior"
heartbeat_context.spawn = true
heartbeat_context.initial_heading = 0xF000
heartbeat_context.entry_delay = 0
heartbeat_context.entry_animation_frame_count = 1
heartbeat_context.movement_allowed = true
heartbeat_context.source_actor_posture = E.Posture.NEUTRAL
heartbeat_context.ai_target_candidates = { heartbeat_target }
heartbeat_context.ai_random_bound = function() return 0 end
check(ActorAI.heartbeat(heartbeat_context) ~= nil,
      "dynamic enemy heartbeat accepts its source movement profile")
local heartbeat_state = game.actors[heartbeat_actor_id]
heartbeat_state.transition_complete = true
heartbeat_state.behavior_timer = 1
heartbeat_state.linked_target_id = heartbeat_target.id
heartbeat_state.movement_active = true
heartbeat_state.movement_heading = 0xF000
heartbeat_state.movement_current_heading = 0xF000
heartbeat_context.spawn = false
local source_think = R.think
R.think = function() return false end
local heartbeat_result = ActorAI.heartbeat(heartbeat_context)
R.think = source_think
check(heartbeat_result ~= nil and heartbeat_result.direction == 7 and
      heartbeat_state.movement_step ~= nil and
      heartbeat_state.movement_step.direction_animation ==
          E.ActorPose.WALK + E.ActorAnimation.DIRECTION_SELECTOR_STEP * 7,
      "dynamic enemy heartbeat preserves truncated direction 7 / C062 selector 0x3C [C]")

game.movement.collision = collision

print(string.format("ai_think tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
