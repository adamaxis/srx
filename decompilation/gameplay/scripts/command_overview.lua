-- Stable root choices for the in-game command menu.
-- FieldSceneRuntime owns page state, selection cadence, drawing, and actions.
local CommandOverview = {}

local function label_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

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
    local ids = { "human", "elf", "dwarf", "orc", "troll" }
    return ids, nil, label_keys(ids, "command_overview.race.option.")
end

function CommandOverview.classes()
    local ids = { "mage", "shaman", "samurai", "decker" }
    return ids, nil, label_keys(ids, "command_overview.class.option.")
end

function CommandOverview.postures()
    local ids = {
        "full_defense",
        "medium_defense",
        "neutral",
        "medium_offense",
        "full_offense",
    }
    return ids, nil, label_keys(ids, "command_overview.posture.option.")
end

return CommandOverview
