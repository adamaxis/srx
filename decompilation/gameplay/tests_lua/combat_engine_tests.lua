-- Focused combat engine contract tests. Expected values below are derived
-- from the US-ROM tables and routines named beside each case; the test model
-- deliberately does not use Combat's result to compute its expected result.
local Combat = require("combat")
local E = require("enums")
local Items = require("items")
local Magic = require("magic")
local Delay = require("actor.delay")

game = { actors = {}, modules = {
    combat = Combat,
    items = Items,
    magic = Magic,
} }

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        passed = passed + 1
    else
        print("FAIL " .. label)
    end
end

local ROM_WEAPONS = {
    streetline = { effect = 1, difficulty = 4, skill = "pistols" },
    model_101t = { effect = 1, difficulty = 6, skill = "pistols" },
    american_l36 = { effect = 1, difficulty = 6, skill = "pistols" },
    security_500 = { effect = 1, difficulty = 6, skill = "pistols" },
    warhawk = { effect = 3, difficulty = 10, skill = "pistols" },
    max_power = { effect = 3, difficulty = 9, skill = "pistols" },
    predator = { effect = 3, difficulty = 9, skill = "pistols" },
    ak_97 = { effect = 3, difficulty = 6, skill = "smgs" },
    hk227_s = { effect = 3, difficulty = 7, skill = "smgs" },
    mach_22 = { effect = 3, difficulty = 6, skill = "smgs" },
    allegiance = { effect = 6, difficulty = 10, skill = "shotguns" },
    roomsweeper = { effect = 6, difficulty = 9, skill = "shotguns" },
}

local ROM_ARMOR = {
    { "armor_vest", 2, 1 },
    { "armor_clothing", 1, 2 },
    { "armor_jacket", 5, 3 },
    { "vest_with_plates", 4, 3 },
    { "lined_duster", 3, 2 },
    { "light_combat_armor", 6, 4 },
    { "heavy_combat_armor", 8, 6 },
}

local ROM_SPELLS = {
    flame_dart = { difficulty = 0, effect = 1 },
    flame_bolt = { difficulty = 1, effect = 3 },
    hellblast = { difficulty = 3, effect = 10 },
    invisibility = { difficulty = 0, effect = 3 },
    sleep = { difficulty = 0, effect = 6 },
    stink = { difficulty = 0, effect = 6 },
    super_barrier = { difficulty = 2, effect = 6 },
    mana_zap = { difficulty = 0, effect = 1 },
    mana_blast = { difficulty = 0, effect = 3 },
    mana_storm = { difficulty = 1, effect = 6 },
    rockskin = { difficulty = 2, effect = 3 },
    heal_wounds = { difficulty = 0, effect = 3 },
    confusion = { difficulty = 0, effect = 6 },
    barrier = { difficulty = 2, effect = 3 },
}

local function source_rng(seed)
    return function(bound)
        -- 0x0D44 advances the 16-bit source seed; 0x0D3A scales it to a bound.
        seed = (seed * 0x20E5 + 1) & 0xFFFF
        return (seed * bound) >> 16
    end
end

local function source_rolls(count, difficulty, random_bound)
    -- ROM 0x0DBA..0x0DFA coerces an empty pool to one die and explodes sixes.
    local successes = 0
    for _ = 1, math.max(count, 1) do
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

local function command_average(combatant)
    -- ROM 0x55ED6 averages Quickness, Intelligence, and Willpower by shifting.
    local attributes = combatant.attributes
    return (attributes.quickness + attributes.intelligence +
            attributes.willpower) // 2
end

local function posture_share(combatant)
    -- ROM 0x55F20 multiplies by posture*25 and divides by 100 (truncate).
    return command_average(combatant) *
        combatant.posture_reduction_percent // 100
end

local function wound_difficulty(combatant)
    -- ROM 0xD5 adds 2 below five, 1 from five through seven, else 0 per meter.
    local result = 0
    for _, meter in ipairs({ combatant.physical_meter,
                              combatant.mental_meter }) do
        if meter < 8 then
            result = result + (meter < 5 and 2 or 1)
        end
    end
    return result
end

local function make_combatant(options)
    local action = options.action
    local weapon = action and ROM_WEAPONS[action] or nil
    local skills = {
        firearms = options.firearms or 0,
        pistols = options.pistols or 0,
        smgs = options.smgs or 0,
        shotguns = options.shotguns or 0,
        melee_combat = options.melee or 0,
        sorcery = options.sorcery or 0,
    }
    local policies = {}
    if weapon ~= nil then
        policies[action] = {
            skill_group = weapon.skill,
            difficulty_adjustment = options.difficulty_adjustment or 0,
            benefits_from_targeting_assist =
                options.benefits_from_targeting_assist == true,
        }
    end
    return {
        is_party_member = options.party == true,
        physical_meter = options.physical_meter or 10,
        mental_meter = options.mental_meter or 10,
        posture_reduction_percent = options.posture_reduction_percent or 0,
        equipped_action_id = action,
        equipped_armor_id = options.armor,
        force = options.force or 0,
        spell_trial_cap = options.spell_trial_cap or 0,
        attributes = {
            body = options.body or 1,
            quickness = options.quickness or 1,
            strength = options.strength or 1,
            intelligence = options.intelligence or 1,
            willpower = options.willpower or 1,
        },
        skills = skills,
        cyberware = {
            datajack = options.datajack == true,
            cybereyes = options.cybereyes == true,
            hand_razors = options.hand_razors == true,
            spurs = options.spurs == true,
            smartlink = options.smartlink == true,
            melee_strength_bonus = options.melee_strength_bonus or 0,
            melee_defense_bonus = options.melee_defense_bonus or 0,
        },
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        actor_facts = {
            lifecycle = {
                active = true, thinking_enabled = true,
                ai_runner = options.party == true,
                downed = false, group_leader = false, dead = false,
            },
            interaction = {
                no_coverage_difficulty_bonus = false,
                unseen = false,
                defense_difficulty_reduction = false,
            },
        },
        action_profile_options = { primary = false, secondary = false },
        inventory = options.inventory or {},
        weapon_action_policies = policies,
    }
end

local function source_weapon_attack(combatant, action, random_bound)
    -- ROM 0x55640 -> 0x12C74 merges Firearms/specialty skill with posture.
    local weapon = ROM_WEAPONS[action]
    local selected = math.max(combatant.skills.firearms,
                              combatant.skills[weapon.skill])
    local trials = math.max(
        selected + math.min(selected, posture_share(combatant)), 1)
    local difficulty = 4 + wound_difficulty(combatant)
    if combatant.cyberware.cybereyes then
        difficulty = difficulty - 1 -- ROM 0x55740
    end
    local policy = combatant.weapon_action_policies[action]
    difficulty = difficulty + policy.difficulty_adjustment
    if policy.benefits_from_targeting_assist then
        if combatant.cyberware.smartlink then
            difficulty = difficulty - 2 -- ROM 0x556C6 smartlink branch
        elseif #combatant.inventory > 0 and
               combatant.inventory[1].id == "smart_goggles" then
            difficulty = difficulty - 1 -- ROM 0x556C6 goggles branch
        end
    end
    return source_rolls(trials, math.max(difficulty, 2), random_bound)
end

local function source_weapon_damage(defender, action, attack_successes,
                                    random_bound)
    local weapon = ROM_WEAPONS[action]
    -- ROM 0x55948..0x559BC rolls the defender's command/body pool at the
    -- item's +0x26 difficulty, then adds the success difference to +0x25's
    -- 0x166AD4-derived effect value.
    local trials = command_average(defender) - posture_share(defender) +
        defender.attributes.body +
        (defender.cyberware.melee_defense_bonus or 0)
    local defense_successes = source_rolls(
        trials, math.max(weapon.difficulty, 2), random_bound)
    local value = weapon.effect + attack_successes - defense_successes
    if value >= 11 then
        value = 10 -- ROM 0x559E6..0x559F0
    end
    value = math.max(value, 0)
    if defender.equipped_armor_id ~= nil then
        value = value - Items.by_id[defender.equipped_armor_id].armor.primary_modifier
    end
    return math.max(value, 0)
end

local function source_spell_residual(force, roll_count, spell)
    -- ROM 0x10660: ceil(force/2), +D4EA0 difficulty modifier, then 20% steps.
    local effective = math.max((force + 1) // 2 + spell.difficulty, 2)
    local percent = effective >= 7 and 10 or (7 - effective) * 20
    local product = roll_count * percent
    local adjusted = product // 100
    if product % 100 >= 50 then
        adjusted = adjusted + 1
    end
    return spell.effect - (adjusted // 2)
end

-- Every firearm row is checked against the ROM's +0x25 power lookup and
-- +0x26 difficulty field at 0xD4540 (stride 0x30) / 0x166AD4.
for action, source in pairs(ROM_WEAPONS) do
    local item = Items.by_id[action]
    check(item ~= nil and item.weapon.source_action.effect_value == source.effect and
          item.weapon.source_action.difficulty == source.difficulty,
          action .. " source effect/difficulty differs from D4540/166AD4")
    local attacker = make_combatant({
        action = action, firearms = 2,
        pistols = 2, smgs = 2, shotguns = 2,
        quickness = 3, intelligence = 2, willpower = 4,
        posture_reduction_percent = 50,
    })
    local rom_random, port_random = source_rng(0x0400 + item.weapon.source_action.difficulty),
                                     source_rng(0x0400 + item.weapon.source_action.difficulty)
    local expected = source_weapon_attack(attacker, action, rom_random)
    local actual = Combat.resolve_weapon_attack_seed({
        combatant = attacker, projectile_action_id = action,
        ai_target_candidates = {}, ai_random_bound = port_random,
    })
    check(actual == expected,
          action .. " seeded attack pool differs from 0x55640/0x12C74")
end

-- ROM 0xD4D50 rows feed 0x55E92 primary (gun) and 0x55EB4 secondary
-- (melee) armor deductions. Check every item and the live gun hit result.
for _, row in ipairs(ROM_ARMOR) do
    local armor_id, primary, secondary = table.unpack(row)
    local armor = Items.by_id[armor_id].armor
    check(armor.primary_modifier == primary and
          armor.secondary_modifier == secondary,
          armor_id .. " differs from D4D50 armor ordinal")
end

local armor_damage = { 1, 2, 0, 0, 0, 0, 0 }
for index, row in ipairs(ROM_ARMOR) do
    local armor_id, primary = row[1], row[2]
    game.actors = {}
    local attacker = make_combatant({
        party = true, action = "american_l36", firearms = 2, pistols = 2,
        quickness = 3, intelligence = 2, willpower = 4,
        posture_reduction_percent = 50,
    })
    local target = {
        id = "armor-hit-" .. armor_id,
        combatant = make_combatant({
            body = 5, quickness = 5, intelligence = 4, willpower = 3,
            posture_reduction_percent = 50, armor = armor_id,
        }),
    }
    local rom_random, port_random = source_rng(0x0437), source_rng(0x0437)
    local expected_attack = source_weapon_attack(
        attacker, "american_l36", rom_random)
    local attack_context = {
        combatant = attacker, projectile_action_id = "american_l36",
        ai_target_candidates = {}, ai_random_bound = port_random,
    }
    local attack = Combat.resolve_weapon_attack_seed(attack_context)
    local expected_damage = source_weapon_damage(
        target.combatant, "american_l36", expected_attack, rom_random)
    local result = Combat.resolve_weapon_hit(
        attack_context, { attack_successes = attack }, target)
    check(expected_attack == 3 and attack == expected_attack and
          expected_damage == armor_damage[index] and result ~= nil and
          result.physical_meter == 10 - expected_damage and
          result.mental_meter == 10 and result.physical == true,
          armor_id .. " seeded gun damage/physical meter differs from ROM " ..
              "0x55948/0x559BC/0x55E92/0x55A90")
    check(result ~= nil and result.physical_meter >= 0 and
          result.mental_meter >= 0,
          armor_id .. " allowed a negative health meter")
end

-- Source-backed armor values are also exposed by the weapon action contract.
-- Smartlink, Cybereyes, and Datajack use separate +0x90 bits; the source
-- target-number branch at 0x556C6 grants -2 for Smartlink and -1 for goggles.
local Weapons = require("data.weapons")
local smartgun = E.Accessory.SMARTGUN
check(Weapons.get("american_l36"):tn_modifier(1 << smartgun, true, false) == -2 and
      Weapons.get("american_l36"):tn_modifier(1 << smartgun, false, true) == -1,
      "Smartlink/Smart Goggles modifiers differ from ROM 0x556C6")
local no_ware = make_combatant({
    action = "american_l36", firearms = 2, pistols = 2,
    quickness = 3, intelligence = 2, willpower = 4,
    posture_reduction_percent = 50,
})
local eye_ware = make_combatant({
    action = "american_l36", firearms = 2, pistols = 2,
    quickness = 3, intelligence = 2, willpower = 4,
    posture_reduction_percent = 50, cybereyes = true,
})
local smartlink = make_combatant({
    action = "american_l36", firearms = 2, pistols = 2,
    quickness = 3, intelligence = 2, willpower = 4,
    posture_reduction_percent = 50, smartlink = true,
    benefits_from_targeting_assist = true,
})
local datajack = make_combatant({
    action = "american_l36", firearms = 2, pistols = 2,
    quickness = 3, intelligence = 2, willpower = 4,
    posture_reduction_percent = 50, datajack = true,
})
local function attack_seed(combatant, seed)
    return Combat.resolve_weapon_attack_seed({
        combatant = combatant, projectile_action_id = "american_l36",
        ai_target_candidates = {}, ai_random_bound = source_rng(seed),
    })
end
check(attack_seed(no_ware, 0x0437) == 3 and
      attack_seed(eye_ware, 0x0437) == source_weapon_attack(
          eye_ware, "american_l36", source_rng(0x0437)),
      "Cybereyes seeded attack does not follow the -1 condition at 0x55740")
check(attack_seed(smartlink, 0x0437) == source_weapon_attack(
          smartlink, "american_l36", source_rng(0x0437)),
      "Smartlink seeded attack does not follow the -2 target adjustment at 0x556C6")
check(attack_seed(datajack, 0x0437) == attack_seed(no_ware, 0x0437),
      "Datajack changed weapon rolls though combat does not read +0x90 bit 0")

-- ROM 0xD7EA checks the first enabled +0x90 capability bit 12, 13, or 14
-- for wired reflexes (+4/+8/+12 to the delay factor); 0xABFC computes the
-- base delay and a Warhawk uses its source half-delay mode. The resulting
-- action delays are 43/35/27 frames (versus 51 with no reflex upgrade).
for _, row in ipairs({
    { 0x0000, 51 }, { 0x1000, 43 }, { 0x2000, 35 }, { 0x4000, 27 },
}) do
    local delay = Delay.player_weapon_action_delay(
        3, 2, row[1], "warhawk", Delay.Use.ATTACK)
    check(delay == row[2], string.format(
        "Wired Reflexes capability %04X derived %s instead of %d frames at 0xD7EA/0xABFC",
        row[1], tostring(delay), row[2]))
end

-- D4EA0 has fourteen spell rows. Assert only the confirmed difficulty and
-- base-effect fields; target-effect/resistance labels remain [H] and are not
-- test expectations. 0x10660 derives the current-force residual from these.
for spell_id, source in pairs(ROM_SPELLS) do
    local spell = Magic.spell_action_definition(spell_id)
    check(spell ~= nil and spell.difficulty_modifier == source.difficulty and
          spell.base_effect == source.effect,
          spell_id .. " differs from confirmed D4EA0 fields")
end

local function spell_caster(force, inventory)
    local combatant = make_combatant({
        party = true, force = force, spell_trial_cap = 8,
        sorcery = 8, posture_reduction_percent = 50,
        body = 4, quickness = 4, intelligence = 4, willpower = 5,
        inventory = inventory,
    })
    return combatant
end

local function spell_target(id)
    return { id = id, combatant = make_combatant({
        body = 4, quickness = 4, intelligence = 4, willpower = 4,
    }) }
end

local mana_storm = ROM_SPELLS.mana_storm
for _, case in ipairs({
    { force = 2, inventory = {}, trials = 6, residual = 2 },
    { force = 4, inventory = {}, trials = 8, residual = 3 },
    { force = 4, inventory = { { slot = 8, id = "gator_totem" } },
      trials = 10, residual = 3 },
}) do
    local target = spell_target("mana-storm-target-" .. case.force .. "-" ..
                                #case.inventory)
    local result = Combat.resolve_spell_action({
        combatant = spell_caster(case.force, case.inventory),
        action_spell_id = "mana_storm", actor_id = "mana-storm-caster",
        ai_target_candidates = {}, ai_random_bound = source_rng(0x0437),
    }, target)
    local base_trials = math.min((8 * 50) // 100, 8) + case.force
    local expected_trials = base_trials +
        (#case.inventory > 0 and 2 or 0) -- slot-8 Gator Totem, ROM 0x104F6
    local expected_successes = source_rolls(
        expected_trials, 4, source_rng(0x0437)) -- ROM 0x0DBA; base TN 4
    local expected_residual = source_spell_residual(
        case.force, (8 - 4) + 5, mana_storm) -- 0x10660, posture 50%, Willpower 5
    check(result ~= nil and result.initial_trial_count == expected_trials and
          result.initial_trial_count == case.trials and
          result.difficulty == 4 and result.successes == expected_successes and
          result.successes > 0 and result.residual_effect == expected_residual and
          result.residual_effect == case.residual,
          "Mana Storm Force/Totem derived values differ from 0x1033E/0x104F6/0x10660")
end

-- The ROM delay is 0x6A - (floor((Q+I)/2) - 1)*4, then the weapon mode;
-- 0xA8EA decrements it before 0xAD1A permits the next action.
local pistol_gate = Delay.player_weapon_action_delay(
    3, 2, 0, "warhawk", Delay.Use.ATTACK)
local cast_gate = Delay.calculate(3, 2, 0, E.ActionDelayMode.NORMAL)
check(pistol_gate == 51 and cast_gate == 102 and
      pistol_gate + 1 == 52 and cast_gate + 1 == 103,
      "source action timing changed from 51/52 and 102/103 pass gates")

local function run_fight(kind, seed)
    game.actors = {}
    local party_a = kind == "party_enemy"
    local port_a = make_combatant({
        party = party_a, action = "warhawk", armor = "armor_vest",
        firearms = 6, pistols = 6, smgs = 6,
        body = 4, quickness = 5, strength = 4,
        intelligence = 5, willpower = 5,
        posture_reduction_percent = 50,
    })
    local rom_a = make_combatant({
        party = party_a, action = "warhawk", armor = "armor_vest",
        firearms = 6, pistols = 6, smgs = 6,
        body = 4, quickness = 5, strength = 4,
        intelligence = 5, willpower = 5,
        posture_reduction_percent = 50,
    })
    local port_b = make_combatant({
        action = "mach_22", armor = "vest_with_plates",
        firearms = 5, pistols = 5, smgs = 5,
        body = 5, quickness = 5, strength = 4,
        intelligence = 4, willpower = 4,
        posture_reduction_percent = 50,
    })
    local rom_b = make_combatant({
        action = "mach_22", armor = "vest_with_plates",
        firearms = 5, pistols = 5, smgs = 5,
        body = 5, quickness = 5, strength = 4,
        intelligence = 4, willpower = 4,
        posture_reduction_percent = 50,
    })
    local port_actor_a = { id = "port-a-" .. kind .. seed,
                           combatant = port_a }
    local port_actor_b = { id = "port-b-" .. kind .. seed,
                           combatant = port_b }
    local rom_rng, port_rng = source_rng(seed), source_rng(seed)
    local mismatches, invariant_failures, actions = 0, 0, 0

    local function attack(port_from, port_to, rom_from, rom_to, action)
        actions = actions + 1
        local rom_attack = source_weapon_attack(rom_from, action, rom_rng)
        local context = {
            combatant = port_from.combatant,
            projectile_action_id = action,
            ai_target_candidates = {},
            ai_random_bound = port_rng,
        }
        local port_attack = Combat.resolve_weapon_attack_seed(context)
        if port_attack ~= rom_attack then
            mismatches = mismatches + 1
        end
        local rom_damage = source_weapon_damage(
            rom_to, action, rom_attack, rom_rng)
        local expected_meter = math.max(0, rom_to.physical_meter - rom_damage)
        rom_to.physical_meter = expected_meter

        local old_mental = port_to.combatant.mental_meter
        local result = Combat.resolve_weapon_hit(
            context, { attack_successes = port_attack }, port_to)
        if result == nil or result.physical_meter ~= expected_meter or
           result.mental_meter ~= old_mental or result.physical ~= true then
            mismatches = mismatches + 1
        elseif result.changed then
            port_to.combatant = game.actors[port_to.id].combatant
        end
        if result == nil or result.physical_meter < 0 or
           result.mental_meter < 0 then
            invariant_failures = invariant_failures + 1
        end
        if expected_meter == 0 and rom_damage > 0 and
           (result == nil or result.physical_meter ~= 0) then
            invariant_failures = invariant_failures + 1
        end
    end

    local rounds = 0
    while rom_a.physical_meter > 0 and rom_b.physical_meter > 0 and
          rounds < 40 do
        rounds = rounds + 1
        attack(port_actor_a, port_actor_b, rom_a, rom_b, "warhawk")
        if rom_b.physical_meter > 0 then
            attack(port_actor_b, port_actor_a, rom_b, rom_a, "mach_22")
        end
    end
    local port_a_meter = port_actor_a.combatant.physical_meter
    local port_b_meter = port_actor_b.combatant.physical_meter
    local rom_winner = rom_a.physical_meter == 0 and "B" or
        (rom_b.physical_meter == 0 and "A" or "draw")
    local port_winner = port_a_meter == 0 and "B" or
        (port_b_meter == 0 and "A" or "draw")
    return rom_winner, port_winner, mismatches, invariant_failures, actions,
           rounds
end

-- 0x55948/0x559BC/0x55A90: 64 independent source seeds per combat mode.
-- The two source weapons decode from D4540/166AD4; each hit is checked before
-- the deterministic outcome distribution is counted.
for _, kind in ipairs({ "enemy_enemy", "party_enemy" }) do
    local rom_outcomes = { A = 0, B = 0, draw = 0 }
    local port_outcomes = { A = 0, B = 0, draw = 0 }
    local mismatches, invariant_failures, actions = 0, 0, 0
    for seed = 1, 64 do
        local rom_winner, port_winner, row_mismatches, row_invariants,
              row_actions = run_fight(kind, seed)
        rom_outcomes[rom_winner] = rom_outcomes[rom_winner] + 1
        port_outcomes[port_winner] = port_outcomes[port_winner] + 1
        mismatches = mismatches + row_mismatches
        invariant_failures = invariant_failures + row_invariants
        actions = actions + row_actions
    end
    check(rom_outcomes.A == 25 and rom_outcomes.B == 39 and
          rom_outcomes.draw == 0,
          kind .. " source-seed distribution changed from formula oracle")
    check(port_outcomes.A == rom_outcomes.A and
          port_outcomes.B == rom_outcomes.B and
          port_outcomes.draw == rom_outcomes.draw and
          mismatches == 0,
          kind .. " 64 seeded fights differ from ROM oracle")
    check(invariant_failures == 0,
          kind .. " fight produced negative meters or failed zero-meter death gate")
    print(string.format(
        "%s: 64 seeded fights, %d source actions, A/B/draw %d/%d/%d",
        kind, actions, rom_outcomes.A, rom_outcomes.B, rom_outcomes.draw))
end

print(string.format("combat_engine_tests: %d/%d passed", passed, total))
if passed ~= total then
    os.exit(1)
end
