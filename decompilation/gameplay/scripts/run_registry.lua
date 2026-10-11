-- ROM 0x1E69C selects one of eight six-byte F3EBC records. The context-zero
-- handler sequence below is expressed as Lua run IDs; the registry only
-- selects a run and dispatches to its separate implementation.
local Registry = {}

local new_game_selection = {
    "ghoul_bounty",
    "bodyguard_escort",
    "package_delivery",
    "bodyguard_escort",
    "package_delivery",
    "bodyguard_escort",
    "package_delivery",
    "ghoul_bounty",
}

function Registry.available_run_ids()
    local ids = {}
    for id, run in pairs(game.runs) do
        if type(run) == "table" and
           type(run.prepare_offer) == "function" and
           type(run.present_offer) == "function" then
            ids[#ids + 1] = id
        end
    end
    table.sort(ids)
    return ids
end

local function extension_offer_slots()
    local slots = {}
    for _, id in ipairs(Registry.available_run_ids()) do
        local weight = game.runs[id].new_game_weight or 0
        assert(type(weight) == "number" and weight >= 0 and
               weight % 1 == 0,
            "Lua run new_game_weight must be a nonnegative integer: " .. id)
        for _ = 1, weight do
            slots[#slots + 1] = id
        end
    end
    return slots
end

function Registry.prepare_new_game_offer()
    -- This entry point is the first-contract pool for a newly created game.
    -- Keep the ROM's eight source slots first and in order. New Lua modules
    -- can opt into an appended extension pool with new_game_weight, without
    -- changing native registration or the source-derived base selection.
    local context = 0
    local extension_slots = extension_offer_slots()
    local slot = game.modules.run_support.bounded_random(
        #new_game_selection + #extension_slots)
    local run_id
    if slot < #new_game_selection then
        run_id = new_game_selection[slot + 1]
    else
        run_id = extension_slots[slot - #new_game_selection + 1]
    end
    local run = game.runs[run_id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "weighted run slot has no Lua implementation: " .. run_id)
    Registry.current_offer = nil
    return run.prepare_offer(context)
end

function Registry.present_new_game_offer()
    local run_id = Registry.prepare_new_game_offer()
    return game.runs[run_id].present_offer()
end

function Registry.current_offer_id()
    return Registry.current_offer and Registry.current_offer.id or "none"
end

function Registry.current_completion_condition()
    return Registry.current_offer and
        Registry.current_offer.completion_condition or "none"
end

function Registry.current_route_ids()
    local route = Registry.current_offer and Registry.current_offer.route
    if route == nil then
        return {
            origin_location = "none",
            origin_area = "none",
            client = "none",
            destination_location = "none",
            destination_area = "none",
        }
    end
    return {
        origin_location = "location_" .. route.origin.location_id,
        origin_area = "area_" .. route.origin.area_id,
        client = "client_" .. route.client_id,
        destination_location =
            "location_" .. route.destination.location_id,
        destination_area = "area_" .. route.destination.area_id,
    }
end

return Registry
