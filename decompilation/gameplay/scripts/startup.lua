-- Stable choice identifiers for the three title-screen cursor menus.
-- The native startup controller owns selection, input timing, and transitions.
local Startup = {}

function Startup.options()
    return { "new_game", "load_game" }
end

function Startup.save_slots()
    return { "slot_one", "slot_two" }
end

function Startup.archetypes()
    return { "samurai", "decker", "gator_shaman" }
end

return Startup
