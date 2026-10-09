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

return CommandOverview
