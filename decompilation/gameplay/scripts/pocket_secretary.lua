-- Stable root choices for the field Pocket Secretary menu.
-- The native presenter owns selection cadence, page drawing, and handlers.
local PocketSecretary = {}

function PocketSecretary.root()
    return {
        "notebook",
        "group_items",
        "cyberdeck",
        "dismiss_runner",
        "save_load",
    }
end

-- All possible Notebook rows in ROM order. The native model applies the
-- Contacts, Mr. Johnsons, and Shadowrunners story-flag gates at runtime.
function PocketSecretary.notebook()
    return {
        "current_run",
        "tips_and_clues",
        "contacts",
        "mr_johnsons",
        "shadowrunners",
    }
end

return PocketSecretary
