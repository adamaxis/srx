-- Lua route for ROM 0x54662's Matrix node-action loop.
-- Start enters the 0x1822C deck loop; returning from it restores this node.
local E = require("enums")
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local MatrixSession = require("menu.matrix_session")
local NodeActions = require("menu.rules.node_actions")
local MatrixText = require("menu.rules.matrix_text")

local M = { window = E.WindowPlane.OFF }
local CONFIRM = P.A | P.B | P.C

local function refresh(st)
    if st.shutdown then
        st.title, st.text, st.labels = "Matrix Message", MatrixText.SHUTDOWN, {}
        return
    end
    if st.transfer and st.transfer.phase == "search" then
        st.title = "Matrix Message"
        st.text = MatrixText.SEARCHING .. "|" ..
                  string.rep(".", st.transfer.dots)
        st.labels = {}
        return
    elseif st.transfer and st.transfer.phase == "confirm" then
        local file = st.transfer.file
        local name = NodeActions.file_name(file.id) or
                     ("File " .. tostring(file.id))
        st.title = MatrixText.DOWNLOAD_FILE
        st.text = name .. "|" .. MatrixText.FILE_METER .. " " ..
                  tostring(file.size)
        st.labels = { "Yes", "No" }
        st.row = st.transfer.choice
        return
    end
    if st.matrix_message then
        st.title, st.text, st.labels = "Matrix Message", st.matrix_message, {}
        return
    end
    local state = st.matrix_state
    local node = MatrixSession.current_node(state)
    if not node then
        st.title, st.text, st.labels = "Matrix", "No active node", {}
        return
    end

    if state.phase == "link_select" or state.phase == "goto" then
        st.title = state.phase == "goto" and "Go To Node" or
                   (state.pending_action == "enter_system" and
                    "Enter System" or "Leave Node")
        st.text = MatrixSession.node_cursor_text(state)
        st.labels = { "Confirm Destination" }
        st.row = 0
        return
    end

    st.title = MatrixSession.node_name(state, state.current_node)
    st.text = MatrixSession.node_summary(state)
    st.labels = {}
    local actions = MatrixSession.node_actions(state)
    for index, action in ipairs(actions) do
        st.labels[index] = action.label
    end
    if #st.labels == 0 then
        st.row = 0
    elseif st.row >= #st.labels then
        st.row = #st.labels - 1
    end
end

function M.new(args)
    return { matrix_state = args.matrix_state, system_id = args.system_id,
             row = 0, title = "Matrix", text = "", labels = {} }
end

function M.enter(st, ctx)
    if not MatrixSession.is_online(st.matrix_state) then
        return { transition = T.POP, result = {} }
    end
    refresh(st)
    ctx.redraw(R.FULL)
end

local function open_deck(st)
    return { transition = T.PUSH, screen = E.MenuScreen.CYBERDECK,
             args = { system_id = st.system_id, matrix_session = true,
                      return_to_node_action = true,
                      matrix_state = st.matrix_state } }
end

local function action_succeeds(st, ctx)
    local state = st.matrix_state
    local node = MatrixSession.current_node(state)
    local game = ctx.game
    local party = game.party()
    local runner = party[game.active_slot()]
    if not node or not runner then return false end
    local deck = game.deck_record()
    return NodeActions.gate_runner_action(
        runner, deck, state.mode, node, state,
        function(dice, target)
            return game.roll_successes(dice, target)
        end)
end

local function show_message(st, text, ctx)
    if st.transfer_return_row ~= nil then
        st.row = st.transfer_return_row
        st.transfer_return_row = nil
    end
    st.transfer = nil
    st.matrix_message = MatrixText.from_rom(text)
    refresh(st)
    ctx.redraw(R.FULL)
end

local function show_file_prompt(st, file, ctx)
    st.matrix_message = nil
    st.transfer = { phase = "confirm", file = file, choice = 0 }
    refresh(st)
    ctx.redraw(R.FULL)
end

local function start_transfer(st, ctx)
    local game = ctx.game
    local node = MatrixSession.current_node(st.matrix_state)
    local world = game.matrix_world_state()
    -- The Yes/No prompt has its own cursor. Keep the selected node action
    -- (Transfer Data) when returning from the prompt or its result message.
    st.transfer_return_row = st.row
    st.transfer_ram = st.transfer_ram or {}
    local result = NodeActions.begin_search(world.run, node, st.transfer_ram)
    st.matrix_message = nil
    if result then
        if result.text == NodeActions.TEXT.UPLOAD_DONE then
            game.commit_matrix_world_state(world)
            show_message(st, result.text, ctx)
        else
            show_file_prompt(st, result.file, ctx)
        end
        return
    end

    -- 0x54D3A prints a dot every 20 frames; after the fourth dot the ROM
    -- waits 9 more frames before checking story files and running detection.
    st.transfer = { phase = "search", frames = NodeActions.SEARCH_DOT_FRAMES,
                    dots = 0, return_row = st.transfer_return_row }
    refresh(st)
    ctx.redraw(R.FULL)
end

local function finish_transfer_search(st, ctx)
    local game = ctx.game
    local node = MatrixSession.current_node(st.matrix_state)
    local party = game.party()
    local runner = party[game.active_slot()]
    local deck = game.deck_record()
    local result = NodeActions.finish_search(
        node, runner, deck, nil,
        function(bound) return game.source_random_bound(bound) end)
    if result.file then
        show_file_prompt(st, result.file, ctx)
    else
        show_message(st, result.text or NodeActions.TEXT.NOTHING, ctx)
    end
end

local function advance_transfer_search(st, ctx)
    local transfer = st.transfer
    transfer.frames = transfer.frames - 1
    if transfer.frames > 0 then return end
    if transfer.dots < 4 then
        transfer.dots = transfer.dots + 1
        transfer.frames = transfer.dots < 4 and
                          NodeActions.SEARCH_DOT_FRAMES or 9
        refresh(st)
        ctx.redraw(R.FULL)
        return
    end
    finish_transfer_search(st, ctx)
end

local function store_transfer_file(st, ctx)
    local game = ctx.game
    local transfer = st.transfer
    local world = game.matrix_world_state()
    local record = game.deck_record()
    local slots = game.deck_slots()
    local slot, text = NodeActions.store(
        record, world.run, MatrixSession.current_node(st.matrix_state),
        st.transfer_ram, transfer.file,
        function(bound) return game.source_random_bound(bound) end)
    if slot ~= nil then
        game.deck_commit(record, slots)
        if transfer.file.objective then
            game.commit_matrix_world_state(world)
        end
    end
    show_message(st, text, ctx)
end

local function advance_transfer(st, events, ctx)
    local transfer = st.transfer
    if transfer.phase == "search" then
        advance_transfer_search(st, ctx)
        return true
    end
    if events & (P.UP | P.DOWN) ~= 0 then
        transfer.choice = 1 - transfer.choice
        st.row = transfer.choice
        refresh(st)
        ctx.redraw(R.FULL)
    elseif events & P.A ~= 0 then
        if transfer.choice == 1 then
            show_message(st, NodeActions.TEXT.DOWNLOAD_CANCELLED, ctx)
        else
            store_transfer_file(st, ctx)
        end
    end
    return true
end

function M.input(st, events, ctx)
    local state = st.matrix_state
    local game = ctx.game
    if not MatrixSession.is_online(state) then
        return { transition = T.POP, result = {} }
    end
    if st.transfer then
        advance_transfer(st, events, ctx)
        return
    end
    if st.shutdown then
        st.shutdown.frames = st.shutdown.frames - 1
        if st.shutdown.frames > 0 then return end
        st.shutdown.index = st.shutdown.index + 1
        local next_value = st.shutdown.values[st.shutdown.index]
        if next_value == nil then
            return { transition = T.EXIT, result = { matrix_crash = true } }
        end
        st.shutdown.frames = st.shutdown.step
        st.matrix_countdown = next_value
        refresh(st)
        ctx.redraw(R.FULL)
        return
    end
    if st.matrix_message then
        if events & P.EDGES ~= 0 then
            st.matrix_message = nil
            refresh(st)
            ctx.redraw(R.FULL)
        end
        return
    end
    if events & P.START ~= 0 then return open_deck(st) end

    if state.phase == "node" then
        local action_count = #MatrixSession.node_actions(state)
        if action_count > 0 and events & P.UP ~= 0 then
            st.row = (st.row - 1) % action_count
            ctx.redraw(R.SUBMENU)
            return
        elseif action_count > 0 and events & P.DOWN ~= 0 then
            st.row = (st.row + 1) % action_count
            ctx.redraw(R.SUBMENU)
            return
        end
        if events & CONFIRM ~= 0 then
            local action = MatrixSession.node_actions(state)[st.row + 1]
            if action and (action.id == "enter_system" or
                           action.id == "leave_node" or
                           action.id == "go_to_node" or
                           action.id == "cancel_alert" or
                           action.id == "crash_system" or
                           action.id == "turn_off_node" or
                           action.id == "erase" or
                           action.id == "lockout" or
                           action.id == "transfer_data") then
                -- 0x548A0 performs the shared 0x55F44/0xDBA runner gate
                -- before dispatch; Cancel Alert itself adds no second roll.
                if action_succeeds(st, ctx) then
                    local navigated = MatrixSession.select_node_action(
                        state, action.id)
                    if not navigated and action.id == "cancel_alert" then
                        st.matrix_message = MatrixText.from_rom(
                            NodeActions.cancel_alert(state).text)
                    elseif not navigated and action.id == "lockout" then
                        st.matrix_message = MatrixText.from_rom(
                            NodeActions.lockout().text)
                    elseif not navigated and action.id == "crash_system" then
                        -- The host supplies the run/field facts it knows and
                        -- commits the mutated typed state to the live world.
                        local world = game.matrix_world_state()
                        world.field = world.field or {}
                        local result = NodeActions.crash_system(
                            world.run, world.field, world.actors)
                        game.commit_matrix_world_state(world)
                        if result.stop_sfx then
                            ctx.host.stop_sound(result.stop_sfx)
                        end
                        st.matrix_message = MatrixText.from_rom(result.text)
                        st.shutdown = {
                            values = result.countdown,
                            step = result.step,
                            frames = result.step,
                            index = 1,
                        }
                        st.matrix_countdown = result.countdown[1]
                    elseif not navigated and action.id == "turn_off_node" then
                        local world = game.matrix_world_state()
                        world.field = world.field or {}
                        world.actors = world.actors or {}
                        local result = NodeActions.turn_off_node(
                            MatrixSession.current_node(state), world.field,
                            world.actors)
                        game.commit_matrix_world_state(world)
                        if result.stop_sfx then
                            ctx.host.stop_sound(result.stop_sfx)
                        end
                        st.matrix_message = MatrixText.from_rom(result.text)
                    elseif not navigated and action.id == "erase" then
                        local world = game.matrix_world_state()
                        local result = NodeActions.erase(
                            world.run, MatrixSession.current_node(state))
                        game.commit_matrix_world_state(world)
                        st.matrix_message = MatrixText.from_rom(result.text)
                    elseif not navigated and action.id == "transfer_data" then
                        start_transfer(st, ctx)
                    end
                else
                    st.matrix_message = MatrixText.from_rom(
                        NodeActions.TEXT.ACTION_FAILED)
                end
                refresh(st)
                ctx.redraw(R.FULL)
            end
        end
        return
    end

    if state.phase == "link_select" then
        if events & P.RIGHT ~= 0 then
            MatrixSession.step_link(state, true)
            refresh(st)
            ctx.redraw(R.FULL)
            return
        elseif events & P.LEFT ~= 0 then
            MatrixSession.step_link(state, false)
            refresh(st)
            ctx.redraw(R.FULL)
            return
        end
    elseif state.phase == "goto" then
        if events & P.RIGHT ~= 0 then
            MatrixSession.step_goto(state, true)
            refresh(st)
            ctx.redraw(R.FULL)
            return
        elseif events & P.LEFT ~= 0 then
            MatrixSession.step_goto(state, false)
            refresh(st)
            ctx.redraw(R.FULL)
            return
        end
    end

    if events & CONFIRM ~= 0 and
       MatrixSession.commit_node_selection(state) then
        st.row = 0
        refresh(st)
        ctx.redraw(R.FULL)
    end
end

function M.child_returned(st, child, result, ctx)
    if result and result.matrix_resumed then
        st.row = 0
        refresh(st)
        ctx.redraw(R.FULL)
    end
end

return M
