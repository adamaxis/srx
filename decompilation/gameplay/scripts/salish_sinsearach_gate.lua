-- Scene 9, mode 9, event 0: the gate dialogue and the guarded path to
-- Lady Gillian's Frosty referral. ROM roots and state gates stay in the
-- JSON conversation graph; this module only opens that graph.
local DialogueGraph = require("dialogue_graph")

local SinsearachGate = {}

local function charge_copt_fare()
    local funds = game.nuyen()
    if type(funds) ~= "number" then return "service_unavailable" end
    if funds < 1000 then return "insufficient" end
    game.set_nuyen(funds - 1000)
    return "paid"
end

function SinsearachGate.present()
    return DialogueGraph.run("salish.sinsearach.gate", {
        ["field.charge_copt_fare"] = charge_copt_fare,
    })
end

return SinsearachGate
