-- rules/clips.lua: Clips page rules 0x11B58 (srx clips_menu_conversion.cpp) [C unless marked]
local E = require("enums")
local C, S = E.ClipsCommand, E.RunnerStatus
local M = { RELOAD_ROUNDS = 0x26 }          -- one clip = 0x26 rounds (same constant as AiTuning.RELOAD_ROUNDS) [C]

function M.cycle(cur, step)
    local c = cur + step
    if c < C.RELOAD then return C.CANCEL elseif c > C.CANCEL then return C.RELOAD end
    return c
end
-- 0x12062 common gate
function M.gate(r, cmd)
    -- ROM TST.B/BGT treats 0x80..0xFF as nonpositive, then clears +0x4F
    -- before returning to 0x11C46; the caller immediately shows Function
    -- Aborted at 0x11C9C. Preserve the byte clear for this signed edge case.
    local reserve = r.clips & 0xFF
    if reserve == 0 or reserve >= 0x80 then
        local cleared = reserve ~= 0
        r.clips = 0
        return E.MenuMessage.FUNCTION_ABORTED, cleared
    end
    if cmd == C.RELOAD and (r.weapon_slot < 0 or r.weapon_code >= 0x0D) then return E.MenuMessage.ITEM_NO_CLIPS end
end
function M.reload(r, cap)
    r.clips = r.clips - 1
    local loaded = r.rounds + M.RELOAD_ROUNDS
    if cap ~= 0 and loaded > cap then loaded = cap end
    r.rounds = loaded & 0xFF
end
function M.recipients(party, source)
    local out = {}
    for i = 0, 2 do
        local r = party[i]
        if i ~= source and r and r.status & S.REMAINDER == 0 and r.status & S.ACTIVE ~= 0 then out[#out + 1] = i end
    end
    return out
end
function M.distribute(party)
    local n, total = 0, 0
    for i = 0, 2 do
        local r = party[i]
        if r and r.status & S.ACTIVE ~= 0 then
            n = n + 1
            total = (total + r.clips) & 0xFF -- source ADD.B at 0x11FFA
        end
    end
    if n < 2 then return false, false end
    if total == 0 then return true, false end -- source branches around writes at 0x12014
    local q, rem = total // n, total % n
    for i = 0, 2 do
        local r = party[i]
        -- The ROM writes the base quotient to all three records; only the
        -- remainder bonus is guarded by status bit 1 (0x12024..0x1203A).
        if r then
            local d = q & 0xFF
            if r.status & S.REMAINDER ~= 0 then d = d + rem end
            r.clips = d & 0xFF
        end
    end
    return true, true
end
return M
