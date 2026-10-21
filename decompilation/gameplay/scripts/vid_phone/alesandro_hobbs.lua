-- Alesandro-specific host effects for his JSON-owned Vid-Phone service graph.
-- ROM commands 0x6B/0x6C buy level-3/level-5 passkeys (0x22934/0x2296E);
-- 0x6D charges 8,000 and sets contact bit 5 (0x229A8).
local AlesandroHobbs = {}

local function service_action(operation)
    local result = game.services.call("vid_phone", operation)
    if result == "purchased" or result == "hired" or
       result == "insufficient" or result == "out_of_stock" or
       result == "unavailable" then
        return result
    end
    return "service_unavailable"
end

function AlesandroHobbs.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "alesandro_hobbs" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Alesandro graph needs a validated contact service binding")

    return {
        ["vid_phone.purchase_maglock_passkey_level_3"] = function()
            return service_action("purchase_maglock_passkey_level_3")
        end,
        ["vid_phone.purchase_maglock_passkey_level_5"] = function()
            return service_action("purchase_maglock_passkey_level_5")
        end,
        ["vid_phone.hire_macklemann"] = function()
            return service_action("hire_macklemann")
        end,
    }
end

return AlesandroHobbs
