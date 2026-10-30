-- rules/clips.lua: Clips page rules 0x11B58 (srx clips_menu_conversion.cpp) [C unless marked]
local E = require("enums")
local C = E.ClipsAction
local Items = require("items").by_id
local M = { RELOAD_ROUNDS = 0x26 }          -- one clip = 0x26 rounds (same constant as AiTuning.RELOAD_ROUNDS) [C]

local ACTION_ORDER = { C.RELOAD, C.TRADE, C.DISTRIBUTE, C.CANCEL }
local ACTION_POSITION = {}
for position, action in ipairs(ACTION_ORDER) do
    ACTION_POSITION[action] = position
end

function M.cycle(cur, step)
    local position = ACTION_POSITION[cur]
    position = (position - 1 + step) % #ACTION_ORDER + 1
    return ACTION_ORDER[position]
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
    local weapon = r.equipped_weapon_id and Items[r.equipped_weapon_id]
    if cmd == C.RELOAD and
       (weapon == nil or weapon.kind ~= "firearm") then
        return E.MenuMessage.ITEM_NO_CLIPS
    end
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
        if i ~= source and r and r.runner_facts.lifecycle.active and
           not r.runner_facts.clip_distribution_remainder then
            out[#out + 1] = i
        end
    end
    return out
end
function M.distribute(party)
    local n, total = 0, 0
    for i = 0, 2 do
        local r = party[i]
        if r and r.runner_facts.lifecycle.active then
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
        -- remainder bonus goes to records marked by the source pass
        -- (0x12024..0x1203A); the host exposes that as a named fact.
        if r then
            local d = q & 0xFF
            if r.runner_facts.clip_distribution_remainder then d = d + rem end
            r.clips = d & 0xFF
        end
    end
    return true, true
end
return M
