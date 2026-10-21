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
local DeckPalette = require("menu.deck_palette")
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
M.GRID_DESCRIPTIONS = {
    'Attack is used to destroy ("crash") IC.|Attack becomes necessary when Masking fails.',
    "Slow reduces the IC's reactions, delaying attacks and alerts.|Slow does nothing against Trace IC.",
    "Degrade attempts to lower a Node's Security Rating.|Both the Node's & the IC's defenses become weakened.",
    "Rebound bounces attacks back at the attacker.|Rebound weakens slowly with each interception.",
    "Medic repairs damage taken by the Persona.|Seriously damaged Personas repair at a slower rate.",
    "Shield reduces damage from most attacks. Each hit it degrades.|Shield is useless against Black IC.",
    "Smoke creates electronic chaos.  ALL actions have added difficulty.|Smoke degrades every 4 seconds.",
    "Mirrors confounds IC and Deckers, decreasing their attack accuracy.|Mirrors degrades every 4 seconds.",
    "Sleaze allows the Persona to bypass the node without affecting it in any manner.",
    "Deception creates passcodes to fool Access & Gray IC.|Deception has no effect on Barrier or Black IC.",
    "Relocate leads any Trace IC on a wild goose chase.|If successful, the Trace ends immediately.",
    "Analyze scans the Node and its IC for information.|Multiple Analyzes may be required for hidden info.",
    "This selection allows you to view information on your current cyberdeck or matrix run.",
    "This selection allows you to see the items you have in storage.",
    "See where you are in the system and retreat to prior node if needed.",
    "An emergency or quick abort from cyberspace.|Some forms of IC may be able to block your jack out attempt and cause damage!",
}
local function update_description(st)
    st.description = M.GRID_DESCRIPTIONS[D.index(st.col, st.row) + 1]
end
local function update_program_details(st, record, program)
    st.runner_detail_labels = D.RUNNER_DETAIL_LABELS
    program = program or D.index(st.col, st.row)
    if program >= D.PROGRAMS then
        st.program_detail_values = nil
        return
    end
    local rating = record.ratings[program] or 0
    if rating == 0 then
        st.program_detail_values = nil
        return
    end
    local size = D.size(program, rating)
    st.program_detail_values = {
        tostring(size),
        tostring(D.free_memory(record)),
        tostring(record.load_io > 0 and D.load_steps(size, record.load_io) or 0),
    }
end
local function update_program_art_state(st, ctx)
    local record = ctx.game.deck_record()
    st.program_ratings = record.ratings
    st.active_programs = record.active

    local previous = st.program_slot_status
    local status = {}
    local changed = false
    local slots = ctx.game.deck_slots()
    for slot = 0, D.SLOTS - 1 do
        local entry = slots[slot]
        local value = entry and entry.status or 0
        status[slot] = value
        if previous == nil or previous[slot] ~= value then
            changed = true
        end
    end
    st.program_slot_status = status
    return changed, record
end
M.update_program_art_state = update_program_art_state

function M.prepare_program_child_surface(st, ctx)
    local _, record = update_program_art_state(st, ctx)
    local program = st.p
    st.deck_parent_program_name = D.PROGRAM_NAMES[program]
    st.program_detail_labels = D.PROGRAM_DETAIL_LABELS
    update_program_details(st, record, program)

    local run = MatrixSession.world_state(ctx.game).run
    if run == nil or (run.state or 0) >= 0 then
        st.runner_detail_lines = D.NO_ACTIVE_RUN_DETAILS
    else
        st.runner_detail_lines = nil
    end
end

local function service_program_loading(st, ctx)
    if not MatrixSession.is_online(st.matrix_state) then return false end
    local slots = ctx.game.deck_slots()
    local record = ctx.game.deck_record()
    if not MatrixSession.advance_program_loading(st.matrix_state, slots) then
        return false
    end
    ctx.game.deck_commit(record, slots)
    return true
end
local MODE = {
    [P.A] = E.MatrixDeckMode.ATTACK,
    [P.B] = E.MatrixDeckMode.BALANCED,
    [P.C] = E.MatrixDeckMode.DEFENSE,
} -- FFF0B7 (read by the 0x55F6C pool step, docs/18)
local RESULT_MODE = {
    [P.A] = E.MatrixModeResult.ATTACK,
    [P.B] = E.MatrixModeResult.BALANCED,
    [P.C] = E.MatrixModeResult.DEFENSE,
}

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
    local deck_palette = args.deck_palette or DeckPalette.new()
    local state = {
        col = 0,
        row = 0,
        title = "Cyberdeck",
        labels = M.GRID_LABELS,
        matrix_session = args.matrix_session == true,
        return_to_node_action = args.return_to_node_action == true,
        return_to_field = args.return_to_field == true,
        system_id = args.system_id,
        matrix_state = args.matrix_state,
        deck_palette = deck_palette,
        deck_palette_phase = DeckPalette.phase(deck_palette),
        program_detail_labels = D.PROGRAM_DETAIL_LABELS,
        storage_empty_message_lines = nil,
    }
    update_description(state)
    return state
end
local function tick_palette(st, ctx)
    local phase, changed = DeckPalette.tick(st.deck_palette)
    st.deck_palette_phase = phase
    if changed then ctx.redraw(R.FULL) end
end
function M.enter(st, ctx)
    if st.legacy_adapter then
        LegacyCyberdeck.enter(st.legacy_state, ctx)
        return
    end
    st.saved = ctx.game.deck_enter()
    -- ROM 0x187AA draws the twelve program icons only when FFFBBE+0x0E+p
    -- has a nonzero installed rating. Keep that typed gameplay data on the
    -- Lua screen state so presentation does not guess which icons exist.
    local _, record = update_program_art_state(st, ctx)
    update_program_details(st, record)
    local run = MatrixSession.world_state(ctx.game).run
    if run == nil or (run.state or 0) >= 0 then
        st.runner_detail_lines = D.NO_ACTIVE_RUN_DETAILS
    else
        st.runner_detail_lines = nil
    end
    update_description(st)
    if st.matrix_state then
        st.matrix_state.mode = E.MatrixModeResult.BALANCED
    end
    ctx.redraw(R.FULL)
end
local function online(st, ctx)
    return st.matrix_session and st.matrix_state ~= nil or
           MatrixSession.is_online(st.matrix_state) or
           ctx.game.matrix_online()
end
local function leave(st, ctx, res)
    ctx.game.deck_leave(st.saved)
    if st.matrix_state then st.matrix_state.mode = st.saved end
    if st.return_to_node_action and
       not (res and res.exit_secretary) then
        return { transition = T.POP, result = { matrix_resumed = true } }
    end
    if st.matrix_session and st.matrix_state and
       not (res and res.exit_secretary) then
        -- ROM 0xE49E returns from the first 0x181F8 deck loop into
        -- 0x173F2/0x1745E, then reaches the world-mode action loop. Keep the
        -- Lua Matrix session alive and hand control to that route; the
        -- node-action screen owns later Deck round trips.
        return { transition = T.REPLACE,
                 screen = E.MenuScreen.MATRIX_NODE_ACTION,
                 args = { system_id = st.system_id,
                          matrix_state = st.matrix_state,
                          return_to_field = st.return_to_field,
                          traversal_view = true } }
    end
    if st.return_to_field then
        MatrixSession.finish(st.matrix_state)
        return { transition = T.EXIT, result = res }
    end
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
    tick_palette(st, ctx)                                                               -- 0x196F2
    service_program_loading(st, ctx)
    local slot_art_changed = update_program_art_state(st, ctx)
    local col, row, moved = MX.step_deck_cursor(
        st.col, st.row,
        events & P.UP ~= 0, events & P.DOWN ~= 0,
        events & P.LEFT ~= 0, events & P.RIGHT ~= 0)
    if moved then
        st.col, st.row = col, row
        st.text = nil
        st.storage_empty_message_lines = nil
        update_description(st)
        update_program_details(st, g.deck_record())
        st.matrix_run_info = nil
        st.matrix_deck_stats = nil
        ctx.redraw(slot_art_changed and R.FULL or R.SUBMENU)
        return
    end
    if events & P.START ~= 0 then return leave(st, ctx) end
    local btn = events & P.A ~= 0 and P.A or events & P.B ~= 0 and P.B or events & P.C ~= 0 and P.C
    if not btn then
        if slot_art_changed then ctx.redraw(R.FULL) end
        return
    end
    g.set_deck_mode(MODE[btn])
    if st.matrix_state then st.matrix_state.mode = RESULT_MODE[btn] end
    st.text = nil
    st.storage_empty_message_lines = nil
    if st.row ~= M.PLUG_ROW then return { transition = T.PUSH, screen = E.MenuScreen.DECK_PROGRAM, args = { col = st.col, row = st.row, deck_palette = st.deck_palette, matrix_state = st.matrix_state, deck_parent_col = st.col, deck_parent_row = st.row } } end
    if st.col == M.PLUG_COL then
        if not online(st, ctx) then return end
        local node = MatrixSession.current_node(st.matrix_state) or g.matrix_node()
        local alert = MatrixSession.is_online(st.matrix_state) and
                      MatrixSession.alert(st.matrix_state) or
                      g.alert()
        local gate_negative
        if MatrixSession.is_online(st.matrix_state) then
            gate_negative = st.matrix_state.world_effect_gate_negative == true
        elseif type(g.world_effect_gate_negative) == "function" then
            gate_negative = g.world_effect_gate_negative() == true
        else
            gate_negative = false
        end
        local black = MX.ic_type(node, alert,
                                 gate_negative,
                                 MatrixSession.is_online(st.matrix_state) and 1 or
                                     g.map_seen()) == M.BLACK_IC and
                      g.in_combat() -- 0x19B40 == 8 && FFDF84
        return { transition = T.PUSH, screen = E.MenuScreen.PULL_PLUG,
                 args = { black_ic = black,
                          deck_palette = st.deck_palette,
                          matrix_state = st.matrix_state } }
    end
    if st.col == 2 and not online(st, ctx) then return end
    return { transition = T.PUSH, screen = M.FUNCTION[st.col],
             args = { deck_palette = st.deck_palette,
                      matrix_state = st.matrix_state,
                      deck_parent_col = st.col,
                      deck_parent_row = st.row } }                                -- 0xE2DFE[col]
end
function M.child_returned(st, child, res, ctx)
    if res.logged_out then return leave(st, ctx, { exit_secretary = true }) end          -- FFF0DC = FF
    if res.exit_deck then
        return leave(st, ctx, st.return_to_node_action and {} or
            (st.matrix_session and { exit_secretary = true } or {}))                    -- 0x549C0 resumes 0x54662
    end
    st.storage_empty_message_lines = res.storage_empty == true and
        D.STORAGE_EMPTY_MESSAGE_LINES or nil
    st.text = type(res.text) == "string" and res.text or nil                           -- plain-rts text remains until redraw
    st.matrix_run_info = type(res.matrix_run_info) == "table"
        and res.matrix_run_info or nil
    st.matrix_deck_stats = type(res.matrix_deck_stats) == "table"
        and res.matrix_deck_stats or nil
    local _, record = update_program_art_state(st, ctx)
    update_program_details(st, record)
    st.list = res.list
    st.deck_palette_phase = DeckPalette.phase(st.deck_palette)
    ctx.redraw(R.FULL)                                                                   -- 0x1882E grid redraw
end

-- Pull the plug: 0xE438A (+0xE43B0 Black IC warning), Yes/No 0xE387E, FFF0DC default 1 = No; A/B/C confirm.
-- Yes + Black IC: Willpower (+0x7C) dice vs TN = IC rating (0x19BF6 d6) via 0xDBA.
--   0 successes -> physical -(TN >> 1) (0x17D90), sfx 0x18; alive -> back to the deck grid (NO logout); dead -> logout.
--   else Body (+0x77) dice vs 5; 0 successes -> stun -3 (min 1), sfx 0x18. Then logout.
-- Logout: FFF0DC = FF, 0x18776, 0x26A00 (sfx 0x29, FFF0B1 = FF).
local Plug = { window = E.WindowPlane.OFF }
function Plug.new(a)
    local deck_palette = a.deck_palette or DeckPalette.new()
    return {
        black_ic = a.black_ic,
        deck_palette = deck_palette,
        deck_palette_phase = DeckPalette.phase(deck_palette),
        matrix_state = a.matrix_state,
        yes = false,
        row = 1,
        title = a.black_ic
            and "WARNING:|Black IC present!|Do you really want|to pull the plug?"
            or "Do you really want|to pull the plug?",
        labels = { "Yes", "No" },
    }
end
function Plug.enter(st, ctx)
    -- 0x1822C enters this prompt from the selected Cyberdeck cell, while the
    -- source leaves the program grid and loaded-slot footer visible behind
    -- the question. Publish those current Lua deck values with the child
    -- screen so the presenter can keep that same deck surface on screen.
    update_program_art_state(st, ctx)
    ctx.redraw(R.MESSAGE)
end
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
    tick_palette(st, ctx)
    if service_program_loading(st, ctx) then ctx.redraw(R.FULL) end
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
