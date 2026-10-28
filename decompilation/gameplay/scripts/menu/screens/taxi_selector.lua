-- The taxi selector shares the source cab-map artwork. Lua owns focus and the
-- confirmed destination; the host only performs the scene transition.
local E = require("enums")
local Taxi = require("data.taxi_destinations")
local M = { window = E.WindowPlane.OFF }
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local CONFIRM = P.HELD_A | P.HELD_B | P.HELD_START
local ANIMATION_FRAMES = 10

local function wrap(value, delta)
    return (value + delta) % #Taxi.cursor_to_region
end

function M.new(args)
    args = args or {}
    local region = args.current_region
    local selection = type(region) == "number" and
                      Taxi.region_to_cursor[region + 1] or nil
    assert(selection ~= nil, "taxi selector requires a known field region")
    return {
        row = selection,
        selection = Taxi.cursor_to_region[selection + 1],
        page = 0,
        animation_frames = ANIMATION_FRAMES,
        caption_visible = false,
        title = "Call Taxi",
        text = "",
        labels = {},
    }
end

function M.enter(st, ctx)
    ctx.redraw(R.FULL)
end

function M.input(st, events, ctx)
    if st.animation_frames > 0 then
        st.animation_frames = st.animation_frames - 1
        if st.animation_frames % 2 == 0 and st.page < 4 then
            st.page = st.page + 1
            ctx.redraw(R.FULL)
        end
        if st.animation_frames == 0 then
            st.caption_visible = true
            ctx.redraw(R.FULL)
        end
        return
    end
    -- ROM 0x134D2 samples FFDD22 after the outline draw. The loop at
    -- 0x1360C/0x1362E reads its held low byte: Up/Right decrement the cursor;
    -- Down/Left increment it. These are held controls, not edge-only menu
    -- navigation events.
    if events & (P.HELD_UP | P.HELD_RIGHT) ~= 0 then
        st.row = wrap(st.row, -1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        return
    end
    if events & (P.HELD_DOWN | P.HELD_LEFT) ~= 0 then
        st.row = wrap(st.row, 1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        return
    end
    -- ROM 0x134D2 routes Start through 0x10AF0; A/B use 0x20714. Both
    -- branches continue through the selected destination transition.
    if events & CONFIRM == 0 then return end

    local region = Taxi.cursor_to_region[st.row + 1]
    -- ROM 0x20714 runs the cabbie dialogue before 0x136B6 can travel [C].
    if ctx.game.begin_taxi_cabbie(region) then
        return { transition = T.EXIT }
    end
end

return M
