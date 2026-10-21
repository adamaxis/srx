-- Lua route for ROM 0x54662's Matrix node-action loop.
-- Start enters the 0x1822C deck loop; returning from it restores this node.
local E = require("enums")
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local MatrixSession = require("menu.matrix_session")
local NodeActions = require("menu.rules.node_actions")
local MatrixText = require("menu.rules.matrix_text")
local MatrixRules = require("menu.rules.matrix")
local MatrixCombat = require("menu.rules.matrix_combat")
local DeckRules = require("menu.rules.deck")
local MatrixPrograms = require("menu.rules.matrix_programs")

local M = { window = E.WindowPlane.OFF }
local CONFIRM = P.A | P.B | P.C

local function refresh(st)
    st.matrix_node_status = nil
    st.matrix_traversal_view = nil
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
    st.matrix_node_status = MatrixSession.node_status(state)
    if st.traversal_view then
        st.matrix_traversal_view = MatrixSession.traversal_view(
            state, st.active_program_label,
            st.active_program_status_label)
    end
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

local function update_deck_view(st, ctx)
    local record = ctx.game.deck_record()
    local slots = ctx.game.deck_slots()
    if type(record) ~= "table" or type(slots) ~= "table" then return end
    st.program_ratings = record.ratings
    st.active_programs = record.active
    local slot_status = {}
    for slot = 0, 4 do
        local entry = slots[slot]
        slot_status[slot] = entry and entry.status or 0
    end
    st.program_slot_status = slot_status

    local selected = MatrixSession.selected_program_slot(st.matrix_state)
    local program = record.active and record.active[selected]
    local active_slot = slots[selected]
    local rating = program and record.ratings and record.ratings[program] or 0
    local name = program and program >= 0 and DeckRules.PROGRAM_NAMES[program]
    st.active_program_label = name and rating > 0
        and (name .. "-" .. tostring(rating)) or ""
    st.active_program_status_label = program and program >= 0 and active_slot
        and MatrixSession.program_status_label(active_slot.phase) or ""
end

local function start_combat_program(st, events, ctx, program)
    local state = st.matrix_state
    if state.phase ~= "node" or (events & CONFIRM) == 0 then return false end
    local game = ctx.game
    local record = game.deck_record()
    local slots = game.deck_slots()
    if type(record) ~= "table" or type(record.active) ~= "table" or
       type(slots) ~= "table" then
        return false
    end

    local slot_index = MatrixSession.selected_program_slot(state)
    local visual = slots[slot_index]
    if record.active[slot_index] ~= program or
       type(visual) ~= "table" or (visual.status or 0) < DeckRules.READY or
       not ((visual.phase or 0) == 1 or (visual.phase or 0) >= 6) then
        return false
    end

    local party = game.party()
    local world
    if program == MatrixCombat.PROGRAM.ATTACK or
       program == MatrixCombat.PROGRAM.SLOW then
        world = MatrixSession.world_state(game)
        if type(world.karma_counter) ~= "number" then return false end
    end
    local party_karma = {}
    for member_index = 0, 2 do
        local member = party[member_index]
        if member then party_karma[member_index] = member.karma end
    end
    local runner = party[game.active_slot()]
    local node = MatrixSession.current_node(state)
    if not runner or not node then return false end

    local mode = MatrixSession.program_mode_from_buttons(events)
    state.mode = mode
    local attributes = runner.attributes or {}
    local skills = runner.skills or {}
    local decker = {
        d81c = (((attributes[1] or 0) + (attributes[4] or 0)) & 0xFF) >> 1,
        computer = skills[7] or 0,
    }
    local deck = {
        mpcp = record.mpcp or 0,
        response = record.response or 0,
        evasion = record.persona and record.persona[2] or 0,
        ratings = record.ratings or {},
    }
    local combat_state = {
        alert = state.alert,
        mode = mode,
        smoke = state.smoke or 0,
        mirrors = state.mirrors or 0,
        karma_counter = world and world.karma_counter or 0,
        passive = world and world.passive_ic_counter or 0,
    }
    local net, result_state = MatrixCombat.run_program(
        deck, decker, node, combat_state, program,
        function(dice, target)
            return game.roll_successes(dice, target)
        end)
    if net > 0 then
        if program == MatrixCombat.PROGRAM.ATTACK then
            MatrixCombat.attack(node, combat_state, net, party)
        elseif program == MatrixCombat.PROGRAM.SLOW then
            MatrixCombat.slow(deck, node, combat_state, net, party)
        else
            MatrixCombat.degrade(node, net)
        end
    end
    if world then
        world.karma_counter = combat_state.karma_counter
        if net > 0 then
            world.passive_ic_counter = combat_state.passive
        end
        local party_karma_changed = false
        for member_index = 0, 2 do
            local member = party[member_index]
            if member and party_karma[member_index] ~= member.karma then
                party_karma_changed = true
                break
            end
        end
        if party_karma_changed and type(game.commit_party) == "function" then
            game.commit_party(party)
        end
        MatrixSession.commit_world_state(game, world)
    end
    visual.phase = result_state
    game.deck_commit(record, slots)
    update_deck_view(st, ctx)
    refresh(st)
    ctx.redraw(R.FULL)
    return true
end

local function start_attack(st, events, ctx)
    return start_combat_program(st, events, ctx,
                                MatrixCombat.PROGRAM.ATTACK)
end

local function start_slow(st, events, ctx)
    return start_combat_program(st, events, ctx, MatrixCombat.PROGRAM.SLOW)
end

local function start_degrade(st, events, ctx)
    return start_combat_program(st, events, ctx,
                                MatrixCombat.PROGRAM.DEGRADE)
end

local function start_medic(st, events, ctx)
    local state = st.matrix_state
    if state.phase ~= "node" or (events & CONFIRM) == 0 then return false end
    local game = ctx.game
    local record = game.deck_record()
    local slots = game.deck_slots()
    if type(record) ~= "table" or type(record.active) ~= "table" or
       type(slots) ~= "table" then
        return false
    end
    local slot_index = MatrixSession.selected_program_slot(state)
    local visual = slots[slot_index]
    if record.active[slot_index] ~= MatrixPrograms.P.MEDIC or
       type(visual) ~= "table" or (visual.status or 0) < DeckRules.READY or
       not ((visual.phase or 0) == 1 or (visual.phase or 0) >= 6) then
        return false
    end

    state.mode = MatrixSession.program_mode_from_buttons(events)
    local deck = {
        persona = record.persona_condition or 0,
        ratings = record.ratings or {},
    }
    local result = MatrixPrograms.medic(
        deck, function(dice, target)
            return game.roll_successes(dice, target)
        end)
    record.persona_condition = deck.persona
    visual.phase = result.state or 7
    game.deck_commit(record, slots)
    update_deck_view(st, ctx)
    refresh(st)
    ctx.redraw(R.FULL)
    return true
end

local function start_analyze(st, events, ctx)
    local state = st.matrix_state
    if not st.traversal_view or state.phase ~= "node" or
       state.analyze_transition or (events & CONFIRM) == 0 then
        return false
    end
    local game = ctx.game
    local record = game.deck_record()
    local slots = game.deck_slots()
    if type(record) ~= "table" or type(record.active) ~= "table" or
       type(slots) ~= "table" then
        return false
    end
    local slot_index = MatrixSession.selected_program_slot(state)
    local program = record.active[slot_index]
    local visual = slots[slot_index]
    if program ~= MatrixPrograms.P.ANALYZE or type(visual) ~= "table" or
       (visual.status or 0) < DeckRules.READY or
       not ((visual.phase or 0) == 1 or (visual.phase or 0) >= 6) then
        return false
    end

    local mode = MatrixSession.program_mode_from_buttons(events)
    state.mode = mode
    local party = game.party()
    local runner = party[game.active_slot()]
    local node = MatrixSession.current_node(state)
    if not runner or not node then return false end
    local attributes = runner.attributes or {}
    local skills = runner.skills or {}
    local persona = record.persona or {}
    local deck = {
        mpcp = record.mpcp or 0,
        response = record.response or 0,
        sensor = persona[3] or 0,
        ratings = record.ratings or {},
    }
    -- ROM 0xD81C is the wrapped byte sum of leader attributes +0x78/+0x7B,
    -- divided by two; Computer is leader skill +0x88.
    local decker = {
        d81c = (((attributes[1] or 0) + (attributes[4] or 0)) & 0xFF) >> 1,
        computer = skills[7] or 0,
    }
    local action_state = {
        alert = state.alert,
        mode = mode,
        smoke = 0,
    }
    local result = MatrixPrograms.analyze(
        deck, decker, node, action_state,
        function(dice, target) return game.roll_successes(dice, target) end)
    if result.fail then
        visual.phase = 7
        DeckRules.unload(record, slots, slot_index)
        game.deck_commit(record, slots)
        update_deck_view(st, ctx)
        refresh(st)
        ctx.redraw(R.FULL)
        return true
    end

    MatrixSession.merge_current_node_knowledge(state, result.level)
    MatrixSession.begin_analyze_transition(
        state, slot_index, result.net > 0 and 6 or 10)
    -- The world loop calls the profile service after 0x1C754 dispatches the
    -- selected program, so the input VBlank also publishes its first phase.
    MatrixSession.advance_analyze_transition(state)
    refresh(st)
    ctx.redraw(R.FULL)
    return true
end

local function advance_analyze(st, ctx)
    local advanced, completed = MatrixSession.advance_analyze_transition(
        st.matrix_state)
    if not advanced then return false end
    if completed then
        local slots = ctx.game.deck_slots()
        local visual = slots[completed.slot]
        if visual then visual.phase = completed.result_state end
        ctx.game.deck_commit(ctx.game.deck_record(), slots)
        update_deck_view(st, ctx)
    end
    refresh(st)
    ctx.redraw(R.FULL)
    return true
end

local function service_program_loading(st, ctx)
    local state = st.matrix_state
    if not MatrixSession.is_online(state) then return false end
    local slots = ctx.game.deck_slots()
    if not MatrixSession.advance_program_loading(state, slots) then
        return false
    end
    ctx.game.deck_commit(ctx.game.deck_record(), slots)
    return true
end

local function show_random_alert(st, ctx)
    local state = st.matrix_state
    local random_bound = ctx.game.matrix_alert_random_bound or
                         ctx.game.source_random_bound
    if not random_bound then return false end
    if not NodeActions.random_alert(state, function(bound)
        return random_bound(bound)
    end) then
        return false
    end

    local previous = state.alert
    local alert, overload = MatrixRules.raise_alert(
        previous, 1, state.overload_armed)
    state.alert = alert
    -- 0x1B706 arms FFDF84 before the Matrix alert probe. 0x56116 clears
    -- that marker below Active Alert and reinitializes the world effect while
    -- it remains active, allowing 0x19B40 to select hidden-node IC art.
    state.in_alert = alert == MatrixRules.ACTIVE
    -- 0x1989A returns from its overload branch before the FFE189 sound-latch
    -- check; the overload path has no separate alert SFX.
    if not overload and alert ~= previous then
        local sound = alert < 0 and E.SoundId.MATRIX_NO_ALERT or
                      alert == 0 and E.SoundId.MATRIX_PASSIVE_ALERT or
                      E.SoundId.MATRIX_ACTIVE_ALERT
        ctx.sound(sound)
    end

    if overload then
        st.matrix_message_kind = nil
        st.matrix_message = nil
        st.message_delay_frames = nil
        st.matrix_transition = {
            frames_remaining = MatrixRules.EXIT_TRANSITION_FRAMES,
            elapsed_frames = 0,
            fade_step = 0,
        }
    else
        st.matrix_message_kind = "system_alert"
        st.matrix_message = MatrixText.SYSTEM_ALERT
        st.message_delay_frames = 25
    end
    refresh(st)
    ctx.redraw(R.FULL)
    return true
end

-- ROM 0x546CC rebuilds the node-action view before 0x54A8E consumes one RNG
-- value. Keep the same order: draw the current node, then let the event
-- replace it with its notice if FFF0A8 crosses the random threshold.
local function rebuild_node_view(st, ctx)
    refresh(st)
    ctx.redraw(R.FULL)
    show_random_alert(st, ctx)
end

function M.new(args)
    return { matrix_state = args.matrix_state or MatrixSession.current_state(),
             system_id = args.system_id,
             node_id = args.node_id,
             node_ic_condition = args.node_ic_condition,
             return_to_field = args.return_to_field == true,
             traversal_view = args.traversal_view == true,
             row = 0, title = "Matrix", text = "", labels = {} }
end

function M.enter(st, ctx)
    if not st.matrix_state then
        st.matrix_state = MatrixSession.current_state()
    end
    if not st.matrix_state then
        if type(st.system_id) == "number" and st.system_id % 1 == 0 and
           st.system_id >= 0 and st.system_id <= 24 then
            local world = MatrixSession.world_state(ctx.game)
            local started, matrix_state = MatrixSession.begin(
                st.system_id, world.field and world.field.alarm,
                st.node_id, st.node_ic_condition)
            if started then st.matrix_state = matrix_state end
        end
    end
    if not MatrixSession.is_online(st.matrix_state) then
        return { transition = T.POP, result = {} }
    end
    -- ROM 0x54662 replaces FFFB30's knowledge nibble for the selected
    -- map cell with 4 before the node-action HUD is rendered.
    MatrixSession.set_current_node_knowledge(st.matrix_state, 4)
    if st.traversal_view then
        MatrixSession.begin_world_entry(st.matrix_state)
        update_deck_view(st, ctx)
    end
    rebuild_node_view(st, ctx)
end

local function open_deck(st)
    return { transition = T.PUSH, screen = E.MenuScreen.CYBERDECK,
             args = { system_id = st.system_id, matrix_session = true,
                      return_to_node_action = true,
                      return_to_field = st.return_to_field,
                      traversal_view = st.traversal_view,
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
    local world = MatrixSession.world_state(game)
    -- The Yes/No prompt has its own cursor. Keep the selected node action
    -- (Transfer Data) when returning from the prompt or its result message.
    st.transfer_return_row = st.row
    st.transfer_ram = st.transfer_ram or {}
    local result = NodeActions.begin_search(
        world.run, node, st.transfer_ram, st.system_id)
    st.matrix_message = nil
    if result then
        if result.text == NodeActions.TEXT.UPLOAD_DONE then
            MatrixSession.commit_world_state(game, world)
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
    local world = MatrixSession.world_state(game)
    local special = world.datastore
    if special then special.system_id = st.system_id end
    local party = game.party()
    local runner = party[game.active_slot()]
    local deck = game.deck_record()
    local result = NodeActions.finish_search(
        node, runner, deck, special,
        function(bound) return game.source_random_bound(bound) end)
    if result.file then
        show_file_prompt(st, result.file, ctx)
    else
        if result.story then MatrixSession.commit_world_state(game, world) end
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
    local world = MatrixSession.world_state(game)
    local record = game.deck_record()
    local slots = game.deck_slots()
    local slot, text = NodeActions.store(
        record, world.run, MatrixSession.current_node(st.matrix_state),
        st.transfer_ram, transfer.file,
        function(bound) return game.source_random_bound(bound) end)
    if slot ~= nil then
        game.deck_commit(record, slots)
        if transfer.file.objective then
            MatrixSession.commit_world_state(game, world)
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

local function advance_matrix_transition(st, ctx)
    local transition = st.matrix_transition
    transition.frames_remaining = transition.frames_remaining - 1
    transition.elapsed_frames = transition.elapsed_frames + 1
    transition.fade_step = math.min(
        MatrixRules.EXIT_FADE_STEPS,
        transition.elapsed_frames // 2)

    if transition.frames_remaining <= 0 then
        st.matrix_transition = nil
        if transition.exit_when_complete then
            MatrixSession.finish(st.matrix_state)
            return { transition = T.EXIT,
                     result = { matrix_crash = true } }
        end
        st.matrix_message_kind = "system_overload"
        st.matrix_message = MatrixText.SYSTEM_OVERLOAD
        st.message_delay_frames = 30
        refresh(st)
    end
    ctx.redraw(R.FULL)
    return nil
end

function M.input(st, events, ctx)
    local state = st.matrix_state
    local game = ctx.game
    if not MatrixSession.is_online(state) then
        return { transition = T.POP, result = {} }
    end
    if st.traversal_view and MatrixSession.advance_world_entry(state) then
        refresh(st)
        ctx.redraw(R.FULL)
        return
    end
    if service_program_loading(st, ctx) then
        update_deck_view(st, ctx)
        refresh(st)
        ctx.redraw(R.FULL)
    end
    if st.traversal_view and state.phase == "node" and
       not st.matrix_transition and not st.transfer and not st.shutdown and
       not st.matrix_message and
       MatrixSession.advance_world_effect_frame(state) then
        refresh(st)
        ctx.redraw(R.FULL)
    end
    if state.analyze_transition then
        advance_analyze(st, ctx)
        return
    end
    if st.matrix_transition then
        return advance_matrix_transition(st, ctx)
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
            -- ROM 0x54B34 calls 0x18776, then 0x26A00, after the 60-frame
            -- zero. Share the 10-frame fade + 54-frame display handoff used
            -- by overload; the source handoff artwork remains unported.
            st.shutdown = nil
            st.matrix_transition = {
                frames_remaining = MatrixRules.EXIT_TRANSITION_FRAMES,
                elapsed_frames = 0,
                fade_step = 0,
                exit_when_complete = true,
            }
            ctx.redraw(R.FULL)
            return
        end
        st.shutdown.frames = st.shutdown.step
        st.matrix_countdown = next_value
        refresh(st)
        ctx.redraw(R.FULL)
        return
    end
    if st.matrix_message then
        if st.message_delay_frames and st.message_delay_frames > 0 then
            st.message_delay_frames = st.message_delay_frames - 1
            return
        end
        if events & P.EDGES ~= 0 then
            local message_kind = st.matrix_message_kind
            st.matrix_message = nil
            st.matrix_message_kind = nil
            st.message_delay_frames = nil
            if message_kind == "system_overload" then
                MatrixSession.finish(state)
                return { transition = T.EXIT,
                         result = { matrix_overload = true } }
            elseif message_kind == "system_alert" then
                -- 0x54A8E returns to the already-built node screen after the
                -- notice; it does not consume a second random draw.
                refresh(st)
                ctx.redraw(R.FULL)
            else
                rebuild_node_view(st, ctx)
            end
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
        if events & (P.LEFT | P.RIGHT) ~= 0 then
            local record = game.deck_record()
            local slots = game.deck_slots()
            if type(record) ~= "table" or type(record.active) ~= "table" or
               type(slots) ~= "table" then
                return
            end
            if MatrixSession.step_program_slot(
                    state, events & P.RIGHT ~= 0, record.active, slots) then
                game.deck_commit(record, slots)
                update_deck_view(st, ctx)
                refresh(st)
                ctx.redraw(R.FULL)
            end
            return
        end
        if start_attack(st, events, ctx) then return end
        if start_slow(st, events, ctx) then return end
        if start_degrade(st, events, ctx) then return end
        if start_medic(st, events, ctx) then return end
        if start_analyze(st, events, ctx) then return end
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
                        local world = MatrixSession.world_state(game)
                        world.field = world.field or {}
                        local result = NodeActions.crash_system(
                            world.run, world.field, world.actors)
                        MatrixSession.commit_world_state(game, world)
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
                        local world = MatrixSession.world_state(game)
                        world.field = world.field or {}
                        world.actors = world.actors or {}
                        local result = NodeActions.turn_off_node(
                            MatrixSession.current_node(state), world.field,
                            world.actors)
                        MatrixSession.commit_world_state(game, world)
                        if result.stop_sfx then
                            ctx.host.stop_sound(result.stop_sfx)
                        end
                        st.matrix_message = MatrixText.from_rom(result.text)
                    elseif not navigated and action.id == "erase" then
                        local world = MatrixSession.world_state(game)
                        local result = NodeActions.erase(
                            world.run, MatrixSession.current_node(state),
                            st.system_id)
                        MatrixSession.commit_world_state(game, world)
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
        if st.traversal_view then
            MatrixSession.set_current_node_knowledge(state, 4)
            MatrixSession.begin_world_entry(state)
        end
        st.row = 0
        rebuild_node_view(st, ctx)
    end
end

function M.child_returned(st, child, result, ctx)
    if result and result.matrix_resumed then
        if st.traversal_view then update_deck_view(st, ctx) end
        st.row = 0
        rebuild_node_view(st, ctx)
    end
end

return M
