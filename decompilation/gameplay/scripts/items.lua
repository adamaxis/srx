-- Shared item labels and detail text are generated from the ROM into
-- dialogue/script-text.lua. This module maps source IDs to their stable keys.
local Items = {}

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

return Items
