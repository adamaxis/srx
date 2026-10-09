-- Shared field-object prompt choices. Native code validates these IDs and
-- maps them to typed interaction commands; ROM-derived labels stay in the
-- shared script-text resource.
local FieldInteractions = {}

function FieldInteractions.maglock_intro()
    return { "more" }
end

function FieldInteractions.maglock_actions()
    return { "electronics", "grenade", "walk-away" }
end

function FieldInteractions.security_terminal_actions()
    return { "electronics", "walk-away" }
end

return FieldInteractions
