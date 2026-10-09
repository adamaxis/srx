-- Stable IDs for the ROM-ordered Attributes/Skills screen copy.
-- Native state owns the page, cursor, upgrades, and karma spending.
local AttributesSkills = {}

function AttributesSkills.text_entries()
    local ids = {
        "body",
        "quickness",
        "strength",
        "charisma",
        "intelligence",
        "willpower",
        "essence",
        "magic",
        "sorcery",
        "firearms",
        "pistols",
        "smgs",
        "shotguns",
        "melee_combat",
        "throwing",
        "computer",
        "biotech",
        "electronics",
        "reputation",
        "negotiation",
        "upgrade_header",
        "maximum_level",
        "mage_only",
        "insufficient_karma",
        "upgrade_prompt",
    }
    local label_keys = {}
    for index, id in ipairs(ids) do
        label_keys[index] = "attributes_skills.text.option." .. id
    end
    return ids, nil, label_keys
end

return AttributesSkills
