-- Lua owns the source-derived item-effect record decisions. Entity/effect
-- allocation, world timeouts, asset-meter updates, and target transforms stay
-- on the typed host boundary.
local E = require("enums")

local EffectSpawn = {}
local Spawn = E.EntityEffectSpawn

-- ROM 0x15A4C: effect position offsets by rounded octant [C].
local position_offsets = {
    { 0x0000, -0x00B0 }, { 0x007C, -0x007C }, { 0x00B0, 0x0000 }, { 0x007C, 0x007C },
    { 0x0000, 0x00B0 }, { -0x007C, 0x007C }, { -0x00B0, 0x0000 }, { -0x007C, -0x007C },
}
-- ROM 0x15A6C: directional terrain-probe offsets by rounded octant [C].
local probe_offsets = {
    { 0x0040, -0x0100 }, { 0x0120, -0x00F0 }, { 0x0180, -0x0040 }, { 0x00D0, 0x0050 },
    { 0x0000, 0x0020 }, { -0x0180, 0x0040 }, { -0x0180, -0x0040 }, { -0x0100, -0x00C0 },
}
-- ROM 0x15E56: 128 signed Q15 direction samples. Verified word-for-word
-- against the US ROM; this is source data, not values reconstructed from GPGX.
local heading_wave = {
    0x0000, 0x0647, 0x0C8B, 0x12C7, 0x18F8, 0x1F19, 0x2527, 0x2B1E,
    0x30FB, 0x36B9, 0x3C56, 0x41CD, 0x471C, 0x4C3F, 0x5133, 0x55F4,
    0x5A81, 0x5ED6, 0x62F1, 0x66CE, 0x6A6C, 0x6DC9, 0x70E1, 0x73B5,
    0x7640, 0x7883, 0x7A7C, 0x7C29, 0x7D89, 0x7E9C, 0x7F61, 0x7FD7,
    0x7FFF, 0x7FD7, 0x7F61, 0x7E9C, 0x7D89, 0x7C29, 0x7A7C, 0x7883,
    0x7640, 0x73B5, 0x70E1, 0x6DC9, 0x6A6C, 0x66CE, 0x62F1, 0x5ED6,
    0x5A81, 0x55F4, 0x5133, 0x4C3F, 0x471C, 0x41CD, 0x3C56, 0x36B9,
    0x30FB, 0x2B1E, 0x2527, 0x1F19, 0x18F8, 0x12C7, 0x0C8B, 0x0647,
    0x0000, 0xF9B9, 0xF375, 0xED39, 0xE708, 0xE0E7, 0xDAD9, 0xD4E2,
    0xCF05, 0xC947, 0xC3AA, 0xBE33, 0xB8E4, 0xB3C1, 0xAECD, 0xAA0C,
    0xA57F, 0xA12A, 0x9D0F, 0x9932, 0x9594, 0x9237, 0x8F1F, 0x8C4B,
    0x89C0, 0x877D, 0x8584, 0x83D7, 0x8277, 0x8164, 0x809F, 0x8029,
    0x8001, 0x8029, 0x809F, 0x8164, 0x8277, 0x83D7, 0x8584, 0x877D,
    0x89C0, 0x8C4B, 0x8F1F, 0x9237, 0x9594, 0x9932, 0x9D0F, 0xA1A2,
    0xA57F, 0xAA0C, 0xAECD, 0xB3C1, 0xB8E4, 0xBE33, 0xC3AA, 0xC947,
    0xCF05, 0xD4E2, 0xDAD9, 0xE0E7, 0xE708, 0xED39, 0xF375, 0xF9B9,
}

local function word(value)
    return value & 0xFFFF
end

local function signed_word(value)
    local result = word(value)
    return result >= 0x8000 and result - 0x10000 or result
end

local function add_word(value, delta)
    return word(value + delta)
end

local function direction_index(heading)
    -- ROM 0x156FA rotates the 16-bit turn by three, rounds a high half-octant,
    -- and then keeps the top three bits [C].
    local rotated = word((heading << 3) | (heading >> 13))
    if rotated & 0x8000 ~= 0 then
        rotated = word(rotated + 1)
    end
    return rotated & 0x0007
end

local function heading_offsets(heading, multiplier, shift)
    -- ROM 0x1575A/0x15806 use the wave index and signed multiply/ASR below.
    local index = (word(heading) >> 9) & 0x7F
    local horizontal_sample = signed_word(heading_wave[index + 1])
    local vertical_sample = signed_word(heading_wave[((index + 96) & 0x7F) + 1])
    local divisor = 1 << shift
    return {
        horizontal = math.floor(horizontal_sample * multiplier / divisor),
        vertical = math.floor(vertical_sample * multiplier / divisor),
    }
end

function EffectSpawn.is_effect_item(item_code)
    return type(item_code) == "number" and
        item_code >= Spawn.FIRST_ITEM_CODE and
        item_code <= Spawn.LAST_ITEM_CODE
end

function EffectSpawn.window_open(animation_counter)
    return type(animation_counter) == "number" and
        (word(animation_counter) & Spawn.ANIMATION_COUNTER_MASK) <=
            Spawn.ANIMATION_WINDOW_END
end

-- Builds the source-derived part of the effect record. The host fills the
-- allocated entity/effect handles and initializes generic record defaults.
-- `target`, when present, is the typed world point returned by the host's
-- source target-transform service.
function EffectSpawn.spawn(parent, target)
    if type(parent) ~= "table" or type(parent.world_x) ~= "number" or
       type(parent.world_y) ~= "number" or type(parent.heading) ~= "number" then
        return nil
    end

    local heading = word(parent.heading)
    local direction = direction_index(heading) + 1
    local position = position_offsets[direction]
    local probe = probe_offsets[direction]
    local record = {
        behavior = Spawn.BEHAVIOR,
        behavior_type = Spawn.BEHAVIOR_TYPE,
        world_x = add_word(parent.world_x, position[1]),
        world_y = add_word(parent.world_y, position[2]),
        directional_probe_x = add_word(parent.world_x, probe[1]),
        directional_probe_y = add_word(parent.world_y, probe[2]),
        heading = heading,
        movement_scale = Spawn.MOVEMENT_SCALE,
        animation_resource = 0,
        field_0x56 = parent.field_0x56 or parent.f56 or 0,
        field_0x57 = parent.field_0x57 or parent.f57 or 0,
        field_0x9D = parent.field_0x9D or parent.f9d or 0,
        lifetime = Spawn.LIFETIME_PASSES,
        movement_offset = Spawn.INITIAL_MOVEMENT_OFFSET,
        vertical_velocity = Spawn.VERTICAL_VELOCITY,
        effect_distance_steps = Spawn.DEFAULT_DISTANCE_STEPS,
        active = true,
    }

    if target == nil then
        record.source_marker = (parent.field_0x87 or parent.f87 or 0) >> 1
        local offsets = heading_offsets(heading, 0x0008, 5)
        record.heading_offset_x = offsets.horizontal
        record.heading_offset_y = offsets.vertical
        return record
    end

    if type(target) ~= "table" or type(target.heading) ~= "number" or
       type(target.distance) ~= "number" then
        return nil
    end
    record.source_marker = target.source_marker or target.marker or 0
    local target_heading = word(target.heading)
    local distance = math.min(word(target.distance), Spawn.MAX_TRAVEL_DISTANCE)
    local distance_steps = (distance // 0x0080) * 2
    record.heading = target_heading
    record.effect_distance_steps = word(distance_steps)
    record.movement_offset = signed_word(-(distance // 0x0080) * 0x20)
    local offsets = heading_offsets(target_heading, 0x0008, 4)
    record.heading_offset_x = offsets.horizontal
    record.heading_offset_y = offsets.vertical
    return record
end

-- Compatibility helper for the test/runtime boundary that passes flat values.
function EffectSpawn.spawn_flat(x, y, heading, f87, linked,
                                target_heading, target_distance, marker)
    local target
    if linked ~= nil and linked ~= 0 then
        target = {
            heading = target_heading,
            distance = target_distance,
            source_marker = marker,
        }
    end
    local record = EffectSpawn.spawn({
        world_x = x, world_y = y, heading = heading, field_0x87 = f87,
    }, target)
    if record == nil then return nil end
    return {
        record.world_x, record.world_y,
        record.directional_probe_x, record.directional_probe_y,
        record.heading, record.source_marker, record.effect_distance_steps,
        record.movement_offset, record.heading_offset_x, record.heading_offset_y,
    }
end

return EffectSpawn
