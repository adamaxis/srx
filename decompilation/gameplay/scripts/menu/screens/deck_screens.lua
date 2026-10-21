-- Lua routes for the four Cyberdeck child screens. ROM source references and
-- their known text endings are kept here; the Qt layer only presents state.
local E = require("enums")
local D = require("menu.rules.deck")
local MatrixSession = require("menu.matrix_session")
local DeckPalette = require("menu.deck_palette")
local Cyberdeck = require("menu.screens.cyberdeck").Cyberdeck
local P, R, T, MM = E.Pad, E.MenuRegion, E.MenuTransition, E.MenuMessage
local ABC = P.A | P.B | P.C

local function tick_palette(st, ctx)
    local phase, changed = DeckPalette.tick(st.deck_palette)
    st.deck_palette_phase = phase
    if changed then ctx.redraw(R.FULL) end
end

local function service_program_loading(st, ctx)
    if not MatrixSession.is_online(st.matrix_state) then return false end
    local slots = ctx.game.deck_slots()
    if not MatrixSession.advance_program_loading(st.matrix_state, slots) then
        return false
    end
    ctx.game.deck_commit(ctx.game.deck_record(), slots)
    ctx.redraw(R.FULL)
    return true
end

local Program = { window = E.WindowPlane.OFF }
function Program.new(args)
    args = args or {}
    local deck_palette = args.deck_palette or DeckPalette.new()
    return { p = D.index(args.col, args.row), choice = 1,
             matrix_state = args.matrix_state,
             deck_parent_col = args.deck_parent_col or args.col,
             deck_parent_row = args.deck_parent_row or args.row,
             deck_palette = deck_palette,
             deck_palette_phase = DeckPalette.phase(deck_palette) }
end
function Program.enter(st, ctx)
    Cyberdeck.prepare_program_child_surface(st, ctx)
    local g = ctx.game
    if st.p < 0 or st.p >= D.PROGRAMS then
        return { transition = T.POP, result = {} }
    end
    local record, slots = g.deck_record(), g.deck_slots()
    if record.ratings[st.p] == 0 then
        return { transition = T.POP, result = {} }
    end
    st.slot = D.loaded_slot(record, st.p)
    if st.slot then
        -- ROM 0x1927A streams 0xE3859 into $C628. Its 0x80 control byte
        -- advances one text row between these two prompt lines.
        st.title = "Program is already|loaded in memory.|Erase from|active memory?"
        st.labels = { "Yes", "No" }
        st.row = st.choice
        ctx.redraw(R.MESSAGE)
        return
    end
    local free = D.free_slot(record)
    if not free then
        return { transition = T.POP, result = { text = "No available slots!" } }
    end
    if not D.fits(record, st.p) then
        return { transition = T.POP, result = { text = "No available memory!" } }
    end
    D.load(record, slots, free, st.p, true)
    g.deck_commit(record, slots)
    return { transition = T.POP, result = {} }
end
function Program.input(st, events, ctx)
    tick_palette(st, ctx)
    service_program_loading(st, ctx)
    Cyberdeck.prepare_program_child_surface(st, ctx)
    if st.slot then
        if events & (P.UP | P.DOWN) ~= 0 then
            st.choice = 1 - st.choice
            st.row = st.choice
            ctx.redraw(R.MESSAGE)
            return
        end
        if events & ABC ~= 0 then
            if st.choice == 0 then
                local g = ctx.game
                local record = g.deck_record()
                local slots = g.deck_slots()
                local program = D.unload(record, slots, st.slot)
                if D.UNLOAD_EFFECT[program] then g.program_unloaded(program) end
                g.deck_commit(record, slots)
            end
            return { transition = T.POP, result = {} }
        end
        if events & P.START ~= 0 then
            return { transition = T.POP, result = {} }
        end
    end
end

local CyberInfo = { window = E.WindowPlane.OFF }
function CyberInfo.new(args)
    args = args or {}
    local deck_palette = args.deck_palette or DeckPalette.new()
    return { row = 0, matrix_state = args.matrix_state,
             deck_parent_col = args.deck_parent_col,
             deck_parent_row = args.deck_parent_row,
             deck_palette = deck_palette,
             deck_palette_phase = DeckPalette.phase(deck_palette) }
end
function CyberInfo.enter(st, ctx)
    -- ROM 0x18AB0 reads the padded title/options at 0xE3886 from $C32C.
    -- The source wording is "Cyberdeck Info", with two leading spaces on
    -- each visible line; the 12-glyph separator is placed by the presenter.
    st.title = "  Cyberdeck Info"
    st.labels = { "  Deck Stats", "  Run Info" }
    Cyberdeck.update_program_art_state(st, ctx)
    ctx.redraw(R.SUBMENU)
end
function CyberInfo.input(st, events, ctx)
    tick_palette(st, ctx)
    service_program_loading(st, ctx)
    Cyberdeck.update_program_art_state(st, ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        st.row = 1 - st.row
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & ABC ~= 0 then
        if st.row == 1 then
            -- ROM 0x18C42 branches on signed FFF0E3: zero is no run,
            -- positive is complete, and negative displays the active
            -- objective's node, file class, and action.
            local run = nil
            if MatrixSession.is_online(st.matrix_state) then
                run = MatrixSession.world_state(ctx.game).run
            end
            local status, action_index = D.run_info(
                run and run.state or 0,
                run and run.objective or 0,
                run and run.value or 0)
            if status == "COMPLETE" then
                return { transition = T.POP,
                         result = { matrix_run_info = {
                             kind = "complete",
                             message = "Shadowrun is| complete!!",
                         } } }
            end
            if status == "ACTIVE" then
                local objective = run.objective
                local node = objective < 0 and "CPU" or "Datastore"
                local node_function = objective < 0 and "Cntrl Processor" or
                                     MatrixSession.file_class_name(objective)
                local action_names = {
                    [0] = "Delete File", "Upload File", "Crash System",
                    "Download File",
                }
                return { transition = T.POP, result = { matrix_run_info = {
                    kind = "active",
                    node_label = "Node:",
                    node = node,
                    function_label = "Node Function:",
                    node_function = node_function,
                    action_label = "Action:",
                    action = action_names[action_index],
                } } }
            end
            -- The field Cyberdeck route has no active run. ROM 0x18C50
            -- writes two text rows separated by a 0x80 row control and
            -- returns without opening a wait screen.
            return { transition = T.POP,
                     result = { matrix_run_info = {
                         kind = "no_run",
                         message = "You are not on|a matrix run!!",
                     } } }
        end
        local record = ctx.game.deck_record()
        local model = D.MODEL_NAMES[record.model] or { "", "" }
        local persona = record.persona
        return { transition = T.POP, result = { matrix_deck_stats = {
            manufacturer = model[1],
            model = model[2],
            labels = D.STAT_LABELS,
            values = {
                record.mpcp, record.hardening, record.memory, record.storage,
                record.load_io, record.response,
                persona[0], persona[1], persona[2], persona[3],
            },
        } } }
    end
    if events & P.START ~= 0 then
        return { transition = T.POP, result = {} }
    end
end

local function file_text(record)
    local lines = {}
    for _, index in ipairs(D.files(record)) do
        local file = record.files[index]
        local name = D.FILE_NAMES[file.id] or ("File " .. file.id)
        lines[#lines + 1] = name .. "  " .. file.size .. " Mp"
    end
    return #lines == 0 and nil or table.concat(lines, "|")
end

local Storage = { window = E.WindowPlane.OFF }
function Storage.new(args)
    args = args or {}
    local deck_palette = args.deck_palette or DeckPalette.new()
    return { row = 0, matrix_state = args.matrix_state,
             deck_parent_col = args.deck_parent_col or args.col,
             deck_parent_row = args.deck_parent_row or args.row,
             deck_palette = deck_palette,
             deck_palette_phase = DeckPalette.phase(deck_palette) }
end
local function update_storage_panel(st, ctx)
    local record = ctx.game.deck_record()
    st.storage_panel_lines = D.STORAGE_PANEL_LINES
    st.storage_total = record.storage or 0
    st.storage_available = D.free_storage(record)
    Cyberdeck.update_program_art_state(st, ctx)
end
function Storage.enter(st, ctx)
    update_storage_panel(st, ctx)
    st.title = "Storage"
    st.labels = { "View", "Erase" }
    ctx.redraw(R.SUBMENU)
end
local function return_file_list(ctx)
    local text = file_text(ctx.game.deck_record())
    if not text then
        return { transition = T.POP,
                 result = { text = "No datafiles in storage" } }
    end
    return { transition = T.POP, result = { text = text } }
end
function Storage.input(st, events, ctx)
    tick_palette(st, ctx)
    service_program_loading(st, ctx)
    update_storage_panel(st, ctx)
    local g = ctx.game
    if st.phase == "erase" then
        if events & (P.UP | P.DOWN) ~= 0 then
            st.choice = 1 - st.choice
            st.row = st.choice
            ctx.redraw(R.MESSAGE)
            return
        end
        -- The ROM's storage prompt accepts A only. B and C do nothing.
        if events & P.A ~= 0 then
            local record = g.deck_record()
            if st.choice == 0 then
                D.erase_file(record, st.file)
                g.deck_commit(record, g.deck_slots())
                return return_file_list(ctx)
            end
            st.file = D.next_file(record, st.file - 1)
            if not st.file then
                return { transition = T.POP,
                         result = { text = "No datafiles in storage" } }
            end
            st.choice, st.row = 0, 0
            st.title = ""
            st.labels = { "Yes", "No" }
            ctx.redraw(R.MESSAGE)
            return
        end
        if events & P.START ~= 0 then return return_file_list(ctx) end
        return
    end

    if events & (P.UP | P.DOWN) ~= 0 then
        st.row = 1 - st.row
        ctx.redraw(R.SUBMENU)
        return
    end
    -- The ROM storage menu accepts A only.
    if events & P.A ~= 0 then
        if st.row == 0 then return return_file_list(ctx) end
        st.file = D.next_file(g.deck_record(), D.FILES - 1)
        if not st.file then
            return { transition = T.POP,
                     result = { text = "No datafiles in storage" } }
        end
        st.phase, st.choice, st.row = "erase", 0, 0
        st.title = ""
        st.labels = { "Yes", "No" }
        ctx.redraw(R.MESSAGE)
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP, result = {} } end
end

local Map = { window = E.WindowPlane.OFF }
function Map.new(args)
    args = args or {}
    local deck_palette = args.deck_palette or DeckPalette.new()
    return { row = 0, matrix_state = args.matrix_state,
             deck_palette = deck_palette,
             deck_palette_phase = DeckPalette.phase(deck_palette) }
end
function Map.enter(st, ctx)
    -- The source only dispatches this screen from the active Matrix session.
    -- Keep a malformed/direct route from calling Matrix-only host services.
    if not MatrixSession.is_online(st.matrix_state) and
       not ctx.game.matrix_online() then
        return { transition = T.POP, result = {} }
    end
    st.title = "System Map"
    st.labels = { "Show Map", "Retreat" }
    ctx.redraw(R.SUBMENU)
end
function Map.input(st, events, ctx)
    service_program_loading(st, ctx)
    local g = ctx.game
    if st.phase == "map" then
        if events & P.START ~= 0 then
            return { transition = T.POP, result = { reenter = true } }
        end
        return
    elseif st.phase == "retreat" then
        if events & ABC ~= 0 then
            g.retreat()
            return { transition = T.POP, result = { exit_deck = true } }
        end
        if events & P.START ~= 0 then
            g.deck_restore_screen()
            return { transition = T.POP, result = { reenter = true } }
        end
        return
    end
    if events & (P.UP | P.DOWN) ~= 0 then
        st.row = 1 - st.row
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & ABC ~= 0 then
        if st.row == 0 then
            st.title = "System Map"
            st.matrix_map_cells =
                MatrixSession.map_visual_cells(st.matrix_state)
            st.matrix_map_labels = MatrixSession.map_labels()
            st.labels = {}
            st.phase = "map"
            ctx.redraw(R.FULL)
            return
        end
        if g.in_combat() then
            return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE,
                     args = { message = MM.RETREAT_COMBAT,
                              text = "Retreat not possible during combat!",
                              mask = P.EDGES } }
        end
        local previous = MatrixSession.is_online(st.matrix_state)
                           and MatrixSession.previous_node(st.matrix_state)
                           or g.previous_node()
        if previous < 0 then
            return { transition = T.POP,
                     result = { text = "Retreat is not possible from this node!" } }
        end
        if MatrixSession.is_online(st.matrix_state) then
            MatrixSession.retreat(st.matrix_state)
            return { transition = T.POP, result = { exit_deck = true } }
        end
        g.show_retreat()
        st.phase = "retreat"
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP, result = {} } end
end

return { Program = Program, CyberInfo = CyberInfo,
         Storage = Storage, Map = Map }
