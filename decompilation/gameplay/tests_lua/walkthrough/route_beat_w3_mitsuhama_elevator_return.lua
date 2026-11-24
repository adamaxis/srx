local E = require("enums")

local kLiftX = 14336
local kLiftY = 1536
local kFloorCount = 4
local kSelectorFeedbackFrames = 10
local kAnimationFramesPerFloor = 17 * 8

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Mitsuhama return elevator advance failed")
end

return {
    steps = {
        {
            id = "w3_mitsuhama_elevator_return_to_scene_30",
            milestone = 4,
            expected = { scene = E.Scene.RESOURCE_ACTIVITY_30 },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_34, kLiftX, kLiftY)
                assert(placed, placement_error or
                    "Scene-34 lift setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open the Scene-34 Mitsuhama lift")
                for _ = 1, kFloorCount do
                    tap(host, "DOWN")
                    advance(host, kSelectorFeedbackFrames)
                end
                tap(host, "A")
                advance(host, kFloorCount * kAnimationFramesPerFloor)
                tap(host, "START")
                assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_30, 180),
                    "Scene-34 elevator did not return to activity 30")
                return "used the live lift selector and returned to activity 30"
            end,
        },
    },
}
