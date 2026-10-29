-- ROM 0x14C7C -> 0xB11C: the close, delayed Hell Hound path resolves
-- 0x55B16 and requests the 0xB146 child only after a defender byte changes.
local E = require("enums")
game = { actors = {}, modules = { combat = require("combat"), items = { catalog = {} }, magic = {} } }
require("actor.init")
local R = require("actor.registry")

local function combatant(record_type, physical_meter)
    return {
        record_type = record_type,
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
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
        },
        action_profile_options = { primary = false, secondary = false },
        physical_meter = physical_meter,
        mental_meter = physical_meter,
        cyberware = {},
        posture_reduction_percent = 0,
        attributes = {
            body = 1, quickness = 0, strength = 0,
            intelligence = 0, willpower = 0,
        },
        skills = {
            sorcery = 0, firearms = 0, pistols = 0, smgs = 0,
            shotguns = 0, melee_combat = 0,
        },
        inventory = {},
    }
end

local function strike_fixture(target_meter)
    game.actors = {}
    local context = {
        entity_kind = E.SpriteType.HELL_HOUND,
        dynamic_world_encounter_actor = true,
        movement_allowed = true,
        ai_random_bound = function() return 0 end,
        combatant = combatant(E.ActorPool.OTHERS, 10),
        action_timing = { quickness = 0, intelligence = 0, modifier = 0 },
        world_effect_slot_occupancy = {},
    }
    local state = {
        entity_kind = E.SpriteType.HELL_HOUND,
        action_delay = -1,
        target_motion_effect_active = false,
        movement_current_heading = 0,
    }
    local target = {
        id = "party.runner.0",
        source_record_slot = 0,
        heading = E.AiTuning.QUARTER_TURN,
        distance = E.AiTuning.HELL_HOUND_CLOSE_RANGE - 1,
        world_x = 4660,
        world_y = 2748,
        combatant = combatant(E.ActorPool.PARTY - 1, target_meter),
    }
    R.resolve_source_target_motion(context, state, target)
    return state, target
end

local changed_state, target = strike_fixture(5)
assert(changed_state.combat_meter_update ~= nil and
       changed_state.combat_meter_update.changed == true and
       changed_state.hit_spark_spawn_request ~= nil and
       changed_state.hit_spark_spawn_request.target_id == target.id and
       changed_state.hit_spark_spawn_request.world_x == target.world_x and
       changed_state.hit_spark_spawn_request.world_y == target.world_y,
       "changed 0xB11C action requests the target-position child")

local unchanged_state = strike_fixture(0)
assert(unchanged_state.combat_meter_update ~= nil and
       unchanged_state.combat_meter_update.changed == false and
       unchanged_state.hit_spark_spawn_request == nil,
       "unchanged 0xB11C action does not request a child")

print("actor_spawn_hit_spark_tests: 2/2 passed")
