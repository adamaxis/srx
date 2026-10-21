-- Native presentation timing recovered from the US ROM and calibrated against
-- the independent 30fps longplay. The runtime consumes named timing and
-- projection facts; it does not walk ROM commands or emulate VDP writes.
--
-- 0x177FC performs the 61-VBlank HUD setup before 0x2679E selects one of the
-- two entry streams. The nonzero-node stream publishes 56 states over 239
-- VBlanks; the zero-node stream publishes 56 over 179 VBlanks. Projection
-- bounds below are measured green-model pixel extents from Section 21 frames
-- 004730..004830. They are visual calibration, not ROM-derived dimensions.
return {
    setup_vblanks = 61,
    primary = {
        duration_vblanks = 179,
        published_at_vblank = {
            3, 6, 9, 13, 16, 19, 23, 26, 29, 33, 36, 39, 43, 46,
            49, 53, 56, 59, 63, 66, 69, 73, 76, 79, 82, 85, 88, 91,
            94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127,
            130, 133, 136, 139, 142, 145, 148, 151, 154, 157, 161, 165,
            169, 173, 176, 179,
        },
    },
    compact = {
        duration_vblanks = 239,
        published_at_vblank = {
            3, 6, 9, 13, 17, 21, 24, 27, 31, 34, 37, 41, 44, 47,
            51, 54, 57, 61, 64, 67, 71, 74, 77, 81, 84, 87, 91, 94,
            97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127, 130,
            133, 136, 139, 142, 145, 148, 151, 154, 157, 160, 191, 194,
            197, 200, 236, 239,
        },
        model_id = 2,
        palette_theme = 2,
        source_green_bounds = { x = 8, y = 5, width = 81, height = 73 },
        projection_samples = {
            { vblank = 40, x = 159, y = 100, width = 3, height = 3 },
            { vblank = 60, x = 156, y = 99, width = 8, height = 6 },
            { vblank = 80, x = 154, y = 93, width = 14, height = 15 },
            { vblank = 100, x = 150, y = 88, width = 22, height = 21 },
            { vblank = 120, x = 148, y = 82, width = 26, height = 27 },
            { vblank = 140, x = 144, y = 74, width = 32, height = 36 },
            { vblank = 160, x = 140, y = 67, width = 40, height = 43 },
            { vblank = 180, x = 136, y = 60, width = 46, height = 51 },
            { vblank = 200, x = 130, y = 53, width = 58, height = 57 },
            { vblank = 220, x = 126, y = 45, width = 62, height = 62 },
            { vblank = 239, x = 122, y = 38, width = 74, height = 67 },
        },
    },
}
