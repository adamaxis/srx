local E = require("enums")
local Delay = require("actor.delay")
local Mode = E.ActionDelayMode
local Capability = E.DelayCapability
local byte_max = 0xFF -- 0xD81C MOVE.B inputs [C]

assert(Delay.calculate(0, 0, 0, Mode.NORMAL) == E.ActionDelay.BASE)
assert(Delay.calculate(0, 0, 0, Mode.HALF) == E.ActionDelay.BASE // 2)
assert(Delay.calculate(0, 0, 0, Mode.DOUBLE) == E.ActionDelay.BASE * 2)
assert(Delay.calculate(0, 0, 0, 3 --[[ synthetic nonzero D0 (asr path) [H] ]]) ==
       E.ActionDelay.BASE // 2)

assert(Delay.calculate(byte_max, byte_max, 0, Mode.NORMAL) ==
       E.ActionDelay.MINIMUM)
-- The +0x78 and +0x7B byte reads wrap before their average. [C]
assert(Delay.calculate(byte_max, 1, 0, Mode.NORMAL) ==
       E.ActionDelay.BASE)

local both_first_two = (1 << Capability.FIRST_BIT) |
    (1 << Capability.SECOND_BIT)
local both_second_three = (1 << Capability.SECOND_BIT) |
    (1 << Capability.THIRD_BIT)
local first_bit_delay = Delay.calculate(2, 2,
    1 << Capability.FIRST_BIT, Mode.NORMAL)
local second_bit_delay = Delay.calculate(2, 2,
    1 << Capability.SECOND_BIT, Mode.NORMAL)
local third_bit_delay = Delay.calculate(2, 2,
    1 << Capability.THIRD_BIT, Mode.NORMAL)
assert(first_bit_delay > second_bit_delay and
       second_bit_delay > third_bit_delay)
-- D7EA keeps the first enabled capability in the 12, 13, 14 scan. [C]
assert(Delay.calculate(2, 2, both_first_two, Mode.NORMAL) == first_bit_delay)
assert(Delay.calculate(2, 2, both_second_three, Mode.NORMAL) == second_bit_delay)

local use = Delay.Use
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.WARHAWK,
       use.ATTACK) == E.ActionDelay.BASE // 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.ALLEGIANCE,
       use.ATTACK) == E.ActionDelay.BASE)
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.WARHAWK,
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.ALLEGIANCE,
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.ROOMSWEEPER,
       use.RELOAD_ACTION) == E.ActionDelay.BASE * 2)
assert(Delay.player_weapon_action_delay(0, 0, 0, E.ItemId.STREETLINE_SPECIAL,
       use.RELOAD_ACTION) == E.ActionDelay.BASE)

-- 0x14226 selects D0=2 only for codes 5, 11, 12; the weapon rows are the
-- single source, so check that exactly those rows carry the double scale. [C]
local Weapons = require("data.weapons")
local reload_double = { [E.ItemId.WARHAWK] = true, [E.ItemId.ALLEGIANCE] = true,
                        [E.ItemId.ROOMSWEEPER] = true }
for _, code in pairs(E.ItemId) do
    local weapon = Weapons.get(code)
    if weapon ~= nil then
        assert((weapon.reload_delay_scale == "double") == (reload_double[code] == true))
    end
end

print("shared actor/player delay calculations: ok")
