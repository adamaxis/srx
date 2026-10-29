-- ROM [C 0xAE7E/0xAEB4] enters 0x1033E with +0xA2 == 0. A1 survives from
-- caller/previous-record state into 0x55740; this suite checks the documented
-- native reading of that undefined defender profile and the 0x15830 split.
local E = require("enums")
game = {
    actors = {},
    modules = {
        combat = require("combat"),
        magic = require("magic"),
        items = require("items"),
    },
}
require("actor.init")
local R = require("actor.registry")

local SpellIdByActionCode = {
    [1] = "flame_dart", [2] = "flame_bolt", [3] = "hellblast",
    [4] = "invisibility", [5] = "sleep", [6] = "stink",
    [7] = "super_barrier", [8] = "mana_zap", [9] = "mana_blast",
    [10] = "mana_storm", [11] = "rockskin", [12] = "heal_wounds",
    [13] = "confusion", [14] = "barrier",
}

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local function caster()
    return {
        record_type = 0,
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
        mental_meter = 0,
        force = 4,
        posture_reduction_percent = 50,
        attributes = {
            body = 12, quickness = 0, strength = 0,
            intelligence = 0, willpower = 12,
        },
        skills = {
            sorcery = 12, firearms = 0, pistols = 0, smgs = 0,
            shotguns = 0, melee_combat = 0,
        },
        magic_cap = 20,
        action_profile_modifier_flags = 0,
        combat_traits = {
            ignores_cover_condition_penalty = true,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
        cyberware = {},
        inventory = {},
    }
end

local function make_context(action_code)
    local alternate = false
    local occupancy = {}
    for slot = 1, 18 do occupancy[slot] = false end
    return {
        actor_id = "field_player_runner",
        phase = "behavior",
        player_spell_available = true,
        player_spell_id = SpellIdByActionCode[action_code],
        combat_confirm_held = true,
        combat_action_ready = true,
        combat_input_blocked = false,
        world_scene_state = 0,
        source_record_slot = 0,
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
        ai_target_candidates = {},
        world_effect_slot_occupancy = occupancy,
        combatant = caster(),
        -- Sixes explode once, then a one makes the accumulated total meet
        -- every tested spell difficulty without consuming an unbounded stream.
        ai_random_bound = function(bound)
            if bound ~= 6 then return nil end
            alternate = not alternate
            return alternate and 5 or 0
        end,
        action_line_clear = function() error("untargeted cast probed a line") end,
    }
end

for action_code = E.SpellActionCode.FLAME_DART,
                    E.SpellActionCode.BARRIER do
    local state = { actor_id = "field_player_runner", action_delay = -1 }
    local context = make_context(action_code)
    local audio = R.begin_player_spell_cast(state, context)
    local result = state.spell_action_resolution
    local is_field_barrier =
        context.player_spell_id == "super_barrier" or
        context.player_spell_id == "barrier"
    check(state.source_action_dispatch == "player_spell" and
          type(result) == "table" and result.untargeted and
          result.target_id == "" and result.source_record_slot == nil,
          string.format("action %d enters 0x1033E without a target slot",
                        action_code))
    check(result ~= nil and result.roll_attempted and
          result.action_result > 0 and result.effect_requested and
          result.difficulty == 6 and
          result.behavior_meter_delta == 1 and
          result.combatant.mental_meter == 1,
          string.format("action %d uses the documented stale-A1 reading, rolls, and drains", action_code))
    if is_field_barrier then
        check(result.field_effect_kind == E.SpellEffectKind.FIELD_BARRIER and
              result.effect_started and result.spell_effect_slot == 0 and
              audio == E.SoundId.FIELD_3B and
              state.player_field_spell_effect ~= nil and
              state.player_field_spell_effect.remaining ==
                  result.action_result *
                      E.SpellTuning.FIELD_BARRIER_TICKS_PER_SUCCESS,
              string.format("action %d spawns 0x49A2 field barrier/audio",
                            action_code))
    else
        check(result.field_effect_kind == E.SpellEffectKind.NONE and
              not result.effect_started and result.spell_effect_slot == nil and
              audio == nil and state.player_field_spell_effect == nil,
              string.format("action %d does not invent a target effect",
                            action_code))
    end
    check(not result.special_result_published,
          string.format("action %d returns at 0x1584C before target/group effects",
                        action_code))
end

local expiring = {
    actor_id = "field_player_runner",
    player_field_spell_effect = {
        spell_id = "barrier",
        effect_slot = 4,
        remaining = 1,
    },
}
R.advance_spell_effect(expiring, {phase = "render"})
check(expiring.player_field_spell_effect == nil and
      expiring.spell_effect_completed and
      expiring.spell_effect_id == "barrier" and
      expiring.spell_effect_phase == 1 and
      expiring.spell_effect_slot_released == 4,
      "0x4ADC field effect terminal pass releases its shared slot")

print(string.format("player_spell_no_target: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
