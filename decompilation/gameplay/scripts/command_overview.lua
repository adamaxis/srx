-- Stable root choices for the in-game command menu.
-- FieldSceneRuntime owns page state, selection cadence, drawing, and actions.
local CommandOverview = {}

function CommandOverview.root()
    return {
        "inventory_left",
        "inventory_right",
        "clips",
        "attributes_skills",
        "cyberware",
        "magic",
        "pocket_secretary",
    }
end

-- Display labels shared by the runner summary and its posture selector.
function CommandOverview.races()
    return { "human", "elf", "dwarf", "orc", "troll" }
end

function CommandOverview.classes()
    return { "mage", "shaman", "samurai", "decker" }
end

function CommandOverview.postures()
    return {
        "full_defense",
        "medium_defense",
        "neutral",
        "medium_offense",
        "full_offense",
    }
end

return CommandOverview
