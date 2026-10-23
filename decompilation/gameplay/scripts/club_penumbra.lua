-- Scene 3/event 0's exterior doorman path. Lua owns the dialogue branches
-- through the Mako introduction; its delayed actor outcome remains a boundary.
local DialogueGraph = require("dialogue_graph")

local ClubPenumbra = {}

function ClubPenumbra.present_doorman()
    return DialogueGraph.run("club_penumbra.scene3_doorman_entry")
end

return ClubPenumbra
