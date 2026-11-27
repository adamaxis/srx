package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local Events = require("field_events")

local checks = 0
local function check(value, message)
    assert(value, message)
    checks = checks + 1
end

local function terrain_context(raw_byte, world_effect_control_bit)
    return {
        collision_kind = "terrain_marker",
        correctional_facility = false,
        resource_site = false,
        special_activity_context = false,
        field_mode_exit_requested = false,
        interaction_gate_active = false,
        activity_sequence_active = false,
        terrain_event_suppressed = false,
        raw_terrain_collision_byte = raw_byte,
        world_effect_control_bit = world_effect_control_bit,
    }
end

local suppressed = Events.resolve_collision(terrain_context(
    E.TerrainCollisionByte.SAFE_MARK_NO_AWARD, false))
check(suppressed ~= nil and
      suppressed.action == "mark_projected_terrain" and
      not suppressed.projected_terrain_event_published,
      "ROM 0xB918's 0xDA safe marker does not publish FFE178")

local eligible = Events.resolve_collision(terrain_context(0x9A, false))
check(eligible ~= nil and
      eligible.action == "mark_projected_terrain" and
      eligible.projected_terrain_event_published,
      "ROM 0xB918 publishes a non-0xDA marker when FFFC1C bit 7 is clear")

local world_effect_suppressed = Events.resolve_collision(
    terrain_context(0x9A, true))
check(world_effect_suppressed ~= nil and
      world_effect_suppressed.action == "mark_projected_terrain" and
      not world_effect_suppressed.projected_terrain_event_published,
      "ROM 0xB920's FFFC1C bit 7 suppresses FFE178")
check(E.WorldEffectControlBit.SAFE_MARK_NO_AWARD == 7,
      "Renraku marker policy names FFFC1C bit 7")

print(string.format("field_events_tests: %d checks passed", checks))
