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

-- Shared Magic panel text. Runner archetype determines which native panel is
-- shown; both variants resolve their copy from this source-backed catalog.
function Magic.panel_text_entries()
    return {
        "defense_title",
        "force_posture_heading",
        "success_label",
        "force_label",
        "availability_label",
        "panel_metric_labels",
        "panel_force_posture_labels",
    }
end

return Magic
