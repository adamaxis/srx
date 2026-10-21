-- Wilma-specific host effects for her JSON-owned Vid-Phone service graph.
-- Contact record 0x10B71E maps commands 0x74/0x75 to handlers
-- 0x22B4A (Rebound) and 0x22BA0 (Degrade): 5,000 nuyen, size
-- checked as rating 3, stored as rating 3.
local WilmaTemmenhoff = {}

local function invoke(operation)
    local result = game.services.call("vid_phone", operation)
    if result == "program_installed" or result == "insufficient" or
       result == "storage_full" or result == "unavailable" then
        return result
    end
    return "service_unavailable"
end

function WilmaTemmenhoff.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "wilma_temmenhoff" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Wilma graph needs a validated contact service binding")

    return {
        ["vid_phone.purchase_wilma_rebound"] = function()
            return invoke("purchase_wilma_rebound")
        end,
        ["vid_phone.purchase_wilma_degrade"] = function()
            return invoke("purchase_wilma_degrade")
        end,
    }
end

return WilmaTemmenhoff
