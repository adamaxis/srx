-- Gunderson's ghoul contract has its own run behavior and success condition.
local GhoulBounty = {}

function GhoulBounty.prepare_offer(context)
    local offer = game.modules.run_support.new_offer(
        "ghoul_bounty",
        "bounty",
        "kill_ghouls_in_redmond",
        "return_to_johnson_for_payment")
    offer.context = context
    offer.area_id = 2
    game.runs.registry.current_offer = offer
    return offer.id
end

function GhoulBounty.present_offer()
    local area = game.modules.run_support.area_suffixes[2]
    game.ui.say_text(
        "There is a community of Ghouls somewhere " .. area ..
        "|They have been preying on the citizens there." ..
        "|Kill as many as possible and return here for payment.", true)
    return "description_presented"
end

return GhoulBounty
