-- rules/first_aid.lua: ROM 0x1253E..0x125F8 medkit and slap-patch effects.
local E = require("enums")
local Items = require("items").by_id
local Inventory = require("menu.rules.inventory")
local M = { REVIVED_CONTINUATION = E.RunnerContinuation.IDLE }

local function add(v, d) local c = v + d; if c <= 0 then return 0 end; return math.min(c, 10) end
local function active(t) return t.runner_facts.lifecycle.active end
local function phys(t, d) if not active(t) then t.physical = 0 else t.physical = add(t.physical, d) end end
local function mental(t, d)
    if not active(t) then t.stun = 0; return end
    local c = t.stun + d
    if c > 0 then t.stun = math.min(c, 10); return end
    phys(t, c); t.stun = 0
end
local function revive(t)
    t.runner_facts.lifecycle.active = true
    t.runner_facts.lifecycle.downed = false
    t.runner_facts.lifecycle.dead = false
end
function M.difficulty(t)
    local d = t.physical >= 7 and 4 or (t.physical >= 4 and 6 or 8)
    local red = t.attributes.body // 3                         -- Body / 3
    d = d > red and d - red or 0
    return math.max(d, 2)
end
function M.targets(party)
    local out = {}
    for i = 0, 2 do
        local runner = party[i]
        if runner and runner.runner_facts.lifecycle.active then
            out[#out + 1] = i
        end
    end
    return out
end
function M.cursor(row, n_active, d)            -- rows = targets + Cancel
    local rows = n_active + 1
    if row >= rows then row = 0 end
    if d < 0 then return row == 0 and rows - 1 or row - 1 end
    if d > 0 then return (row + 1) % rows end
    return row
end
local function consume(user, item)
    if item.uses_remaining > 1 then
        item.uses_remaining = item.uses_remaining - 1
    else
        Inventory.remove_item(user, item)
    end
end
-- returns message (E.MenuMessage) or nil when the item cannot be used
function M.use(user, selected, t, roll)
    local item = selected and Inventory.at_cell(user, selected.cell)
    if item == nil or item.id ~= selected.id or
       (item.uses_remaining or 0) <= 0 then
        return nil
    end
    local kind = Items[item.id] and Items[item.id].kind
    if kind == "medical_kit" then
        if t.physical == 10 and t.stun == 10 then return E.MenuMessage.NOT_INJURED end
        local s = roll(user.skills[E.RunnerSkill.BIOTECH], M.difficulty(t))
        local d = s == 0 and 1 or s
        phys(t, d); mental(t, d); revive(t)
        t.first_aid_continuation = M.REVIVED_CONTINUATION
        consume(user, item); return E.MenuMessage.FIRST_AID_USED
    end
    if kind == "stim_patch" then
        if t.stun == 10 then return E.MenuMessage.NOT_INJURED end
        mental(t, 2); if t.physical ~= 0 then revive(t) end
    elseif kind == "trauma_patch" then
        if t.physical == 10 then return E.MenuMessage.NOT_INJURED end
        phys(t, 2); if t.stun ~= 0 then revive(t) end
    else
        return nil
    end
    consume(user, item); return E.MenuMessage.SLAP_PATCH_USED
end
return M
