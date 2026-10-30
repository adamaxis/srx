-- ROM 0xB1DC/0xB29E: the player allocates a hit spark only after an in-range
-- 0x55C76 reaction changes the linked defender's +0x4C or +0x9C meter.
local E = require("enums")
local Combat = require("combat")
game = {
    actors = {},
    modules = {
        combat = Combat,
        items = { catalog = {} },
        magic = {},
    },
}
local ActorAI = require("actor.init")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function combatant(is_party_member)
    return {
        is_party_member = is_party_member,
        blocks_spell_death_pose = false,
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = false,
                ai_runner = false, downed = false, group_leader = false,
                dead = false },
            interaction = { projectile = false,
                proximity_interaction_pending = false,
                use_collision_heading = false, collision_recovery_active = false,
                action_hook_pending = false, defense_difficulty_reduction = false,
                breathing_out = false, unseen = true,
                no_coverage_difficulty_bonus = true },
        },
        physical_meter = 10,
        mental_meter = 10,
        force = 0,
        posture_reduction_percent = 0,
        attributes = {
            body = 0, quickness = 0, strength = 4,
            intelligence = 0, willpower = 0,
        },
        skills = {
            sorcery = 0, firearms = 0, pistols = 0, smgs = 0,
            shotguns = 0, melee_combat = 1,
        },
        combat_traits = {
            ignores_cover_condition_penalty = true,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
        equipped_action_id = "test_weapon",
        cyberware = {},
        action_profile_options = { primary = false, secondary = false },
        inventory = {},
    }
end

local attacker = combatant(true)
local random_calls = 0
local function successful_reaction_roll(bound)
    if bound ~= 6 then return nil end
    random_calls = random_calls + 1
    return random_calls % 2 == 1 and 5 or 0
end

local target = {
    id = "field_actor_slot_3",
    source_record_slot = 3,
    world_x = 4660,
    world_y = 2748,
    bearing_degrees = 90,
    distance = 256,
    actor_facts = {
        lifecycle = { active = true, dead = false, downed = false,
            ai_runner = false, thinking_enabled = false,
            group_leader = false },
        behavior = { escort = false },
        roles = { pedestrian = false, quest = false },
        interaction = { unseen = false },
    },
    world_activity_actor = true,
    has_action = true,
    combatant = combatant(false),
}

local context = {
    actor_id = "field_player_runner",
    profile_id = "field_runner_k03_none00",
    visual_profile_id = E.ActorVisualProfile.FIELD_RUNNER,
    action_kind = "none",
    action_code = E.ItemId.NONE,
    entry_delay = 0,
    initial_bearing_degrees = 0,
    actor_facts = {
        lifecycle = { active = true, thinking_enabled = false,
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
    },
    spawn = true,
    phase = "behavior",
    player_melee_available = true,
    combat_action_available = true,
    combat_confirm_held = true,
    combat_action_ready = true,
    combat_input_blocked = false,
    combatant = attacker,
    movement_actor_facts = {
        lifecycle = { active = true, thinking_enabled = false,
            ai_runner = false, downed = false, group_leader = false,
            dead = false },
    },
    ai_random_bound = successful_reaction_roll,
    ai_target_candidates = { target },
}

ActorAI.heartbeat(context)
context.spawn = false
local strike_hit = ActorAI.heartbeat(context)
check(strike_hit ~= nil and
      strike_hit.source_action_dispatch == "player_melee_hit" and
      strike_hit.combat_meter_update ~= nil and
      strike_hit.combat_meter_update.changed and
      strike_hit.hit_spark_spawn_request ~= nil and
      strike_hit.hit_spark_spawn_request.target_id == target.id and
      strike_hit.hit_spark_spawn_request.world_x == target.world_x and
      strike_hit.hit_spark_spawn_request.world_y == target.world_y,
      "changed in-range player hit requests a spark at the defender")

target.distance = E.HitSpark.RANGE_LIMIT
local strike_out_of_range = ActorAI.heartbeat(context)
check(strike_out_of_range ~= nil and
      strike_out_of_range.source_action_dispatch == "player_melee" and
      strike_out_of_range.hit_spark_spawn_request == nil,
      "out-of-range player swing does not request a spark")

target.distance = 256
target.actor_facts.lifecycle.dead = true
local strike_dead_target = ActorAI.heartbeat(context)
check(strike_dead_target ~= nil and
      strike_dead_target.source_action_dispatch == "player_melee" and
      strike_dead_target.hit_spark_spawn_request == nil,
      "dead linked record does not request a spark")

target.actor_facts.lifecycle.dead = false
context.ai_random_bound = function() return nil end
local strike_unchanged = ActorAI.heartbeat(context)
check(strike_unchanged ~= nil and
      strike_unchanged.source_action_dispatch == "player_melee_hit" and
      strike_unchanged.combat_meter_update == nil and
      strike_unchanged.hit_spark_spawn_request == nil,
      "in-range strike without a resolved meter change does not request a spark")

print(string.format("player_hit_spark_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
