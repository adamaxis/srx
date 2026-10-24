-- Shop catalog: ROM shop rows as data. Row tables and selectors are cited
-- inline (0x56DC4 / 0x5794E item rows, 0x1AAF40 progression, 0x1AB060
-- Wire Masters). shop.lua owns the menu logic.
local M = {}

function M.service_title(id)
    return "shop.service.option." .. id
end

function M.action_heading(id)
    return "shop.action_heading.option." .. id
end
local service_title, action_heading = M.service_title, M.action_heading


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

M.catalog = {
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

return M
