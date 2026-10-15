-- ROM 0x599DA: eight-row developer menu. The parent row is normally unlocked
-- by the title input sequence at 0x1BAC; Qt currently keeps it exposed for
-- development. Five source effects are wired to typed host services below.
local E = require("enums")
local Grid = require("menu.rules.random_grid")
local P, D, R, T, St = E.Pad, E.DebugOption, E.MenuRegion,
                         E.MenuTransition, E.RunnerStatus

local M = {
    window = E.WindowPlane.SIDEBAR,
    title = "Use at your own risk!!",
    labels = {
        "Nuyen",
        "Spells (pending)",
        "Test Deck (pending)",
        "Pick System (pending)",
        "Heal All",
        "+10 Karma",
        "Contacts",
        "Walk Fast",
    },
    nuyen = 0x3D090,
    contacts = 0x1C7F,
}

local function move_row(row, delta)
    return (row + delta + 8) % 8
end

M.effects = {
    [D.NUYEN] = function(ctx)
        ctx.game.set_nuyen(M.nuyen) -- ROM 0x59B28 writes 250,000.
    end,
    [D.HEAL_ALL] = function(ctx)
        local party = ctx.game.party()
        for slot = 0, 2 do
            local runner = party[slot]
            if runner then
                -- ROM 0x59BCC clears +0x10 bits 6 and 4, then fills
                -- +0x4C and +0x9C for all three live-record slots.
                runner.status = runner.status &
                                ~(St.DEAD | St.DOWN)
                runner.physical = 10
                runner.stun = 10
            end
        end
        ctx.game.commit_party(party)
    end,
    [D.KARMA] = function(ctx)
        local party = ctx.game.party()
        for slot = 0, 2 do
            local runner = party[slot]
            if runner then
                runner.karma = ((runner.karma or 0) + 10) & 0xFF
            end
        end
        ctx.game.commit_party(party) -- ROM 0x59C2A increments +0x8D.
    end,
    [D.CONTACTS] = function(ctx)
        ctx.game.set_contact_flags(M.contacts) -- ROM 0x59C48 writes FFFC2C.
    end,
    [D.WALK_FAST] = function(ctx)
        ctx.game.set_walk_fast() -- ROM 0x59C54 writes FFE168 = 0xFF.
    end,
}

function M.new()
    return {
        row = 0,
        title = M.title,
        labels = M.labels,
        grid_cells = Grid.cells,
        grid_effect_frames = 0,
    }
end

function M.enter(st, ctx)
    Grid.reset()
    st.grid_effect_frames = 0
    ctx.redraw(R.SUBMENU)
end

function M.input(st, events, ctx)
    st.grid_effect_frames = Grid.advance(ctx) -- ROM 0xE660 runs each frame.
    if events & P.UP ~= 0 then
        st.row = move_row(st.row, -1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.DOWN ~= 0 then
        st.row = move_row(st.row, 1)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.A ~= 0 then
        local effect = M.effects[st.row]
        if effect then
            effect(ctx)
            return { transition = T.POP }
        end
        -- These source handlers need the pending spell, deck, Matrix, and
        -- movement-state host services before they can safely be enabled.
        return
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE) -- 0x391A queues sound 0x25.
        ctx.game.reselect_leader()
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end

return M
