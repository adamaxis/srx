package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local Registry = require("run_registry")
local Support = require("run_support")

local run_modules = { registry = Registry }
for _, resolved in pairs(OfferData.run_by_initializer) do
    run_modules[resolved.id] = {
        prepare_offer = function() end,
        resolve_field_event = function() return { action = "no_action" } end,
    }
end
game = { runs = run_modules }

local function matrix_state()
    return {
        state = 0,
        objective = 0,
        value = 0,
        size = 10,
        fc28 = 0,
        fc1c = 0,
        target_system_id = 0,
    }
end

local function source_offer(context, row, skill, roll)
    local resolved = OfferData.run_by_initializer[row.source_initializer]
    local bound = row.negotiation_scale * skill + 1
    local offer = {
        id = resolved.id,
        category = "audit",
        objective = "source_offer",
        completion_condition = "source_completion",
        runner_progress_delta = row.runner_progress_delta,
        source_initializer = row.source_initializer,
        source_offer_type = row.source_initializer,
        source_offer_context = context,
        source_run_type = resolved.type,
        context = context,
        payment = {
            base_amount = row.base_payment,
            negotiated_amount = row.base_payment + roll * 5,
            negotiation_skill = skill,
            negotiation_bound = bound,
            negotiation_roll = roll,
        },
    }
    if resolved.type == E.RunType.MATRIX_OBJECTIVE then
        offer.matrix_run_state = matrix_state()
    end
    return offer
end

local function saved(offer)
    return {
        schema_version = 1,
        current_offer = nil,
        active_offer = offer,
        last_completed_offer = nil,
        last_offer_result = "accepted",
    }
end

local function valid(offer)
    return Registry.validate_saved_state(saved(offer))
end

local function copy_offer(original)
    local copy = {}
    for key, value in pairs(original) do
        if type(value) == "table" then
            local nested = {}
            for nested_key, nested_value in pairs(value) do
                nested[nested_key] = nested_value
            end
            copy[key] = nested
        else
            copy[key] = value
        end
    end
    return copy
end

local checked = 0
local courier
for context = 0, 4 do
    for _, row in ipairs(OfferData.contexts[context]) do
        for skill = 0, 12 do
            local bound = row.negotiation_scale * skill + 1
            for _, roll in ipairs({ 0, bound - 1 }) do
                local offer = source_offer(context, row, skill, roll)
                assert(valid(offer), string.format(
                    "valid source save rejected: context=%d initializer=%d " ..
                    "skill=%d roll=%d",
                    context, row.source_initializer, skill, roll))
                checked = checked + 1
                if context == E.RunOfferContext.JUMP_HOUSE and
                   row.source_initializer == E.RunOfferInitializer.PACKAGE_DELIVERY and
                   row.base_payment == 40 and skill == 3 and roll == 3 then
                    courier = offer
                end
            end
        end
    end
end
assert(checked == 1040,
    "source save matrix must cover 40 rows, 13 ranks, and two roll edges")
assert(courier ~= nil and courier.payment.negotiated_amount == 55,
    "Section 03 Courier offer must resolve to the saved 40 -> 55 amount")
courier.route = {
    origin = { area_id = 2, location_id = 11 },
    destination = { area_id = 2, location_id = 5 },
}

local mismatched_context = copy_offer(courier)
mismatched_context.context = E.RunOfferContext.SCENE_18
mismatched_context.source_offer_context = E.RunOfferContext.SCENE_18
assert(not valid(mismatched_context),
    "initializer 2 cannot be restored as an offer from context 1")

local mismatched_alias = copy_offer(courier)
mismatched_alias.source_initializer = E.RunOfferInitializer.GHOUL_BOUNTY
assert(not valid(mismatched_alias),
    "source initializer aliases must agree")

local missing_context = copy_offer(courier)
missing_context.source_offer_context = nil
assert(not valid(missing_context),
    "source-backed save must retain its Johnson context")

local mismatched_run = copy_offer(courier)
mismatched_run.id = "ghoul_bounty"
assert(not valid(mismatched_run),
    "source initializer must dispatch to its canonical Lua run")

local wrong_progress = copy_offer(courier)
wrong_progress.runner_progress_delta = 2
assert(not valid(wrong_progress),
    "saved progress delta must match the selected ROM offer row")

local missing_payment = copy_offer(courier)
missing_payment.payment = nil
assert(not valid(missing_payment),
    "source-backed accepted run must retain the negotiated payment record")

local wrong_skill = copy_offer(courier)
wrong_skill.payment.negotiation_skill = 13
assert(not valid(wrong_skill),
    "saved Negotiation cannot exceed the Attributes/Skills cap of 12")

local wrong_bound = copy_offer(courier)
wrong_bound.payment.negotiation_bound = 5
assert(not valid(wrong_bound),
    "saved payment bound must equal source scale times skill plus one")

local out_of_range_roll = copy_offer(courier)
out_of_range_roll.payment.negotiation_roll = 4
out_of_range_roll.payment.negotiated_amount = 60
assert(not valid(out_of_range_roll),
    "a maximum-bound roll cannot be restored")

local wrong_amount = copy_offer(courier)
wrong_amount.payment.negotiated_amount = 54
assert(not valid(wrong_amount),
    "saved negotiated payment must equal base plus five per roll")

Registry.current_offer = copy_offer(courier)
Registry.active_offer = nil
assert(Registry.resolve_current_offer("yes") == "accepted" and
       Registry.active_offer.payment.negotiated_amount == 55,
    "acceptance must move the negotiated offer into active-run state")
local accepted_save = Registry.save_state()
assert(Registry.validate_saved_state(accepted_save),
    "accepted Courier run must pass source save validation")
Registry.active_offer = nil
Registry.restore_saved_state(accepted_save)
assert(Registry.active_offer.source_offer_context ==
           E.RunOfferContext.JUMP_HOUSE and
       Registry.active_offer.source_initializer ==
           E.RunOfferInitializer.PACKAGE_DELIVERY and
       Registry.active_offer.payment.base_amount == 40 and
       Registry.active_offer.payment.negotiated_amount == 55 and
       Support.current_run_view(Registry.active_offer).payment_amount == 55,
    "save restore must preserve the accepted Current Run context and amount")

print("run registry save validation passed: " .. tostring(checked) ..
    " source row/rank/roll states; accepted Courier 40 -> 55 persisted; " ..
    "corrupt context, initializer, progression, and payment variants rejected")
