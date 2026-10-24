-- Gunderson's ghoul contract has its own run behavior and success condition.
local GhoulBounty = {}
local E = require("enums")

function GhoulBounty.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local offer = game.modules.run_support.new_offer(
        "ghoul_bounty",
        "bounty",
        "kill_ghouls_in_redmond",
        "return_to_johnson_for_payment",
        nil,
        payment,
        source_record.runner_progress_delta)
    offer.context = context
    offer.area_id = support.choose_ghoul_area(context)
    offer.ghoul_kill_count = 0
    game.runs.registry.current_offer = offer
    return offer.id
end

function GhoulBounty.resolve_actor_defeat(offer, facts)
    local count = offer.ghoul_kill_count or 0
    if facts.race_or_state ~= E.ActorRaceOrState.GHOUL_BOUNTY_TARGET or
       count >= E.GhoulBounty.MAX_DEFEATS then
        return { action = "no_action", objective_progress = count }
    end

    count = count + 1
    offer.ghoul_kill_count = count
    return { action = "objective_advanced", objective_progress = count }
end

function GhoulBounty.resolve_johnson_return(offer, context)
    local count = offer.ghoul_kill_count or 0
    if context ~= offer.context or count <= 0 then
        return {
            action = "no_action",
            objective_progress = count,
        }
    end

    local payment = offer.payment
    assert(type(payment) == "table" and
           type(payment.negotiated_amount) == "number" and
           payment.negotiated_amount >= 0 and
           payment.negotiated_amount % 1 == 0,
        "ghoul bounty has invalid negotiated payment")
    return {
        action = "completed",
        reward_nuyen = payment.negotiated_amount * count,
        objective_progress = count,
    }
end

function GhoulBounty.resolve_field_event(offer, map_id, event_id)
    -- The ROM counts eligible ghoul deaths in its actor/combat path. Its
    -- event-return and payout branch has not been connected to Lua yet, so
    -- this run deliberately does not treat an arbitrary map event as success.
    return { action = "no_action" }
end

function GhoulBounty.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "ghoul_bounty")
    local support = game.modules.run_support
    local area = support.area_suffixes[offer.area_id]
    assert(area ~= nil, "ghoul bounty has an invalid target area")
    game.ui.say_text(
        "There is a community of Ghouls somewhere " .. area ..
        "|They have been preying on the citizens there." ..
        "|Kill as many as possible and return here for payment.", true)
    local payment = offer.payment
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

function GhoulBounty.present_completion()
    -- ROM 0x105AB7..0x105B79 is the successful Jump House return text.
    game.ui.say_text(
        "Looks like you made it through that run.  Let me get this out of the way." ..
        "||<Creds and information are exchanged and Julius smugly reclines in the booth.>" ..
        "||Anything else we can do for each other?", false)
end

return GhoulBounty
