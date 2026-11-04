local E = require("enums")

local Grid = {}
local current_grid

local Layout = E.ObstructionGrid

-- Build FF1700 from the scene's raw collision rectangles. The source list is
-- already decoded as named rectangle fields by the host; rasterization order
-- and inclusive 0x20-unit bounds follow ROM 0x0EE10..0x0EE84 [C].
function Grid.build(rectangles)
    assert(type(rectangles) == "table", "collision rectangles must be an array")

    local grid = {}
    for index = 1, Layout.BYTE_COUNT do
        grid[index] = Layout.CLEAR
    end

    for _, rectangle in ipairs(rectangles) do
        local left = assert(rectangle.left, "rectangle left is required")
        local right = assert(rectangle.right, "rectangle right is required")
        local top = assert(rectangle.top, "rectangle top is required")
        local bottom = assert(rectangle.bottom, "rectangle bottom is required")
        local value = assert(rectangle.value, "rectangle value is required")

        for y = top, bottom, Layout.COORDINATE_STEP do
            local row_offset =
                ((y & Layout.ROW_COORDINATE_MASK) << Layout.Y_ROW_SHIFT)
            for x = left, right, Layout.COORDINATE_STEP do
                local index = ((row_offset +
                    (x >> Layout.X_COLUMN_SHIFT)) & Layout.INDEX_MASK) + 1
                grid[index] = value & Layout.BYTE_MASK
            end
        end
    end

    return grid
end

-- The host supplies only the decoded source rectangles. Keep the active map
-- in Lua so subsequent field behavior can query the same authoritative grid.
function Grid.set_scene(rectangles)
    current_grid = Grid.build(rectangles)
    return true
end

function Grid.current()
    return current_grid
end

function Grid.value_at(index)
    assert(type(index) == "number" and index >= 0 and
        index < Layout.BYTE_COUNT and index % 1 == 0,
        "obstruction-grid index is outside FF1700")
    return current_grid and current_grid[index + 1] or Layout.CLEAR
end

return Grid
