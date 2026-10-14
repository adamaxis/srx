-- Shared UI choice lists. Native presenters own focus and input timing.
local UI = {}

function UI.confirmation()
    local ids = { "yes", "no" }
    return ids, nil, {
        "ui.option.yes",
        "ui.option.no",
    }
end

-- Shared field-stat captions used by item and command overview panels.
function UI.field_metric_labels()
    local ids = {
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
    local label_keys = {}
    for index, id in ipairs(ids) do
        label_keys[index] = "ui.field_metric.option." .. id
    end
    return ids, nil, label_keys
end

return UI
