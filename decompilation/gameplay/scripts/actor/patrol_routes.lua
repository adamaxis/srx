-- Actor patrol identity lookup over the extracted source route data.
local routes = require("data.actor_patrol_routes")
local R = {}

function R.for_actor(actor_id)
    if type(actor_id) ~= "string" or actor_id == "" then
        return nil
    end
    return routes[actor_id]
end

function R.has_route(actor_id)
    return R.for_actor(actor_id) ~= nil
end

return R
