local Events = {}

function Events.walk_twice()
    local first_walk = game.actor.walk_direction("player", "north")
    if not first_walk then
        return
    end
    return game.actor.walk_direction("player", "north_east")
end

function Events.service_roundtrip()
    local result = game.services.call("inventory", "collect")
    if result == "accepted" then
        return "item-collected"
    end
    return "item-unavailable"
end

return Events
