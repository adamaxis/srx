-- Shop content and selection behavior come from the ROM service tables.
-- Native presenters draw these Lua-owned states and apply typed transactions.
local E = require("enums")
local Catalog = require("data.shop_catalog")
local service_title, action_heading = Catalog.service_title, Catalog.action_heading

local Shop = {}

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

function Shop.advance_menu(controller_events)
    local menu = assert(game.shop_menu, "shop menu is not open")
    local definition = assert(Shop.catalog[menu.shop_id],
        "active shop menu definition is missing")
    local function snapshot(event)
        return menu_snapshot(menu, true, event)
    end
    local function move_cursor(delta)
        local maximum = menu.action_panel == 0
            and (menu.menu_result == 0 and 3 or 5)
            or 7
        local candidate = menu.action_cursor + delta
        if candidate < 0 then
            candidate = 0
        elseif candidate > maximum then
            candidate = maximum
        end
        if candidate == menu.action_cursor then
            return snapshot("no_change")
        end
        menu.action_cursor = candidate
        if menu.action_panel == 0 or menu.menu_result == 1 then
            menu.selected_action = candidate
        end
        return snapshot("selection_moved")
    end
    local function move_panel(delta)
        if menu.menu_result == 1 then
            return snapshot("no_change")
        end
        local candidate = menu.action_panel + delta
        if candidate < 0 or candidate > 1 then
            return snapshot("no_change")
        end
        if candidate == menu.action_panel then
            return snapshot("no_change")
        end
        menu.action_panel = candidate
        menu.action_cursor = 0
        if candidate == 0 then
            menu.selected_action = 0
        end
        return snapshot("panel_changed")
    end

    -- Source input priority follows 0x56AC0: cursor, panel, confirm,
    -- runner-menu setup, then return to the field caller. Masks are the
    -- FFDD22 edge byte: LEFT 0x0400, UP 0x0100, RIGHT 0x0800, DOWN 0x0200,
    -- A|B 0x5000, C 0x2000, START 0x8000 [C].
    if (controller_events & E.Pad.LEFT) ~= 0 then
        return move_cursor(-1)
    elseif (controller_events & E.Pad.UP) ~= 0 then
        return move_panel(-1)
    elseif (controller_events & E.Pad.RIGHT) ~= 0 then
        return move_cursor(1)
    elseif (controller_events & E.Pad.DOWN) ~= 0 then
        return move_panel(1)
    elseif (controller_events & (E.Pad.A | E.Pad.B)) ~= 0 then
        if menu.action_panel == 0 then
            menu.selected_action = menu.action_cursor
            return snapshot("no_change")
        end
        local group = definition.groups[
            menu.menu_result == 1 and 1 or menu.selected_action + 1]
        if group == nil then
            return snapshot("source_action_boundary")
        elseif menu.menu_result == 1 then
            return snapshot(group.equipment_upgrade_offers[
                menu.action_cursor + 1] and "equipment_upgrade_selected"
                or "offer_unavailable")
        elseif group.id == "sell" then
            return snapshot("sale_selected")
        elseif #group.progression_offers > 0 then
            return snapshot(group.progression_offers[menu.action_cursor + 1]
                and "progression_selected" or "offer_unavailable")
        elseif #group.offers > 0 then
            return snapshot(group.offers[menu.action_cursor + 1]
                and "purchase_selected" or "offer_unavailable")
        end
        return snapshot("source_action_boundary")
    elseif (controller_events & E.Pad.C) ~= 0 then
        menu.action_cursor = 0
        menu.action_panel = 0
        menu.selected_action = 0
        return snapshot("active_runner_change_requested")
    elseif (controller_events & E.Pad.START) ~= 0 then
        local closed = menu_snapshot(menu, false, "returned_to_field")
        game.shop_menu = nil
        return closed
    end
    return snapshot("no_change")
end

function Shop.reset_menu()
    local menu = assert(game.shop_menu, "shop menu is not open")
    menu.action_cursor = 0
    menu.action_panel = 0
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

return Shop
