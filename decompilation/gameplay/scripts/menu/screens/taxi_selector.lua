-- The taxi selector shares the source cab-map artwork. Lua owns focus and the
-- confirmed destination; the host only performs the scene transition.
local E = require("enums")
local Taxi = require("data.taxi_destinations")
local M = { window = E.WindowPlane.OFF }
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local CONFIRM = P.A | P.B
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
    if events & (P.UP | P.LEFT) ~= 0 then
        st.row = wrap(st.row, -1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        return
    end
    if events & (P.DOWN | P.RIGHT) ~= 0 then
        st.row = wrap(st.row, 1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        return
    end
    -- ROM 0x134D2 routes Start through 0x10AF0 and then finalizes the
    -- selected mode. A/B use 0x20714's mode-exit page; both paths can travel.
    if events & P.START == 0 and events & CONFIRM == 0 then return end

    local region = Taxi.cursor_to_region[st.row + 1]
    -- ROM 0x20714 runs the cabbie dialogue before 0x136B6 can travel [C].
    if ctx.game.begin_taxi_cabbie(region) then
        return { transition = T.EXIT }
    end
end

return M
