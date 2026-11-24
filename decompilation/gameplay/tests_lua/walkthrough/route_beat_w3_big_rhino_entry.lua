local E = require("enums")

-- ROM record 98 at 0x18ABAA, event-table row 5, routes to Scene 11 [C].
local kBigRhinoExteriorRecord98X = 9984
local kBigRhinoExteriorRecord98Y = 9472

return {
    steps = {
        {
            id = "w3_enter_big_rhino",
            milestone = 1,
            expected = {
                scene = E.Scene.BIG_RHINO,
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT,
                    kBigRhinoExteriorRecord98X,
                    kBigRhinoExteriorRecord98Y)
                assert(placed, placement_error or
                    "Big Rhino exterior setup placement failed")
                local entered, movement_error = host.hold_until_modal(
                    "UP", 600, E.Scene.BIG_RHINO)
                assert(entered, movement_error or
                    "real UP did not select Scene-3 record 98")
                assert(host.wait_for_scene(E.Scene.BIG_RHINO, 180),
                    "Big Rhino event-table row 5 did not load Scene 11")
                return "real UP selected Scene-3 record 98 and entered the Big Rhino"
            end,
        },
    },
}
