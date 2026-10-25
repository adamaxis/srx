-- party_down.lua: ROM 0xB330/0xABB6 runner down and 0xC64C/0xC716 wipe.
local P = require("party_down")
local E = require("enums")
local RunRegistry = require("run_registry")

local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

-- Leader alone: down sets 0x10, kind 3 sound 0x2E, C0B2 selector, wipe 0x3C.
local r = P.runner_down_flat(0, 0x01, 0x00, 0x00, 3, 0x4000)
check(r[1] == 0x11, "leader DOWN flag")
check(r[2] == 0x2E, "B05E kind 3 sound")
check(r[3] == 0xE8 + 2 * 4, "C0B2 right selector")
check(r[4] == 0x3C, "lone leader starts FFE166")
-- 0xB35C rounding: 0x1000 rounds to the next direction, 0xF000 wraps to 0.
check(P.runner_down_flat(0, 1, 0, 0, 3, 0x1000)[3] == 0xEC, "heading round up")
check(P.runner_down_flat(0, 1, 0, 0, 3, 0xF000)[3] == 0xE8, "heading wrap")
-- A live hired runner blocks the wipe.
check(P.runner_down_flat(0, 0x01, 0x01, 0x00, 3, 0)[4] == 0, "live hired blocks wipe")
check(P.runner_down_flat(0, 0x01, 0x41, 0x00, 3, 0)[4] == 0x3C, "dead hired allows wipe")
-- A hired runner going down never wipes: the leader lacks bit6 [C].
check(P.runner_down_flat(1, 0x11, 0x01, 0x00, 3, 0)[4] == 0, "hired down no wipe")

-- Wipe: leader revived with floors, hired dismissed, nuyen penalty.
local w = P.resolve_wipe_flat(1000, 0, 0x11, 0, 3, 0x01, 5, 5, 0x00, 0, 0)
check(w[1] == 0x01, "leader flags cleared and active")
check(w[2] == 6 and w[3] == 8, "meter floors 6/8")
check(w[4] == 1 and w[5] == 0, "active hired dismissed")
check(w[6] == 875, "nuyen minus 1/8")
check(w[7] == P.WAKE.LITTLE_CHIBA, "bit0 clear wakes in Little Chiba")
check(w[8] == 0 and w[9] == 0,
      "0xC716 clears both hired +0x4C delay meters")
local cap = P.resolve_wipe_flat(100000, 1, 0x51, 9, 10, 0, 0, 0, 0, 0, 0)
check(cap[6] == 95000, "penalty capped at 5000")
check(cap[2] == 9 and cap[3] == 10, "meters above floor kept")
check(cap[7] == P.WAKE.DOCWAGON, "bit0 set calls DocWagon")

-- 0xC64C's FFFBED/FFFC1C run-failure gates [C].
local employee_failure = P.run_failure_flat(
    E.RunType.EMPLOYEE_RECRUITMENT, E.RunContextFlag.ACTIVE)
check(employee_failure ~= nil and
      employee_failure.event_id ==
          "party_wipe_employee_recruitment_failure" and
      employee_failure.clear_high_effects and
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
      not bodyguard_failure.clear_world_activity_request,
      "type 1 active run shows bodyguard failure at 0xF359E")

local delivery_failure = P.run_failure_flat(
    E.RunType.PACKAGE_DELIVERY, E.RunContextFlag.ACTIVE)
check(delivery_failure ~= nil and
      delivery_failure.event_id == "party_wipe_package_failure" and
      delivery_failure.clear_run_objective and
      not delivery_failure.clear_high_effects,
      "type 2 active run shows package failure and clears FFFC22 bit 9")

local recovery_failure = P.run_failure_flat(
    E.RunType.PACKAGE_RECOVERY, E.RunContextFlag.ACTIVE)
check(recovery_failure ~= nil and
      recovery_failure.event_id == "party_wipe_package_failure" and
      recovery_failure.clear_run_objective,
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
