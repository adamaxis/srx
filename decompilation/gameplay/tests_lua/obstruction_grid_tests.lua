local Grid = require("support.rom_obstruction_grid_reference")

local function read_source_rectangles(path)
    local file = assert(io.open(path, "rb"), "could not open source rectangle records: " .. path)
    local bytes = file:read("*a")
    file:close()
    assert(#bytes % 10 == 0, "terrain rectangle source size is not a whole record count")

    local function read_word(offset)
        local high, low = bytes:byte(offset, offset + 1)
        return (high << 8) | low
    end

    local rectangles = {}
    for offset = 1, #bytes, 10 do
        rectangles[#rectangles + 1] = {
            left = read_word(offset),
            right = read_word(offset + 2),
            top = read_word(offset + 4),
            bottom = read_word(offset + 6),
            value = read_word(offset + 8) & Grid.BYTE_MASK,
        }
    end
    return rectangles
end

local function read_binary(path)
    local file = assert(io.open(path, "rb"), "could not open GPGX grid fixture: " .. path)
    local bytes = file:read("*a")
    file:close()
    assert(#bytes == Grid.BYTE_COUNT,
        "GPGX obstruction grid fixture must contain exactly 4096 bytes")
    return bytes
end

local function encode_grid(grid)
    local bytes = {}
    for index = 1, Grid.BYTE_COUNT do
        local value = assert(grid[index], "obstruction grid has a missing cell")
        bytes[index] = string.char(value)
    end
    return table.concat(bytes)
end

local source_path =
    "decompilation/assets/records/field/scenes/scene-02/terrain-collision-rectangles.bin"
local rectangles = read_source_rectangles(source_path)
-- ROM 0x3232 selects scene 2's source records; this raw file has 101 10-byte
-- records, matching the rasterizer input walked at 0x0EE34..0x0EE64 [C].
assert(#rectangles == 101, "scene-2 source rectangle record count changed")
local grid = Grid.build(rectangles)
local actual = encode_grid(grid)

local captures = {
    {
        path = "decompilation/gameplay/tests_lua/fixtures/obstruction_grid/ff1700_hw2_f64.bin",
        label = "hw2.state continuation frame 64, scene 2",
    },
    {
        path = "decompilation/gameplay/tests_lua/fixtures/obstruction_grid/ff1700_10080_f264.bin",
        label = "playtest46_gpgx_hit_10080.state continuation frame 264, scene 2",
    },
}

for _, capture in ipairs(captures) do
    local expected = read_binary(capture.path)
    if actual ~= expected then
        local first_difference = 1
        while first_difference <= #expected and
            actual:byte(first_difference) == expected:byte(first_difference) do
            first_difference = first_difference + 1
        end
        error(string.format(
            "%s differs at FF1700+0x%03X: built=0x%02X GPGX=0x%02X",
            capture.label,
            first_difference - 1,
            actual:byte(first_difference) or 0,
            expected:byte(first_difference) or 0))
    end
end

assert(Grid.set_scene(rectangles), "reference grid installation failed")
assert(encode_grid(Grid.current()) == actual,
    "test reference grid differs from the builder result")
-- GPGX Step 1 recorded FF2035=0x7F at frame 64; from FF1700 this is offset
-- 0x0935. Keep the source-observed obstruction byte intact [C].
assert(Grid.value_at(0x0935) == Grid.EFFECT_BLOCKING,
    "GPGX frame 64 obstruction cell FF1700+0x0935 must remain 0x7F")

print("obstruction_grid_tests: 2 GPGX FF1700 snapshots match byte-for-byte")
