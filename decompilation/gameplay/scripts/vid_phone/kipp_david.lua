-- Kipp-specific host effects for his JSON-owned service graph.
-- Contact record 0x10AD8A maps commands 0x71-0x73 to handlers
-- 0x22A54, 0x22A9E, and 0x22AF4.
local KippDavid = {}

local function invoke(operation, expected_results)
    local result = game.services.call("vid_phone", operation)
    for _, expected in ipairs(expected_results) do
        if result == expected then return result end
    end
    return "service_unavailable"
end

function KippDavid.actions(contact)
    assert(type(contact) == "table" and contact.id == "kipp_david" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Kipp graph needs a validated contact service binding")

    return {
        ["vid_phone.purchase_excalibur"] = function()
            return invoke("purchase_excalibur", {
                "deck_installed", "insufficient", "unavailable",
            })
        end,
        ["vid_phone.purchase_rebound"] = function()
            return invoke("purchase_rebound", {
                "program_installed", "insufficient", "storage_full",
                "unavailable",
            })
        end,
        ["vid_phone.purchase_degrade"] = function()
            return invoke("purchase_degrade", {
                "program_installed", "insufficient", "storage_full",
                "unavailable",
            })
        end,
    }
end

return KippDavid
