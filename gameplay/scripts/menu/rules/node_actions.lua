-- rules/node_actions.lua: node-action handlers 0x54AEA..0x5561E, ROM-traced 2026-10-15 [C unless marked].
-- node = FF1100 record view (class +1, colour +5, ic_condition +0x0F); st = Matrix RAM (alert FFF0A0);
-- run = {state FFF0E3, objective FFFBF2, value FFFBF6 (signed), size FFFBF7, fc28, fc1c};
-- field = {alarm FFE16F, security_off FFE170, maglocks_off FFE17F}. rand(n) = 0xD3A -> 0..n-1. Texts are ROM addresses.
local DECK = require("menu.rules.deck")
local SPECIAL = require("menu.rules.datastore_special")
local M = {}
M.TEXT = { ALERT_CANCELLED = 0x1666A8, NO_ALERTS = 0x1666DD, SHUTDOWN = 0x166A44, OFFLINE = 0x16684A,
    ERASE_OBJECTIVE = 0x1668CE, ERASED = 0x16685A, LOCKOUT = 0x1669A2, SEARCHING = 0x16674D, NOTHING = 0x1667A5,
    OBJECTIVE_FOUND = 0x166770, UPLOAD_DONE = 0x16692D, STORED_OBJECTIVE = 0x1667DD, STORED = 0x1667F8 }
M.SEARCH_DOT_FRAMES, M.SHUTDOWN_STEP_FRAMES = 20, 60

local function complete(run)                       -- shared "run complete" (0x54B34 / 0x54F72 / 0x5559E)
    run.fc28 = run.fc28 & ~1; run.state = 1; run.fc1c = run.fc1c | 0x80
end
-- 0x54C6A / 0x555A? run check: FFF0E3 < 0 (run in progress) and node class byte == FFFBF2 byte [C].
local function on_objective(run, node) return run.state < 0 and (node.class & 0xFF) == (run.objective & 0xFF) end

-- 0x54AEA Cancel Alert: no roll.
function M.cancel_alert(st)
    if st.alert >= 0 then st.alert = -1; return { text = M.TEXT.ALERT_CANCELLED, redraw = true } end
    return { text = M.TEXT.NO_ALERTS }
end
-- 0x54B34 Crash System: countdown 3..0 at 60 frames, fade 0x18776, logout 0x26A00.
function M.crash_system(run, field)
    if run and run.state < 0 and run.objective < 0 then complete(run) end
    field.alarm = false                                -- 0x1E09A (see M.alarm_off)
    field.security_off, field.maglocks_off = true, true
    return { text = M.TEXT.SHUTDOWN, countdown = { 3, 2, 1, 0 }, step = M.SHUTDOWN_STEP_FRAMES, logout = true }
end
-- 0x1E09A alarm off: if FFE16F set -> clear; actors FF0400..FF0A00 with +0x9E bit1 get bit2; stop sfx 0x30.
function M.alarm_off(field, actors)
    if not field.alarm then return false end
    field.alarm = false
    for _, a in ipairs(actors or {}) do if a.flags9e & 2 ~= 0 then a.flags9e = a.flags9e | 4 end end
    return true                                        -- caller stops sfx 0x30
end
-- 0x54BD2 Turn Off Node by class (+1).
M.TURN_OFF = { [0x27] = { alarm = true, security = true }, [0x28] = { security = true },
               [0x29] = { maglocks = true }, [0x2A] = { alarm = true } }
function M.turn_off_node(node, field, actors)
    local e = M.TURN_OFF[node.class]
    if e then
        if e.alarm then M.alarm_off(field, actors) end
        if e.security then field.security_off = true end   -- FFE170 [H: suppresses guard spawns 0x163DA/0x1DFD8]
        if e.maglocks then field.maglocks_off = true end   -- FFE17F maglocks disengaged
    end
    return { text = M.TEXT.OFFLINE }
end
-- 0x5559E Erase: objective "erase" when class == FFFBF2 and FFFBF6 == 0.
function M.erase(run, node)
    if on_objective(run, node) and run.value == 0 then complete(run); return { text = M.TEXT.ERASE_OBJECTIVE } end
    return { text = M.TEXT.ERASED }
end
-- 0x5561E Lockout: text only.
function M.lockout() return { text = M.TEXT.LOCKOUT } end

-- 0x54C5E Transfer Data, phase 1 (search). r = datastore_special record (system, flags, nuyen ...).
M.VALUE = { [0]=0, 1, 3, 0, 2, 0, 5, 4, 6, 8, 0, 0 }      -- 0x16668A
function M.search(run, node, st, decker, deck, special, rand)
    st.fff0ae = 0
    if on_objective(run, node) then
        if run.value < 0 then complete(run); return { text = M.TEXT.UPLOAD_DONE } end
        st.fff0ae = 0xFF                               -- 0x54CDA: marker byte, stored as file +0x21
        st.fff096, st.fff098 = run.size, run.value & 0xFF
        return { text = M.TEXT.OBJECTIVE_FOUND, file = { objective = true, size = run.size, value = run.value } }
    end
    if special and SPECIAL.matches(special, node.class) then return { story = true } end
    if (decker.int + decker.computer + deck.sensor) // 3 < rand(10) + 1 then return { text = M.TEXT.NOTHING } end
    return { file = { id = node.class * 5 + rand(5), size = (rand(5) + 2) * 10 } }
end
-- Phase 2 (after "Download File?" A = yes). Returns message key or stored slot.
function M.store(rec, run, node, st, file, rand)
    if DECK.free_storage(rec) < file.size then return "NO_STORAGE" end
    local k; for i = 0, DECK.FILES - 1 do if rec.files[i].id < 0 then k = i; break end end
    if not k then return "FIVE_FILES" end
    local f = rec.files[k]; f.id, f.size = file.id or node.class * 5, file.size
    if file.objective then
        -- The 0x54C5E objective branch stores FFF0AE's 0xFF marker at
        -- data-file +0x21; expose it as the signed host value -1.
        f.value = -1
        complete(run)
        return k, M.TEXT.STORED_OBJECTIVE
    end
    local m = node.colour == 4 and 5 or node.colour - 1
    f.value = (M.VALUE[rand(7) + rand(6)] * m) & 0xFF
    return k, M.TEXT.STORED
end
-- Node-action loop 0x54662 gate (A edge, 0x548A0): dice = min(pool, Computer +0x88) + Computer (0 -> 1) - (colour - 1);
-- <= 0 or roll(dice, security +6) <= 0 -> text 0x166AA5; else dispatch 0x1664D8[function].+6[cursor]. FFF0A8 += 1 either way.
M.TEXT.ACTION_FAILED, M.TEXT.SYSTEM_ALERT = 0x166AA5, 0x1669E8

-- ROM 0x54662 calls 0x55F44 with request D1=0, then combines that result
-- with runner +0x88. The typed menu record maps +0x78/+0x7B to attributes[1]
-- and [4], +0x88 to skills[7], and deck +0/+7 to MPCP/Response. The node loop
-- enters with Matrix mode -1 (FFF0B7); keep the byte additions and divide
-- rounding from the source routine before handing the result to gate().
function M.runner_action_inputs(runner, deck, mode)
    local attributes = runner.attributes or {}
    local skills = runner.skills or {}
    local computer = skills[7] or 0
    local delay = (((attributes[1] or 0) + (attributes[4] or 0)) & 0xFF) >> 1
    delay = (delay + computer + 2 * (deck.response or 0)) & 0xFF
    if mode > 0 then
        delay = delay // 3
    elseif mode < 0 then
        delay = delay - delay // 3
    else
        delay = delay >> 1
    end
    return delay, computer
end

function M.gate_runner_action(runner, deck, mode, node, st, roll)
    local delay, computer = M.runner_action_inputs(runner, deck, mode)
    return M.gate(delay, computer, node, st, roll)
end

function M.gate(pool, computer, node, st, roll)
    local d = math.min(pool, computer) + computer; if d == 0 then d = 1 end
    d = d - (node.colour - 1)
    local ok = d > 0 and roll(d, node.security) > 0
    st.attempts = ((st.attempts or 0) + 1) & 0xFF
    return ok
end
-- 0x54A8E random alert (each loop pass): rand(10) < FFF0A8 -> FFF0A8 = 0, raise_alert(+1) (0x1989A); unless logged out
-- (FFF0B1) text 0x1669E8, 25-frame wait (0xC24), button (0xD94).
function M.random_alert(st, rand)
    if rand(10) >= (st.attempts or 0) then return false end
    st.attempts = 0; return true
end
return M
