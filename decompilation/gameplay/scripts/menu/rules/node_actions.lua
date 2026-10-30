-- Matrix node-action policy. Source addresses remain provenance; Lua uses
-- semantic run, node, field, and file facts at runtime.
local E = require("enums")
local FieldAlarm = require("field_alarm")
local DECK = require("menu.rules.deck")
local SPECIAL = require("menu.rules.datastore_special")
local M = {}
M.TEXT = { ALERT_CANCELLED = 0x1666A8, NO_ALERTS = 0x1666DD, SHUTDOWN = 0x166A44, OFFLINE = 0x16684A,
    ERASE_OBJECTIVE = 0x1668CE, ERASED = 0x16685A, LOCKOUT = 0x1669A2, SEARCHING = 0x16674D, NOTHING = 0x1667A5,
    OBJECTIVE_FOUND = 0x166770, UPLOAD_DONE = 0x16692D, STORED_OBJECTIVE = 0x1667DD, STORED = 0x1667F8,
    SEARCH_DOT = 0x166848, STORY_FILE = 0x166968, DOWNLOAD_FILE = 0x1667CE,
    DOWNLOAD_CANCELLED = 0x1667BA, FIVE_FILES = 0x16681B, NO_STORAGE = 0xE383D }
-- 0xC24 waits d5 + 1 VBlanks [C]: search dots 0x54D3A d5 = 0x14, search tail
-- 0x54D50 d5 = 9, Crash System digits 0x54BAE d5 = 0x3C.
M.SEARCH_DOT_FRAMES = 0x14 + 1
M.SEARCH_TAIL_FRAMES = 9 + 1
M.SHUTDOWN_STEP_FRAMES = 0x3C + 1

local function complete(run)                       -- shared run completion (0x54B34 / 0x54F72 / 0x5559E)
    run.completion_marker_present = false
    run.completion_recorded = true
    run.status = E.MatrixRunStatus.COMPLETED
end
-- [C] 0x54C6A / 0x5559E require an active objective and matching file class.
local function on_objective(run, node, current_system_id)
    if run == nil or node == nil or
       run.status ~= E.MatrixRunStatus.ACTIVE or
       type(run.objective) ~= "table" or
       run.objective.node ~= "datastore" or
       node.file_class ~= run.objective.file_class then
        return false
    end
    if run.target_system_id ~= nil and
       run.target_system_id ~= current_system_id then
        return false
    end
    return true
end

-- 0x54AEA Cancel Alert: no roll.
function M.cancel_alert(st)
    if st.alert >= 0 then
        st.alert = -1
        return { text = M.TEXT.ALERT_CANCELLED, redraw = true }
    end
    return { text = M.TEXT.NO_ALERTS }
end
-- 0x54B34 Crash System: countdown 3..0 at 61 VBlanks each, fade 0x18776, logout 0x26A00.
function M.crash_system(run, field, actors)
    if run and run.status == E.MatrixRunStatus.ACTIVE and
       type(run.objective) == "table" and
       run.objective.node == "cpu" and
       run.objective.action == E.MatrixRunAction.CRASH_SYSTEM then
        complete(run)
    end
    M.alarm_off(field, actors)                          -- 0x1E09A
    field.security_off, field.maglocks_off = true, true
    return { text = M.TEXT.SHUTDOWN, countdown = { 3, 2, 1, 0 }, step = M.SHUTDOWN_STEP_FRAMES, logout = true, stop_sfx = E.SoundId.ALARM }
end
-- 0x1E09A alarm off: if FFE16F set -> clear; actors FF0400..FF0A00 with +0x9E bit1 get bit2;
-- always stops E.SoundId.ALARM (0x30) through 0x530D2 (FF 12 30).
function M.alarm_off(field, actors)
    local result = FieldAlarm.clear(field, actors)
    return result ~= nil and result.was_active == true -- caller stops E.SoundId.ALARM
end
-- [C] 0x54BD2 dispatches class-specific shutdown effects. The native rules
-- select those effects using the resolved file-class name.
M.TURN_OFF = {
    ["Security Cntrl"] = { alarm = true, security = true },
    ["Cameras"] = { security = true },
    ["Maglocks"] = { maglocks = true },
    ["Alert Control"] = { alarm = true },
}
function M.turn_off_node(node, field, actors)
    local e = M.TURN_OFF[node.file_class]
    local stop_sfx
    if e then
        if e.alarm then
            M.alarm_off(field, actors)
            stop_sfx = E.SoundId.ALARM
        end
        if e.security then field.security_off = true end   -- FFE170 [H: suppresses guard spawns 0x163DA/0x1DFD8]
        if e.maglocks then field.maglocks_off = true end   -- FFE17F maglocks disengaged
    end
    return { text = M.TEXT.OFFLINE, stop_sfx = stop_sfx }
end
-- [C] 0x5559E Erase completes a matching Delete objective with value zero.
function M.erase(run, node, current_system_id)
    if on_objective(run, node, current_system_id) and
       run.objective.action == E.MatrixRunAction.DELETE_FILE then
        complete(run)
        return { text = M.TEXT.ERASE_OBJECTIVE }
    end
    return { text = M.TEXT.ERASED }
end
-- 0x5561E Lockout: text only.
function M.lockout() return { text = M.TEXT.LOCKOUT } end

-- 0x54C5E Transfer Data, phase 1 (search). r = datastore_special record (system, flags, nuyen ...).
M.VALUE = { [0]=0, 1, 3, 0, 2, 0, 5, 4, 6, 8, 0, 0 }      -- 0x16668A
-- The objective check precedes the browsing animation. Upload objectives
-- complete immediately; file objectives show their named target file.
function M.begin_search(run, node, current_system_id)
    if on_objective(run, node, current_system_id) then
        if run.objective.action == E.MatrixRunAction.UPLOAD_FILE then
            complete(run)
            return { text = M.TEXT.UPLOAD_DONE }
        end
        if run.objective.file_id ~= nil then
            return { text = M.TEXT.OBJECTIVE_FOUND,
                     file = { run_objective = true,
                              id = run.objective.file_id,
                              size = run.file_size } }
        end
    end
    return nil
end

-- Called after the ROM's four 20-frame dots and final 9-frame pause. The deck
-- search reads runner +0x7B/+0x88 and deck +0x0B, then uses rand(10)+1.
local function detection_score(decker, deck)
    decker, deck = decker or {}, deck or {}
    local attributes = decker.attributes or {}
    local skills = decker.skills or {}
    local intelligence = decker.int
    if intelligence == nil then intelligence = attributes[4] or 0 end
    local computer = decker.computer
    if computer == nil then computer = skills[7] or 0 end
    local sensor = deck.sensor
    if sensor == nil then sensor = (deck.persona or {})[3] or 0 end
    return (intelligence + computer + sensor) // 3
end

function M.finish_search(node, decker, deck, special, rand)
    if special and SPECIAL.can_match(special, node.class) and
       SPECIAL.matches(special, node.class) then
        return { story = true, text = M.TEXT.STORY_FILE }
    end
    if detection_score(decker, deck) < rand(10) + 1 then
        return { text = M.TEXT.NOTHING }
    end
    return { file = { id = node.class * 5 + rand(5), size = (rand(5) + 2) * 10 } }
end
function M.search(run, node, decker, deck, special, rand,
                  current_system_id)
    local immediate = M.begin_search(run, node, current_system_id)
    if immediate then return immediate end
    return M.finish_search(node, decker, deck, special, rand)
end
function M.file_name(id) return DECK.FILE_NAMES[id] end
-- Phase 2 (after "Download File?" A = yes). Returns message key or stored slot.
function M.store(rec, run, node, file, rand)
    if not DECK.storage_fits_file(rec, file.size) then
        return nil, M.TEXT.NO_STORAGE
    end
    local k; for i = 0, DECK.FILES - 1 do if rec.files[i].id < 0 then k = i; break end end
    if not k then return nil, M.TEXT.FIVE_FILES end
    local id = file.id
    if id == nil then id = node.class * 5 end
    local f = rec.files[k]; f.id, f.size = id, file.size
    f.run_objective = file.run_objective == true
    if f.run_objective then
        f.data_value = 0
        complete(run)
        return k, M.TEXT.STORED_OBJECTIVE
    end
    local m = node.colour == 4 and 5 or node.colour - 1
    f.data_value = (M.VALUE[rand(7) + rand(6)] * m) & 0xFF
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
-- (FFF0B1) text 0x1669E8, 26-VBlank wait (0xC24 d5 = 0x19), button (0xD94).
function M.random_alert(st, rand)
    if rand(10) >= (st.attempts or 0) then return false end
    st.attempts = 0; return true
end
return M
