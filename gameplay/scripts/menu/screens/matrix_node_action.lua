-- Lua route for ROM 0x54662's Matrix node-action loop.
-- Start enters the 0x1822C deck loop; returning from it restores this node.
local E = require("enums")
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local MatrixSession = require("menu.matrix_session")

local M = { window = E.WindowPlane.OFF }
local CONFIRM = P.A | P.B | P.C

local function refresh(st)
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

function M.input(st, events, ctx)
    local state = st.matrix_state
    if not MatrixSession.is_online(state) then
        return { transition = T.POP, result = {} }
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
            if action and MatrixSession.select_node_action(state, action.id) then
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
