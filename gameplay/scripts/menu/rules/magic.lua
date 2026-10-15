-- ROM 0xFBA6/0x1047E/0x104C8/0x1055E/0x105E6 Magic menu rules.
-- Inputs are typed runner fields and the source-decoded spell table supplied
-- by the host. This module contains no ROM-address or RAM access.
local M = {}

M.ITEM = {
    FETISH = 0x13,
    POWER_FOCUS = 0x18,
    SPELL_FOCUS_BASE = 0x19,
    COMBAT_SENSE = 0x28,
    RAT = 0x29,
    GATOR = 0x2A,
    BEAR = 0x2B,
}

-- 0xC8AE0's selector grouped by spell action. -1 means that action has no
-- slot-7 totem modifier; 0/1/2 selects the corresponding totem word.
M.CATEGORY = {
    [0] = 1, [1] = 1, [2] = 1, [3] = 0, [4] = 1, [5] = 0, [6] = -1,
    [7] = 1, [8] = 1, [9] = 1, [10] = -1, [11] = 2, [12] = 0,
}
M.TOTEM_MOD = {
    [M.ITEM.RAT] = { [0] = 2, [1] = -1, [2] = 0 },
    [M.ITEM.GATOR] = { [0] = -1, [1] = 2, [2] = 0 },
    [M.ITEM.BEAR] = { [0] = 0, [1] = 0, [2] = 2 },
}

-- Spell drain metadata remains useful to the combat-cast path. The menu's
-- displayed Damage strip uses the typed spell effect fields below instead.
M.DRAIN_BOXES = { [0] = 1, [1] = 3, [2] = 6, [3] = 10 }
M.SPELL = {
    [0] = { 0, 1 }, [1] = { 1, 1 }, [2] = { 3, 3 }, [3] = { 1, 0 },
    [4] = { 2, 0 }, [5] = { 2, 0 }, [6] = { 2, 2 }, [7] = { 0, 0 },
    [8] = { 1, 0 }, [9] = { 2, 1 }, [10] = { 1, 2 }, [11] = { 1, 0 },
    [12] = { 2, 0 },
}

local function find(r, code, first, last)
    for slot = first, last, -1 do
        if r.inventory[slot] == code then return slot end
    end
end

-- 0x55F02: +0x80 plus the first Power Focus phase in slots 6..0.
function M.item18_skill(r)
    local value = r.skills[0] or 0
    local slot = find(r, M.ITEM.POWER_FOCUS, 6, 0)
    if slot then value = (value + r.phase[slot]) & 0xFF end
    return value
end

-- 0x55ED6: (Quickness + Intelligence + Willpower) >> 1, plus first Combat
-- Sense phase. Addition follows source byte wrapping.
function M.combat_pool(r)
    local value = ((r.attributes[1] + r.attributes[4] + r.attributes[5]) & 0xFF) >> 1
    for slot = 0, 7 do
        if r.inventory[slot] == M.ITEM.COMBAT_SENSE then
            return (value + r.phase[slot]) & 0xFF
        end
    end
    return value
end

function M.split(pool, posture)
    local reduction = (pool * posture * 25) // 100
    return reduction, pool - reduction
end

local function posture_remainder(value, posture)
    local reduction = (value * posture * 25) // 100
    return value - reduction
end

-- 0x104C8/0x104F6: matching spell focus in slots 6..0, then slot-7 totem.
local function focus_and_totem(r, spell_index, value)
    local focus = find(r, M.ITEM.SPELL_FOCUS_BASE + spell_index, 6, 0)
    if focus then value = value + r.phase[focus] end
    local totem = M.TOTEM_MOD[r.inventory[7]]
    local category = M.CATEGORY[spell_index]
    if totem and category ~= nil and category >= 0 then
        value = value + totem[category]
        if value <= 0 then value = 1 end
    end
    return value
end

local function metric_action_index(table_, row, column)
    -- 0x104C8 selects the count-and-pair list using row*6 + column. This
    -- source stride is distinct from the seven-column cursor layout.
    local entry = table_.entries[(row * 6 + column) + 1]
    return entry and entry.code or nil
end

-- 0x1055E success strip. The live action index selects its matching focus
-- item and slot-7 modifier; if the ROM table lookup has no entry, the base
-- value remains visible without fabricating a modifier.
function M.success_value(r, table_, row, column, force)
    local value = math.min(posture_remainder(M.item18_skill(r), r.posture),
                           r.magic)
    value = value + force
    local action_index = metric_action_index(table_, row, column)
    if action_index ~= nil then
        value = focus_and_totem(r, action_index, value)
    end
    return value
end

local function effect_roll_count(r)
    return posture_remainder(M.item18_skill(r), r.posture) + r.attributes[5]
end

-- 0x10660: ceil(force/2) + the D4EA0 difficulty byte (minimum TN 2),
-- source percentage rounding, then subtract adjusted successes from the
-- typed D4EA0 base effect. Positive values choose a Damage strip row.
function M.effect_value(r, entry, force)
    local tn = ((force + 1) // 2) + entry.difficulty_modifier
    if tn < 2 then tn = 2 end
    local percent = tn >= 7 and 10 or ((7 - tn) * 20)
    local scaled = effect_roll_count(r) * percent
    local adjusted = scaled // 100
    if scaled % 100 >= 50 then adjusted = adjusted + 1 end
    adjusted = adjusted // 2
    return entry.base_effect - adjusted
end

function M.bar_row(value)
    return value == 0 and 0 or (value - 1)
end

-- 0x1047E: active caster pool, rounded to nearest quarter, then Body and
-- Willpower. Values wrap at the source byte boundary before asset indexing.
function M.defense(party, slot_active, r)
    local pool = 0
    for slot = 0, 2 do
        local member = party[slot]
        if member and slot_active[slot] and member.archetype <= 1 then
            local magic = M.item18_skill(member)
            pool = pool + (magic // 4) + ((magic % 4 >= 3) and 1 or 0)
        end
    end
    local physical = (r.attributes[0] + pool) & 0xFF
    local mana = (r.attributes[5] + pool) & 0xFF
    return {
        pool = pool,
        physical = physical,
        physical_row = M.bar_row(physical),
        mana = mana,
        mana_row = M.bar_row(mana),
    }
end

function M.presentation(r, party, slot_active, table_, entry,
                        row, column, force)
    local defense = M.defense(party, slot_active, r)
    local effect = M.effect_value(r, entry, force)
    local success = M.success_value(r, table_, row, column, force)
    return {
        row = row,
        column = column,
        table_entry_index = row * 7 + column,
        spell_code = entry.code,
        force = force,
        maximum_force = entry.force_limit,
        success_value = success,
        success_row = M.bar_row(success),
        effect_value = effect,
        effect_row = effect > 0 and (effect - 1) or -1,
        availability = entry.availability,
        physical_defense_value = defense.physical,
        physical_defense_row = defense.physical_row,
        mana_defense_value = defense.mana,
        mana_defense_row = defense.mana_row,
        posture = r.posture,
    }
end

function M.can_cast(r) return r.stun ~= 1 end
return M
