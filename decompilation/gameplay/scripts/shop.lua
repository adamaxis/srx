-- Shop content and selection behavior come from the ROM service tables.
-- Native presenters draw these Lua-owned states and apply typed transactions.
local E = require("enums")

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

local function service_title(id)
    return "shop.service.option." .. id
end

local function action_heading(id)
    return "shop.action_heading.option." .. id
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

local function offer(kind, value, quantity)
    return {
        type = kind,
        value = value or 0,
        quantity = quantity or 0,
    }
end

local function progression_offer(action_code, source_metadata, base_cost)
    return {
        action_code = action_code,
        source_metadata = source_metadata,
        base_cost = base_cost,
    }
end

local function equipment_upgrade_offer(category, label_id, selection_id,
                                       description_id, base_cost)
    return {
        category = category,
        label_key = "cyberware.text.option." .. label_id,
        selected_name_key = "shop.cyberware.selection_name." .. selection_id,
        description_key = "shop.cyberware.description." .. description_id,
        base_cost = base_cost,
    }
end

local function group(id, first_title_key, second_title_key, offers,
                     progression_offers, equipment_upgrade_offers)
    return {
        id = id,
        title_keys = { first_title_key, second_title_key },
        offers = offers or {},
        progression_offers = progression_offers or {},
        equipment_upgrade_offers = equipment_upgrade_offers or {},
    }
end

local function field_shop_text_keys()
    return {
        universal_clips_title = "shop.universal_clips.title",
        universal_clips_description = "shop.universal_clips.description",
        transaction_prompt = "shop.transaction.prompt",
        no_inventory_slot = "shop.transaction.no_inventory_slot",
        insufficient_funds = "shop.transaction.insufficient_funds",
        special_action_limit = "shop.transaction.special_action_limit",
        protected_sale = "shop.transaction.protected_sale",
        sale_quote_heading = "shop.transaction.sale_quote_heading",
        purchase_price_heading = "shop.transaction.purchase_price_heading",
        progression_training_unavailable = "shop.progression.training_unavailable",
        progression_maximum_level = "shop.progression.maximum_level",
        progression_cost_heading = "shop.progression.cost_heading",
        progression_level_label = "shop.progression.level_label",
        cyberware_already_owned = "shop.cyberware.already_owned",
        cyberware_spurs_conflict = "shop.cyberware.spurs_conflict",
        cyberware_insufficient_essence =
            "shop.cyberware.insufficient_essence",
        cyberware_purchase_heading = "shop.cyberware.purchase_heading",
        cyberware_essence_heading = "shop.cyberware.essence_heading",
        cyberware_essence_cost_heading =
            "shop.cyberware.essence_cost_heading",
    }
end

local function field_action_groups(weapons, armor, equipment)
    return {
        group("weapons", action_heading("ares_weapons_display"),
              action_heading("ares_weapons_available"), weapons),
        group("armor", action_heading("ares_armor_display"),
              action_heading("ares_armor_available"), armor),
        group("other_items", action_heading("ares_other_items_display"),
              action_heading("ares_other_items_equipment"), equipment),
        group("source_action_3",
              action_heading("ares_weapon_accessories_display"),
              action_heading("ares_weapon_accessories_accessories")),
        group("source_action_4",
              action_heading("ares_firearm_accessories_select"),
              action_heading("ares_firearm_accessories_to_accessorize")),
        group("sell", action_heading("ares_sell_item_from"),
              action_heading("ares_sell_inventory")),
    }
end

local function magic_field_shop_groups(spells, items, foci)
    return {
        group("magic_spells", action_heading("magic_spells_display"),
              action_heading("magic_spells"), {}, spells),
        group("magic_items", action_heading("magic_items_display"),
              action_heading("magic_items"), items),
        group("magic_foci", action_heading("magic_foci_display"),
              action_heading("magic_foci"), foci),
        group("sell", action_heading("sell_item_from_secondary"),
              action_heading("sell_inventory_secondary")),
    }
end

Shop.catalog = {
    weapons_world = {
        menu_mode = 0,
        menu_result = -1,
        title_key = service_title("weapons_world"),
        text_keys = field_shop_text_keys(),
        -- The mode-0 rows are at 0x1AAC86 (Weapons), 0x1AADBF
        -- (Armor), and 0x1AAD77 (Equipment), selected by ROM 0x56DC4.
        groups = field_action_groups({
            offer(0x03, 0x00, 0x0F),
            offer(0x04, 0x00, 0x0B),
            offer(0x05, 0x00, 0x0E),
            offer(0x06, 0x00, 0x0C),
            offer(0x0B, 0x00, 0x0F),
            offer(0x0E, 0x06, 0x00),
        }, {
            offer(0x2B),
            offer(0x2C),
        }, {
            offer(0x45, 0x00, 0x28),
            offer(0x0F, 0x06, 0x14),
            offer(0x11, 0x04, 0x00),
            offer(0x10, 0x04, 0x00),
        }),
    },
    merlins_lore = {
        menu_mode = 5,
        menu_result = 0,
        title_key = service_title("merlins_lore"),
        text_keys = field_shop_text_keys(),
        -- ROM mode 5 selects progression rows at 0x1AAF80 and item rows at
        -- 0x1AAF28/0x1AAEE0. Result zero selects headings at 0x1AAAA7.
        groups = magic_field_shop_groups({
            progression_offer(0x07, 0x06, 1000),
            progression_offer(0x08, 0x06, 1000),
            progression_offer(0x09, 0x06, 1000),
            progression_offer(0x06, 0x06, 1000),
            progression_offer(0x0A, 0x06, 1000),
            progression_offer(0x0B, 0x06, 1000),
        }, {
            offer(0x12, 0x0A, 0x0D),
            offer(0x26, 0x04, 0x1E),
            offer(0x27, 0x03, 0x14),
            offer(0x27, 0x04, 0x20),
        }, {
            offer(0x25, 0x03, 0x12),
            offer(0x1D, 0x02, 0x00),
            offer(0x21, 0x02, 0x00),
            offer(0x21, 0x04, 0x20),
            offer(0x22, 0x02, 0x00),
            offer(0x22, 0x03, 0x16),
        }),
    },
    ares_weapon_emporium = {
        menu_mode = 1,
        menu_result = -1,
        title_key = service_title("ares_weapon_emporium"),
        text_keys = field_shop_text_keys(),
        -- ROM 0x56DC4 selects the mode-1 rows at 0x1AAC86 (cursor 0),
        -- 0x1AADBF (cursor 1), and 0x1AAD77 (cursor 2). The title rows at
        -- 0x1AA8AF label those groups Weapons, Armor, and Equipment.
        groups = field_action_groups({
                offer(0x01),
                offer(0x02),
                offer(0x06, 0x00, 0x0F),
                offer(0x04),
                offer(0x05, 0x00, 0x0D),
                offer(0x0D, 0x06, 0x14),
                offer(0x0E, 0x06),
            }, {
                offer(0x2E),
                offer(0x2D, 0x00, 0x0C),
            }, {
                offer(0x45, 0x00, 0x19),
                offer(0x0F, 0x06),
                offer(0x10, 0x04, 0x07),
                offer(0x11, 0x04, 0x07),
            }),
    },
    medicine_lodge = {
        menu_mode = 3,
        menu_result = 0,
        title_key = service_title("medicine_lodge"),
        text_keys = field_shop_text_keys(),
        -- Result zero selects the four ROM heading pairs at 0x1AAAA7.
        -- Group zero is the mode-three spell progression list at 0x1AAF40.
        -- Groups one and two are three-byte item offers from 0x1AAEF8 and
        -- 0x1AAEB0; their raw bytes stay separate from spell progression.
        groups = magic_field_shop_groups({
            progression_offer(0x03, 0x06, 1000),
            progression_offer(0x04, 0x06, 1000),
            progression_offer(0x0C, 0x06, 1000),
            progression_offer(0x05, 0x06, 1000),
            progression_offer(0x0B, 0x06, 1000),
        }, {
                offer(0x12, 0x0A, 0x00),
                offer(0x26, 0x04, 0x14),
        }, {
                offer(0x1B, 0x02, 0x00),
                offer(0x1B, 0x03, 0x14),
                offer(0x1C, 0x02, 0x00),
                offer(0x1C, 0x03, 0x14),
                offer(0x24, 0x02, 0x00), -- Heal Wounds focus, 0x1AAEBC [C]
                offer(0x24, 0x03, 0x14),
        }),
    },
    wire_masters = {
        menu_mode = 7,
        menu_result = 1,
        title_key = service_title("wire_masters"),
        text_keys = field_shop_text_keys(),
        -- The selected category resolves into a cyberware flag at +0x90.
        -- Prices are the eight 32-bit rows at 0x1AB060 + category * 4;
        -- essence thresholds are the target-bit-indexed bytes at 0x1AA8A0.
        equipment_upgrade_meter_thresholds = {
            2, 2, 1, 2, 5, 10, 10, 10,
            10, 5, 5, 5, 20, 10, 20, 32,
        },
        groups = {
            group("cyberware", service_title("wire_masters"),
                  service_title("wire_masters"), {}, {}, {
                equipment_upgrade_offer(0, "datajack", "datajack",
                                        "datajack", 500),
                equipment_upgrade_offer(1, "cyber_eyes", "cyber_eyes",
                                        "cyber_eyes", 3000),
                equipment_upgrade_offer(2, "hand_razors", "hand_razors",
                                        "hand_razors", 1500),
                equipment_upgrade_offer(3, "spurs", "spurs", "spurs",
                                        4000),
                equipment_upgrade_offer(4, "smartlink", "smartlink",
                                        "smartlink", 1500),
                equipment_upgrade_offer(5, "muscle_replace_1",
                                        "muscle_replacement", "muscle_replace",
                                        10000),
                equipment_upgrade_offer(6, "dermal_plating_1",
                                        "dermal_plating", "dermal_plating",
                                        3000),
                equipment_upgrade_offer(7, "wired_reflexes_1",
                                        "wired_reflexes", "wired_reflexes",
                                        20000),
            }),
        },
    },
}

return Shop
