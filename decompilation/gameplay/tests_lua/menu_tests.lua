-- lua5.4 decompilation/gameplay/tests_lua/menu_tests.lua (run from the repository root)
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path
local E = require("enums"); local Route = require("menu")
local P, S, T, Pg = E.Pad, E.MenuScreen, E.MenuTransition, E.MenuPage
local fails, n = 0, 0
local function check(c, msg) n = n + 1; if not c then fails = fails + 1; print("FAIL " .. msg) end end
local function mk(o)
    o = o or {}
    local log = { sounds = {}, redraws = 0, blanks = {}, windows = {}, closed = false, legacy = {} }
    local g = { posture = o.posture or 2, gate = o.gate or 0, reselects = 0 }
    local host = {
        play_sound = function(id) log.sounds[#log.sounds + 1] = id end,
        redraw = function() log.redraws = log.redraws + 1 end,
        blank = function(m, n, line) log.blanks[#log.blanks + 1] = m; if n then log.hold = { n, line } end end,
        set_window = function(w) log.windows[#log.windows + 1] = w end,
        set_screen = function() end,
        menu_closed = function() log.closed = true end,
        legacy_open = function(s) log.legacy[#log.legacy + 1] = s end,
        legacy_advance = function(s, ev) if ev & P.START ~= 0 then return false, o.legacy_exit end; return true end,
    }
    local game = {
        scene_mode = function() return 0 end, in_combat = function() return false end,
        reselect_leader = function() g.reselects = g.reselects + 1 end,
        leader_posture = function() return g.posture end, set_leader_posture = function(v) g.posture = v end,
        leader_archetype = function() return g.gate end,
        party = function() return g.party end, active_slot = function() return 0 end,
        pocket_extra_row = function() return false end, secretary_effect_tick = function() end,
 spell_table = function() return { entries = {} } end,
        -- sr-refactor game API (record copies are committed back explicitly)
        commit_party = function(p) for k, v in pairs(p) do g.party[k] = v end end,
        set_runner_ammo = function(slot, clips, rounds) local r = g.party[slot]; r.clips, r.rounds = clips, rounds end,
        consume_debug_unlock = function() g.unlock = false end,
        set_contact_flags = function(m) g.contacts = m end, set_spell_table = function(t) g.spells = t end,
        source_random_bound = function(n) return 0 end, matrix_alert_random_bound = function(n) return 0 end,
    }
    g.party = { [0] = { status = 1, archetype = 3, magic = 0, species = 0, karma = 0, attributes = {}, skills = {}, inventory = { [0] = 0, 0, 0, 0, 0, 0, 0, 0 }, phase = { [0] = 0, 0, 0, 0, 0, 0, 0, 0 } } }
    for slot, code in pairs(o.items or {}) do g.party[0].inventory[slot] = code end
    if o.inc then g.party[0].status = g.party[0].status | E.RunnerStatus.DOWN end
    local r = Route.new(host, game); r:open(S.COMMAND); return r, log, g
end
local function st(r) return r.stack[1].state end
local function clear(log) log.sounds = {} end

-- open: page 6, row 0, display off/on, sound 0x25 [C 0x10F50, 0x10E2E]
local r, log = mk(); check(st(r).page == Pg.POCKET_SECRETARY and st(r).row == 0, "open page")
check(log.sounds[1] == E.MenuSound.MOVE and #log.sounds == 1, "open sound")
check(log.blanks[1] == E.DisplayBlank.OFF and log.blanks[2] == E.DisplayBlank.ON, "blank")
check(log.hold and log.hold[1] == 28 and log.hold[2] == 175, "build blank hold (emulator 28 black, line 175)")
check(log.windows[1] == E.WindowPlane.SIDEBAR, "sidebar visible")
-- page wrap 6->0->6 [C 0x114A0]
r:frame(P.RIGHT); check(st(r).page == 0, "wrap up")
r:frame(P.LEFT); check(st(r).page == 6, "wrap down")
-- inventory row clamp 0..3
r:frame(P.RIGHT); for _ = 1, 6 do r:frame(P.DOWN) end; check(st(r).row == 3, "row clamp hi")
for _ = 1, 6 do r:frame(P.UP) end; check(st(r).row == 0, "row clamp lo")
-- priority: Left beats A and Start in one frame
r:frame(P.LEFT | P.A | P.START); check(st(r).page == 6 and r:is_open(), "priority")
-- posture: Up on page>=2 pushes, Right raises with 0x25, clamp at 4, Down closes
local r2, l2, g2 = mk(); r2:frame(P.UP); check(r2:top().screen == S.POSTURE, "posture push")
clear(l2); r2:frame(P.RIGHT); check(g2.posture == 3 and l2.sounds[1] == 0x25, "raise")
r2:frame(P.RIGHT); r2:frame(P.RIGHT); check(g2.posture == 4, "clamp")
r2:frame(P.DOWN | P.HELD_LEFT | P.LEFT); check(g2.posture == 3 and r2:top().screen == S.POSTURE, "down+held left adjusts")
r2:frame(P.DOWN); check(r2:top().screen == S.COMMAND, "down closes")
-- held L/R on Up does not open posture
r2:frame(P.UP | P.HELD_RIGHT); check(r2:top().screen == S.COMMAND, "held blocks posture")
-- posture Start exits the whole menu (FFF0B1) -- FSR only closed the dialog (design fix D4)
local r3, l3 = mk(); r3:frame(P.UP); r3:frame(P.START); check(not r3:is_open() and l3.closed, "posture start exits")
-- C: reselect, page 6, never opens posture
local r4, _, g4 = mk(); r4:frame(P.LEFT); r4:frame(P.C)
check(st(r4).page == 6 and r4:top().screen == S.COMMAND and g4.reselects == 1, "C path")
-- A on empty inventory slot: 0x24, no push; on item: push ITEM_ACTION
-- slot 0 = column 0, row 0 (0x12148 row*2+column); any nonzero code
local r5, l5 = mk({ items = { [0] = 0x01 } }); r5:frame(P.RIGHT); clear(l5)
r5:frame(P.DOWN); r5:frame(P.A); check(r5:top().screen == S.COMMAND and l5.sounds[2] == 0x24, "empty slot")
r5:frame(P.UP); r5:frame(P.B); check(r5:top().screen == S.ITEM_ACTION, "item action")
-- attributes return rebuilds and resets page to 6; window OFF while open
local r6, l6 = mk(); for _ = 1, 3 do r6:frame(P.LEFT) end; check(st(r6).page == 3, "page 3")
r6:frame(P.A); check(l6.windows[#l6.windows] == E.WindowPlane.OFF, "attr hides sidebar")
r6:frame(P.START); check(r6:top().screen == S.COMMAND and st(r6).page == 6 and #l6.blanks == 6, "attr rebuild (0xD01C display off/on + 0x10F50 rebuild)")
-- pocket secretary exit flag (FFF0CD) leaves menu; otherwise rebuild
-- (Save/Load is a Lua screen now: screens/save_load.lua; Start = rts, no flags -> PS rebuild) [C 0xEFB4]
local r7 = mk(); r7:frame(P.A); for _ = 1, 4 do r7:frame(P.DOWN) end; r7:frame(P.A)
check(r7:top().screen == S.SAVE_LOAD, "PS save/load screen"); r7:frame(P.START); check(r7:is_open() and r7:top().screen == S.POCKET_SECRETARY, "save/load Start -> PS")
local r8 = mk(); r8:frame(P.A); r8:frame(P.START); check(r8:is_open() and r8:top().screen == S.COMMAND, "PS return")
-- cyberware: any edge returns, refresh (no blank) + 0x25
local r9, l9 = mk(); r9:frame(P.LEFT); r9:frame(P.LEFT); r9:frame(P.A); check(r9:top().screen == S.CYBERWARE, "cyber")
clear(l9); r9:frame(P.B); check(r9:top().screen == S.COMMAND and l9.sounds[1] == 0x25 and #l9.blanks == 2, "cyber return")
-- Start exits; incapacitated leader reselected on exit
local ra, la, ga = mk(); ga.party[0].status = ga.party[0].status | E.RunnerStatus.DEAD
ra:frame(P.START); check(not ra:is_open() and ga.reselects == 1, "start exit")
-- the incapacity test reads the +0x10 status byte (DOWN = bit 4) [C 0x10D40]
local rb, _, gb = mk({ inc = true }); check(gb.reselects == 1, "DOWN leader reselected on open")
local rc, _, gc = mk(); check(gc.reselects == 0, "active leader kept on open")
print(string.format("menu tests: %d/%d passed", n - fails, n)); os.exit(fails == 0 and 0 or 1)
