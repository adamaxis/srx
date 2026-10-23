-- Scene 3/event 0's exterior doorman path and delayed Mako actor-list result.
local DialogueGraph = require("dialogue_graph")

local ClubPenumbra = {}

function ClubPenumbra.present_doorman()
    return DialogueGraph.run("club_penumbra.scene3_doorman_entry")
end

function ClubPenumbra.apply_mako_callback()
    if game.services.call("field", "mako_result_state_available") ~=
       "available" then
        return "service_unavailable"
    end
    if game.services.call("field", "add_heat_2_capped_8") ~= "applied" then
        return "service_unavailable"
    end
    if game.services.call("party", "award_karma_2") ~= "applied" then
        return "service_unavailable"
    end

    game.state.story_flag_bit_11 = true
    game.state.field_mode_flag_bit_20 = false
    game.state.field_mode_flag_bit_21 = false
    game.state.field_mode_flag_bit_24 = true
    game.state.notebook_tip_flag_bit_49 = false

    if game.services.call("field", "advance_strike_team_counter") ~=
       "applied" then
        return "service_unavailable"
    end
    return "applied"
end

function ClubPenumbra.present_mako_activity_result()
    return DialogueGraph.run("club_penumbra.mako_activity_result")
end

return ClubPenumbra
