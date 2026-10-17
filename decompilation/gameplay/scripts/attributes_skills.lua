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

-- ROM 0xC1B70: 20 pointers to the bottom-panel description of each
-- attribute and skill, in label order (0x80 = line break).
function AttributesSkills.descriptions()
    local ids, _, _ = AttributesSkills.text_entries()
    local description_ids = {}
    local description_keys = {}
    for index = 1, 20 do
        description_ids[index] = ids[index]
        description_keys[index] = "attributes_skills.description." .. ids[index]
    end
    return description_ids, nil, description_keys
end

return AttributesSkills
