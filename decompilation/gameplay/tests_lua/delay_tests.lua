local E = require("enums")
local Delay = require("actor.delay")
local Karma = require("karma")
local Mode = E.ActionDelayMode
local Modifier = E.ActionDelayModifier

assert(Delay.calculate(0, 0, 0, Mode.NORMAL) == E.ActionDelay.BASE)
assert(Delay.calculate(0, 0, 0, Mode.HALF) == E.ActionDelay.BASE // 2)
assert(Delay.calculate(0, 0, 0, Mode.DOUBLE) == E.ActionDelay.BASE * 2)
assert(Delay.calculate(0, 0, 0, 3 --[[ synthetic nonzero D0 (asr path) [H] ]]) ==
       E.ActionDelay.BASE // 2)

-- [C] ROM 0xD81C adds the Quickness and Intelligence values before halving.
-- Race caps at 0xC22A6 bound those values to Quickness <= 7 and Intelligence
-- <= 6, so the source storage width never wraps for a legal stat pair. Cover
-- every value up to each race's maxima, each imported modifier, and each mode.
local modes = { Mode.NORMAL, Mode.HALF, Mode.DOUBLE }
local modifiers = {
    Modifier.NONE,
    Modifier.FACTOR_BONUS_4,
    Modifier.FACTOR_BONUS_8,
    Modifier.FACTOR_BONUS_12,
}
for species, maxima in pairs(Karma.ATTRIBUTE_MAXIMA) do
    for quickness = 0, maxima[2] do
        for intelligence = 0, maxima[5] do
            assert(quickness + intelligence <= 13,
                   "race cap unexpectedly exceeds legal delay-stat range")
            for _, modifier in ipairs(modifiers) do
                for _, mode in ipairs(modes) do
                    local factor = math.max(
                        0,
                        math.floor((quickness + intelligence) / 2) +
                            modifier - 1)
                    local expected = E.ActionDelay.BASE -
                        factor * E.ActionDelay.FACTOR_STEP
                    if mode == Mode.DOUBLE then
                        expected = expected * 2
                    elseif mode ~= Mode.NORMAL then
                        expected = math.floor(expected / 2)
                    end
                    expected = math.max(expected, E.ActionDelay.MINIMUM)
                    assert(Delay.calculate(quickness, intelligence, modifier,
                               mode) == expected,
                           string.format(
                               "delay mismatch for race %d Quickness %d Intelligence %d modifier %d mode %d",
                               species, quickness, intelligence, modifier,
                               mode))
                end
            end
        end
    end
end

local first_level_delay =
    Delay.calculate(2, 2, Modifier.FACTOR_BONUS_4, Mode.NORMAL)
local second_level_delay =
    Delay.calculate(2, 2, Modifier.FACTOR_BONUS_8, Mode.NORMAL)
local third_level_delay =
    Delay.calculate(2, 2, Modifier.FACTOR_BONUS_12, Mode.NORMAL)
assert(first_level_delay == 86 and second_level_delay == 70 and
       third_level_delay == 54)

local use = Delay.Use
assert(Delay.player_weapon_action_delay(0, 0, 0, "warhawk",
       use.ATTACK) == E.ActionDelay.BASE // 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, "allegiance",
       use.ATTACK) == E.ActionDelay.BASE)
assert(Delay.player_weapon_action_delay(0, 0, 0, "warhawk",
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, "allegiance",
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, "roomsweeper",
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, "streetline",
       use.RELOAD_ACTION) == E.ActionDelay.BASE)

-- 0x14226 selects D0=2 only for codes 5, 11, 12; the weapon rows are the
-- single source, so check that exactly those rows carry the double scale. [C]
local Weapons = require("data.weapons")
local reload_double_ids = {
    warhawk = true,
    allegiance = true,
    roomsweeper = true,
}
local firearm_count = 0
for id, weapon in pairs(Weapons.by_id) do
    if weapon.fire_mode ~= nil then
        firearm_count = firearm_count + 1
        assert((weapon.reload_delay_scale == "double") ==
                   (reload_double_ids[id] == true),
               "unexpected reload scale for " .. id)
    end
end
assert(firearm_count == 12, "weapon delay coverage missed a firearm row")

print("shared actor/player delay calculations: ok")
