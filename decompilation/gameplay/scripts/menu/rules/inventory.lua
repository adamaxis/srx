-- Runner equipment rules over named item identities and item-specific facts.
-- The host translates source item records into this positioned equipment list.
local E = require("enums")
local Items = require("items").by_id
local B = E.ItemBranch
local M = { COLUMNS = 2, ROWS = 4 }

local FIRST_AID_KINDS = {
    medical_kit = true, stim_patch = true, trauma_patch = true,
}
local AUTOMATIC_USE_KINDS = {
    fetish = true, passkey = true, smart_goggles = true,
    power_focus = true, spell_focus = true, talisman = true,
    spell_lock = true, totem = true,
}
local TRADEABLE_KINDS = {
    firearm = true, grenade = true, medical_kit = true,
    stim_patch = true, trauma_patch = true, fetish = true,
    passkey = true, electronic_kit = true, smart_goggles = true,
    armor = true,
}

function M.cell(column, row)
    if column < 0 or column >= M.COLUMNS or row < 0 or row >= M.ROWS then
        return nil
    end
    return { column = column, row = row }
end

function M.same_cell(left, right)
    return left ~= nil and right ~= nil and
           left.column == right.column and left.row == right.row
end

local function cell_order(cell)
    return cell.row * M.COLUMNS + cell.column
end

function M.at_cell(runner, cell)
    if type(runner.equipment) ~= "table" then return nil end
    for _, item in ipairs(runner.equipment) do
        if M.same_cell(item.cell, cell) then return item end
    end
end

local function equipment_index(runner, cell)
    if type(runner.equipment) ~= "table" then return nil end
    for index, item in ipairs(runner.equipment) do
        if M.same_cell(item.cell, cell) then return index end
    end
end

function M.set_cell(runner, cell, item)
    if cell == nil then return false end
    runner.equipment = runner.equipment or {}
    local index = equipment_index(runner, cell)
    if item == nil then
        if index ~= nil then table.remove(runner.equipment, index) end
        return true
    end

    item.cell = { column = cell.column, row = cell.row }
    if index ~= nil then
        runner.equipment[index] = item
        return true
    end
    local insert_at = #runner.equipment + 1
    for current, existing in ipairs(runner.equipment) do
        if cell_order(existing.cell) > cell_order(cell) then
            insert_at = current
            break
        end
    end
    table.insert(runner.equipment, insert_at, item)
    return true
end

function M.selected(runner, column, row)
    return M.at_cell(runner, M.cell(column, row))
end

local function definition(item)
    return item and Items[item.id] or nil
end

function M.classify(item, pending_medical)
    local data = definition(item)
    if data == nil then return nil end
    if pending_medical and FIRST_AID_KINDS[data.kind] then
        return B.FIRST_AID
    end
    if AUTOMATIC_USE_KINDS[data.kind] then return B.AUTOMATIC_USE end
    if data.kind == "armor" then return B.ARMOR end
    return B.WEAPON_OR_GEAR
end

local function is_firearm(item_id)
    local data = Items[item_id]
    return data ~= nil and data.kind == "firearm"
end

local function percent(rounds, capacity)
    local amount = rounds * 100
    if capacity <= 0 then return 0 end
    local value = amount // capacity
    if amount % capacity ~= 0 then value = value + 1 end
    return value
end

local function scale(percent_value, capacity)
    if capacity <= 0 then return 0 end
    local amount = percent_value * capacity
    local value = amount // 100
    if amount % 100 >= 50 then value = value + 1 end
    return value
end
M.percent, M.scale = percent, scale

function M.toggle_equipment(runner, selected, old_capacity, new_capacity)
    local item = selected and M.at_cell(runner, selected.cell)
    if item == nil or item.id ~= selected.id then return "invalid" end
    local branch = M.classify(item)
    if branch == B.ARMOR then
        -- ROM 0x121A4/0x121A8 clears the global armor and only the selected
        -- cell marker; 0x121B4/0x121B8 equips and marks that selected cell [C].
        if runner.equipped_armor_id ~= nil then
            runner.equipped_armor_id = nil
            item.armor_marker_id = nil
            return "armor_unequipped"
        end
        runner.equipped_armor_id = item.id
        item.armor_marker_id = item.id
        return "armor_equipped"
    end
    if branch ~= B.WEAPON_OR_GEAR then return "rejected" end

    local old_rounds = runner.rounds
    local current_firearm = runner.equipped_weapon_cell ~= nil and
                            is_firearm(runner.equipped_weapon_id)
    local carried_rounds = current_firearm and
        percent(old_rounds, old_capacity) or old_rounds
    if M.same_cell(runner.equipped_weapon_cell, item.cell) then
        runner.rounds = carried_rounds
        runner.equipped_weapon_id = nil
        runner.equipped_weapon_cell = nil
        runner.equipped_weapon_state = E.WeaponSelectionState.EMPTY
        runner.selected_spell_id = nil
        return "unequipped"
    end

    if is_firearm(item.id) then
        if current_firearm then
            runner.rounds = scale(percent(old_rounds, old_capacity),
                                  new_capacity)
        elseif new_capacity == 0 then
            runner.rounds = 0
        else
            runner.rounds = scale(carried_rounds, new_capacity)
        end
    else
        runner.rounds = carried_rounds
    end
    runner.equipped_weapon_id = item.id
    runner.equipped_weapon_cell = {
        column = item.cell.column, row = item.cell.row,
    }
    runner.equipped_weapon_state = nil
    runner.selected_spell_id = nil
    return "equipped"
end

function M.can_discard(item)
    local data = definition(item)
    local totem_cell = M.cell(M.COLUMNS - 1, M.ROWS - 1)
    return not (item and data and data.kind == "totem" and
                M.same_cell(item.cell, totem_cell))
end

function M.can_trade(item)
    local data = definition(item)
    return data ~= nil and TRADEABLE_KINDS[data.kind] == true
end

local function remove(runner, item, discard)
    if M.same_cell(runner.equipped_weapon_cell, item.cell) and
       runner.equipped_weapon_id == item.id then
        runner.equipped_weapon_cell = nil
        runner.equipped_weapon_id = nil
        runner.equipped_weapon_state = E.WeaponSelectionState.EMPTY
        if discard then runner.rounds = 0 end
    end
    if runner.equipped_armor_id == item.id then
        runner.equipped_armor_id = nil
    end
    M.set_cell(runner, item.cell, nil)
end

function M.remove_item(runner, selected)
    local item = selected and M.at_cell(runner, selected.cell)
    if item == nil or item.id ~= selected.id then return false end
    remove(runner, item, false)
    return true
end

function M.discard(runner, selected)
    local item = selected and M.at_cell(runner, selected.cell)
    if item == nil or item.id ~= selected.id or not M.can_discard(item) then
        return false
    end
    remove(runner, item, true)
    return true
end

function M.free_cell(runner)
    for row = 0, M.ROWS - 1 do
        for column = 0, M.COLUMNS - 1 do
            local cell = M.cell(column, row)
            if M.at_cell(runner, cell) == nil then return cell end
        end
    end
end

function M.next_recipient(active, source, after)
    for step = 1, 3 do
        local candidate = (after + step) % 3
        if candidate ~= source and active[candidate] then return candidate end
    end
end

function M.transfer(source, selected, destination, destination_cell)
    local item = selected and M.at_cell(source, selected.cell)
    if item == nil or item.id ~= selected.id or
       M.at_cell(destination, destination_cell) ~= nil or
       not M.can_trade(item) then
        return false
    end

    local transferred = {}
    for key, value in pairs(item) do transferred[key] = value end
    M.set_cell(destination, destination_cell, transferred)
    remove(source, item, false)
    return true
end

return M
