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
require("actor.init")
local ActorAnim = require("actor.anim")
local PartyDown = require("party_down")

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

-- D4EA0 row 8 is Mana Blast (+0x57 code 9); 0x55B78 leaves FFF0CC clear
-- for that row, so 0x559BC reaches the physical write at 0x55A90. Sleep is
-- D4EA0 row 4 (+0x57 code 5), which sets FFF0CC and uses mental +0x9C at
-- 0x55A78. The H target-resistance labels are not checked here.
local function apply_spell_meter_path(spell_id, target_id)
    game.actors = {}
    local target = spell_target(target_id)
    local result = Combat.resolve_spell_impact({
        combatant = spell_caster(0, {}),
        ai_random_bound = function(bound) return bound == 6 and 0 or nil end,
    }, {
        spell_id = spell_id,
        successes = 30,
        target_id = target_id,
    }, target)
    return result
end
local mana_blast_impact = apply_spell_meter_path("mana_blast", "mana-meter")
check(E.SpellActionCode.MANA_BLAST == 9 and
      mana_blast_impact ~= nil and mana_blast_impact.physical_meter < 10 and
      mana_blast_impact.mental_meter == 10,
      "Mana Blast must damage physical +0x4C per 0x55B78/0x55A90")
local sleep_impact = apply_spell_meter_path("sleep", "sleep-meter")
check(E.SpellActionCode.SLEEP == 5 and sleep_impact ~= nil and
      sleep_impact.physical_meter == 10 and sleep_impact.mental_meter < 10,
      "Sleep must damage mental +0x9C per 0x55B78/0x55A78")

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

-- These fixtures are normalized from ROM records, not invented fighters.
-- Runner source fields are read from 0x72A1A; 0x42B0 copies that selected
-- 0x100-byte profile. World-actor combat fields are copied by 0x44BC from the
-- templates at 0x1D66A4 and 0x1D66EE; dynamic-world-activity-actors.tsv keeps
-- those decoded records and source addresses together.
local COMBAT_RECORDS = {
    joshua_decker = {
        rom_record = 0x72A1A, party = true,
        action = "american_l36", item_code = E.ItemId.AMERICAN_L36,
        armor = "armor_clothing", posture_steps = 2,
        physical_meter = 10, mental_meter = 10,
        body = 2, quickness = 5, strength = 2,
        intelligence = 4, willpower = 3,
        firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
        datajack = true,
    },
    weenr2 = {
        rom_record = 0x1D66A4, party = false,
        action = "streetline", item_code = E.ItemId.STREETLINE_SPECIAL,
        armor = "armor_vest", posture_steps = 2,
        physical_meter = 10, mental_meter = 10,
        body = 5, quickness = 4, strength = 4,
        intelligence = 2, willpower = 2,
        firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
        cyberware_flags = 0x0004, hand_razors = true,
    },
    yakuza_weenr = {
        rom_record = 0x1D66EE, party = false,
        action = "mach_22", item_code = E.ItemId.MACH22,
        armor = "armor_jacket", posture_steps = 2,
        physical_meter = 10, mental_meter = 10,
        body = 5, quickness = 5, strength = 4,
        intelligence = 4, willpower = 3,
        firearms = 5, pistols = 0, smgs = 0, shotguns = 0,
        cyberware_flags = 0x0026, cybereyes = true, hand_razors = true,
    },
}

local function source_combatant(profile, source_record_slot)
    local combatant = make_combatant({
        party = profile.party, action = profile.action,
        armor = profile.armor,
        firearms = profile.firearms, pistols = profile.pistols,
        smgs = profile.smgs, shotguns = profile.shotguns,
        body = profile.body, quickness = profile.quickness,
        strength = profile.strength, intelligence = profile.intelligence,
        willpower = profile.willpower,
        physical_meter = profile.physical_meter,
        mental_meter = profile.mental_meter,
        posture_reduction_percent = profile.posture_steps * 25,
        datajack = profile.datajack, cybereyes = profile.cybereyes,
        hand_razors = profile.hand_razors,
    })
    combatant.source_rom_record = profile.rom_record
    combatant.source_item_code = profile.item_code
    combatant.source_record_slot = source_record_slot
    return combatant
end

check(COMBAT_RECORDS.joshua_decker.rom_record == 0x72A1A and
      COMBAT_RECORDS.joshua_decker.item_code == E.ItemId.AMERICAN_L36 and
      COMBAT_RECORDS.weenr2.rom_record == 0x1D66A4 and
      COMBAT_RECORDS.weenr2.item_code == E.ItemId.STREETLINE_SPECIAL and
      COMBAT_RECORDS.yakuza_weenr.rom_record == 0x1D66EE and
      COMBAT_RECORDS.yakuza_weenr.item_code == E.ItemId.MACH22,
      "combat fixtures must retain distinct source profile and item records")

-- 0xA8EA scans active actor records from FF0100 in ascending 0x100-byte
-- strides. Party slot 0 therefore services before world slot 6; world slots
-- 6 and 7 service in that order. The source has record order, not a separate
-- randomized initiative roll.
local party_world_order = {
    source_combatant(COMBAT_RECORDS.weenr2, 6),
    source_combatant(COMBAT_RECORDS.joshua_decker, 0),
}
table.sort(party_world_order, function(left, right)
    return left.source_record_slot < right.source_record_slot
end)
check(party_world_order[1].source_record_slot == 0 and
      party_world_order[1].source_rom_record == 0x72A1A and
      party_world_order[2].source_record_slot == 6 and
      party_world_order[2].source_rom_record == 0x1D66A4,
      "0xA8EA turn service must visit FF0100 before FF0700")

-- A real-profile measured anchor validates the formula oracle independently
-- against the live Lua resolver. Headless seed 0x0001 measured 2 attack
-- successes, 0 defense successes, one physical damage, mental unchanged.
do
    game.actors = {}
    local party = source_combatant(COMBAT_RECORDS.joshua_decker, 0)
    local enemy = source_combatant(COMBAT_RECORDS.weenr2, 6)
    local target = { id = "measured-source-hit", combatant = enemy }
    local rom_rng, port_rng = source_rng(0x0001), source_rng(0x0001)
    local expected_attack = source_weapon_attack(
        party, "american_l36", rom_rng)
    local expected_damage = source_weapon_damage(
        enemy, "american_l36", expected_attack, rom_rng)
    local context = {
        combatant = party, projectile_action_id = "american_l36",
        ai_target_candidates = {}, ai_random_bound = port_rng,
    }
    local attack = Combat.resolve_weapon_attack_seed(context)
    local hit = Combat.resolve_weapon_hit(
        context, { attack_successes = attack }, target)
    check(expected_attack == 2 and attack == 2 and expected_damage == 1 and
          hit ~= nil and hit.physical_meter == 9 and hit.mental_meter == 10,
          "seed 0001 Joshua Decker 0x72A1A -> Weenr2 0x1D66A4 differs from measured 2/0/1, 10->9 physical")
end

local function source_lifecycle_downed(target, physical_meter, mental_meter)
    if target.combatant.is_party_member then
        local down_result = PartyDown.enter_runner_down_state({
            target_role = "leader",
            target_meters = {
                [E.RunnerMeter.PHYSICAL] = physical_meter,
                [E.RunnerMeter.MENTAL] = mental_meter,
            },
            party = {
                leader = { active = true, dead = false },
                companion_one = { active = true, dead = false },
                companion_two = { active = true, dead = false },
            },
        })
        return down_result.lifecycle.downed, false
    end

    local visual_profile = E.ActorVisualProfile.HUMAN_GUARD
    local actor_state = {
        visual_profile_id = visual_profile,
        direction = 0,
        actor_facts = { lifecycle = {
            active = true, ai_runner = false, downed = false, dead = false,
        } },
        incoming_hit = {
            physical_meter = physical_meter,
            mental_meter = mental_meter,
        },
    }
    ActorAnim.begin_received_hit_reaction(actor_state, {
        visual_profile_id = visual_profile,
    })
    return actor_state.actor_facts.lifecycle.downed,
           actor_state.actor_facts.lifecycle.dead == true
end

local function run_fight(kind, seed)
    game.actors = {}
    local profile_a, profile_b, slot_a, slot_b
    if kind == "party_enemy" then
        profile_a, profile_b = COMBAT_RECORDS.joshua_decker,
                              COMBAT_RECORDS.weenr2
        slot_a, slot_b = 0, 6
    else
        profile_a, profile_b = COMBAT_RECORDS.weenr2,
                              COMBAT_RECORDS.yakuza_weenr
        slot_a, slot_b = 6, 7
    end
    local fighter_a = {
        id = "fighter-a-" .. kind .. seed,
        profile = profile_a,
        source_record_slot = slot_a,
        port_actor = { id = "port-a-" .. kind .. seed,
                       combatant = source_combatant(profile_a, slot_a) },
        rom_combatant = source_combatant(profile_a, slot_a),
    }
    local fighter_b = {
        id = "fighter-b-" .. kind .. seed,
        profile = profile_b,
        source_record_slot = slot_b,
        port_actor = { id = "port-b-" .. kind .. seed,
                       combatant = source_combatant(profile_b, slot_b) },
        rom_combatant = source_combatant(profile_b, slot_b),
    }
    local turn_order = { fighter_a, fighter_b }
    table.sort(turn_order, function(left, right)
        return left.source_record_slot < right.source_record_slot
    end)
    local first_slot, second_slot = kind == "party_enemy" and 0 or 6,
                                    kind == "party_enemy" and 6 or 7
    local source_order_matches =
        turn_order[1].source_record_slot == first_slot and
        turn_order[2].source_record_slot == second_slot
    local rom_rng, port_rng = source_rng(seed), source_rng(seed)
    local mismatches, actions = 0, {}
    local party_hurt_pending = false

    local function attack(from, to)
        actions[#actions + 1] = from.source_record_slot
        local action = from.profile.action
        local old_physical = to.port_actor.combatant.physical_meter
        local old_mental = to.port_actor.combatant.mental_meter
        local rom_attack = source_weapon_attack(
            from.rom_combatant, action, rom_rng)
        local context = {
            combatant = from.port_actor.combatant,
            projectile_action_id = action,
            ai_target_candidates = {},
            ai_random_bound = port_rng,
        }
        local port_attack = Combat.resolve_weapon_attack_seed(context)
        if port_attack ~= rom_attack then
            mismatches = mismatches + 1
        end
        local rom_damage = source_weapon_damage(
            to.rom_combatant, action, rom_attack, rom_rng)
        local expected_meter = math.max(
            0, to.rom_combatant.physical_meter - rom_damage)
        to.rom_combatant.physical_meter = expected_meter

        local result = Combat.resolve_weapon_hit(
            context, { attack_successes = port_attack }, to.port_actor)
        if result == nil or result.physical_meter ~= expected_meter or
           result.mental_meter ~= old_mental or result.physical ~= true then
            mismatches = mismatches + 1
        elseif result.changed then
            to.port_actor.combatant = game.actors[to.port_actor.id].combatant
        end

        local meters_nonnegative = result ~= nil and
            result.physical_meter >= 0 and result.mental_meter >= 0 and
            expected_meter >= 0
        check(meters_nonnegative,
              string.format("0x55948 weapon hit made a negative meter (%s, slot %d)",
                            kind, from.source_record_slot))
        check(result ~= nil and result.mental_meter == old_mental and
              to.port_actor.combatant.mental_meter == old_mental,
              string.format("0x55948/0x55A90 conventional hit changed mental +0x9C (%s)",
                            kind))

        local downed, dead = source_lifecycle_downed(
            to.port_actor, result and result.physical_meter or -1,
            result and result.mental_meter or -1)
        check(result ~= nil and downed == (expected_meter == 0) and
              (expected_meter ~= 0 or not dead),
              string.format("0xAFF2/0xB330 down gate disagrees with physical zero (%s, slot %d)",
                            kind, to.source_record_slot))

        if to.port_actor.combatant.is_party_member then
            local pending = game.player_hurt_pending_by_actor_id
            if expected_meter == 0 then
                party_hurt_pending = false
            elseif expected_meter < old_physical then
                party_hurt_pending = true
            end
            check((type(pending) == "table" and
                   pending[to.port_actor.id] == true) ==
                      party_hurt_pending,
                  string.format("0x55A1E party meter branch disagrees with down/hurt state (%s)",
                                kind))
        end
    end

    local rounds = 0
    while fighter_a.rom_combatant.physical_meter > 0 and
          fighter_b.rom_combatant.physical_meter > 0 and rounds < 40 do
        rounds = rounds + 1
        for _, from in ipairs(turn_order) do
            local to = from == fighter_a and fighter_b or fighter_a
            if from.rom_combatant.physical_meter > 0 and
               to.rom_combatant.physical_meter > 0 then
                attack(from, to)
            end
        end
    end

    local turn_order_matches = true
    for index, slot in ipairs(actions) do
        local expected_slot = index % 2 == 1 and first_slot or second_slot
        if slot ~= expected_slot then
            turn_order_matches = false
            break
        end
    end
    local port_a_meter = fighter_a.port_actor.combatant.physical_meter
    local port_b_meter = fighter_b.port_actor.combatant.physical_meter
    local rom_winner = fighter_a.rom_combatant.physical_meter == 0 and "B" or
        (fighter_b.rom_combatant.physical_meter == 0 and "A" or "draw")
    local port_winner = port_a_meter == 0 and "B" or
        (port_b_meter == 0 and "A" or "draw")
    return rom_winner, port_winner, mismatches, #actions, rounds,
           source_order_matches and turn_order_matches
end

-- 0x55640 is shared by the party 0x1514E and world actor 0x14FF8 callers;
-- source record type selects the party/world meter branch at 0x55A1E. The
-- alternating seeded result distribution below is [H] regression-only: it
-- omits source delay clocks and has no emulator-wide distribution anchor.
for _, kind in ipairs({ "enemy_enemy", "party_enemy" }) do
    local rom_outcomes = { A = 0, B = 0, draw = 0 }
    local port_outcomes = { A = 0, B = 0, draw = 0 }
    local mismatches, actions, rounds = 0, 0, 0
    local source_order_matches = true
    for seed = 1, 64 do
        local rom_winner, port_winner, row_mismatches, row_actions,
              row_rounds, row_order_matches = run_fight(kind, seed)
        rom_outcomes[rom_winner] = rom_outcomes[rom_winner] + 1
        port_outcomes[port_winner] = port_outcomes[port_winner] + 1
        mismatches = mismatches + row_mismatches
        actions = actions + row_actions
        rounds = rounds + row_rounds
        source_order_matches = source_order_matches and row_order_matches
    end
    check(port_outcomes.A == rom_outcomes.A and
          port_outcomes.B == rom_outcomes.B and
          port_outcomes.draw == rom_outcomes.draw and mismatches == 0,
          kind .. " source-profile seeded fights differ from the formula model")
    check(source_order_matches,
          kind .. " action sequence violates 0xA8EA source record order")
    print(string.format(
        "%s [H regression-only]: 64 formula-seeded fights, %d actions, " ..
        "%d passes, A/B/draw %d/%d/%d",
        kind, actions, rounds, rom_outcomes.A, rom_outcomes.B,
        rom_outcomes.draw))
end

print(string.format("combat_engine_tests: %d/%d passed", passed, total))
if passed ~= total then
    os.exit(1)
end
