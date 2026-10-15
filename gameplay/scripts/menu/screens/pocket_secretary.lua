-- pocket_secretary.lua: Pocket Secretary 0xC2A6 (ROM loop read: Up/Down via row cursor 0xC376, max row 4 or 5 when
-- FFF101 != 0; A (bit14 only) dispatches table 0xC1400; handler leaving FFF0DC < 0 exits; C -> 0x391A; Start exits) [C].
-- Navigation is silent (docs/09 s12 #7) [C]. Handlers: 0 Notebook 0xF236, 1 Group Items 0xC8C8, 2 Cyberdeck 0x18150,
-- 3 Dismiss Runner 0xC448, 4 Save/Load 0xEFB4, 5 Debug menu 0x599DA (hidden unlabeled row, FFF101 != 0; docs/12) [C].
-- Save/Load sets FFF0DC = FF and FFF0CD = FF after 0x59360 / 0x596C4 (0xF124 / 0xF178) -> the Command menu exits [C].
local E = require("enums")
local P, O, N, R, T, MM = E.Pad, E.PocketOption, E.NotebookOption,
                         E.MenuRegion, E.MenuTransition, E.MenuMessage
local Grid = require("menu.rules.random_grid")
local M = { window = E.WindowPlane.SIDEBAR }

local function row_cursor(cur, d, max) local n = cur + d; if n < 0 then return max elseif n > max then return 0 end; return n end   -- 0xC376 [C]

function M.new() return { row = 0, grid_cells = Grid.cells, grid_effect_frames = 0 } end
function M.enter(st, ctx)
    Grid.reset()
    st.grid_effect_frames = 0
    st.max = ctx.game.pocket_extra_row() and 5 or 4
    ctx.host.blank(E.DisplayBlank.OFF); ctx.redraw(R.FULL); ctx.host.blank(E.DisplayBlank.ON)    -- build (docs/09 s10.7) [C]
end
function M.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)                  -- 0xE660, three ROM RNG draws per frame [C]
    if events & P.UP ~= 0 then st.row = row_cursor(st.row, -1, st.max); ctx.redraw(R.SUBMENU); return end
    if events & P.DOWN ~= 0 then st.row = row_cursor(st.row, 1, st.max); ctx.redraw(R.SUBMENU); return end
    if events & P.A ~= 0 then
        if st.row == O.NOTEBOOK then return { transition = T.PUSH, screen = E.MenuScreen.NOTEBOOK } end
        if st.row == O.GROUP_ITEMS then return { transition = T.PUSH, screen = E.MenuScreen.GROUP_ITEMS } end
        if st.row == O.CYBERDECK then return { transition = T.PUSH, screen = E.MenuScreen.CYBERDECK } end
        if st.row == O.DISMISS_RUNNER then return { transition = T.PUSH, screen = E.MenuScreen.DISMISS_RUNNER } end
        if st.row == O.SAVE_LOAD then return { transition = T.PUSH, screen = E.MenuScreen.SAVE_LOAD } end
        return { transition = T.PUSH, screen = E.MenuScreen.DEBUG_MENU }   -- row 5 (only when FFF101 != 0): debug menu 0x599DA [C]
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE)
        ctx.game.reselect_leader()
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP, result = { exit_menu = false } } end
end
function M.child_returned(st, child, res, ctx)
    if res.exit_menu then return { transition = T.POP, result = { exit_menu = true } } end
    if res.exit_secretary then return { transition = T.POP, result = { exit_menu = false } } end    -- FFF0DC < 0 only [C]   -- FFF0DC<0 -> rts; FFF0CD -> exit [C]
    -- Direct children redraw only the inner C3E6 grid before returning to
    -- C2AA. The lower bezel is outside that clear and keeps its E660 writes.
    st.grid_effect_frames = Grid.frames
    st.grid_cells = Grid.cells
    ctx.host.blank(E.DisplayBlank.OFF); ctx.redraw(R.FULL); ctx.host.blank(E.DisplayBlank.ON)  -- bra 0xC2AA rebuild [C]
end

-- Group Items 0xC8C8 reads FFFC22, displays set bits 14..0, then waits for a
-- newly pressed controller edge at F312. The host presents these item IDs.
local GroupItems = { window = E.WindowPlane.SIDEBAR }
function GroupItems.new()
    return { visible_item_ids = {}, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function GroupItems.enter(st, ctx)
    local flags = ctx.game.group_item_flags()
    if flags == nil then return { transition = T.POP } end
    st.group_item_flags = flags & 0x7FFF
    for item_id = 14, 0, -1 do
        if (st.group_item_flags >> item_id) & 1 ~= 0 then
            st.visible_item_ids[#st.visible_item_ids + 1] = item_id
        end
    end
    ctx.redraw(R.SUBMENU)
end
function GroupItems.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & P.EDGES ~= 0 then return { transition = T.POP } end
end

-- Notebook 0xF236 (srx pocket_secretary_menu_conversion): rows gated by story flags
local Notebook = { window = E.WindowPlane.SIDEBAR }
function Notebook.new()
    return { row = 0, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function Notebook.enter(st, ctx)
    local nb = ctx.game.notebook()
    if not nb or not nb.gates then
        ctx.host.notebook_boundary(E.NotebookBoundary.EVENT_GATES_UNRESOLVED)
        return { transition = T.POP }
    end
    st.options = { N.CURRENT_RUN, N.TIPS_AND_CLUES }
    if nb.gates[1] then st.options[#st.options + 1] = N.CONTACTS end
    if nb.gates[2] then st.options[#st.options + 1] = N.MR_JOHNSONS end
    if nb.gates[3] then st.options[#st.options + 1] = N.SHADOWRUNNERS end
    st.nb = nb; ctx.redraw(R.SUBMENU)
end
function Notebook.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.B | P.START) ~= 0 then return { transition = T.POP } end
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.options
        st.row = events & P.UP ~= 0 and (st.row == 0 and n - 1 or st.row - 1) or (st.row + 1) % n
        ctx.redraw(R.SUBMENU); return
    end
    if events & P.A ~= 0 then
        local refreshed = ctx.game.notebook()
        if refreshed then st.nb = refreshed end
        local opt = st.options[st.row + 1]
        if opt == N.CURRENT_RUN then
            if not st.nb.current_run then
                return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                         args = { message = MM.NO_CURRENT_RUN,
                                  grid_effect = true } }
            end
            ctx.host.notebook_boundary(
                E.NotebookBoundary.CURRENT_RUN_UNRESOLVED)
            return
        end
        if opt == N.TIPS_AND_CLUES then
            if not st.nb.tips then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.TIPS_FLAGS_UNRESOLVED)
                return
            end
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_TIPS,
                     args = { flags = st.nb.tips } }
        end
        if opt == N.MR_JOHNSONS then
            if not st.nb.referrals then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.MR_JOHNSON_FLAGS_UNRESOLVED)
                return
            end
            return { transition = T.PUSH,
                     screen = E.MenuScreen.MR_JOHNSONS,
                     args = { flags = st.nb.referrals } }
        end
        if opt == N.CONTACTS then
            local refreshed = ctx.game.notebook()
            if not refreshed or refreshed.contacts == nil then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.CONTACTS_FLAGS_UNRESOLVED)
                return
            end
            st.nb = refreshed
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_CONTACTS,
                     args = { kind = "contacts", flags = refreshed.contacts } }
        elseif opt == N.SHADOWRUNNERS then
            local refreshed = ctx.game.notebook()
            if not refreshed or refreshed.shadowrunners == nil then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.SHADOWRUNNERS_FLAGS_UNRESOLVED)
                return
            end
            st.nb = refreshed
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_SHADOWRUNNERS,
                     args = { kind = "shadowrunners",
                              flags = refreshed.shadowrunners } }
        end
        return
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE)
        ctx.game.reselect_leader()
    end
end
function Notebook.child_returned(st, child, res, ctx)
    st.grid_effect_frames = Grid.frames
    st.grid_cells = Grid.cells
    ctx.redraw(R.SUBMENU)
end

-- Tips & Clues: Left/Right page (clamped), A/B/Start back, C reselect [srx C]
local Tips = { window = E.WindowPlane.SIDEBAR }
function Tips.new(a)
    local v = {}
    for g = 0, 2 do
        for b = 0, 31 do
            if (a.flags[g + 1] >> b) & 1 == 1 then
                v[#v + 1] = g * 32 + b
            end
        end
    end
    return { visible = v, page = 0, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function Tips.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Tips.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.LEFT | P.RIGHT) ~= 0 then
        if #st.visible == 0 then return end
        if events & P.LEFT ~= 0 then st.page = math.max(0, st.page - 1) else st.page = math.min(#st.visible - 1, st.page + 1) end
        ctx.redraw(R.SUBMENU); return
    end
    if events & (P.A | P.B | P.START) ~= 0 then return { transition = T.POP } end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE)
        ctx.game.reselect_leader()
    end
end

-- Mr. Johnsons: Up/Down wrap, A/B detail (message), C reselect, Start back; closes at once if none [srx C]
local Johnsons = { window = E.WindowPlane.SIDEBAR }
function Johnsons.new(a)
    local v = {}
    for b = 0, 4 do
        if (a.flags >> b) & 1 == 1 then v[#v + 1] = b end
    end
    return { visible = v, row = 0, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function Johnsons.enter(st, ctx)
    if #st.visible == 0 then return { transition = T.POP } end
    ctx.redraw(R.SUBMENU)
end
function Johnsons.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.visible
        st.row = events & P.UP ~= 0 and (st.row == 0 and n - 1 or st.row - 1) or (st.row + 1) % n
        ctx.redraw(R.SUBMENU); return
    end
    if events & (P.A | P.B) ~= 0 then
        return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                 args = { message = MM.MR_JOHNSON_DETAIL,
                          grid_effect = true,
                          data = { index = st.visible[st.row + 1],
                                   row = st.row, visible = st.visible } } }
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE)
        ctx.game.reselect_leader()
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end
function Johnsons.child_returned(st, child, res, ctx)
    st.grid_effect_frames = Grid.frames
    st.grid_cells = Grid.cells
    ctx.redraw(R.SUBMENU)
end

-- Contacts 0xF840 and Shadowrunners 0xF6DC share the ROM 0xF5F4
-- bitmask-list builder. Its inserted eighth row is the source's page-forward
-- string; the final overflow row on the continuation page returns to page 1.
local function build_notebook_record_rows(flags, count)
    local selected = {}
    for source_index = 0, count - 1 do
        if (flags >> source_index) & 1 ~= 0 then
            selected[#selected + 1] = source_index
        end
    end
    if #selected <= 8 then return selected end
    local rows = {}
    for index = 1, 7 do rows[#rows + 1] = selected[index] end
    rows[#rows + 1] = count -- C661D, inserted before source record eight.
    for index = 8, #selected do rows[#rows + 1] = selected[index] end
    rows[#rows + 1] = count + 1 -- C662B, continuation return row.
    return rows
end

local function record_page(rows, page)
    if page == 0 then
        local visible = {}
        for index = 1, math.min(8, #rows) do
            visible[#visible + 1] = rows[index]
        end
        return visible
    end
    local visible = {}
    for index = 9, #rows do visible[#visible + 1] = rows[index] end
    return visible
end

local function make_record_page(count, detail_message)
    local Records = { window = E.WindowPlane.SIDEBAR }
    function Records.new(args)
        local rows = build_notebook_record_rows(args.flags or 0, count)
        return { rows = rows, visible = record_page(rows, 0), row = 0,
                 page = 0, count = count, detail_message = detail_message,
                 grid_cells = Grid.cells, grid_effect_frames = Grid.frames }
    end
    function Records.enter(st, ctx)
        if #st.visible == 0 then return { transition = T.POP } end
        ctx.redraw(R.SUBMENU)
    end
    function Records.input(st, events, ctx)
        st.grid_effect_frames = Grid.advance(ctx)
        if events & (P.UP | P.DOWN) ~= 0 then
            local n = #st.visible
            st.row = events & P.UP ~= 0 and
                (st.row == 0 and n - 1 or st.row - 1) or (st.row + 1) % n
            ctx.redraw(R.SUBMENU); return
        end
        if events & (P.A | P.B) ~= 0 then
            local source_index = st.visible[st.row + 1]
            if st.page == 0 and source_index == count then
                st.page = 1; st.visible = record_page(st.rows, 1); st.row = 0
                ctx.redraw(R.SUBMENU); return
            elseif st.page == 1 and source_index == count + 1 then
                st.page = 0; st.visible = record_page(st.rows, 0); st.row = 0
                ctx.redraw(R.SUBMENU); return
            end
            return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                     args = { message = detail_message, grid_effect = true,
                              data = { index = source_index, row = st.row,
                                       visible = st.visible } } }
        end
        if events & P.C ~= 0 then
            ctx.sound(E.MenuSound.MOVE)
            ctx.game.reselect_leader()
            return
        end
        if events & P.START ~= 0 then return { transition = T.POP } end
    end
    function Records.child_returned(st, child, res, ctx)
        st.grid_effect_frames = Grid.frames
        st.grid_cells = Grid.cells
        ctx.redraw(R.SUBMENU)
    end
    return Records
end

local Contacts = make_record_page(13, E.MenuMessage.NOTEBOOK_CONTACT_DETAIL)
local Shadowrunners = make_record_page(
    10, E.MenuMessage.NOTEBOOK_SHADOWRUNNER_DETAIL)

-- Dismiss Runner 0xC448: rows = Abort + candidates (slots 1..2 active); Up/Down 0x25; A/B/C 0x24 [srx FSR sounds H]
local Dismiss = { window = E.WindowPlane.SIDEBAR }
function Dismiss.new() return { row = 0, grid_cells = Grid.cells, grid_effect_frames = Grid.frames } end
function Dismiss.enter(st, ctx)
    -- C448 replaces the inner panel but shares the parent's E660 lower bezel.
    st.grid_effect_frames = Grid.frames
    st.candidates = {}
    local party = ctx.game.party()
    for s = 1, 2 do if ctx.game.slot_active_set()[s] and party[s] and party[s].status & E.RunnerStatus.ACTIVE ~= 0 then st.candidates[#st.candidates + 1] = s end end
    ctx.redraw(R.SUBMENU)
end
function Dismiss.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.candidates + 1
        st.row = events & P.UP ~= 0 and (st.row == 0 and n - 1 or st.row - 1) or (st.row + 1) % n
        ctx.sound(E.MenuSound.MOVE); ctx.redraw(R.SUBMENU); return
    end
    if events & (P.A | P.B | P.C) ~= 0 then
        ctx.sound(E.MenuSound.SELECT)
        if st.row > 0 then ctx.game.dismiss_runner(st.candidates[st.row]); ctx.redraw(R.SIDEBAR_SLOT | R.COMBAT_BARS) end
        return { transition = T.POP }
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end

return { PocketSecretary = M, GroupItems = GroupItems, Notebook = Notebook,
         Tips = Tips, Johnsons = Johnsons, Contacts = Contacts,
         Shadowrunners = Shadowrunners, Dismiss = Dismiss,
         row_cursor = row_cursor }
