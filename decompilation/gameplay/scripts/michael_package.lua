-- Stoker's copy, choices, gates, and successful outcomes are data in the
-- versioned conversation catalog. Lua registers the small named effects that
-- bridge its source transactions to typed host services.
local DialogueGraph = require("dialogue_graph")
local Encounters = require("field_encounters")
local E = require("enums")
local MichaelPackage = {}

local function arrival_roll()
    local result = game.services.call(
        "random", "bounded_" .. E.StokerMotelArrival.RANDOM_BOUND)
    if type(result) ~= "string" then
        return nil
    end
    local digits = result:match("^roll_(%d+)$")
    local value = digits and tonumber(digits) or nil
    if value == nil or value < 0 or
       value >= E.StokerMotelArrival.RANDOM_BOUND then
        return nil
    end
    return value
end

local function resolve_arrival()
    local roll = arrival_roll()
    if roll == nil then
        return "service_unavailable"
    end
    if roll < E.StokerMotelArrival.AMBUSH_MINIMUM_ROLL then
        return "manager"
    end

    local sequence_id = E.WorldEffectSequence.STOKER_MOTEL_AMBUSH
    local sequence = Encounters.resolve_sequence(
        E.Scene.STOKER_MOTEL, sequence_id)
    if sequence == nil or
       sequence.kind ~= "dynamic_world_effect_sequence" or
       sequence.scene_id ~= E.Scene.STOKER_MOTEL or
       sequence.sequence_id ~= sequence_id or
       sequence.placement_policy ~=
           E.WorldEffectPlacementPolicy.MAP_DERIVED_CANDIDATES or
       #sequence.actors ~= E.StokerMotelArrival.AMBUSH_ACTOR_COUNT then
        return "service_unavailable"
    end
    for index, profile_id in ipairs(
            E.StokerMotelArrival.AMBUSH_PROFILE_IDS) do
        local actor = sequence.actors[index]
        local expected_action_kind = index == 1 and "item" or "none"
        if actor == nil or actor.sequence_order ~= index - 1 or
           actor.sequence_id ~= sequence_id or
           actor.profile_id ~= profile_id or
           actor.action_kind ~= expected_action_kind then
            return "service_unavailable"
        end
    end

    local result = game.services.call(
        "stoker_motel", "spawn_sequence_" .. sequence_id)
    if result == "spawned" then
        return "street_ambush"
    end
    if result == "no_effect" then
        return "manager"
    end
    return "service_unavailable"
end

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
            return resolve_arrival()
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
