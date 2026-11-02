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

check(E.OpeningOrcSequence.HIT_PALETTE_STAGE == 5 and
      E.OpeningOrcSequence.FIRST_FADE_STAGE == 6 and
      E.OpeningOrcSequence.CENTER_CLEAR_STAGE == 12,
      "typed orc stages match 0x5A124, 0x5A16E, and 0x5A1BE")

local expected = {
    [0] = E.OpeningOrcPaletteMode.SOURCE,
    [1] = E.OpeningOrcPaletteMode.SOURCE,
    [2] = E.OpeningOrcPaletteMode.SOURCE,
    [3] = E.OpeningOrcPaletteMode.SOURCE,
    [4] = E.OpeningOrcPaletteMode.SOURCE,
    [5] = E.OpeningOrcPaletteMode.HIT_WHITE,
    [6] = E.OpeningOrcPaletteMode.FADE,
    [7] = E.OpeningOrcPaletteMode.FADE,
    [8] = E.OpeningOrcPaletteMode.FADE,
    [9] = E.OpeningOrcPaletteMode.FADE,
    [10] = E.OpeningOrcPaletteMode.FADE,
    [11] = E.OpeningOrcPaletteMode.FADE,
    [12] = E.OpeningOrcPaletteMode.CLEARED,
}

for stage = 0, 12 do
    local policy = Startup.opening_orc_palette_mode(stage)
    check(type(policy) == "table" and #policy == 1 and
          policy[1] == expected[stage],
          "palette policy at 0x5A0B2 sequence stage " .. stage)
end

check(Startup.opening_orc_palette_mode(13)[1] ==
      E.OpeningOrcPaletteMode.CLEARED,
      "the cleared state remains after the terminal sequence stage")

print(string.format("startup_opening_orc_timeline: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
