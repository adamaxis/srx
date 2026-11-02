-- ROM 0xB1DC -> 0x55C76: Joshua's no-weapon attack is a three-stage
-- exploding-d6 contest, with the defender's armor and body in separate
-- stages. The deterministic trace below uses default Joshua Samurai at
-- 0x7291A, including Hand Razors from +0x90=0x0004, and
-- the first actor in the Scene 2/event 9 target list at 0x1D6D5E. That source
-- actor has armor ordinal 3, whose secondary modifier is 3 in ROM table
-- 0xD4D50. See the Step 1 trace in /workspace/shadowrun/out/progress_notes.md.
local E = require("enums")
local Combat = require("combat")
local Items = require("items")

game = {
    actors = {},
    modules = { combat = Combat, items = Items, magic = {} },
}

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        passed = passed + 1
    else
        print("FAIL " .. label)
    end
end

local function make_combatant(source)
    return {
        is_party_member = source.party,
        physical_meter = source.physical_meter,
        mental_meter = source.mental_meter,
        force = 0,
        posture_reduction_percent = source.posture * 25,
        equipped_action_id = source.action_id,
        equipped_armor_id = source.armor_id,
        attributes = {
            body = source.body,
            quickness = source.quickness,
            strength = source.strength,
            intelligence = source.intelligence,
            willpower = source.willpower,
        },
        skills = {
            sorcery = 0,
            firearms = 0,
            pistols = 0,
            smgs = 0,
            shotguns = 0,
            melee_combat = source.melee,
        },
        cyberware = {
            cybereyes = source.cybereyes,
            hand_razors = source.hand_razors,
            spurs = source.spurs,
            melee_strength_bonus = source.melee_strength_bonus,
            melee_defense_bonus = source.melee_defense_bonus,
        },
        actor_facts = {
            lifecycle = {
                active = true,
                thinking_enabled = false,
                ai_runner = source.party,
                downed = false,
                group_leader = false,
                dead = false,
            },
            interaction = {
                projectile = false,
                proximity_interaction_pending = false,
                use_collision_heading = false,
                collision_recovery_active = false,
                action_hook_pending = false,
                defense_difficulty_reduction = false,
                breathing_out = false,
                unseen = source.unseen,
                no_coverage_difficulty_bonus = source.no_coverage,
            },
        },
        combat_traits = {
            ignores_cover_condition_penalty = source.race == 2 or
                source.race == 4,
            adds_two_condition_difficulty = source.class_code == 0x16,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        action_profile_options = { primary = false, secondary = false },
        inventory = {},
    }
end

local joshua = {
    party = true,
    physical_meter = 10,
    mental_meter = 10,
    body = 4,
    quickness = 3,
    strength = 4,
    intelligence = 2,
    willpower = 4,
    melee = 3,
    posture = E.Posture.NEUTRAL,
    race = 0,
    class_code = 2,
    action_id = nil, -- Unarmed: ROM +0x57 is zero.
    armor_id = Items.by_id.armor_vest.id,
    cybereyes = false,
    hand_razors = true,
    spurs = false,
    melee_strength_bonus = 0,
    melee_defense_bonus = 0,
    unseen = false,
    no_coverage = false,
}

local halloweeners_contract_target = {
    party = false,
    physical_meter = 10,
    mental_meter = 10,
    body = 5,
    quickness = 5,
    strength = 4,
    intelligence = 4,
    willpower = 3,
    melee = 5,
    posture = E.Posture.NEUTRAL,
    race = 0,
    class_code = 2,
    action_code = E.ItemId.MACH22,
    action_id = Items.by_id.mach_22.id,
    armor_ordinal = 3,
    secondary_armor = 3, -- ROM D4D50 +0x26 for armor ordinal 3.
    armor_id = Items.by_id.vest_with_plates.id,
    cybereyes = true,
    hand_razors = true,
    spurs = false,
    melee_strength_bonus = 1,
    melee_defense_bonus = 0,
    unseen = false,
    no_coverage = false,
}

local function source_rng(seed)
    return function(bound)
        -- ROM 0x0D44 advances the 16-bit seed; 0x0D3C returns the high
        -- word of seed*bound. This is also the host's field RNG contract.
        seed = (seed * 0x20E5 + 1) & 0xFFFF
        return (seed * bound) >> 16
    end
end

local function command_average(source)
    return (source.quickness + source.intelligence + source.willpower) // 2
end

local function posture_share(source)
    return (command_average(source) * source.posture * 25) // 100
end

local function source_condition(difficulty, attacker, target)
    -- ROM 0x55740 applies attacker cybereyes, target class 0x16, then the
    -- target's +0x11.7 coverage flag in that order.
    if attacker.cybereyes then
        difficulty = difficulty - 1
    end
    if target.class_code == 0x16 then
        return math.max(difficulty + 2, 2)
    end
    if attacker.cybereyes or attacker.race == 2 or attacker.race == 4 then
        return math.max(difficulty, 2)
    end
    if target.no_coverage or target.unseen then
        difficulty = difficulty + 4
    end
    return math.max(difficulty, 2)
end

local function source_rolls(count, difficulty, random_bound)
    -- ROM 0x0DBE..0x0DFA coerces a zero pool to one trial and explodes sixes.
    local successes = 0
    local trials = count == 0 and 1 or count
    for _ = 1, trials do
        local accumulated = 0
        while true do
            local face = random_bound(6) + 1
            if face == 6 then
                accumulated = accumulated + face
            else
                if accumulated + face >= difficulty then
                    successes = successes + 1
                end
                break
            end
        end
    end
    return successes
end

local function source_unarmed_reaction(random_bound, attacker, defender)
    local meter = (attacker.spurs or attacker.hand_razors)
        and "physical" or "stun"
    -- ROM 0x55C76..0x55CAE: max(posture-share, melee)+melee.
    local first_pool = math.max(posture_share(attacker), attacker.melee) +
        attacker.melee
    local first_successes = source_rolls(first_pool,
        source_condition(E.CombatTuning.DEFAULT_ACTION_DIFFICULTY,
                        attacker, defender), random_bound)
    if first_successes == 0 then
        return 0, meter
    end

    -- ROM 0x55CBE adds three; 0x55EB4 subtracts target armor ordinal 3's
    -- secondary modifier (3), and 0x55E62 has no attacker defense item here.
    local remaining = first_successes + E.CombatTuning.MELEE_BASE_DAMAGE -
        defender.secondary_armor
    if remaining <= 0 then
        return 0, meter
    end

    -- ROM 0x55CD0..0x55D22: max(target posture-share, melee), plus the
    -- skill only for an unarmed defender (this target's +0x57 is code 10).
    local second_pool = math.max(posture_share(defender), defender.melee)
    if defender.action_code == E.ItemId.NONE then
        second_pool = second_pool + defender.melee
    end
    local second_successes = source_rolls(second_pool,
        source_condition(E.CombatTuning.DEFAULT_ACTION_DIFFICULTY,
                        defender, attacker), random_bound)
    remaining = remaining - second_successes
    if remaining <= 0 then
        return 0, meter
    end

    -- ROM 0x55D4E..0x55D9E: target command average minus posture-share,
    -- plus target body and cyberware defense bonus, rolled at attacker strength.
    local final_pool = command_average(defender) -
        posture_share(defender) + defender.body + defender.melee_defense_bonus
    local final_successes = source_rolls(final_pool,
        math.max(attacker.strength + attacker.melee_strength_bonus,
                 E.CombatTuning.MINIMUM_ACTION_DIFFICULTY),
        random_bound)
    remaining = remaining - final_successes
    if remaining <= 0 then
        return 0, meter
    end

    -- ROM 0x55DA2..0x55E60: Samurai Hand Razors subtract two, then choose
    -- the physical meter; without either melee cyberware, 0x55E22 routes stun.
    if attacker.hand_razors then
        remaining = remaining - 2
    end
    return math.max(remaining, 0), meter
end

-- Joshua's ROM Samurai profile `0x7291A` carries Hand Razors in `+0x90=4`.
-- Clear only that cyberware bit for the bare-fist fixture so `0x55E22..0x55E5A`
-- is exercised; both test cases keep the source action code at `+0x57=0`.
local bare_fist_joshua = {}
for key, value in pairs(joshua) do
    bare_fist_joshua[key] = value
end
bare_fist_joshua.hand_razors = false

local function run_sweep(label, attacker, seed, expected_hits,
                         expected_damage, expected_meter)
    local port_random = source_rng(seed)
    local source_random = source_rng(seed)
    local port_hits, port_damage_total = 0, 0
    local source_hits, source_damage_total = 0, 0
    local row_mismatches, route_mismatches = 0, 0
    local first_mismatch

    for attack = 1, 200 do
        local target = {
            id = label .. "-halloweener-" .. attack,
            combatant = make_combatant(halloweeners_contract_target),
        }
        game.actors[target.id] = nil
        local result = Combat.resolve_reaction({
            combatant = make_combatant(attacker),
            ai_target_candidates = {},
            ai_random_bound = port_random,
        }, target)
        local port_route = result and
            (result.physical and "physical" or "stun") or nil
        local port_damage = 0
        if result ~= nil then
            port_damage = result.physical
                and (10 - result.physical_meter)
                or (10 - result.mental_meter)
            if result.physical ~= (expected_meter == "physical") or
               result.changed ~= (port_damage > 0) then
                route_mismatches = route_mismatches + 1
            end
        end
        local source_damage, source_route = source_unarmed_reaction(
            source_random, attacker, halloweeners_contract_target)
        if port_damage > 0 then
            port_hits = port_hits + 1
            port_damage_total = port_damage_total + port_damage
        end
        if source_damage > 0 then
            source_hits = source_hits + 1
            source_damage_total = source_damage_total + source_damage
        end
        if port_damage ~= source_damage or
           (port_damage > 0 and port_route ~= source_route) then
            row_mismatches = row_mismatches + 1
            first_mismatch = first_mismatch or string.format(
                "attack %d port %s/%d / ROM %s/%d", attack,
                port_route or "miss", port_damage,
                source_damage > 0 and source_route or "miss", source_damage)
        end
    end

    -- These seeded expectations come from the ROM pools and meter branch at
    -- 0x55C76..0x55E5A; see /workspace/shadowrun/out/progress_notes.md.
    check(source_hits == expected_hits and
          source_damage_total == expected_damage,
          string.format("%s ROM formula seed changed: %d hits / %d damage",
                        label, source_hits, source_damage_total))
    check(row_mismatches == 0 and port_hits == source_hits and
          port_damage_total == source_damage_total,
          string.format("%s 200 attacks disagree with ROM: port %d hits / %d damage, source %d / %d, row mismatches %d",
                        label, port_hits, port_damage_total, source_hits,
                        source_damage_total, row_mismatches) ..
              (first_mismatch and ("; " .. first_mismatch) or ""))
    check(route_mismatches == 0,
          string.format("%s meter route disagrees with 0x55DA2..0x55E5A in %d rows",
                        label, route_mismatches))
    print(string.format("%s: 200 seeded attacks %d/%d hits, %d %s damage, %d ROM row mismatches",
                        label, port_hits, source_hits, port_damage_total,
                        expected_meter, row_mismatches))
end

-- For bare fists, P(damage>0)=0.0117285458 and E[damage]=0.0153009862
-- from S1~B(6,1/2), S2~B(5,2/3), S3~B(8,1/2), with armor 3 canceling +3.
run_sweep("bare-fist stun", bare_fist_joshua, 0x0437, 3, 5, "stun")
-- The default-Hand-Razor seed contains one physical hit after the ROM's -2.
run_sweep("default Hand-Razor", joshua, 0x0004, 1, 1, "physical")

print(string.format("unarmed_combat_tests: %d/%d passed", passed, total))
if passed ~= total then
    os.exit(1)
end
