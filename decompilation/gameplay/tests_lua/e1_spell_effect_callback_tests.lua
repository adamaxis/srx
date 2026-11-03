-- ROM capture regression for the 0x490A effect callbacks. Source timings and
-- terminal state come from the seeded Genesis Plus GX capture documented in
-- /workspace/shadowrun/out/progress_notes.md (#E1/#E1b Step 1).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
require("actor.init")

local E = require("enums")
local Combat = require("combat")
local Magic = require("magic")
local Actions = require("actor.actions")
local Targeting = require("actor.registry")
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

local effect_fixture = {}
local fixture_file = io.open(ROOT ..
    "/decompilation/gameplay/tests_lua/fixtures/e1_spell_effect_callbacks_gpgx.tsv",
    "r")
check(fixture_file ~= nil, "open the measured E1b GPGX fixture")
if fixture_file ~= nil then
    for line in fixture_file:lines() do
        if line ~= "" and line:sub(1, 1) ~= "#" and
           line:sub(1, 5) ~= "case\t" then
            local fields = {}
            for field in (line .. "\t"):gmatch("(.-)\t") do
                fields[#fields + 1] = field
            end
            effect_fixture[fields[1]] = {
                entry = fields[2],
                heartbeats = tonumber(fields[3]),
                seed_before = tonumber(fields[4], 16),
                seed_terminal = tonumber(fields[5], 16),
                seed_after = tonumber(fields[6], 16),
                physical_before = tonumber(fields[7]),
                physical_after = tonumber(fields[8]),
                mental_before = tonumber(fields[9]),
                mental_after = tonumber(fields[10]),
                details = fields[13],
                selected_slots = fields[14],
                rng_calls = fields[15],
            }
        end
    end
    fixture_file:close()
end

local function fixture_row(case)
    local row = effect_fixture[case]
    check(type(row) == "table", "fixture includes " .. case)
    return row or {}
end

local function matches_fixture(result, row, compare_seed)
    return result.heartbeats == row.heartbeats and
           result.combatant.physical_meter == row.physical_after and
           result.combatant.mental_meter == row.mental_after and
           (not compare_seed or result.seed == row.seed_after)
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

    local seed = options.seed or 1
    local rng_calls = {}
    local function random_bound(bound)
        seed = (seed * 0x20E5 + 1) & 0xFFFF -- ROM 0x0D44/0x0D3A
        local result = (seed * bound) >> 16
        rng_calls[#rng_calls + 1] = {
            bound = bound,
            result = result,
            seed_after = seed,
        }
        return result
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
        direct_route_step_cost = options.target_route_step_cost or 0x0100,
        actor_facts = { lifecycle = { active = true, dead = false } } }
    local candidates = { target }
    local next_candidate
    local candidate_states = {}
    local sleep_targets = options.sleep_targets
    if sleep_targets == nil and options.sleep_fanout then
        sleep_targets = { { id = next_id, direct_route_step_cost = 0x0100 } }
    end
    for index, spec in ipairs(sleep_targets or {}) do
        local candidate_id = spec.id or
            (index == 1 and next_id or (next_id .. "-" .. index))
        local lifecycle = {
            active = spec.active ~= false,
            dead = spec.dead == true,
            thinking_enabled = spec.thinking_enabled == true,
            ai_runner = false,
            downed = false,
            group_leader = false,
        }
        local combatant = make_combatant({ physical = 3, mental = 6 })
        combatant.actor_facts.lifecycle = lifecycle
        local candidate_state = {
            actor_id = candidate_id,
            combatant = combatant,
            actor_facts = { lifecycle = lifecycle },
        }
        local candidate = {
            id = candidate_id,
            direct_route_step_cost = spec.direct_route_step_cost or 0x0100,
            actor_facts = { lifecycle = lifecycle },
            combatant = combatant,
            source_record_slot = spec.source_record_slot,
        }
        candidates[#candidates + 1] = candidate
        candidate_states[candidate_id] = candidate_state
        game.actors[candidate_id] = candidate_state
        if index == 1 then next_candidate = candidate end
    end

    if options.reselect_single_route_target == true then
        -- ROM 0x144CA -> 0x0D3A consumes one source RNG step when an active
        -- actor has lost its retained target, even with one route candidate.
        -- The production Lua equivalent is actor/targeting.lua::linked_target.
        local route_state = {}
        local route_candidate = {
            id = "e1-route-player",
            projected_y_nonzero = true,
            world_activity_actor = false,
            actor_facts = {
                lifecycle = { active = true, dead = false, downed = false },
            },
        }
        local linked = Targeting.linked_target({
            actor_facts = { lifecycle = { ai_runner = false } },
            ai_target_candidates = { route_candidate },
            ai_random_bound = random_bound,
        }, route_state)
        if linked ~= route_candidate then
            route_state.linked_target_id = nil
        end
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
        if options.deactivate_candidate_at_heartbeat == heartbeats then
            local candidate = candidate_states[
                options.deactivate_candidate_id or next_id]
            if candidate ~= nil and type(candidate.actor_facts) == "table" then
                candidate.actor_facts.lifecycle.active = false
                candidate.actor_facts.lifecycle.dead = true
            end
        end
        Actions.advance_spell_effect(target_state, context)
        if type(target_state.combatant) == "table" then
            context.combatant = target_state.combatant
        end
        if target_state.spell_effect_completed then
            break
        end
    end
    local next_pending
    for index = 2, #candidates do
        local candidate_pending = Actions.pending_spell_effects_by_actor_id[
            candidates[index].id]
        if candidate_pending ~= nil then
            next_pending = candidate_pending
            break
        end
    end
    return {
        heartbeats = heartbeats,
        profile = Magic.actor_effect_profile(spell_id),
        state = target_state,
        combatant = target_state.combatant or target_combatant,
        seed = seed,
        rng_calls = rng_calls,
        next_pending = next_pending,
        next_candidate = next_candidate,
        candidates = candidates,
        candidate_states = candidate_states,
        target_state = target_state,
        target_context = context,
    }
end

local function service_pending_target(result, actor_id, maximum_heartbeats)
    local state = result.candidate_states[actor_id]
    local context = {
        actor_id = actor_id,
        scene_id = E.Scene.STOKER_MOTEL,
        phase = "render",
        combatant = state.combatant,
        ai_random_bound = result.target_context.ai_random_bound,
        ai_target_candidates = result.candidates,
    }
    local heartbeats = 0
    while heartbeats < maximum_heartbeats do
        heartbeats = heartbeats + 1
        Actions.advance_spell_effect(state, context)
        if state.spell_effect_completed then break end
    end
    return state, context, heartbeats
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
local code2_resistance_source = fixture_row("code2_bit3_resistance")
check(final_battle_flagged.state.spell_effect_resolution ~= nil and
      final_battle_flagged.seed ~= 1,
      "0x5152 calls 0x55B78 when target +0x10 bit 3 is set")
check(matches_fixture(final_battle_flagged, code2_resistance_source, true),
      "0x5152 bit-3 arm matches measured ROM heartbeats, meters, and RNG")
check(#final_battle_flagged.rng_calls == 5 and
      final_battle_flagged.rng_calls[1].bound == 6 and
      final_battle_flagged.rng_calls[5].bound == 6,
      "0x55B78 performs the five measured bound-6 rolls")

local code2_type15_source = fixture_row("code2_type15_pose_suppression")
-- [C] The seeded ROM source also logs a pre-terminal RNG draw at 0x103B2
-- (0x1033E's first-aid resolver); start at its measured terminal seed so this
-- case compares the 0x5152 callback's own 0x55B78 work. Type 15 then takes
-- 0x51E4 -> 0xAFF2, which preserves the measured no-death-pose outcome.
local final_battle_thon = run_callback("flame_bolt", {
    seed = code2_type15_source.seed_terminal,
    blocks_spell_death_pose = true,
})
check(final_battle_thon.combatant.physical_meter == 0 and
      final_battle_thon.state.incoming_hit ~= nil and
      final_battle_thon.state.incoming_hit.spell_death_variant == nil,
      "0x51C8 code-2 type-15 branch uses 0xAFF2 without 0xB0AC pose")
check(matches_fixture(final_battle_thon, code2_type15_source, true),
      "0x51C8 type-15 arm matches measured ROM heartbeats, meters, and RNG")
local source_type15_d6_count = select(2,
    code2_type15_source.rng_calls:gsub('""caller"":""0559CE""', ""))
check(#final_battle_thon.rng_calls == 5 and source_type15_d6_count == 5 and
      final_battle_thon.rng_calls[1].bound == 6 and
      final_battle_thon.rng_calls[5].bound == 6,
      "type-15 source and Lua callback each draw five d6 at 0x559CE")

local sleep_source = fixture_row("code5_one_marked")
local sleep = run_callback("sleep", {
    sleep_fanout = true,
    seed = sleep_source.seed_before,
})
check(sleep.heartbeats == 62 and sleep.combatant.physical_meter == 3 and
      sleep.combatant.mental_meter == 0 and
      sleep.state.spell_effect_resolution ~= nil,
      "0x52BE/0x5322 applies the seeded code-5 mental-meter effect")
check(matches_fixture(sleep, sleep_source, true),
      "0x52BE/0x5322 matches measured ROM timing, meters, and RNG seed")
check(type(sleep.next_pending) == "table" and
      sleep.next_pending.target_id == "e1-next" and
      sleep.next_pending.phase == 0 and sleep.next_pending.countdown == 5,
      "0x52CC scan queues the first active marked sleep target")
check(sleep_source.selected_slots == "7",
      "the one-marked ROM fixture transfers source slot 7")
check(type(sleep_source.rng_calls) == "string" and
      sleep_source.rng_calls:find('""caller"":""0559CE""', 1, true) ~= nil and
      sleep_source.rng_calls:find('""bound"":6', 1, true) ~= nil,
      "0x55B78 consumes its measured d6 at source caller 0x559CE")

local sleep_zero_source = fixture_row("code5_zero_marked")
local sleep_zero = run_callback("sleep", { seed = sleep_zero_source.seed_before })
check(matches_fixture(sleep_zero, sleep_zero_source, true) and
      sleep_zero.next_pending == nil and sleep_zero_source.selected_slots == "",
      "0x5322 leaves the fan-out empty when no other record is marked")

local sleep_several_source = fixture_row("code5_several_marked")
local sleep_several = run_callback("sleep", {
    -- The source row consumes bound-1 at 0x144CA -> 0x0D3A while relinking
    -- the actor's sole route target, then the code-5 callback draws at 0x559CE.
    -- Exercise the production Lua targeting path before the effect callback.
    seed = sleep_several_source.seed_before,
    reselect_single_route_target = true,
    sleep_targets = {
        { id = "sleep-slot-7", source_record_slot = 7,
            direct_route_step_cost = 0x0100 },
        { id = "sleep-slot-8", source_record_slot = 8,
            direct_route_step_cost = 0x0300 },
    },
})
check(matches_fixture(sleep_several, sleep_several_source, true) and
      sleep_several.next_pending ~= nil and
      sleep_several.next_pending.target_id == "sleep-slot-7" and
      sleep_several_source.selected_slots == "7",
      "0x5322 selects the first source-order record among several marks")
check(#sleep_several.rng_calls == 2 and
      sleep_several.rng_calls[1].bound == 1 and
      sleep_several.rng_calls[1].seed_after == sleep_several_source.seed_terminal and
      sleep_several.rng_calls[2].bound == 6 and
      sleep_several.rng_calls[2].seed_after == sleep_several_source.seed_after and
      sleep_several_source.rng_calls:find('""immediate_caller"":""0144D0""', 1, true) ~= nil,
      "source 0x144CA route relink and 0x559CE code-5 d6 both match Lua RNG")
local second_sleep_state = service_pending_target(
    sleep_several, "sleep-slot-7", 62)
local second_sleep_pending = Actions.pending_spell_effects_by_actor_id[
    "sleep-slot-8"]
check(second_sleep_state.spell_effect_completed and
      second_sleep_pending ~= nil and
      second_sleep_pending.target_id == "sleep-slot-8" and
      sleep_several_source.details:find(
          '""following_transfer_slots"":[8]', 1, true) ~= nil,
      "the second 0x5322 fan-out transfers to source slot 8 after 62 heartbeats")

local sleep_at_limit_source = fixture_row("code5_route_cost_at_500")
local sleep_at_limit = run_callback("sleep", {
    seed = sleep_at_limit_source.seed_before,
    sleep_targets = {
        { id = "sleep-at-500", source_record_slot = 7,
            direct_route_step_cost = 0x0500 },
    },
})
check(matches_fixture(sleep_at_limit, sleep_at_limit_source, true) and
      sleep_at_limit.next_pending == nil and
      sleep_at_limit_source.selected_slots == "",
      "0x1597E's strict route-cost bound excludes exactly 0x500")

local sleep_above_limit_source = fixture_row("code5_route_cost_above_500")
local sleep_above_limit = run_callback("sleep", {
    seed = sleep_above_limit_source.seed_before,
    sleep_targets = {
        { id = "sleep-above-500", source_record_slot = 7,
            direct_route_step_cost = 0x0501 },
    },
})
check(matches_fixture(sleep_above_limit, sleep_above_limit_source, true) and
      sleep_above_limit.next_pending == nil and
      sleep_above_limit_source.selected_slots == "",
      "0x1597E excludes route costs above 0x500")

local sleep_dead_before_source = fixture_row("code5_dead_before_fanout")
local sleep_dead_before = run_callback("sleep", {
    seed = sleep_dead_before_source.seed_before,
    reselect_single_route_target = true,
    sleep_targets = {
        { id = "sleep-slot-7", source_record_slot = 7,
            direct_route_step_cost = 0x0100 },
        { id = "sleep-slot-8", source_record_slot = 8,
            direct_route_step_cost = 0x0200 },
    },
    deactivate_candidate_at_heartbeat = 62,
    deactivate_candidate_id = "sleep-slot-7",
})
check(matches_fixture(sleep_dead_before, sleep_dead_before_source, true) and
      sleep_dead_before.next_pending ~= nil and
      sleep_dead_before.next_pending.target_id == "sleep-slot-8" and
      sleep_dead_before_source.selected_slots == "8",
      "0x5322 skips a marked record that dies before fan-out")

local sleep_dead_mid_source = fixture_row("code5_dead_mid_fanout")
local sleep_dead_mid = run_callback("sleep", {
    seed = sleep_dead_mid_source.seed_before,
    reselect_single_route_target = true,
    sleep_targets = {
        { id = "sleep-slot-7", source_record_slot = 7,
            direct_route_step_cost = 0x0100 },
        { id = "sleep-slot-8", source_record_slot = 8,
            direct_route_step_cost = 0x0200 },
    },
})
local interrupted, _, dead_mid_heartbeats = service_pending_target(
    sleep_dead_mid, "sleep-slot-7", 20)
if interrupted ~= nil then
    interrupted.actor_facts.lifecycle.active = false
    interrupted.actor_facts.lifecycle.dead = true
end
check(matches_fixture(sleep_dead_mid, sleep_dead_mid_source, true) and
      dead_mid_heartbeats == 20 and
      sleep_dead_mid.next_pending ~= nil and interrupted ~= nil and
      interrupted.spell_effect_pending == sleep_dead_mid.next_pending and
      Actions.pending_spell_effects_by_actor_id["sleep-slot-8"] == nil and
      sleep_dead_mid_source.selected_slots == "7",
      "the slot-7 fan-out stalls when its target dies after 20 heartbeats")

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
