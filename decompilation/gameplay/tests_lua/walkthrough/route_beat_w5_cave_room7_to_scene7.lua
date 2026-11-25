local E = require("enums")

assert(E.Scene.CAVE_SCENE_45 ~= nil, "Scene-45 enum is required")
assert(E.Scene.SCENE_7 ~= nil, "Scene-7 enum is required")

-- ROM record 0x1D4E5E: Scene 45 event 10, bounds
-- x=11264..11775 and y=14848..15359 [C].
local kRoom7SouthExitX = 11520
local kRoom7SouthExitY = 15104
local kExitInputFrames = 600
local kSceneTransitionFrames = 180
local kScene7ArrivalX = 2816 -- 0x0B00, world_links_data.lua room 7 [C]
local kScene7ArrivalY = 1920 -- 0x0780, world_links_data.lua room 7 [C]

return {
    steps = {
        {
            id = "prepare_rom_room7_south_exit",
            milestone = 25,
            expected = { scene = E.Scene.CAVE_SCENE_45 },
            run = function(host)
                -- Record 0x1D4E5E checks FFFC10=7. Leave FFDF40 at its
                -- natural zero; the cave exit dispatcher rejects nonzero
                -- FFDF40 at ROM 0x1651E.
                local room, room_error =
                    host.set_field_transition_context_for_setup(
                        E.FieldTransitionContext.CAVE_ROOM_7)
                assert(room, room_error or "ROM cave room setup failed")
                return "seeded the ROM room context for Scene-45 room 7"
            end,
        },
        {
            id = "walk_scene45_room7_south_exit_to_scene7",
            milestone = 25,
            expected = { scene = E.Scene.SCENE_7 },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.CAVE_SCENE_45,
                    kRoom7SouthExitX, kRoom7SouthExitY)
                assert(placed, placement_error or
                    "Scene-45 room-7 south-exit placement failed")
                local entered, movement_error = host.hold_until_modal(
                    "DOWN", kExitInputFrames, E.Scene.SCENE_7)
                assert(entered, movement_error or
                    "real DOWN did not trigger the Scene-45 room-7 south exit")
                assert(host.wait_for_scene(E.Scene.SCENE_7,
                    kSceneTransitionFrames),
                    "Scene-45 room-7 south exit did not load Scene 7")
                local arrival = host.snapshot()
                assert(arrival.x == kScene7ArrivalX and
                       arrival.y == kScene7ArrivalY,
                    "Scene-45 room-7 south exit missed the ROM arrival " ..
                    "(0x0B00,0x0780)")
                return "real DOWN from Scene-45 room 7 reached Scene 7 at the ROM spawn"
            end,
        },
    },
}
