-- Scene-55's completed-fight callback presents the ordered end credits.
-- The ROM trigger is still a source boundary; this Lua entry point owns the
-- credit sequence once FieldSceneRuntime reaches that callback.
local DialogueGraph = require("dialogue_graph")

local TarislarTombCredits = {}

function TarislarTombCredits.present()
    return DialogueGraph.run("tarislar.tomb.credits")
end

return TarislarTombCredits
