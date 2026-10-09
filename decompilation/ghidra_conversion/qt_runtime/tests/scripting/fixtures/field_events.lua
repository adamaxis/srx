local Events = {}

function Events.walk_twice()
    local first_walk = game.actor.walk_heading("player", 0)
    if not first_walk then
        return
    end
    return game.actor.walk_heading("player", 1)
end

return Events
