-- rules/inventory.lua: item selection, equip, discard, trade rules (srx inventory_action_conversion.cpp, ROM cited).
-- Runner tables are 0-based arrays: inventory[0..7], phase[0..7]; weapon_slot -1 (= +0x56 0x7F/neg) when none.
local E = require("enums")
local B, I = E.ItemBranch, E.ItemId
local M = { AUTO_USE_RANGE_FIRST = 0x18, AUTO_USE_RANGE_END = 0x2C }   -- 0x121C0 bounds [C]

M.SLOTS, M.ROWS = 8, 4
function M.slot_of(column, row) return row * 2 + column end          -- 0x12148 [C srx]
function M.selected(r, column, row)
    if column > 1 or row > 3 then return nil end
    local slot = M.slot_of(column, row)
    local code = r.inventory[slot]
    if code == 0 then return nil end
    return { slot = slot, code = code, phase = r.phase[slot] }
end

-- 0x121C0 classification [srx C]
function M.classify(code, pending_medical)
    -- 0x10..0x12 medical; 0x13 fetish, 0x14 passkey, 0x16 goggles; 0x18..0x2B
    -- (codes without E.ItemId names) classify as automatic use [C].
    if pending_medical and code >= I.MEDKIT and code <= I.TRAUMA_PATCH then return B.FIRST_AID end
    if code == I.FETISH or code == I.MAGLOCK_PASSKEY or code == I.SMART_GOGGLES or
        (code >= M.AUTO_USE_RANGE_FIRST and code < M.AUTO_USE_RANGE_END) then return B.AUTOMATIC_USE end
    if code < M.AUTO_USE_RANGE_FIRST then return B.WEAPON_OR_GEAR end
    return B.ARMOR
end

local function percent(rounds, cap)          -- 0x125FA: ceil(rounds*100/cap); cap 0 keeps rounds*100 & 0xFFFF
    local n = rounds * 100
    if cap == 0 then return n & 0xFFFF end
    local p = n // cap
    if n % cap ~= 0 then p = p + 1 end
    return p & 0xFFFF
end
local function scale(pct, cap)
    local s = pct * cap
    local r = s // 100
    if s % 100 >= 50 then r = r + 1 end
    return math.min(r, 0xFF)
end
M.percent, M.scale = percent, scale

-- toggle_selected_equipment (0x1219C armor / 0x125FA, 0x12222 weapons) [srx C]; returns mutation name
function M.toggle_equipment(r, sel, old_cap, new_cap)
    if r.inventory[sel.slot] ~= sel.code or sel.code == 0 then return "invalid" end
    local br = M.classify(sel.code)
    if br == B.ARMOR then
        if r.armor ~= 0 then r.armor = 0; r.phase[sel.slot] = 0; return "armor_unequipped" end
        r.armor = sel.code - 0x2B; r.phase[sel.slot] = r.armor; return "armor_equipped"
    end
    if br ~= B.WEAPON_OR_GEAR then return "rejected" end
    local old = r.rounds
    local valid = r.weapon_slot >= 0 and r.weapon_slot < M.SLOTS
    local cur_weapon = valid and r.weapon_code ~= 0 and r.weapon_code < 0x0D
    local carried = cur_weapon and (percent(old, old_cap) & 0xFF) or old
    if valid and r.weapon_slot == sel.slot then
        r.rounds = carried; r.weapon_slot = -1; r.weapon_code = 0; return "unequipped"
    end
    if sel.code < 0x0D then
        r.rounds = cur_weapon and scale(percent(old, old_cap), new_cap) or (new_cap == 0 and 0 or scale(carried, new_cap))
    else
        r.rounds = carried
    end
    r.weapon_slot = sel.slot; r.weapon_code = sel.code
    return "equipped"
end

function M.can_discard(sel) return not (sel.slot == 7 and sel.code >= 0x29 and sel.code <= 0x2B) end   -- [srx C]
function M.can_trade(code) return code < 0x17 or code >= 0x2C end                                        -- [srx C]

local function remove(r, sel, discard)       -- discard: 0x1275C; transfer: 0x12A30..0x12A4E
    if r.weapon_slot >= 0 and r.weapon_slot == sel.slot then
        r.weapon_slot = -1
        r.weapon_code = 0
        -- Discard clears +0x4E; transfer clears +0x56/+0x57 but retains rounds.
        if discard then r.rounds = 0 end
    end
    if sel.code >= 0x2C and r.armor == sel.code - 0x2B and r.phase[sel.slot] ~= 0 then
        r.armor = 0
    end
    r.inventory[sel.slot] = 0; r.phase[sel.slot] = 0
end
function M.discard(r, sel)
    if r.inventory[sel.slot] ~= sel.code or sel.code == 0 or not M.can_discard(sel) then return false end
    remove(r, sel, true); return true
end

function M.free_slot(r) for i = 0, M.SLOTS - 1 do if r.inventory[i] == 0 then return i end end end
function M.next_recipient(active, source, after)   -- cyclic scan of the 3 party slots [srx C]
    for step = 1, 3 do
        local c = (after + step) % 3
        if c ~= source and active[c] then return c end
    end
end
function M.transfer(src, sel, dst, dst_slot)
    if src.inventory[sel.slot] ~= sel.code or sel.code == 0 or dst.inventory[dst_slot] ~= 0 or not M.can_trade(sel.code) then
        return false
    end
    dst.inventory[dst_slot] = sel.code
    dst.phase[dst_slot] = sel.code >= 0x2C and 0 or sel.phase
    remove(src, sel, false)
    return true
end
return M
