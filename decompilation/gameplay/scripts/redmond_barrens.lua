-- ROM scene-2/event-7 service 22. The host calls the decision before the
-- generic area-event route; this script owns the conditional narrative and
-- reports the completed story fact through the constrained game.state proxy.
local RedmondBarrens = {}

function RedmondBarrens.should_run_vigore_search()
    if game.location.type ~= "field_map" or
       game.location.id ~= "redmond_barrens" then
        return "skip"
    end
    if not game.state.vigore_search_pending or
       game.state.vigore_search_consumed then
        return "skip"
    end
    return "run"
end

function RedmondBarrens.run_vigore_search()
    if RedmondBarrens.should_run_vigore_search() ~= "run" then
        return "skip"
    end

    game.ui.say("redmond.vigore-search.page-0")
    game.ui.say("redmond.vigore-search.page-1")
    game.ui.say("redmond.vigore-search.page-2")
    game.state.vigore_search_completed = true
    return "completed"
end

return RedmondBarrens
