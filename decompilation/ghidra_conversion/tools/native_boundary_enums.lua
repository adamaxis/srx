-- Native adapter enums derived from the source menu selector table.
-- These values are for translating the typed Lua section to the legacy
-- selector expected by existing C++ rendering/conversion helpers. Gameplay
-- scripts must use E.CommandSection from gameplay/scripts/enums.lua instead.
return {
    MenuPage = {
        INVENTORY_LEFT = 0,
        INVENTORY_RIGHT = 1,
        CLIPS = 2,
        ATTRIBUTES_SKILLS = 3,
        CYBERWARE = 4,
        MAGIC = 5,
        POCKET_SECRETARY = 6,
        COUNT = 7,
    },
    MenuPageHandler = {
        CLIPS = 0x11B58,
        ATTRIBUTES_SKILLS = 0xD01C,
        CYBERWARE = 0x11950,
        MAGIC = 0xFBA6,
        POCKET_SECRETARY = 0xC2A6,
    },
    ItemAction = {
        EQUIP_OR_USE = 2,
        DISCARD = 3,
        TRADE = 4,
        CANCEL = 5,
    },
    ClipsCommand = {
        RELOAD = 2,
        TRADE = 3,
        DISTRIBUTE = 4,
        CANCEL = 5,
    },
    ClipTradeCommand = {
        INCREASE = 2,
        DECREASE = 3,
        TRADE = 4,
        CHANGE_RECIPIENT = 5,
    },
}
