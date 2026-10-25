-- ROM 0xFBA6 Magic screen and its 0xFF5A force/posture child loops.
-- Lua owns the selection, input, record changes, and displayed calculations;
-- Qt reads `magic_view` and paints the named source assets.
local E = require("enums")
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local Posture = require("menu.screens.posture")
local Rules = require("menu.rules.magic")

-- ROM FBA6 lays out the icon cells in a seven-column grid; FDFE applies the
-- selected-cell offsets to the cursor sprite. ROM 10276 sends the spell name
-- to C21C; e6 9060/9265/9400 show its first visible glyph at x=128 [C],
-- while the paired port captures start at x=112 [H].
-- FBA6 transfers three columns by two rows for each spell glyph, and FDAE
-- redraws the lower slot borders after those writes; e6 9060/9265/9400 show
-- no third icon row below y=183 [C].
local LAYOUT = {
    title_icon_x = 80, title_icon_y = 24,
    title_text_x = 128, title_text_y = 32,
    spell_grid_x = 32, spell_grid_y = 168,
    spell_grid_column_step = 32, spell_grid_row_step = 24,
    spell_grid_icon_width = 24, spell_grid_icon_height = 16,
    cursor_offset_x = 0, cursor_offset_y = 0,
    posture_bar_x = 160, posture_bar_y = 136,
    posture_text_x = 160, posture_text_y = 144,
}

-- ROM FBA6 clears the overlay phase; FDFE advances it every seven field
-- frames and wraps after four frames [C]. e6 RAM F0DA/F0DB (timer/phase)
-- reads 0/0 through the 12 black build frames, then 5/1 on the first lit
-- frame 8952 and ticks to 6/2 at 8958 [C emulator]. The host holds menu
-- heartbeats while the display is blanked, so entry leaves 5/1 and the first
-- heartbeat (8953) continues from there. e6 also shows one free-running
-- seven-frame clock across spell select <-> Set Force & Posture (B 9060,
-- A 9260, B 9380): ticks stay on 9043 + 7k [C emulator], so the child
-- screens inherit and hand back the timer and phase.
local CURSOR_ENTRY_TIMER, CURSOR_ENTRY_PHASE = 5, 1
local CURSOR_STARTUP_HEARTBEATS = 0

local function active_record(ctx)
    return ctx.game.party()[ctx.game.active_slot()]
end

local function next_cursor(st)
    if (st.cursor_startup_heartbeats or 0) > 0 then
        st.cursor_startup_heartbeats = st.cursor_startup_heartbeats - 1
        return
    end
    -- 0xFDFE only queues the SAT upload, so the slot ring shows the phase
    -- from before this tick for one frame [C e6 8958/8959].
    st.cursor_sat_phase = st.cursor_phase
    if st.cursor_timer == 0 then
        st.cursor_timer = 6
        st.cursor_phase = (st.cursor_phase + 1) % 4
    else
        st.cursor_timer = st.cursor_timer - 1
    end
    if st.magic_view then
        st.magic_view.cursor_phase = st.cursor_phase
        st.magic_view.cursor_sat_phase = st.cursor_sat_phase
    end
end

local function publish_layout(values, st, table_)
    values.title_icon_x, values.title_icon_y =
        LAYOUT.title_icon_x, LAYOUT.title_icon_y
    values.title_text_x, values.title_text_y =
        LAYOUT.title_text_x, LAYOUT.title_text_y
    values.spell_grid_x, values.spell_grid_y =
        LAYOUT.spell_grid_x, LAYOUT.spell_grid_y
    values.spell_grid_column_step = LAYOUT.spell_grid_column_step
    values.spell_grid_row_step = LAYOUT.spell_grid_row_step
    values.spell_grid_icon_width = LAYOUT.spell_grid_icon_width
    values.spell_grid_icon_height = LAYOUT.spell_grid_icon_height
    values.cursor_offset_x = st.sel.column * LAYOUT.spell_grid_column_step
    values.cursor_offset_y = st.sel.row * LAYOUT.spell_grid_row_step
    values.posture_bar_x, values.posture_bar_y =
        LAYOUT.posture_bar_x, LAYOUT.posture_bar_y
    values.posture_text_x, values.posture_text_y =
        LAYOUT.posture_text_x, LAYOUT.posture_text_y
    values.spell_codes = {}
    for index = 1, math.min(table_.entry_count, 14) do
        values.spell_codes[index] = table_.entries[index].code
    end
    values.cursor_phase = st.cursor_phase
    values.cursor_sat_phase = st.cursor_sat_phase or st.cursor_phase
end

local function publish_view(st, ctx, entry, force)
    local record = active_record(ctx)
    local values = Rules.presentation(
        record, ctx.game.party(), ctx.game.slot_active_set(), st.table,
        entry, st.sel.row, st.sel.column, force)
    publish_layout(values, st, st.table)
    st.magic_view = values
end

local M = { window = E.WindowPlane.SIDEBAR, CURSOR_FRAMES = 7 }

function M.max_column(row, last_entry)
    if row == 0 then return math.min(last_entry, 6) end
    return last_entry - 7
end

function M.move_column(sel, delta, last_entry)
    local maximum = M.max_column(sel.row, last_entry)
    local next_column = sel.column + delta
    if next_column < 0 then
        sel.column = math.max(0, maximum)
    elseif next_column <= maximum then
        sel.column = next_column
    else
        sel.column = 0
    end
end

function M.move_row(sel, delta, last_entry)
    if last_entry - 7 < 0 then return end
    local next_row = sel.row + delta
    if next_row < 0 then
        sel.row = 1
    elseif next_row == 2 then
        sel.row = 0
    else
        sel.row = next_row
    end
    if M.max_column(sel.row, last_entry) < sel.column then sel.row = 0 end
end

function M.selected(table_, sel)
    local index = sel.row * 7 + sel.column
    return table_.entries[index + 1], index
end

function M.initial_force(record, entry)
    if record.weapon_slot < 0 and record.weapon_code == entry.code + 1 then
        return record.force
    end
    return entry.force_limit
end

function M.new()
    return {
        sel = { row = 0, column = 0 },
        cursor_timer = 0,
        cursor_phase = 0,
        cursor_startup_heartbeats = 0,
    }
end

function M.enter(st, ctx)
    local record = active_record(ctx)
    if not (record.archetype < E.Archetype.CASTER_LIMIT and
            record.magic ~= 0) then
        local defense = Rules.defense(
            ctx.game.party(), ctx.game.slot_active_set(), record)
        return {
            transition = T.REPLACE,
            screen = E.MenuScreen.MAGICAL_DEFENSE,
            args = {
                magic_view = {
                    physical_defense_value = defense.physical,
                    physical_defense_row = defense.physical_row,
                    mana_defense_value = defense.mana,
                    mana_defense_row = defense.mana_row,
                },
            },
        }
    end

    st.table = ctx.game.spell_table(record.magic_table_index)
    if not st.table or st.table.entry_count == 0 then
        return { transition = T.POP }
    end
    st.last_entry = st.table.entry_count - 1
    st.cursor_timer = CURSOR_ENTRY_TIMER
    st.cursor_phase = CURSOR_ENTRY_PHASE
    st.cursor_sat_phase = CURSOR_ENTRY_PHASE
    st.cursor_startup_heartbeats = CURSOR_STARTUP_HEARTBEATS
    ctx.host.blank(E.DisplayBlank.OFF)
    ctx.redraw(R.FULL)
    -- 12 black frames, display back on at line 149 (emulator 8820..8832) [C emulator]
    ctx.host.blank(E.DisplayBlank.ON, 12, 149)
    local entry = M.selected(st.table, st.sel)
    st.force = M.initial_force(record, entry)
    publish_view(st, ctx, entry, st.force)
end

function M.input(st, events, ctx)
    next_cursor(st)
    local moved = false
    if events & P.LEFT ~= 0 then
        M.move_column(st.sel, -1, st.last_entry)
        moved = true
    elseif events & P.RIGHT ~= 0 then
        M.move_column(st.sel, 1, st.last_entry)
        moved = true
    elseif events & P.UP ~= 0 then
        M.move_row(st.sel, -1, st.last_entry)
        moved = true
    elseif events & P.DOWN ~= 0 then
        M.move_row(st.sel, 1, st.last_entry)
        moved = true
    end
    if moved then
        st.cursor_startup_heartbeats = 0
        st.cursor_timer = 0
        local entry = M.selected(st.table, st.sel)
        if entry then st.force = M.initial_force(active_record(ctx), entry) end
        publish_view(st, ctx, entry, st.force)
        ctx.redraw(R.SUBMENU)
        return
    end

    if events & (P.A | P.B | P.C) ~= 0 then
        local entry, index = M.selected(st.table, st.sel)
        if not entry then return end
        local record = ctx.game.party()[ctx.game.active_slot()]
        local force = M.initial_force(record, entry)
        -- The ROM stores the spell code (and force over the rounds of an
        -- equipped weapon) here, but the sidebar slot is only redrawn when
        -- Force returns: e6 9060-9259 keep the pistol and "12" while Force
        -- is open [C emulator]. The port's HUD reads the live record, so the
        -- write is applied in child_returned, before any other reader.
        st.pending_spell = {
            code = entry.code + 1,
            rounds = (record.weapon_slot >= 0 and record.weapon_code ~= 0)
                and force or nil,
        }
        return {
            transition = T.PUSH,
            screen = E.MenuScreen.MAGIC_FORCE,
            args = {
                entry = entry,
                table = st.table,
                force = force,
                row = st.sel.row,
                column = st.sel.column,
                table_entry_index = index,
                cursor_timer = st.cursor_timer,
                cursor_phase = st.cursor_phase,
            },
        }
    end
    if events & P.START ~= 0 then return { transition = T.POP } end

    local entry = M.selected(st.table, st.sel)
    if entry then publish_view(st, ctx, entry, st.force) end
end

function M.child_returned(st, child, result, ctx)
    if child ~= E.MenuScreen.MAGIC_FORCE then return end
    st.cursor_timer = result.cursor_timer or st.cursor_timer
    st.cursor_phase = result.cursor_phase or st.cursor_phase
    st.cursor_startup_heartbeats = 0
    local party = ctx.game.party()
    local record = party[ctx.game.active_slot()]
    local pending = st.pending_spell
    st.pending_spell = nil
    if pending then
        if pending.rounds then record.rounds = pending.rounds end
        record.weapon_code = pending.code
    end
    record.weapon_slot = -1
    record.force = result.force or record.force
    ctx.game.commit_party(party)
    st.force = record.force
    local entry = M.selected(st.table, st.sel)
    if entry then publish_view(st, ctx, entry, st.force) end
    ctx.redraw(R.SUBMENU | R.SIDEBAR_SLOT | R.COMBAT_BARS)
end

-- 0x10016: D-pad changes force; Down enters posture; A/B leave the setup.
local Force = { window = E.WindowPlane.SIDEBAR }
function Force.new(args)
    return {
        entry = args.entry,
        table = args.table,
        force = args.force,
        row = args.row,
        column = args.column,
        table_entry_index = args.table_entry_index,
        cursor_timer = args.cursor_timer or 0,
        cursor_phase = args.cursor_phase or 0,
    }
end

local function force_view(st, ctx)
    local party = ctx.game.party()
    local record = party[ctx.game.active_slot()]
    local values = Rules.presentation(
        record, party, ctx.game.slot_active_set(), st.table, st.entry,
        st.row, st.column, st.force)
    values.table_entry_index = st.table_entry_index
    publish_layout(values, { sel = { row = st.row, column = st.column },
                              cursor_phase = st.cursor_phase }, st.table)
    st.magic_view = values
end

function Force.enter(st, ctx)
    force_view(st, ctx)
    ctx.redraw(R.SUBMENU)
end

function Force.input(st, events, ctx)
    next_cursor(st)
    local delta = events & P.RIGHT ~= 0 and 1 or
                  (events & P.LEFT ~= 0 and -1 or 0)
    if delta ~= 0 then
        st.force = math.max(1, math.min(st.entry.force_limit, st.force + delta))
        local party = ctx.game.party()
        party[ctx.game.active_slot()].force = st.force
        ctx.game.commit_party(party)
        force_view(st, ctx)
        ctx.redraw(R.SUBMENU)
        return
    end

    if events & (P.A | P.B | P.C) ~= 0 then
        return { transition = T.POP, result = { force = st.force,
                 cursor_timer = st.cursor_timer, cursor_phase = st.cursor_phase } }
    end
    if events & P.DOWN ~= 0 then
        Posture.adjust(ctx, 0)
        return {
            transition = T.PUSH,
            screen = E.MenuScreen.MAGIC_POSTURE,
            args = {
                entry = st.entry,
                table = st.table,
                force = st.force,
                row = st.row,
                column = st.column,
                table_entry_index = st.table_entry_index,
                cursor_timer = st.cursor_timer,
                cursor_phase = st.cursor_phase,
            },
        }
    end
    force_view(st, ctx)
end

function Force.child_returned(st, child, result, ctx)
    st.cursor_timer = result.cursor_timer or st.cursor_timer
    st.cursor_phase = result.cursor_phase or st.cursor_phase
    if child == E.MenuScreen.MAGIC_POSTURE and result.finish then
        return { transition = T.POP, result = { force = st.force,
                 cursor_timer = st.cursor_timer, cursor_phase = st.cursor_phase } }
    end
end

-- 0x10722 posture loop: Up returns to force; A/B/C completes; Left/Right
-- changes the shared runner posture. The source loop has no local sound.
local MagicPosture = { window = E.WindowPlane.SIDEBAR }
function MagicPosture.new(args)
    return {
        entry = args.entry,
        table = args.table,
        force = args.force,
        row = args.row,
        column = args.column,
        table_entry_index = args.table_entry_index,
        cursor_timer = args.cursor_timer or 0,
        cursor_phase = args.cursor_phase or 0,
    }
end

function MagicPosture.enter(st, ctx)
    force_view(st, ctx)
end

function MagicPosture.input(st, events, ctx)
    next_cursor(st)
    local finish = events & (P.A | P.B | P.C) ~= 0
    if events & P.UP ~= 0 then
        Posture.adjust(ctx, 0)
        -- 0x10722 returns on Up before reading action buttons. Its 0xFF5A
        -- caller then checks the still-latched A/C/B edges, so Up+A/B/C
        -- completes the action while Up alone returns to force selection.
        return { transition = T.POP, result = { finish = finish,
                 cursor_timer = st.cursor_timer, cursor_phase = st.cursor_phase } }
    end
    if events & P.RIGHT ~= 0 then
        Posture.adjust(ctx, 1)
        force_view(st, ctx)
        ctx.redraw(R.COMBAT_BARS | R.POSTURE_BLOCK | R.SUBMENU)
        return
    end
    if events & P.LEFT ~= 0 then
        Posture.adjust(ctx, -1)
        force_view(st, ctx)
        ctx.redraw(R.COMBAT_BARS | R.POSTURE_BLOCK | R.SUBMENU)
        return
    end
    if finish then
        Posture.adjust(ctx, 0)
        return { transition = T.POP, result = { finish = true,
                 cursor_timer = st.cursor_timer, cursor_phase = st.cursor_phase } }
    end
    force_view(st, ctx)
end

return { Magic = M, Force = Force, Posture = MagicPosture }
