-- Scene 3/event 0's exterior doorman path and delayed Mako actor-list result.
local DialogueGraph = require("dialogue_graph")
local E = require("enums")
local StoryState = require("story_state")

local ClubPenumbra = {}

function ClubPenumbra.present_doorman()
    return DialogueGraph.run("club_penumbra.scene3_doorman_entry")
end

-- Mako result (ROM callback): +2 heat (cap 8), +2 Karma, story/mode/tip
-- flag changes, then the shared Strike Team step (ROM 0x1F1E0).
function ClubPenumbra.apply_mako_callback()
    local heat = StoryState.byte(E.StoryByte.HEAT)
    if heat == nil or StoryState.byte(E.StoryByte.STRIKE_TEAM) == nil or
       game.state.notebook_tip_flag_bit_49 == nil or
       game.state.field_mode_flag_bit_20 == nil then
        return "service_unavailable"
    end
    -- Karma first: it is the only step that can still fail (no roster), so
    -- nothing has changed when it does.
    if game.services.call("party", "award_karma_2") ~= "applied" or
       not StoryState.set_byte(E.StoryByte.HEAT, math.min(8, heat + 2)) then
        return "service_unavailable"
    end

    game.state.story_flag_bit_11 = true
    game.state.field_mode_flag_bit_20 = false
    game.state.field_mode_flag_bit_21 = false
    game.state.field_mode_flag_bit_24 = true
    game.state.notebook_tip_flag_bit_49 = false

    if StoryState.advance_strike_team() ~= "applied" then
        return "service_unavailable"
    end
    return "applied"
end

function ClubPenumbra.present_mako_activity_result()
    return DialogueGraph.run("club_penumbra.mako_activity_result")
end

return ClubPenumbra
