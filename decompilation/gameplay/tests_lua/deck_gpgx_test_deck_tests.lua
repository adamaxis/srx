-- Compare the production menu route with the same-state headless GPGX capture
-- made after ROM Debug-menu Test Deck. The expected title and values come
-- from fixtures/gpgx_test_deck.lua and its adjacent independent PNG captures.
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Route = require("menu")
local Fixture = dofile("decompilation/gameplay/tests_lua/fixtures/gpgx_test_deck.lua")
local P = E.Pad
local checks, failures = 0, 0

local function check(condition, message)
    checks = checks + 1
    if not condition then
        failures = failures + 1
        io.write("FAIL ", message, "\n")
    end
end

local fixture_directory = "decompilation/gameplay/tests_lua/fixtures/gpgx_test_deck/"
for _, name in ipairs(Fixture.captures) do
    local path = fixture_directory .. name
    local file = io.open(path, "rb")
    local header = file and file:read(24) or nil
    if file then file:close() end
    local valid = header ~= nil and header:sub(1, 8) == "\137PNG\r\n\026\n"
    local width, height
    if valid then width, height = string.unpack(">I4I4", header, 17) end
    check(valid and width == 320 and height == 224,
          "GPGX frame exists at 320x224: " .. name)
end

local function zero_indexed(values)
    local out = {}
    for i, value in ipairs(values) do out[i - 1] = value end
    return out
end

local ratings, files, active, persona, slots = {}, {}, {}, {}, {}
for i = 0, 11 do ratings[i] = Fixture.cyberdeck.program_rating end
for i = 0, 4 do
    files[i] = { id = -1, size = 0 }
    active[i] = Fixture.cyberdeck.active_slots[i + 1]
    slots[i] = { status = 6, timer = 0, phase = 1 }
end
for i = 0, 3 do persona[i] = Fixture.deck_stats.values[i + 7] end

local record = {
    mpcp = Fixture.deck_stats.values[1],
    hardening = Fixture.deck_stats.values[2],
    memory = Fixture.deck_stats.values[3],
    storage = Fixture.deck_stats.values[4],
    load_io = Fixture.deck_stats.values[5],
    response = Fixture.deck_stats.values[6],
    model = -1,
    persona = persona,
    ratings = ratings,
    active = active,
    files = files,
}

local host = {
    play_sound = function() end,
    redraw = function() end,
    set_window = function() end,
    set_screen = function() end,
    menu_closed = function() end,
}
local game = {
    menu_b_policy = E.MenuBPolicy.ROM,
    deck_enter = function() return E.MatrixDeckMode.BALANCED end,
    deck_leave = function() end,
    set_deck_mode = function() end,
    deck_record = function() return record end,
    deck_slots = function() return slots end,
    deck_commit = function() end,
    matrix_online = function() return false end,
    matrix_world_state = function() return {} end,
}

local route = Route.new(host, game)
route:open(E.MenuScreen.CYBERDECK)
local deck_state = route:top().state
check(route:top().screen == E.MenuScreen.CYBERDECK and
      deck_state.col == Fixture.cyberdeck.cursor[1] and
      deck_state.row == Fixture.cyberdeck.cursor[2],
      "GPGX Cyberdeck starts with Attack selected")
local installed = 0
for i = 0, 11 do
    if deck_state.program_ratings[i] == Fixture.cyberdeck.program_rating then
        installed = installed + 1
    end
end
check(installed == Fixture.cyberdeck.program_count and
      #deck_state.labels == 16 and
      deck_state.program_detail_values[1] ==
          tostring(Fixture.cyberdeck.selected_attack[1]) and
      deck_state.program_detail_values[2] ==
          tostring(Fixture.cyberdeck.selected_attack[2]) and
      deck_state.program_detail_values[3] ==
          tostring(Fixture.cyberdeck.selected_attack[3]),
      "GPGX Test Deck grid has twelve programs and Attack 4 detail 32/200/1")
for i = 0, 4 do
    check(deck_state.active_programs[i] == Fixture.cyberdeck.active_slots[i + 1],
          "GPGX Test Deck leaves active slot " .. tostring(i) .. " empty")
end

for _ = 1, 3 do route:frame(P.DOWN) end
deck_state = route:top().state
check(deck_state.col == Fixture.cyberinfo.cyberdeck_cursor[1] and
      deck_state.row == Fixture.cyberinfo.cyberdeck_cursor[2],
      "GPGX CyberInfo grid selection is row 3, column 0")
route:frame(P.A)
local info_state = route:top().state
check(route:top().screen == E.MenuScreen.DECK_CYBERINFO and
      info_state.title == Fixture.cyberinfo.title and
      info_state.labels[1] == Fixture.cyberinfo.labels[1] and
      info_state.labels[2] == Fixture.cyberinfo.labels[2] and
      info_state.row == Fixture.cyberinfo.row,
      "GPGX CyberInfo title, choices, and initial cursor match")
route:frame(P.A)
local stats = route:top().state.matrix_deck_stats
local values_match = stats ~= nil and
    stats.manufacturer == Fixture.deck_stats.manufacturer and
    stats.model == Fixture.deck_stats.model and
    #stats.values == #Fixture.deck_stats.values
if values_match then
    for i, value in ipairs(Fixture.deck_stats.values) do
        if stats.values[i] ~= value then values_match = false end
    end
    for i, label in ipairs(Fixture.deck_stats.labels) do
        if stats.labels[i] ~= label then values_match = false end
    end
end
check(values_match,
      "GPGX Deck Stats maker/model, labels, and ten values match")

io.write("deck_gpgx_test_deck_tests: ", checks - failures, "/", checks,
         " passed\n")
os.exit(failures == 0 and 0 or 1)
