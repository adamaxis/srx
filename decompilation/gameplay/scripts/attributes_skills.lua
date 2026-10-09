-- Stable IDs for the ROM-ordered Attributes/Skills screen copy.
-- Native state owns the page, cursor, upgrades, and karma spending.
local AttributesSkills = {}

function AttributesSkills.text_entries()
    return {
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
end

return AttributesSkills
