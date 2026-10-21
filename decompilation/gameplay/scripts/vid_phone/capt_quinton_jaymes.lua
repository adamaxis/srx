-- Quinton-specific host effects for his JSON-owned call graph.
-- ROM branch structure, copy, ordered choices, and terminals live in
-- dialogue/conversations/vid_phone_services.json.
local CaptQuintonJaymes = {}

local function service_action(operation)
    local result = game.services.call("vid_phone", operation)
    if result == "purchased" or result == "hired" or
       result == "insufficient" or result == "out_of_stock" then
        return result
    end
    return "service_unavailable"
end

function CaptQuintonJaymes.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "capt_quinton_jaymes" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call",
        "Quinton graph needs a validated contact history binding")

    return {
        ["vid_phone.mark_called"] = function()
            if game.state[contact.call_history_fact] ~= false then
                return "call_history_unavailable"
            end
            game.state[contact.call_history_fact] = true
            return true
        end,
        ["vid_phone.purchase_frag_grenades"] = function()
            return service_action("purchase_frag_grenades")
        end,
        ["vid_phone.purchase_hk227_s"] = function()
            return service_action("purchase_hk227_s")
        end,
        ["vid_phone.hire_sharkey"] = function()
            return service_action("hire_sharkey")
        end,
    }
end

return CaptQuintonJaymes
