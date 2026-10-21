-- ROM 0x1F77C selects Wylie's signed run-result root or the 50-nuyen room
-- service. Dialogue copy, gates, choices, payment edges, and outcomes live in
-- the versioned JSON graph; this module is the location's graph entry point.
local DialogueGraph = require("dialogue_graph")

local Inn = {}

function Inn.present_field_exit_prompt()
    return DialogueGraph.run("wylie_gala_inn.room_service")
end

return Inn
