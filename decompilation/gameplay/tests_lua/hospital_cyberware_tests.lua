package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Hospital = require("council_island_hospital")
local ShopCatalog = require("data.shop_catalog")

local inputs = { "down", "confirm" }
local cursor_calls = 0
game = {
    location = {
        type = "building",
        id = "little_chibas_chop_shop",
    },
    field = { package_run_status = "none" },
    ui = {
        cursor_menu = function(menu_id, choices, selected, narrator,
                               text_profile)
            cursor_calls = cursor_calls + 1
            assert(menu_id == "hospital_options")
            assert(choices[1] == "medical_attention" and
                   choices[2] == "cyberware" and choices[3] == "exit")
            assert(narrator == "little_chiba_narrator")
            assert(text_profile == "little_chiba")
            if cursor_calls == 1 then
                assert(selected == 1)
            else
                assert(selected == 2)
            end
            return assert(table.remove(inputs, 1))
        end,
    },
}

assert(Hospital.present_options() == "hospital_cyberware_requested",
       "Little Chiba Cyberware selection must request the ROM service")
assert(cursor_calls == 2 and #inputs == 0,
       "Cyberware selection must follow the hospital cursor path")

local service = assert(ShopCatalog.catalog.wire_masters)
assert(service.menu_mode == 7 and service.menu_result == 1)
assert(#service.groups == 1 and service.groups[1].id == "cyberware")
local offer_text = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local Json = require("json")
local script_text = Json.decode(offer_text:read("*a"))
offer_text:close()
local hospital_catalogs = {
    {
        "little_chiba_cyberware",
        "shop.service.option.little_chiba",
        "Little Chiba",
    },
    {
        "council_island_hospital_cyberware",
        "shop.service.option.council_island_hospital",
        "Council Island Hospital",
    },
    {
        "seattle_general_hospital_cyberware",
        "shop.service.option.seattle_general_hospital",
        "Seattle General Hospital",
    },
}
local little_chiba
for _, row in ipairs(hospital_catalogs) do
    local catalog = assert(ShopCatalog.catalog[row[1]])
    assert(catalog.menu_mode == service.menu_mode and
           catalog.menu_result == service.menu_result and
           catalog.title_key == row[2] and #catalog.groups == 1 and
           catalog.groups[1].id == service.groups[1].id and
           catalog.groups[1].title_keys[1] == row[2] and
           catalog.groups[1].title_keys[2] == row[2] and
           catalog.groups[1].equipment_upgrade_offers ==
               service.groups[1].equipment_upgrade_offers and
           catalog.equipment_upgrade_meter_thresholds ==
               service.equipment_upgrade_meter_thresholds)
    assert(script_text.texts[row[2]] == row[3])
    if row[1] == "little_chiba_cyberware" then
        little_chiba = catalog
    end
end
local offers = little_chiba.groups[1].equipment_upgrade_offers
local expected_prices = { 500, 3000, 1500, 4000, 1500, 10000, 3000, 20000 }
assert(#offers == #expected_prices,
       "ROM cyberware service must expose all eight stock rows")
for index, expected in ipairs(expected_prices) do
    assert(offers[index].category == index - 1 and
           offers[index].base_cost == expected,
           "cyberware row " .. (index - 1) ..
           " must match the ROM 0x1AB060 price table")
end

local expected_essence = {
    2, 2, 1, 2, 5, 10, 10, 10,
    10, 5, 5, 5, 20, 10, 20, 32,
}
assert(#service.equipment_upgrade_meter_thresholds == #expected_essence)
for index, expected in ipairs(expected_essence) do
    assert(service.equipment_upgrade_meter_thresholds[index] == expected,
           "cyberware essence threshold " .. (index - 1) ..
           " must match ROM 0x1AA8A0")
end

-- Site -> catalog view (FFF0B6 title roots 0x0B / 0x0A / 0x08).
local site_catalogs = {
    downtown_site_12_seattle_general_hospital =
        "seattle_general_hospital_cyberware",
    council_island_hospital = "council_island_hospital_cyberware",
    little_chibas_chop_shop = "little_chiba_cyberware",
}
for location_id, catalog_id in pairs(site_catalogs) do
    game.location = { type = "building", id = location_id }
    assert(Hospital.cyberware_catalog_id() == catalog_id,
           location_id .. " must open " .. catalog_id)
    assert(ShopCatalog.catalog[catalog_id] ~= nil)
end

print("Little Chiba Cyberware route and ROM catalog passed")
