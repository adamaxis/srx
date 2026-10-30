-- Native geometry for movement vectors. Bearings are degrees clockwise from
-- north; Lua state never carries the ROM's fixed-width turn representation.
local EntityMotion = {}
local Bearing = require("bearing")
local kDirectionsPerTurn = 128
local kProjectileStepDistance = 96

local function vector_at_motion_resolution(bearing_degrees)
    local normalized = Bearing.normalize(bearing_degrees)
    if normalized == nil then
        return nil
    end
    -- [C] ROM 0x150E8 selects among 128 movement directions. Preserve that
    -- observable resolution while deriving the vector from a semantic angle.
    local direction_index = math.floor(
        normalized * kDirectionsPerTurn / Bearing.FULL_TURN_DEGREES)
    local quantized_degrees = direction_index * Bearing.FULL_TURN_DEGREES /
        kDirectionsPerTurn
    return Bearing.vector(quantized_degrees)
end

local function quantize_coordinate(unit_component, scale)
    -- Host positions land on a discrete subpixel grid. Snap each component
    -- down once after applying the vector so every caller uses one rule.
    return math.floor(unit_component * scale)
end

local function scaled_offsets(bearing_degrees, scale)
    local vector = vector_at_motion_resolution(bearing_degrees)
    if vector == nil then
        return nil
    end
    return {
        horizontal = quantize_coordinate(vector.x, scale),
        vertical = quantize_coordinate(vector.y, scale),
    }
end

-- Projectile movement uses map units throughout Lua. The native boundary
-- adapts the vector to the runtime's fractional coordinate storage.
function EntityMotion.projectile_step(bearing_degrees)
    local vector = vector_at_motion_resolution(bearing_degrees)
    if vector == nil then
        return nil
    end
    return {
        x = vector.x * kProjectileStepDistance,
        y = vector.y * kProjectileStepDistance,
    }
end

-- Formation offsets are a direction vector scaled to a map-unit radius.
function EntityMotion.offset_at_radius(bearing_degrees, radius)
    if type(radius) ~= "number" or radius % 1 ~= 0 or
       radius < 0 then
        return nil
    end
    return scaled_offsets(bearing_degrees, radius)
end

return EntityMotion
