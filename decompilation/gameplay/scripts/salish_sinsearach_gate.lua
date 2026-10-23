-- Scene 9, mode 9, event 0: the gate dialogue and the guarded path to
-- Lady Gillian's Frosty referral. ROM roots and state gates stay in the
-- JSON conversation graph; this module only opens that graph.
local DialogueGraph = require("dialogue_graph")

local SinsearachGate = {}

function SinsearachGate.present()
    return DialogueGraph.run("salish.sinsearach.gate")
end

return SinsearachGate
