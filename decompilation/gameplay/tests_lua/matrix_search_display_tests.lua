-- ROM 0xE1E6/0xE204 System Search display: C19B0 "----.---.----" placeholder,
-- one cell rewritten d6+1 times per round (d6 = rand(11)+10), 2 VBlanks per
-- digit, separators skipped after rounds d7 = 7 / 4, sound 0x36 per round.
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Search = require("menu.screens.matrix_search")

local draws = {}
local sounds = 0
local next_value = 0
local ctx = {
    game = { source_random_bound = function(bound)
        draws[#draws + 1] = bound
        local v = next_value % bound
        next_value = next_value + 7
        return v
    end },
    redraw = function() end,
    sound = function(id)
        assert(id == E.SoundId.MATRIX_SYSTEM_SEARCH, "only sound 0x36 is cued")
        sounds = sounds + 1
    end,
}

local st = Search.new({})
Search.enter(st, ctx)
local frames, result = 0, nil
local seen_codes = {}
local digits_per_round = {}
while not result do
    seen_codes[#seen_codes + 1] = st.code
    assert(#st.code == 13, "code row stays 13 cells: " .. st.code)
    assert(st.code:sub(5, 5) == "." and st.code:sub(9, 9) == ".",
           "separators stay fixed: " .. st.code)
    result = Search.input(st, {}, ctx)
    frames = frames + 1
    assert(frames < 2000, "search must terminate")
end
assert(sounds == 11, "sound 0x36 once per round, got " .. sounds)
assert(result.screen == E.MenuScreen.MATRIX_ENTRY, "routes to matrix entry")

-- Draw order must be: rand(11) then (n) x rand(10) per round, then rand(14).
local i, total_digits, rounds = 1, 0, 0
while draws[i] == 11 do
    rounds = rounds + 1
    i = i + 1
    local n = 0
    while draws[i] == 10 do n = n + 1; i = i + 1 end
    assert(n >= 11 and n <= 21, "d6+1 digits per round (11..21), got " .. n)
    total_digits = total_digits + n
end
assert(rounds == 11 and draws[i] == 14 and draws[i + 1] == nil,
       "11 rounds then one 14-way system selection")
assert(frames == total_digits * 2, "two VBlanks per displayed digit")

-- Placeholders fill left to right; the final row has no '-' left.
assert(seen_codes[1]:match("^%d%-%-%-%.%-%-%-%.%-%-%-%-$"),
       "first frame: one digit then placeholders: " .. seen_codes[1])
assert(st.code:match("^%d%d%d%d%.%d%d%d%.%d%d%d%d$"),
       "final code fills all 11 cells: " .. st.code)
for k = 2, #seen_codes do
    local filled = select(2, seen_codes[k]:gsub("%d", ""))
    local prev = select(2, seen_codes[k - 1]:gsub("%d", ""))
    assert(filled >= prev, "row never shrinks")
end

print("matrix_search_display_tests: 7/7 passed")
