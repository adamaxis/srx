-- ROM 0x1E69C selects a Johnson's offer row. Lua selects by stable contact
-- identity and dispatches the chosen run to its separate implementation.
local Registry = {}
local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local FileClasses = require("data.matrix_file_classes")

local RunStatus = E.MatrixRunStatus
local RunAction = E.MatrixRunAction

local function class_name(class_id)
    local label = FileClasses[class_id]
    return type(label) == "string" and label:match("^%s*(.-)%s*$") or nil
end

-- Saved offers from the previous schema are normalized at the save boundary.
-- New gameplay code sees only named lifecycle and objective facts.
local function matrix_run_state_view(state)
    if type(state) ~= "table" then return nil end

    local status = state.status
    local objective = state.objective
    local file_size = state.file_size
    local completion_marker_present = state.completion_marker_present
    local completion_recorded = state.completion_recorded
    if status == nil then
        local legacy_status = state.state
        local legacy_class = state.objective
        local legacy_value = state.value
        local old_completion_flags = state.fc28
        local old_run_flags = state.fc1c
        local function legacy_integer(value, minimum, maximum)
            return type(value) == "number" and value % 1 == 0 and
                   value >= minimum and value <= maximum
        end
        if not legacy_integer(legacy_status, -128, 127) or
           not legacy_integer(legacy_class, -128, 127) or
           not legacy_integer(legacy_value, -128, 127) or
           not legacy_integer(old_completion_flags, 0, 255) or
           not legacy_integer(old_run_flags, 0, 255) then
            return nil
        end
        status = ((old_run_flags & 0x80) ~= 0 or legacy_status > 0) and
                     RunStatus.COMPLETED or
                 (legacy_status < 0 and RunStatus.ACTIVE or
                  RunStatus.INACTIVE)
        if legacy_class < 0 then
            objective = { node = "cpu", action = RunAction.CRASH_SYSTEM }
        else
            local action = legacy_value < 0 and RunAction.UPLOAD_FILE or
                           legacy_value == 0 and RunAction.DELETE_FILE or
                           RunAction.DOWNLOAD_FILE
            objective = {
                node = "datastore",
                action = action,
                file_class = class_name(legacy_class),
                file_id = legacy_value >= 0 and legacy_value or nil,
            }
        end
        file_size = state.size
        completion_marker_present = (old_completion_flags & 1) ~= 0
        completion_recorded = (old_run_flags & 0x80) ~= 0 or
                              legacy_status > 0
    end

    if (status ~= RunStatus.PENDING and status ~= RunStatus.INACTIVE and
        status ~= RunStatus.ACTIVE and status ~= RunStatus.COMPLETED) or
       type(objective) ~= "table" or
       (objective.node ~= "cpu" and objective.node ~= "datastore") or
       type(file_size) ~= "number" or file_size % 1 ~= 0 or
       file_size < 0 or file_size > 255 or
       type(completion_marker_present) ~= "boolean" or
       type(completion_recorded) ~= "boolean" then
        return nil
    end

    local action = objective.action
    local file_id = objective.file_id
    local valid_action = action == RunAction.CRASH_SYSTEM or
                         action == RunAction.DELETE_FILE or
                         action == RunAction.UPLOAD_FILE or
                         action == RunAction.DOWNLOAD_FILE
    local valid_file_class = false
    for _, label in pairs(FileClasses) do
        if type(label) == "string" and
           label:match("^%s*(.-)%s*$") == objective.file_class then
            valid_file_class = true
            break
        end
    end
    local valid_objective = valid_action and
        ((objective.node == "cpu" and
          action == RunAction.CRASH_SYSTEM and
          objective.file_class == nil and file_id == nil) or
         (objective.node == "datastore" and
          action ~= RunAction.CRASH_SYSTEM and valid_file_class and
          ((action == RunAction.UPLOAD_FILE and file_id == nil) or
           ((action == RunAction.DELETE_FILE or
             action == RunAction.DOWNLOAD_FILE) and
            type(file_id) == "number" and file_id % 1 == 0 and
            file_id >= 0 and file_id <= 69))))
    if not valid_objective then return nil end

    local target_system_id = state.target_system_id
    if target_system_id ~= nil and
       (type(target_system_id) ~= "number" or
        target_system_id % 1 ~= 0 or
        target_system_id < 0 or target_system_id > 24) then
        return nil
    end
    return {
        status = status,
        objective = {
            node = objective.node,
            action = action,
            file_class = objective.file_class,
            file_id = file_id,
        },
        file_size = file_size,
        completion_marker_present = completion_marker_present,
        completion_recorded = completion_recorded,
        target_system_id = target_system_id,
    }
end

local function valid_matrix_run_state(state)
    return matrix_run_state_view(state) ~= nil
end

local function copy_matrix_run_state(state)
    return matrix_run_state_view(state)
end

local function matrix_run_state_for(offer)
    if type(offer) ~= "table" or
       offer.id ~= E.RunId.MATRIX_OBJECTIVE or
       not valid_matrix_run_state(offer.matrix_run_state) then
        return nil
    end
    return copy_matrix_run_state(offer.matrix_run_state)
end

local function set_matrix_run_state(offer, state)
    if type(offer) ~= "table" or
       offer.id ~= E.RunId.MATRIX_OBJECTIVE or
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

local new_game_selection = OfferData.contacts[E.RunContact.JUMP_HOUSE]

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

local function prepare_source_offer(contact_id, source_record)
    local variant_id = source_record.variant_id
    local resolved = OfferData.run_by_variant_id[variant_id]
    assert(resolved ~= nil,
        "offer row has an unknown gameplay variant: " .. tostring(variant_id))
    local run = game.runs[resolved.id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "offer variant has no Lua implementation: " .. resolved.id)
    Registry.current_offer = nil
    Registry._last_offer_result = nil
    local run_id = run.prepare_offer(contact_id, source_record)
    local offer = Registry.current_offer
    assert(run_id == resolved.id and type(offer) == "table" and
           offer.id == resolved.id and offer.contact_id == contact_id,
        "run handler failed to publish its Lua offer state")
    offer.offer_variant_id = variant_id
    offer.target_pool_id = source_record.target_pool_id
    offer.matrix_band_id = source_record.matrix_band_id
    return run_id
end

function Registry.prepare_offer_for_contact(contact_id)
    local rows = OfferData.contacts[contact_id]
    assert(rows ~= nil,
        "Johnson contact has no offer list: " .. tostring(contact_id))
    local slot = game.modules.run_support.bounded_random(#rows)
    return prepare_source_offer(contact_id, rows[slot + 1])
end

function Registry.present_offer_for_contact(contact_id)
    local run_id = Registry.prepare_offer_for_contact(contact_id)
    return game.runs[run_id].present_offer()
end

function Registry.prepare_new_game_offer()
    -- Preserve the original eight Jump House offers before explicitly
    -- weighted gameplay extensions.
    local contact_id = E.RunContact.JUMP_HOUSE
    local extension_slots = extension_offer_slots()
    local slot = game.modules.run_support.bounded_random(
        #new_game_selection + #extension_slots)
    if slot < #new_game_selection then
        return prepare_source_offer(contact_id, new_game_selection[slot + 1])
    end

    local extension = extension_slots[slot - #new_game_selection + 1]
    local run = game.runs[extension.run_id]
    assert(type(run) == "table" and type(run.prepare_offer) == "function",
        "weighted run slot has no Lua implementation: " .. extension.run_id)
    Registry.current_offer = nil
    Registry._last_offer_result = nil
    return run.prepare_offer(contact_id, extension.payment)
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

function Registry.resolve_field_event(location_id)
    assert(type(location_id) == "string" and
           game.modules.run_support.location_details(location_id) ~= nil,
        "run field event needs a semantic place ID")

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

    local result = handler(offer, location_id)
    assert(type(result) == "table" and
           (result.action == "no_action" or
            result.action == "objective_advanced" or
            result.action == "completed" or
            result.action == "entry_dialogue"),
        "run field-event handler returned an invalid action")

    local dialogue_module = ""
    local dialogue_function = ""
    if result.action == "entry_dialogue" then
        dialogue_module = result.dialogue_module
        dialogue_function = result.dialogue_function
        assert(type(dialogue_module) == "string" and
               dialogue_module:match("^[%a_][%w_]*$") ~= nil and
               type(dialogue_function) == "string" and
               dialogue_function:match("^[%a_][%w_]*$") ~= nil,
            "run field-event dialogue must name a Lua task")
    end

    local decision = {
        action = result.action,
        run_id = offer.id,
        reward_nuyen = result.reward_nuyen or 0,
        runner_progress_delta = offer.runner_progress_delta or 0,
    }
    if result.action == "entry_dialogue" then
        decision.dialogue_module = dialogue_module
        decision.dialogue_function = dialogue_function
        local has_actor_group = result.actor_group_id ~= nil
        if has_actor_group then
            local target = offer.route and offer.route.target
            assert(type(result.actor_group_id) == "string" and
                   result.actor_group_id:match("^[%w_.:-]+$") ~= nil and
                   type(target) == "table" and
                   game.modules.run_support.gang_actor_group_id(
                       target.location_id) == result.actor_group_id,
                "run field-event actor group has no semantic target")
            decision.actor_group_id = result.actor_group_id
        end
    end
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
           type(facts.is_ghoul_bounty_target) == "boolean",
        "actor-defeat facts have invalid target classification")

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

function Registry.resolve_johnson_return(contact_id)
    assert(type(contact_id) == "string" and
           OfferData.contacts[contact_id] ~= nil,
        "Johnson-return contact is invalid")

    Registry._pending_johnson_return = nil
    local offer = Registry.active_offer
    if offer == nil then
        return {
            action = "no_action",
            run_id = "none",
            reward_nuyen = 0,
            runner_progress_delta = 0,
            objective_progress = 0,
        }
    end

    local run = game.runs[offer.id]
    local handler = type(run) == "table" and
        run.resolve_johnson_return or nil
    if type(handler) ~= "function" then
        return {
            action = "no_action",
            run_id = offer.id,
            reward_nuyen = 0,
            runner_progress_delta = 0,
            objective_progress = 0,
        }
    end

    local result = handler(offer, contact_id)
    assert(type(result) == "table" and
           (result.action == "no_action" or result.action == "completed"),
        "run Johnson-return handler returned an invalid action")
    local reward = result.reward_nuyen or 0
    local progress = result.objective_progress or 0
    assert(type(reward) == "number" and reward >= 0 and
           reward <= 0xFFFFFFFF and reward % 1 == 0,
        "run Johnson-return handler returned an invalid reward")
    assert(type(progress) == "number" and progress >= 0 and
           progress <= 0xFF and progress % 1 == 0,
        "run Johnson-return handler returned invalid objective progress")
    local progress_delta = offer.runner_progress_delta or 0
    assert(type(progress_delta) == "number" and progress_delta >= 0 and
           progress_delta <= 0xFF and progress_delta % 1 == 0,
        "run offer has an invalid runner progress delta")

    if result.action == "completed" then
        Registry.last_completed_offer = offer
        Registry.active_offer = nil
        Registry._pending_johnson_return = {
            run_id = offer.id,
            contact_id = contact_id,
            objective_progress = progress,
        }
    end
    return {
        action = result.action,
        run_id = offer.id,
        reward_nuyen = reward,
        runner_progress_delta = progress_delta,
        objective_progress = progress,
    }
end

function Registry.take_pending_johnson_return()
    local pending = Registry._pending_johnson_return
    Registry._pending_johnson_return = nil
    if pending == nil then return nil end
    return {
        run_id = pending.run_id,
        contact_id = pending.contact_id,
        objective_progress = pending.objective_progress,
    }
end

-- ROM 0x1E68C clears the active run when the Wanderer booth is vacated.
function Registry.clear_active_offer_for_contact(contact_id)
    assert(type(contact_id) == "string" and
           OfferData.contacts[contact_id] ~= nil,
        "Johnson contact reset is invalid")
    local cleared = false
    local active = Registry.active_offer
    if active ~= nil and active.contact_id == contact_id then
        Registry.active_offer = nil
        cleared = true
    end
    local pending = Registry._pending_johnson_return
    if pending ~= nil and pending.contact_id == contact_id then
        Registry._pending_johnson_return = nil
        cleared = true
    end
    return cleared
end

-- Party recovery consumes stable run and contact identities plus a named
-- scene-reset condition. Source type/context selectors do not enter Lua [C].
function Registry.party_wipe_failure_state(world_activity_scene_reset)
    local offer = Registry.active_offer
    if offer == nil or type(offer.id) ~= "string" then
        return {
            run_id = "none",
            active = false,
            employee_failure_guard = false,
        }
    end
    return {
        run_id = offer.id,
        active = true,
        employee_failure_guard =
            offer.id == E.RunId.EMPLOYEE_RECRUITMENT and
            world_activity_scene_reset == true,
    }
end

-- ROM 0x1E68C resets FFFBEC and clears the run-status byte at FFFC1C.
local function clear_active_offer_and_pending_return()
    local cleared = Registry.active_offer ~= nil or
        Registry._pending_johnson_return ~= nil
    Registry.active_offer = nil
    Registry._pending_johnson_return = nil
    return cleared
end

function Registry.clear_active_offer_for_world_activity_failure()
    -- ROM 0x1F16A's no-record branch calls 0x1E68C, which resets FFFBEC and
    -- FFFC1C. Keep that run-state transition in the Lua registry as well.
    return clear_active_offer_and_pending_return()
end

function Registry.clear_active_offer_after_party_wipe()
    -- Party-wipe recovery reaches the same 0x1E68C reset routine.
    return clear_active_offer_and_pending_return()
end

function Registry.active_run_id()
    return Registry.active_offer and Registry.active_offer.id or "none"
end

-- [C] The ROM retains its selected handler in FFFBED. Lua keeps the equivalent
-- named gameplay variant, including target-pool and Matrix-band differences.
function Registry.active_offer_variant_id()
    return Registry.active_offer and Registry.active_offer.offer_variant_id
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
            origin_region = "none",
            client = "none",
            employee = "none",
            destination_location = "none",
            destination_region = "none",
        }
    end
    local function point_ids(point)
        if type(point) ~= "table" then return "none", "none" end
        local details = game.modules.run_support.location_details(
            point.location_id)
        if details == nil then return "none", "none" end
        return details.location_id, details.region_id
    end
    local origin_location, origin_region = point_ids(route.origin)
    local destination_location, destination_region =
        point_ids(route.destination)
    local target_location, target_region = point_ids(route.target)
    return {
        origin_location = origin_location,
        origin_region = origin_region,
        client = type(route.client_id) == "string"
            and route.client_id or "none",
        employee = type(offer.employee_id) == "string"
            and offer.employee_id or "none",
        destination_location = destination_location,
        destination_region = destination_region,
        target_location = target_location,
        target_region = target_region,
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

    local function integer_between(value, minimum, maximum)
        return type(value) == "number" and value % 1 == 0 and
               value >= minimum and value <= maximum
    end

    -- The saved page and Johnson-return context are derived from the same
    -- accepted ROM offer. Validate the joined row rather than trusting a
    -- separately saved variant, context, progress, or payment tuple.
    local function valid_source_offer(offer)
        local variant_id = offer.offer_variant_id
        if variant_id == nil then return true end
        if type(variant_id) ~= "string" or variant_id == "" then
            return false
        end

        local resolved = OfferData.run_by_variant_id[variant_id]
        local contact_id = offer.contact_id
        if resolved == nil or
           type(contact_id) ~= "string" or
           OfferData.contacts[contact_id] == nil or
           offer.id ~= resolved.id then
            return false
        end

        local payment = offer.payment
        if type(payment) ~= "table" or
           not integer_between(payment.base_amount, 0, 0xFFFF) or
           not integer_between(payment.negotiated_amount, 0, 0xFFFF) or
           not integer_between(payment.negotiation_skill, 0, 12) or
           not integer_between(payment.negotiation_bound, 1, 0xFFFF) or
           not integer_between(payment.negotiation_roll, 0, 0xFFFF) then
            return false
        end

        for _, source_record in ipairs(OfferData.contacts[contact_id]) do
            if source_record.variant_id == variant_id and
               source_record.base_payment == payment.base_amount and
               source_record.runner_progress_delta ==
                   offer.runner_progress_delta then
                local expected_bound = source_record.negotiation_scale *
                    payment.negotiation_skill + 1
                if payment.negotiation_bound == expected_bound and
                   payment.negotiation_roll < expected_bound and
                   payment.negotiated_amount == payment.base_amount +
                       payment.negotiation_roll * 5 then
                    return true
                end
            end
        end
        return false
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
        if not valid_source_offer(offer) then return false end
        local support = game.modules.run_support
        local function valid_location_point(point)
            return type(point) == "table" and
                support.location_details(point.location_id) ~= nil and
                point.area_id == nil and point.location_name == nil and
                point.area_suffix == nil
        end
        local route = offer.route
        if offer.id == E.RunId.BODYGUARD_ESCORT or
           offer.id == E.RunId.PACKAGE_DELIVERY then
            if type(route) ~= "table" or
               not valid_location_point(route.origin) or
               not valid_location_point(route.destination) then
                return false
            end
        elseif offer.id == E.RunId.GANG_CONTRACT or
               offer.id == E.RunId.PACKAGE_RECOVERY or
               offer.id == E.RunId.EMPLOYEE_RECRUITMENT then
            if type(route) ~= "table" or
               not valid_location_point(route.target) then
                return false
            end
        elseif route ~= nil then
            return false
        end
        if offer.id == E.RunId.BODYGUARD_ESCORT then
            if support.client_name(offer.route.client_id) == nil then
                return false
            end
        elseif offer.id == E.RunId.EMPLOYEE_RECRUITMENT then
            if support.client_name(offer.employee_id) == nil then
                return false
            end
        elseif offer.employee_id ~= nil then
            return false
        end
        if offer.id == E.RunId.GANG_CONTRACT then
            if support.gang_contract_variant_text(
                    offer.contract_variant_id) == nil then
                return false
            end
        elseif offer.contract_variant_id ~= nil then
            return false
        end
        if offer.id == E.RunId.MATRIX_OBJECTIVE then
            if not valid_matrix_run_state(offer.matrix_run_state) then
                return false
            end
        elseif offer.matrix_run_state ~= nil then
            return false
        end
        if offer.id == "ghoul_bounty" then
            local region_id = offer.region_id
            if region_id ~= "downtown_seattle" and
               region_id ~= "puyallup_barrens" and
               region_id ~= "redmond_barrens" and
               region_id ~= "penumbra_district" then
                return false
            end
            local count = offer.ghoul_kill_count
            if count ~= nil and
               (type(count) ~= "number" or count % 1 ~= 0 or
                count < 0 or count > E.GhoulBounty.MAX_DEFEATS) then
                return false
            end
        elseif offer.ghoul_kill_count ~= nil or offer.region_id ~= nil then
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
    local function restored_offer(offer)
        if type(offer) ~= "table" then return offer end
        local restored = {}
        for key, value in pairs(offer) do restored[key] = value end
        if restored.id == E.RunId.MATRIX_OBJECTIVE then
            restored.matrix_run_state =
                copy_matrix_run_state(restored.matrix_run_state)
        end
        return restored
    end
    Registry.current_offer = restored_offer(state.current_offer)
    Registry.active_offer = restored_offer(state.active_offer)
    Registry.last_completed_offer = restored_offer(state.last_completed_offer)
    Registry._last_offer_result = state.last_offer_result
end

return Registry
