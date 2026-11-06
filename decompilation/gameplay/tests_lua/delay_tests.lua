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
for race, maxima in pairs(Karma.ATTRIBUTE_MAXIMA) do
    for quickness = 0, maxima.quickness do
        for intelligence = 0, maxima.intelligence do
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
                               "delay mismatch for race %s Quickness %d Intelligence %d modifier %d mode %d",
                               race, quickness, intelligence, modifier,
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

-- The live player bridge passes source +0x90 capability flags. ROM 0xD7EA
-- ignores unrelated low bits, then gives bits 12/13/14 priority in order.
local capability = E.ActionDelayCapability
assert(Delay.player_weapon_action_delay(3, 2, 0x0004, "warhawk",
           use.ATTACK) == 51,
       "unrelated +0x90 bits changed the source pistol delay")
assert(Delay.player_weapon_action_delay(3, 2, capability.FACTOR_BONUS_4,
           "warhawk", use.ATTACK) == 43)
assert(Delay.player_weapon_action_delay(3, 2, capability.FACTOR_BONUS_8,
           "warhawk", use.ATTACK) == 35)
assert(Delay.player_weapon_action_delay(3, 2, capability.FACTOR_BONUS_12,
           "warhawk", use.ATTACK) == 27)
assert(Delay.player_weapon_action_delay(3, 2,
           capability.FACTOR_BONUS_4 + capability.FACTOR_BONUS_8,
           "warhawk", use.ATTACK) == 43,
       "ROM +0x90 capability priority changed")

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

-- The GPGX action capture keeps the finite pose duration separate from the
-- repeat-action gate: Quickness, Intelligence, and Wired Reflexes change the
-- latter, while weapon skill does not. Weapon poses complete on pass 29 and
-- movement resumes on pass 30. Spell poses use passes 57/58. This catches a
-- stale 31/32 interpretation of the C042 samples.
local lockout_file = assert(io.open(
    "decompilation/gameplay/tests_lua/fixtures/post_attack_lockout_gpgx.tsv",
    "r"))
local lockout_headers
local lockout_rows = {}
for line in lockout_file:lines() do
    if line ~= "" and line:sub(1, 1) ~= "#" then
        local values = {}
        for value in (line .. "\t"):gmatch("(.-)\t") do
            values[#values + 1] = value
        end
        if lockout_headers == nil then
            lockout_headers = values
        else
            local row = {}
            for index, name in ipairs(lockout_headers) do
                row[name] = values[index]
            end
            row.pose_complete_pass = assert(
                tonumber(row.pose_complete_pass))
            row.first_movement_pass = assert(
                tonumber(row.first_movement_pass))
            lockout_rows[row.case] = row
            local expected_pose = row.action_kind == "spell" and 57 or 29
            local expected_movement = row.action_kind == "spell" and 58 or 30
            assert(row.pose_complete_pass == expected_pose and
                   row.first_movement_pass == expected_movement,
                   "GPGX pose/movement timing changed for " .. row.case)
        end
    end
end
lockout_file:close()
assert(lockout_rows.pistol_skill0 ~= nil and
       lockout_rows.pistol_skill8 ~= nil and
       lockout_rows.pistol_wr4 ~= nil and
       lockout_rows.pistol_wr8 ~= nil and
       lockout_rows.pistol_wr12 ~= nil and
       lockout_rows.pistol_higher_attributes ~= nil and
       lockout_rows.spell_sorcery0 ~= nil,
       "GPGX lockout capture lost a profile-permutation case")
assert(lockout_rows.pistol_skill0.source_action_gate ==
           lockout_rows.pistol_skill8.source_action_gate and
       lockout_rows.pistol_skill0.pose_complete_pass ==
           lockout_rows.pistol_skill8.pose_complete_pass,
       "weapon skill changed the source action gate or finite pose")
assert(lockout_rows.pistol_wr4.source_action_gate == "43" and
       lockout_rows.pistol_wr8.source_action_gate == "35" and
       lockout_rows.pistol_wr12.source_action_gate == "27" and
       lockout_rows.pistol_wr4.first_movement_pass ==
           lockout_rows.pistol_wr12.first_movement_pass,
       "Wired Reflexes changed the pose duration instead of the action gate")
assert(lockout_rows.pistol_skill0.first_movement_pass ==
           lockout_rows.pistol_higher_attributes.first_movement_pass and
       lockout_rows.spell_sorcery0.first_movement_pass == 58,
       "attribute or spell permutation changed the captured pose boundary")

print("shared actor/player delay calculations: ok")
