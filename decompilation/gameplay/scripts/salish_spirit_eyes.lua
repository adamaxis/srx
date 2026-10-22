-- ROM 0x53D5A handles Scene-4 event 27 (Spirit Eyes). The independent
-- Scene-47 event-25 continuation at 0x53EFA handles Licourtrix's lair.
-- The JSON graph owns its ordered story gates and prompts; registered Lua
-- actions apply the source-backed trigger and item/story state changes.
local DialogueGraph = require("dialogue_graph")

local SpiritEyes = {}

function SpiritEyes.present()
    return DialogueGraph.run("salish.spirit_eyes.ritual")
end

function SpiritEyes.present_serpent_encounter()
    return DialogueGraph.run("salish.licourtrix.encounter")
end

function SpiritEyes.present_renraku_ambush()
    return DialogueGraph.run("salish.renraku.cave_ambush")
end

return SpiritEyes
