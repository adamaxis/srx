-- ROM labels used by the Command equipment summary.
local E = require("enums")

return {
    -- ROM bytes at D1A7E contain three leading spaces before Empty, then
    -- four before Hand; 0x80 separates the two displayed lines. [C]
    [E.CommandStatusAction.EMPTY_HAND] = "   Empty\n    Hand",
    [E.CommandStatusDamage.STUN] = "Stun", -- D1AA5 [C]
    [E.CommandStatusArmor.NONE] = "No Armor", -- D1F45 [C]
}
