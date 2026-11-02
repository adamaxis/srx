local E = require("enums")
local Startup = require("startup")

local pass, total = 0, 0
local function check(condition, message)
    total = total + 1
    if condition then
        pass = pass + 1
    else
        print("FAIL: " .. message)
    end
end

check(E.OpeningTailRotor.FIRST_PALETTE_ENTRY == 17 and
      E.OpeningTailRotor.PHASE_COUNT == 3 and
      E.OpeningTailRotor.ROTATE_INTERVAL == 8 and
      E.OpeningTailRotor.INITIAL_PHASE == 0,
      "typed tail-fan values match ROM 0x5B08E and palette entries 17..19")

local reset = Startup.reset_opening_tail_rotor()
check(reset[1] == E.OpeningTailRotor.INITIAL_PHASE,
      "airport resource install starts with the extracted palette phase")

for frame = 1, 24 do
    local phase = Startup.advance_opening_tail_rotor(frame)
    local expected = math.floor(frame / E.OpeningTailRotor.ROTATE_INTERVAL) %
        E.OpeningTailRotor.PHASE_COUNT
    check(phase[1] == expected,
          "palette phase at source frame " .. frame)
end

Startup.reset_opening_tail_rotor()
local unaligned = Startup.advance_opening_tail_rotor(31)
local boundary = Startup.advance_opening_tail_rotor(32)
check(unaligned[1] == 0 and boundary[1] == 1,
      "phase changes on the absolute source-frame multiple of eight")

print(string.format("startup_opening_tail_rotor_timeline: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
