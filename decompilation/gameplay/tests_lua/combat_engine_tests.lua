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
local ActorAI = require("actor.init")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        passed = passed + 1
    else
        print("FAIL " .. label)
    end
end

local function clear_actor_states()
    for actor_id in pairs(game.actors) do
        game.actors[actor_id] = nil
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

local function source_rng_with_state(seed)
    local function draw(bound)
        -- 0x0D44 advances the seed before 0x0D3A returns the bounded value.
        seed = (seed * 0x20E5 + 1) & 0xFFFF
        return (seed * bound) >> 16
    end
    return draw, function() return seed end
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

local function p46t_weapon_hit(attacker, target, seed)
    local random_bound, seed_after = source_rng_with_state(seed)
    local context = {
        combatant = attacker,
        projectile_action_id = "model_101t",
        ai_target_candidates = {},
        ai_random_bound = random_bound,
    }
    local attack_successes = Combat.resolve_weapon_attack_seed(context)
    local hit = attack_successes ~= nil and Combat.resolve_weapon_hit(
        context, { attack_successes = attack_successes }, target) or nil
    return attack_successes, hit, seed_after()
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
    clear_actor_states()
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
    clear_actor_states()
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

-- The GPGX fixture measures first movement after a forced source action, not
-- just the independent +0xDC repeat-action gate. Each row drives the live
-- player heartbeat for the captured number of field behavior passes.
local function read_post_attack_lockout_fixture()
    local path = "decompilation/gameplay/tests_lua/fixtures/" ..
                 "post_attack_lockout_gpgx.tsv"
    local stream = io.open(path, "r")
    if stream == nil then
        return nil
    end
    local headers, rows
    rows = {}
    for line in stream:lines() do
        if line ~= "" and line:sub(1, 1) ~= "#" then
            local fields = {}
            for value in (line .. "\t"):gmatch("(.-)\t") do
                fields[#fields + 1] = value
            end
            if headers == nil then
                headers = fields
            else
                local row = {}
                for index, key in ipairs(headers) do
                    row[key] = fields[index]
                end
                rows[#rows + 1] = row
            end
        end
    end
    stream:close()
    return rows
end

local function lockout_player_context(actor_id, phase, spawn)
    return {
        actor_id = actor_id,
        profile_id = "field_runner_k03_none00",
        visual_profile_id = E.ActorVisualProfile.FIELD_RUNNER,
        action_kind = "none",
        action_code = 0,
        phase = phase,
        initial_bearing_degrees = 0,
        entry_delay = 0,
        spawn = spawn,
        source_record_slot = 1,
        has_live_actor_record = true,
        controlled_party_member_id = "party.runner.1",
        is_controlled_party_actor = true,
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = false,
                ai_runner = false, downed = false, group_leader = true,
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
        movement_allowed = false,
        movement_input_direction = "east",
        combat_action_available = false,
        combat_action_ready = false,
        combat_confirm_held = false,
        combat_input_blocked = false,
    }
end

local lockout_fixture = read_post_attack_lockout_fixture()
check(type(lockout_fixture) == "table" and #lockout_fixture >= 15,
      "post-attack lockout fixture contains the measured weapon/spell matrix")
if type(lockout_fixture) == "table" then
    for _, row in ipairs(lockout_fixture) do
        clear_actor_states()
        local actor_id = "post_attack_lockout." .. row.case
        local context = lockout_player_context(actor_id, "behavior", true)
        ActorAI.heartbeat(context)
        context.spawn = false
        context.movement_allowed = true
        context.combat_action_ready = true
        context.combat_confirm_held = true
        if row.action_kind == "weapon" then
            context.combat_action_available = true
            context.player_weapon_action_will_fire = true
        else
            context.player_spell_available = true
            context.player_spell_id = "mana_zap"
            context.final_battle_active = true
        end

        local action_decision = ActorAI.heartbeat(context)
        local measured_lockout = tonumber(row.first_movement_pass)
        local gate = tonumber(row.source_action_gate)
        local observed_gate_ready =
            tonumber(row.observed_gate_ready_pass)
        check(action_decision ~= nil and measured_lockout ~= nil and gate ~= nil,
              row.case .. " fixture row parses action, lock, and +0xDC")
        if action_decision ~= nil and measured_lockout ~= nil then
            check(action_decision.movement_bearing_degrees == nil,
                  row.case .. " action pass suppresses held east movement")
            context.combat_action_available = false
            context.player_weapon_action_will_fire = false
            context.player_spell_available = false
            context.combat_action_ready = false
            context.combat_confirm_held = false
            for pass_number = 1, measured_lockout do
                local decision = ActorAI.heartbeat(context)
                local should_move = pass_number == measured_lockout
                check(decision ~= nil and
                      (should_move
                          and decision.movement_bearing_degrees == 90
                          or not should_move and
                             decision.movement_bearing_degrees == nil),
                      string.format("%s GPGX pass %d movement gate", row.case,
                                    pass_number))
            end
            local next_action = tonumber(row.next_action_pass)
            local movement_before_gate =
                tonumber(row.movement_before_gate_ready) == 1
            check(next_action == gate + 1 and
                  observed_gate_ready == next_action and
                  movement_before_gate == (measured_lockout < next_action),
                  row.case .. " pose lock remains distinct from +0xDC gate")
        end
    end
end

-- The emulator-backed fixture below asserts the complete action stream against
-- production Lua. Keep this direct anchor readable: seed 0001 measured 2
-- attack successes, 0 defense successes, one physical damage, mental unchanged.
do
    clear_actor_states()
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

-- Playtest #46T: source-controlled gun damage against the default pedestrian.
-- ROM 0x7291A supplies the selected runner's Body/attributes/skills/posture
-- and item code +0x66=2; E.ItemId.MODEL_101T is 2 and weapon row 0xD4570
-- supplies effect 1 / TN 6. Pedestrian template 0x1D6988 supplies Body 2,
-- Q/I/W 1/2/2, posture 2, physical 10, and armor index 0. The ROM pool
-- formula is cited at 0x55640/0x12C74; hit/defense/effect at 0x55948..0x559F0;
-- armor at 0x55E92..0x55EB2; physical decrement/clamp at 0x55A90..0x55A98.
check(E.ItemId.MODEL_101T == 0x02,
      "ROM item code 2 must remain E.ItemId.MODEL_101T for #46T")

local function p46t_jake(options)
    options = options or {}
    options.party = true
    options.action = "model_101t"
    return make_combatant(options)
end

local function p46t_pedestrian(id)
    return {
        id = id,
        combatant = make_combatant({
            body = 2, quickness = 1, intelligence = 2, willpower = 2,
            posture_reduction_percent = 50,
            physical_meter = 10, mental_meter = 10,
        }),
    }
end

-- The fire frame's render service creates the projectile before the behavior
-- service can roll. ROM 0x15012 still requires 0x55640. The host passes
-- projectile_attack_seed_pending on that first behavior service after render
-- init has already cleared spawn. Samurai 0x7291A is Body 4, Q 3, Str 4,
-- Int 2, Will 4, Firearms 4, Pistols 0, posture 2, item code 2.
do
    clear_actor_states()
    local attacker = p46t_jake({
        body = 4, quickness = 3, strength = 4, intelligence = 2,
        willpower = 4, firearms = 4, pistols = 0,
        posture_reduction_percent = 50,
    })
    local target = p46t_pedestrian("p46t-live-boundary-pedestrian")
    local expected_attack = p46t_weapon_hit(attacker, target, 0xDF65)
    local actor_id = "p46t-live-boundary-projectile"
    local function base_context(phase)
        return {
            actor_id = actor_id,
            profile_id = "weapon_projectile",
            action_kind = "none",
            phase = phase,
            initial_bearing_degrees = 0,
            entry_delay = 0,
            projectile_action_id = "model_101t",
            combatant = attacker,
            actor_facts = {
                lifecycle = {
                    active = true, thinking_enabled = false,
                    ai_runner = false, downed = false,
                    group_leader = false, dead = false,
                },
                interaction = {},
                behavior = { escort = false },
                roles = {},
            },
        }
    end
    local render_context = base_context("render")
    render_context.spawn = true
    render_context.projectile_attack_seed_pending = false
    render_context.projectile_animation = { frame_count = 3 }
    local rendered = ActorAI.heartbeat(render_context)
    local behavior_context = base_context("behavior")
    behavior_context.spawn = false
    behavior_context.projectile_attack_seed_pending = true
    behavior_context.ai_random_bound = source_rng(0xDF65)
    behavior_context.projectile_animation = { frame_count = 3 }
    behavior_context.projectile_movement = {
        role = "primary",
        phase = "awaiting_animation",
        impact_occurred = false,
        animation_finished = false,
        position = { x = 100, y = 100 },
        velocity = { x = 0, y = 0 },
        remaining_flight_range = 960,
        shots_remaining = 0,
        bearing_degrees = 0,
        attack_successes = 0,
        burst_origin = { x = 100, y = 100 },
        animation_action = "unchanged",
    }
    local behaved = ActorAI.heartbeat(behavior_context)
    local published = behaved and behaved.projectile_movement and
        behaved.projectile_movement.attack_successes
    check(rendered ~= nil and expected_attack == 3 and published == 3,
          "live projectile behavior pass must roll 0x55640 after render init")
    clear_actor_states()
end

-- Measured GPGX logic-only seed chain, re-run against production Lua before
-- this permanent test was added. Each attack+defense pair consumes ten draws.
-- 0x2577's raw effect is 2 but only one point is removed because 0x55A90
-- clamps the pedestrian's physical meter at zero.
do
    clear_actor_states()
    local attacker = p46t_jake({
        body = 4, quickness = 3, strength = 4, intelligence = 2,
        willpower = 4, firearms = 4, pistols = 0,
        posture_reduction_percent = 50,
    })
    local target = p46t_pedestrian("p46t-seed-chain-pedestrian")
    local meter = 10
    for _, row in ipairs({
        { seed = 0xDF65, attack = 3, damage = 3, meter = 7,
          end_seed = 0x861B },
        { seed = 0x861B, attack = 1, damage = 1, meter = 6,
          end_seed = 0x5BA1 },
        { seed = 0x5BA1, attack = 4, damage = 5, meter = 1,
          end_seed = 0x2577 },
        { seed = 0x2577, attack = 1, damage = 2, meter = 0,
          end_seed = 0x9D1D },
    }) do
        local raw_target = p46t_pedestrian(
            string.format("p46t-raw-seed-%04X", row.seed))
        local raw_attack, raw_hit, raw_end_seed = p46t_weapon_hit(
            attacker, raw_target, row.seed)
        local raw_damage = raw_hit ~= nil and
            10 - raw_hit.physical_meter or -1
        target.combatant.physical_meter = meter
        local attack, hit, end_seed = p46t_weapon_hit(
            attacker, target, row.seed)
        local measured_damage = hit ~= nil and
            meter - hit.physical_meter or -1
        check(raw_attack == row.attack and raw_damage == row.damage and
              raw_hit ~= nil and raw_hit.mental_meter == 10 and
              raw_end_seed == row.end_seed and attack == row.attack and
              hit ~= nil and measured_damage == math.min(row.damage, meter) and
              hit.physical_meter == row.meter and hit.mental_meter == 10 and
              end_seed == row.end_seed,
              string.format("#46T seed %04X must match ROM attack/damage/meter",
                            row.seed))
        meter = hit ~= nil and hit.physical_meter or meter
    end
end

-- Additional hand-derived cases exercise the same ROM equations with
-- controlled seeds/stat variants. Dice below are the successive bounded d6
-- faces from 0x0D44/0x0D3A; a six explodes per 0x0DBA..0x0DFA. The pedestrian
-- defense pool remains floor((1+2+2)/2)-floor(2*50/100)+Body2 = 3 at TN6.
-- 0x0437, source runner stats: attack pool 4+min(4,2)=6; attack dice
-- 4,2,4,6+3,6+3,2 yield 4 successes at TN4; defense 2,1,2 yields 0.
-- Thus effect 1+4-0=5, no armor, meter 10->5.
-- 0x1234, test stats Q/I/W=2/3/3, Firearms/Pistols=2/0, posture=25%:
-- average4, posture share1, attack pool3; dice 5,6+3,6+3 yield 3
-- successes; defense 3,6+1,3 yields 1; effect 1+3-1=3, meter 10->7.
-- 0x4567, test stats Q/I/W=3/3/3, Firearms/Pistols=3/0, posture=25%:
-- average4, share1, pool4; dice 6+5,4,2,5 yield 3 successes; defense
-- 2,2,3 yields 0; effect 1+3-0=4, meter 10->6.
-- 0xBEEF, test stats Q/I/W=5/4/3, Firearms/Pistols=7/4, posture=75%:
-- average6, share4, pool11; dice 5,2,2,4,3,6+2,2,1,2,6+6+4,1 yield 4
-- successes; defense 1,4,3 yields 0; effect 1+4-0=5, meter 10->5.
-- These variants are formula probes, not claims about additional ROM actors.
for _, row in ipairs({
    { seed = 0x0437, attacker = {
        body = 4, quickness = 3, intelligence = 2, willpower = 4,
        firearms = 4, pistols = 0, posture_reduction_percent = 50,
    }, attack = 4, damage = 5, meter = 5, end_seed = 0x2EB2 },
    { seed = 0x1234, attacker = {
        body = 2, quickness = 2, intelligence = 3, willpower = 3,
        firearms = 2, pistols = 0, posture_reduction_percent = 25,
    }, attack = 3, damage = 3, meter = 7, end_seed = 0x76DD },
    { seed = 0x4567, attacker = {
        body = 3, quickness = 3, intelligence = 3, willpower = 3,
        firearms = 3, pistols = 0, posture_reduction_percent = 25,
    }, attack = 3, damage = 4, meter = 6, end_seed = 0x797F },
    { seed = 0xBEEF, attacker = {
        body = 5, quickness = 5, intelligence = 4, willpower = 3,
        firearms = 7, pistols = 4, posture_reduction_percent = 75,
    }, attack = 4, damage = 5, meter = 5, end_seed = 0x6DBC },
}) do
    clear_actor_states()
    local attacker = p46t_jake(row.attacker)
    local target = p46t_pedestrian(string.format("p46t-case-%04X", row.seed))
    local attack, hit, end_seed = p46t_weapon_hit(attacker, target, row.seed)
    local damage = hit ~= nil and 10 - hit.physical_meter or -1
    check(attack == row.attack and damage == row.damage and hit ~= nil and
          hit.physical_meter == row.meter and hit.mental_meter == 10 and
          end_seed == row.end_seed,
          string.format("#46T controlled formula case %04X", row.seed))
end

-- Exhaust every 16-bit direct seed through the production attack and hit
-- resolvers. ROM 0x559D0..0x559F0 bounds pre-armor effect to 0..10;
-- 0x55A02..0x55A12 and 0x55A90..0x55A98 cannot increase it or make a meter
-- decrement exceed the meter. This unarmored, 10-point pedestrian therefore
-- must take an integer 0..10 physical damage for every seed.
do
    clear_actor_states()
    local attacker = p46t_jake({
        body = 4, quickness = 3, strength = 4, intelligence = 2,
        willpower = 4, firearms = 4, pistols = 0,
        posture_reduction_percent = 50,
    })
    local target = p46t_pedestrian("p46t-all-seeds-pedestrian")
    local invalid = 0
    for seed = 0, 0xFFFF do
        target.combatant.physical_meter = 10
        target.combatant.mental_meter = 10
        local attack, hit = p46t_weapon_hit(attacker, target, seed)
        if attack == nil or hit == nil or
           hit.physical_meter < 0 or hit.physical_meter > 10 or
           hit.mental_meter ~= 10 then
            invalid = invalid + 1
        else
            local damage = 10 - hit.physical_meter
            if damage < 0 or damage > 10 or damage ~= math.floor(damage) then
                invalid = invalid + 1
            end
        end
    end
    check(invalid == 0,
          string.format("#46T damage must stay in ROM range 0..10 for all 65536 seeds (invalid=%d)",
                        invalid))
end

print(string.format("combat_engine_tests: %d/%d passed", passed, total))
if passed ~= total then
    os.exit(1)
end
