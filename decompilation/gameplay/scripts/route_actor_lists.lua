-- Route actor lists: a story beat queues a source activity list that the
-- host publishes when the party next enters `consume_scene`; once all
-- `transitions` actor behavior transitions finish, the host runs
-- RouteActorLists.complete_<name>, which calls the list's callback.
--
-- Host primitive:
--   field.queue_route_actor_list_<consume>_<source>_<list hex>_<transitions>_<name>
--     -> "queued" or nil (another list is pending/active)
local RouteActorLists = {}

RouteActorLists.LISTS = {
    -- Club Penumbra Mako [C]: the Club finalizer enters Scene 17; ROM 0x43B6
    -- drains list 0x1E628 (recorded in Scene 47's activity table) and seeds
    -- FFE177 with its four actor transitions.
    club_penumbra_mako = {
        consume_scene = 17, source_scene = 47, list = 0x1E628, transitions = 4,
        callback = { module = "club_penumbra", fn = "present_mako_activity_result" },
    },
}

function RouteActorLists.queue(name)
    local list = assert(RouteActorLists.LISTS[name], "unknown route actor list")
    return game.services.call("field", string.format(
        "queue_route_actor_list_%d_%d_%x_%d_%s", list.consume_scene,
        list.source_scene, list.list, list.transitions, name))
end

for name, list in pairs(RouteActorLists.LISTS) do
    RouteActorLists["complete_" .. name] = function()
        return require(list.callback.module)[list.callback.fn]()
    end
end

return RouteActorLists
