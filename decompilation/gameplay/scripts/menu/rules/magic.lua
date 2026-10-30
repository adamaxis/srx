-- ROM 0xFBA6/0x1047E/0x104C8/0x1055E/0x105E6 Magic menu rules.
-- Inputs are typed runner fields and the source-decoded spell table supplied
-- by the host. This module contains no ROM-address or RAM access.
local M = {}
local E = require("enums")
local Inventory = require("menu.rules.inventory")
local Items = require("data.items").by_id

M.ITEM = {
    FETISH = "fetish",
    POWER_FOCUS = "power_focus",
    SPELL_FOCUS = {
        flame_dart = "flame_dart_focus",
        flame_bolt = "flame_bolt_focus",
        hellblast = "hellblast_focus",
        invisibility = "invisibility_focus",
        sleep = "sleep_focus",
        stink = "stink_focus",
        super_barrier = "supr_barrier_focus",
        mana_zap = "mana_zap_focus",
        mana_blast = "mana_blast_focus",
        mana_storm = "mana_storm_focus",
        rockskin = "rockskin_focus",
        heal_wounds = "heal_wounds_focus",
        confusion = "confusion_focus",
        barrier = "barrier_focus",
    },
    COMBAT_SENSE = "combat_sense_spell_lock",
    RAT = "rat_totem",
    GATOR = "gator_totem",
    BEAR = "bear_totem",
}

local function find_last(runner, item_id, include)
    for index = #(runner.equipment or {}), 1, -1 do
        local item = runner.equipment[index]
        if item.id == item_id and (include == nil or include(item)) then
            return item
        end
    end
end

-- 0x55F02: Sorcery plus the first Power Focus bonus in positions 7..1.
function M.item18_skill(r)
    local value = r.skills.sorcery or 0
    local totem_cell = Inventory.cell(1, Inventory.ROWS - 1)
    local focus = find_last(r, M.ITEM.POWER_FOCUS, function(item)
        return not Inventory.same_cell(item.cell, totem_cell)
    end)
    if focus then value = value + focus.sorcery_bonus end
    return value
end

-- 0x55ED6: average Quickness, Intelligence, and Willpower, then add the
-- first Combat Sense bonus. Lua keeps this as a normal numeric game value.
function M.combat_pool(r)
    local value = (r.attributes.quickness + r.attributes.intelligence +
                   r.attributes.willpower) // 2
    for _, item in ipairs(r.equipment or {}) do
        if item and item.id == M.ITEM.COMBAT_SENSE then
            return value + item.command_bonus
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

-- 0x104C8/0x104F6: matching spell focus, then slot-7 totem.
local function focus_and_totem(r, spell_id, value)
    local focus_id = M.ITEM.SPELL_FOCUS[spell_id]
    local totem_cell = Inventory.cell(1, Inventory.ROWS - 1)
    local focus = focus_id and find_last(r, focus_id, function(item)
        return not Inventory.same_cell(item.cell, totem_cell)
    end) or nil
    if focus then value = value + focus.spell_action_bonus end
    local totem_item = Inventory.at_cell(r, totem_cell)
    local totem_definition = totem_item and Items[totem_item.id]
    local modifier = totem_definition and
                     totem_definition.totem_spell_modifiers and
                     totem_definition.totem_spell_modifiers[spell_id]
    if modifier ~= nil then
        value = value + modifier
        if value <= 0 then value = 1 end
    end
    return value
end

local function metric_action_id(table_, row, column)
    -- 0x104C8 selects the count-and-pair list using row*6 + column. This
    -- source stride is distinct from the seven-column cursor layout.
    local entry = table_[(row * 6 + column) + 1]
    return entry and entry.id or nil
end

-- 0x1055E success strip. The live action index selects its matching focus
-- item and slot-7 modifier; if the ROM table lookup has no entry, the base
-- value remains visible without fabricating a modifier.
function M.success_value(r, table_, row, column, force)
    local value = math.min(posture_remainder(M.item18_skill(r), r.posture),
                           r.magic)
    value = value + force
    local action_id = metric_action_id(table_, row, column)
    if action_id ~= nil then
        value = focus_and_totem(r, action_id, value)
    end
    return value
end

local function effect_roll_count(r)
    return posture_remainder(M.item18_skill(r), r.posture) +
           r.attributes.willpower
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
        if member and slot_active[slot] and
           E.RunnerClass.CASTERS[member.character_class] then
            local magic = M.item18_skill(member)
            pool = pool + (magic // 4) + ((magic % 4 >= 3) and 1 or 0)
        end
    end
    local physical = r.attributes.body + pool
    local mana = r.attributes.willpower + pool
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
        spell_id = entry.id,
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
