-- Presentation and run-state changes for the 0x1F16A extraction-result
-- callback. C++ supplies the typed callback result and owns field actor cleanup.
local E = require("enums")
local StoryState = require("story_state")

local Results = {}

local function advance_renraku_escalation_if_eligible()
    local registry = game.runs.registry
    local offer = type(registry) == "table" and registry.active_offer or nil
    if type(offer) ~= "table" or
       (offer.source_run_type ~= E.RunType.PACKAGE_RECOVERY and
        offer.source_run_type ~= E.RunType.EMPLOYEE_RECRUITMENT) then
        return
    end

    local target = type(offer.route) == "table" and offer.route.target or nil
    if type(target) == "table" and target.area_id == 5 and
       target.location_id == 6 then
        -- 0x1F16A advances FFFBFB through 0x1F1E0 only for run type 4/5
        -- when the selected target is FFFBEE/FFFBEF = 5/6. The shared helper
        -- preserves the source upper bound of eight.
        assert(StoryState.add_capped(E.StoryByte.STRIKE_TEAM, 1),
            "world-activity Strike Team counter is unresolved")
    end
end

function Results.present_extraction_success()
    advance_renraku_escalation_if_eligible()
    -- ROM descriptor 0xF3D64 has the shared MORE continuation.
    game.ui.say_text(
        "<As you exit the building a Northrup PRC-42 Wasp descends to land " ..
        "in front of you.\n\nThe typically one seated copter has been " ..
        "modified to include a second.>", true)
    -- ROM descriptor 0xF3D78 is the following no-options page.
    game.ui.say_text(
        "<The rigger pilot straps in your man, gives you the thumbs-up, and " ..
        "sleekly disappears into the sky.\n\nYou calmly, so as not to " ..
        "attract attention, stroll off to collect your payment.>", false)
    return "extraction_succeeded"
end

function Results.present_extraction_failure()
    advance_renraku_escalation_if_eligible()
    game.runs.registry.clear_active_offer_for_world_activity_failure()
    -- ROM descriptor 0xF3D80 is a one-page, no-options cancellation result.
    game.ui.say_text(
        "<You see a rigger pilot, your pick-up man, swing his copter in for " ..
        "a landing.\n\nA look of disappointment touches his face as he " ..
        "nullifies your contract on his console.\n\nA run that gets fragged " ..
        "all to drek isn't going to help your rep much!>", false)
    return "extraction_failed"
end

return Results
