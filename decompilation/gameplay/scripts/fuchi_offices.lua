-- ROM title service 14 (0x54152) loads the source entry page, then leaves
-- D6 at zero so the selected event-table route continues into activity 21.
local DialogueGraph = require("dialogue_graph")
local E = require("enums")
local StoryFlags = require("story_flags")
local StoryState = require("story_state")

local FuchiOffices = {}

function FuchiOffices.present_cyberheart_entry()
    local scene = game.location.scene_id
    if scene == E.FieldSafe.FUCHI_ENTRY_SCENE and
       game.field.event_code == E.FieldSafe.FUCHI_ENTRY_EVENT and
       game.field.alternate_result == 0 and
       StoryFlags.is_set(E.StoryFlag.HOSPITAL_VISITED) == true and
       StoryFlags.is_set(E.StoryFlag.CYBERHEART_SAFE) == false then
        -- 0x5418C sets FFE187 before opening the 0x11B7F4 entry dialogue.
        assert(StoryState.set_byte(
            E.StoryByte.FUCHI_SAFE_CONTEXT,
            E.FieldSafe.FUCHI_SAFE_CONTEXT_VALUE),
            "could not arm the 0x541B2 Fuchi safe context")
    end
    return DialogueGraph.run("fuchi_offices.cyberheart_entry")
end

function FuchiOffices.present_cyberheart_safe_found()
    return DialogueGraph.run("fuchi_offices.cyberheart_safe_found")
end

return FuchiOffices
