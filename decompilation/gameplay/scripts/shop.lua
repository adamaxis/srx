-- Shop content and selection behavior come from the ROM service tables.
-- Native presenters draw these Lua-owned states and apply typed transactions.
local E = require("enums")
local Catalog = require("data.shop_catalog")
local service_title, action_heading = Catalog.service_title, Catalog.action_heading
local Route = require("menu.route")
local T = E.MenuTransition

local Shop = {}
local E = require("enums")

local function menu_snapshot(menu, open, event)
    return {
        open = open,
        shop_id = menu.shop_id,
        menu_mode = menu.menu_mode,
        menu_result = menu.menu_result,
        action_cursor = menu.action_cursor,
        action_panel = menu.action_panel,
        selected_action = menu.selected_action,
        event = event,
    }
end

function Shop.open_menu(shop_id)
    local definition = Shop.catalog[shop_id]
    assert(definition ~= nil, "unknown shop menu: " .. tostring(shop_id))
    assert(#definition.groups > 0 and #definition.groups <= 8,
        "shop menu has an invalid ROM action count")
    local menu = {
        shop_id = shop_id,
        menu_mode = definition.menu_mode,
        menu_result = definition.menu_result,
        action_cursor = 0,
        -- Result one starts on the direct eight-choice equipment selector.
        action_panel = definition.menu_result == 1 and 1 or 0,
        selected_action = 0,
    }
    game.shop_menu = menu
    return menu_snapshot(menu, true, "opened")
end

function Shop.advance_menu(controller_events, menu_b_policy,
                           transaction_active)
    local menu = assert(game.shop_menu, "shop menu is not open")
    -- The input route registers one generic shop screen. Its single policy
    -- gate handles B for all catalog entries; the transaction prompt remains
    -- a host service, but ALWAYS_CANCEL still preempts that prompt here.
    require("menu.field_init")
    local result = Route.dispatch_screen_input(
        E.MenuScreen.SHOP, menu, controller_events,
        { game = { menu_b_policy = menu_b_policy } }, menu_b_policy,
        transaction_active ~= true)
    if result.transition == T.EXIT then
        local closed = menu_snapshot(menu, false,
            result.event or "returned_to_field")
        game.shop_menu = nil
        return closed
    end
    return menu_snapshot(menu, true, result.event or "no_change")
end

function Shop.reset_menu()
    local menu = assert(game.shop_menu, "shop menu is not open")
    menu.action_cursor = 0
    menu.action_panel = menu.menu_result == 1 and 1 or 0
    menu.selected_action = 0
    return menu_snapshot(menu, true, "no_change")
end

function Shop.close_menu()
    game.shop_menu = nil
end

local function key_rows(ids, make_key)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = make_key(id)
    end
    return keys
end

-- Host open profile for a field shop route. menu_kind picks the native
-- menu layout (field_store, medicine_lodge, wire_masters); boundary_tag names
-- QA boundaries; gate "runner_deck_fact" means the store opens only when the
-- runner feature flag bit 0 (deck owned) is set, else the shared
-- "store_closed" field-mode notice shows [C ROM Microtronics gate].
local host_open_profiles = {
    ares_weapon_emporium = { boundary_tag = "ares" },
    medicine_lodge = { menu_kind = "medicine_lodge" },
    wire_masters = { menu_kind = "wire_masters" },
    microtronics = {
        gate = "runner_deck_fact",
        menu_kind = "microtronics",
    },
}

function Shop.host_open_profile(service_id)
    local profile = host_open_profiles[service_id] or {}
    return {
        menu_kind = profile.menu_kind or "field_store",
        boundary_tag = profile.boundary_tag or service_id,
        gate = profile.gate or "open",
    }
end

function Shop.services()
    local ids = {
        "weapons_world",
        "ares_weapon_emporium",
        "crime_mall",
        "medicine_lodge",
        "riannons",
        "merlins_lore",
        "microtronics",
        "wire_masters",
        "little_chiba",
        "dr_bobs",
        "council_island_hospital",
        "seattle_general_hospital",
    }
    return ids, nil, key_rows(ids, service_title)
end

function Shop.action_headings()
    local ids = {
        "ares_weapons_display",
        "ares_weapons_available",
        "ares_armor_display",
        "ares_armor_available",
        "ares_other_items_display",
        "ares_other_items_equipment",
        "ares_weapon_accessories_display",
        "ares_weapon_accessories_accessories",
        "ares_firearm_accessories_select",
        "ares_firearm_accessories_to_accessorize",
        "ares_sell_item_from",
        "ares_sell_inventory",
        "combat_utilities_display",
        "combat_utilities",
        "defense_utilities_display",
        "defense_utilities",
        "mask_sense_utilities_display",
        "mask_sense_utilities",
        "cyberdecks_display",
        "cyberdecks",
        "cyberdeck_upgrades_display",
        "cyberdeck_upgrades",
        "cyberdeck_upgrades_second_display",
        "cyberdeck_upgrades_second",
        "magic_spells_display",
        "magic_spells",
        "magic_items_display",
        "magic_items",
        "magic_foci_display",
        "magic_foci",
        "sell_item_from_secondary",
        "sell_inventory_secondary",
    }
    return ids, nil, key_rows(ids, action_heading)
end

Shop.catalog = Catalog.catalog

function Shop.cyberdeck_rating_upgrade_quote(stat, model, mpcp, persona)
    return Catalog.cyberdeck_rating_upgrade_quote(stat, model, mpcp, persona)
end

return Shop
