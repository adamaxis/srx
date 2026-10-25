-- effect_spawn.lua: ROM 0x15646 thrown item record math. Vectors checked
-- against the converted C++ routine (512-case sweep identical, 2026-10-24).
local S = require("effect_spawn")
local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

check(S.window_open(0x003C) and not S.window_open(0x003D) and S.window_open(0x803C),
    "0x1564A window masks 0x7FF, <= 0x3C")
check(S.direction_index(0x0000) == 0 and S.direction_index(0x4000) == 2 and
      S.direction_index(0x8000) == 4 and S.direction_index(0xC000) == 6 and
      S.direction_index(0x1000) == 1 and S.direction_index(0x0FFF) == 0,
    "0x156FA octant with half-octant rounding")

local parent = { x = 0x1000, y = 0x2000, heading = 0, f56 = 0x12, f57 = 0x0D, f87 = 0x44, f9d = 0x9D }
local r = S.spawn(parent, nil)
check(r.x == 0x1000 and r.y == 0x1F50, "0x15A4C up: (0, -0xB0)")
check(r.probe_x == 0x1040 and r.probe_y == 0x1F00, "0x15A6C up probe: (0x40, -0x100)")
check(r.marker == 0x22 and r.distance_steps == 0xB4 and r.movement_offset == -0x500 and
      r.lifetime == 0x13 and r.continuation == 0x15390 and r.vertical_velocity == 0xF400,
    "no-target defaults")
check(r.offset_x == 0 and r.offset_y == -0x2000, "no-target velocity *8>>5")

parent.heading = 0x4000
r = S.spawn(parent, nil)
check(r.x == 0x10B0 and r.y == 0x2000 and r.probe_x == 0x1180 and r.probe_y == 0x1FC0,
    "right: position (0xB0,0), probe (0x180,-0x40)")

parent.heading = 0
r = S.spawn(parent, { heading = 0x4000, distance = 0x800, marker = 0x77 })
check(r.heading == 0x4000 and r.marker == 0x77 and r.distance_steps == 0x20 and
      r.movement_offset == -0x200 and r.offset_x == 0x3FFF and r.offset_y == 0,
    "linked target *8>>4")
check(r.x == 0x1000 and r.y == 0x1F50, "linked spawn offset uses parent heading")
r = S.spawn(parent, { heading = 0x8000, distance = 0x2000, marker = 0 })
check(r.distance_steps == (0xBE0 // 0x80) * 2, "0x157CA distance cap 0xBE0")

print(string.format("effect_spawn: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
