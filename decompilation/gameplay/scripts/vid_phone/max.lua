-- Max's source-defined call tree lives in JSON. This module registers only
-- his 0x83-0x87 typed host transactions and the shared call-history effect.
local Max = {}

local function invoke(operation, accepted_results)
    local result = game.services.call("vid_phone", operation)
    for _, accepted in ipairs(accepted_results) do
        if result == accepted then return result end
    end
    return "service_unavailable"
end

function Max.actions(contact)
    assert(type(contact) == "table" and contact.id == "max" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "call",
        "Max graph needs a validated call-history binding")

    return {
        ["vid_phone.mark_called"] = function()
            if game.state[contact.call_history_fact] ~= false then
                return "call_history_unavailable"
            end
            game.state[contact.call_history_fact] = true
            return true
        end,
        ["vid_phone.issue_max_corporate_badge"] = function()
            return invoke("issue_max_corporate_badge", {
                "badge_issued", "insufficient", "unavailable",
            })
        end,
        ["vid_phone.purchase_max_hk227_s"] = function()
            return invoke("purchase_max_hk227_s", {
                "purchased", "insufficient", "out_of_stock", "unavailable",
            })
        end,
        ["vid_phone.purchase_ak97"] = function()
            return invoke("purchase_ak97", {
                "purchased", "insufficient", "out_of_stock", "unavailable",
            })
        end,
        ["vid_phone.purchase_max_maglock_passkey_level_4"] = function()
            return invoke("purchase_max_maglock_passkey_level_4", {
                "purchased", "insufficient", "out_of_stock", "unavailable",
            })
        end,
        ["vid_phone.purchase_max_maglock_passkey_level_5"] = function()
            return invoke("purchase_max_maglock_passkey_level_5", {
                "purchased", "insufficient", "out_of_stock", "unavailable",
            })
        end,
    }
end

return Max
