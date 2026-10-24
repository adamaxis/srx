-- karma.lua against ROM 0xD82A branches.
local E = require("enums")
local K = require("karma")
local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end
local function p(...) return K.upgrade_preview_flat(...) end
local A, L = E.KarmaUpgrade.AVAILABLE, E.KarmaUpgrade.AT_LIMIT

local r = p(0, 0, 3, 0, 0, 0)          -- Body 3 -> 4 costs 4
check(r[1] == A and r[2] == 0 and r[3] == 0 and r[4] == 4 and r[5] == 4, "attribute cost = next")
check(p(0, 0, 6, 0, 0, 0)[1] == L, "human body max 6")
check(p(0, 0, 10, 4, 0, 0)[1] == A and p(0, 0, 11, 4, 0, 0)[1] == L, "troll body max 11")
check(p(1, 1, 5, 0, 0, 0)[1] == A and p(1, 1, 5, 0, 0, 0)[3] == 5, "willpower index 5")
check(p(1, 2, 1, 0, 0, 0)[1] == E.KarmaUpgrade.NOT_UPGRADEABLE, "essence fixed")
check(p(1, 3, 1, 0, 0, 0)[1] == E.KarmaUpgrade.NOT_UPGRADEABLE, "magic fixed")
check(p(0, 0, 1, 5, 0, 0)[1] == E.KarmaUpgrade.UNKNOWN_SPECIES, "species 5 unknown")
check(p(0, 4, 0, 0, 2, 0)[1] == E.KarmaUpgrade.CLASS_RESTRICTED, "sorcery needs caster")
r = p(0, 4, 2, 0, 1, 0)
check(r[1] == A and r[2] == 1 and r[3] == 0 and r[4] == 3 and r[5] == 6, "sorcery doubled")
r = p(0, 6, 0, 0, 2, 3)                -- untrained pistols, firearms 3
check(r[1] == A and r[3] == 2 and r[4] == 4 and r[5] == 6, "pistols from firearms, ceil 1.5")
r = p(0, 7, 2, 0, 2, 5)                -- trained smgs ignore firearms
check(r[4] == 3 and r[5] == 5, "smgs ceil(4.5) = 5")
check(p(0, 9, 4, 0, 2, 9)[5] == 10, "melee doubled")
check(p(1, 4, 4, 0, 2, 9)[3] == 6 and p(1, 4, 4, 0, 2, 9)[5] == 10, "throwing (right col) doubled")
check(p(1, 9, 12, 0, 2, 0)[1] == L, "skill limit 12")
r = p(0, 8, 0, 0, 2, 12)               -- 0xD920 tests own value, not Firearms
check(r[1] == A and r[4] == 13 and r[5] == 20, "untrained shotguns with firearms 12")
check(p(2, 0, 0, 0, 0, 0)[1] == E.KarmaUpgrade.INVALID, "bad column")

print(string.format("karma: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
