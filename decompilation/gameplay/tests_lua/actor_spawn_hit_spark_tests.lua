-- ROM 0x14C7C -> 0xB11C: the close, delayed Hell Hound path resolves
-- 0x55B16 and requests the 0xB146 child only after a defender byte changes.
local E = require("enums")
game = { actors = {}, modules = { combat = require("combat"), items = { catalog = {} }, magic = {} } }
require("actor.init")
local R = require("actor.registry")

local function combatant(record_type, physical_meter)
    local item_codes, item_phases = {}, {}
    for slot = 1, 8 do
        item_codes[slot] = E.ItemId.NONE
        item_phases[slot] = 0
    end
    return {
        record_type = record_type,
        class_code = 0,
        interaction_flags = 0,
        action_profile_modifier_flags = 0,
        source_flags = E.ActorStatus.ACTIVE,
        physical_meter = physical_meter,
        mental_meter = physical_meter,
        race_or_state = 0,
        cyberware_flags = 0,
        posture = 0,
        command_attribute_77 = 1,
        command_attribute_78 = 0,
        command_attribute_7b = 0,
        command_attribute_7c = 0,
        command_attribute_85 = 0,
        item_codes = item_codes,
        item_phases = item_phases,
    }
end

local function strike_fixture(target_meter)
    game.actors = {}
    local context = {
        entity_kind = E.SpriteType.HELL_HOUND,
        dynamic_world_encounter_actor = true,
        movement_allowed = true,
        action_profile_modifiers = {},
        ai_random_bound = function() return 0 end,
        combatant = combatant(E.ActorPool.OTHERS, 10),
        delay_primary_attribute = 0,
        delay_secondary_attribute = 0,
        world_effect_slot_occupancy = {},
    }
    local state = {
        entity_kind = E.SpriteType.HELL_HOUND,
        action_delay = -1,
        target_motion_effect_active = false,
        movement_current_heading = 0,
    }
    local target = {
        id = "party.runner.0",
        source_record_slot = 0,
        heading = E.AiTuning.QUARTER_TURN,
        distance = E.AiTuning.HELL_HOUND_CLOSE_RANGE - 1,
        world_x = 4660,
        world_y = 2748,
        combatant = combatant(E.ActorPool.PARTY - 1, target_meter),
    }
    R.resolve_source_target_motion(context, state, target)
    return state, target
end

local changed_state, target = strike_fixture(5)
assert(changed_state.combat_meter_update ~= nil and
       changed_state.combat_meter_update.changed == true and
       changed_state.hit_spark_spawn_request ~= nil and
       changed_state.hit_spark_spawn_request.target_id == target.id and
       changed_state.hit_spark_spawn_request.world_x == target.world_x and
       changed_state.hit_spark_spawn_request.world_y == target.world_y,
       "changed 0xB11C action requests the target-position child")

local unchanged_state = strike_fixture(0)
assert(unchanged_state.combat_meter_update ~= nil and
       unchanged_state.combat_meter_update.changed == false and
       unchanged_state.hit_spark_spawn_request == nil,
       "unchanged 0xB11C action does not request a child")

print("actor_spawn_hit_spark_tests: 2/2 passed")
