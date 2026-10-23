-- Scene 3/event 0's exterior doorman path. Lua owns the dialogue branches
-- through the Mako introduction; its actor activity remains a source boundary.
local DialogueGraph = require("dialogue_graph")

local ClubPenumbra = {}

function ClubPenumbra.present_doorman()
    return DialogueGraph.run("club_penumbra.scene3_doorman_entry")
end

return ClubPenumbra
