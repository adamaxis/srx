-- Source-order checks for the 0x13904 actor think dispatcher.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
require("actor.init")
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

local s, c = state(), context({ source_actor_posture = E.Posture.NEUTRAL })
local due_count = 0
local first_path
for pass_index = 1, E.AiTuning.THINK_FRAMES + 1 do
    local path = R.advance_think(s, c)
    if path ~= "wait" then
        due_count = due_count + 1
        first_path = path
    end
end
check(due_count == 1 and first_path == "patrol",
      "0x13904 runs the first thought, then counts 20 intervening passes")
local second_path = R.advance_think(s, c)
check(second_path == "patrol" and
      s.behavior_timer == E.AiTuning.THINK_FRAMES,
      "0x1391A reloads the named think period after a due pass")

s, c = state(), context({ source_think_suppressed = true })
check(R.advance_think(s, c) == "skip" and
      s.behavior_timer == E.AiTuning.THINK_RETRY,
      "0x1390E suppression keeps the timer negative for next-pass retry")
c.source_think_suppressed = false
check(R.advance_think(s, c) == "patrol" and
      s.behavior_timer == E.AiTuning.THINK_FRAMES,
      "FFE175 clear retries immediately and refreshes +0xB6")

local thon_state = state()
local thon_path = R.think_gate(thon_state,
    context({ entity_kind = E.SpriteType.THON }))
check(thon_path == "combat" and
      thon_state.ai_think_gap == "thon_period_path_unported",
      "0x13920 identifies the THON transfer and leaves its special body explicit")

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

local hold_path = R.think_gate(state(), context({ source_projection_active = false,
    world_activity_active = true, world_scene_state = E.EndingState.FINAL_FIGHT,
    source_action_ammo = E.AiTuning.ENDING_HOLD_ROUNDS }))
s = state({ source_animation_selector = E.AiTuning.ANIM_BUSY })
check(hold_path == "hold" and R.apply_think_path(s, c, hold_path) and
      s.animation_id == "idle" and not s.movement_active,
      "0x13E40 idle pose sets the held idle continuation")
s = state({ source_animation_selector = E.AiTuning.ANIM_BUSY + 1,
            movement_active = true, animation_id = "action" })
R.apply_think_path(s, c, "hold")
check(s.animation_id == "action" and s.movement_active,
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
local flee_path = R.think_gate(s, c)
R.apply_think_path(s, c, flee_path)
check(flee_path == "flee" and measured_x == 0x3F80 and
      measured_y == 0x4480 and
      (s.source_behavior_flags & E.BehaviorFlag.FLEEING) ~= 0,
      "0x13E9E stores camera-biased flee coordinates once and 0x145A2 routes")
local stored_x = s.flee_target_x
c.source_camera_x = 0x3000
R.apply_think_path(s, c, "flee")
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
local watch_path = R.think_gate(s, context({
    source_actor_posture = E.Posture.MEDIUM_OFFENSE,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.WATCH_DIST - 1,
          heading = 0x2000, interaction_flags = 0 },
    },
}))
R.apply_think_path(s, c, watch_path)
check(watch_path == "watch" and s.movement_current_heading == 0x2000,
      "0x13F68 watch branch faces FFFC04")

local function guard_case(overrides, actor_overrides, line_clear)
    local gc = context({
        source_record_flags = E.ActorStatus.ACTIVE | E.ActorStatus.GROUP_LEADER,
        source_actor_posture = E.Posture.NEUTRAL,
        source_alarm_active = false,
        source_scene_mode = E.SceneMode.BUILDING,
        guard_facing_reference = 0x1000,
        action_line_clear = function() return line_clear ~= false end,
        ai_target_candidates = {
            { id = "party.runner.0", source_record_slot = 0,
              world_activity_actor = false, world_x = 0x2000,
              world_y = 0x2400, distance = 0x300, heading = 0x1000,
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
s, c = guard_case({ guard_facing_reference = false })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) == 0 and
      s.ai_think_gap == "guard_facing_reference_unresolved",
      "0x13FB4 keeps the missing FF13AE producer explicit")
s, c = guard_case({ guard_facing_reference = 0x1000 },
                  { movement_current_heading = 0x5000 })
check((s.source_spawn_state_flags & E.ActorFlag.SPOTTED) == 0,
      "0x13FC6 rejects a facing difference at the quarter-turn boundary")

check(R.think_gate(state(), context({ source_projection_active = false })) ==
      "patrol", "0x142AE is returned as patrol while waypoint service is absent")
check(R.think_gate(state({ source_behavior_flags = E.BehaviorFlag.ESCORT }),
      context({ source_behavior_flags = E.BehaviorFlag.ESCORT })) ==
      "follow_leader", "0x1450E formation follow is named while slot support is absent")

print(string.format("ai_think tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
