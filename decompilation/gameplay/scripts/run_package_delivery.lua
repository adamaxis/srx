-- Courier contracts track the package and pay only after safe delivery.
local PackageDelivery = {}

function PackageDelivery.prepare_offer(context)
    local support = game.modules.run_support
    local route = support.choose_contract_route(context)
    local offer = game.modules.run_support.new_offer(
        "package_delivery",
        "courier",
        "deliver_package_to_destination",
        "package_delivered_safely",
        route)
    game.runs.registry.current_offer = offer
    return offer.id
end

function PackageDelivery.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "package_delivery")
    local route = offer.route
    game.ui.say_text(
        "Go to " .. route.origin.location_name .. " " ..
        route.origin.area_suffix ..
        "|A contact there will give you a package." ..
        "|Take it to " .. route.destination.location_name .. " " ..
        route.destination.area_suffix ..
        "|You will be paid upon safe delivery of the package.", true)
    return "description_presented"
end

return PackageDelivery
