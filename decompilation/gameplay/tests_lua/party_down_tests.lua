-- party_down.lua: ROM 0xB330/0xABB6 runner down and 0xC64C/0xC716 wipe.
local P = require("party_down")

local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

-- Leader alone: down sets 0x10, kind 3 sound 0x2E, C0B2 selector, wipe 0x3C.
local r = P.runner_down_flat(0, 0x01, 0x00, 0x00, 3, 0x4000)
check(r[1] == 0x11, "leader DOWN flag")
check(r[2] == 0x2E, "B05E kind 3 sound")
check(r[3] == 0xE8 + 2 * 4, "C0B2 right selector")
check(r[4] == 0x3C, "lone leader starts FFE166")
-- 0xB35C rounding: 0x1000 rounds to the next direction, 0xF000 wraps to 0.
check(P.runner_down_flat(0, 1, 0, 0, 3, 0x1000)[3] == 0xEC, "heading round up")
check(P.runner_down_flat(0, 1, 0, 0, 3, 0xF000)[3] == 0xE8, "heading wrap")
-- A live hired runner blocks the wipe.
check(P.runner_down_flat(0, 0x01, 0x01, 0x00, 3, 0)[4] == 0, "live hired blocks wipe")
check(P.runner_down_flat(0, 0x01, 0x41, 0x00, 3, 0)[4] == 0x3C, "dead hired allows wipe")
-- A hired runner going down never wipes: the leader lacks bit6 [C].
check(P.runner_down_flat(1, 0x11, 0x01, 0x00, 3, 0)[4] == 0, "hired down no wipe")

-- Wipe: leader revived with floors, hired dismissed, nuyen penalty.
local w = P.resolve_wipe_flat(1000, 0, 0x11, 0, 3, 0x01, 5, 5, 0x00, 0, 0)
check(w[1] == 0x01, "leader flags cleared and active")
check(w[2] == 6 and w[3] == 8, "meter floors 6/8")
check(w[4] == 1 and w[5] == 0, "active hired dismissed")
check(w[6] == 875, "nuyen minus 1/8")
check(w[7] == P.WAKE.LITTLE_CHIBA, "bit0 clear wakes in Little Chiba")
local cap = P.resolve_wipe_flat(100000, 1, 0x51, 9, 10, 0, 0, 0, 0, 0, 0)
check(cap[6] == 95000, "penalty capped at 5000")
check(cap[2] == 9 and cap[3] == 10, "meters above floor kept")
check(cap[7] == P.WAKE.DOCWAGON, "bit0 set calls DocWagon")

print(string.format("party_down: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
