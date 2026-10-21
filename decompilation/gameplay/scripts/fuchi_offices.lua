-- ROM title service 14 (0x54152) loads the source entry page, then leaves
-- D6 at zero so the selected event-table route continues into activity 21.
local DialogueGraph = require("dialogue_graph")

local FuchiOffices = {}

function FuchiOffices.present_cyberheart_entry()
    return DialogueGraph.run("fuchi_offices.cyberheart_entry")
end

return FuchiOffices
