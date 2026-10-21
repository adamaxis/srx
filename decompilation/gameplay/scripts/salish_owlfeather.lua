-- ROM title services 2-4 in scene 6/event 0 share one JSON-owned dialogue
-- graph. These named adapters preserve the location-link entry points while
-- keeping entry identity explicit in the graph's typed invocation context.
local DialogueGraph = require("dialogue_graph")

local Owlfeather = {}
local GRAPH_ID = "salish.owlfeather.lodge_services"

local function present(entry_id)
    return DialogueGraph.run(GRAPH_ID, nil, {
        dialogue = { entry_id = entry_id },
    })
end

function Owlfeather.present_first_contact()
    return present("first_contact")
end

function Owlfeather.present_search_followup()
    return present("search_followup")
end

function Owlfeather.present_return_followup()
    return present("return_followup")
end

return Owlfeather
