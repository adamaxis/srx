-- ROM title service 12 (0x54076), scene 5/event 6.
-- The one-shot opening is gated by FFFC1E bit 16 set, bit 12 clear, and
-- FFF0D5 == 0. It loads dialogue root 0x110A2A, then leaves D6 at zero so
-- the scene's event-table destination (resource activity 35) still applies.
local E = require("enums")
local StoryFlags = require("story_flags")

local RenrakuOffices = {}
local LOCATION_ID = "renraku_offices_frontage"

function RenrakuOffices.present_map_overlay_entry()
    if game.location.type ~= "building" or
       game.location.id ~= LOCATION_ID or
       StoryFlags.is_set(E.StoryFlag.ITO_FIGHT_WON) ~= true or
       StoryFlags.is_set(E.StoryFlag.RENRAKU_MAP_OVERLAY) ~= false then
        return "denied"
    end

    StoryFlags.begin_trigger(E.StoryFlag.RENRAKU_MAP_OVERLAY)
    game.ui.say("entry")
    return "enter_scene_35"
end

return RenrakuOffices
