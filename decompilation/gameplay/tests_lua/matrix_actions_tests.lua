-- lua5.4 decompilation/gameplay/tests_lua/matrix_actions_tests.lua (from the repository root): node actions 0x54AEA..0x5561E, programs, trace bar [C ROM].
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path
local NA = require("menu.rules.node_actions"); local PR = require("menu.rules.matrix_programs")
local DECK = require("menu.rules.deck")
local E = require("enums")
local n, f = 0, 0
local function ok(c, m) n = n + 1; if not c then f = f + 1; print("FAIL " .. m) end end
local st = { alert = 0 }
ok(NA.cancel_alert(st).redraw and st.alert == -1, "cancel"); ok(NA.cancel_alert(st).text == NA.TEXT.NO_ALERTS, "no alerts")
local run = {
    status = E.MatrixRunStatus.ACTIVE,
    objective = { node = "cpu", action = E.MatrixRunAction.CRASH_SYSTEM },
    file_size = 0, completion_marker_present = true,
    completion_recorded = false,
}; local field = { alarm = true }
local r = NA.crash_system(run, field)
ok(run.status == E.MatrixRunStatus.COMPLETED and
   not run.completion_marker_present and run.completion_recorded and
   field.security_off and field.maglocks_off and r.logout, "crash")
local a = {
    { actor_facts = { roles = {
        spotted = false, reinforcement = true,
        despawn_offscreen = false, quest = false, runner = false,
        pedestrian = false, hostile = false,
    } } },
    { actor_facts = { roles = {
        spotted = false, reinforcement = false,
        despawn_offscreen = false, quest = false, runner = false,
        pedestrian = false, hostile = false,
    } } },
}; field = { alarm = true }
NA.turn_off_node({ file_class = "Security Cntrl" }, field, a)
ok(not field.alarm and field.security_off and
   a[1].actor_facts.roles.despawn_offscreen and
   not a[2].actor_facts.roles.despawn_offscreen, "turnoff 27")
field = {}; NA.turn_off_node({ file_class = "Maglocks" }, field); ok(field.maglocks_off and not field.security_off, "turnoff maglocks")
run = {
    status = E.MatrixRunStatus.ACTIVE,
    objective = { node = "datastore", action = E.MatrixRunAction.DELETE_FILE,
                  file_class = "Security Files", file_id = 0 },
    file_size = 30, completion_marker_present = true,
    completion_recorded = false,
}
local run_node = { class = 5, file_class = "Security Files" }
ok(NA.erase(run, run_node).text == NA.TEXT.ERASE_OBJECTIVE and
   run.status == E.MatrixRunStatus.COMPLETED, "erase objective")
ok(NA.erase(run, run_node).text == NA.TEXT.ERASED, "erase plain")
local seq; local function rand(k) return table.remove(seq, 1) % k end
seq = { 9 }
ok(NA.search({ status = E.MatrixRunStatus.INACTIVE }, { class = 2 },
    { int = 3, computer = 3 }, { sensor = 3 }, nil, rand).text ==
    NA.TEXT.NOTHING, "nothing")
seq = { 0, 3, 2 }
local s = NA.search({ status = E.MatrixRunStatus.INACTIVE }, { class = 2 },
    { int = 3, computer = 3 }, { sensor = 3 }, nil, rand)
ok(s.file and s.file.id == 13 and s.file.size == 40, "found")
local rec = { files = {} }; for i = 0, 4 do rec.files[i] = { id = -1, size = 0, data_value = 0, run_objective = false } end
local fs = DECK.free_storage; DECK.free_storage = function() return 100 end
seq = { 6, 3 }; local k = NA.store(rec, {}, { class = 2, colour = 4 }, s.file, rand)
ok(k == 0 and rec.files[0].data_value == 40 and rec.files[0].id == 13, "store red: table[9]=8 * 5")
DECK.free_storage = function() return 10 end; local k2, t2 = NA.store(rec, {}, {}, s.file, rand); ok(k2 == nil and t2 == NA.TEXT.NO_STORAGE, "no storage (sr-refactor API: nil, text 0xE383D)")
DECK.free_storage = fs
local deck = { persona = 4, ratings = { [4] = 3 } }; local tn0
ok(PR.medic(deck, function(d, tn) tn0 = tn; return 3 end).healed == 3 and deck.persona == 7 and tn0 == 6, "medic")
deck.persona = 9; PR.medic(deck, function(d, tn) tn0 = tn; return 5 end); ok(deck.persona == 10 and tn0 == 4, "medic cap")
local node = { ic_condition = 10 }; st = { rebound = 3, karma_counter = 5 }
ok(PR.rebound_hit(node, st, 2, {}) == "reflected" and st.rebound == 2 and node.ic_condition == 9, "rebound >")
ok(PR.rebound_hit(node, st, 2, {}) == "reflected_unload" and st.rebound == 0, "rebound ==")
ok(PR.rebound_hit(node, st, 2, {}) == "failed_unload", "rebound <")
ok(PR.sleaze(nil, nil, nil, {}, nil, true).state == 11, "sleaze refused")
local bar = { state = 2, pos = 0x7100, speed = 0x50, type = 7 }
ok(PR.trace_frame(bar, {}) == "burn_dump", "trace A"); bar.type = 6; bar.pos = 0
ok(PR.trace_frame(bar, {}) == "running", "trace B")
st = { trace_rewind = true }; ok(PR.trace_frame(bar, st, 0x1B00) == "cancelled" and not st.trace_rewind, "rewind")
deck = { mpcp = 3, hardening = 1, response = 2 }
ok(PR.burn(deck, { ic_code = 7, ic_rating = 4, ic_rating2 = 4 }, { alert = 0 }, function() return 5 end) == 4 and deck.mpcp == 0 and deck.response == 0, "burn")
do -- Analyze returns a capped semantic discovery level.
  local PRm = require("menu.rules.matrix_combat"); local dt = PRm.decker_test
  PRm.decker_test = function() return 7 end
  local r2 = PR.analyze({ sensor = 1 }, {}, { security = 4 }, {}, function(d, tn) ok(d == 4 and tn == 2, "analyze roll"); return 1 end)
  ok(r2.net == 6 and r2.analysis_level == 4, "analyze semantic result")
  PRm.decker_test = dt
end
do local a2, b2 = PR.dump_jitter(0x8593); ok(a2 == -19 and b2 == -5, "dump jitter") end
do
  local objective_run = {
    status = E.MatrixRunStatus.ACTIVE,
    objective = { node = "datastore", action = E.MatrixRunAction.DOWNLOAD_FILE,
                  file_class = "Project Files", file_id = 3 },
    file_size = 20, completion_marker_present = true,
    completion_recorded = false,
  }
  local s2 = NA.search(objective_run,
      { class = 7, file_class = "Project Files" }, {}, {}, nil, rand)
  ok(s2.file.run_objective and s2.file.id == 3 and s2.file.size == 20,
     "semantic objective file preview")
  local objective_record = { storage = 100, files = {}, ratings = {} }
  for i = 0, 4 do objective_record.files[i] = { id = -1, size = 0, data_value = 0 } end
  for i = 0, 11 do objective_record.ratings[i] = 0 end
  local slot = NA.store(objective_record, objective_run,
      { class = 7, colour = 2 }, s2.file, rand)
  ok(slot == 0 and objective_record.files[0].run_objective and
     objective_run.status == E.MatrixRunStatus.COMPLETED,
     "storing an objective uses named completion facts")
end
local inactive_preview = NA.begin_search({
    status = E.MatrixRunStatus.INACTIVE,
    objective = { node = "datastore", action = E.MatrixRunAction.DOWNLOAD_FILE,
                  file_class = "Project Files", file_id = 3 },
    file_size = 20,
}, { class = 7, file_class = "Project Files" })
ok(inactive_preview == nil, "inactive run does not expose its target file")
print(("%d/%d"):format(n - f, n))
do local st3 = { attempts = 0 }
  local okg = NA.gate(4, 3, { colour = 2, security = 5 }, st3, function(d, tn) assert(d == 5 and tn == 5); return 1 end)
  print(okg and st3.attempts == 1 and "gate ok" or "FAIL gate")
  print((not NA.random_alert(st3, function() return 1 end)) and NA.random_alert(st3, function() return 0 end) and st3.attempts == 0 and "ralert ok" or "FAIL ralert") end
