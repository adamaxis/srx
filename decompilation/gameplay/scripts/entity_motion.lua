-- Native geometry for movement vectors.
-- ROM 0x15E56 is source evidence for orientation and scale. Its 128 stored
-- samples are not runtime data; Lua computes their circle positions with
-- trigonometry and applies the relevant game-world distance.
local EntityMotion = {}

local full_turn = 2 * math.pi
local turn_units = 0x10000
local direction_sample_count = 128
local turn_units_per_sample = turn_units / direction_sample_count
local north_angle = -math.pi / 2
local direction_scale = 0x7fff

local function unit_direction(heading)
    if type(heading) ~= "number" or heading % 1 ~= 0 or
       heading < 0 or heading > 0xffff then
        return nil
    end
    -- The source selects one of 128 evenly spaced orientations. Preserve that
    -- visible direction cadence while replacing the copied sample array with
    -- a mathematical vector.
    local sample = math.floor(heading / turn_units_per_sample)
    local angle = north_angle + sample * full_turn / direction_sample_count
    local horizontal = math.cos(angle)
    local vertical = math.sin(angle)
    if math.abs(horizontal) < 1e-12 then horizontal = 0 end
    if math.abs(vertical) < 1e-12 then vertical = 0 end
    return horizontal, vertical
end

local function truncate_product(unit_component, scale)
    local product = unit_component * scale
    local nearest_integer = math.floor(product + 0.5)
    if math.abs(product - nearest_integer) < 1e-9 then
        return nearest_integer
    end
    return math.floor(product)
end

local function scaled_offsets(heading, scale)
    local horizontal, vertical = unit_direction(heading)
    if horizontal == nil then
        return nil
    end
    return {
        horizontal = truncate_product(horizontal, scale),
        vertical = truncate_product(vertical, scale),
    }
end

-- Projectile movement uses a 12/16 direction scale in 8.8 world units.
-- The maximum component is 96 map units (ROM 0x150E8).
function EntityMotion.heading_offsets(heading)
    return scaled_offsets(heading, direction_scale * 12 / 16)
end

-- Retain call-site compatibility while expressing scaling as ordinary vector
-- magnitude, rather than as samples and shifts from the ROM's sine table.
function EntityMotion.scaled_heading_offsets(heading, multiplier, shift)
    if type(multiplier) ~= "number" or multiplier % 1 ~= 0 or
       type(shift) ~= "number" or shift % 1 ~= 0 or
       shift < 0 or shift > 30 then
        return nil
    end
    return scaled_offsets(
        heading, direction_scale * multiplier / (1 << shift))
end

-- Formation offsets are a direction vector scaled to a map-unit radius.
function EntityMotion.offset_at_radius(heading, radius)
    if type(radius) ~= "number" or radius % 1 ~= 0 or
       radius < 0 or radius > 0xffff then
        return nil
    end
    return scaled_offsets(heading, direction_scale * radius / 0x8000)
end

return EntityMotion
