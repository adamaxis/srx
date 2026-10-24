-- Courier contracts track the package and pay only after safe delivery.
local PackageDelivery = {}

function PackageDelivery.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local route = support.choose_contract_route(context)
    local offer = game.modules.run_support.new_offer(
        "package_delivery",
        "courier",
        "deliver_package_to_destination",
        "package_delivered_safely",
        route,
        payment,
        source_record.runner_progress_delta)
    game.runs.registry.current_offer = offer
    return offer.id
end

function PackageDelivery.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "package_delivery")
    local route = offer.route
    -- ROM 0x1EBAA appends the origin, area suffix, and F23B8 stream into one
    -- text buffer. F23B8 alone supplies the two 0x80 row breaks before
    -- "Take it to"; the other phrase boundaries wrap naturally at 26 columns.
    game.ui.say_text(
        "Go to " .. route.origin.location_name .. " " ..
        route.origin.area_suffix .. "  " ..
        "A contact there will give you a package." ..
        "||Take it to " .. route.destination.location_name .. " " ..
        route.destination.area_suffix .. "  " ..
        "You will be paid upon safe delivery of the package.", true)
    local payment = offer.payment
    local support = game.modules.run_support
    local prompt =
        "<You are offered" ..
        support.amount_in_reserved_field(payment.base_amount, 6) ..
        "$|for the run.|After negotiations, you|come to an agreement.>|" ..
        "Payment for this run is|" ..
        support.amount_in_reserved_field(payment.negotiated_amount, 5) ..
        "$.|Whatta' you say, friend?"
    local choice = game.ui.prompt_text(
        "run_acceptance", prompt, { "yes", "no" })
    return game.modules.run_support.resolve_offer_decision(choice)
end

function PackageDelivery.resolve_field_event(offer, map_id, event_id)
    local route = offer.route
    if type(route) ~= "table" then
        return { action = "no_action" }
    end

    -- ROM 0x1EF2C compares FFFBF8/FFFBF9 against the chosen origin first,
    -- then checks the destination after the run's active bit has been set.
    -- For type 2 it changes the alternate result at each match; the origin
    -- match arms delivery, and the destination match enters the common payout.
    local origin_matches =
        map_id == route.origin.area_id and
        event_id == route.origin.location_id
    local destination_matches =
        map_id == route.destination.area_id and
        event_id == route.destination.location_id

    if not offer.package_collected then
        if origin_matches then
            offer.package_collected = true
            return { action = "objective_advanced" }
        end
        return { action = "no_action" }
    end

    if destination_matches then
        offer.completed = true
        return {
            action = "completed",
            reward_nuyen = offer.payment.negotiated_amount,
        }
    end
    return { action = "no_action" }
end

return PackageDelivery
