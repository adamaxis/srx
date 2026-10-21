-- ROM title service 12 (0x54076), scene 5/event 6.
-- The entry page resumes the already selected activity-35 event route.
local DialogueGraph = require("dialogue_graph")

local RenrakuOffices = {}

function RenrakuOffices.present_map_overlay_entry()
    return DialogueGraph.run("renraku_offices.map_overlay_entry")
end

return RenrakuOffices
