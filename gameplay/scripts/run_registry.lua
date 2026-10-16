-- ROM 0x1E69C selects one of eight six-byte F3EBC records. The context-zero
-- handler sequence below is expressed as Lua run IDs; the registry only
-- selects a run and dispatches to its separate implementation.
local Registry = {}
local E = require("enums")
local OfferData = require("data.johnson_run_offers")

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

local new_game_selection =
    OfferData.contexts[E.RunOfferContext.JUMP_HOUSE]

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

local function prepare_source_offer(context, source_record)
    local source_type = source_record.source_initializer
    local resolved = OfferData.run_by_initializer[source_type]
    assert(resolved ~= nil,
        "ROM offer row has an unknown initializer: " .. tostring(source_type))
    local run = game.runs[resolved.id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "ROM offer initializer has no Lua implementation: " .. resolved.id)
    Registry.current_offer = nil
    Registry._last_offer_result = nil
    local run_id = run.prepare_offer(context, source_record)
    local offer = Registry.current_offer
    assert(run_id == resolved.id and type(offer) == "table" and
           offer.id == resolved.id,
        "run initializer failed to publish its Lua offer state")
    offer.context = context
    offer.source_initializer = source_type
    offer.source_offer_type = source_type
    offer.source_offer_context = context
    offer.source_run_type = resolved.type
    return run_id
end

function Registry.prepare_offer_for_context(context)
    local rows = OfferData.contexts[context]
    assert(rows ~= nil,
        "ROM Johnson offer context is not present: " .. tostring(context))
    local slot = game.modules.run_support.bounded_random(#rows)
    return prepare_source_offer(context, rows[slot + 1])
end

function Registry.present_offer_for_context(context)
    local run_id = Registry.prepare_offer_for_context(context)
    return game.runs[run_id].present_offer()
end

function Registry.prepare_new_game_offer()
    -- New Game uses ROM context zero. Keep the original eight source rows
    -- first; explicitly weighted extensions may be appended after that pool.
    local context = E.RunOfferContext.JUMP_HOUSE
    local extension_slots = extension_offer_slots()
    local slot = game.modules.run_support.bounded_random(
        #new_game_selection + #extension_slots)
    if slot < #new_game_selection then
        return prepare_source_offer(context, new_game_selection[slot + 1])
    end

    local extension = extension_slots[slot - #new_game_selection + 1]
    local run = game.runs[extension.run_id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "weighted run slot has no Lua implementation: " .. extension.run_id)
    Registry.current_offer = nil
    Registry._last_offer_result = nil
    return run.prepare_offer(context, extension.payment)
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

function Registry.resolve_actor_defeat(facts)
    assert(type(facts) == "table" and
           type(facts.race_or_state) == "number" and
           facts.race_or_state % 1 == 0 and
           facts.race_or_state >= -128 and facts.race_or_state <= 127,
        "actor-defeat facts have an invalid race/state byte")

    local offer = Registry.active_offer
    if offer == nil then
        return {
            action = "no_action",
            run_id = "none",
            objective_progress = 0,
        }
    end

    local run = game.runs[offer.id]
    local handler = type(run) == "table" and run.resolve_actor_defeat or nil
    if type(handler) ~= "function" then
        return {
            action = "no_action",
            run_id = offer.id,
            objective_progress = 0,
        }
    end

    local result = handler(offer, facts)
    assert(type(result) == "table" and
           (result.action == "no_action" or
            result.action == "objective_advanced"),
        "run actor-defeat handler returned an invalid action")
    local progress = result.objective_progress or 0
    assert(type(progress) == "number" and progress >= 0 and
           progress <= 255 and progress % 1 == 0,
        "run actor-defeat handler returned invalid objective progress")
    return {
        action = result.action,
        run_id = offer.id,
        objective_progress = progress,
    }
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
    local function pair_ids(pair)
        if type(pair) ~= "table" then return "none", "none" end
        return "location_" .. tostring(pair.location_id),
               "area_" .. tostring(pair.area_id)
    end
    local origin_location, origin_area = pair_ids(route.origin)
    local destination_location, destination_area = pair_ids(route.destination)
    local target_location, target_area = pair_ids(route.target)
    return {
        origin_location = origin_location,
        origin_area = origin_area,
        client = route.client_id ~= nil and
            "client_" .. tostring(route.client_id) or "none",
        destination_location = destination_location,
        destination_area = destination_area,
        target_location = target_location,
        target_area = target_area,
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
        if offer.source_offer_type ~= nil then
            local initializer = offer.source_offer_type
            local context = offer.source_offer_context
            local resolved = type(initializer) == "number" and
                OfferData.run_by_initializer[initializer] or nil
            if resolved == nil or type(context) ~= "number" or
               context % 1 ~= 0 or OfferData.contexts[context] == nil or
               offer.source_run_type ~= resolved.type then
                return false
            end
        end
        if offer.source_run_type == E.RunType.MATRIX_OBJECTIVE then
            if not valid_matrix_run_state(offer.matrix_run_state) then
                return false
            end
        elseif offer.matrix_run_state ~= nil then
            return false
        end
        if offer.id == "ghoul_bounty" then
            local count = offer.ghoul_kill_count
            if count ~= nil and
               (type(count) ~= "number" or count % 1 ~= 0 or
                count < 0 or count > E.GhoulBounty.MAX_DEFEATS) then
                return false
            end
        elseif offer.ghoul_kill_count ~= nil then
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
