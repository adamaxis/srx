-- Scene-55's completed-fight callback presents the ordered end credits.
-- The warning callback runs the ROM pre-credit timeline before opening these
-- credits; the live fight trigger remains owned by FieldSceneRuntime.
local DialogueGraph = require("dialogue_graph")

local TarislarTombCredits = {}

function TarislarTombCredits.present()
    return DialogueGraph.run("tarislar.tomb.credits")
end

function TarislarTombCredits.present_ending_warning()
    return DialogueGraph.run("tarislar.tomb.ending_warning", {
        ["tarislar_tomb.start_ending_presentation"] = function()
            assert(type(game) == "table" and
                   type(game.services) == "table" and
                   type(game.services.call) == "function",
                "ending presentation needs the field host service")
            assert(game.services.call(
                "field", "start_tarislar_tomb_ending_presentation") ==
                    "applied",
                "field host could not start the ROM ending presentation")
            return true
        end,
    })
end

return TarislarTombCredits
