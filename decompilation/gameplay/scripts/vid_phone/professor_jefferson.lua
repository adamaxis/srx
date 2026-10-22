-- Professor Jefferson's lesson and paid offers live in the JSON graph. Lua
-- names the two host transactions and owns the contact's call-history effect.
local ProfessorJefferson = {}

local function invoke(operation, accepted_results)
    local result = game.services.call("vid_phone", operation)
    for _, accepted in ipairs(accepted_results) do
        if result == accepted then return result end
    end
    return "service_unavailable"
end

function ProfessorJefferson.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "professor_jefferson" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "call",
        "Professor Jefferson graph needs a validated call-history binding")

    return {
        ["vid_phone.mark_called"] = function()
            if game.state[contact.call_history_fact] ~= false then
                return "call_history_unavailable"
            end
            game.state[contact.call_history_fact] = true
            return true
        end,
        ["vid_phone.purchase_riannons_discount"] = function()
            return invoke("purchase_riannons_discount", {
                "discount_enabled", "insufficient", "unavailable",
            })
        end,
        ["vid_phone.hire_gregory_wilns"] = function()
            return invoke("hire_gregory_wilns", {
                "hired", "insufficient", "unavailable",
            })
        end,
    }
end

return ProfessorJefferson
