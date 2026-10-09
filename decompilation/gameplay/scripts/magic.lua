-- Stable spell IDs follow the ROM D4EA0 name/description table order.
-- The native spell table filters this catalog for the active runner.
local Magic = {}

function Magic.spells()
    return {
        "flame_dart",
        "flame_bolt",
        "hellblast",
        "invisibility",
        "sleep",
        "stink",
        "super_barrier",
        "mana_zap",
        "mana_blast",
        "mana_storm",
        "rockskin",
        "heal_wounds",
        "confusion",
    }
end

return Magic
