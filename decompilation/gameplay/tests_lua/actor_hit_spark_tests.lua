-- ROM 0xB278..0xB29E: dynamic-actor melee requests the shared 0xAC4E child only
-- when 0x55C76 changes either defender byte (+0x4C/+0x9C) [C].
local E = require("enums")
game = {
    actors = {},
    modules = {
        combat = {
            resolve_reaction = function(_, target)
                return {
                    target_id = target.id,
                    changed = game.hit_spark_test_meter_changed == true,
                }
            end,
        },
        items = { catalog = {} },
        magic = {},
    },
    hit_spark_test_meter_changed = false,
}
require("actor.init")
local R = require("actor.registry")

local function fixture(meter_changed)
    game.hit_spark_test_meter_changed = meter_changed
    local state = {
        entity_kind = E.SpriteType.CORP_GUARD,
        action_delay = -1,
        movement_current_heading = 0,
        target_motion_effect_active = false,
    }
    local context = {
        dynamic_world_encounter_actor = true,
        entity_kind = E.SpriteType.CORP_GUARD,
        phase = "behavior",
        initial_heading = 0,
        target_heading_audio_alternate = false,
    }
    local target = {
        id = "party.runner.0",
        heading = 0,
        world_x = 4660,
        world_y = 2748,
    }
    R.begin_dynamic_target_action(state, context, target)
    return state, target
end

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local changed_state, target = fixture(true)
check(changed_state.combat_meter_update ~= nil and
      changed_state.combat_meter_update.changed == true and
      changed_state.hit_spark_spawn_request ~= nil and
      changed_state.hit_spark_spawn_request.target_id == target.id and
      changed_state.hit_spark_spawn_request.world_x == target.world_x and
      changed_state.hit_spark_spawn_request.world_y == target.world_y,
      "changed actor-side melee requests the shared spark at its target")

local unchanged_state = fixture(false)
check(unchanged_state.combat_meter_update ~= nil and
      unchanged_state.combat_meter_update.changed == false and
      unchanged_state.hit_spark_spawn_request == nil,
      "unchanged actor-side melee does not request a spark")

print(string.format("actor_hit_spark_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
