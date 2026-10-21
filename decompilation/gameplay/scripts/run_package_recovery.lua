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

function PackageRecovery.resolve_field_event(offer, map_id, event_id)
    local route = type(offer) == "table" and offer.route or nil
    local target = type(route) == "table" and route.target or nil
    if type(target) ~= "table" or
       type(target.area_id) ~= "number" or
       type(target.location_id) ~= "number" then
        return { action = "no_action" }
    end

    -- ROM 0x1F046 matches FFFBF8/FFFBF9 to FFFBEE/FFFBEF before showing
    -- the package-recovery entry passage at 0xFDF90.
    if map_id == target.area_id and event_id == target.location_id then
        return {
            action = "entry_dialogue",
            dialogue_module = "run_package_recovery",
            dialogue_function = "present_target_entry",
        }
    end
    return { action = "no_action" }
end

function PackageRecovery.present_target_entry()
    local offer = game.runs.registry.active_offer
    assert(offer ~= nil and offer.id == "package_recovery")
    -- ROM 0xFDF90; the descriptor uses 0xFE5BF for its MORE indicator.
    game.ui.say_text(
        "<You walk into the reception area and single out a lone suit " ..
        "standing off to the side.>\n\n" ..
        "You give him a quick jab in the solar plexus and he drops, " ..
        "wheezing and clutching his chest.>", true)
    return "shown"
end

return PackageRecovery
