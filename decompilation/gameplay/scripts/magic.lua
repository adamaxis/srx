-- Stable spell IDs follow the ROM D4EA0 name/description table order.
-- The native spell table filters this catalog for the active runner.
local Magic = {}

local spell_ids = {
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

local function text_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

function Magic.spells()
    return spell_ids, nil, text_keys(spell_ids, "magic.spells.option.")
end

function Magic.spell_descriptions()
    return spell_ids, nil,
        text_keys(spell_ids, "magic.spells.description.")
end

-- Shared Magic panel text. Runner archetype determines which native panel is
-- shown; both variants resolve their copy from this source-backed catalog.
function Magic.panel_text_entries()
    local ids = {
        "defense_title",
        "force_posture_heading",
        "success_label",
        "force_label",
        "availability_label",
        "panel_metric_labels",
        "panel_force_posture_labels",
    }
    return ids, nil, text_keys(ids, "magic.panel.text.option.")
end

return Magic
