-- ROM title service 14 (0x54152) loads the source entry page, then leaves
-- D6 at zero so the selected event-table route continues into activity 21.
local DialogueGraph = require("dialogue_graph")
local E = require("enums")
local StoryFlags = require("story_flags")

local FuchiOffices = {}
local LOCATION_ID = "penumbra_site_02_fuchi"

function FuchiOffices.present_cyberheart_entry()
    if game.location.type == "building" and
       game.location.id == LOCATION_ID and
       game.field.package_run_status == E.PackageRunStatus.NONE and
       StoryFlags.is_set(E.StoryFlag.HOSPITAL_VISITED) == true and
       StoryFlags.is_set(E.StoryFlag.CYBERHEART_SAFE) == false then
        -- 0x5418C arms the cyber-heart safe before opening the entry page.
        assert(game.services.call(
            "field", "arm_fuchi_cyberheart_safe") == "applied",
            "could not arm the Fuchi cyber-heart safe")
    end
    return DialogueGraph.run("fuchi_offices.cyberheart_entry")
end

function FuchiOffices.present_cyberheart_safe_found()
    return DialogueGraph.run("fuchi_offices.cyberheart_safe_found")
end

return FuchiOffices
