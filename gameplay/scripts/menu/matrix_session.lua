-- Matrix session state is Lua-owned. The source map and node records are
-- imported offline from the ROM into data/matrix_systems.lua.
local Systems = require("data.matrix_systems")
local E = require("enums")
local Rules = require("menu.rules.matrix")
local Movement = require("menu.rules.matrix_move")

local M = { active = nil }

local function source_links(source)
    local links = {}
    for index, target in ipairs(source or {}) do
        links[index - 1] = target
    end
    return links
end

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
        links = source_links(source.links),
        ic_condition = source.ic_condition,
        ic_reaction = source.ic_reaction,
        ic_rating = source.ic_rating,
        ic_rating2 = source.ic_rating2,
        ic_code = source.ic_combo_id,
    }
end

local function remember_node_colour(state, node_id)
    local node = state.nodes[node_id]
    if node then
        state.map_colours[node.y * Rules.W + node.x] = node.colour
    end
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
        mode = E.MatrixModeResult.ATTACK,
        alert = Rules.NONE,
        in_alert = false,
        known = {},
        map_colours = {},
    }
    state.known = Rules.reveal(map, state.known, entry_node.x, entry_node.y)
    remember_node_colour(state, 0)
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
    if not state or state.phase ~= "node" or state.previous_node < 0 then
        return false
    end
    state.current_node, state.previous_node = state.previous_node,
                                              state.current_node
    local node = state.nodes[state.current_node]
    Rules.reveal(state.map, state.known, node.x, node.y)
    remember_node_colour(state, state.current_node)
    state.phase = "node"
    state.pending_action = nil
    return true
end

-- Build the source 14x10 map's render records. Unknown cells use the ROM's
-- blank tile; known cells select the exact 2x2 source table record. The low
-- map colour byte only changes when a node is first entered, so revealed but
-- unvisited nodes and all connectors retain the source grey/blue palette.
function M.map_visual_cells(state)
    state = state or M.active
    if not state then return {} end
    local cells = {}
    for cell = 0, Rules.W * Rules.H - 1 do
        local known = state.known[cell] == true
        local kind = known and state.map[cell] or 0
        local colour = known and state.map_colours[cell] or 0
        local palette_word = Rules.cell_style(colour)
        local tile_table = colour >= 3 and Rules.TILE_B or Rules.TILE_A
        local source_tiles = tile_table[kind] or tile_table[0]
        local words = {}
        for tile = 1, 4 do
            words[tile] = palette_word | source_tiles[tile]
        end
        cells[cell + 1] = { tiles = words }
    end
    return cells
end

function M.map_labels()
    local labels = {}
    for index, label in ipairs(Rules.MAP_LABELS) do
        labels[index] = label
    end
    return labels
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

function M.node_name(state, node_id)
    state = state or M.active
    local node = state and state.nodes[node_id]
    if not node then return "Unknown Node" end
    return table.concat(NODE_NAMES[node.function_id] or { "Unknown Node" }, " ")
end

function M.node_summary(state)
    local node = M.current_node(state)
    if not node then return "" end
    local file_class = FILE_CLASSES[node.file_class_id] or "Unknown"
    local colour = COLOURS[node.colour] or "Unknown"
    return file_class .. "|" .. colour .. "-" .. tostring(node.security)
end

function M.node_cursor_text(state)
    if not state then return "" end
    local target
    if state.phase == "link_select" then
        local node = M.current_node(state)
        target = node and node.links[state.link_cursor]
        return "Link " .. tostring(state.link_cursor + 1) .. "|" ..
               "Destination: " .. M.node_name(state, target)
    end
    if state.phase == "goto" then
        target = state.goto_cursor
        return "Go To Node|Destination: " .. M.node_name(state, target)
    end
    return M.node_summary(state)
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
    if not node or state.phase ~= "node" then return false end
    local available = false
    for _, action in ipairs(M.node_actions(state)) do
        if action.id == action_id then available = true; break end
    end
    if not available then return false end
    if action_id == "enter_system" or action_id == "leave_node" then
        state.phase = "link_select"
        state.pending_action = action_id
        state.link_cursor = 0
        return true
    end
    if action_id == "go_to_node" then
        state.phase = "goto"
        state.goto_cursor = 0
        state.pending_action = action_id
        return true
    end
    return false
end

function M.step_link(state, right)
    if not state or state.phase ~= "link_select" then return false end
    local node = M.current_node(state)
    if not node then return false end
    state.link_cursor = right
                         and Movement.link_right(node.links, state.link_cursor)
                         or Movement.link_left(node.links, state.link_cursor)
    return true
end

function M.step_goto(state, right)
    if not state or state.phase ~= "goto" then return false end
    state.goto_cursor = right
        and Movement.goto_right(state.nodes, state.current_node,
                                state.goto_cursor)
        or Movement.goto_left(state.nodes, state.current_node,
                              state.goto_cursor)
    return true
end

local function enter_node(state, target, previous)
    local next_node = state.nodes[target]
    if not next_node then return false end
    state.current_node, state.previous_node = target, previous
    state.phase = "node"
    state.pending_action = nil
    state.link_cursor, state.goto_cursor = 0, nil
    Rules.reveal(state.map, state.known, next_node.x, next_node.y)
    remember_node_colour(state, target)
    return true
end

-- ROM 0x53704 confirms a highlighted link; 0x53864 confirms a CPU Go To
-- Node target. Keep cursor movement and confirmation separate so input can
-- be routed through the Lua screen without an implicit move on menu choice.
function M.commit_node_selection(state)
    if not state then return false end
    if state.phase == "link_select" then
        local node = M.current_node(state)
        if not node then return false end
        local target, previous = Movement.follow(
            state.current_node, node.links, state.link_cursor)
        if target == nil or target < 0 then return false end
        return enter_node(state, target, previous)
    end
    if state.phase == "goto" then
        local target, previous = Movement.goto_commit(
            state.nodes, state.current_node, state.goto_cursor)
        return enter_node(state, target, previous)
    end
    return false
end

function M.cancel_node_selection(state)
    if not state or (state.phase ~= "link_select" and
                     state.phase ~= "goto") then
        return false
    end
    state.phase = "node"
    state.pending_action = nil
    state.link_cursor, state.goto_cursor = 0, nil
    return true
end

function M.finish(state)
    if state == nil or M.active == state then M.active = nil end
end

return M
