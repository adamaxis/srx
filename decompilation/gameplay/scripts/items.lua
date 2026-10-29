-- Semantic catalog for all 50 items. Stable string IDs are the gameplay keys;
-- the host translates those IDs to source inventory values at the boundary.
local Items = {}
local ItemDefinitions = require("data.items")
local SourceWeaponRecords = require("data.weapons")
local E = require("enums")
local ItemKind = E.ItemKind

local item_kind_by_value = {}
for name, value in pairs(ItemKind) do
    item_kind_by_value[value] = name:lower()
end

local image_family_by_kind = {
    firearm = "weapons", grenade = "weapons",
    medical_kit = "first.aid", stim_patch = "first.aid",
    trauma_patch = "first.aid", fetish = "miscellaneous",
    passkey = "miscellaneous", electronic_kit = "miscellaneous",
    smart_goggles = "miscellaneous", enchanted_dagger = "magic",
    power_focus = "magic", spell_focus = "magic", talisman = "magic",
    spell_lock = "magic", totem = "magic", armor = "armor",
}

local armor_item_order = {
    "armor_vest", "armor_clothing", "armor_jacket", "vest_with_plates",
    "lined_duster", "light_combat_armor", "heavy_combat_armor",
}
local armor_index_by_item_id = {}
for index, id in ipairs(armor_item_order) do
    armor_index_by_item_id[id] = index
end

local function item_image_suffix(asset_index)
    return asset_index < 10 and ("0" .. asset_index) or tostring(asset_index)
end

local function label_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

local function define_item(id, asset_index)
    local definition = ItemDefinitions.by_id[id]
    assert(definition ~= nil and definition.id == id,
           "item has no matching semantic definition: " .. id)
    local kind = item_kind_by_value[definition.kind]
    assert(kind ~= nil, "item definition has an unknown kind: " .. id)
    local image_family = image_family_by_kind[kind]
    assert(image_family ~= nil, "item kind has no image family: " .. kind)

    local image_suffix = item_image_suffix(asset_index)
    local item = {
        id = id,
        kind = kind,
        name_key = "items.name." .. asset_index,
        category_key = "items.category." .. asset_index,
        target_motion_range = definition.target_motion_range,
        has_detail_art = definition.has_detail_art,
        presentation = {
            icon_resource_key = "items." .. image_family ..
                ".icons.item." .. image_suffix,
            detail_resource_key = "items." .. image_family ..
                ".detail.item." .. image_suffix,
        },
    }
    if definition.purchase_base ~= nil then
        item.pricing = {
            purchase_base = definition.purchase_base,
            resale_base = math.floor(definition.purchase_base / 2),
        }
    end
    if definition.totem_spell_modifiers ~= nil then
        item.spell_action_modifiers = definition.totem_spell_modifiers
    end
    if kind == "firearm" then
        item.weapon = {}
    end
    if kind == "armor" then
        item.armor = { armor_index = armor_index_by_item_id[id] }
    end

    local template = SourceWeaponRecords.get(id)
    if template ~= nil then
        assert(template.id == id and
               template.purchase_base == definition.purchase_base,
               "weapon profile disagrees with its item definition: " .. id)
        if item.weapon ~= nil then
            assert(template.fire_mode == "single_round" or
                       template.fire_mode == "three_round_burst",
                   "firearm has no source firing mode: " .. id)
            assert(template.automatic ==
                       (template.fire_mode == "three_round_burst") and
                       template.rounds_per_shot ==
                           (template.fire_mode == "three_round_burst" and 3 or 1),
                   "weapon firing mode and round cost disagree: " .. id)
            local power_profile = ({
                [4] = "low", [6] = "light", [7] = "medium",
                [9] = "high", [10] = "brutal",
            })[template.power_rating]
            assert(power_profile ~= nil,
                   "weapon power has no display profile: " .. id)
            item.weapon.source_action = template.source_action
            item.weapon.magazine_capacity = template.magazine_capacity
            item.weapon.damage_effect = template.damage_effect
            item.weapon.power_profile = power_profile
            item.weapon.fire_mode = template.fire_mode
            item.weapon.trigger_behavior = template.trigger_behavior
            item.weapon.rounds_per_action = template.rounds_per_shot
            item.weapon.attack_cadence = template.attack_cadence
            item.weapon.attack_delay_scale = template.attack_delay_scale
            item.weapon.reload_delay_scale = template.reload_delay_scale
            assert(type(item.weapon.source_action) == "table" and
                       item.weapon.fire_mode ~= nil and
                       item.weapon.trigger_behavior ~= nil and
                       item.weapon.attack_cadence ~= nil and
                       item.weapon.attack_delay_scale ~= nil and
                       item.weapon.reload_delay_scale ~= nil,
                   "firearm template is incomplete: " .. id)
        end
    elseif kind == "firearm" then
        error("firearm has no source weapon template: " .. id)
    end

    if kind == "armor" then
        assert(type(definition.armor) == "table",
               "armor item has no modifier definition: " .. id)
        item.armor.primary_modifier = definition.armor.primary_modifier
        item.armor.secondary_modifier = definition.armor.secondary_modifier
    end
    return item
end

Items.catalog = {}
Items.by_id = {}
for asset_index, id in ipairs(ItemDefinitions.catalog_order) do
    local item = define_item(id, asset_index)
    Items.catalog[asset_index] = item
    Items.by_id[item.id] = item
end
assert(#Items.catalog == 50, "the item catalog must contain 50 entries")

local detail_text_keys = {}
for index = 0, 31 do
    detail_text_keys[#detail_text_keys + 1] = "items.action_detail." .. index
end

Items.detail_text_keys = detail_text_keys
Items.detail_label_keys = {
    "items.detail_label.ammo",
    "items.detail_label.uses",
    "items.detail_label.rating",
}

-- ROM-ordered action rows; native controllers forward these stable IDs.
function Items.inventory_actions()
    return { "equip_or_use", "discard", "trade", "cancel" }
end

function Items.ui_text_entries()
    local ids = {
        "equipped_prefix", "equipped_suffix", "discard_prompt",
        "cannot_discard", "cannot_trade", "trade_recipient",
        "trade_full_inventory", "automatic_use", "first_aid_prompt",
        "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
        "first_aid_slap_patch_used", "first_aid_medkit_used",
    }
    return ids, nil, label_keys(ids, "items.text.option.")
end

function Items.display_text_entries()
    local ids = {
        "power_meager", "power_minor", "power_very_low", "power_low",
        "power_very_light", "power_light", "power_medium",
        "power_medium_high", "power_high", "power_brutal", "power_severe",
        "power_massive", "power_incredible", "armor_vest_top",
        "armor_vest_bottom", "armor_clothing_top", "armor_clothing_bottom",
        "armor_jacket_top", "armor_jacket_bottom", "armor_plates_top",
        "armor_plates_bottom", "lined_duster_top", "lined_duster_bottom",
        "light_combat_armor_top", "light_combat_armor_bottom",
        "heavy_combat_armor_top", "heavy_combat_armor_bottom",
    }
    return ids, nil, label_keys(ids, "items.display.option.")
end

return Items
