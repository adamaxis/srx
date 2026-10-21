-- Matrix session state is Lua-owned. The source map and node records are
-- imported offline from the ROM into data/matrix_systems.lua.
local Systems = require("data.matrix_systems")
local E = require("enums")
local RunRegistry = require("run_registry")
local Rules = require("menu.rules.matrix")
local Movement = require("menu.rules.matrix_move")
local FILE_CLASSES = require("data.matrix_file_classes")
local HUD = require("data.matrix_hud_labels")
local WORLD_EFFECT_MODES = require("data.matrix_world_effect_modes")
local WORLD_ENTRY = require("data.matrix_entry_profiles")

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
        -- The ROM's node +1 byte is also the class consumed by node actions
        -- (including 0x27..0x2A Turn Off Node dispatch).
        class = source.file_class_id,
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

function M.begin(system_id, field_alarm, initial_node_id,
                 initial_node_ic_condition)
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
    if initial_node_id ~= nil and
       (type(initial_node_id) ~= "number" or
        initial_node_id % 1 ~= 0 or nodes[initial_node_id] == nil) then
        return false
    end
    if initial_node_ic_condition ~= nil and
       (type(initial_node_ic_condition) ~= "number" or
        initial_node_ic_condition % 1 ~= 0 or
        initial_node_ic_condition < 0 or
        initial_node_ic_condition > 0x7F) then
        return false
    end
    local selected_node_id = initial_node_id or 0
    local selected_node = nodes[selected_node_id]
    if initial_node_ic_condition ~= nil then
        selected_node.ic_condition = initial_node_ic_condition
    end

    local state = {
        system_id = system_id,
        map = map,
        nodes = nodes,
        node_at = node_at,
        current_node = selected_node_id,
        -- FFF0D4 selects one of the five loaded Matrix program slots. The
        -- world service moves this cursor horizontally before the A/B/C
        -- program action service runs.
        selected_program_slot = 0,
        previous_node = -1,
        link_cursor = 0,
        phase = "node",
        -- ROM 0x1817E clears FFF0B7 before entering the Matrix loop. This
        -- byte is the signed attack/balanced/defense mode consumed by the
        -- Matrix pool rules, so a newly opened system starts balanced.
        mode = E.MatrixModeResult.BALANCED,
        alert = Rules.initial_alert(field_alarm),
        attempts = 0,
        overload_armed = true,
        -- 0x174E4 clears FFDF84 on entry, independently of FFE16F's alert.
        in_alert = false,
        known = {},
        map_colours = {},
        -- FFFB30 is indexed by map cell. Keep its high nibble as a typed
        -- per-cell knowledge rank instead of deriving it from visibility.
        knowledge_ranks = {},
    }
    state.known = Rules.reveal(map, state.known,
                              selected_node.x, selected_node.y)
    remember_node_colour(state, selected_node_id)
    M.active = state
    return true, state
end

-- ROM 0xE034 clears FFF0E3 before system selection. ROM 0xE110 checks run
-- type 6, then sets it negative only when the selected system matches FFFBF0
-- and FFFC1C bit 7 is clear; all other entries remain outside the active run.
function M.activate_run_for_system(system_id, run)
    if type(system_id) ~= "number" or system_id % 1 ~= 0 or
       system_id < 0 or system_id > 24 or type(run) ~= "table" or
       type(run.target_system_id) ~= "number" or
       run.target_system_id % 1 ~= 0 or run.target_system_id < 0 or
       run.target_system_id > 24 then
        return false
    end

    local next_state
    if system_id == run.target_system_id and
       (((run.fc1c or 0) & 0x80) == 0) then
        next_state = -1
    else
        next_state = 0
    end
    if run.state == next_state then return false end
    run.state = next_state
    return true
end

-- Matrix objective RAM belongs to the accepted Lua run offer and is included
-- in run_registry.save_state(). The host supplies only field/world facts; this
-- projection keeps run state in one Lua-owned place while node rules operate
-- on a temporary typed view.
function M.world_state(game)
    local world = game.matrix_world_state()
    local run = RunRegistry.active_matrix_run_state()
    if run ~= nil then world.run = run end
    return world
end

function M.commit_world_state(game, world)
    if world.run ~= nil and
       RunRegistry.active_matrix_run_state() ~= nil then
        assert(RunRegistry.commit_active_matrix_run_state(world.run),
            "active Matrix objective state failed validation")
        world.run = nil
    end
    game.commit_matrix_world_state(world)
end

function M.is_online(state)
    return state ~= nil or M.active ~= nil
end

function M.current_state()
    return M.active
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

local NODE_NAMES = {
    [0] = { "System Access", "Node" }, { "CPU" },
    { "Sub-Processor", "Unit" }, { "I/O Port" },
    { "Datastore" }, { "Slave", "Module" },
}
local NODE_FUNCTION_LABELS = HUD.node_functions

function M.set_current_node_knowledge(state, rank)
    if type(state) ~= "table" or type(rank) ~= "number" or
       rank % 1 ~= 0 or rank < 0 or rank > 4 then
        return false
    end
    local node = M.current_node(state)
    if not node then return false end
    state.knowledge_ranks = state.knowledge_ranks or {}
    state.knowledge_ranks[node.y * Rules.W + node.x] = rank
    return true
end

-- ROM 0x19D50 ORs Analyze's result nibble into FFFB30 instead of replacing
-- the rank that node entry published at 0x54662.
function M.merge_current_node_knowledge(state, rank)
    if type(state) ~= "table" or type(rank) ~= "number" or
       rank % 1 ~= 0 or rank < 0 or rank > 4 then
        return false
    end
    local node = M.current_node(state)
    if not node then return false end
    state.knowledge_ranks = state.knowledge_ranks or {}
    local cell = node.y * Rules.W + node.x
    state.knowledge_ranks[cell] =
        (state.knowledge_ranks[cell] or 0) | rank
    return true
end

function M.current_node_knowledge(state)
    state = state or M.active
    local node = M.current_node(state)
    if not node then return 0 end
    return (state.knowledge_ranks or {})[node.y * Rules.W + node.x] or 0
end

function M.begin_world_entry(state)
    if type(state) ~= "table" or state.phase ~= "node" then
        return false
    end
    local node = M.current_node(state)
    if not node then return false end
    -- ROM 0x2679E selects the short stream only when the selected node's
    -- first FF1100 byte is zero; every nonzero node uses the compact stream.
    local profile_id = node.function_id == 0 and "primary" or "compact"
    local profile = WORLD_ENTRY[profile_id]
    if type(profile) ~= "table" then return false end
    state.world_entry = {
        profile_id = profile_id,
        phase = "setup",
        setup_vblanks = 0,
        sequence_vblanks = 0,
        published_frame = 0,
    }
    state.analyze_runner_idle = false
    return true
end

function M.advance_world_entry(state)
    if type(state) ~= "table" or type(state.world_entry) ~= "table" then
        return false
    end
    local entry = state.world_entry
    local profile = WORLD_ENTRY[entry.profile_id]
    if type(profile) ~= "table" or entry.phase == "settled" then
        return false
    end
    if entry.phase == "setup" then
        entry.setup_vblanks = entry.setup_vblanks + 1
        if entry.setup_vblanks >= WORLD_ENTRY.setup_vblanks then
            entry.setup_vblanks = WORLD_ENTRY.setup_vblanks
            entry.phase = "motion"
        end
        return true
    end

    entry.sequence_vblanks = math.min(
        profile.duration_vblanks, entry.sequence_vblanks + 1)
    while entry.published_frame < #profile.published_at_vblank and
          profile.published_at_vblank[entry.published_frame + 1] <=
              entry.sequence_vblanks do
        entry.published_frame = entry.published_frame + 1
    end
    if entry.sequence_vblanks >= profile.duration_vblanks then
        entry.phase = "settled"
        entry.published_frame = #profile.published_at_vblank
    end
    return true
end

local function model_canvas_for_entry(state, entry)
    local node = M.current_node(state)
    local profile = entry and WORLD_ENTRY[entry.profile_id]
    if not node or not profile or node.function_id ~= profile.model_id or
       node.colour ~= profile.palette_theme or
       type(profile.projection_samples) ~= "table" then
        return nil
    end

    local samples = profile.projection_samples
    local elapsed = entry.sequence_vblanks or 0
    if elapsed < samples[1].vblank then return nil end
    local before, after = samples[#samples], samples[#samples]
    for index = 2, #samples do
        if elapsed <= samples[index].vblank then
            before, after = samples[index - 1], samples[index]
            break
        end
    end
    local span = after.vblank - before.vblank
    local amount = span > 0 and
                   (elapsed - before.vblank) / span or 0
    local function interpolate(key)
        local value = before[key] + (after[key] - before[key]) * amount
        return math.floor(value + 0.5)
    end
    local x, y = interpolate("x"), interpolate("y")
    local width, height = interpolate("width"), interpolate("height")
    local source = profile.source_green_bounds
    if width <= 0 or height <= 0 or not source then return nil end

    -- Convert the independently measured green-pixel extent into the named
    -- source image canvas. The source bounds are counted from the extracted
    -- 96x80 model; the host still draws that image directly with nearest
    -- neighbor sampling and does not reconstruct Genesis tile output.
    local scale_x = width / source.width
    local scale_y = height / source.height
    local round = function(value) return math.floor(value + 0.5) end
    local canvas = profile.source_canvas or { width = 96, height = 80 }
    return {
        x = x - round(source.x * scale_x),
        y = y - round(source.y * scale_y),
        width = round(canvas.width * scale_x),
        height = round(canvas.height * scale_y),
    }
end

local function world_effect_animation(state, node, rank)
    -- 0x19B40 selects the current node's IC type through E31A7; 0x1A908
    -- installs that mode record, or the 1AA10 fallback for an unseen node.
    local mode = Rules.ic_type(node, state.alert or Rules.NONE,
                               state.in_alert == true, rank)
    local animation = state.world_effect_animation
    if type(animation) ~= "table" or animation.mode ~= mode then
        local record = mode == Rules.UNSEEN and WORLD_EFFECT_MODES.default or
                       WORLD_EFFECT_MODES[mode]
        animation = {
            mode = mode,
            phase = 0,
            elapsed = 0,
            cadence = record and record.cadence or 0,
            phase_count = record and record.phase_count or 0,
        }
        state.world_effect_animation = animation
    end
    return animation
end

-- Called once for each active node-action frame. The source advances the
-- selected 0x1AA40 record at its own byte cadence; Lua owns this animation
-- state, while the host only presents the named extracted frame.
function M.advance_world_effect_frame(state)
    if type(state) ~= "table" or state.phase ~= "node" then return false end
    local node = M.current_node(state)
    if not node then return false end
    local animation = world_effect_animation(
        state, node, M.current_node_knowledge(state))
    if animation.cadence <= 0 or animation.phase_count <= 0 then
        return false
    end
    animation.elapsed = animation.elapsed + 1
    if animation.elapsed < animation.cadence then return false end
    animation.elapsed = 0
    animation.phase = (animation.phase + 1) % animation.phase_count
    return true
end

function M.selected_program_slot(state)
    state = state or M.active
    return state and state.selected_program_slot or 0
end

-- ROM 0x199D6 reads the held C bit first, then A; C assigns attack mode,
-- A assigns balanced mode, and the remaining confirmation path assigns
-- defense mode (B). Preserve the source priority if multiple buttons arrive
-- in the same controller word.
function M.program_mode_from_buttons(events)
    events = type(events) == "number" and events or 0
    if events & E.Pad.C ~= 0 then return E.MatrixModeResult.ATTACK end
    if events & E.Pad.A ~= 0 then return E.MatrixModeResult.BALANCED end
    return E.MatrixModeResult.DEFENSE
end

function M.step_program_slot(state, right, active, slots)
    if type(state) ~= "table" or state.phase ~= "node" then return false end
    local current = state.selected_program_slot or 0
    local phases = {}
    if type(slots) == "table" then
        for slot = 0, 4 do
            phases[slot] = type(slots[slot]) == "table" and
                           (slots[slot].phase or 0) or 0
        end
    end
    local next_slot = (current + (right and 1 or -1)) % 5
    if type(active) == "table" and type(slots) == "table" then
        next_slot = Rules.cycle_slot(current, right and 1 or -1,
                                     active, phases)
        for slot = 0, 4 do
            local visual = slots[slot]
            if type(visual) == "table" and
               phases[slot] ~= visual.phase then
                visual.phase = phases[slot]
            end
        end
    end
    state.selected_program_slot = next_slot
    return true, next_slot ~= current
end

-- ROM 0x1BE82 starts the Analyze success transition. The profile timer is
-- FFE00C=0x3C; 0x1BEBA decrements it before choosing the ring phase. Behavior
-- A begins at paired phase zero and 0x1C3FA advances it to raised phase one
-- after five source calls.
function M.begin_analyze_transition(state, slot, result_state)
    if type(state) ~= "table" or type(slot) ~= "number" or
       slot % 1 ~= 0 or slot < 0 or slot > 4 or
       (result_state ~= 6 and result_state ~= 10) then
        return false
    end
    state.analyze_transition = {
        remaining = 0x3C,
        elapsed = 0,
        ring_phase = 0,
        runner_phase = 0,
        slot = slot,
        result_state = result_state,
    }
    return true
end

function M.advance_analyze_transition(state)
    local transition = type(state) == "table" and
                       state.analyze_transition or nil
    if type(transition) ~= "table" then return false end
    transition.remaining = transition.remaining - 1
    transition.elapsed = transition.elapsed + 1
    if transition.elapsed >= 5 then transition.runner_phase = 1 end
    if transition.remaining < 0 then
        state.analyze_transition = nil
        state.analyze_runner_idle = true
        return true, transition
    end
    -- The source NEG/LSR/AND sequence is a logical shift of the 16-bit
    -- countdown after it has been decremented.
    transition.ring_phase = ((-transition.remaining) >> 1) & 3
    return true
end

function M.program_status_label(status)
    if type(status) ~= "number" or status % 1 ~= 0 then return "" end
    return HUD.program_states[status] or ""
end

-- ROM 0x19788 advances only the first incomplete visual slot. FFF0ED is the
-- six-step footer state; FFF0F2 supplies that slot's delay between steps, and
-- completion publishes FFF0E8 as Ready. Keep the source countdown with the
-- Lua-owned Matrix session so it continues across deck and traversal screens.
function M.advance_program_loading(state, slots)
    if type(state) ~= "table" or type(slots) ~= "table" then
        return false
    end

    local loading = state.program_loading
    if type(loading) ~= "table" then
        local pending = false
        for slot = 0, 4 do
            local visual = slots[slot]
            if type(visual) == "table" and (visual.status or 0) ~= 6 then
                pending = true
                break
            end
        end
        if not pending then return false end
        loading = { progress_delay = -1 }
        state.program_loading = loading
    end
    loading.progress_delay = (loading.progress_delay or -1) - 1
    if loading.progress_delay >= 0 then return false end

    local selected
    for slot = 0, 4 do
        local visual = slots[slot]
        if type(visual) == "table" and (visual.status or 0) ~= 6 then
            selected = slot
            break
        end
    end
    if selected == nil then
        state.program_loading = nil
        return false
    end

    local visual = slots[selected]
    visual.status = (visual.status or 0) + 1
    loading.progress_delay = visual.timer or 0
    if visual.status >= 6 then
        visual.status = 6
        visual.phase = 1
        local pending = false
        for slot = 0, 4 do
            local other = slots[slot]
            if type(other) == "table" and (other.status or 0) ~= 6 then
                pending = true
                break
            end
        end
        if not pending then state.program_loading = nil end
    end
    return true
end

function M.file_class_name(class_id)
    local label = FILE_CLASSES[class_id]
    if label == nil then return nil end
    -- These strings are padded to fixed ROM cells for direct rendering. Run
    -- Info places the same source label in a host message, so discard only
    -- the cell padding while preserving the authored spelling.
    return label:match("^%s*(.-)%s*$")
end
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
    local status = M.node_status(state)
    if not status then return "" end
    return status.file_class_label .. "|" .. status.alert_band_label ..
           tostring(status.action_difficulty)
end

function M.node_status(state)
    local node = M.current_node(state)
    if not node then return nil end
    local rank = M.current_node_knowledge(state)
    -- ROM 0x546F2 keeps function values 0 and 1, then subtracts one from
    -- values 2..5 before indexing the five 0x38-byte layout records at
    -- 0x166330. The same +5 color byte selects the 0x166448 palette table.
    local background_profile = node.function_id < 2 and node.function_id or
                               node.function_id - 1
    return {
        knowledge_rank = rank,
        file_class_label = rank >= 3 and
                           (FILE_CLASSES[node.file_class_id] or "") or "",
        alert_band_label = HUD.security_bands[rank > 0 and node.colour or 0]
                           or HUD.security_bands[0],
        action_difficulty = rank >= 2 and node.security or -1,
        background_profile = background_profile,
        palette_theme = node.colour,
        -- ROM 0x17858 indexes the E2EDA records with the current node's first
        -- FF1100 byte, exposed by the Lua topology as function_id.
        world_model_id = node.function_id,
    }
end

function M.traversal_view(state, active_program_label,
                          active_program_status_label)
    local node = M.current_node(state)
    local status = M.node_status(state)
    if not node or not status then return nil end
    local effect_animation = world_effect_animation(
        state, node, status.knowledge_rank)
    local entry = state.world_entry
    local entry_profile = entry and WORLD_ENTRY[entry.profile_id] or nil
    local entry_phase = entry and entry.phase == "setup" and 0 or
                        entry and entry.phase == "motion" and 1 or 2
    local model_canvas = model_canvas_for_entry(state, entry)

    local ice_label, ice_rating_label, ice_rating_column = "", "", 0
    local second_ice_label, second_ice_rating_label = "", ""
    local second_ice_rating_column = 0
    if node.ic_code < 0 then
        ice_label = HUD.ic_visibility[0]
    elseif node.ic_condition <= 0 then
        ice_label = HUD.ic_visibility[2]
    elseif status.knowledge_rank <= 0 then
        -- 0x17A5E emits "Ice Unknown" until the map-status high nibble is
        -- positive, even if another combat path can infer the IC type.
        ice_label = HUD.ic_visibility[1]
    else
        local combo = Rules.IC_TABLE[node.ic_code]
        local alert = state.alert or Rules.NONE
        local alert_bonus = alert < 0 and 0 or alert == 0 and 1 or 2
        local first_type = combo and combo[1]
        local second_type = combo and combo[2]
        if type(first_type) == "number" and first_type >= 0 and
           first_type <= 8 then
            ice_label = HUD.ic_types[first_type]
            ice_rating_column = HUD.ic_rating_columns[first_type]
            ice_rating_label = status.knowledge_rank >= 2 and
                tostring(node.ic_rating + alert_bonus) or "?"
        else
            ice_label = HUD.ic_visibility[0]
        end
        if type(second_type) == "number" and second_type >= 0 and
           second_type <= 8 then
            second_ice_label = HUD.ic_types[second_type]
            second_ice_rating_column = HUD.ic_rating_columns[second_type]
            second_ice_rating_label = status.knowledge_rank >= 2 and
                tostring(node.ic_rating2 + alert_bonus) or "?"
        end
    end

    return {
        world_model_id = status.world_model_id,
        -- ROM 0x19DF2 leaves D0 at zero when 0x17C3C reports no known node
        -- rank. Otherwise it selects the active node's security-color row.
        -- Row zero is the neutral gray model palette at 0xE2D52.
        palette_theme = status.knowledge_rank > 0 and
                        status.palette_theme or 0,
        world_effect_mode = effect_animation.mode,
        world_effect_phase = effect_animation.phase,
        action_mode = state.mode or E.MatrixModeResult.BALANCED,
        world_entry_phase = entry_phase,
        world_entry_setup_vblanks = entry and entry.setup_vblanks or 0,
        world_entry_sequence_vblanks = entry and
                                       entry.sequence_vblanks or 0,
        world_entry_sequence_duration = entry_profile and
                                        entry_profile.duration_vblanks or 0,
        world_entry_published_frame = entry and entry.published_frame or 0,
        analyze_effect_phase = state.analyze_transition and
                               state.analyze_transition.ring_phase or -1,
        analyze_runner_phase = state.analyze_transition and
                               state.analyze_transition.runner_phase or
                               state.analyze_runner_idle and 2 or -1,
        model_canvas_x = model_canvas and model_canvas.x or 0,
        model_canvas_y = model_canvas and model_canvas.y or 0,
        model_canvas_width = model_canvas and model_canvas.width or 0,
        model_canvas_height = model_canvas and model_canvas.height or 0,
        knowledge_rank = status.knowledge_rank,
        function_label = NODE_FUNCTION_LABELS[node.function_id],
        file_class_label = status.file_class_label,
        alert_band_label = status.alert_band_label,
        action_difficulty = status.action_difficulty,
        action_difficulty_label = status.action_difficulty >= 0 and
                                  tostring(status.action_difficulty) or "",
        ice_label = ice_label,
        ice_rating_label = ice_rating_label,
        ice_rating_column = ice_rating_column,
        second_ice_label = second_ice_label,
        second_ice_rating_label = second_ice_rating_label,
        second_ice_rating_column = second_ice_rating_column,
        alert_label = HUD.alert_states[(state.alert or -1) + 1] or "",
        active_program_label = active_program_label or "",
        active_program_status_label = active_program_status_label or "",
    }
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
    local status = M.node_status(state)
    lines[#lines + 1] = status.alert_band_label ..
                         tostring(status.action_difficulty)
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
