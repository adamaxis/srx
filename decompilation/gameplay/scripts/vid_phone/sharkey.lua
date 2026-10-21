-- Sharkey-specific host effects for his JSON-owned service graph.
-- The ROM contact record at 0x10A7F2 selects commands 0x6E-0x70. Dialogue
-- copy, choices, transaction branches, and outcomes live in JSON.
local Sharkey = {}
local Passages = require("data.vid_phone_passages")

local function invoke(operation, expected_results)
    local result = game.services.call("vid_phone", operation)
    for _, expected in ipairs(expected_results) do
        if result == expected then return result end
    end
    return "service_unavailable"
end

local function passage_action(passage_id)
    local operation = Passages.service_action(passage_id)
    if operation == nil then return "service_unavailable" end
    return invoke(operation, { "passage_requested", "insufficient",
                               "unavailable" })
end

function Sharkey.actions(contact)
    assert(type(contact) == "table" and contact.id == "sharkey" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Sharkey graph needs a validated contact service binding")

    return {
        ["vid_phone.purchase_salish_passage"] = function()
            return passage_action("salish_shidhe")
        end,
        ["vid_phone.purchase_sinsearach_passage"] = function()
            return passage_action("sinsearach")
        end,
        ["vid_phone.purchase_smart_goggles"] = function()
            return invoke("purchase_smart_goggles", {
                "purchased", "insufficient", "out_of_stock", "unavailable",
            })
        end,
    }
end

return Sharkey
