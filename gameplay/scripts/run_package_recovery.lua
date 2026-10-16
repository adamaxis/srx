-- A package-recovery contract selects a source location and a presentation
-- variant from the source initializer. Its later item pickup and payout are
-- still waiting on the field item/event bridge.
local PackageRecovery = {}

function PackageRecovery.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local target = support.choose_facility_target(
        source_record.source_initializer)
    local variant = support.bounded_random(12)
    local offer = support.new_offer(
        "package_recovery",
        "recovery",
        "recover_the_client_package",
        "return_the_recovered_package_to_the_johnson",
        { target = target },
        payment,
        source_record.runner_progress_delta)
    offer.context = context
    offer.presentation_variant = variant
    game.runs.registry.current_offer = offer
    return offer.id
end

function PackageRecovery.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "package_recovery")
    local target = offer.route.target
    game.ui.say_text(
        "Go to " .. target.location_name .. " " .. target.area_suffix ..
        "|A package was taken from a courier of mine.|Get the package and " ..
        "bring it back to me.|I will then pay you for your services.", true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for recovering the package.")
end

function PackageRecovery.resolve_field_event()
    -- Source item pickup and return-to-Johnson success are not connected to
    -- this field-event API yet; do not mark a map event as a completion.
    return { action = "no_action" }
end

return PackageRecovery
