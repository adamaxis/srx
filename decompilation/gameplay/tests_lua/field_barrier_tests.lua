local E = require("enums")
local B = require("field_barrier")

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local expected_bases = {
    [0] = 1888, [1] = 1953, [2] = 2082, [3] = 2145,
    [4] = 2208, [5] = 2142, [6] = 2077, [7] = 1950,
}
local expected_offsets = {
    [0] = { 0, 1, -1, 2, -2 },
    [1] = { 0, -1, -2, 64, 128 },
    [2] = { 0, -64, 64, -128, 128 },
    [3] = { 0, -1, -2, -64, -128 },
    [4] = { 0, 1, -1, 2, -2 },
    [5] = { 0, 1, 2, -64, -128 },
    [6] = { 0, -64, 64, -128, 128 },
    [7] = { 0, 1, 2, 64, 128 },
}

for octant = 0, 7 do
    local placement = B.place_flat(
        0x4000, 0x4000, octant << E.FieldBarrier.HEADING_OCTANT_SHIFT,
                                   E.SpellActionCode.SUPER_BARRIER)
    local cells_match = placement[1] == octant
    for index = 1, E.FieldBarrier.CELL_COUNT do
        cells_match = cells_match and
            placement[index + 3] == expected_bases[octant] +
                expected_offsets[octant][index]
    end
    check(cells_match and
          placement[9] == E.FieldBarrier.CELL_CODE_SUPER_BARRIER,
          string.format("0x49FA/0x4BB0 octant %d Super Barrier cells and code",
                        octant))

    local ordinary = B.place_flat(
        0x4000, 0x4000, octant << E.FieldBarrier.HEADING_OCTANT_SHIFT,
                                  E.SpellActionCode.BARRIER)
    local same_cells = ordinary[1] == octant
    for index = 1, E.FieldBarrier.CELL_COUNT do
        same_cells = same_cells and
            ordinary[index + 3] == placement[index + 3]
    end
    check(same_cells and
          ordinary[9] == E.FieldBarrier.CELL_CODE_BARRIER,
          string.format("0x4A74 octant %d Barrier retains cells and selects 0x7E",
                        octant))
end

-- Hand-computed wall examples use the source centre cells for caster (0x4000,0x4000):
-- N base 1888 gives a horizontal run; E base 2082 gives a vertical run.
local wall_examples = {
    { 0, { 1888, 1889, 1887, 1890, 1886 }, "horizontal N" },
    { 2, { 2082, 2018, 2146, 1954, 2210 }, "vertical E" },
    { 1, { 1953, 1952, 1951, 2017, 2081 }, "NE L" },
    { 3, { 2145, 2144, 2143, 2081, 2017 }, "SE L" },
    { 5, { 2142, 2143, 2144, 2078, 2014 }, "SW L" },
    { 7, { 1950, 1951, 1952, 2014, 2078 }, "NW L" },
}
for _, example in ipairs(wall_examples) do
    local octant, expected = example[1], example[2]
    local placement = B.place_flat(
        0x4000, 0x4000,
        octant << E.FieldBarrier.HEADING_OCTANT_SHIFT,
                                   E.SpellActionCode.SUPER_BARRIER)
    local matches = true
    for index = 1, E.FieldBarrier.CELL_COUNT do
        matches = matches and placement[index + 3] == expected[index]
    end
    check(matches, "hand-computed 0x4A34/0x4BD0 " .. example[3])
end

local rounded = B.place_flat(0x4001, 0x4080, 0,
                             E.SpellActionCode.BARRIER)
check(rounded[2] == 0x4100 and rounded[3] == 0x3D00 and
      rounded[4] == 1952 and rounded[9] == E.FieldBarrier.CELL_CODE_BARRIER,
      "0x4A18 snaps both wrapped word axes down to the cell centre")

local wrapped = B.place_flat(0, 0, 0,
                             E.SpellActionCode.SUPER_BARRIER)
check(wrapped[2] == 0x0100 and wrapped[3] == 0xFB00 and
      wrapped[4] == 8000,
      "0x4A18 applies u16 wrap before centre snapping")

print(string.format("field_barrier: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
