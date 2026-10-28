-- The taxi selector shares the source cab-map artwork. Lua owns focus and the
-- confirmed destination; the host only performs the scene transition.
local E = require("enums")
local Taxi = require("data.taxi_destinations")
local M = { window = E.WindowPlane.OFF }
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
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
    -- 0x195C stores new button edges in FFDD22's high byte and held state in
    -- its low byte. The selector's byte-sized BTSTs at 0x135FC and 0x1360C
    -- test the high-byte edges: Up/Right decrement once, then Down/Left
    -- increment once (0x13618..0x13662).
    local moved = false
    if events & (P.UP | P.RIGHT) ~= 0 then
        st.row = wrap(st.row, -1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        moved = true
    end
    if events & (P.DOWN | P.LEFT) ~= 0 then
        st.row = wrap(st.row, 1)
        st.selection = Taxi.cursor_to_region[st.row + 1]
        st.page = 0
        st.animation_frames = ANIMATION_FRAMES
        st.caption_visible = false
        ctx.redraw(R.FULL)
        moved = true
    end

    -- Directions are processed before buttons. A and B both enter 0x20714;
    -- C calls the shared 0x391A runner-switch action; Start takes the separate
    -- 0x10AF0 exit path. A/B therefore win over C/Start in combined input.
    if events & (P.A | P.B) ~= 0 then
        local region = Taxi.cursor_to_region[st.row + 1]
        if ctx.game.begin_taxi_cabbie(region) then
            return { transition = T.EXIT }
        end
        return
    end
    if events & P.C ~= 0 then
        ctx.game.reselect_leader()
    end
    if events & P.START ~= 0 then
        return { transition = T.EXIT }
    end
    if moved then return end
end

return M
