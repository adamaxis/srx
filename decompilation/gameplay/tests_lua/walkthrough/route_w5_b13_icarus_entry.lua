local E = require("enums")

local kIcarusEntranceWorldX = 27392
local kIcarusEntranceWorldY = 10496
local kSceneTransitionFrames = 180

local function enter_icarus(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE, kIcarusEntranceWorldX,
        kIcarusEntranceWorldY)
    assert(placed, placement_error or "Icarus entrance placement failed")
    local entered, movement_error = host.hold_until_modal(
        "UP", kSceneTransitionFrames, E.Scene.ICARUS_DESCENDING)
    assert(entered, movement_error or "Icarus entrance did not change scene")
    assert(host.wait_for_scene(E.Scene.ICARUS_DESCENDING, kSceneTransitionFrames),
        "Icarus entrance did not reach Scene 12")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "enter_icarus_descending_from_downtown",
            milestone = 13,
            expected = { scene = E.Scene.ICARUS_DESCENDING },
            run = function(host)
                enter_icarus(host)
                return "entered Scene 12 through its live Scene-0 door"
            end,
        },
    },
}
