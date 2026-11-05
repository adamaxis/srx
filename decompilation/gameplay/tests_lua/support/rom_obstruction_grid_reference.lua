-- Test-only reference for ROM FF1700 rasterization. The game runtime keeps
-- collision queries in the native field host; this module exists solely to
-- compare decoded scene rectangles with independent GPGX memory captures.
local Layout = {
    BYTE_COUNT = 0x1000,
    INDEX_MASK = 0x0FFF,
    COORDINATE_STEP = 0x20,
    ROW_COORDINATE_MASK = 0xFFE0,
    X_COLUMN_SHIFT = 5,
    Y_ROW_SHIFT = 1,
    BYTE_MASK = 0xFF,
    CLEAR = 0x00,
    EFFECT_BLOCKING = 0x7F,
}

local Reference = {
    BYTE_COUNT = Layout.BYTE_COUNT,
    BYTE_MASK = Layout.BYTE_MASK,
    EFFECT_BLOCKING = Layout.EFFECT_BLOCKING,
}
local current_grid

-- ROM 0x0EE10..0x0EE84 rasterizes the decoded source rectangles over
-- inclusive 0x20-unit bounds [C].
function Reference.build(rectangles)
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

function Reference.set_scene(rectangles)
    current_grid = Reference.build(rectangles)
    return true
end

function Reference.current()
    return current_grid
end

function Reference.value_at(index)
    assert(type(index) == "number" and index >= 0 and
        index < Layout.BYTE_COUNT and index % 1 == 0,
        "obstruction-grid index is outside FF1700")
    return current_grid and current_grid[index + 1] or Layout.CLEAR
end

return Reference
