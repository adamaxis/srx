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

local function actor_facts(overrides)
    local facts = {
        lifecycle = {
            active = true, thinking_enabled = false, ai_runner = false,
            downed = false, group_leader = false, dead = false,
        },
        interaction = {
            projectile = false, proximity_interaction_pending = false,
            use_collision_heading = false, collision_recovery_active = false,
            action_hook_pending = false, defense_difficulty_reduction = false,
            breathing_out = false, unseen = false,
            no_coverage_difficulty_bonus = false,
        },
        behavior = {
            route_clear = false, path_active = false,
            restart_steering = false, wall_following = false,
            in_target_band = false, route_transition = false,
            fleeing = false, escort = false,
        },
        roles = {
            spotted = false, reinforcement = false,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false,
        },
    }
    for group, values in pairs(overrides or {}) do
        for name, value in pairs(values) do
            facts[group][name] = value
        end
    end
    return facts
end

local function context(overrides)
    local c = {
        visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD,
        actor_facts = actor_facts(),
        source_projection_active = true,
        source_think_suppressed = false,
        source_alarm_active = false,
        scene_mode = "street",
        world_activity_active = false,
        field_sequence_active = false,
        final_battle_active = false,
        actor_behavior_profile = {
            engagement_policy = "patrol", approach_band = "far",
        },
        animation_blocks_behavior = false,
        ammunition_remaining = 0,
        hold_for_ending_activity = false,
        controlled_party_member_id = "party.runner.0",
        formation_position = "primary",
        ai_target_candidates = {
            { id = "party.runner.0", source_record_slot = 0,
              formation_position = "primary",
              world_activity_actor = false, world_x = 0x2000,
              world_y = 0x2400, bearing_degrees = 22.5, distance = 0x300,
              projected_y_nonzero = true,
              actor_facts = actor_facts() },
        },
    }
    for key, value in pairs(overrides or {}) do
        if key == "actor_facts" then
            c.actor_facts = actor_facts(value)
        else
            c[key] = value
        end
    end
    return c
end

local function state(overrides)
    local s = {
        behavior_timer = 0,
        actor_facts = actor_facts(),
        animation_id = "idle",
        movement_current_bearing_degrees = 22.5,
        movement_active = false,
    }
    for key, value in pairs(overrides or {}) do
        if key == "actor_facts" then
            s.actor_facts = actor_facts(value)
        else
            s[key] = value
        end
    end
    return s
end

local function think_path(actor, facts)
    R.think(actor, facts)
    return actor.ai_think_path
end

local approach_band_cases = {
    { band = "far", minimum = 1600, maximum = 1920 },
    { band = "medium", minimum = 1120, maximum = 1440 },
    { band = "close", minimum = 480, maximum = 960 },
}
for _, band_case in ipairs(approach_band_cases) do
    local bounds, resolved = R.source_motion_bounds(context({
        actor_facts = { lifecycle = { ai_runner = true } },
        actor_behavior_profile = {
            engagement_policy = "engage", approach_band = band_case.band,
        },
    }), state())
    check(resolved and bounds.minimum_distance == band_case.minimum and
          bounds.maximum_distance == band_case.maximum,
          "named " .. band_case.band .. " actor approach band selects its world-unit range")
end
local unresolved_bounds, unresolved_band = R.source_motion_bounds(context({
    actor_facts = { lifecycle = { ai_runner = true } },
    actor_behavior_profile = { engagement_policy = "engage" },
}), state())
check(not unresolved_band and unresolved_bounds == nil,
      "actor approach without a recovered band leaves the route pending")

local recorded_action_delay = { action_delay = 2 }
R.advance_source_action_delay(recorded_action_delay, {
    phase = "behavior", has_live_actor_record = true,
})
local unrecorded_action_delay = { action_delay = 2 }
R.advance_source_action_delay(unrecorded_action_delay, {
    phase = "behavior", has_live_actor_record = false,
})
local dynamic_action_delay = { action_delay = 2 }
R.advance_source_action_delay(dynamic_action_delay, {
    phase = "behavior", dynamic_world_encounter_actor = true,
})
check(recorded_action_delay.action_delay == 1 and
      unrecorded_action_delay.action_delay == 2 and
      dynamic_action_delay.action_delay == 1,
      "0xA8EA advances delay for live and dynamic actors without slot bytes [C]")

local function wounded_candidate(id, party_role, physical_meter,
                                world_activity_actor)
    return {
        id = id,
        party_member_role = party_role,
        world_activity_actor = world_activity_actor == true,
        actor_facts = actor_facts(),
        combatant = { physical_meter = physical_meter },
    }
end
local party_candidates = {
    wounded_candidate("party.runner.leader", "leader", 5),
    wounded_candidate("party.runner.one", "companion_one", 2),
    wounded_candidate("party.runner.two", "companion_two", 1),
    wounded_candidate("world.actor", "leader", 0, true),
}
local wounded = R.first_wounded_party_runner({
    ai_target_candidates = party_candidates,
})
check(wounded ~= nil and wounded.id == "party.runner.one",
      "0x13C20 selects the first wounded party role and skips world actors [C]")
party_candidates[2].combatant.physical_meter = 4
wounded = R.first_wounded_party_runner({
    ai_target_candidates = party_candidates,
})
check(wounded ~= nil and wounded.id == "party.runner.two",
      "0x13C20 continues to the next party role when the first is not wounded [C]")

-- ROM 0x1A342 allocates field actor records and 0x44BC copies their
-- identity into the actor state [C]. Lua receives live membership as a fact.
local field_actor_context = {
    actor_id = "pedestrian.test",
    profile_id = "pedestrian_k0b_none00",
    visual_profile_id = E.ActorVisualProfile.PEDESTRIAN_RAT_SHAMAN,
    action_kind = R.ActionKind.none,
    initial_bearing_degrees = 92.8125,
    entry_delay = 0,
    source_actor_has_waypoint_data = true,
    live_field_actor = true,
    actor_facts = actor_facts(),
}
local field_actor_profile = R.source_profile(field_actor_context)
local field_actor_state = R.initial_state(
    field_actor_context, field_actor_profile)
field_actor_context.actor_id = "pedestrian.detached"
field_actor_context.live_field_actor = false
local detached_actor_state = R.initial_state(
    field_actor_context, R.source_profile(field_actor_context))
check(field_actor_profile ~= nil and field_actor_state ~= nil and
      field_actor_state.live_field_actor == true and
      field_actor_state.movement_current_bearing_degrees == 92.8125 and
      detached_actor_state ~= nil and
      detached_actor_state.live_field_actor == false and
      detached_actor_state.movement_current_bearing_degrees == nil,
      "source patrol movement starts only for a live field actor [C]")

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

local s, c = state(), context({ actor_behavior_profile = {
    engagement_policy = "patrol", approach_band = "far",
} })
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

-- [C] ROM 0x13A4C pauses Thon's shared cadence during an active animation;
-- 0x13A66 reloads the interval to eight after it expires. Assert that as a
-- semantic cooldown rather than reproducing the original byte arithmetic.
local thon_state = state()
local thon_context = context({
    visual_profile_id = E.ActorVisualProfile.THON,
    thon_action_cooldown_remaining = E.AiTuning.THON_ACTION_INTERVAL,
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, projected_y_nonzero = true,
          actor_facts = actor_facts(),
          world_x = 0x2000, world_y = 0x2400,
          bearing_degrees = 22.5, distance = 0x300 },
    },
    ai_random_bound = function() return 0 end,
    action_line_clear = function() return true end,
})
local thon_waits = 0
for _ = 1, E.AiTuning.THON_ACTION_INTERVAL do
    local path = R.think_gate(thon_state, thon_context)
    if path == "continue_movement" then
        thon_waits = thon_waits + 1
    end
    thon_context.thon_action_cooldown_remaining =
        thon_state.thon_action_cooldown_remaining
end
local thon_fire_path = R.think_gate(thon_state, thon_context)
check(thon_waits == E.AiTuning.THON_ACTION_INTERVAL and
      thon_fire_path == "thon_action" and
      thon_state.thon_action_cooldown_remaining ==
          E.AiTuning.THON_ACTION_INTERVAL and
      thon_state.ai_target_id == "party.runner.0" and
      thon_state.movement_current_bearing_degrees == 22.5 and
      thon_state.animation_id == "spell",
      "Thon waits for eight behavior checks, then aims and casts")

local offscreen_runner = {
    id = "party.runner.1",
    world_activity_actor = false,
    projected_y_nonzero = false,
    actor_facts = actor_facts(),
}
local visible_runner = {
    id = "party.runner.0",
    world_activity_actor = false,
    projected_y_nonzero = true,
    actor_facts = actor_facts(),
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
    visual_profile_id = E.ActorVisualProfile.GHOUL_EYE_FIVER,
    dynamic_world_encounter_actor = true,
    movement_allowed = true,
    initial_bearing_degrees = 0,
})
local strike_state = state({
    visual_profile_id = E.ActorVisualProfile.GHOUL_EYE_FIVER,
    action_delay = -1,
    movement_current_bearing_degrees = 0,
})
local strike_target = {
    id = "party.runner.0",
    bearing_degrees = E.AiTuning.QUARTER_TURN_DEGREES,
    distance = E.AiTuning.CLOSE_STRIKE_RANGE - 1,
}
local strike_in_range = R.resolve_source_target_motion(
    strike_context, strike_state, strike_target)
local strike_audio, strike_heading_audio = R.begin_dynamic_target_action(
    strike_state, strike_context, strike_target)
local strike_result = R.snapshot(strike_state, "hold", strike_audio, nil,
                                 strike_heading_audio)
check(strike_in_range and strike_result.combat_engagement_started == true and
      strike_state.combat_engagement_started == false,
      "0xB258 accepted strike emits one combat-engagement event [C]")

local thon_unknown_state = state()
check(R.think_gate(thon_unknown_state,
      context({ visual_profile_id = E.ActorVisualProfile.THON })) == "thon_unresolved" and
      thon_unknown_state.ai_think_gap ==
          "thon_action_cooldown_unavailable",
      "THON leaves an unavailable action cooldown as an explicit gap")

local thon_busy_state = state()
local thon_los_calls = 0
thon_context.animation_blocks_behavior = true
thon_context.thon_action_cooldown_remaining =
    E.AiTuning.THON_ACTION_INTERVAL
thon_context.action_line_clear = function()
    thon_los_calls = thon_los_calls + 1
    return true
end
check(R.think_gate(thon_busy_state, thon_context) == "thon_busy" and
      thon_busy_state.thon_action_cooldown_remaining == nil and
      thon_los_calls == 0,
      "an active Thon animation pauses its cooldown and action checks")

local escort_path = R.think_gate(state({
    actor_facts = { behavior = { escort = true } },
}), context({ actor_facts = { behavior = { escort = true } } }))
check(escort_path == "follow_leader",
      "0x1392A escort branch selects 0x14508 follow")

check(R.think_gate(state({ actor_facts = { roles = { quest = true } } }),
    context({ actor_facts = { roles = { quest = true } } })) == "skip",
    "0x1393C +0x9E.b3 returns")

check(R.think_gate(state({ actor_facts = { roles = { pedestrian = true } } }),
    context({ actor_facts = { roles = { pedestrian = true } },
              world_activity_active = true })) == "flee",
    "0x1394A pedestrian with FFDF84 active enters the flee route")

s = state({ actor_facts = { roles = { despawn_offscreen = true } } })
local despawn_path = R.think_gate(s, context({
    actor_facts = { roles = { despawn_offscreen = true } },
    source_projection_active = false,
}))
check(despawn_path == "skip" and s.ai_deactivate_requested,
      "0x13962 offscreen +0x9E.b2 calls 0x1E514")

check(R.think_gate(state({ actor_facts = { behavior = { fleeing = true } } }),
    context({ actor_facts = { behavior = { fleeing = true } } })) == "flee",
    "0x1397A +0xB0.b6 returns to the stored flee route")

check(R.think_gate(state(), context({ source_projection_active = false })) ==
      "patrol", "0x13984 offscreen with FFDF84 clear selects patrol")
check(R.think_gate(state(), context({ source_projection_active = false,
      world_activity_active = true })) == "follow_leader",
      "0x139B4 active offscreen actor follows FFFC04")
check(R.think_gate(state(), context({ source_projection_active = false,
      world_activity_active = true,
      field_sequence_active = true,
      hold_for_ending_activity = true })) == "continue_movement",
      "0x13996 reaches 0x13E40 without clearing the existing movement continuation")
check(R.think_gate(state({ actor_facts = {
          lifecycle = { ai_runner = true },
      } }), context({ source_projection_active = false,
      world_activity_active = true,
      field_sequence_active = true,
      hold_for_ending_activity = true,
      actor_facts = { lifecycle = { ai_runner = true } } })) ==
      "follow_leader",
      "0x139A0 +0x10.b3 bypasses the ending-state hold branch")

s = state({ actor_facts = { roles = { hostile = true } } })
local hostile_path = R.think_gate(s, context({
    actor_facts = { roles = { hostile = true } },
}))
check(hostile_path == "combat" and s.combat_engagement_started,
      "0x139BE hostile on-screen branch starts combat engagement [C]")
check(R.think_gate(state(), context({ world_activity_active = true })) ==
      "combat", "0x139CE active FFDF84 takes the existing combat path")

local hold_context = context({ source_projection_active = false,
    world_activity_active = true, field_sequence_active = true,
    hold_for_ending_activity = true })
s = state({ movement_active = true, movement_bearing_degrees = 45,
            movement_route_distance = 0x500 })
hold_context.animation_blocks_behavior = false
check(R.think(s, hold_context) and
      s.ai_think_path == "continue_movement" and s.movement_active and
      s.movement_bearing_degrees == 45 and
      s.movement_route_distance == 0x500,
      "0x13E40 preserves active movement while refreshing B45C when due [C]")
s = state({ movement_active = true, animation_id = "action" })
hold_context.animation_blocks_behavior = true
check(R.think(s, hold_context) and s.animation_id == "action" and
      s.movement_active,
      "0x13E40 leaves a busy pose and its current movement alone")

s = state({ actor_facts = { roles = { pedestrian = true } } })
c = context({ actor_facts = { roles = { pedestrian = true } },
               world_activity_active = true,
               source_camera_x = 0x1000, source_camera_y = 0x6000,
               source_camera_limit_x = 0x8000,
               source_camera_limit_y = 0x8000 })
local measured_x, measured_y
c.measure_destination = function(x, y)
    measured_x, measured_y = x, y
    return { bearing_degrees = 45, distance = 0x500, step_cost = 0x300 }
end
local flee_handled = R.think(s, c)
check(flee_handled and s.ai_think_path == "flee" and measured_x == 0x3F80 and
      measured_y == 0x4480 and
      s.actor_facts.behavior.fleeing,
      "0x13E9E stores camera-biased flee coordinates once and 0x145A2 routes")
local stored_x = s.flee_target_x
c.source_camera_x = 0x3000
s.behavior_timer = 0
R.think(s, c)
check(s.flee_target_x == stored_x,
      "0x13E9E does not replace an existing flee destination")

s = state({ movement_active = true,
            actor_facts = { behavior = { fleeing = true } } })
check(R.think_gate(s, context({ actor_facts = { behavior =
      { fleeing = true } }, source_projection_active = false,
      animation_blocks_behavior = true })) == "continue_movement",
      "0x1397A offscreen flee route reaches 0x13E40 and preserves movement [C]")

s = state({ movement_active = true, movement_bearing_degrees = 270,
            movement_route_distance = 0x380,
            actor_facts = { roles = { pedestrian = true } } })
local offscreen_pedestrian = context({
    actor_facts = { roles = { pedestrian = true } },
    world_activity_active = true,
    source_projection_active = false,
})
check(R.think(s, offscreen_pedestrian) and
      s.ai_think_path == "continue_movement" and s.movement_active and
      s.movement_bearing_degrees == 270 and
      s.movement_route_distance == 0x380,
      "0x1394A offscreen pedestrian path preserves its movement continuation [C]")

check(R.think_gate(state(), context({ actor_behavior_profile = {
      engagement_policy = "patrol", approach_band = "far",
  } })) ==
      "patrol", "0x13EF4 posture <= 2 selects 0x142AE")
s = state()
check(R.think_gate(s, context({ actor_behavior_profile = {
    engagement_policy = "engage_within_range", approach_band = "medium",
},
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.ENGAGE_DIST,
          bearing_degrees = 22.5, actor_facts = actor_facts() },
    } })) == "patrol" and not s.combat_engagement_started,
    "0x13EF4 posture 4 at ENGAGE_DIST does not engage")
s = state()
check(R.think_gate(s, context({ actor_behavior_profile = {
    engagement_policy = "engage_within_range", approach_band = "medium",
},
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.ENGAGE_DIST - 1,
          bearing_degrees = 22.5, actor_facts = actor_facts() },
    } })) == "combat" and s.combat_engagement_started,
    "0x13EF4 posture 4 inside ENGAGE_DIST starts combat engagement")
check(R.think_gate(state(), context({ actor_behavior_profile = {
      engagement_policy = "engage", approach_band = "medium",
  } })) == "combat",
      "0x13F36 posture above 4 selects combat")
check(R.think_gate(state(), context({ actor_behavior_profile = {
      engagement_policy = "watch_nearby_runner", approach_band = "medium",
  },
      ai_target_candidates = {
          { id = "party.runner.0", source_record_slot = 0,
            world_activity_actor = false, distance = E.AiTuning.WATCH_DIST,
            bearing_degrees = 22.5, actor_facts = actor_facts() },
      } })) == "patrol",
      "0x13F52 watch distance at WATCH_DIST returns to patrol")
s = state()
local watch_context = context({
    actor_behavior_profile = {
        engagement_policy = "watch_nearby_runner", approach_band = "medium",
    },
    ai_target_candidates = {
        { id = "party.runner.0", source_record_slot = 0,
          world_activity_actor = false, distance = E.AiTuning.WATCH_DIST - 1,
          bearing_degrees = 45, actor_facts = actor_facts() },
    },
})
check(R.think(s, watch_context) and s.ai_think_path == "watch" and
      s.movement_current_bearing_degrees == 45,
      "0x13F68 watch branch faces FFFC04")
local missing_profile_context = context()
missing_profile_context.actor_behavior_profile = nil
local missing_profile_state = state()
check(R.think_gate(missing_profile_state, missing_profile_context) == "combat" and
      missing_profile_state.ai_think_gap ==
          "actor_behavior_profile_unavailable",
      "actor think keeps an explicit gap when the host profile is unavailable")
local invalid_profile_state = state()
check(R.think_gate(invalid_profile_state, context({
      actor_behavior_profile = { engagement_policy = "unrecognized" },
  })) == "combat" and
      invalid_profile_state.ai_think_gap == "actor_behavior_profile_invalid",
      "actor think does not infer behavior from an unknown policy")

local function guard_case(overrides, actor_overrides, line_clear)
    local gc = context({
        actor_facts = { lifecycle = { group_leader = true } },
        actor_behavior_profile = {
            engagement_policy = "patrol", approach_band = "far",
        },
        source_alarm_active = false,
        scene_mode = "building",
        action_line_clear = function() return line_clear ~= false end,
        ai_target_candidates = {
            { id = "party.runner.0", source_record_slot = 0,
              world_activity_actor = false, world_x = 0x2000,
              world_y = 0x2400, distance = 0x300, bearing_degrees = 22.5,
              actor_facts = actor_facts() },
        },
    })
    for key, value in pairs(overrides or {}) do gc[key] = value end
    local gs = state({ actor_facts = {
        lifecycle = { group_leader = true },
    },
        movement_current_bearing_degrees = 45 })
    for key, value in pairs(actor_overrides or {}) do gs[key] = value end
    return gs, gc, R.think_gate(gs, gc)
end

s, c = guard_case()
check(s.actor_facts.roles.spotted and
      s.field_event_request == E.EventFn.GUARD_SPOTS_YOU,
      "0x13F76 clear line and facing schedules GUARD_SPOTS_YOU in building mode")
s, c = guard_case({ scene_mode = "street" })
check(s.actor_facts.roles.spotted and
      s.field_event_request == nil,
      "0x13FF6 marks spotted outside building mode without the event")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, bearing_degrees = 22.5,
      actor_facts = actor_facts({ interaction = { unseen = true } }) },
} })
check(s.actor_facts.roles.spotted and
      s.field_event_request == nil,
      "0x13FDE suppresses the event while active runner +0x11.b7 is set")
s, c = guard_case({}, {}, false)
check(not s.actor_facts.roles.spotted,
      "0x13FAC blocked line check does not mark the actor spotted")
s, c = guard_case({ source_alarm_active = true })
check(s.combat_engagement_started and
      not s.actor_facts.roles.spotted,
      "0x13F80 alarm branch starts combat engagement before spotting [C]")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, bearing_degrees = 157.5,
      actor_facts = actor_facts({ lifecycle = { active = false } }) },
} })
check(s.actor_facts.roles.spotted and
      s.ai_think_gap == nil,
      "0x14D8E keeps the guard heading in FF13AE when leader ACTIVE is clear")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, bearing_degrees = 22.5,
      actor_facts = actor_facts(),
} } }, { movement_current_bearing_degrees = 112.5 })
check(not s.actor_facts.roles.spotted,
      "0x13FC6 rejects a facing difference at the quarter-turn boundary")
s, c = guard_case({ ai_target_candidates = {
    { id = "party.runner.0", source_record_slot = 0,
      world_activity_actor = false, world_x = 0x2000, world_y = 0x2400,
      distance = 0x300, bearing_degrees = 180.005493164063,
      actor_facts = actor_facts(),
} } }, { movement_current_bearing_degrees = 179.994506835938 })
check(s.actor_facts.roles.spotted,
      "0x13FB4 wraps the signed heading delta across 0x8000")

check(R.think_gate(state(), context({ source_projection_active = false })) ==
      "patrol", "0x142AE selects the patrol branch [C]")
check(R.think_gate(state({ actor_facts = {
          behavior = { escort = true } } }),
      context({ actor_facts = { behavior = { escort = true } } })) ==
      "follow_leader", "0x1450E formation follow is selected by escort [C]")

local ActorWaypointRoutes = require("data.actor_patrol_routes")
local routes_use_actor_ids = true
for actor_id, route in pairs(ActorWaypointRoutes) do
    if type(actor_id) ~= "string" or actor_id == "" or
       type(route) ~= "table" then
        routes_use_actor_ids = false
        break
    end
end
check(routes_use_actor_ids,
      "patrol content is keyed by actor identity, not ROM table position")
local patrol_actor_id = "field_scene_5_actor_0"
local patrol_route = ActorWaypointRoutes[patrol_actor_id]
check(patrol_route ~= nil,
      "actor route data contains the ROM 0x679F0 patrol path [C]")
if patrol_route ~= nil then
    local route_context = context({
        actor_id = patrol_actor_id,
        world_position = {
            x = patrol_route.initial.x,
            y = patrol_route.initial.y,
        },
    })
    route_context.measure_destination = function()
        return { bearing_degrees = 0, distance = 0, step_cost = 0 }
    end
    s = state()
    check(R.think(s, route_context) and
          s.ai_route_target.x == 15616 and s.ai_route_target.y == 4096 and
          s.ai_waypoint_index == E.AiTuning.PATROL_WAYPOINT_STRIDE,
          "0x142F4 advances +0xE4 and selects the first patrol pair [C]")

    s.behavior_timer = 0
    route_context.world_position = patrol_route.points[1]
    s.ai_route_target = patrol_route.points[1]
    check(R.think(s, route_context) and
          s.ai_route_target.x == 15616 and s.ai_route_target.y == 8704 and
          s.ai_waypoint_index == E.AiTuning.PATROL_WAYPOINT_STRIDE * 2,
          "0x142F4 advances to the next ROM waypoint pair [C]")

    s.behavior_timer = 0
    local last_point = patrol_route.points[#patrol_route.points]
    route_context.world_position = last_point
    s.ai_route_target = last_point
    s.ai_waypoint_index = #patrol_route.points *
                          E.AiTuning.PATROL_WAYPOINT_STRIDE
    check(R.think(s, route_context) and
          s.ai_route_target.x == patrol_route.initial.x and
          s.ai_route_target.y == patrol_route.initial.y and
          s.ai_waypoint_index == 0,
          "0x142F6 clears +0xE4 and wraps to the spawn pair [C]")
end

local redmond_route_records = {
    ActorWaypointRoutes["field_scene_2_actor_0"],
    ActorWaypointRoutes["field_scene_2_actor_1"],
    ActorWaypointRoutes["field_scene_2_actor_2"],
}
local redmond_expected = {
    { initial_x = 13440, initial_y = 8704, x = 10240, y = 8704 },
    { initial_x = 21248, initial_y = 14080, x = 21248, y = 9472 },
    { initial_x = 25216, initial_y = 6400, x = 25216, y = 2944 },
}
for source_order = 0, 2 do
    local route = redmond_route_records[source_order + 1]
    local expected = redmond_expected[source_order + 1]
    local valid = route ~= nil and route.initial ~= nil and
        route.initial.x == expected.initial_x and
        route.initial.y == expected.initial_y and
        type(route.points) == "table" and route.points[1] ~= nil and
        route.points[1].x == expected.x and route.points[1].y == expected.y
    check(valid,
          string.format("scene-2 pedestrian %d has its ROM route data", source_order))
    if valid then
        local route_context = context({
            actor_id = "field_scene_2_actor_" .. source_order,
            visual_profile_id = E.ActorVisualProfile.PEDESTRIAN_RAT_SHAMAN,
            actor_facts = { roles = { pedestrian = true } },
            world_position = route.initial,
        })
        route_context.measure_destination = function()
            return { bearing_degrees = 0, distance = 0, step_cost = 0 }
        end
        local pedestrian_state = state()
        check(R.think(pedestrian_state, route_context) and
              pedestrian_state.ai_route_target.x == expected.x and
              pedestrian_state.ai_route_target.y == expected.y and
              pedestrian_state.ai_waypoint_index ==
                  E.AiTuning.PATROL_WAYPOINT_STRIDE,
              string.format("0x142F4 selects pedestrian %d's first Redmond waypoint", source_order))
    end
end

-- Exercise every extracted +0xE0 route pair for the six regional maps. ROM
-- 0x142F4 advances the four-byte cursor at each reached point; 0x142F6 clears
-- it at the zero-X terminator and resumes from the actor's initial pair.
-- Route counts are the independently recorded ROM list counts, while the
-- extracted coordinates supply the actual source target values.
local regional_pedestrian_routes = {
    { scene = 0, counts = { 19, 13, 9 } }, -- ROM list 0xD5538
    { scene = 1, counts = { 10, 4, 7 } }, -- ROM list 0x17D660
    { scene = 2, counts = { 11, 7, 3 } }, -- ROM list 0x172D1C
    { scene = 3, counts = { 8, 2, 7 } }, -- ROM list 0x18A73A
    { scene = 4, counts = { 17, 11, 7 } }, -- ROM list 0x15B998
    { scene = 5, counts = { 8, 5, 2 } }, -- ROM list 0x679F0
}
local regional_route_steps = 0
for _, region in ipairs(regional_pedestrian_routes) do
    for source_order = 0, 2 do
        local actor_id = "field_scene_" .. region.scene .. "_actor_" ..
            source_order
        local route = ActorWaypointRoutes[actor_id]
        local expected_points = region.counts[source_order + 1]
        local valid_route = route ~= nil and route.initial ~= nil and
            type(route.initial.x) == "number" and
            type(route.initial.y) == "number" and
            type(route.points) == "table" and
            #route.points == expected_points and
            route.group_leader == true
        check(valid_route,
              string.format("scene %d pedestrian %d has %d source waypoints",
                            region.scene, source_order, expected_points))
        if valid_route then
            local route_context = context({
                actor_id = actor_id,
                visual_profile_id = E.ActorVisualProfile.PEDESTRIAN_RAT_SHAMAN,
                actor_facts = { roles = { pedestrian = true } },
                world_position = route.initial,
            })
            route_context.measure_destination = function(target_x, target_y)
                if target_x ~= route_context.world_position.x or
                   target_y ~= route_context.world_position.y then
                    return nil
                end
                return { bearing_degrees = 0, distance = 0, step_cost = 0 }
            end

            local pedestrian_state = state({
                actor_facts = { roles = { pedestrian = true } },
            })
            local arrived_at = route.initial
            for step = 1, expected_points + 2 do
                route_context.world_position = arrived_at
                pedestrian_state.behavior_timer = 0

                local expected_target
                local expected_cursor
                if step <= expected_points then
                    expected_target = route.points[step]
                    expected_cursor = step * E.AiTuning.PATROL_WAYPOINT_STRIDE
                elseif step == expected_points + 1 then
                    expected_target = route.initial
                    expected_cursor = E.AiTuning.PATROL_INITIAL_CURSOR
                else
                    expected_target = route.points[1]
                    expected_cursor = E.AiTuning.PATROL_WAYPOINT_STRIDE
                end

                check(R.think(pedestrian_state, route_context) and
                      pedestrian_state.ai_route_target.x ==
                          expected_target.x and
                      pedestrian_state.ai_route_target.y ==
                          expected_target.y and
                      pedestrian_state.ai_waypoint_index == expected_cursor,
                      string.format(
                          "scene %d pedestrian %d route step %d follows ROM cursor",
                          region.scene, source_order, step))
                arrived_at = expected_target
                regional_route_steps = regional_route_steps + 1
            end
        end
    end
end
check(regional_route_steps == 186,
      "all 18 regional pedestrian routes include each waypoint and wrap");

local follow_route_step_clear = game.movement.follow_route_step_clear
game.movement.follow_route_step_clear = function() return true end
local function formation_case(actor_position, leader_position, leader_heading,
                              expected_x, expected_y, description)
    local route_x, route_y
    local leader_id = leader_position == "primary"
        and "party.runner.0" or "party.runner.1"
    local formation_context = context({
        controlled_party_member_id = leader_id,
        formation_position = actor_position,
        actor_facts = { behavior = { escort = true } },
        ai_target_candidates = {
            { id = leader_id,
              formation_position = leader_position,
              world_activity_actor = false,
              world_x = 8192, world_y = 9216,
              facing_degrees = leader_heading,
              bearing_degrees = 0, distance = 0, actor_facts = actor_facts() },
        },
    })
    formation_context.measure_destination = function(x, y)
        route_x, route_y = x, y
        return {
            bearing_degrees = 0,
            distance = E.AiTuning.ARRIVE_DIST + 1,
            step_cost = E.AiTuning.ARRIVE_DIST + 1,
        }
    end
    local follower = state({ actor_facts = { behavior = { escort = true } } })
    local handled = R.think(follower, formation_context)
    check(handled and follower.ai_route_target.x == expected_x and
          follower.ai_route_target.y == expected_y and
          route_x == expected_x and route_y == expected_y and
          follower.movement_active == true,
          description)
end

formation_case("primary", "primary", 0, 8033, 9374,
    "0x1451E gives the primary actor the counterclockwise offset [C]")
formation_case("secondary", "primary", 0, 8033, 9374,
    "0x14532 gives the secondary actor that offset behind a primary leader [C]")
formation_case("secondary", "secondary", 0, 8350, 9374,
    "0x1452A gives the secondary actor the clockwise offset otherwise [C]")
formation_case("other", "secondary", 0, 8350, 9374,
    "0x1452A gives other actors the clockwise offset [C]")
formation_case("primary", "primary", E.AiTuning.QUARTER_TURN_DEGREES,
    8033, 9057,
    "0x1451E rotates a quarter-turn leader heading to the primary offset [C]")

local collision_calls = 0
game.movement.follow_route_step_clear = function()
    collision_calls = collision_calls + 1
    return collision_calls ~= 1
end
local reverse_x, reverse_y
local blocked_context = context({
    controlled_party_member_id = "party.runner.0",
    formation_position = "primary",
    actor_facts = { behavior = { escort = true } },
    ai_target_candidates = {
        { id = "party.runner.0", formation_position = "primary",
          world_activity_actor = false,
          world_x = 8192, world_y = 9216,
          facing_degrees = 0, bearing_degrees = 0, distance = 0,
          actor_facts = actor_facts() },
    },
})
blocked_context.measure_destination = function(x, y)
    reverse_x, reverse_y = x, y
    return {
        bearing_degrees = 0,
        distance = E.AiTuning.ARRIVE_DIST + 1,
        step_cost = E.AiTuning.ARRIVE_DIST + 1,
    }
end
s = state({ actor_facts = { behavior = { escort = true } } })
R.think(s, blocked_context)
check(collision_calls == 2 and reverse_x == 8351 and reverse_y == 9058 and
      s.ai_route_target.x == reverse_x and s.ai_route_target.y == reverse_y,
      "0x14564 negates a blocked formation offset and probes the reverse [C]")
game.movement.follow_route_step_clear = follow_route_step_clear

-- ROM 0xB574 and 0xB6BA-0xB6DE truncate dynamic-enemy walk headings even
-- when +0x10.THINK is clear. The player BABC heading path remains rounded.
local function movement_fixture(dynamic_actor)
    return {
        dynamic_world_encounter_actor = dynamic_actor,
        visual_profile_id = dynamic_actor and E.ActorVisualProfile.HUMAN_GUARD
            or E.ActorVisualProfile.FIELD_RUNNER,
        actor_facts = actor_facts({
            lifecycle = { active = true, thinking_enabled = false },
            roles = { hostile = dynamic_actor == true },
        }),
        movement_actor_facts = actor_facts({
            lifecycle = { active = true, thinking_enabled = false },
        }),
        source_projection_active = true,
        world_activity_active = false,
        movement_services_available = true,
        movement_default_step_distance = 10,
        movement_offsets = {
            { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 },
            { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 },
        },
        movement_state = {
            substate_update = "unchanged",
            cardinal_direction = 0,
            mark_reset_after_block = false,
            mark_directional_fallback = false,
            clear_collision_recovery = false,
            clear_pending_event = false,
            position_x = 0x4000,
            position_y = 0x4000,
            route_facts = {
                route_clear = false, path_active = false,
                restart_steering = false, wall_following = false,
                in_target_band = false, route_transition = false,
                fleeing = false, escort = false,
            },
            route_status = 0,
        },
    }
end

local function movement_bearing_case(bearing_degrees, expected_direction,
                                     dynamic_actor, description)
    local movement_context = movement_fixture(dynamic_actor)
    local actor_state = {
        movement_bearing_degrees = bearing_degrees,
        movement_current_bearing_degrees = bearing_degrees,
        movement_route_distance = 0x500,
    }
    local step = R.advance_entity_movement(movement_context, actor_state)
    check(step ~= nil and step.outcome == "accepted" and
          actor_state.direction == expected_direction and
          actor_state.animation_id == "walk" and
          step.direction_animation == nil and
          R.movement_direction_from_bearing_degrees(
              movement_context, bearing_degrees) ==
              expected_direction,
          description)
end

game.movement.probe = function(x, y, direction, distance)
    local offsets = {
        north = { x = 0, y = -1 },
        north_east = { x = 1, y = -1 },
        east = { x = 1, y = 0 },
        south_east = { x = 1, y = 1 },
        south = { x = 0, y = 1 },
        south_west = { x = -1, y = 1 },
        west = { x = -1, y = 0 },
        north_west = { x = -1, y = -1 },
    }
    local offset = offsets[direction]
    return { kind = "clear", blocks_route_fallback = false,
        terrain_is_clear = true,
        candidate_x = (x + offset.x * distance) % 0x10000,
        candidate_y = (y + offset.y * distance) % 0x10000 }
end

-- ROM 0x148D2 chooses the dominant axis, 0x14854 probes paired clear
-- corridors, and 0x14914 checks the obstruction line. This synthetic wall
-- fixture checks the native waypoint policy only; it is not a GPGX route
-- capture or a claim that the full route matches the emulator.
local function vertical_wall_path_clear(wall_x, wall_top, wall_bottom)
    return function(from_x, from_y, to_x, to_y)
        if from_x == to_x then
            if from_x ~= wall_x then return true end
            return math.max(from_y, to_y) < wall_top or
                math.min(from_y, to_y) > wall_bottom
        end
        local crosses_wall =
            (from_x <= wall_x and to_x >= wall_x) or
            (to_x <= wall_x and from_x >= wall_x)
        if not crosses_wall then return true end
        local portion = (wall_x - from_x) / (to_x - from_x)
        local crossing_y = from_y + (to_y - from_y) * portion
        return crossing_y < wall_top or crossing_y > wall_bottom
    end
end

local wall_path_clear = vertical_wall_path_clear(0x2800, 0x1F80, 0x2080)
local wall_route = R.choose_obstruction_waypoint(
    0x2000, 0x2000, 0x3000, 0x2000, wall_path_clear)
check(wall_route ~= nil and wall_route.side == -1 and
      #wall_route.waypoints == 2 and
      wall_route.waypoints[1].x == 0x2100 and
      wall_route.waypoints[1].y == 0x1F00 and
      wall_route.waypoints[2].x == 0x3100 and
      wall_route.waypoints[2].y == 0x1F00,
      "host waypoint policy finds a clear parallel corridor in the synthetic wall [H]")
check(R.choose_obstruction_waypoint(
          0x2000, 0x2000, 0x3000, 0x2000,
          function() return true end) == nil,
      "clear direct path does not create an obstruction waypoint [C: 0x14914]")
local sealed_wall_route = R.choose_obstruction_waypoint(
    0x2000, 0x2000, 0x3000, 0x2000,
    function(from_x, from_y, to_x, to_y)
        local crosses_wall =
            (from_x <= 0x2800 and to_x >= 0x2800) or
            (to_x <= 0x2800 and from_x >= 0x2800)
        return not crosses_wall
    end)
check(sealed_wall_route == nil,
      "host waypoint policy gives up when the synthetic wall seals both sides [H]")
local vertical_route = R.choose_obstruction_waypoint(
    0x2000, 0x2000, 0x2000, 0x3000,
    function(from_x, from_y, to_x, to_y)
        if from_x == to_x and from_x == 0x2000 then
            return math.max(from_y, to_y) < 0x2800
        end
        return true
    end)
check(vertical_route ~= nil and vertical_route.side == -1 and
      vertical_route.waypoints[1].x == 0x1F00 and
      vertical_route.waypoints[2].x == 0x1F00,
      "host waypoint policy detours a vertical path along the other axis [H]")
local alternate_side_route = R.choose_obstruction_waypoint(
    0x2000, 0x2000, 0x3000, 0x2000, wall_path_clear,
    { side = -1, origin_cell_x = 0x2100, origin_cell_y = 0x2100 })
check(alternate_side_route ~= nil and alternate_side_route.side == 1 and
      alternate_side_route.waypoints[1].y == 0x2300,
      "host waypoint policy retries the opposite side after a blocked corridor [H]")
local heading_strip_cases = {
    { 0,   0x2100, 0x1F00, 0x2300, 0x1F00 },
    { 45,  0x2100, 0x1F00, 0x2300, 0x1F00 },
    { 90,  0x2300, 0x2100, 0x2300, 0x2300 },
    { 135, 0x2300, 0x2100, 0x2300, 0x2300 },
    { 180, 0x2100, 0x2300, 0x1F00, 0x2300 },
    { 225, 0x2100, 0x2300, 0x1F00, 0x2300 },
    { 270, 0x1F00, 0x2100, 0x1F00, 0x1F00 },
    { 315, 0x1F00, 0x2100, 0x1F00, 0x1F00 },
}
for _, strip_case in ipairs(heading_strip_cases) do
    local heading_strip_route = R.choose_obstruction_waypoint(
        0x2000, 0x2000, 0x3000, 0x2000,
        function(from_x, from_y, to_x, to_y)
            return (from_x == 0x2000 and from_y == 0x2000 and
                    to_x == strip_case[2] and to_y == strip_case[3]) or
                (from_x == strip_case[2] and from_y == strip_case[3] and
                 to_x == strip_case[4] and to_y == strip_case[5])
        end,
        nil, strip_case[1])
    check(heading_strip_route ~= nil and
          heading_strip_route.strategy == "heading_oriented_strip" and
          #heading_strip_route.waypoints == 1 and
          heading_strip_route.waypoints[1].x == strip_case[2] and
          heading_strip_route.waypoints[1].y == strip_case[3],
          string.format("0x14AF4 strip starts in source cardinal order at heading %d", strip_case[1]))
end
local rotated_heading_strip_route = R.choose_obstruction_waypoint(
    0x2000, 0x2000, 0x3000, 0x2000,
    function(from_x, from_y, to_x, to_y)
        return (from_x == 0x2000 and from_y == 0x2000 and
                to_x == 0x1F00 and to_y == 0x2100) or
            (from_x == 0x1F00 and from_y == 0x2100 and
             to_x == 0x1F00 and to_y == 0x1F00)
    end,
    nil, 0)
check(rotated_heading_strip_route ~= nil and
      rotated_heading_strip_route.strategy == "heading_oriented_strip" and
      rotated_heading_strip_route.waypoints[1].x == 0x1F00 and
      rotated_heading_strip_route.waypoints[1].y == 0x2100,
      "0x14AF4 rotates to the next open strip after its first direction is blocked [C]")

movement_bearing_case(337.5, 7, true,
    "enemy B574/C062 truncates 0xF000 with THINK clear to direction 7 / selector 0x3C [C]")
movement_bearing_case(22.5, 0, true,
    "enemy B574/C062 truncates 0x1000 with THINK clear to direction 0 / selector 0x20 [C]")
movement_bearing_case(337.5, 0, false,
    "non-dynamic controller heading keeps the rounded BABC direction [C]")

-- Scene 0x23 source order 1 follows 0x1DC3D6 from (0x6080,0x4C80)
-- toward (0x4C00,0x1A80). ROM 0x15D3C measures 0xF000; 0x145A2 installs
-- B574, whose generic B5BE and C062 selectors truncate that heading to 7.
-- Its 0x44BC-published +0x10 is 0x05, so THINK is clear even though it is a
-- routed actor. [C: ROM route/template tables and disassembly]
local route_actor_context = movement_fixture(false)
route_actor_context.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
route_actor_context.source_actor_has_waypoint_data = true
route_actor_context.actor_facts.roles.reinforcement = true
route_actor_context.movement_actor_facts.lifecycle.active = true
route_actor_context.combatant = { attributes = { quickness = 19 } }
local route_actor_state = {
    movement_bearing_degrees = 337.5,
    movement_current_bearing_degrees = 337.5,
    movement_route_distance = 0x3C40,
}
local route_actor_step = R.advance_entity_movement(
    route_actor_context, route_actor_state)
check(route_actor_step ~= nil and route_actor_step.outcome == "accepted" and
      route_actor_state.direction == 7 and
      route_actor_state.animation_id == "walk" and
      route_actor_step.direction_animation == nil and
      R.movement_direction_from_bearing_degrees(
          route_actor_context, 337.5) == 7,
      "Scene-35 default route guard B574/C062 truncates measured 0xF000 to direction 7 [C]")

-- Regional pedestrian records have +0x10 THINK clear, carry a source waypoint
-- distance in +0xC8, and use the B6E4 distance table. Exercise every threshold
-- boundary, including the >=0x460 return to the ordinary 10-unit vector. [C]
local regional_pedestrian_speed_cases = {
    { 0x00FF, 8 }, { 0x0100, 10 }, { 0x01FF, 10 },
    { 0x0200, 15 }, { 0x035F, 15 }, { 0x0360, 20 },
    { 0x045F, 20 }, { 0x0460, 10 },
}
for _, speed_case in ipairs(regional_pedestrian_speed_cases) do
    local speed_context = movement_fixture(false)
    speed_context.visual_profile_id = E.ActorVisualProfile.PEDESTRIAN_RAT_SHAMAN
    speed_context.source_actor_has_waypoint_data = true
    speed_context.actor_facts.roles.pedestrian = true
    speed_context.movement_actor_facts.lifecycle.active = true
    speed_context.combatant = { attributes = { quickness = 1 } }
    local speed_state = {
        movement_bearing_degrees = 90,
        movement_current_bearing_degrees = 90,
        movement_route_distance = speed_case[1],
    }
    local speed_step = R.advance_entity_movement(speed_context, speed_state)
    check(speed_step ~= nil and speed_step.outcome == "accepted" and
          speed_state.movement_distance_per_update == speed_case[2],
          string.format("regional pedestrian B6E4 distance 0x%04X selects %d-unit vector [C]",
                        speed_case[1], speed_case[2]))
end

-- The six regional profiles and Scene 35 contain 19 of 379 waypoint edges
-- whose 0x15D3C result crosses the old BABC rounding boundary. These ten
-- distinct measured headings cover every affected selector value. [C]
local route_heading_cases = {
    { 34.453125, 0 }, { 114.609375, 2 }, { 115.48828125, 2 },
    { 118.125, 2 }, { 157.5, 3 }, { 170.15625, 3 },
    { 254.53125, 5 }, { 262.96875, 5 }, { 337.5, 7 },
    { 357.1875, 7 },
}
for _, route_case in ipairs(route_heading_cases) do
    check(R.movement_direction_from_bearing_degrees(
              route_actor_context, route_case[1]) == route_case[2],
          string.format("default routed B574 bearing %.5f degrees truncates to %d [C]",
                        route_case[1], route_case[2]))
end

local saved_movement_probe = game.movement.probe
local saved_path_query = game.movement.path_is_clear
local route_path_query_calls = 0
game.movement.path_is_clear = function(from_x, from_y, to_x, to_y)
    route_path_query_calls = route_path_query_calls + 1
    return wall_path_clear(from_x, from_y, to_x, to_y)
end
game.movement.probe = function(x, y, direction)
    if direction == nil then
        return { kind = "clear", blocks_route_fallback = false,
            terrain_is_clear = true, candidate_x = x, candidate_y = y }
    end
    return { kind = "blocked", blocks_route_fallback = true,
        terrain_is_clear = false, candidate_x = x, candidate_y = y }
end
local obstruction_context = movement_fixture(true)
obstruction_context.movement_state.position_x = 0x2000
obstruction_context.movement_state.position_y = 0x2000
obstruction_context.ai_target_candidates = {
    { id = "party.runner.0", world_x = 0x3000, world_y = 0x2000 },
}
obstruction_context.measure_destination = function(x, y)
    local bearing = x == 0x3000 and y == 0x2000 and 45 or 315
    return { bearing_degrees = bearing, distance = 0x500,
        step_cost = 0x300 }
end
local obstruction_state = {
    ai_target_id = "party.runner.0",
    movement_bearing_degrees = 22.5,
    movement_current_bearing_degrees = 22.5,
    movement_route_distance = 0x1000,
}
local blocked_route_step = R.advance_entity_movement(
    obstruction_context, obstruction_state)
check(blocked_route_step ~= nil and
      blocked_route_step.outcome == "reset" and
      route_path_query_calls > 0 and
      obstruction_state.obstruction_route ~= nil and
      obstruction_state.obstruction_route.target_id == "party.runner.0" and
      obstruction_state.obstruction_route.index == 1,
      "blocked dynamic actor movement stores a target-linked obstruction route [H]")
if obstruction_state.obstruction_route ~= nil then
    local first_waypoint =
        obstruction_state.obstruction_route.waypoints[1]
    obstruction_context.movement_state.position_x = first_waypoint.x
    obstruction_context.movement_state.position_y = first_waypoint.y
    game.movement.probe = function(x, y, direction, distance)
        local offsets = {
            north = { x = 0, y = -1 }, north_east = { x = 1, y = -1 },
            east = { x = 1, y = 0 }, south_east = { x = 1, y = 1 },
            south = { x = 0, y = 1 }, south_west = { x = -1, y = 1 },
            west = { x = -1, y = 0 }, north_west = { x = -1, y = -1 },
        }
        local offset = offsets[direction] or { x = 0, y = 0 }
        return { kind = "clear", blocks_route_fallback = false,
            terrain_is_clear = true,
            candidate_x = (x + offset.x * (distance or 0)) % 0x10000,
            candidate_y = (y + offset.y * (distance or 0)) % 0x10000 }
    end
    local next_route_step = R.advance_entity_movement(
        obstruction_context, obstruction_state)
    check(next_route_step ~= nil and
          next_route_step.outcome == "accepted" and
          obstruction_state.obstruction_route ~= nil and
          obstruction_state.obstruction_route.index == 2,
          "reaching the entry waypoint advances to the corridor exit [H]")
    local exit_waypoint =
        obstruction_state.obstruction_route.waypoints[2]
    obstruction_context.movement_state.position_x = exit_waypoint.x
    obstruction_context.movement_state.position_y = exit_waypoint.y
    local resumed_chase_step = R.advance_entity_movement(
        obstruction_context, obstruction_state)
    check(resumed_chase_step ~= nil and
          resumed_chase_step.outcome == "accepted" and
          obstruction_state.obstruction_route == nil and
          obstruction_state.movement_bearing_degrees == 45,
          "route queue completion reacquires the live target heading [C: 0x149C0]")
end

-- ROM 0x14ADE clears the obstruction-route flags through 0x14756 when every
-- rotating strip candidate is blocked. A failed replan must discard Lua's
-- prior waypoint queue and restore the live target heading instead of
-- repeatedly steering at the stale point. [C]
local stale_route_context = movement_fixture(true)
stale_route_context.movement_state.position_x = 0x2000
stale_route_context.movement_state.position_y = 0x2000
stale_route_context.ai_target_candidates = {
    { id = "party.runner.0", world_x = 0x3000, world_y = 0x2000 },
}
stale_route_context.measure_destination = function(x, y)
    local bearing = x == 0x3000 and y == 0x2000 and 45 or 315
    return { bearing_degrees = bearing, distance = 0x500,
        step_cost = 0x300 }
end
game.movement.path_is_clear = function() return false end
game.movement.probe = function(x, y, direction)
    if direction == nil then
        return { kind = "clear", blocks_route_fallback = false,
            terrain_is_clear = true, candidate_x = x, candidate_y = y }
    end
    return { kind = "blocked", blocks_route_fallback = true,
        terrain_is_clear = false, candidate_x = x, candidate_y = y }
end
local stale_route_state = {
    ai_target_id = "party.runner.0",
    movement_bearing_degrees = 315,
    movement_current_bearing_degrees = 315,
    movement_route_distance = 0x500,
    obstruction_route = {
        target_id = "party.runner.0",
        index = 1,
        waypoints = { { x = 0x2100, y = 0x1F00 } },
        side = -1,
        origin_cell_x = 0x2100,
        origin_cell_y = 0x2100,
        target_cell_x = 0x3100,
        target_cell_y = 0x2100,
    },
}
local failed_replan_step = R.advance_entity_movement(
    stale_route_context, stale_route_state)
check(failed_replan_step ~= nil and
      failed_replan_step.outcome == "reset" and
      stale_route_state.obstruction_route == nil and
      stale_route_state.movement_bearing_degrees == 45 and
      stale_route_state.movement_current_bearing_degrees == 45,
      "failed obstruction replan clears the stale queue and returns to the live target [C: 0x14756]")
game.movement.probe = saved_movement_probe
game.movement.path_is_clear = saved_path_query

local heartbeat_actor_id = "playtest13_dynamic_hellhound"
local heartbeat_target = {
    id = "party.runner.0",
    world_activity_actor = false,
    projected_y_nonzero = true,
    actor_facts = actor_facts(),
    bearing_degrees = 337.5,
    distance = E.AiTuning.HELL_HOUND_PURSUIT_RANGE,
}
local heartbeat_context = movement_fixture(true)
heartbeat_context.actor_id = heartbeat_actor_id
heartbeat_context.profile_id = "hell_hound_k0c_none00"
heartbeat_context.visual_profile_id = E.ActorVisualProfile.HELL_HOUND
heartbeat_context.action_kind = "none"
heartbeat_context.action_code = E.ItemId.NONE
heartbeat_context.phase = "behavior"
heartbeat_context.spawn = true
heartbeat_context.initial_bearing_degrees = 337.5
heartbeat_context.entry_delay = 0
heartbeat_context.entry_animation_frame_count = 1
heartbeat_context.movement_allowed = true
heartbeat_context.actor_behavior_profile = {
    engagement_policy = "patrol", approach_band = "far",
}
heartbeat_context.source_record_slot = 7
heartbeat_context.ai_target_candidates = { heartbeat_target }
heartbeat_context.ai_random_bound = function() return 0 end
check(ActorAI.heartbeat(heartbeat_context) ~= nil and
      game.actors[heartbeat_actor_id].source_record_slot == nil,
      "actor state keeps stable identity without caching the source record position")
local heartbeat_state = game.actors[heartbeat_actor_id]
heartbeat_state.transition_complete = true
heartbeat_state.behavior_timer = 1
heartbeat_state.linked_target_id = heartbeat_target.id
heartbeat_state.movement_active = true
heartbeat_state.movement_bearing_degrees = 337.5
heartbeat_state.movement_current_bearing_degrees = 337.5
heartbeat_context.spawn = false
local source_think = R.think
R.think = function() return false end
local heartbeat_result = ActorAI.heartbeat(heartbeat_context)
R.think = source_think
check(heartbeat_result ~= nil and heartbeat_result.direction == 7 and
      heartbeat_state.movement_step ~= nil and
      heartbeat_state.animation_id == "walk" and
      heartbeat_state.movement_step.direction_animation == nil,
      "dynamic enemy heartbeat preserves semantic walking state and truncated direction 7 [C]")

heartbeat_context.live_field_actor = true
heartbeat_context.actor_facts = actor_facts({
    behavior = { escort = true },
})
heartbeat_state.pending_lifecycle_event =
    E.ActorLifecycleEvent.ESCORT_CONCLUDED
local escort_concluded = ActorAI.heartbeat(heartbeat_context)
check(escort_concluded ~= nil and
      heartbeat_state.pending_lifecycle_event == nil and
      heartbeat_state.actor_facts.lifecycle.active == false and
      heartbeat_state.actor_facts.behavior.escort == false,
      "escort conclusion applies as a semantic actor lifecycle transition")

game.movement.follow_route_step_clear = follow_route_step_clear

print(string.format("ai_think tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
