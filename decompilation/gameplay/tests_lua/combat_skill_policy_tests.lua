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
        cyberware_flags = 0,
        action_profile_modifier_flags = 0,
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

print(string.format("combat_skill_policy_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
