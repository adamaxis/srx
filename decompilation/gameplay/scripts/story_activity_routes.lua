-- Route activities: a story beat requests semantic content by activity id.
-- The host resolves that id against imported actor data, publishes the
-- activity at the next field-scene entry, then calls complete_<name> after
-- every published member completes its behavior transition.
--
-- Host primitive: field.queue_activity_<activity id>
--   -> "queued" or nil (another activity is pending/active)
local StoryActivityRoutes = {}

StoryActivityRoutes.ACTIVITIES = {
    -- ROM 0x225D2 queues the Mako activity; 0x43B6 publishes its four
    -- extracted actor records. The import map owns all source selectors.
    club_penumbra_mako = {
        callback = { module = "club_penumbra", fn = "present_mako_activity_result" },
    },
}

function StoryActivityRoutes.request(activity_id)
    assert(StoryActivityRoutes.ACTIVITIES[activity_id],
           "unknown story activity")
    return game.services.call("field", string.format(
        "queue_activity_%s", activity_id))
end

for activity_id, activity in pairs(StoryActivityRoutes.ACTIVITIES) do
    StoryActivityRoutes["complete_" .. activity_id] = function()
        return require(activity.callback.module)[activity.callback.fn]()
    end
end

return StoryActivityRoutes
