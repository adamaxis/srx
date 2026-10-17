-- ROM scene-2/event-7 service 22. The host calls the decision before the
-- generic area-event route; this script owns the conditional narrative and
-- reports the completed story fact through the constrained game.state proxy.
local E = require("enums")
local StoryFlags = require("story_flags")
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
    -- 0x53A78 publishes trigger 22 in FFDD64 before calling handler 0x545B6.
    StoryFlags.begin_trigger(E.StoryFlag.LANDMARK_MAP_TAKEN)
    if RedmondBarrens.should_run_vigore_search() ~= "run" then
        return "skip"
    end

    game.ui.say("redmond.vigore-search.page-0")
    game.ui.say("redmond.vigore-search.page-1")
    game.ui.say("redmond.vigore-search.page-2")
    -- 0x5463C: the handler's jsr 0x2196C after the conversation marks
    -- FFFC1E bit FFDD64 (22, LANDMARK_MAP_TAKEN) [C].
    StoryFlags.finish_trigger(E.StoryFlag.LANDMARK_MAP_TAKEN)
    game.state.vigore_search_completed = true
    return "completed"
end

return RedmondBarrens
