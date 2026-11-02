-- ROM capture regression for the 0x490A effect callbacks. Source timings and
-- terminal state come from the seeded Genesis Plus GX capture documented in
-- /workspace/shadowrun/out/progress_notes.md (#E1 Step 1).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
require("actor.init")

local E = require("enums")
local Combat = require("combat")
local Magic = require("magic")
local Actions = require("actor.actions")
game.modules.combat = Combat
game.modules.magic = Magic
game.modules.items = require("items")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        passed = passed + 1
    else
        print("FAIL " .. label)
    end
end

local function clear_table(value)
    for key in pairs(value) do
        value[key] = nil
    end
end

local function make_combatant(options)
    options = options or {}
    return {
        is_party_member = options.party == true,
        physical_meter = options.physical or 10,
        mental_meter = options.mental or 10,
        posture_reduction_percent = 0,
        force = options.force or 0,
        spell_trial_cap = 0,
        interaction_flags = options.interaction_flags or 0,
        blocks_spell_death_pose = options.blocks_spell_death_pose == true,
        attributes = { body = options.body or 1,
            willpower = options.willpower or 1,
            quickness = 1, strength = 1, intelligence = 1 },
        skills = { firearms = 0, pistols = 0, smgs = 0, shotguns = 0,
            melee_combat = 0, sorcery = 0 },
        cyberware = { datajack = false, cybereyes = false,
            hand_razors = false, spurs = false, smartlink = false,
            melee_strength_bonus = 0, melee_defense_bonus = 0 },
        combat_traits = { is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false },
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = true,
                ai_runner = options.party == true or options.ai_runner == true,
                downed = false,
                group_leader = false, dead = false },
            interaction = { no_coverage_difficulty_bonus = false,
                unseen = false, defense_difficulty_reduction = false },
        },
        inventory = {}, weapon_action_policies = {},
    }
end

local function run_callback(spell_id, options)
    options = options or {}
    clear_table(game.actors)
    clear_table(Actions.pending_spell_effects_by_actor_id)
    clear_table(Actions.spell_effect_scans_by_actor_id)

    local seed = 1
    local function random_bound(bound)
        seed = (seed * 0x20E5 + 1) & 0xFFFF -- ROM 0x0D44/0x0D3A
        return (seed * bound) >> 16
    end
    local target_id = "e1-target"
    local next_id = "e1-next"
    local caster_id = "e1-caster"
    local caster = make_combatant({ party = true, force = 2,
        body = 2, willpower = 4 })
    local target_combatant = make_combatant({ body = 4, willpower = 1,
        physical = 3, mental = 6,
        ai_runner = options.ai_runner,
        interaction_flags = options.interaction_flags,
        blocks_spell_death_pose = options.blocks_spell_death_pose })
    local target_state = { actor_id = target_id,
        combatant = target_combatant }
    game.actors[target_id] = target_state

    local target = { id = target_id, combatant = target_combatant,
        direct_route_step_cost = 0x0100,
        actor_facts = { lifecycle = { active = true, dead = false } } }
    local candidates = { target }
    local next_candidate
    if options.sleep_fanout then
        next_candidate = { id = next_id, direct_route_step_cost = 0x0100,
            actor_facts = { lifecycle = { active = true, dead = false } } }
        candidates[#candidates + 1] = next_candidate
        game.actors[next_id] = { actor_id = next_id }
    end

    local cast_context = {
        action_spell_id = spell_id,
        scene_id = E.Scene.STOKER_MOTEL,
        actor_facts = { lifecycle = { ai_runner = false } },
        ai_target_candidates = candidates,
    }
    Actions.queue_spell_effect({ actor_id = caster_id }, cast_context,
        target, { effect_started = true, spell_effect_slot = 3,
            successes = 4, combatant = caster })

    local context = {
        actor_id = target_id,
        scene_id = E.Scene.STOKER_MOTEL,
        phase = "render",
        combatant = target_combatant,
        ai_random_bound = random_bound,
        ai_target_candidates = candidates,
        final_battle_active = options.final_battle_active == true,
    }
    local heartbeats = 0
    while heartbeats < 100 do
        heartbeats = heartbeats + 1
        Actions.advance_spell_effect(target_state, context)
        if type(target_state.combatant) == "table" then
            context.combatant = target_state.combatant
        end
        if target_state.spell_effect_completed then
            break
        end
    end
    return {
        heartbeats = heartbeats,
        profile = Magic.actor_effect_profile(spell_id),
        state = target_state,
        combatant = target_state.combatant or target_combatant,
        seed = seed,
        next_pending = Actions.pending_spell_effects_by_actor_id[next_id],
        next_candidate = next_candidate,
    }
end

-- Source callback clocks/results: 0x521E code 1 = (12, 0/6),
-- 0x5152 code 2 direct clear = (31, 0/6, seed unchanged),
-- 0x52BE -> 0x5322 code 5 = (62, 3/0), 0x508A code 8 = (10, 0/6),
-- and 0x4FE2 code 9 = (31, 0/6). The full ROM measurements use meters 3/6.
local flame_dart = run_callback("flame_dart")
check(flame_dart.heartbeats == 12 and
      flame_dart.combatant.physical_meter == 0 and
      flame_dart.combatant.mental_meter == 6 and
      flame_dart.seed == 0x3E62,
      "0x521E code 1 matches the seeded 12-heartbeat physical effect")

local final_battle_clear = run_callback("flame_bolt", {
    final_battle_active = true,
})
check(final_battle_clear.heartbeats == 31 and
      final_battle_clear.combatant.physical_meter == 0 and
      final_battle_clear.combatant.mental_meter == 6 and
      final_battle_clear.seed == 1,
      "0x5152 code 2 direct-clear branch preserves the seeded RNG")
check(final_battle_clear.state.incoming_hit ~= nil and
      final_battle_clear.state.incoming_hit.spell_death_variant ==
          "spell_death_e0",
      "0x5152 direct-clear branch dispatches 0xB0AC for ordinary target")

local final_battle_flagged = run_callback("flame_bolt", {
    final_battle_active = true,
    ai_runner = true,
})
check(final_battle_flagged.state.spell_effect_resolution ~= nil and
      final_battle_flagged.seed ~= 1,
      "0x5152 calls 0x55B78 when target +0x10 bit 3 is set")

local final_battle_thon = run_callback("flame_bolt", {
    final_battle_active = true,
    blocks_spell_death_pose = true,
})
check(final_battle_thon.combatant.physical_meter == 0 and
      final_battle_thon.state.incoming_hit ~= nil and
      final_battle_thon.state.incoming_hit.spell_death_variant == nil,
      "0x51C8 code-2 type-15 branch uses 0xAFF2 without 0xB0AC pose")

local sleep = run_callback("sleep", { sleep_fanout = true })
check(sleep.heartbeats == 62 and sleep.combatant.physical_meter == 3 and
      sleep.combatant.mental_meter == 0 and
      sleep.state.spell_effect_resolution ~= nil,
      "0x52BE/0x5322 applies the seeded code-5 mental-meter effect")
check(type(sleep.next_pending) == "table" and
      sleep.next_pending.target_id == "e1-next" and
      sleep.next_pending.phase == 0 and sleep.next_pending.countdown == 5,
      "0x52CC scan queues the first active marked sleep target")

local mana_zap = run_callback("mana_zap")
check(mana_zap.profile ~= nil and mana_zap.heartbeats == 10 and
      mana_zap.combatant.physical_meter == 0 and
      mana_zap.combatant.mental_meter == 6 and
      mana_zap.seed == 0x20E6,
      "0x508A code 8 has a timed actor callback and physical effect")

local mana_blast = run_callback("mana_blast")
check(mana_blast.heartbeats == 31 and
      mana_blast.combatant.physical_meter == 0 and
      mana_blast.combatant.mental_meter == 6 and
      mana_blast.seed == 0x20E6,
      "0x4FE2 code 9 keeps the measured physical effect")

print(string.format("E1 spell effect callback tests: %d/%d passed",
    passed, total))
if passed ~= total then os.exit(1) end
