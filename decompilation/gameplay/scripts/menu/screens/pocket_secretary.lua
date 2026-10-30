-- pocket_secretary.lua: Pocket Secretary 0xC2A6 (ROM loop read: Up/Down via row cursor 0xC376, max row 4 or 5 when
-- FFF101 != 0; A (bit14 only) dispatches table 0xC1400; handler leaving FFF0DC < 0 exits; C -> 0x391A; Start exits) [C].
-- Navigation is silent (docs/09 s12 #7) [C]. Handlers: 0 Notebook 0xF236, 1 Group Items 0xC8C8, 2 Cyberdeck 0x18150,
-- 3 Dismiss Runner 0xC448, 4 Save/Load 0xEFB4, 5 Debug menu 0x599DA (hidden unlabeled row, FFF101 != 0; docs/12) [C].
-- Save/Load sets FFF0DC = FF and FFF0CD = FF after 0x59360 / 0x596C4 (0xF124 / 0xF178) -> the Command menu exits [C].
local E = require("enums")
local P, O, N, R, T, MM = E.Pad, E.PocketOption, E.NotebookOption,
                         E.MenuRegion, E.MenuTransition, E.MenuMessage
local POCKET_OPTION_ORDER = {
    O.NOTEBOOK, O.GROUP_ITEMS, O.CYBERDECK,
    O.DISMISS_RUNNER, O.SAVE_LOAD, O.DEBUG,
}
local Grid = require("menu.rules.random_grid")
local Cursor = require("menu.rules.cursor")
local M = { window = E.WindowPlane.SIDEBAR }

function M.new() return { row = 0, grid_cells = Grid.cells, grid_effect_frames = 0 } end
function M.enter(st, ctx)
    Grid.reset()
    st.grid_effect_frames = 0
    st.max = ctx.game.pocket_extra_row() and 5 or 4
    -- 0x10918..0x10AE6 build with display off (docs/09 s10.7) [C]; 15 black frames, display back on at line 216
    -- (emulator 8660..8675) [C emulator]
    ctx.host.blank(E.DisplayBlank.OFF); ctx.redraw(R.FULL); ctx.host.blank(E.DisplayBlank.ON, 15, 216)
end
function M.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)                  -- 0xE660, three ROM RNG draws per frame [C]
    if events & P.UP ~= 0 then st.row = Cursor.step(st.row, -1, st.max); ctx.redraw(R.SUBMENU); return end
    if events & P.DOWN ~= 0 then st.row = Cursor.step(st.row, 1, st.max); ctx.redraw(R.SUBMENU); return end
    if events & P.A ~= 0 then
        local selected_option = POCKET_OPTION_ORDER[st.row + 1]
        if selected_option == O.NOTEBOOK then return { transition = T.PUSH, screen = E.MenuScreen.NOTEBOOK } end
        if selected_option == O.GROUP_ITEMS then return { transition = T.PUSH, screen = E.MenuScreen.GROUP_ITEMS } end
        if selected_option == O.CYBERDECK then
            local available = ctx.game.cyberdeck_available()
            if available == false then
                return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                         args = { message = MM.NO_CYBERDECK,
                                  grid_effect = true } }
            end
            if available == nil then
                return { transition = T.PUSH,
                         screen = E.MenuScreen.CYBERDECK,
                         args = { legacy_adapter = true } }
            end
            return { transition = T.PUSH, screen = E.MenuScreen.CYBERDECK }
        end
        if selected_option == O.DISMISS_RUNNER then return { transition = T.PUSH, screen = E.MenuScreen.DISMISS_RUNNER } end
        if selected_option == O.SAVE_LOAD then return { transition = T.PUSH, screen = E.MenuScreen.SAVE_LOAD } end
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
    st.row = 0 -- [C] 0xC2C8 clears the shared row before the C2AA rebuild.
    -- Direct children redraw only the inner C3E6 grid before returning to
    -- C2AA. The lower bezel is outside that clear and keeps its E660 writes.
    st.grid_effect_frames = Grid.frames
    st.grid_cells = Grid.cells
    ctx.host.blank(E.DisplayBlank.OFF); ctx.redraw(R.FULL); ctx.host.blank(E.DisplayBlank.ON)  -- bra 0xC2AA rebuild [C]
end

-- Group Items 0xC8C8 displays the available catalog entries, then waits for a
-- newly pressed controller edge at F312. The host resolves save state into
-- ordered item IDs before Lua sees it.
local GroupItems = { window = E.WindowPlane.SIDEBAR }
function GroupItems.new()
    return { visible_item_ids = {}, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function GroupItems.enter(st, ctx)
    local item_ids = ctx.game.group_item_ids()
    if item_ids == nil then return { transition = T.POP } end
    st.visible_item_ids = item_ids
    ctx.redraw(R.SUBMENU)
end
function GroupItems.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & P.EDGES ~= 0 then return { transition = T.POP } end
end

-- Notebook 0xF236 (srx pocket_secretary_menu_conversion): rows gated by story flags
local Notebook = { window = E.WindowPlane.SIDEBAR }
local function active_run_id()
    -- The accepted run record lives in the Lua run registry. FFFBEC's active
    -- run state is consumed by the ROM Current Run branch at 0xF2E2/0x1E72C;
    -- the menu host's older boolean is not updated when Lua accepts an offer.
    local live = rawget(_G, "game")   -- gameplay global; absent in unit hosts
    local runs = type(live) == "table" and live.runs or nil
    local registry = type(runs) == "table" and runs.registry or nil
    if type(registry) ~= "table" or
       type(registry.active_run_id) ~= "function" then
        return nil
    end
    local id = registry.active_run_id()
    if type(id) ~= "string" or id == "none" then return nil end
    return id
end

local function active_run_view()
    local live = rawget(_G, "game")
    local runs = type(live) == "table" and live.runs or nil
    local registry = type(runs) == "table" and runs.registry or nil
    local modules = type(live) == "table" and live.modules or nil
    local support = type(modules) == "table" and modules.run_support or nil
    if type(registry) ~= "table" or
       type(registry.active_offer) ~= "table" or
       type(support) ~= "table" or
       type(support.current_run_view) ~= "function" then
        return nil
    end
    return support.current_run_view(registry.active_offer)
end

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
    st.nb = nb
    st.nb.active_run_id = active_run_id()
    if st.nb.active_run_id ~= nil then st.nb.current_run = true end
    ctx.redraw(R.SUBMENU)
end
function Notebook.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & P.START ~= 0 then return { transition = T.POP } end
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.options
        st.row = Cursor.step(st.row, events & P.UP ~= 0 and -1 or 1, n - 1)
        ctx.redraw(R.SUBMENU); return
    end
    if events & (P.A | P.B) ~= 0 then
        local refreshed = ctx.game.notebook()
        if refreshed then st.nb = refreshed end
        st.nb.active_run_id = active_run_id()
        if st.nb.active_run_id ~= nil then st.nb.current_run = true end
        local opt = st.options[st.row + 1]
        if opt == N.CURRENT_RUN then
            if not st.nb.current_run then
                return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                         args = { message = MM.NO_CURRENT_RUN,
                                  grid_effect = true } }
            end
            local view = active_run_view()
            if view ~= nil then
                return { transition = T.PUSH,
                         screen = E.MenuScreen.NOTEBOOK_CURRENT_RUN,
                         args = { view = view } }
            end
            ctx.host.notebook_boundary(E.NotebookBoundary.CURRENT_RUN_UNRESOLVED)
            return
        end
        if opt == N.TIPS_AND_CLUES then
            if not st.nb.tip_ids then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.TIPS_UNRESOLVED)
                return
            end
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_TIPS,
                     args = { entry_ids = st.nb.tip_ids } }
        end
        if opt == N.MR_JOHNSONS then
            if not st.nb.referral_ids then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.MR_JOHNSON_UNRESOLVED)
                return
            end
            return { transition = T.PUSH,
                     screen = E.MenuScreen.MR_JOHNSONS,
                     args = { entry_ids = st.nb.referral_ids } }
        end
        if opt == N.CONTACTS then
            local refreshed = ctx.game.notebook()
            if not refreshed or refreshed.contact_ids == nil then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.CONTACTS_UNRESOLVED)
                return
            end
            st.nb = refreshed
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_CONTACTS,
                     args = { kind = "contacts",
                              entry_ids = refreshed.contact_ids } }
        elseif opt == N.SHADOWRUNNERS then
            local refreshed = ctx.game.notebook()
            if not refreshed or refreshed.shadowrunner_ids == nil then
                ctx.host.notebook_boundary(
                    E.NotebookBoundary.SHADOWRUNNERS_UNRESOLVED)
                return
            end
            st.nb = refreshed
            return { transition = T.PUSH,
                     screen = E.MenuScreen.NOTEBOOK_SHADOWRUNNERS,
                     args = { kind = "shadowrunners",
                              entry_ids = refreshed.shadowrunner_ids } }
        end
        return
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE)
        ctx.game.reselect_leader()
    end
end
function Notebook.child_returned(st, child, res, ctx)
    st.row = 0 -- [C] F236 rebuild clears the shared row before drawing.
    st.grid_effect_frames = Grid.frames
    st.grid_cells = Grid.cells
    ctx.redraw(R.SUBMENU)
end

-- Tips & Clues: Left/Right page (clamped), A/B/Start back, C reselect [srx C]
local Tips = { window = E.WindowPlane.SIDEBAR }
function Tips.new(a)
    return { visible_entry_ids = a.entry_ids or {}, page = 0,
             grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function Tips.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Tips.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.LEFT | P.RIGHT) ~= 0 then
        if #st.visible_entry_ids == 0 then return end
        if events & P.LEFT ~= 0 then st.page = math.max(0, st.page - 1) else st.page = math.min(#st.visible_entry_ids - 1, st.page + 1) end
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
    return { visible_entry_ids = a.entry_ids or {}, row = 0,
             grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function Johnsons.enter(st, ctx)
    if #st.visible_entry_ids == 0 then return { transition = T.POP } end
    ctx.redraw(R.SUBMENU)
end
function Johnsons.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.visible_entry_ids
        st.row = Cursor.step(st.row, events & P.UP ~= 0 and -1 or 1, n - 1)
        ctx.redraw(R.SUBMENU); return
    end
    if events & (P.A | P.B) ~= 0 then
        return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                 args = { message = MM.MR_JOHNSON_DETAIL,
                          grid_effect = true,
                          data = { entry_id =
                                       st.visible_entry_ids[st.row + 1],
                                   row = st.row,
                                   visible_entry_ids = st.visible_entry_ids } } }
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

-- The host supplies unlocked record catalog IDs. Lua owns the page choices and
-- inserts the forward/return rows around that ordered list.
local function build_notebook_record_rows(entry_ids, count)
    local selected = {}
    for index = 1, math.min(#entry_ids, count) do
        selected[#selected + 1] = entry_ids[index]
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
        local rows = build_notebook_record_rows(args.entry_ids or {}, count)
        return { rows = rows,
                 visible_entry_ids = record_page(rows, 0), row = 0,
                 page = 0, count = count, detail_message = detail_message,
                 grid_cells = Grid.cells, grid_effect_frames = Grid.frames }
    end
    function Records.enter(st, ctx)
        if #st.visible_entry_ids == 0 then return { transition = T.POP } end
        ctx.redraw(R.SUBMENU)
    end
    function Records.input(st, events, ctx)
        st.grid_effect_frames = Grid.advance(ctx)
        if events & (P.UP | P.DOWN) ~= 0 then
            local n = #st.visible_entry_ids
            st.row = Cursor.step(st.row, events & P.UP ~= 0 and -1 or 1,
                                 n - 1)
            ctx.redraw(R.SUBMENU); return
        end
        if events & (P.A | P.B) ~= 0 then
            local entry_id = st.visible_entry_ids[st.row + 1]
            if st.page == 0 and entry_id == count then
                st.page = 1; st.visible_entry_ids = record_page(st.rows, 1); st.row = 0
                ctx.redraw(R.SUBMENU); return
            elseif st.page == 1 and entry_id == count + 1 then
                st.page = 0; st.visible_entry_ids = record_page(st.rows, 0); st.row = 0
                ctx.redraw(R.SUBMENU); return
            end
            return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE,
                     args = { message = detail_message, grid_effect = true,
                              data = { entry_id = entry_id, row = st.row,
                                       visible_entry_ids =
                                           st.visible_entry_ids } } }
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

-- Positive Current Run is a single read page followed by ROM 0xF312's fresh
-- high-byte controller edge. The accepted Lua offer supplies the typed view;
-- Qt only paints it and forwards the edge.
local CurrentRun = { window = E.WindowPlane.SIDEBAR }
function CurrentRun.new(args)
    return { view = args.view, grid_cells = Grid.cells,
             grid_effect_frames = Grid.frames }
end
function CurrentRun.enter(st, ctx)
    if type(st.view) ~= "table" or type(st.view.body_lines) ~= "table" then
        return { transition = T.POP }
    end
    ctx.redraw(R.SUBMENU)
end
function CurrentRun.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    st.grid_cells = Grid.cells
    if events & P.EDGES ~= 0 then return { transition = T.POP } end
end

-- Dismiss Runner 0xC448: rows = Abort + candidates (slots 1..2 active); ROM 0xC4C0 branches to 0xC4FA and plays MOVE on open [C ROM].
local Dismiss = { window = E.WindowPlane.SIDEBAR }
function Dismiss.new()
    return {
        row = 0, grid_cells = Grid.cells, grid_effect_frames = Grid.frames,
        layout = { separator_x = 88, separator_y = 48 }, -- C448 text at C216 [C]
    }
end
function Dismiss.enter(st, ctx)
    ctx.sound(E.MenuSound.MOVE)
    -- C448 replaces the inner panel but shares the parent's E660 lower bezel.
    st.grid_effect_frames = Grid.frames
    st.candidates = {}
    local party = ctx.game.party()
    for s = 1, 2 do
        local runner = party[s]
        if ctx.game.slot_active_set()[s] and runner and
           runner.runner_facts.lifecycle.active then
            st.candidates[#st.candidates + 1] = s
        end
    end
    ctx.redraw(R.SUBMENU)
end
function Dismiss.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        local n = #st.candidates + 1
        st.row = Cursor.step(st.row, events & P.UP ~= 0 and -1 or 1, n - 1)
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
         CurrentRun = CurrentRun,
         row_cursor = Cursor.step }
