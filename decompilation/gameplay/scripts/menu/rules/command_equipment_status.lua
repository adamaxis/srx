-- Status-panel data selected by ROM 0x111E8/0x12EAE/0x13000.
local E = require("enums")
local Weapons = require("data.weapons").by_id
local Items = require("data.items").by_id
local Labels = require("data.command_equipment_status")

local M = {}

local DAMAGE_BAR_BY_EFFECT = {
    [E.WeaponDamageEffect.LIGHT] = E.WeaponDamageBar.LIGHT,
    [E.WeaponDamageEffect.MEDIUM] = E.WeaponDamageBar.MEDIUM,
    [E.WeaponDamageEffect.HEAVY] = E.WeaponDamageBar.HEAVY,
}

local function power_label_index(rating)
    return math.max(E.CommandPowerLabel.MEAGER,
        math.min(E.CommandPowerLabel.INCREDIBLE, rating - 1))
end

local function unarmed_power_bonus(cyberware_flags)
    for _, adjustment in ipairs(E.CommandPowerCyberwareBonus) do
        if cyberware_flags & adjustment.mask ~= 0 then
            return adjustment.value
        end
    end
    return 0
end

function M.command_view(runner)
    if runner == nil then return nil end

    local action_id = runner.equipped_weapon_id
    local armor_id = runner.equipped_armor_id
    local view = {
        action_item_id = action_id or "",
        armor_item_id = armor_id or "",
        action_name = "",
        damage_effect_label = "",
        no_armor = armor_id == nil,
        armor_label = armor_id == nil and
            Labels[E.CommandStatusArmor.NONE] or
            ((Items[armor_id] and Items[armor_id].name) or ""),
    }

    -- The negative +0x56 spell route has its own D4EA0 detail row and Power
    -- selection, rendered by the shared special-action presenter.
    if runner.selected_spell_id ~= nil then
        view.special_action = true
        return view
    end

    local weapon = action_id and Weapons[action_id]
    if weapon ~= nil then
        view.damage_bar_index = DAMAGE_BAR_BY_EFFECT[weapon.damage_effect]
        view.power_label_index = power_label_index(weapon.power_rating)
        return view
    end

    -- A semantic item ID can exist for armor, utility gear, or a focus. Its
    -- ordinary item name remains available, but those records have no weapon
    -- Damage/Power row in D4540.
    if action_id ~= nil then return view end

    if runner.equipped_action_code ~= E.ItemId.NONE then
        return view
    end

    -- Zero-action branch: D1A7E names the action. 0x12EAE selects the Stun
    -- strip unless +0x90 bits 2/3 override it; bit 3 is tested first.
    view.action_name = Labels[E.CommandStatusAction.EMPTY_HAND]
    local cyberware = runner.cyberware_flags or 0
    if cyberware & E.CyberwareMask.SPURS ~= 0 then
        view.damage_bar_index = E.CommandStatusDamageBar.STUN
    elseif cyberware & E.CyberwareMask.HAND_RAZORS ~= 0 then
        view.damage_bar_index = E.CommandStatusDamageBar.HAND_RAZORS
    else
        view.damage_bar_index = E.CommandStatusDamageBar.STUN
        view.damage_effect_label = Labels[E.CommandStatusDamage.STUN]
    end

    -- 0x1305A reads +0x79 and adds the first +0x90 bit 5..8 bonus; 0x13094
    -- converts that one-based strength to the D1F5E label row.
    local strength = runner.attributes and runner.attributes.strength or 0
    view.power_label_index = power_label_index(
        strength + unarmed_power_bonus(cyberware))
    return view
end

return M
