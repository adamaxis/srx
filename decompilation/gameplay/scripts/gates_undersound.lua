-- ROM title service 15 (0x54222), scene 0/event 3.
-- The no-choice page at 0x1111DE returns to scene 0. The service clears
-- FFFC30 bit 26, sets bit 27, and calls 0x2196C for trigger 15.
local E = require("enums")
local StoryFlags = require("story_flags")

local GatesUndersound = {}
local LOCATION_ID = "downtown_site_04_gates_undersound"

function GatesUndersound.present_ogami_departure()
    if game.location.type ~= "building" or
       game.location.id ~= LOCATION_ID or
       game.field.event_code ~= 3 or game.field.alternate_result ~= 0 or
       StoryFlags.is_set(E.StoryFlag.MAKO_DEFEATED) ~= true or
       StoryFlags.is_set(E.StoryFlag.OGAMI_LEFT) ~= false then
        return "denied"
    end

    StoryFlags.begin_trigger(E.StoryFlag.OGAMI_LEFT)
    game.ui.say("entry")
    game.state.field_mode_effect_flag_bit_26 = false
    game.state.field_mode_effect_flag_bit_27 = true
    StoryFlags.finish_trigger(E.StoryFlag.OGAMI_LEFT)
    return "field_return"
end

return GatesUndersound
