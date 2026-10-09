-- Shared item labels and detail text are generated from the ROM into
-- dialogue/script-text.lua. This module maps source IDs to their stable keys.
local Items = {}

local function label_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

local function source_item(code)
    return {
        code = code,
        name_key = "items.name." .. code,
        category_key = "items.category." .. code,
    }
end

local catalog = {}
for code = 1, 50 do
    catalog[#catalog + 1] = source_item(code)
end

local detail_text_keys = {}
for index = 0, 31 do
    detail_text_keys[#detail_text_keys + 1] = "items.action_detail." .. index
end

Items.catalog = catalog
Items.detail_text_keys = detail_text_keys
Items.detail_label_keys = {
    "items.detail_label.ammo",
    "items.detail_label.uses",
    "items.detail_label.rating",
}

-- ROM 0x11740/0x118C6 action row order. The native controller keeps the
-- four source icon positions and cursor timing; dispatch resolves these IDs.
function Items.inventory_actions()
    return { "equip_or_use", "discard", "trade", "cancel" }
end

-- Stable IDs for the ROM-backed inventory-action and First Aid copy.
function Items.ui_text_entries()
    local ids = {
        "equipped_prefix",
        "equipped_suffix",
        "discard_prompt",
        "cannot_discard",
        "cannot_trade",
        "trade_recipient",
        "trade_full_inventory",
        "automatic_use",
        "first_aid_prompt",
        "first_aid_label",
        "first_aid_cancel",
        "first_aid_not_injured",
        "first_aid_slap_patch_used",
        "first_aid_medkit_used",
    }
    return ids, nil, label_keys(ids, "items.text.option.")
end

-- ROM-ordered item detail labels for weapon power and armor names.
function Items.display_text_entries()
    local ids = {
        "power_meager",
        "power_minor",
        "power_very_low",
        "power_low",
        "power_very_light",
        "power_light",
        "power_medium",
        "power_medium_high",
        "power_high",
        "power_brutal",
        "power_severe",
        "power_massive",
        "power_incredible",
        "armor_vest_top",
        "armor_vest_bottom",
        "armor_clothing_top",
        "armor_clothing_bottom",
        "armor_jacket_top",
        "armor_jacket_bottom",
        "armor_plates_top",
        "armor_plates_bottom",
        "lined_duster_top",
        "lined_duster_bottom",
        "light_combat_armor_top",
        "light_combat_armor_bottom",
        "heavy_combat_armor_top",
        "heavy_combat_armor_bottom",
    }
    return ids, nil, label_keys(ids, "items.display.option.")
end

return Items
