-- party_down.lua: ROM 0xB330/0xABB6 runner down and 0xC64C/0xC716 wipe.
local P = require("party_down")
local E = require("enums")
local RunRegistry = require("run_registry")

local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

local function party_facts(target_role, companion_one, companion_two,
                           target_meters)
    return {
        target_role = target_role,
        target_meters = target_meters or {
            [E.RunnerMeter.PHYSICAL] = 0,
            [E.RunnerMeter.MENTAL] = 0,
        },
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
local meters_not_depleted = P.enter_runner_down_state(party_facts(
    "leader", { active = false, dead = false },
    { active = false, dead = false }, {
        [E.RunnerMeter.PHYSICAL] = 4,
        [E.RunnerMeter.MENTAL] = 6,
    }))
check(not meters_not_depleted.lifecycle.downed and
      not meters_not_depleted.recovery.party_wipe_imminent,
      "positive physical and mental meters do not down a runner")
local physical_zero = P.enter_runner_down_state(party_facts(
    "leader", { active = false, dead = false },
    { active = false, dead = false }, {
        [E.RunnerMeter.PHYSICAL] = 0,
        [E.RunnerMeter.MENTAL] = 6,
    }))
check(physical_zero.lifecycle.downed and
      physical_zero.recovery.party_wipe_imminent,
      "zero physical meter downs the runner and starts a lone wipe")
local mental_zero = P.enter_runner_down_state(party_facts(
    "leader", { active = false, dead = false },
    { active = false, dead = false }, {
        [E.RunnerMeter.PHYSICAL] = 4,
        [E.RunnerMeter.MENTAL] = 0,
    }))
check(mental_zero.lifecycle.downed and
      mental_zero.recovery.party_wipe_imminent,
      "zero mental meter downs the runner and starts a lone wipe")
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

-- ROM 0x391A walks semantic party roles, wraps after the roster-end member,
-- and skips dead/downed members unless the named override is enabled [C].
local function member(active, downed, dead, selection_pending, ends_roster)
    return {
        lifecycle = { active = active, downed = downed, dead = dead },
        selection_pending = selection_pending,
        ends_roster = ends_roster,
    }
end
local refresh_from_second = P.refresh_runner_selection({
    previous_role = "companion_two",
    suppression_override = false,
    party = {
        leader = member(true, false, false, false, false),
        companion_one = member(false, false, true, false, false),
        companion_two = member(false, false, false, true, true),
    },
})
check(refresh_from_second.active_role == "leader" and
      refresh_from_second.selection_pending.leader and
      not refresh_from_second.selection_pending.companion_two,
      "0x391A wraps from the roster end to the revived leader")
check(refresh_from_second.active_role_changed,
      "0x391A reports an active-role change")
local refresh_leader_only = P.refresh_runner_selection({
    previous_role = "leader",
    suppression_override = false,
    party = {
        leader = member(true, false, false, false, false),
        companion_one = member(false, false, true, false, false),
        companion_two = member(false, true, false, false, true),
    },
})
check(refresh_leader_only.active_role == "leader" and
      not refresh_leader_only.active_role_changed and
      refresh_leader_only.selection_pending.leader,
      "0x391A retains the only eligible runner after a full scan")
local refresh_override = P.refresh_runner_selection({
    previous_role = "leader",
    suppression_override = true,
    party = {
        leader = member(true, false, false, false, false),
        companion_one = member(true, true, false, false, false),
        companion_two = member(false, false, false, false, true),
    },
})
check(refresh_override.active_role == "companion_one" and
      refresh_override.active_role_changed,
      "0x391A override accepts an active downed companion")

-- 0xC64C's FFFBED/FFFC1C run-failure gates [C].
local employee_failure = P.select_run_failure({
    run_id = "employee_recruitment",
    active = true,
    employee_failure_guard = false,
})
check(employee_failure ~= nil and
      employee_failure.event_id ==
          "party_wipe_employee_recruitment_failure" and
      employee_failure.clear_high_effects and
      employee_failure.clear_player_spawn_request and
      employee_failure.clear_world_activity_request,
      "type 5 active run shows extraction failure at 0xF3D5C")
check(P.select_run_failure({
          run_id = "employee_recruitment",
          active = true,
          employee_failure_guard = true,
      }) == nil,
      "type 5 world-activity scene-reset bit suppresses failure")

local bodyguard_failure = P.select_run_failure({
    run_id = "bodyguard_escort",
    active = true,
    employee_failure_guard = false,
})
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
RunRegistry.active_offer = { id = "bodyguard_escort" }
local sweep_pending = P.prepare_run_failure({
    world_activity_scene_reset = false,
})
check(sweep_pending.clear_high_effects and
      sweep_pending.clear_player_spawn_request and
      not sweep_pending.clear_world_activity_request,
      "0x1DED8 requests high-effect and spawn-template clears")
RunRegistry.active_offer = { id = "bodyguard_escort" }
local sweep_clear = P.prepare_run_failure({
    world_activity_scene_reset = false,
})
check(sweep_clear.clear_high_effects and
      sweep_clear.clear_player_spawn_request and
      not sweep_clear.clear_world_activity_request,
      "0x1DED8 requests the same clears without pending host state")
_G.game = prior_game

local delivery_failure = P.select_run_failure({
    run_id = "package_delivery",
    active = true,
    employee_failure_guard = false,
})
check(delivery_failure ~= nil and
      delivery_failure.event_id == "party_wipe_package_failure" and
      delivery_failure.clear_run_objective and
      not delivery_failure.clear_high_effects and
      not delivery_failure.clear_player_spawn_request and
      delivery_failure.run_id == "package_delivery",
      "type 2 active run shows package failure and clears FFFC22 bit 9")

local recovery_failure = P.select_run_failure({
    run_id = "package_recovery",
    active = true,
    employee_failure_guard = false,
})
check(recovery_failure ~= nil and
      recovery_failure.event_id == "party_wipe_package_failure" and
      recovery_failure.clear_run_objective and
      not recovery_failure.clear_player_spawn_request,
      "type 4 active run takes the shared type 2 failure path")

check(P.select_run_failure({
          run_id = "bodyguard_escort",
          active = false,
          employee_failure_guard = false,
      }) == nil,
      "inactive bodyguard run does not show a failure page")
check(P.select_run_failure({
          run_id = "matrix_objective",
          active = true,
          employee_failure_guard = false,
      }) == nil,
      "type 6 has no C64C run-failure branch")
check(P.select_run_failure({
          run_id = "none",
          active = false,
          employee_failure_guard = false,
      }) == nil,
      "no active run skips run-failure presentation")

-- Registry active offers are the semantic run state used by recovery. Their
-- source type and context encodings stay at the import/host boundary [C].
RunRegistry.active_offer = {
    id = "employee_recruitment",
}
local run_state = RunRegistry.party_wipe_failure_state(false)
check(run_state.run_id == "employee_recruitment" and
      run_state.active and not run_state.employee_failure_guard,
      "accepted employee run publishes active gameplay state")
run_state = RunRegistry.party_wipe_failure_state(true)
check(run_state.employee_failure_guard,
      "world-activity scene reset suppresses employee failure")
check(P.select_run_failure(run_state) == nil,
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

-- ROM 0xC778..0xC79C chooses a distinct speaker profile for each wake scene.
local saved_game = game
local wake_pages = {}
game = {
    state = { motel_bill_paid = true },
    ui = { say = function(...)
        wake_pages[#wake_pages + 1] = { ... }
    end },
}
local wake_event = P.present_wake()
check(wake_event == "party_wipe_docwagon" and #wake_pages == 2 and
      wake_pages[1][1] == "party_wipe_docwagon" and
      wake_pages[1][2] == "party_wipe_docwagon_narrator" and
      wake_pages[1][3] == "field.world_event" and
      wake_pages[2][1] == "party_wipe_docwagon_dropoff" and
      wake_pages[2][2] == "party_wipe_docwagon_narrator",
      "paid wake pages carry the DocWagon portrait profile")

game.state.motel_bill_paid = false
wake_pages = {}
wake_event = P.present_wake()
check(wake_event == "party_wipe_little_chiba" and #wake_pages == 1 and
      wake_pages[1][1] == "party_wipe_little_chiba" and
      wake_pages[1][2] == "party_wipe_little_chiba_narrator" and
      wake_pages[1][3] == "field.world_event",
      "unpaid wake page carries the Little Chiba portrait profile")
game = saved_game

print(string.format("party_down: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
