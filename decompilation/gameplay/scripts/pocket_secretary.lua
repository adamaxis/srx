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

return PocketSecretary
