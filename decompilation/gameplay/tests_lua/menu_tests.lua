-- lua5.4 decompilation/gameplay/tests_lua/menu_tests.lua (run from the repository root)
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path
local E = require("enums"); local Route = require("menu")
local P, S, T, Pg = E.Pad, E.MenuScreen, E.MenuTransition, E.MenuPage
local fails, n = 0, 0
local function check(c, msg) n = n + 1; if not c then fails = fails + 1; print("FAIL " .. msg) end end
local function exact_masks(log, phase, frame, expected)
    local actual_counts, expected_counts, count = {}, {}, 0
    local function key(rect) return table.concat(rect, ",") end
    for _, mask in ipairs(log.masks or {}) do
        if mask[1] == phase and mask[2] == frame then
            local rect = { mask[3], mask[4], mask[5], mask[6] }
            local k = key(rect)
            actual_counts[k] = (actual_counts[k] or 0) + 1
            count = count + 1
        end
    end
    for _, rect in ipairs(expected) do
        local k = key(rect)
        expected_counts[k] = (expected_counts[k] or 0) + 1
    end
    if count ~= #expected then return false end
    for k, expected_count in pairs(expected_counts) do
        if actual_counts[k] ~= expected_count then return false end
    end
    return true
end
local function mk(o)
    o = o or {}
    local log = { sounds = {}, redraws = 0, blanks = {}, windows = {}, closed = false, legacy = {} }
    local g = { posture = o.posture or 2, gate = o.gate or 0, reselects = 0 }
    local host = {
        play_sound = function(id) log.sounds[#log.sounds + 1] = id end,
        redraw = function() log.redraws = log.redraws + 1 end,
        blank = function(m, n, line) log.blanks[#log.blanks + 1] = m; if n then log.hold = { n, line } end end,
        set_window = function(w) log.windows[#log.windows + 1] = w end,
        mask = function(phase, frame, x, y, w, h) log.masks = log.masks or {}; log.masks[#log.masks + 1] = { phase, frame, x, y, w, h } end,
        set_screen = function() end,
        menu_closed = function(_, field_return_transition)
            log.closed = true
            log.field_return_transition = field_return_transition
        end,
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
check(log.hold and log.hold[1] == 27 and log.hold[2] == 175, "open build blank hold (emulator 27 black, line 175)")
check(log.windows[1] == E.WindowPlane.SIDEBAR, "sidebar visible")
check(exact_masks(log, E.MenuMask.LEAD, 0, { { 0, 30, 320, 194 } }) and
      exact_masks(log, E.MenuMask.REVEAL, 0, {
          { 0, 168, 56, 8 }, { 80, 168, 40, 8 }, { 240, 168, 40, 8 },
          { 312, 168, 8, 8 }, { 16, 176, 24, 8 }, { 96, 176, 24, 8 },
          { 152, 176, 56, 8 }, { 240, 176, 24, 8 }, { 56, 192, 24, 8 },
          { 56, 200, 24, 8 }, { 112, 200, 48, 8 }, { 168, 200, 72, 8 },
      }) and
      exact_masks(log, E.MenuMask.REVEAL, 1, {
          { 128, 24, 40, 8 }, { 176, 24, 48, 8 }, { 128, 40, 64, 8 },
          { 200, 40, 32, 8 }, { 128, 48, 48, 8 }, { 200, 48, 32, 8 },
          { 136, 64, 40, 8 }, { 184, 64, 32, 8 }, { 184, 72, 40, 8 },
          { 112, 80, 48, 8 }, { 168, 80, 56, 8 }, { 112, 104, 24, 8 },
          { 144, 104, 104, 8 }, { 112, 112, 24, 8 }, { 144, 112, 104, 8 },
      }) and #log.masks == 28, "open lead and staged reveal masks (emulator e2 8600/8628/8629)")
-- AS exit 0xD4A0 staged frames + return build [C emulator e2 8960..8990]
do
    local ra, la = mk(); st(ra).page = Pg.ATTRIBUTES_SKILLS; ra:frame(P.A)
    check(ra:top().screen == S.ATTRIBUTES_SKILLS, "AS open")
    la.masks = {}; la.hold = nil; ra:frame(P.START)
    local lead0, lead1, rev0, rev1 = 0, 0, 0, 0
    for _, m in ipairs(la.masks) do
        if m[1] == E.MenuMask.LEAD then if m[2] == 0 then lead0 = lead0 + 1 else lead1 = lead1 + 1 end
        elseif m[2] == 0 then rev0 = rev0 + 1 else rev1 = rev1 + 1 end
    end
    check(ra:top().screen == S.COMMAND and lead0 == 6 and lead1 == 2 and rev0 == 1 and rev1 == 1, "AS exit masks")
    check(la.hold and la.hold[1] == 27 and la.hold[2] == 211, "AS return hold 27 black, line 211")
end
-- page wrap 6->0->6 [C 0x114A0]
r:frame(P.RIGHT); check(st(r).page == 0, "wrap up")
r:frame(P.LEFT); check(st(r).page == 6, "wrap down")
-- [C emulator] Model 101T appears after eight VBlanks; Armor Clothing after seven.
-- [H] The Lua hold represents the CPU-bound picture decode and extends the
-- measured per-picture delays to the native menu route.
do
    local held, held_log, held_game = mk({ items = {
        [0] = E.ItemId.MODEL_101T,
        [1] = E.ItemId.ARMOR_CLOTHING,
        [2] = E.ItemId.FETISH,
    } })
    local runner = held_game.party[0]
    runner.weapon_slot, runner.weapon_code, runner.rounds = 0,
        E.ItemId.MODEL_101T, 12
    clear(held_log)
    held:frame(P.RIGHT)
    check(st(held).page == 0 and st(held).inventory_cursor_hidden_frames == 1 and
          st(held).inventory_cursor_hidden and
          not st(held).inventory_detail_pending and
          st(held).inventory_ammo_count == 12 and
          held_log.sounds[1] == E.MenuSound.MOVE,
          "inventory entry draws details and sound while cursor waits one frame [C emulator]")
    held:frame(0)
    check(st(held).inventory_cursor_hidden_frames == 0 and
          not st(held).inventory_cursor_hidden,
          "inventory entry cursor appears on the next frame [C emulator]")

    held:frame(P.RIGHT)
    check(st(held).inventory_item_hold_frames == 7 and
          st(held).inventory_detail_pending and
          st(held).inventory_cursor_hidden and
          st(held).inventory_ammo_count == nil and #held_log.sounds == 1,
          "armor detail holds blank with deferred sound [C emulator]")
    for _ = 1, 6 do held:frame(0) end
    check(st(held).inventory_detail_pending and #held_log.sounds == 1,
          "seven-frame picture hold keeps panel blank for six following frames [C emulator]")
    held:frame(0)
    check(st(held).inventory_detail_pending and #held_log.sounds == 2 and
          held_log.sounds[2] == E.MenuSound.MOVE,
          "armor MOVE cue on the seventh following frame, panel still blank [C emulator zsnd e6]")
    held:frame(0)
    check(not st(held).inventory_detail_pending and
          not st(held).inventory_cursor_hidden and #held_log.sounds == 2,
          "armor detail appears one frame after the MOVE cue [C emulator e6 8748/8789]")

    held:frame(P.LEFT)
    check(st(held).inventory_item_hold_frames == 8 and
          st(held).inventory_detail_pending and #held_log.sounds == 2 and
          st(held).inventory_cursor_hidden and
          st(held).inventory_ammo_count == 12,
          "pistol detail uses the measured eight-frame hold and live rounds [C emulator]")
    for _ = 1, 7 do held:frame(0) end
    check(st(held).inventory_detail_pending and #held_log.sounds == 2,
          "pistol detail remains blank through seven following frames [C emulator]")
    held:frame(0)
    check(st(held).inventory_detail_pending and #held_log.sounds == 3 and
          held_log.sounds[3] == E.MenuSound.MOVE,
          "pistol MOVE cue on the eighth following frame, panel still blank [C emulator zsnd e6]")
    held:frame(0)
    check(not st(held).inventory_detail_pending and
          not st(held).inventory_cursor_hidden and #held_log.sounds == 3,
          "pistol detail appears one frame after the MOVE cue [C emulator e6 8748/8789]")
end
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
do
    local close, close_log = mk()
    close:frame(P.START)
    local state = close:field_return_transition_snapshot()
    check(not close:is_open() and close_log.field_return_transition and state and
          state.active and state.elapsed_frames == 0 and
          state.phase == E.FieldReturnTransitionPhase.MENU_FADE_OUT,
          "command Start begins the field-return palette transition")
    for frame = 1, E.FieldReturnTransitionTiming.FIELD_LIVE_FRAME do
        close:frame(frame == 50 and P.START or 0)
        state = close:field_return_transition_snapshot()
        if frame == 2 then
            check(state.phase == E.FieldReturnTransitionPhase.MENU_FADE_OUT and
                  state.palette_step == 0,
                  "menu stays unchanged through P+2")
        elseif frame == 15 then
            check(state.phase == E.FieldReturnTransitionPhase.MENU_FADE_OUT and
                  state.palette_step == E.FieldReturnTransitionTiming.PALETTE_STEPS,
                  "menu fade reaches black at P+15")
        elseif frame == 50 then
            check(not close:is_open() and state.active and
                  state.phase == E.FieldReturnTransitionPhase.BLACK_HOLD,
                  "Start during black hold is ignored")
        elseif frame == 99 then
            check(state.phase == E.FieldReturnTransitionPhase.BLACK_HOLD,
                  "black hold lasts through P+99")
        elseif frame == 100 then
            check(state.phase == E.FieldReturnTransitionPhase.FIELD_FADE_IN and
                  state.palette_step == 1,
                  "field fade-in begins at P+100")
        elseif frame == E.FieldReturnTransitionTiming.FIELD_LIVE_FRAME then
            check(not state.active and
                  state.phase == E.FieldReturnTransitionPhase.COMPLETE,
                  "field is live at P+112")
        end
    end
end
-- C: reselect, page 6, never opens posture
local r4, _, g4 = mk(); r4:frame(P.LEFT); r4:frame(P.C)
check(st(r4).page == 6 and r4:top().screen == S.COMMAND and g4.reselects == 1, "C path")
-- A on empty inventory slot: 0x24, no push; on item: push ITEM_ACTION
-- slot 0 = column 0, row 0 (0x12148 row*2+column); any nonzero code
local r5, l5 = mk({ items = { [0] = 0x01 } }); r5:frame(P.RIGHT); clear(l5)
r5:frame(P.DOWN); r5:frame(P.A); check(r5:top().screen == S.COMMAND and l5.sounds[2] == 0x24, "empty slot")
r5:frame(P.UP); r5:frame(P.B); check(r5:top().screen == S.ITEM_ACTION, "item action")
-- ROM 0x117BA..0x117D0 and 0x11824..0x1183A select a 17-cell footer for
-- each action selector; 0x11788..0x11798 choose the First Aid table for
-- medical item codes. The source padding is part of the text placement.
do
    local expected_actions = {
        [E.ItemAction.EQUIP_OR_USE] = "  Equip/Unequip  ",
        [E.ItemAction.DISCARD] = "     Discard     ",
        [E.ItemAction.TRADE] = "      Trade      ",
        [E.ItemAction.CANCEL] = "     Cancel      ",
    }
    local expected_first_aid = {
        [E.ItemAction.EQUIP_OR_USE] = "    First Aid    ",
        [E.ItemAction.DISCARD] = "     Discard     ",
        [E.ItemAction.TRADE] = "      Trade      ",
        [E.ItemAction.CANCEL] = "     Cancel      ",
    }
    local function action_route(item_code)
        local route = mk({ items = { [0] = item_code } })
        route:frame(P.RIGHT) -- pocket secretary wraps to Inventory
        route:frame(P.A)
        return route
    end
    for _, fixture in ipairs({
        { E.ItemId.STREETLINE_SPECIAL, expected_actions, "ordinary" },
        { E.ItemId.MEDKIT, expected_first_aid, "First Aid" },
    }) do
        local route = action_route(fixture[1])
        for selector = E.ItemAction.EQUIP_OR_USE, E.ItemAction.CANCEL do
            check(route:top().state.selector == selector and
                  route:top().state.footer_text == fixture[2][selector],
                  fixture[3] .. " action footer selector " .. selector)
            if selector < E.ItemAction.CANCEL then route:frame(P.RIGHT) end
        end
    end
end
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
