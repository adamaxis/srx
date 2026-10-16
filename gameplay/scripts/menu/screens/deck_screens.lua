-- Lua routes for the four Cyberdeck child screens. ROM source references and
-- their known text endings are kept here; the Qt layer only presents state.
local E = require("enums")
local D = require("menu.rules.deck")
local MatrixSession = require("menu.matrix_session")
local P, R, T, MM = E.Pad, E.MenuRegion, E.MenuTransition, E.MenuMessage
local ABC = P.A | P.B | P.C

local Program = { window = E.WindowPlane.OFF }
function Program.new(args)
    return { p = D.index(args.col, args.row), choice = 1 }
end
function Program.enter(st, ctx)
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
        st.title = "Program is already loaded in memory."
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
    ctx.game.deck_tick()
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
function CyberInfo.new() return { row = 0 } end
function CyberInfo.enter(st, ctx)
    st.title = "Cyberdeck Information"
    st.labels = { "Deck Stats", "Run Info" }
    ctx.redraw(R.SUBMENU)
end
function CyberInfo.input(st, events, ctx)
    ctx.game.deck_tick()
    if events & (P.UP | P.DOWN) ~= 0 then
        st.row = 1 - st.row
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & ABC ~= 0 then
        if st.row == 1 then
            -- FFF0E3 is clear in the field Cyberdeck route; the ROM prints
            -- this text and returns without opening a wait screen.
            return { transition = T.POP,
                     result = { text = "You are not on a matrix run!!" } }
        end
        local record = ctx.game.deck_record()
        local p = record.persona
        local text = table.concat({
            "MPCP: " .. record.mpcp,
            "Hardening: " .. record.hardening,
            "Memory: " .. record.memory .. " Mp",
            "Storage: " .. record.storage .. " Mp",
            "Load/IO: " .. record.load_io,
            "Response: " .. record.response,
            "Bod/Evasion/Masking/Sensor: " ..
                p[0] .. "/" .. p[1] .. "/" .. p[2] .. "/" .. p[3],
        }, "|")
        return { transition = T.POP, result = { text = text } }
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
function Storage.new() return { row = 0 } end
function Storage.enter(st, ctx)
    st.title = "Cyberdeck Storage"
    st.labels = { "View Files", "Erase File" }
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
    local g = ctx.game
    g.deck_tick()
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
    return { row = 0, matrix_state = args.matrix_state }
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
    g.deck_tick()
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
