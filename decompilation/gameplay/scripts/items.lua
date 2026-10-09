-- Shared item labels and detail text are generated from the ROM into
-- dialogue/script-text.lua. This module maps source IDs to their stable keys.
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

return {
    catalog = catalog,
    detail_text_keys = detail_text_keys,
    detail_label_keys = {
        "items.detail_label.ammo",
        "items.detail_label.uses",
        "items.detail_label.rating",
    },
}
