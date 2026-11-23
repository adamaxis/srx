local E = require("enums")

local kSalishWestExitApproachX = 768
local kSalishWestExitApproachY = 21760
local kWestExitInputFrames = 180
local kSceneTransitionFrames = 180

local function enter_salish_cave(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SALISH_SHIDHE, kSalishWestExitApproachX,
        kSalishWestExitApproachY)
    assert(placed, placement_error or "Salish west-exit placement failed")
    local entered, movement_error = host.hold_until_modal(
        "LEFT", kWestExitInputFrames, E.Scene.CAVE_SCENE_45)
    assert(entered, movement_error or
        "physical Salish west exit did not reach Cave Scene 45")
    assert(host.wait_for_scene(E.Scene.CAVE_SCENE_45,
        kSceneTransitionFrames),
        "Salish west exit did not load Cave Scene 45")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "take_salish_west_exit_to_cave_45_b25",
            milestone = 25,
            expected = { scene = E.Scene.CAVE_SCENE_45 },
            run = function(host)
                enter_salish_cave(host)
                return "walked the live Salish west exit into Cave Scene 45"
            end,
        },
    },
}
