-- ROM 0xB1DC/0xB29E: the player allocates a hit spark only after an in-range
-- 0x55C76 reaction changes the linked defender's +0x4C or +0x9C meter.
local E = require("enums")
local Combat = require("combat")
game = {
    actors = {},
    modules = {
        combat = Combat,
        items = { catalog = {} },
        magic = {},
    },
}
local ActorAI = require("actor.init")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function combatant(record_type)
    local item_codes, item_phases = {}, {}
    for slot = 1, 8 do
        item_codes[slot] = 0
        item_phases[slot] = 0
    end
    return {
        record_type = record_type,
        source_flags = E.ActorStatus.ACTIVE,
        physical_meter = 10,
        mental_meter = 10,
        force = 0,
        posture = 0,
        command_attribute_78 = 0,
        command_attribute_7b = 0,
        command_attribute_7c = 0,
        command_attribute_77 = 0,
        command_attribute_79 = 4,
        command_attribute_85 = 1,
        race_or_state = 4,
        class_code = 0,
        armor_index = 0,
        equipped_action_code = 1,
        interaction_flags = 128,
        cyberware_flags = 0,
        action_profile_modifier_flags = 0,
        item_codes = item_codes,
        item_phases = item_phases,
    }
end

local attacker = combatant(E.ActorPool.PARTY)
local random_calls = 0
local function successful_reaction_roll(bound)
    if bound ~= 6 then return nil end
    random_calls = random_calls + 1
    return random_calls % 2 == 1 and 5 or 0
end

local target = {
    id = "field_actor_slot_3",
    source_record_slot = 3,
    world_x = 4660,
    world_y = 2748,
    heading = 16384,
    distance = 256,
    service_flags = E.ActorStatus.ACTIVE,
    world_activity_actor = true,
    has_action = true,
    combatant = combatant(E.ActorPool.OTHERS),
}

local context = {
    actor_id = "field_player_runner",
    profile_id = "field_runner_k03_none00",
    entity_kind = E.SpriteType.FIELD_RUNNER,
    action_kind = "none",
    action_code = E.ItemId.NONE,
    entry_delay = 0,
    initial_heading = 0,
    spawn = true,
    phase = "behavior",
    player_melee_available = true,
    combat_action_available = true,
    combat_confirm_held = true,
    combat_action_ready = true,
    combat_input_blocked = false,
    combatant = attacker,
    ai_random_bound = successful_reaction_roll,
    ai_target_candidates = { target },
}

ActorAI.heartbeat(context)
context.spawn = false
local strike_hit = ActorAI.heartbeat(context)
check(strike_hit ~= nil and
      strike_hit.source_action_dispatch == "player_melee_hit" and
      strike_hit.combat_meter_update ~= nil and
      strike_hit.combat_meter_update.changed and
      strike_hit.hit_spark_spawn_request ~= nil and
      strike_hit.hit_spark_spawn_request.target_id == target.id and
      strike_hit.hit_spark_spawn_request.world_x == target.world_x and
      strike_hit.hit_spark_spawn_request.world_y == target.world_y,
      "changed in-range player hit requests a spark at the defender")

target.distance = E.HitSpark.RANGE_LIMIT
local strike_out_of_range = ActorAI.heartbeat(context)
check(strike_out_of_range ~= nil and
      strike_out_of_range.source_action_dispatch == "player_melee" and
      strike_out_of_range.hit_spark_spawn_request == nil,
      "out-of-range player swing does not request a spark")

target.distance = 256
target.service_flags = E.ActorStatus.ACTIVE | E.ActorStatus.DEAD
local strike_dead_target = ActorAI.heartbeat(context)
check(strike_dead_target ~= nil and
      strike_dead_target.source_action_dispatch == "player_melee" and
      strike_dead_target.hit_spark_spawn_request == nil,
      "dead linked record does not request a spark")

target.service_flags = E.ActorStatus.ACTIVE
context.ai_random_bound = function() return nil end
local strike_unchanged = ActorAI.heartbeat(context)
check(strike_unchanged ~= nil and
      strike_unchanged.source_action_dispatch == "player_melee_hit" and
      strike_unchanged.combat_meter_update == nil and
      strike_unchanged.hit_spark_spawn_request == nil,
      "in-range strike without a resolved meter change does not request a spark")

print(string.format("player_hit_spark_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
