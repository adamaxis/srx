-- Content-owned map event rules. Coordinates are world coordinates in the
-- native scene model; this rule corresponds to the known New Game spawn.
return {
    map_events = {
        {
            id = 0x00020001,
            scene = 2,
            trigger = "scene_entry",
            scope = {
                min_x = 0x3CE0,
                max_x = 0x3CE0,
                min_y = 0x2D00,
                max_y = 0x2D00,
            },
            requires = { "new_game_opening_completed" },
            ["repeat"] = "once_per_session",
            action = "start_new_game_opening_walk",
        },
    },
}
