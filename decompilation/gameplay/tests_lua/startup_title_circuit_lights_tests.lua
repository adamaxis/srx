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

-- US ROM 0x5D8BA leaves FFDF90/FFDF92 for 0x2634. The headless ROM trace
-- enters the title light loop at phase 6/countdown 2; 0x2634 advances at
-- title frames 256, 261, 266, ... after the completion-state-2 gate.
check(E.TitleCircuitLight.PRE_TITLE_PHASE_COUNT == 8 and
      E.TitleCircuitLight.PHASE_COUNT == 7 and
      E.TitleCircuitLight.ACTIVE_COMPLETION_STATE == 2 and
      E.TitleCircuitLight.TITLE_ENTRY_COUNTDOWN == 2 and
      E.TitleCircuitLight.COUNTDOWN_RELOAD == 4,
      "typed title-light values match the two source tables")

local seeded = Startup.reset_title_circuit_lights(6)
check(seeded[1] == 6 and seeded[2] == 2,
      "title setup carries the pre-title shimmer phase and countdown")

local expected_phase = function(frame)
    if frame < 256 then
        return 6
    end
    return math.floor((frame - 256) / 5) % E.TitleCircuitLight.PHASE_COUNT
end

local expected_countdown = function(frame)
    if frame < 254 then
        return 2
    elseif frame < 256 then
        return 255 - frame
    end
    return E.TitleCircuitLight.COUNTDOWN_RELOAD - ((frame - 256) % 5)
end

for frame = 198, 318 do
    local completion_state = frame >= 254
        and E.TitleCircuitLight.ACTIVE_COMPLETION_STATE
        or 1
    local state = Startup.advance_title_circuit_lights(completion_state)
    check(state[1] == expected_phase(frame) and
          state[2] == expected_countdown(frame),
          "phase/countdown at title update " .. frame)
end

print(string.format("startup_title_circuit_lights: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
