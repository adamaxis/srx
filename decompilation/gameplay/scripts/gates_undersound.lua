-- ROM title service 15 (0x54222), scene 0/event 3.
-- The reusable JSON graph owns gating, page order, effects, and its outcome.
local DialogueGraph = require("dialogue_graph")

local GatesUndersound = {}

function GatesUndersound.present_ogami_departure()
    return DialogueGraph.run("gates_undersound.ogami_departure")
end

return GatesUndersound
