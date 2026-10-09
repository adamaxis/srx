-- Shared UI choice lists. Native presenters own focus and input timing.
local UI = {}

function UI.confirmation()
    return { "yes", "no" }
end

return UI
