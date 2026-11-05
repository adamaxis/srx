-- ROM 0x5A62/0x5A98 culls the projected entity; 0x13904 can retire an
-- off-window reinforcement through 0x1E514, including during an ordinary
-- down reaction. [C]
local H = dofile("decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
local E = require("enums")
require("actor.init")
local ActorAI = require("actor.heartbeat")
local R = require("actor.registry")
local Combat = require("combat")
local FieldAlarm = require("field_alarm")
game.modules.combat = Combat
game.modules.items = require("items")
game.modules.magic = require("magic")

local pass, total = 0, 0
local function check(condition, message)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL: " .. message) end
end

local function actor_facts()
    return {
        lifecycle = { active = true, thinking_enabled = true,
            ai_runner = false, downed = false, group_leader = false,
            dead = false },
        interaction = { projectile = false,
            proximity_interaction_pending = false,
            use_collision_heading = false, collision_recovery_active = false,
            action_hook_pending = false, defense_difficulty_reduction = false,
            breathing_out = false, unseen = false,
            no_coverage_difficulty_bonus = false },
        behavior = { route_clear = false, path_active = false,
            restart_steering = false, wall_following = false,
            in_target_band = false, route_transition = false,
            fleeing = false, escort = false },
        roles = { spotted = false, reinforcement = false,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false },
    }
end

local function target_combatant(physical)
    return {
        is_party_member = false, physical_meter = physical, mental_meter = 10,
        posture_reduction_percent = 50, equipped_action_id = "streetline",
        equipped_armor_id = "armor_vest", force = 0,
        attributes = { body = 5, quickness = 4, strength = 4,
            intelligence = 2, willpower = 2 },
        skills = { firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
            melee_combat = 0, sorcery = 0 },
        cyberware = { hand_razors = true, spurs = false,
            melee_strength_bonus = 0, melee_defense_bonus = 0 },
        combat_traits = { ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false, takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false },
        actor_facts = actor_facts(), inventory = {},
    }
end

local attacker = {
    is_party_member = true, physical_meter = 10, mental_meter = 10,
    posture_reduction_percent = 50, equipped_action_id = "american_l36",
    force = 0,
    attributes = { body = 2, quickness = 5, strength = 2,
        intelligence = 4, willpower = 3 },
    skills = { firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
        melee_combat = 0, sorcery = 0 },
    cyberware = { datajack = true },
    combat_traits = { ignores_cover_condition_penalty = false,
        adds_two_condition_difficulty = false,
        is_special_spell_target = false, takes_fixed_meter_damage = false,
        qualifies_for_party_support_bonus = false,
        is_ghoul_bounty_target = false },
    actor_facts = { lifecycle = { active = true, thinking_enabled = true,
        ai_runner = true, downed = false, group_leader = false,
        dead = false }, interaction = { no_coverage_difficulty_bonus = false,
        unseen = false, defense_difficulty_reduction = false } },
    inventory = {},
    weapon_action_policies = { american_l36 = { skill_group = "pistols",
        difficulty_adjustment = 0, benefits_from_targeting_assist = false } },
}

local function context_for(actor_id, visual_profile_id)
    local thon_actor = visual_profile_id == E.ActorVisualProfile.THON
    local context = H.context(actor_id)
    context.actor_id = actor_id
    context.profile_id = "weenr2_k08_item01"
    context.dynamic_world_encounter_actor = true
    context.visual_profile_id = visual_profile_id or
        E.ActorVisualProfile.YAKUZA_WEENR
    context.action_kind = thon_actor and "spell" or "item"
    context.action_code = thon_actor and E.SpellActionCode.FLAME_DART or
        E.ItemId.STREETLINE_SPECIAL
    context.action_item_id = thon_actor and nil or "streetline"
    context.action_spell_id = thon_actor and "flame_dart" or nil
    context.actor_facts = actor_facts()
    context.combatant = target_combatant(1)
    context.phase = "behavior"
    context.spawn = true
    context.initial_bearing_degrees = 0
    context.entry_delay = 0
    context.entry_animation_frame_count = 1
    context.live_field_actor = true
    context.source_record_slot = 6
    context.source_projection_active = true
    context.source_spawn_state_flags = 0
    context.source_interaction_flags = 0
    context.source_item_spawn_result = nil
    context.source_think_suppressed = false
    context.world_activity_active = false
    context.world_scene_state = 0
    context.movement_allowed = false
    context.movement_services_available = false
    context.ai_target_candidates = {}
    context.world_target_selection_candidates = {}
    context.combat_meter_service_blocked = false
    context.ai_random_bound = function() return 0 end
    return context
end

local function run_phase(context, phase, projected)
    local state = game.actors[context.actor_id]
    context.spawn = false
    context.phase = phase
    context.actor_facts = state.actor_facts
    context.combatant = state.combatant
    context.physical_condition = state.combatant.physical_meter
    context.source_projection_active = projected
    return ActorAI.heartbeat(context)
end

local actor_id = "offscreen-death-reinforcement"
local context = context_for(actor_id)
local spawned = ActorAI.heartbeat(context)
check(spawned ~= nil, "Yakuza Weenr source actor spawns")
local state = game.actors[actor_id]
state.actor_facts.roles.reinforcement = true
local alarm_result = FieldAlarm.clear({ alarm_active = true }, { state })
check(alarm_result ~= nil and
      state.actor_facts.roles.despawn_offscreen == true,
      "0x1E09A promotes the reinforcement to off-window retirement [C]")

-- The first service completes the entry delay; the next due 0x13904 think
-- starts +0xB6 at 0x14. The following off-window thought is due on pass 21.
local entered = run_phase(context, "behavior", true)
local thought = run_phase(context, "behavior", true)
check(entered ~= nil and thought ~= nil and
      state.behavior_timer == E.AiTuning.THINK_FRAMES,
      "source think period starts at E.AiTuning.THINK_FRAMES [C: 0x13904]")

local seed = 0x0437
local function source_random(bound)
    seed = (seed * 0x20E5 + 1) & 0xFFFF
    return (seed * bound) >> 16
end
local successes = Combat.resolve_weapon_attack_seed({
    combatant = attacker,
    projectile_action_id = "american_l36",
    ai_random_bound = source_random,
})
local hit = Combat.resolve_weapon_hit({
    projectile_action_id = "american_l36",
    ai_random_bound = source_random,
}, { attack_successes = successes }, {
    id = actor_id,
    combatant = state.combatant,
})
check(successes == 3 and hit ~= nil and hit.physical_meter == 0,
      "seed 0x0437 L36 hit is lethal to Weenr2 at physical 1 [C]")

-- ROM 0xAFF2 -> 0xB330 selects the ordinary down list. At 0x13904 the
-- off-window +0x9E.b2 check precedes the dead branch and calls 0x1E514.
local down = run_phase(context, "behavior", false)
state = game.actors[actor_id]
check(down ~= nil and state.animation_id == "down" and
      state.actor_facts.lifecycle.active,
      "off-window lethal hit enters ordinary down animation")
for _ = 1, 16 do
    local render = run_phase(context, "render", false)
    check(render ~= nil, "ordinary death render heartbeat is serviced")
end
local finished = run_phase(context, "behavior", false)
state = game.actors[actor_id]
check(finished ~= nil and state.damage_reaction_animation_complete and
      state.actor_facts.lifecycle.dead,
      "Weenr2 two-frame death list completes in 16 render updates [C: 0x8FC4]")

local retired_on_pass
for i = 1, 21 do
    local decision = run_phase(context, "behavior", false)
    check(decision ~= nil, "off-window actor service pass is handled")
    state = game.actors[actor_id]
    if not state.actor_facts.lifecycle.active then
        retired_on_pass = i
        break
    end
end
check(retired_on_pass == nil and
      state.actor_facts.lifecycle.active == true and
      state.actor_facts.lifecycle.dead == true,
      "0xA91C..0xA922 skips 0x13904 after an ordinary corpse is marked dead [C]")

-- The ordinary corpse remains an active record, but projection does not
-- restart its finished down list or replace its presentation.
local reentry = run_phase(context, "render", true)
state = game.actors[actor_id]
check(reentry ~= nil and state.actor_facts.lifecycle.active and
      state.actor_facts.lifecycle.dead and state.animation_id == "down",
      "re-entry leaves the ordinary active corpse down [C: 0xAB02]")

local function copy_facts(source)
    local result = {}
    for group, values in pairs(source) do
        result[group] = {}
        for name, value in pairs(values) do
            result[group][name] = value
        end
    end
    return result
end

local function copy_state(source)
    local result = {}
    for name, value in pairs(source) do result[name] = value end
    result.actor_facts = copy_facts(source.actor_facts)
    return result
end

-- [C] ROM 0x1394A/0x1397A reaches 0x13E8C; projected +0x0C == 0 then
-- enters 0x13E40, which retains or refreshes the B45C continuation.
local think_path_cases = {
    {
        name = "continue_movement", expected = "continue_movement",
        projection = false,
        configure = function(s, c)
            c.world_activity_active = true
            c.field_sequence_active = true
            c.hold_for_ending_activity = true
        end,
    },
    {
        name = "watch", expected = "watch", projection = true,
        configure = function(_, c)
            c.actor_behavior_profile = {
                engagement_policy = "watch_nearby_runner",
            }
            c.ai_target_candidates = {
                { id = "party.runner.0", world_activity_actor = false,
                  projected_y_nonzero = true, bearing_degrees = 90,
                  distance = E.AiTuning.WATCH_DIST - 1,
                  actor_facts = actor_facts() },
            }
        end,
    },
    {
        name = "patrol", expected = "patrol", projection = false,
        configure = function() end,
    },
    {
        name = "thon_continue_movement",
        expected = "continue_movement", projection = true,
        visual_profile = E.ActorVisualProfile.THON,
        configure = function(_, c)
            c.animation_blocks_behavior = false
            c.thon_action_cooldown_remaining = 1
        end,
    },
    {
        name = "flee", expected = "flee", projection = true,
        configure = function(s)
            s.actor_facts.behavior.fleeing = true
            s.flee_target_x, s.flee_target_y = 0x3000, 0x3000
        end,
    },
    {
        name = "follow_leader", expected = "follow_leader",
        projection = false,
        configure = function(s)
            s.actor_facts.behavior.escort = true
        end,
    },
    {
        name = "skip", expected = "skip", projection = true,
        configure = function(s)
            s.actor_facts.roles.quest = true
        end,
    },
    {
        name = "combat", expected = "combat", projection = true,
        configure = function(s, c)
            s.actor_facts.roles.hostile = true
            c.world_activity_active = true
        end,
    },
    {
        name = "thon_busy", expected = "thon_busy", projection = true,
        visual_profile = E.ActorVisualProfile.THON,
        configure = function(_, c)
            c.animation_blocks_behavior = true
            c.thon_action_cooldown_remaining = 1
        end,
    },
    {
        name = "thon_action", expected = "thon_action", projection = true,
        down_expected = "thon_busy",
        visual_profile = E.ActorVisualProfile.THON,
        configure = function(_, c)
            c.animation_blocks_behavior = false
            c.thon_action_cooldown_remaining = 0
            c.ai_target_candidates = {
                { id = "party.runner.0", world_activity_actor = false,
                  projected_y_nonzero = true, bearing_degrees = 90,
                  world_x = 0x2000, world_y = 0x2400, distance = 0x300,
                  actor_facts = actor_facts() },
            }
            c.ai_random_bound = function() return 0 end
            c.action_line_clear = function() return true end
        end,
    },
}

local original_think = R.think
local think_calls = 0
R.think = function(...)
    think_calls = think_calls + 1
    return original_think(...)
end

local function prepare_held_actor(case, lifecycle_dead, phase_name)
    local held_id = "offscreen-death-" .. phase_name .. "-" .. case.name
    local held_context = context_for(held_id, case.visual_profile)
    local spawned_held = ActorAI.heartbeat(held_context)
    local held_state = game.actors[held_id]
    check(spawned_held ~= nil and held_state ~= nil,
          phase_name .. "/" .. case.name .. " actor setup")
    if held_state == nil then return end

    held_state.damage_reaction_active = true
    held_state.animation_id = "down"
    held_state.animation_frame = 1
    held_state.animation_progress = 0.25
    held_state.damage_reaction_animation_complete = lifecycle_dead
    held_state.actor_facts.lifecycle.downed = true
    held_state.actor_facts.lifecycle.dead = lifecycle_dead
    held_state.actor_facts.roles.despawn_offscreen = false
    held_state.behavior_timer = 0
    held_state.movement_active = true
    held_state.movement_bearing_degrees = 90
    held_state.movement_route_distance = 123
    held_state.movement_current_bearing_degrees = 45
    held_state.flee_target_x = 0x3000
    held_state.flee_target_y = 0x3000
    held_state.ai_route_target = { x = 0x3100, y = 0x3200 }
    held_state.ai_target_id = "party.runner.0"
    held_state.ai_target_bearing_degrees = 22.5
    held_state.ai_target_distance = 0x300
    held_state.combat_action_requested = false
    held_state.spell_action_resolution = nil
    held_state.ai_think_audio_event = nil

    held_context.spawn = false
    held_context.phase = "behavior"
    held_context.actor_facts = held_state.actor_facts
    held_context.combatant = held_state.combatant
    held_context.physical_condition = 0
    held_context.source_think_suppressed = false
    held_context.source_projection_active = case.projection
    held_context.animation_blocks_behavior = not lifecycle_dead
    held_context.world_activity_active = false
    held_context.field_sequence_active = false
    held_context.hold_for_ending_activity = false
    held_context.actor_behavior_profile = {
        engagement_policy = "patrol",
    }
    held_context.controlled_party_member_id = "party.runner.0"
    held_context.ai_target_candidates = {}
    held_context.source_camera_x = 0x1000
    held_context.source_camera_y = 0x1000
    held_context.source_camera_limit_x = 0x8000
    held_context.source_camera_limit_y = 0x8000
    held_context.measure_destination = function()
        return { bearing_degrees = 180, distance = 0x400, step_cost = 0x400 }
    end
    case.configure(held_state, held_context)
    held_context.actor_facts = held_state.actor_facts

    local gate_state = copy_state(held_state)
    gate_state.damage_reaction_active = false
    gate_state.animation_id = "idle"
    local gate_context = {
        visual_profile_id = case.visual_profile or
            E.ActorVisualProfile.YAKUZA_WEENR,
        actor_facts = gate_state.actor_facts,
        source_projection_active = case.projection,
        source_think_suppressed = false,
        source_alarm_active = false,
        world_activity_active = held_context.world_activity_active,
        field_sequence_active = held_context.field_sequence_active,
        hold_for_ending_activity = held_context.hold_for_ending_activity,
        actor_behavior_profile = held_context.actor_behavior_profile,
        animation_blocks_behavior = held_context.animation_blocks_behavior,
        thon_action_cooldown_remaining =
            held_context.thon_action_cooldown_remaining,
        controlled_party_member_id = "party.runner.0",
        ai_target_candidates = held_context.ai_target_candidates,
        ai_random_bound = held_context.ai_random_bound,
        action_line_clear = held_context.action_line_clear,
        combat_meter_service_blocked = false,
    }
    local selected_path = R.think_gate(gate_state, gate_context)
    check(selected_path == case.expected,
          phase_name .. "/" .. case.name .. " source path precondition")

    local before = {
        animation_id = held_state.animation_id,
        animation_frame = held_state.animation_frame,
        animation_progress = held_state.animation_progress,
        movement_active = held_state.movement_active,
        movement_bearing_degrees = held_state.movement_bearing_degrees,
        movement_route_distance = held_state.movement_route_distance,
        movement_current_bearing_degrees =
            held_state.movement_current_bearing_degrees,
        movement_step = held_state.movement_step,
        route_target = held_state.ai_route_target,
        ai_target_id = held_state.ai_target_id,
        ai_target_bearing_degrees = held_state.ai_target_bearing_degrees,
        ai_target_distance = held_state.ai_target_distance,
        behavior_timer = held_state.behavior_timer,
        think_path = held_state.ai_think_path,
        x = 0x1000, y = 0x2000,
        dead = held_state.actor_facts.lifecycle.dead,
        action_dispatch = held_state.source_action_dispatch,
        action_animation_complete = held_state.action_animation_complete,
        item_action_complete = held_state.item_action_complete,
        pending_source_action = held_state.pending_source_action,
        last_action_successes = held_state.last_action_successes,
    }
    held_context.world_position = { x = before.x, y = before.y }
    local calls_before = think_calls
    local decision = ActorAI.heartbeat(held_context)
    local expected_runtime_path
    if lifecycle_dead then
        expected_runtime_path = before.think_path
    else
        expected_runtime_path = case.down_expected or case.expected
    end
    -- The live +0xDC cooldown may count down at 0xA906 during the actor
    -- service; assertions below concern new actions and movement only.
    check(decision ~= nil and decision.action == "hold" and
          decision.audio_event == nil and
          decision.target_heading_audio_event == nil and
          decision.projectile_movement == nil and
          think_calls == calls_before + (lifecycle_dead and 0 or 1) and
          held_state.ai_think_path == expected_runtime_path and
          held_state.animation_id == "down" and
          held_state.animation_frame == before.animation_frame and
          held_state.animation_progress == before.animation_progress and
          held_state.movement_active == before.movement_active and
          held_state.movement_bearing_degrees == before.movement_bearing_degrees and
          held_state.movement_route_distance == before.movement_route_distance and
          held_state.movement_current_bearing_degrees ==
              before.movement_current_bearing_degrees and
          held_state.movement_step == before.movement_step and
          held_state.ai_route_target == before.route_target and
          held_state.ai_target_id == before.ai_target_id and
          held_state.ai_target_bearing_degrees == before.ai_target_bearing_degrees and
          held_state.ai_target_distance == before.ai_target_distance and
          held_context.world_position.x == before.x and
          held_context.world_position.y == before.y and
          held_state.behavior_timer ==
              (lifecycle_dead and before.behavior_timer or
               E.AiTuning.THINK_FRAMES) and
          held_state.actor_facts.lifecycle.dead == before.dead and
          held_state.actor_facts.lifecycle.active and
          held_state.combat_action_requested == false and
          held_state.spell_action_resolution == nil and
          held_state.ai_think_audio_event == nil and
          held_state.source_action_dispatch == before.action_dispatch and
          held_state.action_animation_complete ==
              before.action_animation_complete and
          held_state.item_action_complete == before.item_action_complete and
          held_state.pending_source_action == before.pending_source_action and
          held_state.last_action_successes == before.last_action_successes,
          phase_name .. "/" .. case.name .. " preserves down/dead state and starts no action/audio")
end

for _, case in ipairs(think_path_cases) do
    prepare_held_actor(case, false, "down")
    prepare_held_actor(case, true, "dead")
end
R.think = original_think

-- A promoted off-screen record is retired at the due thought while still
-- down. Ordinary actors after 0xAB02 keep their active corpse state.
local retiring_id = "offscreen-death-retirement-during-down"
local retiring_context = context_for(retiring_id)
local retiring_spawn = ActorAI.heartbeat(retiring_context)
local retiring_state = game.actors[retiring_id]
check(retiring_spawn ~= nil and retiring_state ~= nil,
      "offscreen retirement actor setup")
retiring_state.actor_facts.roles.reinforcement = true
FieldAlarm.clear({ alarm_active = true }, { retiring_state })
retiring_state.damage_reaction_active = true
retiring_state.damage_reaction_animation_complete = false
retiring_state.animation_id = "down"
retiring_state.animation_frame = 1
retiring_state.animation_progress = 0.25
retiring_state.actor_facts.lifecycle.downed = true
retiring_state.actor_facts.lifecycle.dead = false
retiring_state.behavior_timer = E.AiTuning.THINK_FRAMES
retiring_state.movement_active = true
retiring_state.movement_bearing_degrees = 90
retiring_state.movement_route_distance = 123
retiring_state.movement_current_bearing_degrees = 45
retiring_state.ai_route_target = { x = 0x3100, y = 0x3200 }
retiring_context.spawn = false
retiring_context.phase = "behavior"
retiring_context.actor_facts = retiring_state.actor_facts
retiring_context.combatant = retiring_state.combatant
retiring_context.source_projection_active = false
retiring_context.source_think_suppressed = false
retiring_context.physical_condition = 0
retiring_context.animation_blocks_behavior = true
retiring_context.world_position = { x = 0x2000, y = 0x2400 }
local down_retirement_pass
local retirement_decision
for i = 1, E.AiTuning.THINK_FRAMES + 1 do
    retirement_decision = ActorAI.heartbeat(retiring_context)
    check(retirement_decision ~= nil,
          "downed off-screen service pass is handled")
    if not retiring_state.actor_facts.lifecycle.active then
        down_retirement_pass = i
        break
    end
end
check(down_retirement_pass == E.AiTuning.THINK_FRAMES + 1 and
      retiring_state.actor_facts.lifecycle.dead == false and
      retiring_state.animation_id == "down" and
      not retiring_state.actor_facts.lifecycle.active and
      retiring_state.movement_active and
      retiring_state.movement_bearing_degrees == 90 and
      retiring_state.movement_route_distance == 123 and
      retiring_state.movement_current_bearing_degrees == 45 and
      retiring_state.ai_route_target.x == 0x3100 and
      retiring_state.ai_route_target.y == 0x3200 and
      retiring_context.world_position.x == 0x2000 and
      retiring_context.world_position.y == 0x2400 and
      retirement_decision.audio_event == nil and
      retiring_state.spell_action_resolution == nil and
      retiring_state.ai_think_audio_event == nil,
      "0x13962 retires the promoted off-screen record without movement or action [C]")
local retired_render = ActorAI.heartbeat({
    actor_id = retiring_id,
    profile_id = retiring_context.profile_id,
    dynamic_world_encounter_actor = true,
    visual_profile_id = retiring_context.visual_profile_id,
    action_kind = retiring_context.action_kind,
    action_code = retiring_context.action_code,
    action_item_id = retiring_context.action_item_id,
    actor_facts = retiring_state.actor_facts,
    phase = "render",
    spawn = false,
    initial_bearing_degrees = retiring_context.initial_bearing_degrees,
    entry_delay = retiring_context.entry_delay,
    source_projection_active = true,
    source_think_suppressed = false,
})
check(retired_render == nil and
      not retiring_state.actor_facts.lifecycle.active and
      retiring_state.animation_id == "down",
      "a source-retired downed actor cannot re-enter [C: 0x1E514]")

print(string.format("offscreen_death tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
