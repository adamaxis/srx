-- Field barrier gameplay placement and animation choices.
-- The native adapter translates the game's source heading and equipped item
-- into a compass direction and barrier strength before calling this module.
local E = require("enums")
local Bearing = require("bearing")

local FieldBarrier = {}

local function snap_to_cell_center(position)
    return math.floor(position / E.Grid.CELL_SIZE) * E.Grid.CELL_SIZE +
        E.Grid.CELL_CENTER_OFFSET
end

local function direction_from_bearing(bearing_degrees)
    local sector = Bearing.octant(bearing_degrees, false)
    return sector ~= nil and E.FieldBarrier.DIRECTIONS[sector + 1] or nil
end

function FieldBarrier.place(request)
    if type(request) ~= "table" then
        return nil
    end
    local position = request.map_position
    local bearing_degrees = request.bearing_degrees
    local strength = request.strength
    if type(position) ~= "table" or type(position.x) ~= "number" or
       type(position.y) ~= "number" then
        return nil
    end
    if strength ~= "standard" and strength ~= "reinforced" then
        return nil
    end

    local direction = direction_from_bearing(bearing_degrees)
    local center_offset = direction and
        E.FieldBarrier.CENTER_OFFSET_BY_DIRECTION[direction]
    local footprint = direction and
        E.FieldBarrier.FOOTPRINT_BY_DIRECTION[direction]
    if center_offset == nil or footprint == nil then
        return nil
    end

    -- [C] ROM 0x4A18/0x4A2A places the wall on the nearest cell center after
    -- applying the direction offset. Cell size and center are expressed here
    -- as map geometry; source word wrapping is applied by the native adapter.
    local center = {
        x = snap_to_cell_center(position.x + center_offset.x),
        y = snap_to_cell_center(position.y + center_offset.y),
    }

    local cells = {}
    for index, offset in ipairs(footprint) do
        cells[index] = { x = offset.x, y = offset.y }
    end
    return {
        direction = direction,
        center = center,
        footprint = cells,
        strength = strength,
    }
end

-- The native presenter owns palette, priority, and asset selection. Lua sends
-- only the frame and the two animated wall segments' semantic offsets.
function FieldBarrier.sprite_plan(request)
    if type(request) ~= "table" then
        return nil
    end
    local center = request.center
    local direction = request.direction
    local remaining_frames = request.remaining_frames
    local offsets = E.FieldBarrier.SPRITE_PIXEL_OFFSETS_BY_DIRECTION[direction]
    if type(center) ~= "table" or type(center.x) ~= "number" or
       type(center.y) ~= "number" or offsets == nil or
       type(remaining_frames) ~= "number" or remaining_frames < 0 then
        return nil
    end

    local first_variant = remaining_frames % E.FieldBarrier.SPRITE_VARIANT_COUNT
    local second_variant = (first_variant + 1) %
        E.FieldBarrier.SPRITE_VARIANT_COUNT
    local function segment(variant)
        local offset = offsets[variant + 1]
        return {
            variant = variant,
            offset = { x = offset.x, y = offset.y },
        }
    end

    return {
        frame = remaining_frames % E.FieldBarrier.SPRITE_FRAME_COUNT,
        center = { x = center.x, y = center.y },
        segments = { segment(first_variant), segment(second_variant) },
    }
end

return FieldBarrier
