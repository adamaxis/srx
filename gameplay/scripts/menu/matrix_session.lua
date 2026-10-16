-- Matrix session state is Lua-owned. The source map and node records are
-- imported offline from the ROM into data/matrix_systems.lua.
local Systems = require("data.matrix_systems")
local Rules = require("menu.rules.matrix")
local Movement = require("menu.rules.matrix_move")

local M = { active = nil }

local function typed_node(source)
    if not source then return nil end
    return {
        function_id = source.function_id,
        fn = source.function_id,
        file_class_id = source.file_class_id,
        x = source.x,
        y = source.y,
        colour = source.security_color,
        security = source.security,
        links = source.links,
        ic_condition = source.ic_condition,
        ic_reaction = source.ic_reaction,
        ic_rating = source.ic_rating,
        ic_rating2 = source.ic_rating2,
        ic_code = source.ic_combo_id,
    }
end

function M.begin(system_id)
    local source = Systems[system_id]
    if type(source) ~= "table" then return false end

    local map = {}
    for index, value in ipairs(source.map) do map[index - 1] = value end
    local nodes, node_at = {}, {}
    for index, source_node in pairs(source.nodes) do
        local node = typed_node(source_node)
        nodes[index] = node
        node_at[node.y * Rules.W + node.x] = index
    end
    local entry_node = nodes[0]
    if not entry_node then return false end

    local state = {
        system_id = system_id,
        map = map,
        nodes = nodes,
        node_at = node_at,
        current_node = 0,
        previous_node = -1,
        link_cursor = 0,
        phase = "node",
        alert = Rules.NONE,
        in_alert = false,
        known = {},
    }
    state.known = Rules.reveal(map, state.known, entry_node.x, entry_node.y)
    M.active = state
    return true, state
end

function M.is_online(state)
    return state ~= nil or M.active ~= nil
end

function M.system_id(state)
    state = state or M.active
    return state and state.system_id or nil
end

function M.current_node(state)
    state = state or M.active
    if not state then return nil end
    return state.nodes[state.current_node]
end

function M.alert(state)
    state = state or M.active
    return state and state.alert or Rules.NONE
end

function M.previous_node(state)
    state = state or M.active
    return state and state.previous_node or -1
end

function M.retreat(state)
    state = state or M.active
    if not state or state.previous_node < 0 then return false end
    state.current_node, state.previous_node = state.previous_node,
                                              state.current_node
    local node = state.nodes[state.current_node]
    Rules.reveal(state.map, state.known, node.x, node.y)
    return true
end

-- Transitional text presentation of the exact ROM 14x10 map. Extracted
-- Matrix tile art will replace this view without changing the Lua state.
function M.map_text(state)
    state = state or M.active
    if not state then return "" end
    local symbols = { [0] = "S", "C", "P", "I", "D", "L" }
    local rows = {}
    for y = 0, Rules.H - 1 do
        local cells = {}
        for x = 0, Rules.W - 1 do
            local cell = y * Rules.W + x
            local value = " "
            if state.known[cell] then
                local node_id = state.node_at[cell]
                if node_id == state.current_node then
                    value = "@"
                elseif node_id ~= nil then
                    value = symbols[state.nodes[node_id].function_id] or "?"
                elseif state.map[cell] > 0x0B then
                    value = "+"
                end
            end
            cells[#cells + 1] = value
        end
        rows[#rows + 1] = table.concat(cells)
    end
    return table.concat(rows, "|")
end

local FILE_CLASSES = {
    [0] = "Outdated", "System", "Mngmnt", "Project", "Security",
    "Dead Projects", "Competition", "Marketing", "Simsense", "Financial",
    "Confidntl", "Legal", "Prisoner", "Case Files", "Unlisted",
}
local NODE_NAMES = {
    [0] = { "System Access", "Node" }, { "CPU" },
    { "Sub-Processor", "Unit" }, { "I/O Port" },
    { "Datastore" }, { "Slave", "Module" },
}
local COLOURS = { [1] = "Blue", "Green", "Orange", "Red" }
local ACTIONS = {
    [0] = { { id = "enter_system", label = "Enter System" } },
    [1] = {
        { id = "go_to_node", label = "Go To Node" },
        { id = "cancel_alert", label = "Cancel Alert" },
        { id = "crash_system", label = "Crash System" },
    },
    [2] = { { id = "leave_node", label = "Leave Node" } },
    [3] = {
        { id = "leave_node", label = "Leave Node" },
        { id = "lockout", label = "Lockout" },
    },
    [4] = {
        { id = "leave_node", label = "Leave Node" },
        { id = "transfer_data", label = "Transfer Data" },
        { id = "erase", label = "Erase" },
    },
    [5] = {
        { id = "leave_node", label = "Leave Node" },
        { id = "turn_off_node", label = "Turn Off Node" },
    },
}

function M.node_actions(state)
    local node = M.current_node(state)
    return node and ACTIONS[node.function_id] or {}
end

function M.node_text(state, selected_row)
    local node = M.current_node(state)
    if not node then return "" end
    local lines = {}
    local file_class = FILE_CLASSES[node.file_class_id]
    if file_class then lines[#lines + 1] = file_class end
    for _, line in ipairs(NODE_NAMES[node.function_id] or { "Unknown Node" }) do
        lines[#lines + 1] = line
    end
    local colour = COLOURS[node.colour] or "Unknown"
    lines[#lines + 1] = colour .. "-" .. tostring(node.security)
    lines[#lines + 1] = ""
    for index, action in ipairs(M.node_actions(state)) do
        lines[#lines + 1] =
            (index - 1 == selected_row and "> " or "  ") .. action.label
    end
    return table.concat(lines, "\n")
end

function M.select_node_action(state, action_id)
    local node = M.current_node(state)
    if not node then return false end
    if action_id == "enter_system" or action_id == "leave_node" then
        local target, previous = Movement.follow(
            state.current_node, node.links, state.link_cursor)
        if target == nil or target < 0 or state.nodes[target] == nil then
            return false
        end
        state.current_node, state.previous_node = target, previous
        state.phase = "node"
        state.link_cursor = 0
        local next_node = state.nodes[target]
        Rules.reveal(state.map, state.known, next_node.x, next_node.y)
        return true
    end
    if action_id == "go_to_node" then
        state.phase = "goto"
        state.goto_cursor = 0
        return true
    end
    return false
end

function M.step_link(state, right)
    local node = M.current_node(state)
    if not node then return false end
    state.link_cursor = right
                         and Movement.link_right(node.links, state.link_cursor)
                         or Movement.link_left(node.links, state.link_cursor)
    return true
end

function M.finish(state)
    if state == nil or M.active == state then M.active = nil end
end

return M
