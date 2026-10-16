-- Bodyguard contracts track an escort and pay on safe arrival. Location
-- selection and the escort's actor behavior are owned by this run module.
local BodyguardEscort = {}

function BodyguardEscort.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local route = support.choose_contract_route(context)
    local offer = game.modules.run_support.new_offer(
        "bodyguard_escort",
        "escort",
        "escort_client_to_destination",
        "client_reaches_destination_alive",
        route,
        payment,
        source_record.runner_progress_delta)
    game.runs.registry.current_offer = offer
    return offer.id
end

function BodyguardEscort.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "bodyguard_escort")
    local route = offer.route
    game.ui.say_text(
        "Go to " .. route.origin.location_name .. " " ..
        route.origin.area_suffix .. "|There you will meet my client, " ..
        route.client_name .. ".", true)
    local terms =
        "Escort my client to " .. route.destination.location_name .. " " ..
        route.destination.area_suffix ..
        "|You will be paid by my client upon safe arrival at the specified destination."
    -- ROM 0x214CE invokes this handler-specific second description and
    -- returns its text-only descriptor; the ghoul and courier branches instead
    -- enter the shared 0x214F6 payment/confirmation screen.
    game.ui.say_text(terms, false)
    return game.modules.run_support.present_run_acceptance(
        offer, "for escorting my client safely to the destination.")
end

function BodyguardEscort.resolve_field_event()
    -- The ROM spawns the escort client at the selected origin and evaluates
    -- their actor state at the destination. Actor spawn/death facts are not
    -- yet part of the field-event Lua contract, so no event is treated as
    -- completion here.
    return { action = "no_action" }
end

return BodyguardEscort
