-- Bearings are degrees clockwise from north, independent of the ROM's
-- 16-bit turn encoding. The native scripting boundary converts source values.
local Bearing = {}

Bearing.FULL_TURN_DEGREES = 360
Bearing.EIGHTH_TURN_DEGREES = 45

function Bearing.normalize(degrees)
    if type(degrees) ~= "number" or degrees ~= degrees or
       degrees == math.huge or degrees == -math.huge then
        return nil
    end
    local normalized = degrees % Bearing.FULL_TURN_DEGREES
    return normalized < 0 and normalized + Bearing.FULL_TURN_DEGREES or normalized
end

function Bearing.signed_difference(from_degrees, to_degrees)
    local from = Bearing.normalize(from_degrees)
    local to = Bearing.normalize(to_degrees)
    if from == nil or to == nil then
        return nil
    end
    local difference = (to - from) % Bearing.FULL_TURN_DEGREES
    if difference > 180 then
        difference = difference - Bearing.FULL_TURN_DEGREES
    end
    -- Keep the same deterministic clockwise choice for an exact half turn.
    if difference == -180 then
        return 180
    end
    return difference
end

function Bearing.approach(current_degrees, target_degrees, max_step_degrees)
    local current = Bearing.normalize(current_degrees)
    local target = Bearing.normalize(target_degrees)
    local difference = Bearing.signed_difference(current_degrees, target_degrees)
    if current == nil or target == nil or difference == nil or
       type(max_step_degrees) ~= "number" or max_step_degrees < 0 then
        return nil
    end
    if difference > max_step_degrees then
        return Bearing.normalize(current + max_step_degrees)
    end
    if difference < -max_step_degrees then
        return Bearing.normalize(current - max_step_degrees)
    end
    return target
end

function Bearing.octant(degrees, round_to_nearest)
    local normalized = Bearing.normalize(degrees)
    if normalized == nil then
        return nil
    end
    local sector = normalized / Bearing.EIGHTH_TURN_DEGREES
    if round_to_nearest then
        sector = math.floor(sector + 0.5)
    else
        sector = math.floor(sector)
    end
    return sector % 8
end

function Bearing.vector(degrees)
    local normalized = Bearing.normalize(degrees)
    if normalized == nil then
        return nil
    end
    local radians = normalized * math.pi / 180
    local x = math.sin(radians)
    local y = -math.cos(radians)
    if math.abs(x) < 1e-12 then x = 0 end
    if math.abs(y) < 1e-12 then y = 0 end
    return { x = x, y = y }
end

return Bearing
