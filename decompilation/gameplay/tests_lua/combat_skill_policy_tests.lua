-- ROM 0xD920 caps trained skills at 12; the untrained specialty path at
-- 0xD928..0xD99A can start at Firearms + 1, making 13 the legal maximum.
-- Exercise every legal pair through the public weapon-action resolver.
local Combat = require("combat")
local E = require("enums")
local Karma = require("karma")
local postures = {
    E.Posture.FULL_DEFENSE,
    E.Posture.MEDIUM_DEFENSE,
    E.Posture.NEUTRAL,
    E.Posture.MEDIUM_OFFENSE,
    E.Posture.FULL_OFFENSE,
}
game = {
    actors = {},
    modules = {
        combat = Combat,
        items = { catalog = {} },
        magic = {},
    },
}

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function combatant(firearms, pistols, maxima, posture)
    return {
        physical_meter = 10,
        mental_meter = 10,
        posture_reduction_percent = posture * 25,
        cyberware = {},
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
        action_profile_options = { primary = false, secondary = false },
        attributes = {
            body = 0,
            quickness = maxima[2],
            strength = 0,
            intelligence = maxima[5],
            willpower = maxima[6],
        },
        skills = {
            firearms = firearms,
            pistols = pistols,
        },
        actor_facts = {
            lifecycle = { active = true },
            interaction = {},
        },
        inventory = {},
        weapon_action_policies = {
            test_sidearm = {
                skill_group = "pistols",
                difficulty_adjustment = 0,
                benefits_from_targeting_assist = false,
            },
        },
    }
end

for species, maxima in pairs(Karma.ATTRIBUTE_MAXIMA) do
    local command_average = math.floor(
        (maxima[2] + maxima[5] + maxima[6]) / 2)
    for _, posture in ipairs(postures) do
        local posture_component = math.floor(
            command_average * posture * 25 / 100)
        local function check_pair(firearms, pistols)
            local result = Combat.resolve_weapon_attack_seed({
                combatant = combatant(
                    firearms, pistols, maxima, posture),
                projectile_action_id = "test_sidearm",
                ai_random_bound = function(bound)
                    if bound ~= 6 then return nil end
                    return 4 -- face 5 succeeds at the source minimum difficulty.
                end,
            })
            local selected = math.max(firearms, pistols)
            local expected = math.max(
                selected + math.min(selected, posture_component), 1)
            check(result == expected,
                  string.format(
                      "species %d posture %d Firearms %d / Pistols %d -> %s (expected %d)",
                      species, posture, firearms, pistols,
                      tostring(result), expected))
        end
        for firearms = 0, Karma.SKILL_LIMIT - 1 do
            for pistols = 0, Karma.SKILL_LIMIT - 1 do
                check_pair(firearms, pistols)
            end
        end
        -- The untrained-specialty path can start Pistols one above the
        -- trained-skill cap when Firearms is already at 12.
        check_pair(Karma.SKILL_LIMIT - 1, Karma.SKILL_LIMIT)
    end
end

local function reaction_combatant(strength)
    return {
        record_type = 3,
        physical_meter = 10,
        mental_meter = 10,
        posture_reduction_percent = 0,
        cyberware = {},
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        action_profile_options = { primary = false, secondary = false },
        attributes = {
            body = 2,
            quickness = 2,
            strength = strength,
            intelligence = 2,
            willpower = 2,
        },
        skills = { melee_combat = 1 },
        actor_facts = {
            lifecycle = { active = true },
            interaction = {
                no_coverage_difficulty_bonus = false,
                defense_difficulty_reduction = false,
            },
        },
        inventory = {},
    }
end

local function reaction_with_profile(use_profile)
    local attacker = reaction_combatant(5)
    attacker.action_profile_options.primary = use_profile
    local defender = reaction_combatant(0)
    if use_profile then
        defender.dynamic_action_profile = {
            primary = {
                roll_count = 3,
                difficulty = 1,
                attribute = E.ActorAttribute.BODY,
            },
        }
    end
    local target = { id = "profile_target", combatant = defender }
    return Combat.resolve_reaction({
        combatant = attacker,
        ai_target_candidates = {},
        ai_random_bound = function(bound)
            if bound ~= 6 then return nil end
            return 3 -- face 4 passes difficulty 4 and fails difficulty 5.
        end,
    }, target)
end

local baseline_reaction = reaction_with_profile(false)
local profile_reaction = reaction_with_profile(true)
check(baseline_reaction ~= nil and baseline_reaction.mental_meter == 8 and
      profile_reaction == nil,
      "linked named profile data was not applied to the primary action rule")

local armor_catalog = {
    {
        id = "test_weapon",
        kind = "firearm",
        weapon = { source_action = { difficulty = 2, effect_value = 1 } },
    },
    {
        id = "heavy_combat_armor",
        kind = "armor",
        armor = { primary_modifier = 8, secondary_modifier = 6 },
    },
}
game.modules.items.catalog = armor_catalog

local function armor_hit(equipped_armor_id, target_id)
    local target = {
        id = target_id,
        combatant = {
            record_type = 3,
            physical_meter = 10,
            mental_meter = 10,
            posture_reduction_percent = 0,
            equipped_armor_id = equipped_armor_id,
            attributes = {
                body = 1,
                quickness = 2,
                strength = 1,
                intelligence = 2,
                willpower = 2,
            },
            skills = {},
            cyberware = {},
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
                    active = true,
                    thinking_enabled = true,
                    ai_runner = false,
                    downed = false,
                    group_leader = false,
                    dead = false,
                },
                interaction = {
                    no_coverage_difficulty_bonus = true,
                    defense_difficulty_reduction = false,
                },
            },
            inventory = {},
            action_profile_options = { primary = false, secondary = false },
        },
    }
    return Combat.resolve_weapon_hit({
        projectile_action_id = "test_weapon",
        ai_random_bound = function() return 0 end,
    }, { action_result = 5 }, target)
end

local unarmored_hit = armor_hit(nil, "unarmored_target")
local armored_hit = armor_hit("heavy_combat_armor", "armored_target")
check(unarmored_hit ~= nil and unarmored_hit.physical_meter == 4 and
      armored_hit ~= nil and armored_hit.physical_meter == 10,
      "weapon resolution did not select armor modifiers by item identity")

print(string.format("combat_skill_policy_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
