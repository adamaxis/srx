-- Host animation tuning in rendered frames per update or map unit.
-- Values preserve the observed ROM cadence without running its fixed-point
-- rate arithmetic in gameplay code [C: 0x5FDC, 0x6092].
return {
    runner_walk = {
        frames_per_update = 0.15625,
    },
    moving_actor_walk = {
        distance_bands = {
            {
                max_distance_per_update = 21,
                frames_per_map_unit = 0.0068359375,
            },
            {
                frames_per_map_unit = 0.00390625,
            },
        },
    },
    gargoyle_walk = {
        default_frames_per_update = 0.09375,
        frame_overrides = {
            [2] = 0.03515625,
        },
    },
}
