-- Shop content describes the ROM's service-title and action-heading tables,
-- plus available stock. Native presenters own layout, focus, input cadence,
-- and transactions.
local Shop = {}

function Shop.services()
    return {
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
end

function Shop.action_headings()
    return {
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
end

local function service_title(id)
    return "shop.service.option." .. id
end

local function action_heading(id)
    return "shop.action_heading.option." .. id
end

local function offer(kind, value, quantity)
    return {
        type = kind,
        value = value or 0,
        quantity = quantity or 0,
    }
end

local function group(id, first_title_key, second_title_key, offers)
    return {
        id = id,
        title_keys = { first_title_key, second_title_key },
        offers = offers or {},
    }
end

Shop.catalog = {
    ares_weapon_emporium = {
        title_key = service_title("ares_weapon_emporium"),
        text_keys = {
            universal_clips_title = "shop.universal_clips.title",
            universal_clips_description = "shop.universal_clips.description",
            transaction_prompt = "shop.transaction.prompt",
            no_inventory_slot = "shop.transaction.no_inventory_slot",
            insufficient_funds = "shop.transaction.insufficient_funds",
            special_action_limit = "shop.transaction.special_action_limit",
            protected_sale = "shop.transaction.protected_sale",
            sale_quote_heading = "shop.transaction.sale_quote_heading",
            purchase_price_heading = "shop.transaction.purchase_price_heading",
        },
        -- Keep these six groups in the order used by the ROM selector.
        groups = {
            group(
                "weapons", action_heading("ares_weapons_display"),
                action_heading("ares_weapons_available"), {
                offer(0x01),
                offer(0x02),
                offer(0x06, 0x00, 0x0F),
                offer(0x04),
                offer(0x05, 0x00, 0x0D),
                offer(0x0D, 0x06, 0x14),
                offer(0x0E, 0x06),
            }),
            group(
                "other_items", action_heading("ares_other_items_display"),
                action_heading("ares_other_items_equipment"), {
                offer(0x45, 0x00, 0x19),
                offer(0x0F, 0x06),
                offer(0x10, 0x04, 0x07),
                offer(0x11, 0x04, 0x07),
            }),
            group("armor", action_heading("ares_armor_display"),
                  action_heading("ares_armor_available"), {
                offer(0x2E),
                offer(0x2D, 0x00, 0x0C),
            }),
            group(
                "source_action_3",
                action_heading("ares_weapon_accessories_display"),
                action_heading("ares_weapon_accessories_accessories")),
            group(
                "source_action_4",
                action_heading("ares_firearm_accessories_select"),
                action_heading("ares_firearm_accessories_to_accessorize")),
            group("sell", action_heading("ares_sell_item_from"),
                  action_heading("ares_sell_inventory")),
        },
    },
}

return Shop
