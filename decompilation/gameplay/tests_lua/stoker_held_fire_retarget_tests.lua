-- GPGX Stoker exit pedestrian route: held fire clears its dead target and
-- releases the C042 pose on the measured source frames.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({})
require("actor.init")
local R = require("actor.registry")
local ActorAI = require("actor.heartbeat")
local E = require("enums")
local pass, total = 0, 0
local function check(condition, message)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL: " .. message) end
end

local fixture_path = ROOT ..
    "/decompilation/gameplay/tests_lua/fixtures/gpgx_stoker_exit_pedestrians/held_fire_target_pose.tsv"
local file = assert(io.open(fixture_path, "r"))
local rows = {}
local headers
for line in file:lines() do
    if line ~= "" and line:sub(1, 1) ~= "#" then
        local values = {}
        for value in (line .. "\t"):gmatch("(.-)\t") do
            values[#values + 1] = value
        end
        if headers == nil then
            headers = values
        else
            local row = {}
            for index, key in ipairs(headers) do
                row[key] = values[index]
            end
            row.frame = assert(tonumber(row.frame))
            if row.target_slot == "none" then
                row.target_slot = nil
            else
                row.target_slot = assert(tonumber(row.target_slot))
            end
            for _, key in ipairs({ "target_flags", "target_meter",
                    "player_pose", "animation_clock", "action_gate", "pad",
                    "slot4_flags", "slot4_action_word", "slot5_flags",
                    "slot5_action_word" }) do
                row[key] = assert(tonumber(row[key]))
            end
            rows[row.frame] = row
        end
    end
end
file:close()

local function source_candidates(row)
    local function candidate(slot, flags, action_word)
        return {
            id = "gpgx_stoker_slot_" .. slot,
            actor_facts = {
                lifecycle = {
                    active = (flags & 0x01) ~= 0,
                    downed = (flags & 0x10) ~= 0,
                    dead = (flags & 0x40) ~= 0,
                },
                behavior = { escort = false },
                roles = { pedestrian = true },
            },
            projected_y_nonzero = action_word ~= 0,
        }
    end
    return {
        candidate(3, row.target_flags, 0x00C8),
        candidate(4, row.slot4_flags, row.slot4_action_word),
        candidate(5, row.slot5_flags, row.slot5_action_word),
    }
end

local selected = {
    target_selection_processed = false,
    selected_world_actor_id = "gpgx_stoker_slot_3",
}
local initial = assert(rows[10263])
local selection_context = {
    phase = "behavior",
    target_selection_requested = false,
    target_selection_policy = "standard",
    target_selection_mode = 2,
    world_activity_active = true,
    target_selection_current_actor_id = "gpgx_stoker_slot_3",
    world_target_selection_candidates = source_candidates(initial),
}
R.advance_world_target_selection(selected, selection_context)
check(not selected.target_selection_processed and
      selected.selected_world_actor_id == "gpgx_stoker_slot_3" and
      initial.target_slot == 3 and initial.pad == 0x0040,
      "GPGX frame 10263 retains linked record slot 3 while A is held [C]")

for frame = 10264, 10279 do
    local still_alive = {
        target_flags = initial.target_flags,
        slot4_flags = initial.slot4_flags,
        slot4_action_word = initial.slot4_action_word,
        slot5_flags = initial.slot5_flags,
        slot5_action_word = initial.slot5_action_word,
    }
    selection_context.target_selection_requested = false
    selection_context.target_selection_current_actor_id =
        "gpgx_stoker_slot_3"
    selection_context.world_target_selection_candidates =
        source_candidates(still_alive)
    R.advance_world_target_selection(selected, selection_context)
end
check(not selected.target_selection_processed and
      selected.selected_world_actor_id == "gpgx_stoker_slot_3",
      "held fire keeps the live linked target through frame 10279 [C]")

local dead = assert(rows[10280])
local decision = ActorAI.select_world_service_target(
    E.WorldTargetSelectionPolicy.STANDARD, source_candidates(dead))
check(type(decision) == "table" and decision.selected_actor_id == "" and
      dead.target_slot == nil and
      dead.target_flags == 0x75 and dead.pad == 0x0040,
      "GPGX frame 10280 clears dead slot 3 and Lua's world-service callback finds no eligible next record [C]")

-- The recorded GPGX route has no second live hostile. Exercise the missing
-- candidate permutation separately against the source-ordered list contract:
-- ROM 0x1DC4C scans eligible actor slots after clearing the prior target.
-- These rows are synthetic policy inputs, not a second emulator capture.
local function make_eligible(candidate)
    candidate.actor_facts.lifecycle.active = true
    candidate.actor_facts.lifecycle.dead = false
    candidate.actor_facts.lifecycle.downed = false
    candidate.actor_facts.behavior.escort = false
    candidate.projected_y_nonzero = true
end

local next_hostiles = source_candidates(dead)
make_eligible(next_hostiles[2])
make_eligible(next_hostiles[3])
local second_hostile = ActorAI.select_world_service_target(
    E.WorldTargetSelectionPolicy.STANDARD, next_hostiles)
check(second_hostile.selected_actor_id == "gpgx_stoker_slot_4",
      "after slot 3 is dead, the first eligible source-ordered hostile in slot 4 is selected [C: 0x1DC4C]")

next_hostiles[2].actor_facts.lifecycle.dead = true
local third_hostile = ActorAI.select_world_service_target(
    E.WorldTargetSelectionPolicy.STANDARD, next_hostiles)
check(third_hostile.selected_actor_id == "gpgx_stoker_slot_5",
      "a dead slot 4 is skipped and the next eligible hostile in slot 5 is selected [C: 0x1DC4C]")

next_hostiles[3].actor_facts.lifecycle.downed = true
local no_eligible_hostile = ActorAI.select_world_service_target(
    E.WorldTargetSelectionPolicy.STANDARD, next_hostiles)
check(no_eligible_hostile.selected_actor_id == "",
      "a downed slot 5 is excluded after the other hostile dies [C: 0x1DC4C]")

local pose = {
    animation_id = "idle",
    animation_frame = 0,
    animation_elapsed_updates = 0,
    direction = E.ActorFacing.EAST,
    player_weapon_animation_start_pending = true,
}
local function advance_held_pose()
    R.advance_player_animation(pose, {
        animation_mode = E.ActorAnimationMode.ATTACK,
        animation_direction_override = E.ActorFacing.EAST,
    })
end
advance_held_pose() -- frame 10263
for _ = 1, 29 do advance_held_pose() end -- through frame 10292
local completed = assert(rows[10292])
check(pose.animation_id == "attack" and
      pose.player_weapon_animation_complete == true and
      completed.player_pose == 0x804C and
      completed.frame - initial.frame == 29,
      "GPGX frame 10292 is the finite C042 completion marker [C]")
advance_held_pose() -- frame 10293
local neutral = assert(rows[10293])
check(pose.animation_id == "idle" and
      neutral.player_pose == 0x000C and neutral.pad == 0x0040 and
      neutral.frame - initial.frame == 30,
      "held A releases the C042 pose to neutral at frame 10293 [C]")
for _ = 1, 21 do advance_held_pose() end -- through frame 10314
pose.player_weapon_animation_start_pending = true
advance_held_pose() -- repeated held-fire action at frame 10315
local repeated = assert(rows[10315])
check(pose.animation_id == "attack" and
      pose.player_weapon_animation_complete ~= true and
      repeated.player_pose == 0x004C and repeated.pad == 0x0040 and
      repeated.frame - initial.frame == 52,
      "the next held-fire action restarts C042 at frame 10315 [C]")

print(string.format("stoker held-fire retarget tests: %d/%d passed",
                    pass, total))
if pass ~= total then os.exit(1) end
