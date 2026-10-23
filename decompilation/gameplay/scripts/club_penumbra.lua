-- Scene 3/event 0's exterior doorman path. Lua owns the dialogue branches;
-- the graph stops at the still-unmodeled pending Mako activity handoff.
local DialogueGraph = require("dialogue_graph")

local ClubPenumbra = {}

function ClubPenumbra.present_doorman()
    return DialogueGraph.run("club_penumbra.scene3_doorman_entry")
end

return ClubPenumbra
