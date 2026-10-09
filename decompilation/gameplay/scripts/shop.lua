-- Shop content describes the source order, labels, and available stock.
-- The native menu still owns drawing, focus, input cadence, and transactions.
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

return {
    catalog = {
        ares_weapon_emporium = {
            title_key = "shop.ares.title",
            text_keys = {
                transaction_prompt = "shop.transaction.prompt",
                no_inventory_slot = "shop.transaction.no_inventory_slot",
                insufficient_funds = "shop.transaction.insufficient_funds",
                special_action_limit = "shop.transaction.special_action_limit",
                protected_sale = "shop.transaction.protected_sale",
                sale_quote_heading = "shop.transaction.sale_quote_heading",
                purchase_price_heading = "shop.transaction.purchase_price_heading",
            },
            -- Keep these six groups in the order used by the shop selector.
            groups = {
                group(
                    "weapons", "shop.ares.weapons.display",
                    "shop.ares.weapons.available", {
                    offer(0x01),
                    offer(0x02),
                    offer(0x06, 0x00, 0x0F),
                    offer(0x04),
                    offer(0x05, 0x00, 0x0D),
                    offer(0x0D, 0x06, 0x14),
                    offer(0x0E, 0x06),
                }),
                group(
                    "other_items", "shop.ares.other_items.display",
                    "shop.ares.other_items.equipment", {
                    offer(0x45, 0x00, 0x19),
                    offer(0x0F, 0x06),
                    offer(0x10, 0x04, 0x07),
                    offer(0x11, 0x04, 0x07),
                }),
                group("armor", "shop.ares.armor.display",
                      "shop.ares.armor.available", {
                    offer(0x2E),
                    offer(0x2D, 0x00, 0x0C),
                }),
                group(
                    "source_action_3", "shop.ares.weapon_accessories.display",
                    "shop.ares.weapon_accessories.accessories"),
                group(
                    "source_action_4", "shop.ares.firearm_accessories.select",
                    "shop.ares.firearm_accessories.to_accessorize"),
                group("sell", "shop.ares.sell.item_from",
                      "shop.ares.sell.inventory"),
            },
        },
    },
}
