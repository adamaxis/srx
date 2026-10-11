-- Bodyguard contracts track an escort and pay on safe arrival. Location
-- selection and the escort's actor behavior are owned by this run module.
local BodyguardEscort = {}

function BodyguardEscort.prepare_offer(context)
    local support = game.modules.run_support
    local route = support.choose_contract_route(context)
    local offer = game.modules.run_support.new_offer(
        "bodyguard_escort",
        "escort",
        "escort_client_to_destination",
        "client_reaches_destination_alive",
        route)
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
        route.client_name .. ".|Escort my client to " ..
        route.destination.location_name .. " " ..
        route.destination.area_suffix ..
        "|You will be paid by my client upon safe arrival at the specified destination.",
        true)
    return "description_presented"
end

return BodyguardEscort
