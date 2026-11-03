-- Hotel Karma upgrade rules from ROM 0xD82A (preview) and 0xD3EA (apply).
-- C++ supplies a semantic runner profile and owns the menu presentation.
local E = require("enums")
local Karma = {}

-- ROM 0xC22A6 supplies six core-attribute ceilings for each race.
Karma.ATTRIBUTE_MAXIMA = {
    [E.RunnerRace.HUMAN] = {
        body = 6, quickness = 6, strength = 6,
        charisma = 6, intelligence = 6, willpower = 6,
    },
    [E.RunnerRace.ELF] = {
        body = 6, quickness = 7, strength = 6,
        charisma = 8, intelligence = 6, willpower = 6,
    },
    [E.RunnerRace.DWARF] = {
        body = 7, quickness = 5, strength = 8,
        charisma = 6, intelligence = 6, willpower = 7,
    },
    [E.RunnerRace.ORC] = {
        body = 9, quickness = 6, strength = 8,
        charisma = 5, intelligence = 5, willpower = 6,
    },
    [E.RunnerRace.TROLL] = {
        body = 11, quickness = 5, strength = 10,
        charisma = 4, intelligence = 4, willpower = 5,
    },
}

-- Grid order from 0xC1A2A, represented as profile fields rather than record
-- positions. Essence and Magic are visible fixed rows, not upgradeable stats.
local ATTRIBUTE_GRID = {
    E.RunnerAttribute.BODY, E.RunnerAttribute.QUICKNESS,
    E.RunnerAttribute.STRENGTH, E.RunnerAttribute.CHARISMA,
    E.RunnerAttribute.INTELLIGENCE, E.RunnerAttribute.WILLPOWER,
    "essence", "magic",
}
local SKILL_GRID = {
    E.RunnerSkill.SORCERY, E.RunnerSkill.FIREARMS,
    E.RunnerSkill.PISTOLS, E.RunnerSkill.SMGS,
    E.RunnerSkill.SHOTGUNS, E.RunnerSkill.MELEE_COMBAT,
    E.RunnerSkill.THROWING, E.RunnerSkill.COMPUTER,
    E.RunnerSkill.BIOTECH, E.RunnerSkill.ELECTRONICS,
    E.RunnerSkill.REPUTATION, E.RunnerSkill.NEGOTIATION,
}
Karma.SKILL_LIMIT = 13 -- ROM 0xD920 rejects the next rank at thirteen.

local function result(status, section, field, next_value, karma_cost)
    return {
        status = status,
        section = section,
        field = field,
        next_value = next_value or 0,
        karma_cost = karma_cost or 0,
        -- The 0x4000 source text mode covers the preview, limit feedback,
        -- and confirmation text on this child screen [C 0xD9B8, 0xD9DE,
        -- 0xD53C]. The native presenter resolves this semantic role to CRAM.
        text_tone = E.MenuTextTone.FOCUSED,
    }
end

-- profile = { race, character_class, attributes = {...}, skills = {...} }
-- Coordinates are the Attributes/Skills screen's two columns and ten rows.
function Karma.upgrade_preview(profile, column, row)
    if column < 0 or column > 1 or row < 0 or row > 9 then
        return result(E.KarmaUpgrade.INVALID)
    end
    if row < 4 then
        local field = ATTRIBUTE_GRID[column * 4 + row + 1]
        if column == 1 and row >= 2 then
            return result(E.KarmaUpgrade.NOT_UPGRADEABLE,
                          "attribute", field)
        end
        local maxima = Karma.ATTRIBUTE_MAXIMA[profile.race]
        if maxima == nil then
            return result(E.KarmaUpgrade.UNKNOWN_RACE,
                          "attribute", field)
        end
        local next_value = profile.attributes[field] + 1
        if next_value > maxima[field] then
            return result(E.KarmaUpgrade.AT_LIMIT, "attribute", field)
        end
        return result(E.KarmaUpgrade.AVAILABLE, "attribute", field,
                      next_value, next_value)
    end

    local skill = SKILL_GRID[column * 6 + row - 3]
    if skill == E.RunnerSkill.SORCERY and
       not E.RunnerClass.CASTERS[profile.character_class] then
        return result(E.KarmaUpgrade.CLASS_RESTRICTED, "skill", skill)
    end

    local current = profile.skills[skill]
    local next_value = current + 1
    if next_value >= Karma.SKILL_LIMIT then
        return result(E.KarmaUpgrade.AT_LIMIT, "skill", skill)
    end
    local reduced_cost = column == 0 and
        (skill == E.RunnerSkill.PISTOLS or
         skill == E.RunnerSkill.SMGS or
         skill == E.RunnerSkill.SHOTGUNS)
    if not reduced_cost then
        return result(E.KarmaUpgrade.AVAILABLE, "skill", skill,
                      next_value, next_value * 2)
    end

    local base = current == 0 and profile.skills.firearms or current
    next_value = base + 1
    return result(E.KarmaUpgrade.AVAILABLE, "skill", skill, next_value,
                  (next_value * 15 + 9) // 10)
end

return Karma
