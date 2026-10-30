-- rules/matrix.lua: Matrix state rules decoded 2026-10-15 [C unless marked].
-- Node record (FF1100 + node*0x14, docs/05 s8): +3 x, +4 y, +0x0F IC condition, +0x11/+0x12 IC rating(s), +0x13 IC code.
-- FFF0A0 = system alert: 0xFF none, 0 passive, 1 active (texts 0xE3074 + (a+1)*0xE: No / Passive / Active Alert).
-- FFF0B0 = jacked in: FF only between 0x17410 and 0x17422 (around the Matrix session 0x1745E); deck menus opened
--   from the Pocket Secretary see 0 (Pull the plug and Show Map/Retreat are inert) [C].
local M = {}
M.NONE, M.PASSIVE, M.ACTIVE = -1, 0, 1
-- The Matrix exit used by overload (0x1989A), Crash System (0x54B34), and
-- Pull Plug (0x18450) calls 0x18776 (five palette steps, two VBlanks each),
-- then 0x26A00. The source presents a line cross that collapses over 14 + 6
-- VBlanks, followed by the center tile sequence held for 11 + 6 + 6 + 11.
M.EXIT_FADE_FRAMES = 10
M.EXIT_DISPLAY_TRANSITION_FRAMES = 54
M.EXIT_TRANSITION_FRAMES =
    M.EXIT_FADE_FRAMES + M.EXIT_DISPLAY_TRANSITION_FRAMES
M.EXIT_FADE_STEPS = 5

local function logout_visual(elapsed_frames)
    if elapsed_frames < M.EXIT_FADE_FRAMES then
        return { phase = "fade" }
    end

    -- 0x26A00 draws its initial cross before its first wait. Each following
    -- VBlank replaces one 8x8 edge tile with the source's blank pattern 4B5.
    -- The first edge address lies just outside the visible viewport, hence
    -- the first wait leaves the full cross unchanged.
    local display_frame = elapsed_frames - M.EXIT_FADE_FRAMES
    if display_frame <= 19 then
        local horizontal_tiles = math.min(19, math.max(0, display_frame - 1))
        local vertical_tiles = math.min(13, horizontal_tiles)
        return {
            phase = "cross",
            horizontal_left = 8 + horizontal_tiles * 8,
            horizontal_right = 320 - horizontal_tiles * 8,
            vertical_top = 8 + vertical_tiles * 8,
            vertical_bottom = 224 - vertical_tiles * 8,
        }
    end
    if display_frame <= 30 then return { phase = "dot" } end
    if display_frame <= 36 then return { phase = "sparkle" } end
    if display_frame <= 42 then return { phase = "dot" } end
    return { phase = "black" }
end

function M.new_exit_transition(exit_when_complete)
    local transition = {
        frames_remaining = M.EXIT_TRANSITION_FRAMES,
        elapsed_frames = 0,
        fade_step = 0,
        exit_when_complete = exit_when_complete == true,
    }
    transition.visual = logout_visual(transition.elapsed_frames)
    return transition
end
-- Advance one source VBlank. The first ten frames perform five two-frame
-- palette steps; the logout sound starts on the first display-transition
-- frame, followed by the ROM's 54-frame screen handoff.
function M.advance_exit_transition(transition)
    transition.frames_remaining = transition.frames_remaining - 1
    transition.elapsed_frames = transition.elapsed_frames + 1
    transition.fade_step = math.min(
        M.EXIT_FADE_STEPS,
        transition.elapsed_frames // 2)
    transition.visual = logout_visual(transition.elapsed_frames)
    return transition.elapsed_frames == M.EXIT_FADE_FRAMES,
           transition.frames_remaining <= 0
end
-- 0xC24 waits d5 + 1 VBlanks (dbra) [C]. Jack-in 0xE47A d5 = 0x14; random
-- alert 0x54AD4 d5 = 0x19; overload 0x198F4 d5 = 0x1E. 0xD94 then waits one
-- more VBlank before it samples the pad edge.
M.JACK_IN_WAIT_FRAMES = 0x14 + 1
M.ALERT_MESSAGE_WAIT_FRAMES = 0x19 + 1
M.OVERLOAD_MESSAGE_WAIT_FRAMES = 0x1E + 1
M.NO_IC, M.UNSEEN = 0x45, -1
-- 0x1833C..0x18418: the visible deck grid is four columns by four rows.
-- Direction priority is up, down, left, right; movement wraps silently.
function M.step_deck_cursor(column, row, up, down, left, right)
    if up then return column, (row + 3) & 3, true end
    if down then return column, (row + 1) & 3, true end
    if left then return (column + 3) & 3, row, true end
    if right then return (column + 1) & 3, row, true end
    return column, row, false
end
-- 0xE31A7: per IC code {normal, active-alert} type bytes (0xFF = unused). Codes 7..16 are two-layer IC:
-- the second type applies once the alert is ACTIVE (0x19B82: FFF0A0 > 0 and 7 <= code < 0x11).
M.IC_TABLE = { [0] =
    {0,255},{1,255},{2,255},{3,255},{8,255},{6,255},{7,255},
    {0,6},{0,7},{1,6},{1,7},{0,2},{0,3},{0,8},{1,2},{1,3},{1,8},
    {0,4},{0,5},{1,4},{1,5},{2,4},{2,5},{3,4},{3,5},{8,4},{8,5},
} -- Codes 0..26, both bytes copied from ROM 0xE31A7.
local function layered(code, alert) return alert > 0 and code >= 7 and code < 0x11 end
-- [C] 0x19B40 selects IC type. The native model supplies whether this node
-- has been discovered, not the source's packed per-node analysis value.
-- world_effect_gate_negative comes from FFDF84 < 0; alert remains independent.
function M.ic_type(node, alert, world_effect_gate_negative, node_discovered)
    local code = node.ic_code
    if node.ic_condition <= 0 or node.ic_condition >= 0x80 or
       type(code) ~= "number" or code < 0 or code > 26 then
        return M.NO_IC
    end -- signed BLE/BMI guards at 0x19B58/0x19B64; table ends after code 26.
    if not world_effect_gate_negative and not node_discovered then
        return M.UNSEEN
    end
    local types = M.IC_TABLE[code]
    if not types then return M.NO_IC end
    return types[layered(code, alert) and 2 or 1]
end
-- 0x19BF6: IC rating = +0x11 (or +0x12 for the second layer) + alert bonus (none 0, passive +1, active +2)
function M.ic_rating(node, alert)
    local r = layered(node.ic_code, alert) and node.ic_rating2 or node.ic_rating
    return r + (alert < 0 and 0 or alert == 0 and 1 or 2)
end
-- 0x1989A: raise/lower alert by d. Result 0/1 kept; < 0 -> NONE; > 1 -> ACTIVE and, if FFF0A6 is set,
-- SYSTEM OVERLOAD (0x18776 fade + 0x26A00 logout + text 0x166A75, 30-frame wait, 0xD94). If FFE189 changes,
-- sound 0x54 = none, 0x53 = passive, 0x52 = active.
function M.raise_alert(alert, d, overload_armed)
    local a = (alert + d + 128) % 256 - 128            -- add.b
    if a == 0 or a == 1 then return a, false end
    if a > 1 then return M.ACTIVE, overload_armed end
    return M.NONE, false
end
-- 0x173F2: initial alert = FFE16F != 0 -> ACTIVE else NONE. After the session: if FFDF40 == 2 and alert == ACTIVE -> FFE191 = 1.
function M.initial_alert(ffe16f)
    local alarm = ffe16f == true or
                  (type(ffe16f) == "number" and ffe16f ~= 0)
    return alarm and M.ACTIVE or M.NONE
end

-- [C] ROM 0x532BC draws a 14x10 topology from the selected system record.
-- The offline import retains source tile variants; session state and
-- discovery use row/column coordinates. ROM 0x533EE reveals the current cell,
-- then 0x53434 follows connectors in four directions. Connector-set source
-- tables are 0x15FCAF/0x15FC9A/0x15FCA8/0x15FCA1. Its horizontal row wrap is
-- preserved as an explicit coordinate rule in adjacent_cell.
M.W, M.H = 14, 10
-- 0x532BC renders these six fixed row labels from 0x15FD36 at plane-A
-- address 0xC204; the host presents the same text from the extracted font.
M.MAP_LABELS = { "CPU", "SPU", "IOP", "SAN", "SM", "DS" }
local function set(t) local s = {}; for _, v in ipairs(t) do s[v] = true end; return s end
M.CONNECT_LEFT, M.CONNECT_RIGHT = set{12,14,15,18,19,21,22}, set{12,16,17,18,19,20,22}
M.CONNECT_UP, M.CONNECT_DOWN = set{13,14,16,18,20,21,22}, set{13,15,17,19,20,21,22}
local DIRS = {   -- order from 0x53434: up, right, down, left.
    { dx = 0, dy = -1, block = "up", own = "CONNECT_UP", neighbor = "CONNECT_DOWN", child_block = "down" },
    { dx = 1, dy = 0, block = "right", own = "CONNECT_RIGHT", neighbor = "CONNECT_LEFT", child_block = "left" },
    { dx = 0, dy = 1, block = "down", own = "CONNECT_DOWN", neighbor = "CONNECT_UP", child_block = "up" },
    { dx = -1, dy = 0, block = "left", own = "CONNECT_LEFT", neighbor = "CONNECT_RIGHT", child_block = "right" },
}
local function adjacent_cell(x, y, dx, dy)
    x, y = x + dx, y + dy
    -- Source 0x53434 checks the flattened board bounds, but not horizontal
    -- row edges. Preserve that observable connector wrap in coordinates.
    if x < 0 then
        x, y = M.W - 1, y - 1
    elseif x >= M.W then
        x, y = 0, y + 1
    end
    if y < 0 or y >= M.H then return nil end
    return x, y
end

local function reveal_cell(cell_knowledge, x, y)
    local row = cell_knowledge[y]
    if type(row) ~= "table" then
        row = {}
        cell_knowledge[y] = row
    end
    local facts = row[x]
    if type(facts) ~= "table" then
        facts = {
            analysis_level = 0,
            security_color = "unvisited",
        }
        row[x] = facts
    end
    facts.revealed = true
end

local function flood(map, cell_knowledge, expanded, x, y, block)
    -- Connector loops can revisit a cell, so track this reveal pass to keep
    -- traversal finite.
    local expanded_row = expanded[y]
    if not expanded_row then
        expanded_row = {}
        expanded[y] = expanded_row
    end
    if expanded_row[x] then return end
    expanded_row[x] = true
    local current_kind = map[y][x] or 0
    for _, d in ipairs(DIRS) do
        if not block[d.block] then
            local next_x, next_y = adjacent_cell(x, y, d.dx, d.dy)
            if next_x ~= nil then
                local next_kind = map[next_y][next_x] or 0
                if next_kind > 0x0B then
                    if M[d.neighbor][next_kind] then
                        reveal_cell(cell_knowledge, next_x, next_y)
                        flood(map, cell_knowledge, expanded, next_x, next_y,
                              { [d.child_block] = true })
                    end
                elseif M[d.own][current_kind] then
                    reveal_cell(cell_knowledge, next_x, next_y)
                end
            end
        end
    end
end
function M.reveal(map, cell_knowledge, x, y)
    if type(map) ~= "table" or type(map[y]) ~= "table" or
       map[y][x] == nil then
        return cell_knowledge
    end
    reveal_cell(cell_knowledge, x, y)
    flood(map, cell_knowledge, {}, x, y, {})
    return cell_knowledge
end
-- [C] The original 0x53338 draw uses revealed state to choose a blank or
-- visible cell, then remembered security color to choose a visual family.
-- Source behavior comes from 0x19DAC (first-visit color), 0x19D50 (Analyze
-- result), 0x17A88 (no-IC detail), 0x54676 (post-action detail), 0x19B40
-- (known/unseen gate), and 0x532A8 (CPU reveals the whole map and arms FFF0A6).
-- The runtime stores named cell facts rather than the original packed status
-- byte; the palette and tile records below are presenter data.
-- Tile tables (4 name words = 2x2 cell, 0x15C3CC = blank): TILE_A[0..22] (0 empty, 1..11 node icons, 12..22 connectors),
--   TILE_B[0..11] (node icons for Orange/Red). Quirk: TILE_B has no connector rows; colour is only ever OR'd on node
--   cells, so 12..22 never index it [C].
M.TILE_A = { [0] = {0x00BF,0x00BF,0x00BF,0x00BF}, {0x00C0,0x08C0,0x10C0,0x18C0}, {0x00CD,0x08CD,0x10CD,0x18CD}, {0x00D0,0x08D0,0x10D0,0x18D0}, {0x00D5,0x08D5,0x10D5,0x18D5}, {0x08C2,0x00C2,0x08C9,0x00C9}, {0x00D1,0x08D1,0x10D1,0x18D1}, {0x00C1,0x00C2,0x00C8,0x00C9}, {0x00C1,0x08C1,0x00C8,0x08C8}, {0x08C2,0x08C1,0x08C9,0x08C8}, {0x00D1,0x08D1,0x10D6,0x18D6}, {0x00D6,0x08D6,0x10D1,0x18D1}, {0x00C6,0x08C6,0x10C6,0x18C6}, {0x00CF,0x08CF,0x10CF,0x18CF}, {0x00C7,0x00D4,0x00CC,0x00BF}, {0x10CC,0x00BF,0x10C7,0x10D4}, {0x08D4,0x08C7,0x00BF,0x08CC}, {0x00BF,0x18CC,0x18D4,0x18C7}, {0x00C7,0x08C7,0x00CC,0x08CC}, {0x10CC,0x18CC,0x10C7,0x18C7}, {0x08D4,0x08C7,0x18D4,0x18C7}, {0x00C7,0x00D4,0x10C7,0x10D4}, {0x00C7,0x08C7,0x10C7,0x18C7} }
M.TILE_B = { [0] = {0x00BF,0x00BF,0x00BF,0x00BF}, {0x00C3,0x08C3,0x10C3,0x18C3}, {0x00CE,0x08CE,0x10CE,0x18CE}, {0x00D2,0x08D2,0x10D2,0x18D2}, {0x00D7,0x08D7,0x10D7,0x18D7}, {0x08C5,0x00C5,0x08CB,0x00CB}, {0x00D3,0x08D3,0x10D3,0x18D3}, {0x00C4,0x00C5,0x00CA,0x00CB}, {0x00C4,0x08C4,0x00CA,0x08CA}, {0x08C5,0x08C4,0x08CB,0x08CA}, {0x00D3,0x08D3,0x10D8,0x18D8}, {0x00D8,0x08D8,0x10D3,0x18D3} }
local CELL_STYLES = {
    unvisited = { palette = 0x2000, tile_family = "standard" },
    blue = { palette = 0x4000, tile_family = "standard" },
    green = { palette = 0x6000, tile_family = "standard" },
    orange = { palette = 0x4000, tile_family = "warm" },
    red = { palette = 0x6000, tile_family = "warm" },
}
function M.cell_style(security_color)
    local style = CELL_STYLES[security_color] or CELL_STYLES.unvisited
    return style.palette, style.tile_family
end

-- 0x19432 (Matrix loop, Left/Right): before moving, if the program in the current slot is not 3/5/6/7 and its run state
-- FFF0E8[slot] >= 6, set it to 1 (0x17C84 on the FIRST slot holding that code). Then FFF0D4 += d, wrap (<0 -> 4, >= 5 -> 0),
-- redraw cursor (0x19504 / 0x194D8) and slot HUD (0x17BC0: name 0xE2F22 + code*12).
M.PERSISTENT = { [3] = true, [5] = true, [6] = true, [7] = true }
function M.cycle_slot(cur, d, active, run_state)
    local code = active[cur]
    if not M.PERSISTENT[code] and run_state[cur] >= 6 then
        for s = 0, 4 do if active[s] == code then run_state[s] = 1; break end end
    end
    local n = cur + d; if n < 0 then n = 4 elseif n >= 5 then n = 0 end
    return n
end

-- 0x26A00 logout (callers run the 0x18776 fade first: 5 steps x 2 frames, 0x1088 each).
-- FFF0B1 = FF; sfx 0x29; DMA 0x15BE02 -> VRAM 0x9620 (0xA0 words); 0x10952; screen 0xE2E3A; then a "screen off"
-- line animation: 14 frames collapse (both axes), 6 frames horizontal, centre-dot blink 11+6+6+11 frames. d7 = 0.
M.LOGOUT_FADE_FRAMES, M.LOGOUT_FRAMES, M.LOGOUT_SOUND = 10, 14 + 6 + 11 + 6 + 6 + 11, 0x29
-- Deck byte uses [C]: +0x07 Response: Matrix delay FFF0A7 = 0x1D - (6 + 4*Response + 0xD81C(decker)) (0x17EB4);
--   pool +2*Response (0x55F64). +0x08 Bod: TN of the IC attack roll (0x56280, IC dice = rating, min TN 2).
--   +0x09 Evasion / +0x0A Masking / +0x0B Sensor: TNs of IC opposed rolls (0x56050.., 0x56148, 0x5659C..0x565B2);
--   Sensor also in the node detect test (Int + Computer + Sensor)/3 vs rand(10)+1 (0x54D64).
function M.matrix_delay(response, d81c) return (0x1D - (6 + 4 * response + d81c)) & 0xFF end
return M
