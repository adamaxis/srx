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

function Owlfeather.present_mitsuhama_entry()
    return DialogueGraph.run("salish.owlfeather.mitsuhama_entry")
end

function Owlfeather.present_mitsuhama_contact()
    return DialogueGraph.run("salish.owlfeather.mitsuhama_contact")
end

return Owlfeather
