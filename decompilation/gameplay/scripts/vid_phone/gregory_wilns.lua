-- Gregory-specific host effects for his JSON-owned Vid-Phone service graph.
-- ROM record 0x10C7FA branches to this tree at 0x10C838. Commands 0x77 and
-- 0x78 dispatch to 0x22C22 (Merlin's Lore discount) and 0x22C46 (Power Focus).
local GregoryWilns = {}

local function invoke(operation, accepted_results)
    local result = game.services.call("vid_phone", operation)
    for _, accepted in ipairs(accepted_results) do
        if result == accepted then return result end
    end
    return "service_unavailable"
end

function GregoryWilns.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "gregory_wilns" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Gregory graph needs a validated contact service binding")

    return {
        ["vid_phone.purchase_merlins_lore_discount"] = function()
            return invoke("purchase_merlins_lore_discount", {
                "discount_enabled", "insufficient", "unavailable",
            })
        end,
        ["vid_phone.purchase_power_focus"] = function()
            return invoke("purchase_power_focus", {
                "purchased", "insufficient", "ineligible", "out_of_stock",
                "unavailable",
            })
        end,
    }
end

return GregoryWilns
