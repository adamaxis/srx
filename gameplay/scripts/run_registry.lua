-- ROM 0x1E69C selects one of eight six-byte F3EBC records. The context-zero
-- handler sequence below is expressed as Lua run IDs; the registry only
-- selects a run and dispatches to its separate implementation.
local Registry = {}
local E = require("enums")

local function valid_matrix_run_state(state)
    if type(state) ~= "table" then return false end
    local function signed_byte(value)
        return type(value) == "number" and value % 1 == 0 and
               value >= -128 and value <= 127
    end
    local function byte(value)
        return type(value) == "number" and value % 1 == 0 and
               value >= 0 and value <= 0xFF
    end
    if not signed_byte(state.state) or
       not signed_byte(state.objective) or
       not signed_byte(state.value) or not byte(state.size) or
       not byte(state.fc28) or not byte(state.fc1c) then
        return false
    end
    return state.target_system_id == nil or
        (type(state.target_system_id) == "number" and
         state.target_system_id % 1 == 0 and
         state.target_system_id >= 0 and state.target_system_id <= 24)
end

local function copy_matrix_run_state(state)
    if state == nil then return nil end
    return {
        state = state.state,
        objective = state.objective,
        value = state.value,
        size = state.size,
        fc28 = state.fc28,
        fc1c = state.fc1c,
        target_system_id = state.target_system_id,
    }
end

local function matrix_run_state_for(offer)
    if type(offer) ~= "table" or
       offer.source_run_type ~= E.RunType.MATRIX_OBJECTIVE or
       not valid_matrix_run_state(offer.matrix_run_state) then
        return nil
    end
    return copy_matrix_run_state(offer.matrix_run_state)
end

local function set_matrix_run_state(offer, state)
    if type(offer) ~= "table" or
       offer.source_run_type ~= E.RunType.MATRIX_OBJECTIVE or
       not valid_matrix_run_state(state) then
        return false
    end
    offer.matrix_run_state = copy_matrix_run_state(state)
    return true
end

function Registry.current_matrix_run_state()
    return matrix_run_state_for(Registry.current_offer)
end

function Registry.active_matrix_run_state()
    return matrix_run_state_for(Registry.active_offer)
end

function Registry.commit_current_matrix_run_state(state)
    return set_matrix_run_state(Registry.current_offer, state)
end

function Registry.commit_active_matrix_run_state(state)
    return set_matrix_run_state(Registry.active_offer, state)
end

local new_game_selection = {
    -- ROM 0x1E69C source rows in F3EBC, context zero. The handler selects the
    -- Lua run module; payment and negotiation values are typed content data.
    {
        run_id = "ghoul_bounty", base_payment = 10, negotiation_scale = 3,
        runner_progress_delta = 1,
    },
    {
        run_id = "bodyguard_escort", base_payment = 45,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "package_delivery", base_payment = 35,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "bodyguard_escort", base_payment = 50,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "package_delivery", base_payment = 40,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "bodyguard_escort", base_payment = 55,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "package_delivery", base_payment = 45,
        negotiation_scale = 1, runner_progress_delta = 1,
    },
    {
        run_id = "ghoul_bounty", base_payment = 20, negotiation_scale = 2,
        runner_progress_delta = 1,
    },
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

function Registry.resolve_field_event(map_id, event_id)
    assert(type(map_id) == "number" and map_id >= 0 and
           map_id <= 255 and map_id % 1 == 0)
    assert(type(event_id) == "number" and event_id >= 0 and
           event_id <= 255 and event_id % 1 == 0)

    local offer = Registry.active_offer
    if offer == nil then
        return {
            action = "no_action",
            run_id = "none",
            reward_nuyen = 0,
            runner_progress_delta = 0,
        }
    end

    local run = game.runs[offer.id]
    local handler = type(run) == "table" and run.resolve_field_event or nil
    if type(handler) ~= "function" then
        return {
            action = "no_action",
            run_id = offer.id,
            reward_nuyen = 0,
            runner_progress_delta = 0,
        }
    end

    local result = handler(offer, map_id, event_id)
    assert(type(result) == "table" and
           (result.action == "no_action" or
            result.action == "objective_advanced" or
            result.action == "completed"),
        "run field-event handler returned an invalid action")

    local decision = {
        action = result.action,
        run_id = offer.id,
        reward_nuyen = result.reward_nuyen or 0,
        runner_progress_delta = offer.runner_progress_delta or 0,
    }
    assert(type(decision.reward_nuyen) == "number" and
           decision.reward_nuyen >= 0 and decision.reward_nuyen % 1 == 0 and
           decision.reward_nuyen <= 0xFFFFFFFF,
        "run field-event handler returned an invalid reward")
    assert(type(decision.runner_progress_delta) == "number" and
           decision.runner_progress_delta >= 0 and
           decision.runner_progress_delta <= 255 and
           decision.runner_progress_delta % 1 == 0,
        "run offer has an invalid runner progress delta")

    if decision.action == "completed" then
        Registry.last_completed_offer = offer
        Registry.active_offer = nil
    end
    return decision
end

function Registry.active_run_id()
    return Registry.active_offer and Registry.active_offer.id or "none"
end

function Registry.active_completion_condition()
    return Registry.active_offer and
        Registry.active_offer.completion_condition or "none"
end

function Registry.last_completed_offer_id()
    return Registry.last_completed_offer and
        Registry.last_completed_offer.id or "none"
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
            runner_progress_delta = "none",
        }
    end
    return {
        base_amount = tostring(payment.base_amount),
        negotiated_amount = tostring(payment.negotiated_amount),
        negotiation_bound = tostring(payment.negotiation_bound),
        negotiation_roll = tostring(payment.negotiation_roll),
        negotiation_skill = tostring(payment.negotiation_skill),
        runner_progress_delta = tostring(offer.runner_progress_delta or 0),
    }
end

-- The host save file stores durable run state as ordinary JSON-compatible
-- data. The game script remains responsible for deciding which parts of its
-- live registry make up that state; native code only transports the value.
function Registry.save_state()
    return {
        schema_version = 1,
        current_offer = Registry.current_offer,
        active_offer = Registry.active_offer,
        last_completed_offer = Registry.last_completed_offer,
        last_offer_result = Registry._last_offer_result,
    }
end

function Registry.validate_saved_state(state)
    if type(state) ~= "table" or state.schema_version ~= 1 then
        return false, "The save has an unsupported run-state version."
    end

    local function valid_offer(offer)
        if offer == nil then return true end
        if type(offer) ~= "table" or type(offer.id) ~= "string" or
           type(offer.category) ~= "string" or
           type(offer.objective) ~= "string" or
           type(offer.completion_condition) ~= "string" or
           type(offer.runner_progress_delta) ~= "number" or
           offer.runner_progress_delta < 0 or
           offer.runner_progress_delta > 255 or
           offer.runner_progress_delta % 1 ~= 0 then
            return false
        end
        if offer.source_run_type ~= nil and
           (type(offer.source_run_type) ~= "number" or
            offer.source_run_type % 1 ~= 0 or
            offer.source_run_type < 0 or offer.source_run_type > 255) then
            return false
        end
        if offer.source_run_type == E.RunType.MATRIX_OBJECTIVE then
            if not valid_matrix_run_state(offer.matrix_run_state) then
                return false
            end
        elseif offer.matrix_run_state ~= nil then
            return false
        end
        local run = game.runs[offer.id]
        return type(run) == "table" and
               type(run.prepare_offer) == "function" and
               type(run.resolve_field_event) == "function"
    end

    if not valid_offer(state.current_offer) or
       not valid_offer(state.active_offer) or
       not valid_offer(state.last_completed_offer) then
        return false, "The save contains an invalid run offer."
    end
    if state.last_offer_result ~= nil and
       state.last_offer_result ~= "accepted" and
       state.last_offer_result ~= "declined" then
        return false, "The save contains an invalid run decision."
    end
    return true
end

function Registry.restore_saved_state(state)
    local valid, message = Registry.validate_saved_state(state)
    assert(valid, message)
    Registry.current_offer = state.current_offer
    Registry.active_offer = state.active_offer
    Registry.last_completed_offer = state.last_completed_offer
    Registry._last_offer_result = state.last_offer_result
end

return Registry
