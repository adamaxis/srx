-- Host state snapshots are versioned JSON files behind the narrow game API.
-- Lua owns the screens and feedback; it does not reproduce cartridge SRAM.
local E = require("enums")
local P, R, T, MM, SL = E.Pad, E.MenuRegion, E.MenuTransition,
                         E.MenuMessage, E.SaveLoadOption
local CONFIRM = P.A | P.B | P.C

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
        title = "Save or Load",
        labels = { "Save Game", "Load Game" },
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
    return { op = args.op, row = 0, title = "Select Save Slot", labels = {} }
end
function Slot.enter(st, ctx)
    local function label(slot)
        local status = ctx.game.save_status(slot)
        local prefix = "Game " .. tostring(slot + 1) .. ": "
        if status == 1 then return prefix .. "Saved" end
        if status == 2 then return prefix .. "Unreadable" end
        if status == 3 then return prefix .. "Incompatible Version" end
        return prefix .. "Empty"
    end
    st.title = st.op == SL.SAVE and "Save Game" or "Load Game"
    st.labels = { label(0), label(1) }
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
