local E = require("enums")

-- Scene-45 trigger 0x1D4E4A is event 8 at x=0..511, y=9728..10239.
local kRoom5NorthExitX = 256
local kRoom5NorthExitY = 9984
local kExitInputFrames = 600
local kSceneTransitionFrames = 180
local kScene44NorthArrivalX = 24064 -- 0x5E00, world_links_data.lua room 5
local kScene44NorthArrivalY = 8704 -- 0x2200, world_links_data.lua room 5

return {
    steps = {
        {
            id = "prepare_rom_room5_north_exit",
            milestone = 25,
            expected = { scene = E.Scene.CAVE_SCENE_45 },
            run = function(host)
                -- ROM 0x1D4E4A requires FFFC10=5. Leave FFDF40 at its
                -- natural zero: ROM 0x1651E rejects event 8 when nonzero.
                local room, room_error =
                    host.set_field_transition_context_for_setup(
                        E.FieldTransitionContext.CAVE_ROOM_5)
                assert(room, room_error or "ROM cave room setup failed")
                return "seeded the ROM room context for Scene-45 room 5"
            end,
        },
        {
            id = "walk_scene45_room5_north_exit_to_scene44",
            milestone = 25,
            expected = { scene = E.Scene.CAVE_SCENE_44 },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.CAVE_SCENE_45,
                    kRoom5NorthExitX, kRoom5NorthExitY)
                assert(placed, placement_error or
                    "Scene-45 room-5 north-exit placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kExitInputFrames, E.Scene.CAVE_SCENE_44)
                assert(opened, movement_error or
                    "real UP did not trigger the Scene-45 room-5 north exit")
                assert(host.wait_for_scene(E.Scene.CAVE_SCENE_44,
                    kSceneTransitionFrames),
                    "Scene-45 room-5 north exit did not load Scene 44")
                local arrival = host.snapshot()
                assert(arrival.x == kScene44NorthArrivalX and
                       arrival.y == kScene44NorthArrivalY,
                    "Scene-45 room-5 north exit did not use ROM arrival " ..
                    "(0x5E00,0x2200)")
                return "real UP from Scene-45 room 5 reached Scene 44 at the ROM spawn"
            end,
        },
    },
}
