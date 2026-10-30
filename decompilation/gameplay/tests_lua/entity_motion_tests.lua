-- Entity motion uses native vector geometry rather than the ROM's copied
-- 0x15E56 sample array. Cardinal orientation and scale are source-backed by
-- the 0x150E8 and 0x15DCC call paths [C].
local motion = require("entity_motion")
local pass, total = 0, 0

local function check(condition, label)
    total = total + 1
    if condition then
        pass = pass + 1
    else
        print("FAIL " .. label)
    end
end

local north = motion.heading_offsets(0x0000)
local east = motion.heading_offsets(0x4000)
local south = motion.heading_offsets(0x8000)
local west = motion.heading_offsets(0xC000)
check(north.horizontal == 0 and north.vertical == -0x6000,
      "north vector uses the 96-unit projectile step")
check(east.horizontal == 0x5FFF and east.vertical == 0,
      "east vector uses the 96-unit projectile step")
check(south.horizontal == 0 and south.vertical == 0x5FFF,
      "south vector uses the 96-unit projectile step")
check(west.horizontal == -0x6000 and west.vertical == 0,
      "west vector uses the 96-unit projectile step")

local diagonal = motion.heading_offsets(0x2000)
check(math.abs(math.abs(diagonal.horizontal) -
               math.abs(diagonal.vertical)) <= 1,
      "diagonal vector has equal horizontal and vertical components")

local linked_east = motion.scaled_heading_offsets(0x4000, 8, 4)
check(linked_east.horizontal == 0x3FFF and linked_east.vertical == 0,
      "linked effect offset scales a direction vector")
local formation_north = motion.offset_at_radius(0, 0xE0)
check(formation_north.horizontal == 0 and
      formation_north.vertical == -0xE0,
      "formation offset applies its map-unit radius")

check(motion.heading_offsets(0x10000) == nil,
      "out-of-range turn is rejected")

print(string.format("entity_motion: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
