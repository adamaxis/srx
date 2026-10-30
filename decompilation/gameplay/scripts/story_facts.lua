-- Semantic story progression facts.
--
-- Lua stores named facts such as `motel_bill_paid` and `ilene_met`. Their
-- persistence representation is owned by the host adapter. This module keeps
-- gameplay rules independent of save slots, bit positions, and ROM dialogue
-- command encodings.
local E = require("enums")

local StoryFacts = {}
local known = {}

for _, fact_id in pairs(E.StoryFact) do
    known[fact_id] = true
end

function StoryFacts.is_known(fact_id)
    return type(fact_id) == "string" and known[fact_id] == true
end

local function checked_fact(fact_id)
    assert(StoryFacts.is_known(fact_id),
        "unknown story fact: " .. tostring(fact_id))
    return fact_id
end

-- Returns true/false, or nil when the host has not restored the fact yet.
function StoryFacts.is_set(fact_id)
    return game.state[checked_fact(fact_id)]
end

function StoryFacts.set(fact_id, value)
    if value == nil then value = true end
    assert(type(value) == "boolean",
        "story fact value must be boolean")
    game.state[checked_fact(fact_id)] = value
    return true
end

return StoryFacts
