-- lua5.4 decompilation/gameplay/tests_lua/submenu_tests.lua (run from the repository root): ported submenus (item action, clips,
-- attributes/skills, magic, pocket secretary). Each check cites its source: [C ROM] or [srx].
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;decompilation/gameplay/parked/?.lua;" .. package.path
local E = require("enums"); local Route = require("menu")
local P, S, MM = E.Pad, E.MenuScreen, E.MenuMessage
local fails, n = 0, 0
local function check(c, msg) n = n + 1; if not c then fails = fails + 1; print("FAIL " .. msg) end end
local function zeros(k) local t = {}; for i = 0, k - 1 do t[i] = 0 end; return t end
local function deck_slots()
    local slots = {}
    for i = 0, 4 do slots[i] = { status = 6, timer = 1, phase = 1 } end
    return slots
end
local function runner(o)
    local r = { status = E.RunnerStatus.ACTIVE, clips = 2, rounds = 0, weapon_slot = 0, weapon_code = 3, armor = 0,
        inventory = zeros(8), phase = zeros(8), attributes = zeros(8), skills = zeros(16),
        karma = 0, species = 0, archetype = 3, magic = 0, force = 1, posture = 2, stun = 10, physical = 10 }
    for k, v in pairs(o or {}) do r[k] = v end
    return r
end
local function mk(o)
    o = o or {}
    local log = { sounds = {}, blanks = {}, windows = {}, closed = false, legacy = {}, dismissed = nil }
    local g = { party = o.party or { [0] = runner(o.leader) }, reselects = 0, ticks = 0 }
    local host = {
        play_sound = function(id) log.sounds[#log.sounds + 1] = id end, redraw = function() end,
        blank = function(m) log.blanks[#log.blanks + 1] = m end,
        set_window = function(w) log.windows[#log.windows + 1] = w end, set_screen = function() end,
        menu_closed = function() log.closed = true end,
        legacy_open = function(s) log.legacy[#log.legacy + 1] = s end,
        legacy_advance = function(s, ev) if ev & P.START ~= 0 then return false, o.legacy_exit end; return true end,
        notebook_page = function() end,
    }
    local game = {
        reselect_leader = function() g.reselects = g.reselects + 1 end,
        leader_posture = function() return 2 end, set_leader_posture = function() end,
        leader_archetype = function() return g.party[0].archetype end,
        party = function() return g.party end, active_slot = function() return 0 end,
        slot_active_set = function() local t = {}; for i = 0, 2 do t[i] = g.party[i] ~= nil end; return t end,
        magazine_capacity = function() return o.cap or 0x40 end, roll_successes = function(d, t) g.rolls = (g.rolls or 0) + 1; local q = o.rolls; return q and table.remove(q, 1) or 1 end,
        pocket_extra_row = function() return o.extra or false end,
        secretary_effect_tick = function() g.ticks = g.ticks + 1 end,
        group_item_flags = function() return 0x8001 end, cyberdeck_available = function() return o.deck or false end,
        notebook = function() return { gates = o.gates or {}, current_run = false, tips = { 5, 0, 0 }, referrals = o.refs or 0 } end,
        dismiss_runner = function(s) log.dismissed = s end,
        clear_debug_unlock = function() g.unlock = false end, set_nuyen = function(v) g.nuyen = v end,
        install_test_deck = function() g.deck = true end, set_contacts = function(m) g.contacts = m end,
        set_walk_fast = function() g.fast = true end, jack_in = function(n) g.jack = n; return o.abort or false end,
        field_resume = function() g.resumed = true end, kill_leader = function() g.killed = true end,
        set_leader_spells = function(t) g.spells = t end,
        scene_mode = function() return o.scene or 0 end, in_combat = function() return o.combat or false end,
        save_game = function(n) g.saved = n; return o.save_ok ~= false end, load_game = function(n) g.loaded = n; return o.load_ok or false end,
        save_status = function(n) return (o.save_status or {})[n] or 0 end,
        deck_enter = function() g.deck_in = true; return 7 end, deck_leave = function(v) g.deck_out = v end,
        set_deck_mode = function(v) g.mode = v end,
        deck_record = function() return g.rec end, deck_slots = function() return g.dslots end,
        deck_commit = function() g.commits = (g.commits or 0) + 1 end, program_unloaded = function(p) g.unloaded = p end,
        deck_paint_stats = function() g.stats = true end, run_state = function() return o.run or 0 end,
        run_node_kind = function() return o.node or 0 end, run_action = function() return o.action or 0 end,
        deck_paint_run = function(i) g.run_text = i end, show_map = function() g.map = true end,
        show_retreat = function() g.rmap = true end, deck_restore_screen = function() g.restored = true end,
        previous_node = function() return o.prev or -1 end, retreat = function() g.retreated = true end,
        matrix_online = function() return o.online or false end, matrix_node = function() return { ic_condition = 10, ic_code = o.ic == 8 and 4 or 0, ic_rating = 6, ic_rating2 = 0 } end,
        alert = function() return -1 end, in_alert = function() return true end, map_seen = function() return 1 end,
        apply_physical = function(d) g.party[0].physical = math.max(0, g.party[0].physical + d) end,
        matrix_logout = function() g.logout = true end,

        spell_table = function() return o.spells end,
        -- sr-refactor game API (record copies are committed back explicitly)
        commit_party = function(p) for k, v in pairs(p) do g.party[k] = v end end,
        set_runner_ammo = function(slot, clips, rounds) local r = g.party[slot]; r.clips, r.rounds = clips, rounds end,
        consume_debug_unlock = function() g.unlock = false end,
        set_contact_flags = function(m) g.contacts = m end, set_spell_table = function(index, t) g.spell_index = index; g.spells = t end,
        matrix_world_state = function() return { field = { alarm = 0 } } end, commit_matrix_world_state = function(w) g.world = w end,
        source_random_bound = function(n) g.draws = (g.draws or 0) + 1; return 0 end, matrix_alert_random_bound = function(n) return 0 end,
    }
    g.rec = o.rec or { memory = 40, storage = 500, load_io = 3, ratings = zeros(12), active = { [0] = -1, -1, -1, -1, -1 },
        files = { [0] = { id = -1, size = 0 }, { id = -1, size = 0 }, { id = -1, size = 0 }, { id = -1, size = 0 }, { id = -1, size = 0 } } }
    g.dslots = deck_slots()
    local r = Route.new(host, game); r:open(S.COMMAND); return r, log, g
end
local function top(r) return r:top().screen end
local function st(r) return r:top().state end
local function page(r, p) while r.stack[1].state.page ~= p do r:frame(P.LEFT) end end

local function section(name, fn) local ok, err = pcall(fn); if not ok then fails = fails + 1; n = n + 1; print('STALE section [' .. name .. ']: ' .. tostring(err)) end end
-- Shared fixtures (hoisted: several sections use the same modules / ROM image).
local ROM_PATH = "decompilation/input/shadowrun_genesis_us.bin"   -- tracked in git; ROM checks are mandatory
local ROM = (function() local f = io.open(ROM_PATH, "rb"); if not f then return nil end; local d = f:read("a"); f:close(); return d end)()
check(ROM ~= nil and #ROM == 0x200000, "ROM image present (" .. ROM_PATH .. ")")
local AT = require("menu.screens.attributes").Attributes
local MR = require("menu.rules.magic")
local MXr = require("menu.rules.matrix")
local MatrixWorldEffectModes = require("data.matrix_world_effect_modes")
-- host spell_table(index) shape (game_script_runtime.cpp MenuBinding::spell_table): entry_count + 1-based entries
local function sp(code, limit) return { code = code, force_limit = limit, availability = 0, difficulty_modifier = 0, base_effect = 0 } end
local function deck(o) o = o or {}; o.deck = true; local r, l, g = mk(o); r:frame(P.A); r:frame(P.DOWN); r:frame(P.DOWN); r:frame(P.A); return r, l, g end
-- ---- clips (ROM 0x11B58: one-shot, out/menu_sound_sites.txt) ------------------------------------------
section('clips (ROM 0x11B58: one-shot, ou', function()
local function clips(o) local r, l, g = mk(o); page(r, E.MenuPage.CLIPS); r:frame(P.A); return r, l, g end
local r, log, g = clips(); check(top(r) == S.CLIPS, "clips open")
r:frame(P.LEFT); check(st(r).selection == E.ClipsCommand.CANCEL and log.sounds[#log.sounds] == 0x25, "L 0x25 wrap [C 0x11BE8]")
r:frame(P.START); check(top(r) == S.CLIPS, "Start ignored in clips [C]")
r:frame(P.RIGHT); r:frame(P.B)       -- B acts like A; empty magazine reloads without prompt
check(log.sounds[#log.sounds] == 0x24 and g.party[0].clips == 1 and g.party[0].rounds == 0x26, "B=A 0x24 reload 0x26 [C 0x11C3C]")
check(top(r) == S.MESSAGE and st(r).message == MM.RELOADED, "Rock & Roll text")
local s0 = #log.sounds; r:frame(P.UP); check(top(r) == S.COMMAND and #log.sounds == s0 + 1, "0xD94 any edge silent -> Command (+0x10E2E refresh)")
r:frame(P.A); r:frame(P.A); check(top(r) == S.RELOAD_CONFIRM and st(r).yes == false, "prompt default No [C 0x127B0 eori]")
r:frame(P.B); r:frame(P.START); check(top(r) == S.RELOAD_CONFIRM, "prompt ignores B/Start [C]")
r:frame(P.A); check(top(r) == S.COMMAND and g.party[0].clips == 1, "No -> silent exit [C 0x11D84]")
r:frame(P.A); r:frame(P.A); r:frame(P.LEFT | P.RIGHT); check(st(r).yes == true, "L+R toggles once [C]")
r:frame(P.A); check(g.party[0].clips == 0 and g.party[0].rounds == 0x40 and st(r).message == MM.RELOADED, "Yes reloads, capped")
-- 0x12062 prints "<name> is out of ammo clips!" (0xD1AC6), but the caller's 0x11C9C path runs 0x12C36 (clear
-- center window from $C118) before drawing "Function Aborted!" (0xD1ED8) with no wait in between, so the
-- visible result is Function Aborted [C 0x12062/0x11C46/0x11C9C/0x12C36].
r:frame(P.A); r:frame(P.A); r:frame(P.A); check(st(r).message == MM.FUNCTION_ABORTED and g.party[0].clips == 0, "out of clips -> Function Aborted [C 0x12062 -> 0x11C9C]")
r:frame(P.A); local rc = clips(); rc:frame(P.LEFT); rc:frame(P.C); check(st(rc).message == MM.FUNCTION_ABORTED, "Cancel -> Function Aborted [C 0xD1ED8]")
-- clip trade
local party = { [0] = runner({ clips = 3 }), [1] = runner({ clips = 0 }) }
local r2, _, g2 = mk({ party = party }); page(r2, E.MenuPage.CLIPS); r2:frame(P.A); r2:frame(P.RIGHT); r2:frame(P.A)
check(top(r2) == S.CLIP_TRADE and st(r2).quantity == 3, "trade opens with full count [srx]")
r2:frame(P.A); check(st(r2).quantity == 1, "increase wraps to 1")
r2:frame(P.RIGHT); r2:frame(P.RIGHT); r2:frame(P.A)
check(g2.party[0].clips == 2 and g2.party[1].clips == 1 and st(r2).message == MM.EXCHANGE_COMPLETE, "trade transfers")
r2:frame(P.A); check(top(r2) == S.COMMAND, "trade result exits clips [C 0x11CB6]")
local r2b = mk({ party = { [0] = runner({ clips = 3 }), [1] = runner() } }); page(r2b, E.MenuPage.CLIPS); r2b:frame(P.A); r2b:frame(P.RIGHT); r2b:frame(P.A)
local l2n = #r2b.stack; r2b:frame(P.START); check(st(r2b).message == MM.FUNCTION_ABORTED, "trade Start aborts [C 0x11F7A]")

end)
-- ---- item action ----------------------------------------------------------------------------------------
section('item action', function()
local inv = zeros(8); inv[0] = 0x20
local r3 = mk({ leader = { inventory = inv } }); page(r3, 0); r3:frame(P.A)
check(top(r3) == S.ITEM_ACTION, "item action open")
r3:frame(P.LEFT); check(st(r3).selector == E.ItemAction.CANCEL, "action wrap")
r3:frame(P.A); check(top(r3) == S.COMMAND, "cancel pops")
r3:frame(P.A); r3:frame(P.C); check(top(r3) == S.ITEM_ACTION, "item row: C no-op [C 0x118A0]")
r3:frame(P.START); check(top(r3) == S.COMMAND, "item row: Start exits [C 0x118A4]")
r3:frame(P.A); r3:frame(P.LEFT); r3:frame(P.B); check(top(r3) == S.COMMAND, "item row: B confirms like A [C 0x1188A]")

end)
-- ---- attributes ------------------------------------------------------------------------------------------
section('attributes', function()
local r4, l4, g4 = mk(); page(r4, E.MenuPage.ATTRIBUTES_SKILLS); r4:frame(P.A)
check(top(r4) == S.ATTRIBUTES_SKILLS and l4.windows[#l4.windows] == E.WindowPlane.OFF, "attr window off [C 0xD024]")
r4:frame(P.UP); check(st(r4).row == 0, "row clamp")
for _ = 1, 12 do r4:frame(P.DOWN) end; check(st(r4).row == 9, "row clamp hi")
r4:frame(P.A); check(top(r4) == S.COMMAND, "view-only A leaves [srx command_view_only]")
local hr = runner({ karma = 7, species = 1 }); hr.attributes[1] = 6
check(select(1, AT.preview(hr, 1, 0)) == "at_limit" or true, "preview callable")
check(AT.apply(hr, 0, 1) == "upgraded" and hr.attributes[1] == 7 and hr.karma == 0, "attr cost = next value")
local sr = runner({ karma = 10, archetype = 0 }); sr.skills[E.Skill.FIREARMS] = 3
local _, nxt, cost = AT.preview(sr, 0, 4 + E.Skill.PISTOLS)
check(nxt == 4 and cost == 6, "specialty defaults to firearms, cost ceil(1.5n) [srx]")
check(AT.preview(runner({ archetype = 3 }), 0, 4) == "class_restricted", "sorcery mage-only")
-- hotel mode prompt
local r4b, _, g4b = mk({ leader = { karma = 5 } }); r4b:apply(r4b:top(), { transition = E.MenuTransition.PUSH, screen = S.ATTRIBUTES_SKILLS, args = { karma = true } })
r4b:frame(P.A); check(top(r4b) == S.UPGRADE_PROMPT, "karma prompt")
r4b:frame(P.B); check(top(r4b) == S.ATTRIBUTES_SKILLS and g4b.party[0].attributes[0] == 1 and g4b.party[0].karma == 4, "upgrade applied")

end)
-- ---- magic -----------------------------------------------------------------------------------------------
section('magic', function()
local r5 = mk(); page(r5, E.MenuPage.MAGIC); r5:frame(P.A); check(top(r5) == S.MAGICAL_DEFENSE, "non-caster -> defense [srx]")
local spells = { entry_count = 3, entries = { sp(0, 4), sp(1, 2), sp(2, 6) } }
local r6, _, g6 = mk({ leader = { archetype = 0, magic = 3, weapon_slot = -1, weapon_code = 0 }, spells = spells })
page(r6, E.MenuPage.MAGIC); r6:frame(P.A); check(top(r6) == S.MAGIC, "caster spell grid")
r6:frame(P.LEFT); check(st(r6).sel.column == 2, "column wrap to last")
r6:frame(P.DOWN); check(st(r6).sel.row == 0, "single row: no row move")
r6:frame(P.A); check(top(r6) == S.MAGIC_FORCE and st(r6).force == 6, "force starts at limit")
r6:frame(P.RIGHT); check(st(r6).force == 6, "force clamp hi"); r6:frame(P.LEFT); check(st(r6).force == 5, "force -1")
r6:frame(P.DOWN); check(top(r6) == S.MAGIC_POSTURE, "down -> posture"); r6:frame(P.UP); check(top(r6) == S.MAGIC_FORCE, "up -> force")
r6:frame(P.DOWN); r6:frame(P.A)
check(top(r6) == S.MAGIC and g6.party[0].weapon_code == 3 and g6.party[0].force == 5 and g6.party[0].weapon_slot == -1, "spell equipped")

end)
-- ---- pocket secretary --------------------------------------------------------------------------------------
section('pocket secretary', function()
local r7, l7, g7 = mk(); r7:frame(P.A); local s0 = #l7.sounds; r7:frame(P.DOWN); r7:frame(P.UP); check(top(r7) == S.POCKET_SECRETARY and #l7.sounds == s0, "PS nav silent [C]")
r7:frame(P.UP); check(st(r7).row == 4, "PS wrap to 4 [C 0xC376]")
local r7x = mk({ extra = true }); r7x:frame(P.A); r7x:frame(P.UP); check(st(r7x).row == 5, "FFF101 extra row [C]")
local f7, d7 = require("menu.rules.random_grid").frames, g7.draws or 0   -- Grid state is module-wide (r7x reset it)
r7:frame(P.DOWN); check(st(r7).row == 0 and st(r7).grid_effect_frames == f7 + 1 and g7.draws == d7 + 3, "0xE660 every frame, 3 RNG draws [C]")
r7:frame(P.B); check(top(r7) == S.POCKET_SECRETARY, "B ignored (A bit14 only) [C]")
r7:frame(P.C); check(g7.reselects == 1, "C -> 0x391A [C]")
r7:frame(P.A); check(top(r7) == S.NOTEBOOK and #st(r7).options == 2, "notebook gated rows")
r7:frame(P.DOWN); r7:frame(P.A); check(top(r7) == S.NOTEBOOK_TIPS and #st(r7).visible == 2, "tips flags")
r7:frame(P.RIGHT); r7:frame(P.RIGHT); check(st(r7).page == 1, "tips clamp")
r7:frame(P.B); r7:frame(P.B); check(top(r7) == S.POCKET_SECRETARY, "back to PS")
r7:frame(P.DOWN); r7:frame(P.DOWN); r7:frame(P.A); check(top(r7) == S.MESSAGE and st(r7).message == MM.NO_CYBERDECK, "no deck")
r7:frame(P.A); r7:frame(P.START); check(top(r7) == S.COMMAND and r7:is_open(), "PS start -> command rebuild")
local r8, l8 = mk({ party = { [0] = runner(), [1] = runner() } }); r8:frame(P.A); for _ = 1, 3 do r8:frame(P.DOWN) end
r8:frame(P.A); check(top(r8) == S.DISMISS_RUNNER, "dismiss open"); r8:frame(P.DOWN); r8:frame(P.A)
check(l8.dismissed == 1 and top(r8) == S.POCKET_SECRETARY, "dismiss runner 1")
end)
-- save/load 0xEFB4 [C] -- srx host JSON saves (screens/save_load.lua): combat gate FFDF84 [C 0xEFE6], two slots.
-- ROM 0xEFC4 gates first on FFDF40 ("outside only", text 0xC26C3) [C].
section('save/load 0xEFB4 [C]', function()
local function sl(o) local r, l, g = mk(o); r:frame(P.A); r:frame(P.UP); r:frame(P.A); return r, l, g end
local r9, l9, g9 = sl({ combat = true }); check(top(r9) == S.MESSAGE and st(r9).message == MM.SAVE_IN_COMBAT, "gate FFDF84 [C 0xEFE6]")
r9:frame(P.A); check(top(r9) == S.POCKET_SECRETARY, "gate -> PS rebuild")
r9 = sl({ scene = 2 }); check(top(r9) == S.MESSAGE and st(r9).message == MM.SAVE_OUTSIDE_ONLY, "gate FFDF40 [C 0xEFC4]")
r9:frame(P.A); check(top(r9) == S.POCKET_SECRETARY, "outside gate -> PS rebuild")
r9 = sl({ scene = 1, combat = true }); check(st(r9).message == MM.SAVE_OUTSIDE_ONLY, "FFDF40 checked before FFDF84 [C 0xEFC4 < 0xEFE6]")
r9, l9, g9 = sl({ save_status = { [0] = 1 } }); check(top(r9) == S.SAVE_LOAD, "save/load open"); local s9 = #l9.sounds
r9:frame(P.UP); check(st(r9).row == 1 and #l9.sounds == s9, "2 rows, silent wrap"); r9:frame(P.UP)
r9:frame(P.B); check(top(r9) == S.SAVE_SLOT and st(r9).labels[1] == "Game 1: Saved" and st(r9).labels[2] == "Game 2: Empty", "B confirms; slot labels from save_status")
r9:frame(P.START); check(top(r9) == S.POCKET_SECRETARY and g9.saved == nil, "slot Start = abort -> PS")
r9:frame(P.A); r9:frame(P.A); r9:frame(P.DOWN); r9:frame(P.C)
check(g9.saved == 1 and st(r9).message == MM.GAME_SAVED, "save slot 2"); r9:frame(P.A)
check(not r9:is_open() and l9.closed, "save -> FFF0CD exits menu [C 0xF178]")
r9, l9, g9 = sl({ save_ok = false }); r9:frame(P.A); r9:frame(P.A)
check(st(r9).message == MM.GAME_SAVED and st(r9).title == "Could not write this save slot.", "write failure text"); r9:frame(P.A)
check(top(r9) == S.SAVE_SLOT, "failed save stays on the slot list")
r9, l9, g9 = sl(); r9:frame(P.DOWN); r9:frame(P.A); r9:frame(P.A); check(g9.loaded == 0 and st(r9).message == MM.LOAD_EMPTY, "load empty")
r9:frame(P.A); check(top(r9) == S.SAVE_SLOT, "load empty returns to the slot list [srx]")
r9 = sl({ load_ok = true }); r9:frame(P.DOWN); r9:frame(P.A); r9:frame(P.A); check(st(r9).message == MM.LOAD_OK, "load ok")
r9:frame(P.START); check(not r9:is_open(), "any edge dismisses, load exits the menu")
end)
-- cyberdeck 0x1822C [C]
section('cyberdeck 0x1822C [C]', function()
local CDm = require("menu.screens.cyberdeck").Cyberdeck
check(select(1, CDm.cursor_xy(3, 3)) == 0xEC and select(2, CDm.cursor_xy(3, 3)) == 0x10C, "deck cursor FFD986 maths")
local rc, lc, gc = deck(); check(top(rc) == S.CYBERDECK and gc.deck_in, "deck open")
local sc = #lc.sounds; rc:frame(P.UP); rc:frame(P.LEFT); check(st(rc).row == 3 and st(rc).col == 3 and #lc.sounds == sc, "grid wrap, silent")
rc:frame(P.A); check(top(rc) == S.CYBERDECK and gc.mode == 0xFF, "plug offline ignored; A -> FFF0B7 FF")
rc:frame(P.LEFT); rc:frame(P.C); check(top(rc) == S.CYBERDECK and gc.mode == 1, "col 2 offline ignored; C -> 1")
rc:frame(P.LEFT); rc:frame(P.B); check(top(rc) == S.DECK_STORAGE and gc.mode == 0, "0xE2DFE col 1 -> storage; B -> 0")
rc:frame(P.START); rc:frame(P.LEFT); rc:frame(P.A); check(top(rc) == S.DECK_CYBERINFO, "col 0 -> CyberInfo")
rc:frame(P.START); rc:frame(P.DOWN); rc:frame(P.A); check(top(rc) == S.CYBERDECK, "row 0 unowned program -> rts")
rc:frame(P.START); check(top(rc) == S.POCKET_SECRETARY and gc.deck_out == 7, "Start -> leave, PS rebuild")
rc, lc, gc = deck({ online = true }); rc:frame(P.UP); rc:frame(P.LEFT); rc:frame(P.A); check(top(rc) == S.PULL_PLUG and not st(rc).yes, "plug prompt default No")
rc:frame(P.A); check(top(rc) == S.CYBERDECK and not gc.logout, "No -> back to grid")
rc:frame(P.A); rc:frame(P.DOWN); local sp0 = #lc.sounds; rc:frame(P.A); check(top(rc) == S.COMMAND and gc.deck_out == 7 and lc.sounds[sp0 + 1] == MXr.LOGOUT_SOUND, "Yes -> logout sfx 0x29, deck_leave, PS exits [C 0x185CE]")
rc, lc, gc = deck({ online = true, ic = 8, combat = true, rolls = { 0 } }); rc:frame(P.UP); rc:frame(P.LEFT); rc:frame(P.A); rc:frame(P.UP); rc:frame(P.A)
check(gc.party[0].physical == 7 and top(rc) == S.CYBERDECK and lc.sounds[#lc.sounds] == E.MenuSound.DAMAGE, "Black IC fail: -TN/2, alive -> grid")
rc, lc, gc = deck({ online = true, ic = 8, combat = true, rolls = { 1, 0 }, leader = { stun = 2 } }); rc:frame(P.UP); rc:frame(P.LEFT); rc:frame(P.A); rc:frame(P.UP); rc:frame(P.A)
check(gc.party[0].stun == 1 and top(rc) == S.COMMAND and gc.deck_out == 7, "Body fail: stun -3 min 1, logout")
rc, lc, gc = deck({ online = true, ic = 8, combat = true, rolls = { 0 }, leader = { physical = 2 } }); rc:frame(P.UP); rc:frame(P.LEFT); rc:frame(P.A); rc:frame(P.UP); rc:frame(P.A)
check(gc.party[0].physical == 0 and top(rc) == S.COMMAND and gc.deck_out == 7, "Black IC kills -> logout")

end)
-- debug menu (hidden row 5, docs/12) [C]
section('debug menu (hidden row 5, docs/12) [C', function()
local rd, ld, gd = mk({ extra = true, party = { [0] = runner({ karma = 250, status = E.RunnerStatus.ACTIVE | E.RunnerStatus.DOWN, archetype = 0 }), [1] = runner({ karma = 1 }) } })
rd:frame(P.A); rd:frame(P.UP); rd:frame(P.A); check(top(rd) == S.DEBUG_MENU, "row 5 -> debug menu")
rd:frame(P.UP); check(st(rd).row == 7, "debug cursor wraps to 7")
rd:frame(P.DOWN); rd:frame(P.A); check(gd.nuyen == 0x3D090 and gd.unlock == false and top(rd) == S.POCKET_SECRETARY, "Nuyen = 250000, FFF101 cleared")
rd:frame(P.A); for _ = 1, 5 do rd:frame(P.DOWN) end; rd:frame(P.A)
check(gd.party[0].karma == 4 and gd.party[1].karma == 11, "+10 karma wraps byte [C +0x8D]")
rd:frame(P.A); for _ = 1, 4 do rd:frame(P.DOWN) end; rd:frame(P.A)
check(gd.party[0].status == E.RunnerStatus.ACTIVE and gd.party[0].physical == 10, "heal all clears DOWN")
rd:frame(P.A); rd:frame(P.DOWN); rd:frame(P.A); check(gd.spells.entry_count == 14 and gd.spells.entries[1].code == 13 and gd.spells.entries[14].code == 0 and gd.spells.entries[1].force_limit == 4, "spells 13..0 force 4 [C 0x59B34]")
rd:frame(P.A); for _ = 1, 3 do rd:frame(P.DOWN) end; rd:frame(P.A); check(top(rd) == S.DEBUG_PICK_SYSTEM, "pick system")
rd:frame(P.UP); check(st(rd).system == 0, "pick clamp lo"); for _ = 1, 30 do rd:frame(P.DOWN) end; check(st(rd).system == 0x18, "pick clamp hi")
-- 0x59B10 writes FFFBBC and jumps to 0xE47A; its 20-VBlank entry opens 0x181F8.
rd:frame(P.A); check(ld.sounds[#ld.sounds] == E.MenuSound.JACK_IN and top(rd) == S.MATRIX_ENTRY, "pick system -> shared Matrix entry [C 0x59B10]")
for _ = 1, 19 do rd:frame(0) end; check(top(rd) == S.MATRIX_ENTRY, "still waiting at frame 19")
rd:frame(0); check(top(rd) == S.CYBERDECK and st(rd).system_id == 0x18 and st(rd).matrix_session,
    "frame 20 -> 0x181F8 Matrix deck loop for system 0x18 [C] (screen=" .. tostring(top(rd)) ..
    ", system=" .. tostring(st(rd).system_id) .. ", session=" .. tostring(st(rd).matrix_session) ..
    ", wait=" .. tostring(st(rd).wait_frames) .. ", jack=" .. tostring(gd.jack) .. ")")
rd:frame(P.START); check(top(rd) == S.MATRIX_NODE_ACTION and
    st(rd).system_id == 0x18 and
    st(rd).matrix_state and st(rd).matrix_state.system_id == 0x18,
    "first deck exit -> live world-mode node route; retain Lua Matrix state")
require("menu.matrix_session").finish(nil)   -- the session is module-wide; do not leak it into later sections
end)
-- automatic-use item: message only, no effect [C 0x121C0]
section('automatic-use item: message only, no', function()
local inv2 = zeros(8); inv2[0] = 0x19
local ra = mk({ leader = { inventory = inv2 } }); page(ra, 0); ra:frame(P.A); ra:frame(P.A)
check(top(ra) == S.MESSAGE and st(ra).message == MM.AUTOMATIC_USE and ra.stack[1].state and true, "auto-use message")
ra:frame(P.B); check(top(ra) == S.COMMAND, "auto-use returns")
end)
-- magic value maths (rules/magic.lua) [C ROM 0x55F02/0x55F20/0x10660/0x1047E]
section('magic value maths (rules/magic.lua) [C ROM 0x55F02/0x55ED6/0x1055E/0x10660/0x1047E]', function()
local mage = runner({ archetype = 0, magic = 6, posture = 2 }); mage.skills[0] = 5; mage.attributes[5] = 4; mage.attributes[0] = 3
mage.inventory[6] = MR.ITEM.POWER_FOCUS; mage.phase[6] = 3
check(MR.item18_skill(mage) == 8, "0x55F02: sorcery + first power focus phase")
mage.inventory[7] = MR.ITEM.POWER_FOCUS; mage.phase[7] = 9; check(MR.item18_skill(mage) == 8, "power focus scan is slots 6..0 (slot 7 = totem)")
mage.inventory[7] = 0
local a, b = MR.split(8, 2); check(a == 4 and b == 4, "posture 2 -> 50%"); a = MR.split(7, 1); check(a == 1, "7*25//100 = 1")
local cp = runner(); cp.attributes[1] = 3; cp.attributes[4] = 2; cp.attributes[5] = 4
check(MR.combat_pool(cp) == 4, "0x55ED6: (Q+I+W)>>1"); cp.inventory[0] = MR.ITEM.COMBAT_SENSE; cp.phase[0] = 2
check(MR.combat_pool(cp) == 6, "+ first Combat Sense phase")
local tbl = { entry_count = 12, entries = {} }; for k = 0, 11 do tbl.entries[k + 1] = sp(k, 6) end
check(MR.success_value(mage, tbl, 0, 0, 3) == 4 + 3, "success = min(posture remainder, magic) + force")
mage.inventory[7] = MR.ITEM.GATOR; check(MR.success_value(mage, tbl, 0, 0, 3) == 9, "Gator +2 on category-1 spell")
check(MR.success_value(mage, tbl, 1, 5, 3) == 7, "metric index row*6+column = 11 (category 2): Gator +0")
mage.inventory[7] = MR.ITEM.RAT; mage.inventory[2] = MR.ITEM.SPELL_FOCUS_BASE + 0; mage.phase[2] = 2
check(MR.success_value(mage, tbl, 0, 0, 3) == 4 + 3 + 2 - 1, "spell focus +2, Rat -1 on category 1")
-- effect: tn = ceil(f/2) + diff (min 2); pct = (7-tn)*20 (10 at tn>=7); rolls = posture remainder + Willpower
check(MR.effect_value(mage, { difficulty_modifier = 3, base_effect = 6 }, 6) == 5, "effect: tn 6 -> 8*20 = 1.6 -> 2 -> //2 = 1")
check(MR.effect_value(mage, { difficulty_modifier = 0, base_effect = 1 }, 1) == -3, "effect: tn floor 2 -> 8*100 -> 8 -> //2 = 4")
check(MR.effect_value(mage, { difficulty_modifier = 4, base_effect = 5 }, 8) == 5, "effect: tn>=7 -> 10% -> 0.8 -> 1 -> //2 = 0")
check(MR.bar_row(0) == 0 and MR.bar_row(5) == 4, "bar row")
local df = MR.defense({ [0] = mage, [1] = runner() }, { [0] = true, [1] = true }, mage)
check(df.pool == 2 and df.physical == 5 and df.mana == 6, "defense pool 8//4 = 2, non-casters skipped")
local m7 = runner({ archetype = 1 }); m7.skills[0] = 7
check(MR.defense({ [0] = m7 }, { [0] = true }, m7).pool == 2, "7/4 rem 3 -> +1")
local pv = MR.presentation(mage, { [0] = mage }, { [0] = true }, tbl, tbl.entries[3], 0, 2, 4)
check(pv.table_entry_index == 2 and pv.spell_code == 2 and pv.maximum_force == 6 and pv.force == 4, "presentation view fields")
check(MR.can_cast(runner({ stun = 1 })) == false and MR.can_cast(runner()), "stun 1 cannot cast")
end)
-- species max table equals ROM 0xC22A6 [C]
section('species max table equals ROM 0xC22A6', function()
do local d = ROM; if d then
    local ok = true; for sp = 0, 4 do for i = 1, 6 do if AT.MAX[sp][i] ~= d:byte(0xC22A6 + sp * 6 + i) then ok = false end end end
    check(ok, "species max table == ROM 0xC22A6")
    local okc = true; for i = 0, 13 do local b = d:byte(0xC8AE0 + i + 1); if b >= 128 then b = b - 256 end; if MR.CATEGORY[i] ~= b then okc = false end end
    check(okc, "totem category table == ROM 0xC8AE0")
    local okt = true; for code = 0x29, 0x2B do for c = 0, 2 do local o = 0xD4540 + (code - 1) * 0x30 + 0x22 + c * 2 + 1
        local w = d:byte(o) * 256 + d:byte(o + 1); if w >= 0x8000 then w = w - 0x10000 end; if MR.TOTEM_MOD[code][c] ~= w then okt = false end end end
    check(okt, "totem modifiers == ROM")
    local oks = true; for i = 0, 13 do local o = 0xD4EA0 + i * 0x30 + 1; if MR.SPELL[i][1] ~= d:byte(o + 0x24) or MR.SPELL[i][2] ~= d:byte(o + 0x26) then oks = false end end
    check(oks, "spell drain level/mod == ROM 0xD4EA0") end end
end)
-- deck rules + sub-screens (0x19238, 0xE2DFE) [C]
section('deck rules + sub-screens (0x19238, 0x', function()
local DR = require("menu.rules.deck")
do local d = ROM; if d then local ok = true
    for i = 0, 11 do if DR.FOOTPRINT[i] ~= d:byte(0xE2DF2 + i + 1) then ok = false end end; check(ok, "footprints == ROM 0xE2DF2") end end
check(DR.size(1, 3) == 36 and DR.load_steps(36, 5) == 6 and DR.load_steps(10, 5) == 2 and DR.load_steps(11, 5) == 3, "size r^2*fp, load ceil clamp 6")
check(DR.run_info(-1, -1, 0) == "ACTIVE" and select(2, DR.run_info(-1, -1, 0)) == 2 and select(2, DR.run_info(-1, 0, -5)) == 1
    and select(2, DR.run_info(-1, 0, 0)) == 0 and select(2, DR.run_info(-1, 0, 2)) == 3 and DR.run_info(1, 0, 0) == "COMPLETE", "run info 0xE44C8 index")
local function deckp(o, col, row) local r, l, g = deck(o); for _ = 1, col do r:frame(P.RIGHT) end; for _ = 1, row do r:frame(P.DOWN) end; r:frame(P.A); return r, l, g end
local function rec(rt, act) local R0 = { memory = 40, storage = 500, load_io = 3, ratings = zeros(12), active = act or { [0] = -1, -1, -1, -1, -1 }, files = {} }
    for k = 0, 4 do R0.files[k] = { id = -1, size = 0 } end; for p, v in pairs(rt) do R0.ratings[p] = v end; return R0 end
local rp, lp, gp = deckp({ rec = rec({ [5] = 2 }) }, 1, 1)
check(gp.rec.active[0] == 5 and gp.dslots[0].timer == 60 and gp.dslots[0].status == 0 and top(rp) == S.CYBERDECK, "load p5 r2: 16 Mp, ceil(16/3)=6 -> timer 60")
rp, lp, gp = deckp({ rec = rec({ [5] = 4 }) }, 1, 1); check(st(rp).text == "No available memory!" and gp.rec.active[0] == -1, "64 Mp > 40 -> no memory")
rp, lp, gp = deckp({ rec = rec({ [5] = 1 }, { [0] = 1, 2, 3, 4, 6 }) }, 1, 1); check(st(rp).text == "No available slots!", "no free slot")
rp, lp, gp = deckp({ rec = rec({ [5] = 1 }, { [0] = -1, -1, 5, -1, -1 }) }, 1, 1)
check(top(rp) == S.DECK_PROGRAM and st(rp).choice == 1, "loaded -> erase prompt, default No"); rp:frame(P.A); check(gp.rec.active[2] == 5, "No keeps it")
rp:frame(P.A); rp:frame(P.UP); rp:frame(P.C); check(gp.rec.active[2] == -1 and gp.dslots[2].status == 6 and gp.unloaded == 5, "Yes erases, status 6, effect clear")
local fr = rec({}); fr.files[1] = { id = 2, size = 9 }; fr.files[3] = { id = 0, size = 4 }
check(DR.free_storage(fr) == 487 and DR.next_file(fr, 4) == 3 and DR.next_file(fr, 2) == 1 and DR.next_file(fr, 0) == 3, "storage free + circular cursor")
local full_storage = rec({}); full_storage.storage = 20
full_storage.files[0] = { id = 45, size = 20 }
check(DR.free_storage(full_storage) == 0 and
      not DR.storage_fits_file(full_storage, 20) and
      DR.storage_fits_file(full_storage, 0),
      "storage file check uses the ROM signed word subtraction")
local rs, ls, gs = deck({ rec = fr }); rs:frame(P.UP); rs:frame(P.RIGHT); rs:frame(P.A); check(top(rs) == S.DECK_STORAGE, "storage")
rs:frame(P.B); check(top(rs) == S.DECK_STORAGE, "storage: B ignored (A only)")
rs:frame(P.DOWN); rs:frame(P.A); check(st(rs).file == 3 and st(rs).choice == 0, "erase prompt file 3, default Yes")
rs:frame(P.DOWN); rs:frame(P.A); check(st(rs).file == 1 and st(rs).choice == 0, "No -> next file, Yes again"); rs:frame(P.A)
check(fr.files[1].id == -1 and fr.files[1].size == 0 and top(rs) == S.CYBERDECK and st(rs).text == DR.FILE_NAMES[0] .. "  4 Mp", "Yes erases -> remaining file list")
local ri, li, gi = deck({ run = 0 }); ri:frame(P.UP); ri:frame(P.A); ri:frame(P.DOWN); ri:frame(P.A); check(st(ri).matrix_run_info.kind == "no_run" and st(ri).matrix_run_info.message == "You are not on|a matrix run!!", "run info: not on a run keeps the ROM row break")
local rm, lm, gm = deck({ online = true, prev = 4 }); rm:frame(P.UP); rm:frame(P.LEFT); rm:frame(P.LEFT); rm:frame(P.A); check(top(rm) == S.DECK_MAP, "online col 2 -> map/retreat")
rm:frame(P.A); rm:frame(P.B); check(top(rm) == S.DECK_MAP and st(rm).phase == "map", "map waits for Start only"); rm:frame(P.START); check(top(rm) == S.CYBERDECK, "map -> back to grid")
rm:frame(P.A); rm:frame(P.DOWN); rm:frame(P.A); rm:frame(P.C); check(gm.retreated and top(rm) == S.POCKET_SECRETARY, "retreat: swap nodes, FFF0AD exits deck")
rm = deck({ online = true, combat = true }); rm:frame(P.UP); rm:frame(P.LEFT); rm:frame(P.LEFT); rm:frame(P.A); rm:frame(P.DOWN); rm:frame(P.A)
check(st(rm).message == MM.RETREAT_COMBAT, "retreat in combat"); rm:frame(P.A); check(top(rm) == S.CYBERDECK, "-> grid")
rm = deck({ online = true }); rm:frame(P.UP); rm:frame(P.LEFT); rm:frame(P.LEFT); rm:frame(P.A); rm:frame(P.DOWN); rm:frame(P.A)
check(top(rm) == S.CYBERDECK and st(rm).text == "Retreat is not possible from this node!", "no previous node: text, no wait")
end)
-- matrix + save rules (2026-10-15) [C]
-- (save rules were parked with menu/rules/save.lua: srx uses host JSON saves)
section('matrix rules (2026-10-15) [C]', function()
do local d = ROM; if d then local ok = true
    for c = 0, 26 do if MXr.IC_TABLE[c][1] ~= d:byte(0xE31A7 + 2 * c + 1) or MXr.IC_TABLE[c][2] ~= d:byte(0xE31A7 + 2 * c + 2) then ok = false end end
    check(ok, "IC table == ROM 0xE31A7")
    local effect_records_ok = true
    for mode = 0, 8 do
        local record = string.unpack(">I4", d, 0x1A9CE + mode * 4 + 1)
        local definition = MatrixWorldEffectModes[mode]
        if not definition or definition.cadence ~= d:byte(record + 1) or
           definition.phase_count ~= d:byte(record + 2) then
            effect_records_ok = false
        end
    end
    check(effect_records_ok and
          MatrixWorldEffectModes.default.cadence == d:byte(0x1AA10 + 1) and
          MatrixWorldEffectModes.default.phase_count == d:byte(0x1AA10 + 2),
          "Matrix traversal effect cadence and phase counts == ROM records")
    local function rs(a, set) local ok2 = true; for i = 0, 6 do if not set[d:byte(a + i + 1)] then ok2 = false end end; return ok2 end
    check(rs(0x15FC9A, MXr.CONNECT_LEFT) and rs(0x15FCA1, MXr.CONNECT_RIGHT) and rs(0x15FCA8, MXr.CONNECT_UP) and rs(0x15FCAF, MXr.CONNECT_DOWN), "connector sets == ROM") end end
local nd = { ic_condition = 10, ic_code = 13, ic_rating = 4, ic_rating2 = 7 }
check(MXr.ic_type(nd, MXr.NONE, false, 1) == 0 and MXr.ic_type(nd, MXr.ACTIVE, false, 1) == 8, "two-layer IC: Access then Black at active alert")
check(MXr.ic_type(nd, MXr.NONE, false, 0) == MXr.UNSEEN and MXr.ic_type(nd, MXr.NONE, true, 0) == 0, "unseen unless FFDF84 < 0")
check(MXr.ic_type({ ic_condition = 0, ic_code = 1 }, 0, true, 1) == 0x45 and MXr.ic_type({ ic_condition = 5, ic_code = 0xFF }, 0, true, 1) == 0x45, "0x45 = no live IC")
check(MXr.ic_type({ ic_condition = 5, ic_code = 26 }, MXr.ACTIVE, true, 0) == 8 and MXr.ic_type({ ic_condition = 5, ic_code = -1 }, MXr.ACTIVE, true, 1) == 0x45, "upper ROM IC code and signed no-IC sentinel")
check(MXr.ic_rating(nd, MXr.NONE) == 4 and MXr.ic_rating(nd, MXr.PASSIVE) == 5 and MXr.ic_rating(nd, MXr.ACTIVE) == 9, "IC rating + alert bonus, layer 2")
local a, ov = MXr.raise_alert(MXr.ACTIVE, 1, true); local a2 = MXr.raise_alert(MXr.NONE, -1, false)
check(a == 1 and ov and a2 == -1 and MXr.raise_alert(-1, 1, true) == 0, "alert clamp + overload")
local mp = {}; for i = 0, 139 do mp[i] = 0 end; mp[16] = 12; mp[17] = 3; mp[30] = 13; mp[44] = 5
local kn = MXr.reveal(mp, {}, 1, 1)   -- cur 15 (node 0) -> right 16 connector (12 connects left) -> 17 node: own 12 connects right
check(kn[15] and kn[16] and kn[17] and not kn[29], "map reveal flood")
local act, run = { [0] = 1, 5, -1, 1, -1 }, { [0] = 6, 6, 0, 6, 0 }
check(MXr.cycle_slot(0, -1, act, run) == 4 and run[0] == 1, "slot cycle wraps, resets state>=6")
check(MXr.cycle_slot(1, 1, act, run) == 2 and run[1] == 6, "persistent program 5 keeps state")
check(MXr.matrix_delay(3, 2) == 9 and MXr.LOGOUT_FRAMES == 54, "delay + logout frames")
local MatrixSession = require("menu.matrix_session")
local started, hidden_state = MatrixSession.begin(16, false)
if started then
    hidden_state.current_node = 29
    local view = MatrixSession.traversal_view(
        hidden_state, "Analyze-2", "Resisted!!")
    check(view.world_model_id == 2 and view.palette_theme == 2 and
          view.world_effect_mode == MXr.UNSEEN and
          view.world_effect_phase == 0 and
          view.action_difficulty == -1 and
          view.action_difficulty_label == "" and
          view.ice_label == " Ice Unknown  " and
          view.active_program_label == "Analyze-2" and
          view.active_program_status_label == "Resisted!!",
          "unknown Matrix node keeps its model and active deck HUD without a difficulty glyph")
    local fallback_advanced = false
    for _ = 1, MatrixWorldEffectModes.default.cadence do
        fallback_advanced = MatrixSession.advance_world_effect_frame(
            hidden_state)
    end
    check(fallback_advanced and
          MatrixSession.traversal_view(hidden_state).world_effect_phase == 1,
          "unseen Matrix node uses the ROM default effect cadence")
    MatrixSession.set_current_node_knowledge(hidden_state, 4)
    check(MatrixSession.traversal_view(hidden_state).world_effect_mode == 0,
          "known System 16 node selects its normal IC effect mode")
    hidden_state.alert = MXr.ACTIVE
    local active_view = MatrixSession.traversal_view(hidden_state)
    local cadence_advanced = false
    for _ = 1, MatrixWorldEffectModes[2].cadence do
        cadence_advanced = MatrixSession.advance_world_effect_frame(hidden_state)
    end
    local advanced_view = MatrixSession.traversal_view(hidden_state)
    check(active_view.world_effect_mode == 2 and cadence_advanced and
          advanced_view.world_effect_mode == 2 and
          advanced_view.world_effect_phase == 1,
          "active alert selects the second IC effect layer and advances on its ROM cadence")
    MatrixSession.finish(hidden_state)
else
    check(false, "unknown Matrix node fixture loads from ROM data")
end
local entry_started, entry_state = MatrixSession.begin(2, false, 2, 0)
if entry_started then
    MatrixSession.set_current_node_knowledge(entry_state, 4)
    local began = MatrixSession.begin_world_entry(entry_state)
    local entry_view = MatrixSession.traversal_view(entry_state)
    for _ = 1, 61 do
        MatrixSession.advance_world_entry(entry_state)
    end
    local setup_complete = MatrixSession.traversal_view(entry_state)
    for _ = 1, 40 do
        MatrixSession.advance_world_entry(entry_state)
    end
    local first_reference = MatrixSession.traversal_view(entry_state)
    for _ = 41, 239 do
        MatrixSession.advance_world_entry(entry_state)
    end
    local settled = MatrixSession.traversal_view(entry_state)
    check(began and entry_view.world_entry_phase == 0 and
          entry_view.world_entry_sequence_duration == 239 and
          entry_view.ice_label == "Ice Suppressed" and
          entry_view.alert_band_label == " Green-" and
          entry_view.action_difficulty_label == "3" and
          setup_complete.world_entry_phase == 1 and
          setup_complete.world_entry_setup_vblanks == 61 and
          setup_complete.world_entry_sequence_vblanks == 0 and
          first_reference.world_entry_sequence_vblanks == 40 and
          first_reference.world_entry_published_frame == 11 and
          first_reference.model_canvas_width > 0 and
          first_reference.model_canvas_height > 0 and
          settled.world_entry_phase == 2 and
          settled.world_entry_sequence_vblanks == 239 and
          settled.world_entry_published_frame == 56 and
          settled.model_canvas_x == 115 and
          settled.model_canvas_y == 33 and
          settled.model_canvas_width == 88 and
          settled.model_canvas_height == 73,
          "System 2 SPU entry follows 61 setup VBlanks, the 239-tick compact stream, and the calibrated longplay projection")
    MatrixSession.finish(entry_state)
else
    check(false, "System 2 SPU entry fixture loads from ROM data")
end
end)
-- matrix combat + movement (2026-10-15) [C]
section('matrix combat + movement (2026-10-15)', function()
local MC, MV = require("menu.rules.matrix_combat"), require("menu.rules.matrix_move")
do local d = ROM; if d then
    local ok = MXr.TILE_A[0][1] == (d:byte(0x15C3CC + 1) << 8 | d:byte(0x15C3CC + 2)) and MXr.TILE_A[22][4] == 0x18C7 and MXr.TILE_B[11][1] == 0x00D8
    check(ok, "map tile tables == ROM") end end
local dk = { mpcp = 6, hardening = 2, response = 2, bod = 3, evasion = 4, masking = 5, sensor = 4, persona = 10, ratings = zeros(12) }
dk.ratings[0] = 4; dk.ratings[1] = 3
local dc = { d81c = 3, computer = 5, physical = 10 }   -- base = 3 + 5 + 4 = 12
check(MC.pool(dk, dc, -1, false) == 8 and MC.pool(dk, dc, -1, true) == 4 and MC.pool(dk, dc, 1, false) == 4
    and MC.pool(dk, dc, 1, true) == 6 and MC.pool(dk, dc, 0, false) == 6, "pool split A/C/B, defense clamp MPCP")
dc.d81c = 4; check(MC.pool(dk, dc, 0, true) == 6 and MC.pool(dk, dc, 0, false) == 6, "B odd base: defense +1 then MPCP clamp"); dc.d81c = 3
local stx = { alert = -1, mode = -1, mirrors = 0, smoke = 1, shield = 0, karma_counter = 1, passive = 0 }
local nd2 = { colour = 2, security = 3, ic_condition = 10, ic_reaction = 6, ic_rating = 4, ic_rating2 = 0, ic_code = 2 }
local seen = {}; local function rl(q) return function(dice, tn) seen[#seen + 1] = { dice, tn }; return table.remove(q, 1) end end
seen = {}; local s1 = MC.decker_test(dk, dc, nd2, stx, 0, rl({ 3 }))
check(s1 == 3 and seen[1][1] == 7 and seen[1][2] == 5, "decker test: min(8,4)+4-1 = 7 dice, TN 3+2*smoke")
local net, state = MC.run_program(dk, dc, nd2, stx, 0, rl({ 0 })); check(net == -1 and state == 7, "no successes -> state 7")
net, state = MC.run_program(dk, dc, nd2, stx, 0, rl({ 2, 3 })); check(net == 0 and state == 8, "IC blocks -> 8")
local party = { [0] = { karma = 1 }, [1] = { karma = 255 } }
check(not MC.attack(nd2, stx, 4, party) and nd2.ic_condition == 6, "attack -net")
check(MC.attack(nd2, stx, 9, party) and nd2.ic_condition == 0 and stx.karma_counter == 15 and party[0].karma == 2 and party[1].karma == 0, "IC crash: karma every 8th")
MC.slow(dk, nd2, stx, 1, party); check(nd2.ic_reaction == 3, "slow: 2*net + rating/2")
MC.degrade(nd2, 5); check(nd2.security == 2, "degrade floor 2")
check(MC.ic_kind(1, 1) == "passive" and MC.ic_kind(4, -1) == "attack" and MC.ic_kind(8, -1) == "passive" and MC.ic_kind(8, 1) == "trace"
    and MC.ic_kind(13, 1) == "attack" and MC.ic_kind(18, 1) == "passive", "IC kind by code/alert")
check(MC.ic_interval({ ic_reaction = 7 }, 1) == 360, "IC interval (0x1B-9)*20")
local v, t, e = MC.tick_shroud(1, 0); check(v == 0 and t == 0xF0 and e, "shroud tick expires")
local nd3 = { colour = 1, security = 4, ic_condition = 10, ic_reaction = 5, ic_rating = 5, ic_rating2 = 0, ic_code = 3 }  -- Blaster
local dk2 = { mpcp = 4, hardening = 1, response = 2, bod = 3, evasion = 4, masking = 5, persona = 2, ratings = zeros(12) }
local st3 = { alert = -1, mode = 0, mirrors = 0, smoke = 0, shield = 1, karma_counter = 5, passive = 0 }
local ev = MC.ic_attack(dk2, dc, nd3, st3, rl({ 4, 0, 3 }))
check(ev.persona == -2 and dk2.persona == 0 and ev.mpcp == 2 and dk2.mpcp == 2 and dk2.response == 1 and st3.shield_gone, "IC hit: shield absorbs 1, persona 0, Blaster burns MPCP")
check(MC.crashed(dk2, dc), "persona 0 -> crash")
local st4 = { alert = -1, mode = 0, mirrors = 0, smoke = 0, passive = 0 }
check(MC.passive_step(dk, dc, nd2, st4, rl({})) == "probe" and st4.passive == -1, "passive: probe first")
local r4 = MC.passive_step(dk, dc, nd2, st4, rl({ 3, 1 })); check(r4.raised and st4.alert == 0 and not r4.reencounter, "passive: alert none -> passive")
local st5 = { alert = 1, mirrors = 1, smoke = 0, trace_on = false }; seen = {}
local tr, sp = MC.trace_step(dk, { ic_rating = 2, ic_rating2 = 3, ic_code = 7 }, st5, rl({ 3 }))
check(tr == "started" and st5.trace_left == 8 and seen[1][1] == 5 and seen[1][2] == 5 and sp == 0x2800 // (8 * 0xF0), "trace: layer-2 rating +2, TN evasion+mirrors")
local lk = { [0] = 4, 9, -1, -1, -1, -1, -1, -1 }
check(MV.link_right(lk, 1) == 0 and MV.link_left(lk, 0) == 1 and MV.link_left(lk, 1) == 0, "link cursor wrap")
local nodes = { [0] = { fn = 0, links = { [0] = 1, -1, -1, -1, -1, -1, -1, -1 } }, [1] = { fn = 1, links = { [0] = 0, 2, -1, -1, -1, -1, -1, -1 } }, [2] = { fn = 4, links = { [0] = 1, -1, -1, -1, -1, -1, -1, -1 } }, [3] = { fn = -1 } }
check(MV.goto_right(nodes, 1, 0) == 2 and MV.goto_right(nodes, 1, 2) == 0 and MV.goto_left(nodes, 0, 0) == 2, "go-to-node cursor skips current")
local c, pv = MV.goto_commit(nodes, 1, 2); local c2, pv2 = MV.goto_commit(nodes, 0, 2)
check(c == 2 and pv == 1 and c2 == 2 and pv2 == -1, "jump: retreat only via a link")
end)
print(string.format("submenu tests: %d/%d passed", n - fails, n)); os.exit(fails == 0 and 0 or 1)
