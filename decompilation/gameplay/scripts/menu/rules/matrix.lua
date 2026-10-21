-- rules/matrix.lua: Matrix state rules decoded 2026-10-15 [C unless marked].
-- Node record (FF1100 + node*0x14, docs/05 s8): +3 x, +4 y, +0x0F IC condition, +0x11/+0x12 IC rating(s), +0x13 IC code.
-- FFF0A0 = system alert: 0xFF none, 0 passive, 1 active (texts 0xE3074 + (a+1)*0xE: No / Passive / Active Alert).
-- FFF0B0 = jacked in: FF only between 0x17410 and 0x17422 (around the Matrix session 0x1745E); deck menus opened
--   from the Pocket Secretary see 0 (Pull the plug and Show Map/Retreat are inert) [C].
local M = {}
M.NONE, M.PASSIVE, M.ACTIVE = -1, 0, 1
-- The Matrix exit used by overload (0x1989A) and Crash System (0x54B34)
-- calls 0x18776 (five palette steps, two VBlanks each), then 0x26A00
-- (14 + 6 sweep/settle ticks and 34 timed restore ticks). The host presents
-- those 64 frames as a fade to black plus a clean display handoff; the source
-- transition artwork remains a separate asset/presentation task.
M.EXIT_FADE_FRAMES = 10
M.EXIT_DISPLAY_TRANSITION_FRAMES = 54
M.EXIT_TRANSITION_FRAMES =
    M.EXIT_FADE_FRAMES + M.EXIT_DISPLAY_TRANSITION_FRAMES
M.EXIT_FADE_STEPS = 5
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
-- 0x19B40: IC type of the current node. map_seen = FFFB30[FFF102] >> 4 (0x17C3C);
-- world_effect_gate_negative = FFDF84 < 0. FFF0A0 alert and FFDF84 are separate.
function M.ic_type(node, alert, world_effect_gate_negative, map_seen)
    local code = node.ic_code
    if node.ic_condition <= 0 or node.ic_condition >= 0x80 or
       type(code) ~= "number" or code < 0 or code > 26 then
        return M.NO_IC
    end -- signed BLE/BMI guards at 0x19B58/0x19B64; table ends after code 26.
    if not world_effect_gate_negative and map_seen <= 0 then return M.UNSEEN end
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

-- Map 0x532BC: 14x10 cells. map[cell] = system map byte (0x15DB48[sys].map); <= 0x0B node cells, >= 0x0C connectors.
-- 0x533EE reveal: cur = y*14+x of the current node (FFF102), known[cur] = FF, then flood 0x53434 in 4 directions with a
-- "came from" block bit. Connector sets: CONNECT_DOWN 0x15FCAF, CONNECT_LEFT 0x15FC9A, CONNECT_UP 0x15FCA8,
-- CONNECT_RIGHT 0x15FCA1. Quirk [C]: left/right moves do not stop at row edges (only 0 <= cell <= 0x8B is checked).
M.W, M.H, M.LAST = 14, 10, 0x8B
-- 0x532BC renders these six fixed row labels from 0x15FD36 at plane-A
-- address 0xC204; the host presents the same text from the extracted font.
M.MAP_LABELS = { "CPU", "SPU", "IOP", "SAN", "SM", "DS" }
local function set(t) local s = {}; for _, v in ipairs(t) do s[v] = true end; return s end
M.CONNECT_LEFT, M.CONNECT_RIGHT = set{12,14,15,18,19,21,22}, set{12,16,17,18,19,20,22}
M.CONNECT_UP, M.CONNECT_DOWN = set{13,14,16,18,20,21,22}, set{13,15,17,19,20,21,22}
local DIRS = {   -- order 0x53434: up, right, down, left; {delta, block-bit, neighbour-set, own-set, block-for-child}
    { -14, "up",    "CONNECT_DOWN",  "CONNECT_UP",    "down" },
    {   1, "right", "CONNECT_LEFT",  "CONNECT_RIGHT", "left" },
    {  14, "down",  "CONNECT_UP",    "CONNECT_DOWN",  "up" },
    {  -1, "left",  "CONNECT_RIGHT", "CONNECT_LEFT",  "right" },
}
local function flood(map, known, expanded, cur, block)
    -- The ROM uses connector masks to avoid walking straight back across the
    -- edge it just traversed. A closed connector loop can still revisit a
    -- cell, so track this reveal pass to keep traversal finite.
    if expanded[cur] then return end
    expanded[cur] = true
    for _, d in ipairs(DIRS) do
        if not block[d[2]] then
            local c = cur + d[1]
            if c >= 0 and c <= M.LAST then
                if map[c] > 0x0B then
                    if M[d[3]][map[c]] then
                        known[c] = true
                        flood(map, known, expanded, c, { [d[5]] = true })
                    end
                elseif M[d[4]][map[cur]] then known[c] = true end
            end
        end
    end
end
function M.reveal(map, known, x, y)
    local cur = y * M.W + x
    known[cur] = true
    flood(map, known, {}, cur, {})
    return known
end
-- 0x53338 draw: unknown cell -> blank tile 0x15C3CC. Known: status nibble FFFB30[cell] & 0xF selects
-- palette/tile table: 0 -> pal 1 (0x2000) tbl 0x15FC0E; 1 -> pal 2 0x15FC0E; 2 -> pal 3 0x15FC0E; 3 -> pal 2 0x15FC6A;
-- >= 4 -> pal 3 0x15FC6A; tile = tbl[map byte].
-- FFFB30[cell] [C, 2026-10-15]: low nibble = node colour (+5: 1 Blue, 2 Green, 3 Orange, 4 Red) OR'd in on the first
--   visit (0x19DAC; 0 = never visited -> grey pal 1). High nibble = what is known about the IC: min(analyze net, 4)
--   (0x19D50), 4 when the node has no IC (0x17A88 ors 0x40) or after a node action (0x54676 sets 4); > 0 = "seen" for
--   0x19B40. Entering the CPU (fn 1) at 0x54676 also reveals the whole map (0x532A8: FFFAA4 all FF) and arms FFF0A6.
-- Tile tables (4 name words = 2x2 cell, 0x15C3CC = blank): TILE_A[0..22] (0 empty, 1..11 node icons, 12..22 connectors),
--   TILE_B[0..11] (node icons for Orange/Red). Quirk: TILE_B has no connector rows; colour is only ever OR'd on node
--   cells, so 12..22 never index it [C].
M.TILE_A = { [0] = {0x00BF,0x00BF,0x00BF,0x00BF}, {0x00C0,0x08C0,0x10C0,0x18C0}, {0x00CD,0x08CD,0x10CD,0x18CD}, {0x00D0,0x08D0,0x10D0,0x18D0}, {0x00D5,0x08D5,0x10D5,0x18D5}, {0x08C2,0x00C2,0x08C9,0x00C9}, {0x00D1,0x08D1,0x10D1,0x18D1}, {0x00C1,0x00C2,0x00C8,0x00C9}, {0x00C1,0x08C1,0x00C8,0x08C8}, {0x08C2,0x08C1,0x08C9,0x08C8}, {0x00D1,0x08D1,0x10D6,0x18D6}, {0x00D6,0x08D6,0x10D1,0x18D1}, {0x00C6,0x08C6,0x10C6,0x18C6}, {0x00CF,0x08CF,0x10CF,0x18CF}, {0x00C7,0x00D4,0x00CC,0x00BF}, {0x10CC,0x00BF,0x10C7,0x10D4}, {0x08D4,0x08C7,0x00BF,0x08CC}, {0x00BF,0x18CC,0x18D4,0x18C7}, {0x00C7,0x08C7,0x00CC,0x08CC}, {0x10CC,0x18CC,0x10C7,0x18C7}, {0x08D4,0x08C7,0x18D4,0x18C7}, {0x00C7,0x00D4,0x10C7,0x10D4}, {0x00C7,0x08C7,0x10C7,0x18C7} }
M.TILE_B = { [0] = {0x00BF,0x00BF,0x00BF,0x00BF}, {0x00C3,0x08C3,0x10C3,0x18C3}, {0x00CE,0x08CE,0x10CE,0x18CE}, {0x00D2,0x08D2,0x10D2,0x18D2}, {0x00D7,0x08D7,0x10D7,0x18D7}, {0x08C5,0x00C5,0x08CB,0x00CB}, {0x00D3,0x08D3,0x10D3,0x18D3}, {0x00C4,0x00C5,0x00CA,0x00CB}, {0x00C4,0x08C4,0x00CA,0x08CA}, {0x08C5,0x08C4,0x08CB,0x08CA}, {0x00D3,0x08D3,0x10D8,0x18D8}, {0x00D8,0x08D8,0x10D3,0x18D3} }
function M.cell_style(nibble)
    if nibble == 0 then return 0x2000, 0x15FC0E elseif nibble == 1 then return 0x4000, 0x15FC0E
    elseif nibble == 2 then return 0x6000, 0x15FC0E elseif nibble == 3 then return 0x4000, 0x15FC6A end
    return 0x6000, 0x15FC6A
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
