-- Cold-boot Genesis Plus GX route for playtest #34c.
-- This is controller input only: no RAM writes, state loads, debug teleports,
-- or scene-load shortcuts. Frames are zero-based completed-frame boundaries.
local E = require("enums")

return {
    frame_count = 10050,
    actions = {
        { frame = 4200, pad = E.Pad.START, pressed = true },
        { frame = 4201, pad = E.Pad.START, pressed = false },
        { frame = 4330, pad = E.Pad.A, pressed = true }, -- New Game
        { frame = 4331, pad = E.Pad.A, pressed = false },
        { frame = 4440, pad = E.Pad.A, pressed = true }, -- Samurai
        { frame = 4441, pad = E.Pad.A, pressed = false },
        { frame = 8400, pad = E.Pad.A, pressed = true }, -- Stoker arrival page
        { frame = 8401, pad = E.Pad.A, pressed = false },
        { frame = 8480, pad = E.Pad.A, pressed = true }, -- ask for a room
        { frame = 8481, pad = E.Pad.A, pressed = false },
        { frame = 8600, pad = E.Pad.B, pressed = true }, -- leave Stoker's
        { frame = 8601, pad = E.Pad.B, pressed = false },
        { frame = 9300, pad = E.Pad.A, pressed = true }, -- close leave reply
        { frame = 9301, pad = E.Pad.A, pressed = false },
        -- The return is still in its ROM fade at 9400. Wait until field
        -- input is live, then walk around the motel frontage toward the
        -- first Scene-2 pedestrian route.
        { frame = 9600, pad = E.Pad.LEFT, pressed = true },
        { frame = 9654, pad = E.Pad.LEFT, pressed = false },
        { frame = 9654, pad = E.Pad.UP, pressed = true },
        { frame = 9717, pad = E.Pad.UP, pressed = false },
        { frame = 9717, pad = E.Pad.DOWN, pressed = true },
        { frame = 9761, pad = E.Pad.DOWN, pressed = false },
        { frame = 9761, pad = E.Pad.LEFT, pressed = true },
        { frame = 9847, pad = E.Pad.LEFT, pressed = false },
    },
}
