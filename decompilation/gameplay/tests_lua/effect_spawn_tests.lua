-- Lua-owned 0x15646 record decisions checked against an independent GPGX RAM
-- capture. Source addresses and the capture setup are recorded in the fixture.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local E = require("enums")
local Spawn = require("effect_spawn")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function signed_word(value)
    value = value & 0xFFFF
    return value >= 0x8000 and value - 0x10000 or value
end

local fixture_file = io.open(ROOT ..
    "/decompilation/gameplay/tests_lua/fixtures/effect_spawn_gpgx.tsv", "r")
check(fixture_file ~= nil, "open the independent 0x15646 GPGX fixture")
local fixture
if fixture_file ~= nil then
    for line in fixture_file:lines() do
        if line ~= "" and line:sub(1, 1) ~= "#" and
           line:sub(1, 5) ~= "case\t" then
            local fields = {}
            for field in (line .. "\t"):gmatch("(.-)\t") do
                fields[#fields + 1] = field
            end
            fixture = {
                case = fields[1],
                item_code = tonumber(fields[2]),
                parent_x = tonumber(fields[3], 16),
                parent_y = tonumber(fields[4], 16),
                heading = tonumber(fields[5], 16),
                field_0x87 = tonumber(fields[6], 16),
                world_x = tonumber(fields[7], 16),
                world_y = tonumber(fields[8], 16),
                probe_x = tonumber(fields[9], 16),
                probe_y = tonumber(fields[10], 16),
                result_heading = tonumber(fields[11], 16),
                source_marker = tonumber(fields[12], 16),
                distance_steps = tonumber(fields[13], 16),
                movement_offset = tonumber(fields[14], 16),
                vertical_velocity = tonumber(fields[15], 16),
                offset_x = tonumber(fields[16], 16),
                offset_y = tonumber(fields[17], 16),
                lifetime = tonumber(fields[18], 16),
                behavior_type = tonumber(fields[19], 16),
                movement_scale = tonumber(fields[20], 16),
            }
            break
        end
    end
    fixture_file:close()
end
check(type(fixture) == "table" and fixture.case == "north_no_target",
    "fixture contains the measured north-facing no-target spawn")

local source_record
if fixture ~= nil then
    source_record = Spawn.spawn({
        world_x = fixture.parent_x,
        world_y = fixture.parent_y,
        heading = fixture.heading,
        field_0x56 = 0,
        field_0x57 = fixture.item_code,
        field_0x87 = fixture.field_0x87,
        field_0x9D = 0x9D,
    })
end
check(Spawn.is_effect_item(E.ItemId.FRAG_GRENADE) and
      Spawn.is_effect_item(E.ItemId.SCATTER_GRENADE) and
      Spawn.is_effect_item(E.ItemId.CONCUSSION_GRENADE) and
      not Spawn.is_effect_item(E.ItemId.ROOMSWEEPER) and
      not Spawn.is_effect_item(E.ItemId.MEDKIT),
    "effect-item codes use the E.ItemId range")
check(Spawn.window_open(0x003C) and not Spawn.window_open(0x003D) and
      Spawn.window_open(0x803C) and not Spawn.window_open(0x07FF),
    "0x1564A masks the animation counter and admits through 0x003C")

if fixture ~= nil and source_record ~= nil then
    check(source_record.world_x == fixture.world_x and
          source_record.world_y == fixture.world_y and
          source_record.directional_probe_x == fixture.probe_x and
          source_record.directional_probe_y == fixture.probe_y,
          "0x15646 north spawn position and probe match GPGX")
    check(source_record.heading == fixture.result_heading and
          source_record.source_marker == fixture.source_marker and
          source_record.effect_distance_steps == fixture.distance_steps and
          source_record.movement_offset == signed_word(fixture.movement_offset),
          "0x15646 no-target heading, marker, and travel defaults match GPGX")
    check(source_record.vertical_velocity == fixture.vertical_velocity and
          source_record.heading_offset_x == signed_word(fixture.offset_x) and
          source_record.heading_offset_y == signed_word(fixture.offset_y) and
          source_record.lifetime == fixture.lifetime and
          source_record.movement_scale == fixture.movement_scale and
          source_record.behavior_type == fixture.behavior_type and
          source_record.behavior == E.EntityEffectSpawn.BEHAVIOR,
          "0x15646 movement state matches GPGX and uses a semantic behavior")
end

-- ROM 0x15A4C/0x15A6C define all eight rounded spawn/probe directions [C].
local expected_position = {
    { 0x0000, -0x00B0 }, { 0x007C, -0x007C }, { 0x00B0, 0x0000 },
    { 0x007C, 0x007C }, { 0x0000, 0x00B0 }, { -0x007C, 0x007C },
    { -0x00B0, 0x0000 }, { -0x007C, -0x007C },
}
local expected_probe = {
    { 0x0040, -0x0100 }, { 0x0120, -0x00F0 }, { 0x0180, -0x0040 },
    { 0x00D0, 0x0050 }, { 0x0000, 0x0020 }, { -0x0180, 0x0040 },
    { -0x0180, -0x0040 }, { -0x0100, -0x00C0 },
}
for direction = 0, 7 do
    local heading = direction * 0x2000
    local record = Spawn.spawn({ world_x = 0x1000, world_y = 0x2000,
        heading = heading, field_0x87 = 0x44 })
    local position = expected_position[direction + 1]
    local probe = expected_probe[direction + 1]
    check(record ~= nil and
          record.world_x == ((0x1000 + position[1]) & 0xFFFF) and
          record.world_y == ((0x2000 + position[2]) & 0xFFFF) and
          record.directional_probe_x == ((0x1000 + probe[1]) & 0xFFFF) and
          record.directional_probe_y == ((0x2000 + probe[2]) & 0xFFFF),
          string.format("ROM direction %d retains position/probe offsets", direction))
end

local east = Spawn.spawn({ world_x = 0x1000, world_y = 0x2000,
    heading = 0x4000, field_0x87 = 0x44 })
check(east ~= nil and east.world_x == 0x10B0 and east.world_y == 0x2000 and
      east.directional_probe_x == 0x1180 and east.directional_probe_y == 0x1FC0,
      "0x156FA rounds the eastward spawn to ROM direction two")
check(east ~= nil and east.heading_offset_x == 0x1FFF and
      east.heading_offset_y == 0,
      "0x1575A uses the no-target 8/32 heading scale")

local source_wave_entry = Spawn.spawn({ world_x = 0x1000, world_y = 0x2000,
    heading = 0xDE00, field_0x87 = 0x44 })
check(source_wave_entry ~= nil and source_wave_entry.heading_offset_x == -6040,
      "ROM wave sample 0x15F34 keeps its measured signed value")

local linked = Spawn.spawn({ world_x = 0x1000, world_y = 0x2000,
    heading = 0, field_0x87 = 0x44 }, {
        heading = 0x4000, distance = 0x0800, source_marker = 0x77,
    })
check(linked ~= nil and linked.heading == 0x4000 and
      linked.source_marker == 0x77 and linked.effect_distance_steps == 0x20 and
      linked.movement_offset == -0x200,
      "0x15D3C east target yields the source heading and bounded travel step")
check(linked ~= nil and linked.heading_offset_x == 0x3FFF and
      linked.heading_offset_y == 0,
      "linked movement uses the target heading at source scale")

local capped = Spawn.spawn({ world_x = 0x1000, world_y = 0x2000,
    heading = 0, field_0x87 = 0x44 }, {
        heading = 0x4000, distance = 0x1000, source_marker = 0x77,
    })
check(capped ~= nil and capped.effect_distance_steps == 0x2E and
      capped.movement_offset == -0x02E0,
      "0x157CA caps linked travel at 0x0BE0 before source step conversion")

local wrapped = Spawn.spawn({ world_x = 0x0010, world_y = 0xFFF0,
    heading = 0, field_0x87 = 0x44 })
check(wrapped ~= nil and wrapped.world_x == 0x0010 and
      wrapped.world_y == 0xFF40 and wrapped.directional_probe_x == 0x0050 and
      wrapped.directional_probe_y == 0xFEF0,
      "spawn position and probe retain source word wrapping")

print(string.format("effect_spawn: %d/%d", passed, total))
os.exit(passed == total and 0 or 1)
