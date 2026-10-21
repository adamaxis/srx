-- Stoker's copy, choices, gates, and successful outcomes are data in the
-- versioned conversation catalog. Lua registers the small named effects that
-- bridge its source transactions to typed host services.
local DialogueGraph = require("dialogue_graph")
local MichaelPackage = {}

local function registered_actions()
    return {
        ["stoker_motel.prepare_debt_offer"] = function()
            local result = game.services.call(
                "michael_package", "prepare_debt_offer")
            return result == "ready" and "ready" or "service_unavailable"
        end,
        ["stoker_motel.pay_debt"] = function()
            local result = game.services.call("michael_package", "pay_debt")
            if result == "paid" or result == "insufficient" then
                return result
            end
            return "service_unavailable"
        end,
        ["stoker_motel.prepare_referral"] = function()
            local result = game.services.call(
                "michael_package", "jump_house_referral")
            return result == "ready" and "ready" or "service_unavailable"
        end,
        ["stoker_motel.resolve_arrival"] = function()
            local result = game.services.call(
                "stoker_motel", "resolve_arrival")
            if result == "manager" or result == "street_ambush" then
                return result
            end
            return "service_unavailable"
        end,
    }
end

function MichaelPackage.stoker_interaction()
    return DialogueGraph.run(
        "stoker_motel.indexed_service", registered_actions())
end

function MichaelPackage.present_field_exit_prompt()
    return DialogueGraph.run(
        "stoker_motel.field_exit_prompt", registered_actions())
end

return MichaelPackage
