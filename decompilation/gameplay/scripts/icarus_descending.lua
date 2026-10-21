-- Scene 12/event 30, ROM 0x20990. The bodyguards block the booth while
-- FFFC1E bit 10 is clear. The accessible branch uses the shared Johnson
-- interaction at the contact profile's ROM-derived run context.
local IcarusDescending = {}

local LOCATION_ID = "icarus_descending_booth"

function IcarusDescending.present_access_blocked()
    if game.location.type ~= "building" or
       game.location.id ~= LOCATION_ID or
       game.field.event_code ~= 30 or
       game.state.story_flag_bit_10 ~= false then
        return "denied"
    end

    game.ui.say("access_blocked")
    return "field_return"
end

return IcarusDescending
