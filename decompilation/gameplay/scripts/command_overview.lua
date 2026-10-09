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
    }, nil, {
        "field.command_overview.option.inventory_left",
        "field.command_overview.option.inventory_right",
        "field.command_overview.option.clips",
        "field.command_overview.option.attributes_skills",
        "field.command_overview.option.cyberware",
        "field.command_overview.option.magic",
        "field.command_overview.option.pocket_secretary",
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
