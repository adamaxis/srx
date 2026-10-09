-- Shared UI choice lists. Native presenters own focus and input timing.
local UI = {}

function UI.confirmation()
    return { "yes", "no" }
end

-- Shared field-stat captions used by item and command overview panels.
function UI.field_metric_labels()
    return {
        "physical",
        "mental",
        "attack",
        "defense",
        "damage",
        "drain",
        "none",
        "no_armor",
        "weapons",
        "guns",
        "power",
    }
end

return UI
