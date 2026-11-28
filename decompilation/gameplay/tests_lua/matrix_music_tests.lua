-- ROM Matrix music: 0x1745E stop-all (0x1752C) then play_music(MATRIX 0x03)
-- at 0x17656 on Matrix world entry; deck from the world: stop-all 0x1B150,
-- play_music(DECK_MENU 0x04) 0x1B15A; back: stop-all 0x1B19A, MATRIX 0x1B1A4.
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local MatrixSession = require("menu.matrix_session")
local Node = require("menu.screens.matrix_node_action")

assert(E.MusicId.MATRIX == 0x03 and E.MusicId.DECK_MENU == 0x04)

local log = {}
local game = setmetatable({
    source_random_bound = function() return 9 end,
    matrix_alert_random_bound = function() return 9 end,
}, { __index = function() return function() return nil end end })
local ctx = {
    game = game,
    redraw = function() end,
    sound = function() end,
    music = function(id) log[#log + 1] = "music:" .. id end,
    stop_music = function() log[#log + 1] = "stop" end,
}

local started, state = MatrixSession.begin(0, nil, nil, nil, false, nil, nil)
assert(started and state, "Matrix session starts for system 0")
local st = Node.new({ system_id = 0, matrix_state = state, traversal_view = true })
local ok, err = pcall(Node.enter, st, ctx)
assert(log[1] == "stop" and log[2] == "music:3",
       "world entry stops the field song then plays MATRIX: " .. table.concat(log, ","))

log = {}
local res = Node.input(st, E.Pad.START, ctx)
-- Start may be consumed by the entry sequence; drive until the deck opens.
local guard = 0
while not (res and res.screen == E.MenuScreen.CYBERDECK) and guard < 2000 do
    res = Node.input(st, guard % 2 == 0 and E.Pad.START or 0, ctx)
    guard = guard + 1
end
assert(res and res.screen == E.MenuScreen.CYBERDECK, "Start opens the deck")
assert(log[#log - 1] == "stop" and log[#log] == "music:4",
       "deck plays DECK_MENU after stop-all: " .. table.concat(log, ","))

log = {}
Node.child_returned(st, nil, { matrix_resumed = true }, ctx)
assert(log[1] == "stop" and log[2] == "music:3",
       "deck return restores MATRIX: " .. table.concat(log, ","))

-- Non-world (traversal_view = false) node screens do not touch music.
log = {}
local st2 = Node.new({ system_id = 0, matrix_state = state })
pcall(Node.enter, st2, ctx)
assert(#log == 0, "non-world node screen keeps current music")

print("matrix_music_tests: 4/4 passed")
