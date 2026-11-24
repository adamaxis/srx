local E = require("enums")

local kMitsuhamaLiftX = 14592
local kMitsuhamaLiftY = 17152
local kFloorSelectionCount = 4
local kNavigationFeedbackFrames = 10
local kAnimationFramesPerFloor = 17 * 8

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Mitsuhama elevator advance failed")
end

return {
    steps = {
        {
            id = "w3_mitsuhama_elevator_to_floor_4",
            milestone = 1,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_34,
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_30,
                    kMitsuhamaLiftX, kMitsuhamaLiftY)
                assert(placed, placement_error or
                    "Scene-30 lift setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open the Mitsuhama lift")
                for _ = 1, kFloorSelectionCount do
                    tap(host, "UP")
                    -- The selector ignores navigation for nine feedback frames
                    -- before polling again (ROM timing documented by the
                    -- elevator selector runtime).
                    advance(host, kNavigationFeedbackFrames)
                end
                tap(host, "A")
                -- Each adjacent floor step runs 17 frames at an 8-frame
                -- cadence; wait for all selected floors to become current.
                advance(host, kFloorSelectionCount * kAnimationFramesPerFloor)
                tap(host, "START")
                assert(host.wait_for_scene(
                    E.Scene.RESOURCE_ACTIVITY_34, 180),
                    "Mitsuhama elevator did not reach floor 4 / activity 34")
                return "selected floor 4 through the real lift menu and reached activity 34"
            end,
        },
    },
}
