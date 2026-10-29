-- party_down.lua: ROM 0xB330/0xABB6 runner down and 0xC64C/0xC716 wipe.
local P = require("party_down")
local E = require("enums")
local RunRegistry = require("run_registry")

local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

local function party_facts(target_role, companion_one, companion_two)
    return {
        target_role = target_role,
        party = {
            leader = { active = true, dead = false },
            companion_one = companion_one,
            companion_two = companion_two,
        },
    }
end

-- The script receives party lifecycle facts. Audio selection, actor facing,
-- source flags, and the native countdown are handled outside Lua.
local alone = P.enter_runner_down_state(party_facts(
    "leader", { active = false, dead = false },
    { active = false, dead = false }))
check(alone.lifecycle.downed, "leader becomes downed")
check(alone.recovery.party_wipe_imminent,
      "lone leader starts party recovery")
local live_companion = P.enter_runner_down_state(party_facts(
    "leader", { active = true, dead = false },
    { active = false, dead = false }))
check(not live_companion.recovery.party_wipe_imminent,
      "active companion blocks party recovery")
local dead_companion = P.enter_runner_down_state(party_facts(
    "leader", { active = true, dead = true },
    { active = false, dead = false }))
check(dead_companion.recovery.party_wipe_imminent,
      "dead companion allows party recovery")
local companion_down = P.enter_runner_down_state(party_facts(
    "companion_one", { active = true, dead = false },
    { active = false, dead = false }))
check(companion_down.lifecycle.downed and
      not companion_down.recovery.party_wipe_imminent,
      "companion downing does not wipe a living leader")

local function recovery_facts(nuyen, motel_bill_paid, leader, companion_one,
                              companion_two)
    return {
        wallet = { nuyen = nuyen },
        motel_bill_paid = motel_bill_paid,
        party = {
            leader = leader,
            companion_one = companion_one,
            companion_two = companion_two,
        },
    }
end

-- ROM 0xC716/C726: revive the leader, floor both meters, dismiss active
-- companions, clear their physical meters, and apply the wallet penalty.
local w = P.resolve_party_wipe(recovery_facts(
    1000, false,
    { lifecycle = { active = true, downed = true, dead = false },
      meters = { physical = 0, mental = 3 } },
    { lifecycle = { active = true, downed = false, dead = false } },
    { lifecycle = { active = false, downed = false, dead = false } }))
check(w.leader.lifecycle.active and not w.leader.lifecycle.downed and
      not w.leader.lifecycle.dead, "leader lifecycle is restored")
check(w.leader.meters.physical == 6 and w.leader.meters.mental == 8,
      "leader meter floors 6/8")
check(w.companions.companion_one.dismissed and
      not w.companions.companion_two.dismissed,
      "active companion is dismissed")
check(w.wallet.nuyen == 875, "nuyen minus 1/8")
check(w.wake_destination == "little_chiba",
      "unpaid motel bill selects Little Chiba")
check(w.companions.companion_one.physical_meter == 0 and
      w.companions.companion_two.physical_meter == 0,
      "0xC716 clears both companion physical meters")
local cap = P.resolve_party_wipe(recovery_facts(
    100000, true,
    { lifecycle = { active = true, downed = true, dead = true },
      meters = { physical = 9, mental = 10 } },
    { lifecycle = { active = false, downed = false, dead = false } },
    { lifecycle = { active = false, downed = false, dead = false } }))
check(cap.wallet.nuyen == 95000, "penalty capped at 5000")
check(cap.leader.meters.physical == 9 and cap.leader.meters.mental == 10,
      "meters above floor kept")
check(cap.wake_destination == "docwagon",
      "paid motel bill selects DocWagon")

-- 0x391A repeats the source runner walk after the wipe recovery [C]. C++
-- passes the effective player-record flags, with dismissed slots inactive.
local refresh_from_second = P.refresh_runner_after_wipe_flat(
    2, false, 0, E.RunnerStatus.ACTIVE,
    1, E.RunnerStatus.DEAD, 2, 0)
check(refresh_from_second[1] == 0 and
      refresh_from_second[2] & E.RunnerStatus.SELECTION_MARKER ~= 0 and
      refresh_from_second[3] & E.RunnerStatus.SELECTION_MARKER == 0 and
      refresh_from_second[4] & E.RunnerStatus.SELECTION_MARKER == 0,
      "0x391A wraps from the type-2 record to the revived leader")
check(refresh_from_second[5] == 1 and
      refresh_from_second[6] == E.MenuSound.MOVE,
      "0x391A reports the changed active runner and event 0x25")
local refresh_leader_only = P.refresh_runner_after_wipe_flat(
    0, false, 0, E.RunnerStatus.ACTIVE,
    1, E.RunnerStatus.DEAD, 2, E.RunnerStatus.DOWN)
check(refresh_leader_only[1] == 0 and refresh_leader_only[5] == 0 and
      refresh_leader_only[2] & E.RunnerStatus.SELECTION_MARKER ~= 0,
      "0x391A retains the only eligible runner after a complete scan")
local refresh_override = P.refresh_runner_after_wipe_flat(
    0, true, 0, E.RunnerStatus.ACTIVE,
    1, E.RunnerStatus.ACTIVE | E.RunnerStatus.DEAD,
    2, E.RunnerStatus.DOWN)
check(refresh_override[1] == 1,
      "0x391A FFF09D override accepts an active suppressed runner")

-- 0xC64C's FFFBED/FFFC1C run-failure gates [C].
local employee_failure = P.run_failure_flat(
    E.RunType.EMPLOYEE_RECRUITMENT, E.RunContextFlag.ACTIVE)
check(employee_failure ~= nil and
      employee_failure.event_id ==
          "party_wipe_employee_recruitment_failure" and
      employee_failure.clear_high_effects and
      employee_failure.clear_player_spawn_request and
      employee_failure.clear_world_activity_request,
      "type 5 active run shows extraction failure at 0xF3D5C")
check(P.run_failure_flat(
          E.RunType.EMPLOYEE_RECRUITMENT,
          E.RunContextFlag.ACTIVE |
              E.RunContextFlag.EMPLOYEE_FAILURE_GUARD) == nil,
      "type 5 world-activity scene-reset bit suppresses failure")

local bodyguard_failure = P.run_failure_flat(
    E.RunType.BODYGUARD_ESCORT, E.RunContextFlag.ACTIVE)
check(bodyguard_failure ~= nil and
      bodyguard_failure.event_id == "party_wipe_bodyguard_failure" and
      bodyguard_failure.clear_high_effects and
      bodyguard_failure.clear_player_spawn_request and
      not bodyguard_failure.clear_world_activity_request,
      "type 1 active run shows bodyguard failure at 0xF359E")

-- The C++ field host passes its typed FFFC3C mirror. 0x1DED8 clears that
-- longword whenever selected, including when it was already clear [C].
local prior_game = rawget(_G, "game")
_G.game = { runs = { registry = RunRegistry }, state = {} }
RunRegistry.active_offer = { source_run_type = E.RunType.BODYGUARD_ESCORT }
local sweep_pending = P.prepare_run_failure_flat(false, true)
check(sweep_pending[2] == 1 and sweep_pending[5] == 1 and
      sweep_pending[6] == 1,
      "0x1DED8 requests FFFC3C clear after receiving a pending marker")
RunRegistry.active_offer = { source_run_type = E.RunType.BODYGUARD_ESCORT }
local sweep_clear = P.prepare_run_failure_flat(false, false)
check(sweep_clear[2] == 1 and sweep_clear[5] == 1 and
      sweep_clear[6] == 0,
      "0x1DED8 still requests FFFC3C clear when the marker is already clear")
_G.game = prior_game

local delivery_failure = P.run_failure_flat(
    E.RunType.PACKAGE_DELIVERY, E.RunContextFlag.ACTIVE)
check(delivery_failure ~= nil and
      delivery_failure.event_id == "party_wipe_package_failure" and
      delivery_failure.clear_run_objective and
      not delivery_failure.clear_high_effects and
      not delivery_failure.clear_player_spawn_request and
      delivery_failure.run_type == E.RunType.PACKAGE_DELIVERY,
      "type 2 active run shows package failure and clears FFFC22 bit 9")

local recovery_failure = P.run_failure_flat(
    E.RunType.PACKAGE_RECOVERY, E.RunContextFlag.ACTIVE)
check(recovery_failure ~= nil and
      recovery_failure.event_id == "party_wipe_package_failure" and
      recovery_failure.clear_run_objective and
      not recovery_failure.clear_player_spawn_request,
      "type 4 active run takes the shared type 2 failure path")

check(P.run_failure_flat(E.RunType.BODYGUARD_ESCORT, 0) == nil,
      "type 1 without FFFC1C bit 0 does not fail a run")
check(P.run_failure_flat(E.RunType.MATRIX_OBJECTIVE,
                         E.RunContextFlag.ACTIVE) == nil,
      "type 6 has no C64C run-failure branch")
check(P.run_failure_flat(E.RunType.NONE, 0) == nil,
      "no active run skips run-failure presentation")

-- Registry active offers are the Lua model of FFFBED and FFFC1C [H mapping].
RunRegistry.active_offer = {
    source_run_type = E.RunType.EMPLOYEE_RECRUITMENT,
}
local run_state = RunRegistry.party_wipe_failure_state(false)
check(run_state.run_type == E.RunType.EMPLOYEE_RECRUITMENT and
      run_state.flags == E.RunContextFlag.ACTIVE,
      "accepted source offer publishes active FFFC1C state")
run_state = RunRegistry.party_wipe_failure_state(true)
check(run_state.flags ==
          (E.RunContextFlag.ACTIVE |
           E.RunContextFlag.EMPLOYEE_FAILURE_GUARD),
      "FFE18F scene reset publishes FFFC1C bit 7")
check(P.run_failure_flat(run_state.run_type, run_state.flags) == nil,
      "typed scene-reset state suppresses the active type-5 wipe page")
RunRegistry.clear_active_offer_after_party_wipe()
check(RunRegistry.active_offer == nil,
      "0x1E68C clears the active Lua run on failure")

-- 0xC7D2 chooses the two 0x2A32/0x2A44 paths. 0x2B78 aligns their
-- continuation coordinates to 0x80-unit placement boundaries [C].
local docwagon_destination = P.wake_destination(true)
local docwagon_arrival = E.PartyWipeArrival[docwagon_destination]
check(docwagon_destination == E.PartyWipeDestination.DOCWAGON and
      docwagon_arrival.scene == 0 and docwagon_arrival.x == 0x6200 and
      docwagon_arrival.y == 0x2D00,
      "paid motel flag selects DocWagon scene 0 arrival")
local little_chiba_destination = P.wake_destination(false)
local little_chiba_arrival = E.PartyWipeArrival[little_chiba_destination]
check(little_chiba_destination == E.PartyWipeDestination.LITTLE_CHIBA and
      little_chiba_arrival.scene == 2 and little_chiba_arrival.x == 0x6680 and
      little_chiba_arrival.y == 0x2300,
      "unpaid motel flag selects Little Chiba scene 2 arrival")

print(string.format("party_down: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
