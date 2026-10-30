-- Entity motion uses native vector geometry rather than copied ROM samples.
-- The 0x150E8 path establishes 128 direction samples and a roughly 96-unit
-- projectile step; 0x15DCC uses the same direction concept for formations [C].
local motion = require("entity_motion")
local Bearing = require("bearing")
local pass, total = 0, 0

local function check(condition, label)
    total = total + 1
    if condition then
        pass = pass + 1
    else
        print("FAIL " .. label)
    end
end

local north = motion.projectile_step(0)
local east = motion.projectile_step(90)
local south = motion.projectile_step(180)
local west = motion.projectile_step(270)
check(north.x == 0 and north.y == -96,
      "north vector uses the 96-unit projectile step")
check(east.x == 96 and east.y == 0,
      "east vector uses the 96-unit projectile step")
check(south.x == 0 and south.y == 96,
      "south vector uses the 96-unit projectile step")
check(west.x == -96 and west.y == 0,
      "west vector uses the 96-unit projectile step")

local diagonal = motion.projectile_step(45)
check(math.abs(math.abs(diagonal.x) - math.abs(diagonal.y)) < 1e-9,
      "diagonal vector has equal horizontal and vertical components")

local formation_east = motion.offset_at_radius(90, 64)
check(formation_east.horizontal == 64 and formation_east.vertical == 0,
      "formation offset applies a map-unit radius")
local formation_north = motion.offset_at_radius(0, 224)
check(formation_north.horizontal == 0 and
      formation_north.vertical == -224,
      "formation offset applies its map-unit radius")

check(motion.projectile_step(math.huge) == nil,
      "non-finite bearings are rejected")

check(Bearing.normalize(-10) == 350 and
      Bearing.normalize(370) == 10,
      "bearings wrap into the clockwise degree range")
check(Bearing.signed_difference(350, 10) == 20 and
      Bearing.signed_difference(10, 350) == -20,
      "bearing differences take the shorter turn across north")
check(Bearing.signed_difference(0, 180) == 180,
      "an exact half turn chooses clockwise consistently")
check(Bearing.approach(350, 10, 11.25) == 1.25 and
      Bearing.approach(0, 11.25, 11.25) == 11.25,
      "actor turning advances by a semantic angular step")
check(Bearing.octant(22.5, true) == 1 and
      Bearing.octant(22.5, false) == 0,
      "animation facing rounds while walking direction truncates [C]")

print(string.format("entity_motion: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
