local B = require("field_barrier")
local E = require("enums")

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local directions = E.FieldBarrier.DIRECTIONS
local function place(position, bearing_degrees, strength)
    return B.place({
        map_position = position,
        bearing_degrees = bearing_degrees,
        strength = strength,
    })
end
local function sprite_plan(center, direction, remaining_frames)
    return B.sprite_plan({
        center = center,
        direction = direction,
        remaining_frames = remaining_frames,
    })
end
local expected_bases = {
    north = 1888, northeast = 1953, east = 2082, southeast = 2145,
    south = 2208, southwest = 2142, west = 2077, northwest = 1950,
}
local expected_centers = {
    north = { x = 0x4100, y = 0x3B00 },
    northeast = { x = 0x4300, y = 0x3D00 },
    east = { x = 0x4500, y = 0x4100 },
    southeast = { x = 0x4300, y = 0x4300 },
    south = { x = 0x4100, y = 0x4500 },
    southwest = { x = 0x3D00, y = 0x4300 },
    west = { x = 0x3B00, y = 0x4100 },
    northwest = { x = 0x3D00, y = 0x3D00 },
}
local expected_footprints = {
    north = { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 2, 0 }, { -2, 0 } },
    northeast = { { 0, 0 }, { -1, 0 }, { -2, 0 }, { 0, 1 }, { 0, 2 } },
    east = { { 0, 0 }, { 0, -1 }, { 0, 1 }, { 0, -2 }, { 0, 2 } },
    southeast = { { 0, 0 }, { -1, 0 }, { -2, 0 }, { 0, -1 }, { 0, -2 } },
    south = { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 2, 0 }, { -2, 0 } },
    southwest = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, -1 }, { 0, -2 } },
    west = { { 0, 0 }, { 0, -1 }, { 0, 1 }, { 0, -2 }, { 0, 2 } },
    northwest = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 0, 2 } },
}

for index, direction in ipairs(directions) do
    local placement = place(
        { x = 0x4000, y = 0x4000 }, (index - 1) * 45, "reinforced")
    local footprint_matches = placement ~= nil and
        #placement.footprint == #expected_footprints[direction]
    for cell = 1, #expected_footprints[direction] do
        local actual = placement.footprint[cell]
        local expected = expected_footprints[direction][cell]
        footprint_matches = footprint_matches and
            actual.x == expected[1] and actual.y == expected[2]
    end
    local center = expected_centers[direction]
    check(placement ~= nil and placement.direction == direction and
          placement.center.x == center.x and
          placement.center.y == center.y and footprint_matches and
          placement.strength == "reinforced",
          "0x4AAC/0x4BB0 " .. direction .. " semantic placement")
    check(expected_bases[direction] ==
          (math.floor(placement.center.y / E.Grid.CELL_SIZE) * E.Grid.COLUMNS +
           math.floor(placement.center.x / E.Grid.CELL_SIZE)),
          "0x4A34 " .. direction .. " native adapter base-cell reference")

    local ordinary = place(
        { x = 0x4000, y = 0x4000 }, (index - 1) * 45, "standard")
    check(ordinary.direction == placement.direction and
          ordinary.center.x == placement.center.x and
          ordinary.center.y == placement.center.y and
          ordinary.strength == "standard",
          "0x4A74 " .. direction .. " strength leaves placement unchanged")
end

local rounded = place(
    { x = 0x4001, y = 0x4080 }, 0, "standard")
check(rounded.center.x == 0x4100 and rounded.center.y == 0x3D00 and
      rounded.footprint[1].x == 0 and rounded.footprint[1].y == 0,
      "0x4A18 snaps map position to the center of its cell")

-- Lua retains an ordinary map point here. The native adapter applies the
-- source word wrap before projecting it onto the collision grid [C: 0x4A18].
local edge = place({ x = 0, y = 0 }, 0, "reinforced")
check(edge.center.x == E.Grid.CELL_CENTER_OFFSET and edge.center.y == -1280 and
      edge.footprint[1].x == 0 and edge.footprint[1].y == 0,
      "edge placement stays semantic until the native map adapter")
check(place({ x = 0, y = 0 }, 0, "unknown") == nil,
      "placement rejects unknown barrier strength")
check(place({ x = 0, y = 0 }, math.huge, "standard") == nil,
      "placement rejects a non-finite bearing")

local center = { x = E.Grid.CELL_CENTER_OFFSET, y = E.Grid.CELL_CENTER_OFFSET }
local function check_sprite_plan(direction, remaining, label)
    local plan = sprite_plan(center, direction, remaining)
    local first = remaining % E.FieldBarrier.SPRITE_VARIANT_COUNT
    local second = (first + 1) % E.FieldBarrier.SPRITE_VARIANT_COUNT
    local expected_first = E.FieldBarrier.SPRITE_PIXEL_OFFSETS_BY_DIRECTION[direction][first + 1]
    local expected_second = E.FieldBarrier.SPRITE_PIXEL_OFFSETS_BY_DIRECTION[direction][second + 1]
    check(plan ~= nil and plan.frame == remaining % E.FieldBarrier.SPRITE_FRAME_COUNT and
          plan.center.x == center.x and plan.center.y == center.y and
          #plan.segments == E.FieldBarrier.SPRITE_SEGMENT_COUNT and
          plan.segments[1].variant == first and
          plan.segments[1].offset.x == expected_first.x and
          plan.segments[1].offset.y == expected_first.y and
          plan.segments[2].variant == second and
          plan.segments[2].offset.x == expected_second.x and
          plan.segments[2].offset.y == expected_second.y,
          label)
end

for lifetime = 0, E.FieldBarrier.SPRITE_VARIANT_COUNT - 1 do
    check_sprite_plan("north", lifetime,
        string.format("north segments advance at frame %d", lifetime))
end
for lifetime = 0, E.FieldBarrier.SPRITE_FRAME_COUNT - 1 do
    check_sprite_plan("northeast", lifetime,
        string.format("NE frame follows elapsed lifetime %d", lifetime))
end
for _, direction in ipairs(directions) do
    check_sprite_plan(direction, 3,
        direction .. " sprite offsets follow its placement")
end
check(sprite_plan(center, "north", -1) == nil,
      "sprite plan rejects negative remaining frames")

print(string.format("field_barrier: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
