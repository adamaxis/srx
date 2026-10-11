-- ROM 0x1E69C selects one of eight six-byte F3EBC records. The context-zero
-- handler sequence below is expressed as Lua run IDs; the registry only
-- selects a run and dispatches to its separate implementation.
local Registry = {}

local new_game_selection = {
    -- ROM 0x1E69C source rows in F3EBC, context zero. The handler selects the
    -- Lua run module; payment and negotiation values are typed content data.
    { run_id = "ghoul_bounty", base_payment = 10, negotiation_scale = 3 },
    { run_id = "bodyguard_escort", base_payment = 45, negotiation_scale = 1 },
    { run_id = "package_delivery", base_payment = 35, negotiation_scale = 1 },
    { run_id = "bodyguard_escort", base_payment = 50, negotiation_scale = 1 },
    { run_id = "package_delivery", base_payment = 40, negotiation_scale = 1 },
    { run_id = "bodyguard_escort", base_payment = 55, negotiation_scale = 1 },
    { run_id = "package_delivery", base_payment = 45, negotiation_scale = 1 },
    { run_id = "ghoul_bounty", base_payment = 20, negotiation_scale = 2 },
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
        if weight > 0 then
            local payment = game.runs[id].new_game_payment
            assert(type(payment) == "table" and
                   type(payment.base_payment) == "number" and
                   type(payment.negotiation_scale) == "number",
                "weighted Lua runs must declare payment terms: " .. id)
        end
        for _ = 1, weight do
            slots[#slots + 1] = {
                run_id = id,
                payment = game.runs[id].new_game_payment,
            }
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
    local payment_record
    if slot < #new_game_selection then
        payment_record = new_game_selection[slot + 1]
        run_id = payment_record.run_id
    else
        local extension = extension_slots[slot - #new_game_selection + 1]
        run_id = extension.run_id
        payment_record = extension.payment
    end
    local run = game.runs[run_id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "weighted run slot has no Lua implementation: " .. run_id)
    Registry.current_offer = nil
    Registry._last_offer_result = nil
    return run.prepare_offer(context, payment_record)
end

function Registry.present_new_game_offer()
    local run_id = Registry.prepare_new_game_offer()
    return game.runs[run_id].present_offer()
end

function Registry.resolve_current_offer(choice)
    assert(choice == "yes" or choice == "no",
        "run decision must be yes or no")
    assert(Registry.current_offer ~= nil,
        "run decision has no pending offer")
    if choice == "yes" then
        Registry.current_offer.accepted = true
        Registry.active_offer = Registry.current_offer
        Registry._last_offer_result = "accepted"
    else
        Registry.current_offer.accepted = false
        Registry._last_offer_result = "declined"
    end
    Registry.current_offer = nil
    return Registry._last_offer_result
end

function Registry.active_run_id()
    return Registry.active_offer and Registry.active_offer.id or "none"
end

function Registry.active_completion_condition()
    return Registry.active_offer and
        Registry.active_offer.completion_condition or "none"
end

function Registry.last_offer_result()
    return Registry._last_offer_result or "none"
end

function Registry.current_offer_id()
    return Registry.current_offer and Registry.current_offer.id or "none"
end

function Registry.current_completion_condition()
    return Registry.current_offer and
        Registry.current_offer.completion_condition or "none"
end

local function route_ids(offer)
    local route = offer and offer.route
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

function Registry.current_route_ids()
    return route_ids(Registry.current_offer)
end

function Registry.active_route_ids()
    return route_ids(Registry.active_offer)
end

function Registry.current_payment_values()
    local offer = Registry.current_offer or Registry.active_offer
    local payment = offer and offer.payment
    if payment == nil then
        return {
            base_amount = "none",
            negotiated_amount = "none",
            negotiation_bound = "none",
            negotiation_roll = "none",
            negotiation_skill = "none",
        }
    end
    return {
        base_amount = tostring(payment.base_amount),
        negotiated_amount = tostring(payment.negotiated_amount),
        negotiation_bound = tostring(payment.negotiation_bound),
        negotiation_roll = tostring(payment.negotiation_roll),
        negotiation_skill = tostring(payment.negotiation_skill),
    }
end

return Registry
