-- cyberdeck.lua: Pocket Secretary row 2 -> 0x18150 -> deck loop 0x1822C, ROM-traced [C unless marked].
-- 0x18150: FF018F b0 == 0 -> 0xE43D1 "You do not have a cyberdeck!!" (Pocket Secretary handles it). Else host
--   deck_enter (window off, FFF0B7 = 0, screen 0xE2E3A, 0x177CC, deck +0x0C = 10), loop, deck_leave (field restore).
-- 0x1822C: saves/restores FFF0B7 + FFDD08, clears FFF0B1. 4x4 grid FFF0D0 col / FFF0D1 row, 0..3 wrap, silent;
--   cursor FFD986 x = col*0x20+0x8C, y = row*0x28+0x94; 0x196F2 per frame (host tick).
--   A -> FFF0B7 = FF, B -> 0, C -> 1 (0x18630..), then dispatch 0x1842E:
--     row < 3 -> program slot screen 0x19238 (screens/deck_screens.lua); row 3 col 3 -> pull the plug (only when FFF0B0 online);
--     row 3 col 0..2 -> table 0xE2DFE (0x18AA6, 0x18D1A, 0x19018; col 2 only online); FFF0AD set -> loop exits.
--   Start -> exit (d7 = -1).
local E = require("enums")
local D = require("menu.rules.deck")
local MX = require("menu.rules.matrix")
local MatrixSession = require("menu.matrix_session")
local LegacyCyberdeck = require("menu.screens.legacy").make(
    E.MenuScreen.CYBERDECK, E.WindowPlane.OFF)
local P, R, T, MM, S = E.Pad, E.MenuRegion, E.MenuTransition, E.MenuMessage, E.MenuSound
local M = { window = E.WindowPlane.OFF, PLUG_ROW = 3, PLUG_COL = 3, BLACK_IC = 8, BODY_TN = 5, STUN_LOSS = 3 }
M.GRID_LABELS = {
    D.PROGRAM_NAMES[0], D.PROGRAM_NAMES[1], D.PROGRAM_NAMES[2], D.PROGRAM_NAMES[3],
    D.PROGRAM_NAMES[4], D.PROGRAM_NAMES[5], D.PROGRAM_NAMES[6], D.PROGRAM_NAMES[7],
    D.PROGRAM_NAMES[8], D.PROGRAM_NAMES[9], D.PROGRAM_NAMES[10], D.PROGRAM_NAMES[11],
    "CyberInfo", "Storage", "Map", "Pull Plug",
}
local MODE = {
    [P.A] = E.MatrixDeckMode.ATTACK,
    [P.B] = E.MatrixDeckMode.BALANCED,
    [P.C] = E.MatrixDeckMode.DEFENSE,
} -- FFF0B7 (read by the 0x55F6C pool step, docs/18)

M.FUNCTION = { [0] = E.MenuScreen.DECK_CYBERINFO, E.MenuScreen.DECK_STORAGE, E.MenuScreen.DECK_MAP }   -- 0x18AA6, 0x18D1A, 0x19018
function M.cursor_xy(col, row) return col * 0x20 + 0x8C, row * 0x28 + 0x94 end

function M.new(args)
    args = args or {}
    if args.legacy_adapter then
        return {
            legacy_adapter = true,
            legacy_state = LegacyCyberdeck.new(args),
        }
    end
    return {
        col = 0,
        row = 0,
        title = "Cyberdeck",
        labels = M.GRID_LABELS,
        matrix_session = args.matrix_session == true,
        system_id = args.system_id,
        matrix_state = args.matrix_state,
    }
end
function M.enter(st, ctx)
    if st.legacy_adapter then
        LegacyCyberdeck.enter(st.legacy_state, ctx)
        return
    end
    st.saved = ctx.game.deck_enter()
    ctx.redraw(R.FULL)
end
local function online(st, ctx)
    return st.matrix_session and st.matrix_state ~= nil or
           MatrixSession.is_online(st.matrix_state) or
           ctx.game.matrix_online()
end
local function leave(st, ctx, res)
    ctx.game.deck_leave(st.saved)
    if st.matrix_session then
        MatrixSession.finish(st.matrix_state)
        -- ROM 0xE47A returns through field resume and rebuilds the Command
        -- page after a normal Matrix exit. Drop hidden Pocket Secretary and
        -- debug children as part of that rebuild.
        return { transition = T.REBUILD,
                 screen = E.MenuScreen.COMMAND }
    end
    return { transition = T.POP, result = res }
end
function M.input(st, events, ctx)
    if st.legacy_adapter then
        return LegacyCyberdeck.input(st.legacy_state, events, ctx)
    end
    local g = ctx.game
    g.deck_tick()                                                                        -- 0x196F2
    local col, row, moved = MX.step_deck_cursor(
        st.col, st.row,
        events & P.UP ~= 0, events & P.DOWN ~= 0,
        events & P.LEFT ~= 0, events & P.RIGHT ~= 0)
    if moved then st.col, st.row = col, row; st.text = nil; ctx.redraw(R.SUBMENU); return end
    if events & P.START ~= 0 then return leave(st, ctx) end
    local btn = events & P.A ~= 0 and P.A or events & P.B ~= 0 and P.B or events & P.C ~= 0 and P.C
    if not btn then return end
    g.set_deck_mode(MODE[btn])
    st.text = nil
    if st.row ~= M.PLUG_ROW then return { transition = T.PUSH, screen = E.MenuScreen.DECK_PROGRAM, args = { col = st.col, row = st.row } } end
    if st.col == M.PLUG_COL then
        if not online(st, ctx) then return end
        local node = MatrixSession.current_node(st.matrix_state) or g.matrix_node()
        local alert = MatrixSession.is_online(st.matrix_state) and
                      MatrixSession.alert(st.matrix_state) or
                      g.alert()
        local black = MX.ic_type(node, alert,
                                 MatrixSession.is_online(st.matrix_state) or
                                     g.in_alert(),
                                 MatrixSession.is_online(st.matrix_state) and 1 or
                                     g.map_seen()) == M.BLACK_IC and
                      g.in_combat() -- 0x19B40 == 8 && FFDF84
        return { transition = T.PUSH, screen = E.MenuScreen.PULL_PLUG,
                 args = { black_ic = black,
                          matrix_state = st.matrix_state } }
    end
    if st.col == 2 and not online(st, ctx) then return end
    return { transition = T.PUSH, screen = M.FUNCTION[st.col],
             args = { matrix_state = st.matrix_state } }                              -- 0xE2DFE[col]
end
function M.child_returned(st, child, res, ctx)
    if res.logged_out then return leave(st, ctx, { exit_secretary = true }) end          -- FFF0DC = FF
    if res.exit_deck then
        return leave(st, ctx, st.matrix_session and
            { exit_secretary = true } or {})                                            -- FFF0AD -> clr, d7 = 0, exit
    end
    st.text = type(res.text) == "string" and res.text or nil                           -- plain-rts text remains until redraw
    st.list = res.list
    ctx.redraw(R.FULL)                                                                   -- 0x1882E grid redraw
end

-- Pull the plug: 0xE438A (+0xE43B0 Black IC warning), Yes/No 0xE387E, FFF0DC default 1 = No; A/B/C confirm.
-- Yes + Black IC: Willpower (+0x7C) dice vs TN = IC rating (0x19BF6 d6) via 0xDBA.
--   0 successes -> physical -(TN >> 1) (0x17D90), sfx 0x18; alive -> back to the deck grid (NO logout); dead -> logout.
--   else Body (+0x77) dice vs 5; 0 successes -> stun -3 (min 1), sfx 0x18. Then logout.
-- Logout: FFF0DC = FF, 0x18776, 0x26A00 (sfx 0x29, FFF0B1 = FF).
local Plug = { window = E.WindowPlane.OFF }
function Plug.new(a)
    return {
        black_ic = a.black_ic,
        matrix_state = a.matrix_state,
        yes = false,
        row = 1,
        title = a.black_ic
            and "WARNING:|Black IC present!|Do you really want|to pull the plug?"
            or "Do you really want|to pull the plug?",
        labels = { "Yes", "No" },
    }
end
function Plug.enter(st, ctx) ctx.redraw(R.MESSAGE) end
function M.dumpshock(ctx, matrix_state)
    local g = ctx.game
    local party = g.party()
    local r = party[g.active_slot()]
    local node = MatrixSession.current_node(matrix_state) or g.matrix_node()
    local alert = MatrixSession.is_online(matrix_state) and
                  MatrixSession.alert(matrix_state) or
                  g.alert()
    local tn = MX.ic_rating(node, alert)                                -- 0x19BF6
    if g.roll_successes(r.attributes[5], tn) == 0 then
        r.physical = math.max(0, r.physical - (tn >> 1))
        g.commit_party(party)
        ctx.sound(S.DAMAGE)
        return r.physical == 0
    end
    if g.roll_successes(r.attributes[0], M.BODY_TN) == 0 then
        r.stun = r.stun - M.STUN_LOSS
        if r.stun < 1 then r.stun = 1 end
        g.commit_party(party)
        ctx.sound(S.DAMAGE)
    end
    return true
end
function Plug.input(st, events, ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        st.yes = not st.yes
        st.row = st.yes and 0 or 1
        ctx.redraw(R.MESSAGE)
        return
    end  -- [H toggle on 2 rows]
    if events & (P.A | P.B | P.C) == 0 then return end
    if not st.yes then return { transition = T.POP, result = {} } end
    if st.black_ic and not M.dumpshock(ctx, st.matrix_state) then
        return { transition = T.POP, result = {} }
    end
    MatrixSession.finish(st.matrix_state)
    ctx.sound(MX.LOGOUT_SOUND)
    return { transition = T.POP, result = { logged_out = true } }
end

return { Cyberdeck = M, PullPlug = Plug }
