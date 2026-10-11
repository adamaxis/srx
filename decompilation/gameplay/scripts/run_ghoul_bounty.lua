-- Gunderson's ghoul contract has its own run behavior and success condition.
local GhoulBounty = {}

function GhoulBounty.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local offer = game.modules.run_support.new_offer(
        "ghoul_bounty",
        "bounty",
        "kill_ghouls_in_redmond",
        "return_to_johnson_for_payment",
        nil,
        payment)
    offer.context = context
    offer.area_id = 2
    game.runs.registry.current_offer = offer
    return offer.id
end

function GhoulBounty.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "ghoul_bounty")
    local area = game.modules.run_support.area_suffixes[2]
    game.ui.say_text(
        "There is a community of Ghouls somewhere " .. area ..
        "|They have been preying on the citizens there." ..
        "|Kill as many as possible and return here for payment.", true)
    local payment = offer.payment
    local support = game.modules.run_support
    local prompt =
        "<You are offered" ..
        support.amount_in_reserved_field(payment.base_amount, 6) ..
        "$|for each ghoul killed.|After negotiations, you|come to an agreement.>|" ..
        "Payment for this run is|" ..
        support.amount_in_reserved_field(payment.negotiated_amount, 5) ..
        "$ each, up to twenty|kills max.|Whatta' you say, friend?"
    local choice = game.ui.prompt_text(
        "run_acceptance", prompt, { "yes", "no" })
    return game.modules.run_support.resolve_offer_decision(choice)
end

return GhoulBounty
