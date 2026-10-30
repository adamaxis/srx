-- Party runner down (ROM 0xB330) and party wipe recovery (0xC64C/0xC716).
local E = require("enums")

local PartyDown = {}

PartyDown.LEADER_MIN_PHYSICAL = 6     -- 0xC750 [C]
PartyDown.LEADER_MIN_MENTAL = 8       -- 0xC764 [C]
PartyDown.NUYEN_PENALTY_CAP = 5000    -- 0xC7AA 0x1388 [C]

-- The source node's first longword is its page text pointer. Its second
-- longword is 0xFFFFFFFF for each one-page wipe message; 0x1F3C4 then waits
-- and returns before drawing option columns or following later pointers [C].
local RUN_FAILURE_PRESENTATION = {
    [E.RunId.BODYGUARD_ESCORT] = {
        event_id = "party_wipe_bodyguard_failure",
        text_key = "party_wipe.run_failure.bodyguard",
        clear_high_effects = true,
        clear_player_spawn_request = true,
    }, -- 0xF359E -> 0xF354B [C]
    [E.RunId.PACKAGE_DELIVERY] = {
        event_id = "party_wipe_package_failure",
        text_key = "party_wipe.run_failure.package",
        clear_run_objective = true,
    }, -- 0xF2510 -> 0xF2421 [C]
    [E.RunId.PACKAGE_RECOVERY] = {
        event_id = "party_wipe_package_failure",
        text_key = "party_wipe.run_failure.package",
        clear_run_objective = true,
    }, -- 0xC70C branches through the type-2 handler [C]
    [E.RunId.EMPLOYEE_RECRUITMENT] = {
        event_id = "party_wipe_employee_recruitment_failure",
        text_key = "party_wipe.run_failure.employee_recruitment",
        clear_high_effects = true,
        clear_player_spawn_request = true,
        clear_world_activity_request = true,
    }, -- 0xF3D5C -> 0xF3CD3 [C]
}

local pending_run_failure_event
local pending_wake_destination

-- The field host supplies gameplay facts and applies the returned lifecycle
-- transition to its native party model. Source flags, animation selectors,
-- audio IDs, and frame counters stay at their owning host boundaries.
-- ROM 0xB330 temporarily marks the target dead while 0xABB6 checks whether
-- the leader and both companion positions are unavailable; it then marks the
-- target down and arms the recovery delay when that check succeeds [C].
function PartyDown.enter_runner_down_state(context)
    assert(type(context) == "table" and type(context.party) == "table",
           "party_down: missing party facts")
    local target = context.target_role
    assert(target == "leader" or target == "companion_one" or
           target == "companion_two", "party_down: unknown target role")

    local party = context.party
    local leader = party.leader
    local companion_one = party.companion_one
    local companion_two = party.companion_two
    assert(type(leader) == "table" and type(companion_one) == "table" and
           type(companion_two) == "table",
           "party_down: incomplete party lifecycle facts")

    local leader_is_dead = leader.dead == true or target == "leader"
    local function companion_is_unavailable(member, role)
        return member.active ~= true or member.dead == true or target == role
    end

    return {
        lifecycle = { downed = true },
        recovery = {
            party_wipe_imminent = leader_is_dead and
                companion_is_unavailable(companion_one, "companion_one") and
                companion_is_unavailable(companion_two, "companion_two"),
        },
    }
end

-- Recovery receives semantic roster and wallet state. The field host applies
-- the returned lifecycle, meter, dismissal, and travel decisions to its
-- native systems. ROM 0xC716 dismisses active companions, 0xC726 revives the
-- leader with meter floors, and 0xC7A2 subtracts one eighth of current Nuyen
-- up to 5,000; the paid motel bill selects the DocWagon arrival at 0xC778 [C].
function PartyDown.resolve_party_wipe(context)
    assert(type(context) == "table" and type(context.party) == "table" and
           type(context.wallet) == "table",
           "party_down: missing recovery facts")
    local leader = context.party.leader
    local companion_one = context.party.companion_one
    local companion_two = context.party.companion_two
    assert(type(leader) == "table" and type(leader.meters) == "table" and
           type(leader.lifecycle) == "table" and
           type(companion_one) == "table" and
           type(companion_one.lifecycle) == "table" and
           type(companion_two) == "table" and
           type(companion_two.lifecycle) == "table",
           "party_down: incomplete recovery facts")

    local funds = math.max(math.tointeger(context.wallet.nuyen or 0), 0)
    local penalty = math.min(funds >> 3, PartyDown.NUYEN_PENALTY_CAP)
    return {
        leader = {
            lifecycle = { active = true, downed = false, dead = false },
            meters = {
                physical = math.max(
                    math.tointeger(leader.meters.physical or 0),
                    PartyDown.LEADER_MIN_PHYSICAL),
                mental = math.max(
                    math.tointeger(leader.meters.mental or 0),
                    PartyDown.LEADER_MIN_MENTAL),
            },
        },
        companions = {
            companion_one = {
                dismissed = companion_one.lifecycle.active == true,
                physical_meter = 0,
            },
            companion_two = {
                dismissed = companion_two.lifecycle.active == true,
                physical_meter = 0,
            },
        },
        wallet = { nuyen = funds - penalty },
        wake_destination = context.motel_bill_paid == true
            and "docwagon" or "little_chiba",
    }
end

-- ROM 0x391A walks party roles in order and wraps after the role marked as
-- the roster end. It chooses an active member, skipping dead/downed members
-- unless the suppression override is enabled. The host supplies those named
-- facts and applies the active role and transient selection indicators [C].
function PartyDown.refresh_runner_selection(context)
    assert(type(context) == "table" and type(context.party) == "table",
           "party_down: missing roster facts")
    local roles = { "leader", "companion_one", "companion_two" }
    local previous = context.previous_role
    assert(previous == roles[1] or previous == roles[2] or
           previous == roles[3], "party_down: unknown current party role")

    local party = context.party
    local selection_pending = {}
    for _, role in ipairs(roles) do
        local member = party[role]
        assert(type(member) == "table" and
               type(member.lifecycle) == "table",
               "party_down: incomplete roster member facts")
        selection_pending[role] = member.selection_pending == true
    end

    local cursor = previous
    local selected = previous
    local override = context.suppression_override == true
    for _ = 1, #roles do
        local current_member = party[cursor]
        selection_pending[cursor] = false
        local next_role
        if current_member.ends_roster == true then
            next_role = roles[1]
        else
            local current_index
            for index, role in ipairs(roles) do
                if role == cursor then
                    current_index = index
                    break
                end
            end
            assert(current_index ~= nil and current_index < #roles,
                   "party_down: roster end role is missing")
            next_role = roles[current_index + 1]
        end

        selection_pending[next_role] = true
        local candidate = party[next_role]
        local eligible = candidate.lifecycle.active == true and
            (override or
             (candidate.lifecycle.dead ~= true and
              candidate.lifecycle.downed ~= true))
        if eligible then
            selected = next_role
            break
        end
        cursor = next_role
    end

    return {
        active_role = selected,
        selection_pending = selection_pending,
        active_role_changed = selected ~= previous,
    }
end

-- ROM 0xC64C dispatches by run identity when a run is active. Employee
-- Recruitment alone is suppressed by the world-activity scene-reset fact;
-- 0x1E68C clears the active run after a selected failure page [C].
function PartyDown.select_run_failure(run_state)
    if type(run_state) ~= "table" or run_state.active ~= true then return nil end
    if run_state.run_id == E.RunId.EMPLOYEE_RECRUITMENT and
       run_state.employee_failure_guard == true then
        return nil
    end
    local presentation = RUN_FAILURE_PRESENTATION[run_state.run_id]
    if presentation == nil then return nil end
    return {
        run_id = run_state.run_id,
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

-- Query and mutate the Lua run registry before the failure page, matching
-- 0xC64C's order before 0xC716. Inputs and output use named facts; the native
-- host resolves whether the run-objective story fact is available [C].
function PartyDown.prepare_run_failure(context)
    assert(type(context) == "table",
           "party_down: missing run-failure context")
    local registry = game.runs.registry
    local run_state = registry.party_wipe_failure_state(
        context.world_activity_scene_reset == true)
    local failure = PartyDown.select_run_failure(run_state)
    if failure == nil then
        pending_run_failure_event = nil
        return {
            page_pending = false,
            clear_high_effects = false,
            clear_world_activity_request = false,
            objective_result = "unchanged",
            clear_player_spawn_request = false,
        }
    end

    registry.clear_active_offer_after_party_wipe()
    local objective_result = "unchanged"
    if failure.clear_run_objective then
        local key = "group_item_flag_bit_" ..
            E.GroupItemBit.RUN_OBJECTIVE_ACTIVE
        if game.state[key] == nil then
            objective_result = "unavailable"
        else
            game.state[key] = false
            objective_result = "cleared"
        end
    end
    pending_run_failure_event = failure.event_id
    return {
        page_pending = true,
        clear_high_effects = failure.clear_high_effects,
        clear_world_activity_request = failure.clear_world_activity_request,
        objective_result = objective_result,
        clear_player_spawn_request = failure.clear_player_spawn_request,
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
    local StoryFacts = require("story_facts")
    local paid = StoryFacts.is_set(E.StoryFact.MOTEL_BILL_PAID)
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
