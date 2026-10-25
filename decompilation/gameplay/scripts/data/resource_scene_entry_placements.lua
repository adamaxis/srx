-- Eight-word ordinary-entry profiles selected by ROM 0x169E6 and consumed
-- at 0x2B78..0x2BD0: camera x/y, then Joshua and two runner position pairs.
-- 0x16950 selects every resource scene from 0x15 through 0x2B. Values below
-- were re-read from the US ROM placement pointers named on each row.
local E = require("enums")

return {
    [E.Scene.RESOURCE_ACTIVITY_21] = { -- ROM placement words at 0x1D750A
        camera_x = 0x3C00, camera_y = 0x4100,
        runner_positions = {
            { x = 0x4600, y = 0x4600 },
            { x = 0x4600, y = 0x4800 },
            { x = 0x4600, y = 0x4A00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_22] = { -- ROM placement words at 0x1D7BC0
        camera_x = 0x2000, camera_y = 0x3600,
        runner_positions = {
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_23] = { -- ROM placement words at 0x1D82E0
        camera_x = 0x2000, camera_y = 0x3600,
        runner_positions = {
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_24] = { -- ROM placement words at 0x1D7BC0
        camera_x = 0x2000, camera_y = 0x3600,
        runner_positions = {
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_25] = { -- ROM placement words at 0x1D8B5C
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
            { x = 0x4600, y = 0x4D00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_26] = { -- ROM placement words at 0x1D9916
        camera_x = 0x4400, camera_y = 0x2E00,
        runner_positions = {
            { x = 0x4C00, y = 0x3300 },
            { x = 0x4C00, y = 0x3500 },
            { x = 0x4C00, y = 0x3700 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_27] = { -- ROM placement words at 0x1DA002
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_28] = { -- ROM placement words at 0x1DA81A
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_29] = { -- ROM placement words at 0x1DA81A
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_30] = { -- ROM placement words at 0x1DAFE0
        camera_x = 0x1600, camera_y = 0x4400,
        runner_positions = {
            { x = 0x1F00, y = 0x4900 },
            { x = 0x1F00, y = 0x4B00 },
            { x = 0x1F00, y = 0x4D00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_31] = { -- ROM placement words at 0x1DB5CA
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_32] = { -- ROM placement words at 0x1DBBD4
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_33] = { -- ROM placement words at 0x1DBBD4
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_34] = { -- ROM placement words at 0x1DB5CA
        camera_x = 0x3C00, camera_y = 0x4600,
        runner_positions = {
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
            { x = 0x4600, y = 0x4B00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_35] = { -- ROM placement words at 0x1DC34E
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x3A00 },
            { x = 0x3B00, y = 0x3C00 },
            { x = 0x3B00, y = 0x3E00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_36] = { -- ROM placement words at 0x1DCB08
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_37] = { -- ROM placement words at 0x1DCB08
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_38] = { -- ROM placement words at 0x1DD212
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_39] = { -- ROM placement words at 0x1DD988
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_40] = { -- ROM placement words at 0x1DDE12
        camera_x = 0x3200, camera_y = 0x3600,
        runner_positions = {
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
            { x = 0x3B00, y = 0x4000 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_41] = { -- ROM placement words at 0x1DE360
        camera_x = 0x0000, camera_y = 0x4780,
        runner_positions = {
            { x = 0x0880, y = 0x4B00 },
            { x = 0x0880, y = 0x4D00 },
            { x = 0x0880, y = 0x4F00 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_42] = { -- ROM placement words at 0x1DE984
        camera_x = 0x2800, camera_y = 0x3200,
        runner_positions = {
            { x = 0x3200, y = 0x3700 },
            { x = 0x3200, y = 0x3700 },
            { x = 0x3200, y = 0x3700 },
        },
    },
    [E.Scene.RESOURCE_ACTIVITY_43] = { -- ROM placement words at 0x1DEEFE
        camera_x = 0x2800, camera_y = 0x3200,
        runner_positions = {
            { x = 0x3200, y = 0x3700 },
            { x = 0x3200, y = 0x3700 },
            { x = 0x3200, y = 0x3700 },
        },
    },
}
