-- Party runner down (ROM 0xB330) and party wipe recovery (0xC64C/0xC716).
-- Record flags are the +0x10 status byte (E.RunnerStatus). Slot 0..2 are
-- FF0110/FF0210/FF0310. The host owns timers, text panels and scene moves.
local E = require("enums")

local PartyDown = {}

local ACTIVE = E.RunnerStatus.ACTIVE  -- 0x01
local DOWN = E.RunnerStatus.DOWN      -- 0x10
local DEAD = E.RunnerStatus.DEAD      -- 0x40

PartyDown.WIPE_COUNTDOWN = 0x3C       -- 0xABF2 FFE166 = 60 frames [C]
PartyDown.DOWN_SELECTOR_BASE = 0xE8   -- 0xC0B2 words E8..104 [C]
PartyDown.DOWN_LIST_INCREMENT = 0x200 -- C0B2 list header 0x8008, +0x200 [C]
PartyDown.LEADER_MIN_PHYSICAL = 6     -- 0xC750 [C]
PartyDown.LEADER_MIN_MENTAL = 8       -- 0xC764 [C]
PartyDown.NUYEN_PENALTY_CAP = 5000    -- 0xC7AA 0x1388 [C]
PartyDown.WAKE = E.PartyWipeDestination -- 0xC778 FFFC1E bit0 [C]

-- The source node's first longword is its page text pointer. Its second
-- longword is 0xFFFFFFFF for each one-page wipe message; 0x1F3C4 then waits
-- and returns before drawing option columns or following later pointers [C].
local RUN_FAILURE_PRESENTATION = {
    [E.RunType.BODYGUARD_ESCORT] = {
        event_id = "party_wipe_bodyguard_failure",
        text_key = "party_wipe.run_failure.bodyguard",
        clear_high_effects = true,
        clear_player_spawn_request = true,
    }, -- 0xF359E -> 0xF354B [C]
    [E.RunType.PACKAGE_DELIVERY] = {
        event_id = "party_wipe_package_failure",
        text_key = "party_wipe.run_failure.package",
        clear_run_objective = true,
    }, -- 0xF2510 -> 0xF2421 [C]
    [E.RunType.PACKAGE_RECOVERY] = {
        event_id = "party_wipe_package_failure",
        text_key = "party_wipe.run_failure.package",
        clear_run_objective = true,
    }, -- 0xC70C branches through the type-2 handler [C]
    [E.RunType.EMPLOYEE_RECRUITMENT] = {
        event_id = "party_wipe_employee_recruitment_failure",
        text_key = "party_wipe.run_failure.employee_recruitment",
        clear_high_effects = true,
        clear_player_spawn_request = true,
        clear_world_activity_request = true,
    }, -- 0xF3D5C -> 0xF3CD3 [C]
}

local pending_run_failure_event
local pending_wake_destination

-- 0xB05E word table indexed by record +0x0E (kind) [C].
local DOWN_SOUND = { [0] = 0x40, 0x40, 0x40, 0x2E, 0x2F, 0x2E, 0x58, 0x2E }

local function byte(v) return math.tointeger(v or 0) & 0xff end

-- 0xABB6 with the downing record's bit6 set by 0xB330: the leader must be
-- the downing record, and slots 1/2 must be inactive or DEAD [C].
local function all_down(slot, flags)
    local function dead(i)
        local f = flags[i]
        if i == slot then f = f | DEAD end
        if i == 0 then return f & DEAD ~= 0 end
        return f & ACTIVE == 0 or f & DEAD ~= 0
    end
    return dead(0) and dead(1) and dead(2)
end

-- Flat call: slot, flags0, flags1, flags2, kind, heading.
-- Returns { new_flags, sound_id, selector, wipe_countdown }.
function PartyDown.runner_down_flat(slot, f0, f1, f2, kind, heading)
    slot = math.tointeger(slot)
    assert(slot and slot >= 0 and slot <= 2, "party_down: slot 0..2")
    local flags = { [0] = byte(f0), byte(f1), byte(f2) }
    local wipe = all_down(slot, flags) and PartyDown.WIPE_COUNTDOWN or 0
    -- 0xB35C: dir = round(+0x2E / 0x2000) & 7.
    local h = math.tointeger(heading or 0) & 0xffff
    local d = ((h + 0x1000) >> 13) & 7
    return {
        flags[slot] | DOWN,
        DOWN_SOUND[math.tointeger(kind or 0) & 7] or 0,
        PartyDown.DOWN_SELECTOR_BASE + d * 4,
        wipe,
    }
end

-- Flat call: nuyen, story_bit0, then per slot (flags, physical, mental).
-- Returns { leader_flags, leader_physical, leader_mental, dismiss1,
--           dismiss2, nuyen, wake, hired1_delay, hired2_delay }.
function PartyDown.resolve_wipe_flat(nuyen, story_bit0, f0, p0, m0, f1, _p1,
                                     _m1, f2, _p2, _m2)
    -- 0xC716 clears hired +0x4C first, so 0xC802 dismisses every active
    -- hired runner [C]. The typed +0x4C delay-meter outputs are zeros.
    local dismiss1 = byte(f1) & ACTIVE ~= 0 and 1 or 0
    local dismiss2 = byte(f2) & ACTIVE ~= 0 and 1 or 0
    -- 0xC726: leader clears DEAD/DOWN, sets ACTIVE; meters floor at 6/8.
    local leader = (byte(f0) & ~(DEAD | DOWN) | ACTIVE) & 0xff
    local physical = math.max(byte(p0), PartyDown.LEADER_MIN_PHYSICAL)
    local mental = math.max(byte(m0), PartyDown.LEADER_MIN_MENTAL)
    -- 0xC7A2: FFFBFE -= min(FFFBFE >> 3, 5000).
    local funds = math.max(math.tointeger(nuyen or 0), 0)
    funds = funds - math.min(funds >> 3, PartyDown.NUYEN_PENALTY_CAP)
    local wake = (story_bit0 ~= 0 and story_bit0 ~= false)
        and PartyDown.WAKE.DOCWAGON or PartyDown.WAKE.LITTLE_CHIBA
    return { leader, physical, mental, dismiss1, dismiss2, funds, wake, 0, 0 }
end

-- ROM 0x391A walks forward from FFFC04, wrapping after a type-2 record. The
-- host supplies typed player-slot facts; C++ applies the returned selection
-- marker, active slot, HUD state, and field projection [C].
-- 0x39AE requires ACTIVE; 0x39B6 honors FFF09D, then 0x39C0/0x39C8 skip
-- records with DEAD or DOWN set. The selected runner's 0x1E540 marker reset
-- and the following world-actor call stay in the host/combat boundary [C].
function PartyDown.refresh_runner_after_wipe_flat(
    current_slot, suppression_override, type0, flags0, type1, flags1,
    type2, flags2)
    local refresh = E.PartyWipeRunnerRefresh
    local selected_flag = E.RunnerStatus.SELECTION_MARKER
    local active_flag = E.RunnerStatus.ACTIVE
    local suppressed_flags = E.RunnerStatus.DEAD | E.RunnerStatus.DOWN
    local flags = { [0] = byte(flags0), byte(flags1), byte(flags2) }
    local types = { [0] = byte(type0), byte(type1), byte(type2) }
    local previous = math.tointeger(current_slot or refresh.FIRST_SLOT)
    assert(previous and previous >= refresh.FIRST_SLOT and
           previous < refresh.SLOT_COUNT,
           "party_down: active runner slot out of range")

    local cursor = previous
    local selected = previous
    local override = suppression_override == true or
        math.tointeger(suppression_override or 0) ~= 0
    for _ = 1, refresh.SLOT_COUNT do
        flags[cursor] = flags[cursor] & ~selected_flag
        if types[cursor] == refresh.TERMINAL_RECORD_TYPE then
            cursor = refresh.FIRST_SLOT
        else
            cursor = cursor + 1
        end
        assert(cursor < refresh.SLOT_COUNT,
               "party_down: roster has no type-2 terminal record")

        flags[cursor] = flags[cursor] | selected_flag
        local candidate_flags = flags[cursor]
        local eligible = candidate_flags & active_flag ~= 0 and
            (override or candidate_flags & suppressed_flags == 0)
        if eligible then
            selected = cursor
            break
        end
    end

    return {
        selected,
        flags[refresh.FIRST_SLOT],
        flags[refresh.FIRST_SLOT + 1],
        flags[refresh.FIRST_SLOT + 2],
        selected ~= previous and 1 or 0,
        E.MenuSound.MOVE,
    }
end

-- FFFBED selects the source run handler. FFFC1C bit 0 must be set; only the
-- employee-recruitment branch also rejects bit 7. 0x1E68C clears FFFBEC and
-- FFFC1C after selecting these one-page messages [C].
function PartyDown.run_failure_flat(run_type, run_flags)
    run_type = math.tointeger(run_type or E.RunType.NONE)
    run_flags = byte(run_flags)
    if run_flags & E.RunContextFlag.ACTIVE == 0 then return nil end
    if run_type == E.RunType.EMPLOYEE_RECRUITMENT and
       run_flags & E.RunContextFlag.EMPLOYEE_FAILURE_GUARD ~= 0 then
        return nil
    end
    local presentation = RUN_FAILURE_PRESENTATION[run_type]
    if presentation == nil then return nil end
    return {
        run_type = run_type,
        event_id = presentation.event_id,
        text_key = presentation.text_key,
        clear_high_effects = presentation.clear_high_effects == true,
        clear_player_spawn_request =
            presentation.clear_player_spawn_request == true,
        clear_run_objective = presentation.clear_run_objective == true,
        clear_world_activity_request =
            presentation.clear_world_activity_request == true,
    }
end

-- Query and mutate the existing Lua run registry before the failure page,
-- matching 0xC64C's order before 0xC716. The single FFFC22 bit is changed only
-- when the host has restored that source word; nil remains explicitly unknown.
function PartyDown.prepare_run_failure_flat(world_activity_scene_reset,
                                             player_spawn_request_pending)
    local had_player_spawn_request =
        player_spawn_request_pending == true or
        math.tointeger(player_spawn_request_pending or 0) ~= 0
    local registry = game.runs.registry
    local run_state = registry.party_wipe_failure_state(
        world_activity_scene_reset)
    local failure = PartyDown.run_failure_flat(
        run_state.run_type, run_state.flags)
    if failure == nil then
        pending_run_failure_event = nil
        return { E.RunType.NONE, 0, 0, 0, 0,
                 had_player_spawn_request and 1 or 0 }
    end

    registry.clear_active_offer_after_party_wipe()
    local objective_flag_status = 0
    if failure.clear_run_objective then
        local key = "group_item_flag_bit_" ..
            E.GroupItemBit.RUN_OBJECTIVE_ACTIVE
        if game.state[key] == nil then
            objective_flag_status = 2 -- source FFFC22 not restored
        else
            game.state[key] = false
            objective_flag_status = 1
        end
    end
    pending_run_failure_event = failure.event_id
    return {
        failure.run_type,
        failure.clear_high_effects and 1 or 0,
        failure.clear_world_activity_request and 1 or 0,
        objective_flag_status,
        failure.clear_player_spawn_request and 1 or 0,
        had_player_spawn_request and 1 or 0,
    }
end

function PartyDown.present_run_failure()
    local event_id = pending_run_failure_event
    pending_run_failure_event = nil
    assert(event_id ~= nil, "party_down: no pending run-failure page")
    game.ui.say(event_id)
    return "run_failure_presented"
end

function PartyDown.wake_destination(story_bit0)
    return (story_bit0 == true or story_bit0 == 1)
        and E.PartyWipeDestination.DOCWAGON
        or E.PartyWipeDestination.LITTLE_CHIBA
end

function PartyDown.wake_arrival(story_bit0)
    local destination = PartyDown.wake_destination(story_bit0)
    return E.PartyWipeArrival[destination]
end

-- The script selects both scene and arrival point. C++ receives this typed
-- host command only after the wake passage closes, then runs the shared fade.
function PartyDown.take_pending_wake_destination_flat()
    local destination = pending_wake_destination
    pending_wake_destination = nil
    if destination == nil then return {} end
    local arrival = E.PartyWipeArrival[destination]
    assert(arrival ~= nil, "party_down: missing wake arrival data")
    return { arrival.scene, arrival.x, arrival.y }
end

-- Dialogue task: present the 0xC778 wake-up passage. DocWagon (C25A0) has
-- two records, Little Chiba (C25BC) one [C].
function PartyDown.present_wake()
    local StoryFlags = require("story_flags")
    local paid = StoryFlags.is_set(E.StoryFlag.MOTEL_BILL_PAID)
    pending_wake_destination = PartyDown.wake_destination(paid)
    if paid then
        game.ui.say("party_wipe_docwagon")
        game.ui.say("party_wipe_docwagon_dropoff")
        return "party_wipe_docwagon"
    end
    game.ui.say("party_wipe_little_chiba")
    return "party_wipe_little_chiba"
end

return PartyDown
