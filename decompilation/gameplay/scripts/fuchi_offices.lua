-- ROM title service 14 (0x54152), scene 3/event 2.
-- After Hospital Visited is set and Cyberheart Safe is clear, the source
-- loads root 0x11B7F4 and leaves D6 at zero, preserving activity scene 21.
local E = require("enums")
local StoryFlags = require("story_flags")

local FuchiOffices = {}
local LOCATION_ID = "penumbra_site_02_fuchi"

function FuchiOffices.present_cyberheart_entry()
    if game.location.type ~= "building" or
       game.location.id ~= LOCATION_ID or
       StoryFlags.is_set(E.StoryFlag.HOSPITAL_VISITED) ~= true or
       StoryFlags.is_set(E.StoryFlag.CYBERHEART_SAFE) ~= false then
        return "denied"
    end

    StoryFlags.begin_trigger(E.StoryFlag.CYBERHEART_SAFE)
    game.ui.say("entry")
    return "enter_scene_21"
end

return FuchiOffices
