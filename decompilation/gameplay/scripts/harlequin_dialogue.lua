-- Harlequin's Lua conversation is shared by the Tarislar Apartments and the
-- Red Buffalo no-gift continuation. The main and map-information paths use
-- shared JSON graphs; the active location supplies their presentation.
local DialogueGraph = require("dialogue_graph")

local HarlequinDialogue = {}

local function presentation_context()
    local location = game.location
    assert(type(location.speaker_id) == "string" and
           type(location.text_profile) == "string",
        "Harlequin dialogue needs its location presentation context")
    return {
        presentation = {
            speakerId = location.speaker_id,
            textProfile = location.text_profile,
        },
    }
end

local function return_to_field_result(outcome)
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    end
    return outcome
end

function HarlequinDialogue.present_map_information()
    -- The same ROM conversation serves both Scene 1 and Scene 6. The active
    -- location supplies its speaker and menu profile to the shared graph.
    return return_to_field_result(DialogueGraph.run(
        "harlequin.map_information", nil, presentation_context()))
end

function HarlequinDialogue.present_main_conversation()
    local outcome = DialogueGraph.run(
        "harlequin.main_conversation", nil, presentation_context())
    if type(outcome) == "table" and
       outcome.type == "return_to_caller" then
        -- The Ito response continues into the separately reusable map graph.
        return HarlequinDialogue.present_map_information()
    end
    return return_to_field_result(outcome)
end

return HarlequinDialogue
