-- Gang contracts are authored as a dedicated run module. Their offer text,
-- target pool, and random variant come from ROM tables F36FA and F3E7A.
local GangContract = {}

function GangContract.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local target = support.choose_gang_contract_target()
    local variant = support.bounded_random(#support.gang_contract_texts)
    local offer = support.new_offer(
        "gang_contract",
        "gang",
        "resolve_the_selected_gang_contract",
        "return_to_the_johnson_after_the_gang_contract",
        { target = target },
        payment,
        source_record.runner_progress_delta)
    offer.context = context
    offer.variant = variant
    game.runs.registry.current_offer = offer
    return offer.id
end

function GangContract.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "gang_contract")
    local target = offer.route.target
    game.ui.say_text(
        "Go to " .. target.location_name .. " " .. target.area_suffix ..
        "|" .. game.modules.run_support.gang_contract_texts[offer.variant + 1],
        true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for completing this gang contract.")
end

function GangContract.resolve_field_event()
    -- The ROM turns this offer into an armed encounter at its target. The
    -- field actor/encounter result is not yet exposed to the run script.
    return { action = "no_action" }
end

return GangContract
