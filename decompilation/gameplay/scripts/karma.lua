-- Hotel Karma screen upgrade rules, ROM 0xD82A (preview) / 0xD3EA (apply) [C].
-- C++ owns the grid layout, cursor, prompt and record writes; Lua owns which
-- cell can be raised, to what value, and for how much Karma.
local E = require("enums")
local Karma = {}

-- 0xC22A6: one 6-byte row per species (+0x9A) of attribute maxima for
-- Body, Quickness, Strength, Charisma, Intelligence, Willpower [C].
Karma.ATTRIBUTE_MAXIMA = {
    [0] = { 6, 6, 6, 6, 6, 6 },
    [1] = { 6, 7, 6, 8, 6, 6 },
    [2] = { 7, 5, 8, 6, 6, 7 },
    [3] = { 9, 6, 8, 5, 5, 6 },
    [4] = { 11, 5, 10, 4, 4, 5 },
}
Karma.ATTRIBUTE_ROWS = 4          -- 0xD852: FFF0D1 < 4 = attribute cell
Karma.UPGRADEABLE_ATTRIBUTES = 6  -- 0xD876: index >= 6 (Essence, Magic) is fixed
Karma.SKILL_LIMIT = 0x0D          -- 0xD920: own value + 1 >= 0x0D is maximum level

-- upgrade_preview_flat(column, row, current, species, archetype, firearms)
-- -> { status (E.KarmaUpgrade), section (0 attribute / 1 skill),
--      value_index, next_value, karma_cost }
function Karma.upgrade_preview_flat(column, row, current, species, archetype, firearms)
    local function out(status, section, index, next_value, cost)
        return { status, section or 0, index or 0, next_value or 0, cost or 0 }
    end
    if column < 0 or column > 1 or row < 0 or row > 9 then
        return out(E.KarmaUpgrade.INVALID)
    end
    if row < Karma.ATTRIBUTE_ROWS then
        local index = column * 4 + row                       -- 0xD860
        if index >= Karma.UPGRADEABLE_ATTRIBUTES then
            return out(E.KarmaUpgrade.NOT_UPGRADEABLE, 0, index)
        end
        local maxima = Karma.ATTRIBUTE_MAXIMA[species]
        if maxima == nil then
            return out(E.KarmaUpgrade.UNKNOWN_SPECIES, 0, index)
        end
        local next_value = current + 1
        if next_value > maxima[index + 1] then               -- 0xD8B4 BGT
            return out(E.KarmaUpgrade.AT_LIMIT, 0, index)
        end
        return out(E.KarmaUpgrade.AVAILABLE, 0, index, next_value, next_value)
    end
    local skill_row = row - Karma.ATTRIBUTE_ROWS
    local index = column * 6 + skill_row                     -- 0xD8C8
    if index == E.Skill.SORCERY and archetype >= E.Archetype.CASTER_LIMIT then
        return out(E.KarmaUpgrade.CLASS_RESTRICTED, 1, index) -- 0xD8EA +0x9B
    end
    if current + 1 >= Karma.SKILL_LIMIT then                 -- 0xD920, own value
        return out(E.KarmaUpgrade.AT_LIMIT, 1, index)
    end
    -- 0xD928..0xD99A: left-column Pistols/SMGs/Shotguns cost ceil(1.5 * next);
    -- an untrained one starts from Firearms + 1. The row-6 (+0x85) branch at
    -- 0xD968 is unreachable because row - 4 never exceeds 5.
    local reduced = column == 0 and index >= E.Skill.PISTOLS and index <= E.Skill.SHOTGUNS
    if not reduced then
        return out(E.KarmaUpgrade.AVAILABLE, 1, index, current + 1, (current + 1) * 2)
    end
    local base = current == 0 and firearms or current
    local next_value = base + 1
    return out(E.KarmaUpgrade.AVAILABLE, 1, index, next_value, (next_value * 15 + 9) // 10)
end

return Karma
