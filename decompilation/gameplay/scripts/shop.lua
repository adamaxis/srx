-- Shared shop catalog. The native menu owns drawing, focus, input cadence,
-- and transactions; Lua supplies the available stock for each category.
local function offer(kind, value, quantity)
    return {
        type = kind,
        value = value or 0,
        quantity = quantity or 0,
    }
end

local function group(id, offers)
    return { id = id, offers = offers or {} }
end

return {
    catalog = {
        ares_weapon_emporium = {
            -- Keep these six groups in the order used by the shop selector.
            groups = {
                group("weapons", {
                    offer(0x01),
                    offer(0x02),
                    offer(0x06, 0x00, 0x0F),
                    offer(0x04),
                    offer(0x05, 0x00, 0x0D),
                    offer(0x0D, 0x06, 0x14),
                    offer(0x0E, 0x06),
                }),
                group("other_items", {
                    offer(0x45, 0x00, 0x19),
                    offer(0x0F, 0x06),
                    offer(0x10, 0x04, 0x07),
                    offer(0x11, 0x04, 0x07),
                }),
                group("armor", {
                    offer(0x2E),
                    offer(0x2D, 0x00, 0x0C),
                }),
                group("source_action_3"),
                group("source_action_4"),
                group("sell"),
            },
        },
    },
}
