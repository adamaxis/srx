-- rules/first_aid.lua: ROM 0x1253E..0x125F8 medkit and slap-patch effects.
local E = require("enums")
local S = E.RunnerStatus
local M = { REVIVED_STATE = E.StateFn.IDLE }   -- continuation 0xB45C after treatment

local function add(v, d) local c = v + d; if c <= 0 then return 0 end; return math.min(c, 10) end
local function phys(t, d) if t.status & S.ACTIVE == 0 then t.physical = 0 else t.physical = add(t.physical, d) end end
local function mental(t, d)
    if t.status & S.ACTIVE == 0 then t.stun = 0; return end
    local c = t.stun + d
    if c > 0 then t.stun = math.min(c, 10); return end
    phys(t, c); t.stun = 0
end
local function revive(t) t.status = (t.status & ~(S.DOWN | S.DEAD)) | S.ACTIVE end
function M.difficulty(t)
    local d = t.physical >= 7 and 4 or (t.physical >= 4 and 6 or 8)
    local red = t.attributes[0] // 3                          -- Body / 3
    d = d > red and d - red or 0
    return math.max(d, 2)
end
function M.targets(party)
    local out = {}
    for i = 0, 2 do if party[i] and party[i].status & S.ACTIVE ~= 0 then out[#out + 1] = i end end
    return out
end
function M.cursor(row, n_active, d)            -- rows = targets + Cancel
    local rows = n_active + 1
    if row >= rows then row = 0 end
    if d < 0 then return row == 0 and rows - 1 or row - 1 end
    if d > 0 then return (row + 1) % rows end
    return row
end
local function consume(user, slot)
    if user.phase[slot] > 1 then user.phase[slot] = user.phase[slot] - 1 else user.inventory[slot] = 0; user.phase[slot] = 0 end
end
-- returns message (E.MenuMessage) or nil when the item cannot be used
function M.use(user, slot, t, roll)
    local code, ph = user.inventory[slot], user.phase[slot]
    if code < 0x10 or code > 0x12 or ph == 0 then return nil end
    if code == 0x10 then
        if t.physical == 10 and t.stun == 10 then return E.MenuMessage.NOT_INJURED end
        local s = roll(user.skills[E.Skill.BIOTECH], M.difficulty(t))
        local d = s == 0 and 1 or s
        phys(t, d); mental(t, d); revive(t); t.state_function = M.REVIVED_STATE
        consume(user, slot); return E.MenuMessage.FIRST_AID_USED
    end
    if code == 0x11 then
        if t.stun == 10 then return E.MenuMessage.NOT_INJURED end
        mental(t, 2); if t.physical ~= 0 then revive(t) end
    else
        if t.physical == 10 then return E.MenuMessage.NOT_INJURED end
        phys(t, 2); if t.stun ~= 0 then revive(t) end
    end
    consume(user, slot); return E.MenuMessage.SLAP_PATCH_USED
end
return M
