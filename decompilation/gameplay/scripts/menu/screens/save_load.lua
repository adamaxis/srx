-- Host state snapshots are versioned JSON files behind the narrow game API.
-- Lua owns the screens and feedback; it does not reproduce cartridge SRAM.
local E = require("enums")
local Text = require("pocket_secretary")
local P, R, T, MM, SL = E.Pad, E.MenuRegion, E.MenuTransition,
                         E.MenuMessage, E.SaveLoadOption
local CONFIRM = P.A | P.B | P.C

-- ROM 0xF000 writes the 0xC2600 rows at 0xC518 and its cursor at 0xC514 [C].
local SAVE_LOAD_LAYOUT = {
    label_x = 96, label_y = 80,
    cursor_x = 80, cursor_y = 80,
    row_spacing = 16,
}
-- ROM 0xF188 writes 0x1B6518 at 0xC39C and moves the cursor from 0xC398 [C].
local SLOT_LAYOUT = {
    title_x = 88, title_y = 32,
    separator_x = 88, separator_y = 40,
    label_x = 112, label_y = 56,
    cursor_x = 96, cursor_y = 56,
    row_spacing = 16,
}

local function wrap(value, delta, maximum)
    value = value + delta
    if value < 0 then return maximum end
    if value > maximum then return 0 end
    return value
end

local function wait(message, text, done)
    return {
        transition = T.PUSH,
        screen = E.MenuScreen.MESSAGE,
        args = {
            message = message,
            text = text,
            mask = P.EDGES,
            data = { done = done == true },
        },
    }
end

local SaveLoad = { window = E.WindowPlane.SIDEBAR }
function SaveLoad.new()
    return {
        row = SL.SAVE,
        title = "",
        labels = Text.save_load_options(),
        layout = SAVE_LOAD_LAYOUT,
    }
end
function SaveLoad.enter(st, ctx)
    -- 0xEFC4: tst.w FFDF40; nonzero -> text 0xC26C3, checked before FFDF84 [C].
    if ctx.game.scene_mode() ~= E.SceneMode.NONE then
        st.gated = true
        return wait(MM.SAVE_OUTSIDE_ONLY,
                    "Save/Load only works in an outside area.")
    end
    if ctx.game.in_combat() then        -- 0xEFE6: tst.w FFDF84 [C]
        st.gated = true
        return wait(MM.SAVE_IN_COMBAT,
                    "Save and load are unavailable during an encounter.")
    end
    ctx.redraw(R.SUBMENU)
end
function SaveLoad.input(st, events, ctx)
    if events & P.UP ~= 0 then
        st.row = wrap(st.row, -1, 1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.DOWN ~= 0 then
        st.row = wrap(st.row, 1, 1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & CONFIRM ~= 0 then
        return {
            transition = T.PUSH,
            screen = E.MenuScreen.SAVE_SLOT,
            args = { op = st.row },
        }
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end
function SaveLoad.child_returned(st, _, result)
    if st.gated then return { transition = T.POP } end
    if result.done then
        return { transition = T.POP, result = { exit_menu = true } }
    end
    return { transition = T.POP }
end

local Slot = { window = E.WindowPlane.SIDEBAR }
function Slot.new(args)
    return {
        op = args.op, row = 0, title = "",
        labels = Text.save_load_slots(),
        layout = SLOT_LAYOUT,
    }
end
function Slot.enter(st, ctx)
    st.title = Text.save_load_slot_titles()[st.op + 1]
    ctx.redraw(R.SUBMENU)
end
function Slot.input(st, events, ctx)
    if events & P.UP ~= 0 then
        st.row = wrap(st.row, -1, 1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.DOWN ~= 0 then
        st.row = wrap(st.row, 1, 1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.START ~= 0 then
        return { transition = T.POP, result = {} }
    end
    if events & CONFIRM == 0 then return end

    if st.op == SL.SAVE then
        local saved = ctx.game.save_game(st.row)
        return wait(MM.GAME_SAVED,
                    saved and "Game saved." or "Could not write this save slot.",
                    saved)
    end
    local loaded = ctx.game.load_game(st.row)
    return wait(loaded and MM.LOAD_OK or MM.LOAD_EMPTY,
                loaded and "Game loaded." or "This save slot is empty or invalid.",
                loaded)
end
function Slot.child_returned(_, _, result)
    if result.data and result.data.done then
        return { transition = T.POP, result = { done = true } }
    end
end

return { SaveLoad = SaveLoad, Slot = Slot }
