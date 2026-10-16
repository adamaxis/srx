-- ROM 0x599DA: eight-row developer menu. The parent row is normally unlocked
-- by the title input sequence at 0x1BAC; Qt currently keeps it exposed for
-- development. Six source effects are wired to typed host services below.
local E = require("enums")
local Grid = require("menu.rules.random_grid")
local MatrixSession = require("menu.matrix_session")
local P, D, R, T, St = E.Pad, E.DebugOption, E.MenuRegion,
                         E.MenuTransition, E.RunnerStatus

local M = {
    window = E.WindowPlane.SIDEBAR,
    title = "Use at your own risk!!",
    labels = {
        "Nuyen",
        "Spells",
        "Test Deck",
        "Pick System",
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
    [D.SPELLS] = function(ctx)
        -- ROM 0x59B34 is a no-op for non-magic archetypes. For a caster it
        -- replaces that runner's editable spell row with IDs 13..0 at force 4.
        if ctx.game.leader_archetype() >= E.Archetype.CASTER_LIMIT then
            return
        end
        local party = ctx.game.party()
        local runner = party[ctx.game.active_slot()]
        if not runner then return end
        local entries = {}
        for code = 13, 0, -1 do
            entries[#entries + 1] = { code = code, force_limit = 4 }
        end
        ctx.game.set_spell_table(runner.magic_table_index, {
            entry_count = #entries,
            entries = entries,
        })
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
    [D.TEST_DECK] = function(ctx)
        ctx.game.install_test_deck() -- ROM 0x59B7E seeds the deck and passcodes.
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
            ctx.game.consume_debug_unlock() -- ROM 0x59A64 clears FFF101.
            effect(ctx)
            return { transition = T.POP }
        end
        if st.row == D.PICK_SYSTEM then
            ctx.game.consume_debug_unlock() -- ROM 0x59A64 clears FFF101.
            return { transition = T.REPLACE,
                     screen = E.MenuScreen.DEBUG_PICK_SYSTEM }
        end
        return
    end
    if events & P.C ~= 0 then
        ctx.sound(E.MenuSound.MOVE) -- 0x391A queues sound 0x25.
        ctx.game.reselect_leader()
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end

-- ROM 0x59A9A: Up and Down change the system number within 0..24; Start
-- returns to the debug menu. A plays the jack-in sound and waits 20 VBlanks.
-- The current host route seeds that system's Lua data and opens the online
-- Cyberdeck model; the ROM's 0x181F8/0x1822C world loop remains unported.
local Pick = { window = E.WindowPlane.SIDEBAR, MAX_SYSTEM = 0x18 }

local function show_system(st)
    st.text = string.format("%02d", st.system)
end

function Pick.new()
    return { system = 0, title = "Pick System", labels = {}, text = "00" }
end

function Pick.enter(st, ctx)
    show_system(st)
    ctx.redraw(R.SUBMENU)
end

function Pick.input(st, events, ctx)
    if st.wait_frames then
        st.wait_frames = st.wait_frames - 1
        if st.wait_frames <= 0 then
            st.wait_frames = nil
            local started, session = MatrixSession.begin(st.system)
            if not started then return end
            local world = ctx.game.matrix_world_state()
            if world.run and
               MatrixSession.activate_run_for_system(st.system, world.run) then
                ctx.game.commit_matrix_world_state(world)
            end
            return { transition = T.REPLACE,
                     screen = E.MenuScreen.MATRIX_NODE_ACTION,
                     args = { system_id = st.system,
                              matrix_session = true,
                              matrix_state = session } }
        end
        return
    end

    if events & P.UP ~= 0 then
        if st.system > 0 then st.system = st.system - 1 end
        show_system(st)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.DOWN ~= 0 then
        if st.system < Pick.MAX_SYSTEM then st.system = st.system + 1 end
        show_system(st)
        ctx.redraw(R.SUBMENU)
        return
    end
    if events & P.A ~= 0 then
        ctx.sound(E.MenuSound.JACK_IN)
        st.wait_frames = 20
        return
    end
    if events & P.START ~= 0 then return { transition = T.POP } end
end

return { Debug = M, Pick = Pick }
