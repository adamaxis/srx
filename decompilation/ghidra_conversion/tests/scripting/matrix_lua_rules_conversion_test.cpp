extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

std::filesystem::path g_script_root;

int load_test_module(lua_State* state) {
    const char* module_name = luaL_checkstring(state, 1);
    const std::string name(module_name);

    lua_getglobal(state, "__matrix_test_modules");
    lua_getfield(state, -1, name.c_str());
    if (!lua_isnil(state, -1)) {
        lua_remove(state, -2);
        return 1;
    }
    lua_pop(state, 2);

    auto relative = name;
    for (auto& character : relative) {
        if (character == '.') {
            character = '/';
        }
    }
    const auto module_path = g_script_root / (relative + ".lua");
    if (luaL_loadfilex(state, module_path.string().c_str(), "t") != LUA_OK ||
        lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return lua_error(state);
    }

    lua_getglobal(state, "__matrix_test_modules");
    lua_pushvalue(state, -2);
    lua_setfield(state, -2, name.c_str());
    lua_pop(state, 1);
    return 1;
}

// Returns gameplay/<relative> file contents (dialogue JSON content).
int read_gameplay_file(lua_State* state) {
    const std::string relative(luaL_checkstring(state, 1));
    std::ifstream file(g_script_root / ".." / relative, std::ios::binary);
    if (!file) {
        return luaL_error(state, "missing gameplay file %s", relative.c_str());
    }
    const std::string source((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    lua_pushlstring(state, source.data(), source.size());
    return 1;
}

bool run_rules(lua_State* state) {
    constexpr char kRules[] = R"lua(
local E = require("enums")
local MX = require("menu.rules.matrix")
local C = require("menu.rules.matrix_combat")
local D = require("menu.rules.deck")
local Move = require("menu.rules.matrix_move")
local Actions = require("menu.rules.node_actions")
local Programs = require("menu.rules.matrix_programs")
local Systems = require("data.matrix_systems")
local HUD = require("data.matrix_hud_labels")
local Session = require("menu.matrix_session")
local Cyberdeck = require("menu.screens.cyberdeck")
local MatrixNodeAction = require("menu.screens.matrix_node_action")
local DeckPalette = require("menu.deck_palette")
local MatrixText = require("menu.rules.matrix_text")
local SpecialDatastore = require("menu.rules.datastore_special")

local checks = 0
local function eq(actual, expected, label)
    assert(actual == expected,
        (label .. ": expected %s, got %s"):format(tostring(expected), tostring(actual)))
    checks = checks + 1
end

local function matrix_run(status, node, action, file_class, file_id,
                          file_size, target_system_id)
    return {
        status = status,
        objective = {
            node = node,
            action = action,
            file_class = file_class,
            file_id = file_id,
        },
        file_size = file_size or 0,
        completion_marker_present = true,
        completion_recorded = status == E.MatrixRunStatus.COMPLETED,
        target_system_id = target_system_id,
    }
end

eq(Session.program_mode_from_buttons(E.Pad.C), E.MatrixModeResult.ATTACK,
   "ROM C button selects attack pool mode")
eq(Session.program_mode_from_buttons(E.Pad.A), E.MatrixModeResult.BALANCED,
   "ROM A button selects balanced pool mode")
eq(Session.program_mode_from_buttons(E.Pad.B), E.MatrixModeResult.DEFENSE,
   "ROM B button selects defense pool mode")
eq(Session.program_mode_from_buttons(E.Pad.C | E.Pad.A),
   E.MatrixModeResult.ATTACK, "ROM C takes priority over simultaneous A")
eq(Session.program_mode_from_buttons(E.Pad.A | E.Pad.B),
   E.MatrixModeResult.BALANCED, "ROM A takes priority over simultaneous B")

-- Keep these Matrix rule fixtures scoped so later cases can declare locals.
do
local function run_matrix_medic(roll_result)
    local record = {
        active = { [0] = Programs.P.MEDIC, [1] = -1, [2] = -1,
                   [3] = -1, [4] = -1 },
        ratings = { [Programs.P.MEDIC] = 3 },
        persona_condition = 4,
    }
    local slots = {
        [0] = { status = 6, phase = 1, timer = 0 },
        [1] = { status = 6, phase = 1, timer = 0 },
        [2] = { status = 6, phase = 1, timer = 0 },
        [3] = { status = 6, phase = 1, timer = 0 },
        [4] = { status = 6, phase = 1, timer = 0 },
    }
    local state = {
        phase = "node", current_node = 0, selected_program_slot = 0,
        nodes = { [0] = { function_id = 0, file_class_id = 0, colour = 1,
                          security = 3, x = 0, y = 0, ic_condition = 0,
                          ic_code = 0 } },
        alert = MX.NONE, mode = MX.NONE,
    }
    local commits, roll_dice, roll_tn = 0, nil, nil
    local context = {
        game = {
            deck_record = function() return record end,
            deck_slots = function() return slots end,
            deck_commit = function(next_record, next_slots)
                record, slots = next_record, next_slots
                commits = commits + 1
            end,
            roll_successes = function(dice, target)
                roll_dice, roll_tn = dice, target
                return roll_result
            end,
        },
        redraw = function() end,
        sound = function() end,
        host = {},
    }
    local screen = MatrixNodeAction.new({ matrix_state = state })
    MatrixNodeAction.enter(screen, context)
    MatrixNodeAction.input(screen, E.Pad.A, context)
    return record, slots, state, screen, commits, roll_dice, roll_tn
end

local medic_record, medic_slots, medic_state, medic_screen, medic_commits,
      medic_dice, medic_tn = run_matrix_medic(3)
eq(medic_dice, 3, "Medic rolls its loaded program rating")
eq(medic_tn, 6, "Medic target number uses current persona condition")
eq(medic_record.persona_condition, 7,
   "active Matrix Medic commits persona recovery to the deck record")
eq(medic_slots[0].phase, 6,
   "successful Matrix Medic publishes the ROM success status")
eq(medic_state.mode, E.MatrixModeResult.BALANCED,
   "active Matrix Medic routes A through the ROM balanced mode")
eq(medic_commits, 1, "active Matrix Medic commits its changed deck state")

local failed_medic_record, failed_medic_slots, _, _, failed_medic_commits,
      failed_medic_dice, failed_medic_tn = run_matrix_medic(0)
eq(failed_medic_dice, 3, "failed Matrix Medic still rolls the loaded rating")
eq(failed_medic_tn, 6, "failed Matrix Medic preserves the source target")
eq(failed_medic_record.persona_condition, 4,
   "failed Matrix Medic leaves persona condition unchanged")
eq(failed_medic_slots[0].phase, 7,
   "failed Matrix Medic publishes the ROM failure status")
eq(failed_medic_record.active[0], Programs.P.MEDIC,
   "failed Matrix Medic does not unload its program")
eq(failed_medic_commits, 1,
   "failed Matrix Medic commits the updated program status")

local function test_matrix_degrade_route()
local knowledge_state = {
    current_node = 0, nodes = { [0] = { x = 0, y = 0 } },
    map_knowledge = { system_id = 0, cells = {} },
}
eq(Session.set_current_node_analysis_level(knowledge_state, 4), true,
   "node entry sets the current analysis level")
eq(Session.merge_current_node_analysis_level(knowledge_state, 1), true,
   "Analyze merges its result into the current analysis state")
eq(Session.current_node_analysis_level(knowledge_state), 5,
   "Analyze combines successive discovery results")
eq(Session.merge_current_node_analysis_level(knowledge_state, 2), true,
   "later Analyze results continue merging into the same node")
eq(Session.current_node_analysis_level(knowledge_state), 7,
   "combined analysis results preserve their reported level")

local function run_matrix_degrade(roll_results)
    local record = {
        mpcp = 8, response = 2, persona = { [2] = 1 },
        active = { [0] = C.PROGRAM.DEGRADE, [1] = -1, [2] = -1,
                   [3] = -1, [4] = -1 },
        ratings = { [C.PROGRAM.DEGRADE] = 3 },
    }
    local slots = {
        [0] = { status = 6, phase = 1, timer = 0 },
        [1] = { status = 6, phase = 1, timer = 0 },
        [2] = { status = 6, phase = 1, timer = 0 },
        [3] = { status = 6, phase = 1, timer = 0 },
        [4] = { status = 6, phase = 1, timer = 0 },
    }
    local runner = {
        attributes = { [1] = 8, [4] = 4 }, skills = { [7] = 7 },
    }
    local state = {
        phase = "node", current_node = 0, selected_program_slot = 0,
        nodes = { [0] = { function_id = 0, file_class_id = 0, colour = 1,
                          security = 4, x = 0, y = 0, ic_condition = 1,
                          ic_code = 3, ic_rating = 1, ic_rating2 = 1,
                          ic_reaction = 3 } },
        alert = MX.NONE, mode = MX.NONE,
    }
    local calls, rolls, commits = 0, {}, 0
    local context = {
        game = {
            party = function() return { [0] = runner } end,
            active_slot = function() return 0 end,
            deck_record = function() return record end,
            deck_slots = function() return slots end,
            deck_commit = function(next_record, next_slots)
                record, slots = next_record, next_slots
                commits = commits + 1
            end,
            roll_successes = function(dice, target)
                calls = calls + 1
                rolls[calls] = { dice = dice, target = target }
                return roll_results[calls]
            end,
        },
        redraw = function() end,
        sound = function() end,
        host = {},
    }
    local screen = MatrixNodeAction.new({ matrix_state = state })
    MatrixNodeAction.enter(screen, context)
    MatrixNodeAction.input(screen, E.Pad.A, context)
    return record, slots, state, calls, rolls, commits
end

local degrade_record, degrade_slots, degrade_state, degrade_calls,
      degrade_rolls, degrade_commits = run_matrix_degrade({ 4, 1 })
eq(degrade_calls, 2, "Degrade runs decker and IC opposition rolls")
eq(degrade_rolls[1].dice, 6, "Degrade uses the ROM program dice pool")
eq(degrade_rolls[1].target, 4, "Degrade uses node security as its target")
eq(degrade_rolls[2].dice, 1, "Degrade opposes with active IC rating")
eq(degrade_rolls[2].target, 2, "Degrade opposition uses persona Evasion")
eq(degrade_state.mode, E.MatrixModeResult.BALANCED,
   "active Matrix Degrade routes A through the ROM balanced mode")
eq(degrade_state.nodes[0].security, 3,
   "successful Degrade lowers security by half net successes")
eq(degrade_slots[0].phase, C.RESULT.SUCCESS,
   "successful Degrade publishes the ROM success status")
eq(degrade_record.active[0], C.PROGRAM.DEGRADE,
   "successful Degrade keeps the loaded program")
eq(degrade_commits, 1,
   "active Matrix Degrade commits its result status")

local _, blocked_slots, blocked_state, blocked_calls =
    run_matrix_degrade({ 1, 1 })
eq(blocked_calls, 2, "blocked Degrade still resolves IC opposition")
eq(blocked_state.nodes[0].security, 4,
   "blocked Degrade leaves node security unchanged")
eq(blocked_slots[0].phase, C.RESULT.BLOCKED,
   "blocked Degrade publishes the ROM blocked status")

local failed_degrade, failed_slots, failed_state, failed_calls =
    run_matrix_degrade({ 0 })
eq(failed_calls, 1, "failed Degrade skips the IC opposition roll")
eq(failed_state.nodes[0].security, 4,
   "failed Degrade leaves node security unchanged")
eq(failed_slots[0].phase, C.RESULT.FAIL,
   "failed Degrade publishes the ROM failure status")
eq(failed_degrade.active[0], C.PROGRAM.DEGRADE,
   "failed Degrade retains its loaded program")
end
test_matrix_degrade_route()

do
local function run_matrix_combat_program_route(program, condition, reaction,
                                               counter, roll_results)
    local record = {
        mpcp = 8, response = 2, persona = { [2] = 1 },
        active = { [0] = program, [1] = -1, [2] = -1,
                   [3] = -1, [4] = -1 },
        ratings = { [program] = 3 },
    }
    local slots = {
        [0] = { status = 6, phase = 1, timer = 0 },
        [1] = { status = 6, phase = 1, timer = 0 },
        [2] = { status = 6, phase = 1, timer = 0 },
        [3] = { status = 6, phase = 1, timer = 0 },
        [4] = { status = 6, phase = 1, timer = 0 },
    }
    local party = {
        [0] = { karma = 1, attributes = { [1] = 8, [4] = 4 },
                skills = { [7] = 7 } },
        [1] = { karma = 4 },
        [2] = { karma = 255 },
    }
    local state = {
        phase = "node", current_node = 0, selected_program_slot = 0,
        nodes = { [0] = { function_id = 0, file_class_id = 0, colour = 1,
                          security = 4, x = 0, y = 0,
                          ic_condition = condition, ic_reaction = reaction,
                          ic_code = 3, ic_rating = 1, ic_rating2 = 0 } },
        alert = MX.NONE, mode = MX.NONE,
    }
    local world = { karma_counter = counter, passive_ic_counter = -3 }
    local calls, roll_index = 0, 0
    local party_commits, world_commits = 0, 0
    local context = {
        game = {
            party = function() return party end,
            active_slot = function() return 0 end,
            deck_record = function() return record end,
            deck_slots = function() return slots end,
            deck_commit = function(next_record, next_slots)
                record, slots = next_record, next_slots
                calls = calls + 1
            end,
            commit_party = function(next_party)
                party = next_party
                party_commits = party_commits + 1
            end,
            matrix_world_state = function() return world end,
            commit_matrix_world_state = function(next_world)
                world = next_world
                world_commits = world_commits + 1
            end,
            roll_successes = function()
                roll_index = roll_index + 1
                return roll_results[roll_index]
            end,
        },
        redraw = function() end,
        sound = function() end,
        host = {},
    }
    local screen = MatrixNodeAction.new({ matrix_state = state })
    MatrixNodeAction.enter(screen, context)
    MatrixNodeAction.input(screen, E.Pad.A, context)
    return record, slots, state, party, world, calls, party_commits,
           world_commits, roll_index
end

local attack_record, attack_slots, attack_state, attack_party, attack_world,
      attack_commits, attack_party_commits, attack_world_commits,
      attack_rolls = run_matrix_combat_program_route(
          C.PROGRAM.ATTACK, 4, 3, 1, { 5, 0 })
eq(attack_rolls, 2, "Attack runs the common decker and IC tests")
eq(attack_state.nodes[0].ic_condition, 0,
   "ROM Attack clears the current IC condition when net damage exceeds it")
eq(attack_slots[0].phase, C.RESULT.SUCCESS,
   "Attack publishes the ROM success slot phase")
eq(attack_world.karma_counter, 15,
   "defeating IC rolls the shared signed FFF0CE counter")
eq(attack_world.passive_ic_counter, 0,
   "Attack clears the FFF0A3 passive IC probe state")
eq(attack_party[0].karma, 2,
   "FFF0CE rollover awards Karma to runner record zero")
eq(attack_party[1].karma, 5,
   "FFF0CE rollover awards Karma to runner record one")
eq(attack_party[2].karma, 0,
   "FFF0CE rollover wraps Karma byte values")
eq(attack_commits, 1, "Attack commits the result slot")
eq(attack_party_commits, 1, "Attack commits party Karma after rollover")
eq(attack_world_commits, 1, "Attack commits its shared world counter")
eq(attack_record.active[0], C.PROGRAM.ATTACK,
   "Attack keeps its loaded program")

local slow_record, slow_slots, slow_state, slow_party, slow_world,
      slow_commits, slow_party_commits, slow_world_commits, slow_rolls =
    run_matrix_combat_program_route(C.PROGRAM.SLOW, 4, 2, 1, { 1, 0 })
eq(slow_rolls, 2, "Slow runs the common decker and IC tests")
eq(slow_state.nodes[0].ic_reaction, 0,
   "Slow clears IC reaction when its ROM delta crosses zero")
eq(slow_state.nodes[0].ic_condition, 0,
   "Slow crash also clears the current IC condition")
eq(slow_slots[0].phase, C.RESULT.SUCCESS,
   "Slow publishes the ROM success slot phase")
eq(slow_world.karma_counter, 15,
   "Slow crash shares the FFF0CE karma rollover")
eq(slow_world.passive_ic_counter, 0,
   "Slow clears the FFF0A3 passive IC probe state")
eq(slow_party[1].karma, 5,
   "Slow rollover awards Karma to the party")
eq(slow_commits, 1, "Slow commits the result slot")
eq(slow_party_commits, 1, "Slow commits party Karma after rollover")
eq(slow_world_commits, 1, "Slow commits its shared world counter")
eq(slow_record.active[0], C.PROGRAM.SLOW,
   "Slow keeps its loaded program")

local partial_attack_record, _, partial_attack_state, partial_attack_party,
      partial_attack_world, partial_attack_commits,
      partial_attack_party_commits = run_matrix_combat_program_route(
          C.PROGRAM.ATTACK, 4, 3, 7, { 2, 0 })
eq(partial_attack_state.nodes[0].ic_condition, 2,
   "Attack subtracts positive net damage without defeating the IC")
eq(partial_attack_world.karma_counter, 7,
   "nonlethal Attack leaves FFF0CE unchanged")
eq(partial_attack_world.passive_ic_counter, 0,
   "nonlethal Attack still clears FFF0A3")
eq(partial_attack_party[0].karma, 1,
   "nonlethal Attack does not award Karma")
eq(partial_attack_party_commits, 0,
   "nonlethal Attack does not commit an unchanged party")
eq(partial_attack_commits, 1,
   "nonlethal Attack commits its result status")
eq(partial_attack_record.active[0], C.PROGRAM.ATTACK,
   "nonlethal Attack retains its program")

local failed_attack_record, failed_attack_slots, failed_attack_state,
      failed_attack_party, failed_attack_world, failed_attack_commits,
      failed_attack_party_commits, _, failed_attack_rolls =
    run_matrix_combat_program_route(C.PROGRAM.ATTACK, 4, 3, 7, { 0 })
eq(failed_attack_rolls, 1, "failed Attack stops before IC opposition")
eq(failed_attack_slots[0].phase, C.RESULT.FAIL,
   "failed Attack publishes the ROM failure phase")
eq(failed_attack_state.nodes[0].ic_condition, 4,
   "failed Attack leaves IC condition unchanged")
eq(failed_attack_world.karma_counter, 7,
   "failed Attack leaves FFF0CE unchanged")
eq(failed_attack_world.passive_ic_counter, -3,
   "failed Attack does not clear FFF0A3 before the result tail")
eq(failed_attack_party[0].karma, 1,
   "failed Attack does not award Karma")
eq(failed_attack_party_commits, 0,
   "failed Attack leaves party state uncommitted")
eq(failed_attack_commits, 1,
   "failed Attack commits its result status")
eq(failed_attack_record.active[0], C.PROGRAM.ATTACK,
   "failed Attack retains its program")

local partial_slow_record, _, partial_slow_state, partial_slow_party,
      partial_slow_world, partial_slow_commits,
      partial_slow_party_commits = run_matrix_combat_program_route(
          C.PROGRAM.SLOW, 4, 7, 7, { 1, 0 })
eq(partial_slow_state.nodes[0].ic_reaction, 4,
   "Slow subtracts twice net plus half rating when reaction stays nonnegative")
eq(partial_slow_state.nodes[0].ic_condition, 4,
   "noncrashing Slow leaves IC condition unchanged")
eq(partial_slow_world.karma_counter, 7,
   "noncrashing Slow leaves FFF0CE unchanged")
eq(partial_slow_world.passive_ic_counter, 0,
   "noncrashing Slow still clears FFF0A3")
eq(partial_slow_party[0].karma, 1,
   "noncrashing Slow does not award Karma")
eq(partial_slow_party_commits, 0,
   "noncrashing Slow does not commit an unchanged party")
eq(partial_slow_commits, 1,
   "noncrashing Slow commits its result status")
eq(partial_slow_record.active[0], C.PROGRAM.SLOW,
   "noncrashing Slow retains its program")
end

local palette_state = DeckPalette.new()
for frame = 1, 21 do
    local phase, changed = DeckPalette.tick(palette_state)
    eq(phase, ((frame - 1) // 5 + 1) % 4,
       "ROM deck palette phase at frame " .. frame)
    eq(changed, frame == 1 or frame % 5 == 1,
       "ROM deck palette cadence at frame " .. frame)
end

-- Deck grid input uses newly pressed edges, with vertical priority and silent wrap.
local col, row, moved = MX.step_deck_cursor(0, 0, true, true, true, true)
eq(col, 0, "up priority column")
eq(row, 3, "up wraps from first row")
eq(moved, true, "direction moved")
col, row, moved = MX.step_deck_cursor(2, 3, false, true, false, false)
eq(row, 0, "down wraps from last row")
eq(col, 2, "down keeps column")
col, row = MX.step_deck_cursor(0, 1, false, false, true, false)
eq(col, 3, "left wraps from first column")
col, row = MX.step_deck_cursor(3, 1, false, false, false, true)
eq(col, 0, "right wraps from last column")
col, row, moved = MX.step_deck_cursor(2, 1, false, false, false, false)
eq(col, 2, "no edge keeps column")
eq(row, 1, "no edge keeps row")
eq(moved, false, "no edge does not redraw")
eq(E.MatrixDeckMode.ATTACK, 0xFF, "A selects attack pool")
eq(E.MatrixDeckMode.BALANCED, 0, "B selects balanced pool")
eq(E.MatrixDeckMode.DEFENSE, 1, "C selects defense pool")

-- The offline importer writes the complete ROM graph as named Lua records.
-- Every debug-selectable system must initialize from that catalog without
-- keeping ROM addresses in the active runtime.
local node_count = 0
for system_id = 0, 24 do
    local system = Systems[system_id]
    assert(system and system.id == system_id, "missing Matrix system " .. system_id)
    eq(#system.map, 140, "Matrix map cell count " .. system_id)
    assert(system.nodes[0], "Matrix system lacks its access node " .. system_id)
    for _ in pairs(system.nodes) do node_count = node_count + 1 end
end
eq(node_count, 406, "ROM Matrix node catalog count")
for class_id = 0, 46 do
    assert(Session.file_class_name(class_id) ~= nil,
        "ROM Matrix file-class label " .. class_id)
end
eq(Session.file_class_name(33), "Data Routing",
    "Matrix file-class lookup includes extended node records")
eq(HUD.program_states[10], " Resisted!!  ",
    "active program status preserves its ROM cell padding")
eq(HUD.alert_states[2], "Active Alert ",
    "alert status preserves the ROM's trailing space")
eq(HUD.ic_types[3], "  Blaster-    ",
    "secondary IC labels preserve the ROM's fixed-width cells")
eq(HUD.ic_rating_columns[3], 10,
    "IC rating column follows the ROM type-position table")
eq(HUD.program_states[15], "Active Alert ",
    "program-state catalog includes its final ROM record")
eq(HUD.node_functions[5], "SM ",
    "node-function catalog preserves padded abbreviations")
eq(HUD.security_bands[4], "   Red-",
    "security-band catalog includes the red record")
eq(HUD.ic_types[8], "  BlackIce-   ",
    "IC-type catalog includes the final ROM record")
eq(HUD.ic_visibility[2], "Ice Suppressed",
    "IC visibility catalog includes the suppression state")
local routed_spu_state = select(2, Session.begin(2, false, 2))
local routed_spu_status = Session.node_status(routed_spu_state)
eq(routed_spu_status.analysis_level, 0,
    "unvisited Matrix node starts with no IC knowledge")
eq(routed_spu_status.file_class_label, "",
    "unrevealed node hides its file-class label")
eq(routed_spu_status.action_difficulty, -1,
    "unrevealed node hides its security rating")
eq(Session.traversal_view(routed_spu_state).palette_theme, 0,
    "ROM keeps an unrevealed world model on neutral palette row zero")
eq(Session.set_current_node_analysis_level(routed_spu_state, 4), true,
    "node-action entry sets the source analysis level")
routed_spu_status = Session.node_status(routed_spu_state)
eq(routed_spu_status.file_class_label, " Data Routing ",
    "system 2 SPU renders its padded ROM file-class label")
eq(routed_spu_status.action_difficulty, Systems[2].nodes[2].security,
    "rank-four node displays its ROM security rating")
eq(Session.traversal_view(routed_spu_state).palette_theme,
    Systems[2].nodes[2].security_color,
    "known world model selects its ROM security-color palette row")
do
local active_spu_state = select(2, Session.begin(15, false, 13))
active_spu_state.alert = MX.PASSIVE
active_spu_state.attempts = 1
local alert_deck = { active = { [0] = -1, [1] = -1, [2] = -1,
                                [3] = -1, [4] = -1 }, ratings = {} }
local alert_slots = {
    [0] = { status = 0, phase = 0 }, [1] = { status = 0, phase = 0 },
    [2] = { status = 0, phase = 0 }, [3] = { status = 0, phase = 0 },
    [4] = { status = 0, phase = 0 },
}
local alert_context = {
    game = {
        deck_record = function() return alert_deck end,
        deck_slots = function() return alert_slots end,
        deck_commit = function(record, slots)
            alert_deck = record
            alert_slots = slots
        end,
        matrix_alert_random_bound = function(bound)
            eq(bound, 10, "Matrix alert uses the ROM ten-sided bound")
            return 0
        end,
    },
    redraw = function() end,
    sound = function() end,
}
local alert_screen = MatrixNodeAction.new({
    matrix_state = active_spu_state, traversal_view = true,
})
MatrixNodeAction.enter(alert_screen, alert_context)
eq(active_spu_state.alert, MX.PASSIVE,
    "ROM alert probe waits until the world-entry sequence finishes")
eq(alert_screen.matrix_message, nil,
    "world-entry view does not show the later node-action notice")
for _ = 1, 61 do
    MatrixNodeAction.input(alert_screen, 0, alert_context)
end
eq(Session.traversal_view(active_spu_state).world_effect_phase, 0,
    "ROM world-effect timer starts after the 61-VBlank HUD setup")
for _ = 1, 238 do
    MatrixNodeAction.input(alert_screen, 0, alert_context)
end
eq(active_spu_state.alert, MX.PASSIVE,
    "random-alert probe remains after the 238 serviced entry frames")
eq(Session.traversal_view(active_spu_state).world_effect_phase, 4,
    "the unseen entry record uses node reaction timing during entry")
eq(Session.traversal_view(active_spu_state).world_effect_mode, MX.UNSEEN,
    "0x1A908 selects from the rank-zero state before 0x54662 raises rank")
MatrixNodeAction.input(alert_screen, 0, alert_context)
eq(active_spu_state.alert, MX.ACTIVE,
    "ROM random-alert event runs after the world-entry handoff")
eq(active_spu_state.world_effect_gate_negative, false,
    "0x54A8E raises FFF0A0 without changing the FFDF84 gate")
eq(alert_screen.message_delay_frames, 26,
    "ROM alert notice starts its 26-VBlank wait after entry")
for _ = 1, 26 do MatrixNodeAction.input(alert_screen, 0, alert_context) end
MatrixNodeAction.input(alert_screen, E.Pad.A, alert_context)
for _ = 1, 14 do MatrixNodeAction.input(alert_screen, 0, alert_context) end
eq(Session.traversal_view(active_spu_state).world_effect_phase, 4,
    "stored world-effect phase remains fixed while the node-action loop is open")
eq(Session.set_current_node_analysis_level(active_spu_state, 0), true,
    "active-alert reproduction keeps the SPU unknown")
local active_spu_view = Session.traversal_view(active_spu_state)
eq(active_spu_view.world_model_id, 2,
    "Section 15 SPU route uses world model 2")
eq(active_spu_view.palette_theme, 0,
    "unknown active-alert SPU keeps neutral model palette")
eq(active_spu_view.ice_label, HUD.ic_visibility[1],
    "active-alert SPU still presents Ice Unknown")
eq(active_spu_view.world_effect_mode, MX.UNSEEN,
    "the node-action alert does not replace the mode stored at 0x1A908")
eq(Session.begin_world_entry(active_spu_state), true,
    "a later source entry initializes a fresh world-effect record")
eq(Session.traversal_view(active_spu_state).world_effect_mode, MX.UNSEEN,
    "Active Alert alone does not make 0x19B40 read the hidden node's IC")
local gate_started, gate_session = Session.begin(15, false, 13, nil, true)
eq(gate_started, true, "direct Matrix entry accepts a typed FFDF84 gate fact")
eq(Session.begin_world_entry(gate_session), true,
    "the explicit negative-gate fixture initializes a fresh effect record")
eq(Session.traversal_view(gate_session).world_effect_mode, 3,
    "System 15/node 13 selects Blaster mode when FFDF84 is negative")
eq(Session.traversal_view(gate_session).world_effect_phase, 0,
    "fresh mode selection resets the entry phase")
Session.finish(gate_session)
local memory_previous_active = Session.active
local remembered_node = Systems[15].nodes[13]
local map_knowledge = {
    system_id = 15,
    cells = {
        [remembered_node.y] = {
            [remembered_node.x] = {
                analysis_level = 4,
                security_color = "green",
            },
        },
        [9] = { [13] = { revealed = true } },
    },
}
local memory_started, memory_state = Session.begin(
    15, false, 13, nil, false, nil, map_knowledge)
eq(memory_started, true, "same-system Matrix entry accepts map knowledge")
eq(Session.current_node_analysis_level(memory_state), 4,
    "direct entry restores the current node's analysis rank")
eq(memory_state.map_knowledge.cells[remembered_node.y]
       [remembered_node.x].security_color, "green",
    "direct entry restores the visited node's named security color")
eq(memory_state.map_knowledge.cells[9][13].revealed, true,
    "same-system direct entry restores a revealed cell")
eq(Session.begin_world_entry(memory_state), true,
    "restored map knowledge participates in source world-mode selection")
eq(Session.traversal_view(memory_state).world_effect_mode, 3,
    "remembered rank reveals the System 15/node 13 Blaster effect without FFDF84")
Session.set_current_node_analysis_level(memory_state, 4)
eq(memory_state.map_knowledge.cells[remembered_node.y]
       [remembered_node.x].analysis_level, 4,
    "rank updates preserve the remembered analysis rank")
eq(memory_state.map_knowledge.cells[remembered_node.y]
       [remembered_node.x].security_color, "green",
    "rank updates preserve the remembered security color")
local reopened_started, reopened_state = Session.begin(
    15, false, 13, nil, false, nil, memory_state.map_knowledge)
eq(reopened_started, true, "reopening the same system reuses its RAM memory")
eq(Session.current_node_analysis_level(reopened_state), 4,
    "same-system re-entry retains the analyzed node rank")
eq(reopened_state.map_knowledge.cells[remembered_node.y]
       [remembered_node.x].security_color, "green",
    "same-system re-entry retains the visited map color")
local switched_started, switched_state = Session.begin(
    16, false, 0, nil, false, nil, reopened_state.map_knowledge)
eq(switched_started, true, "switching Matrix systems starts a valid session")
eq(switched_state.map_knowledge.cells[remembered_node.y]
       [remembered_node.x], nil,
    "a system change clears the previous cell's analysis and color")
eq(switched_state.map_knowledge.cells[9][13], nil,
    "a system change clears the previous cell's revealed state")
eq(Session.current_node_analysis_level(switched_state), 0,
    "a different system does not inherit the previous node's knowledge")
Session.active = memory_previous_active
active_spu_state.phase = "link_select"
active_spu_state.link_cursor = 0
MatrixNodeAction.input(alert_screen, E.Pad.A, alert_context)
eq(active_spu_state.current_node, 0,
    "link confirmation selects the linked System Access node")
eq(alert_screen.world_entry_pending, true,
    "traversal link confirmation starts the next world-entry sequence")
eq(Session.current_node_analysis_level(active_spu_state), 0,
    "the new node's knowledge rank stays unchanged during entry")
for _ = 1, 400 do
    if not alert_screen.world_entry_pending then break end
    MatrixNodeAction.input(alert_screen, 0, alert_context)
end
eq(alert_screen.world_entry_pending, false,
    "the second world-entry sequence reaches its node-action handoff")
eq(Session.current_node_analysis_level(active_spu_state), 4,
    "the node-action handoff applies rank four after world entry")
end
local loading_state = {}
local loading_slots = {
    [0] = { status = 5, timer = 2, phase = 0 },
    [1] = { status = 0, timer = 1, phase = 0 },
    [2] = { status = 6, timer = 0, phase = 1 },
    [3] = { status = 6, timer = 0, phase = 1 },
    [4] = { status = 6, timer = 0, phase = 1 },
}
eq(Session.advance_program_loading(loading_state, loading_slots), true,
    "ROM service advances the first incomplete footer slot immediately")
eq(loading_slots[0].status, 6,
    "ROM loader completes the first footer slot before later slots")
eq(loading_slots[0].phase, 1,
    "completed footer slot publishes the Ready state")
eq(loading_slots[1].status, 0,
    "next footer slot waits for a later source service pass")
eq(loading_state.program_loading.progress_delay, 2,
    "ROM loader installs the completed slot delay")
eq(Session.advance_program_loading(loading_state, loading_slots), false,
    "ROM loader counts down the first inter-step frame")
eq(Session.advance_program_loading(loading_state, loading_slots), false,
    "ROM loader counts down the second inter-step frame")
eq(Session.advance_program_loading(loading_state, loading_slots), true,
    "ROM loader advances one slot after the signed delay expires")
eq(loading_slots[1].status, 1,
    "ROM loader advances the next incomplete slot by one visual state")
loading_slots[1].status, loading_slots[1].phase = 6, 1
eq(Session.advance_program_loading(loading_state, loading_slots), false,
    "ROM loader preserves the remaining per-slot countdown")
eq(Session.advance_program_loading(loading_state, loading_slots), false,
    "ready footer slots do not advance after the delay expires")
eq(loading_state.program_loading, nil,
    "loader countdown state retires when all footer slots are ready")
eq(Session.file_class_name(46), "HVAC Systems",
    "Matrix file-class lookup includes the final ROM record")
local _, paired_ice_state = Session.begin(16, true)
paired_ice_state.current_node = 4
local paired_ice_view = Session.traversal_view(
    paired_ice_state, "Analyze-2", Session.program_status_label(10))
eq(paired_ice_view.ice_label, HUD.ic_visibility[1],
    "unseen IC uses the ROM unknown label")
eq(paired_ice_view.file_class_label, "",
    "rank one hides the node file class")
eq(paired_ice_view.action_difficulty, -1,
    "unseen node hides the node security rating")
Session.set_current_node_analysis_level(paired_ice_state, 1)
paired_ice_view = Session.traversal_view(
    paired_ice_state, "Analyze-2", Session.program_status_label(10))
eq(paired_ice_view.ice_label, HUD.ic_types[0],
    "rank one reveals the IC type")
eq(paired_ice_view.ice_rating_label, "?",
    "rank one shows the IC type but hides its rating")
Session.set_current_node_analysis_level(paired_ice_state, 4)
paired_ice_view = Session.traversal_view(
    paired_ice_state, "Analyze-2", Session.program_status_label(10))
eq(paired_ice_view.function_label, "SPU",
    "Matrix HUD node function comes from the ROM text table")
eq(paired_ice_view.file_class_label, " Data Routing ",
    "rank four reveals the ROM file-class cell")
eq(paired_ice_view.ice_label, HUD.ic_types[0],
    "first IC layer comes from the two-byte ROM combo table")
eq(paired_ice_view.ice_rating_label, "7",
    "first IC rating adds the active-alert bonus")
eq(paired_ice_view.second_ice_label, HUD.ic_types[3],
    "second IC layer renders on the second source row")
eq(paired_ice_view.second_ice_rating_label, "6",
    "second IC rating uses the node's second rating byte")
eq(paired_ice_view.alert_label, HUD.alert_states[2],
    "node HUD alert string uses the source alert table")
eq(paired_ice_view.active_program_status_label, HUD.program_states[10],
    "active program status uses the FFF0E8 HUD table")
local started, first_session = Session.begin(0)
eq(started, true, "Matrix session starts from system zero")
eq(first_session.current_node, 0, "Matrix session enters access node")
eq(first_session.mode, E.MatrixModeResult.BALANCED,
    "ROM Matrix entry clears the deck mode to balanced")
eq(first_session.nodes[0].links[0], 4, "system zero access link")
eq(first_session.map[9][13] ~= nil, true,
    "Matrix session copies the bottom-right map coordinate")
eq(first_session.map[10], nil,
    "Matrix session contains only the ten coordinate rows")
local alarm_started, alarm_session = Session.begin(0, true)
eq(alarm_started, true, "alarm-backed Matrix session starts")
eq(alarm_session.alert, MX.ACTIVE,
   "FFE16F seeds an active Matrix alert on entry")
eq(alarm_session.world_effect_gate_negative, false,
   "the typed Matrix session keeps FFDF84 separate from its initial alert")
eq(alarm_session.attempts, 0,
   "Matrix action-attempt counter starts at zero")
eq(alarm_session.overload_armed, true,
   "node-action entry arms the ROM overload response")
Session.finish(alarm_session)
local targeted_entry = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.DELETE_FILE, "Security Files", 0, 30, 6)
eq(Session.activate_run_for_system(5, targeted_entry), true,
    "entering another system deactivates a Matrix objective")
eq(targeted_entry.status, E.MatrixRunStatus.INACTIVE,
    "an unfinished Matrix objective is inactive outside its target system")
eq(Session.activate_run_for_system(5, targeted_entry), false,
    "re-entering the same non-target system leaves its state stable")
eq(Session.activate_run_for_system(6, targeted_entry), true,
    "entering the objective system activates the Matrix run")
eq(targeted_entry.status, E.MatrixRunStatus.ACTIVE,
    "the ROM target system restores the active Matrix run state")
local completed_targeted_entry = matrix_run(E.MatrixRunStatus.COMPLETED,
    "datastore", E.MatrixRunAction.DELETE_FILE, "Security Files", 0, 30, 6)
eq(Session.activate_run_for_system(5, completed_targeted_entry), true,
    "fresh Matrix entry resets the prior completed-state display")
eq(completed_targeted_entry.status, E.MatrixRunStatus.INACTIVE,
    "completed-run flag prevents reactivation of the target system")
eq(table.concat(Session.map_labels(), ","), "CPU,SPU,IOP,SAN,SM,DS",
    "Matrix map labels match ROM text table")
local seen_world_models = {}
for system_id = 0, 24 do
    local system = Systems[system_id]
    for node_id, node in pairs(system.nodes) do
        local status = Session.node_status({
            nodes = system.nodes, current_node = node_id,
        })
        assert(status and status.world_model_id == node.function_id,
            "node status preserves the ROM world-model id")
        seen_world_models[status.world_model_id] = true
    end
end
for model_id = 0, 5 do
    assert(seen_world_models[model_id],
        "Matrix topology includes world-model id " .. model_id)
end
for system_id = 0, 24 do
    local ok, session = Session.begin(system_id)
    assert(ok, "Matrix map visual state initializes for system " .. system_id)
    local visuals = Session.map_visual_cells(session)
    assert(#visuals == 140, "Matrix map visual cell count for system " .. system_id)
    for cell = 1, 140 do
        assert(#visuals[cell].tiles == 4,
            "Matrix cell tile word count for system " .. system_id)
        for _, word in ipairs(visuals[cell].tiles) do
            local pattern = word & 0x07FF
            local palette = word & 0x6000
            assert(pattern >= 0x00BF and pattern <= 0x00D8,
                "Matrix cell pattern range for system " .. system_id)
            assert(palette == 0x2000 or palette == 0x4000 or
                   palette == 0x6000,
                "Matrix cell palette range for system " .. system_id)
        end
    end
    Session.finish(session)
end
Session.finish(first_session)

local mode_state = { mode = E.MatrixModeResult.ATTACK }
local restored_deck_mode
local deck_screen = Cyberdeck.Cyberdeck.new({ matrix_state = mode_state,
    matrix_session = true, return_to_node_action = true })
local deck_context = { redraw = function() end, game = {
    deck_enter = function() return E.MatrixModeResult.ATTACK end,
    deck_leave = function(mode) restored_deck_mode = mode end,
    set_deck_mode = function() end,
    deck_record = function()
        return { ratings = {}, active = {}, load_io = 0, memory = 0 }
    end,
    deck_slots = function() return {} end,
    matrix_world_state = function() return {} end,
} }
Cyberdeck.Cyberdeck.enter(deck_screen, deck_context)
eq(mode_state.mode, E.MatrixModeResult.BALANCED,
    "Cyberdeck entry mirrors the source temporary balanced mode")
Cyberdeck.Cyberdeck.input(deck_screen, E.Pad.C, deck_context)
eq(mode_state.mode, E.MatrixModeResult.DEFENSE,
    "Cyberdeck C input mirrors the signed defense mode into Lua")
Cyberdeck.Cyberdeck.input(deck_screen, E.Pad.START, deck_context)
eq(mode_state.mode, E.MatrixModeResult.ATTACK,
    "Cyberdeck exit restores the saved Matrix action mode")
eq(restored_deck_mode, E.MatrixModeResult.ATTACK,
    "host deck mode receives the same saved mode on exit")

-- Matrix node actions only select a movement mode. The ROM keeps movement in
-- its own input loop, then commits the highlighted link or CPU destination.
local no_alert_state = { alert = -1 }
local no_alert_result = Actions.cancel_alert(no_alert_state)
eq(no_alert_state.alert, -1, "Cancel Alert leaves the none state unchanged")
eq(no_alert_result.text, Actions.TEXT.NO_ALERTS,
    "Cancel Alert selects the ROM no-alert message")
local active_alert_state = { alert = 1 }
local cancelled_alert_result = Actions.cancel_alert(active_alert_state)
eq(active_alert_state.alert, -1, "Cancel Alert clears an active alert")
eq(cancelled_alert_result.text, Actions.TEXT.ALERT_CANCELLED,
    "Cancel Alert selects the ROM cancellation message")
eq(Actions.lockout().text, Actions.TEXT.LOCKOUT,
    "Lockout selects its ROM text address")
eq(MatrixText.from_rom(Actions.TEXT.LOCKOUT), MatrixText.LOCKOUT,
    "ROM Lockout text address resolves to the decoded message")
eq(MatrixText.from_rom(Actions.TEXT.SHUTDOWN),
    "  INITIATING||SYSTEM  SHUTDOWN|| IN - - SECONDS",
    "ROM Crash System text address resolves to the decoded message")
eq(MatrixText.from_rom(Actions.TEXT.OFFLINE), "NODE IS OFFLINE",
    "ROM Turn Off Node text address resolves to its decoded message")
eq(MatrixText.from_rom(Actions.TEXT.ERASE_OBJECTIVE),
    "  The recog-code| rapidly finds the|data and zaps it to| electron heaven!!|| Run accomplished!",
    "ROM objective Erase text preserves source line breaks")
eq(MatrixText.from_rom(Actions.TEXT.ERASED),
    "You find a file that| appears important| and obliterate it|  from memory!!!|| Score one for the|downtrodden hacker!",
    "ROM ordinary Erase text preserves source line breaks")
local crash_run = matrix_run(E.MatrixRunStatus.ACTIVE, "cpu",
    E.MatrixRunAction.CRASH_SYSTEM, nil, nil, 0)
local crash_field = { alarm = true, security_off = false, maglocks_off = false }
local alarm_actors = {
    { actor_facts = { roles = { spotted = false, reinforcement = true,
        despawn_offscreen = false, quest = false, runner = false,
        pedestrian = false, hostile = false } } },
    { actor_facts = { roles = { spotted = false, reinforcement = false,
        despawn_offscreen = true, quest = false, runner = false,
        pedestrian = false, hostile = false } } },
}
local crash_result = Actions.crash_system(crash_run, crash_field, alarm_actors)
eq(crash_run.status, E.MatrixRunStatus.COMPLETED,
   "Crash System completes a CPU objective")
eq(crash_run.completion_marker_present, false,
   "Crash System clears the semantic completion marker")
eq(crash_run.completion_recorded, true,
   "Crash System records objective completion")
eq(crash_field.alarm, false, "Crash System turns the alarm off")
eq(crash_field.security_off, true, "Crash System disables security")
eq(crash_field.maglocks_off, true, "Crash System disengages maglocks")
eq(alarm_actors[1].actor_facts.roles.despawn_offscreen, true,
    "alarm-off marks reinforcement actors for despawn")
eq(alarm_actors[2].actor_facts.roles.despawn_offscreen, true,
    "alarm-off preserves an unrelated despawn role")
eq(crash_result.step, 61, "Crash System wait uses D5=$3C plus the DBF pass")
eq(crash_result.stop_sfx, 0x30, "Crash System stops the source alarm effect")
local partial_run = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.DELETE_FILE, "Outdated Files", 0, 30)
Actions.crash_system(partial_run, {})
eq(partial_run.status, E.MatrixRunStatus.ACTIVE,
   "Crash System leaves a datastore objective incomplete")
eq(partial_run.completion_marker_present, true,
   "incomplete Crash System preserves the objective marker")
local absent_run_result = Actions.crash_system(nil, {})
eq(absent_run_result.logout, true, "Crash System handles an unknown run without inventing one")

local class_27_field = { alarm = true }
local class_27_actors = { { actor_facts = { roles = {
    spotted = false, reinforcement = true, despawn_offscreen = false,
    quest = false, runner = false, pedestrian = false, hostile = false,
} } } }
local class_27_result = Actions.turn_off_node(
    { file_class = "Security Cntrl" }, class_27_field, class_27_actors)
eq(class_27_field.alarm, false, "class 0x27 turns off the alarm")
eq(class_27_field.security_off, true, "class 0x27 disables building security")
eq(class_27_actors[1].actor_facts.roles.despawn_offscreen, true,
    "class 0x27 marks alarm responders for despawn")
eq(class_27_result.stop_sfx, 0x30, "class 0x27 stops the alarm sound")
eq(class_27_result.text, Actions.TEXT.OFFLINE, "class 0x27 uses ROM offline text")
local class_28_field = { alarm = true }
local class_28_result = Actions.turn_off_node(
    { file_class = "Cameras" }, class_28_field)
eq(class_28_field.security_off, true, "class 0x28 disables security")
eq(class_28_field.alarm, true, "class 0x28 leaves the alarm unchanged")
eq(class_28_result.stop_sfx, nil, "class 0x28 does not stop alarm audio")
local class_29_field = { alarm = true }
Actions.turn_off_node({ file_class = "Maglocks" }, class_29_field)
eq(class_29_field.maglocks_off, true, "class 0x29 disengages maglocks")
eq(class_29_field.alarm, true, "class 0x29 leaves the alarm unchanged")
local class_2a_field = { alarm = true }
local class_2a_result = Actions.turn_off_node(
    { file_class = "Alert Control" }, class_2a_field)
eq(class_2a_field.alarm, false, "class 0x2A turns off the alarm")
eq(class_2a_field.security_off, nil, "class 0x2A leaves security unchanged")
eq(class_2a_result.stop_sfx, 0x30, "class 0x2A stops the alarm sound")

local linked_session_ok, linked_session = Session.begin(0)
eq(linked_session_ok, true, "node navigation session starts")
eq(Session.select_node_action(linked_session, "go_to_node"), false,
    "non-CPU nodes do not expose Go To Node")
eq(Session.select_node_action(linked_session, "enter_system"), true,
    "system access action opens link navigation")
eq(linked_session.phase, "link_select",
    "choosing Enter System does not move immediately")
eq(linked_session.current_node, 0,
    "link navigation preserves the current node until confirm")
eq(Session.retreat(linked_session), false,
    "retreat is unavailable during link selection")
eq(Session.step_link(linked_session, true), true,
    "link navigation accepts a right edge")
eq(linked_session.link_cursor, 1,
    "link cursor advances across the source link array")
eq(Session.commit_node_selection(linked_session), true,
    "confirm enters the selected linked node")
eq(linked_session.current_node, 6,
    "link confirmation uses the selected link target")
eq(linked_session.previous_node, 0,
    "link confirmation records the source node for retreat")
eq(linked_session.phase, "node",
    "link confirmation returns to node actions")
eq(Session.retreat(linked_session), true,
    "retreat returns to the previous Matrix node")
eq(linked_session.current_node, 0,
    "retreat restores the previous node")
eq(linked_session.previous_node, 6,
    "retreat keeps the node just left as the new previous node")

local cpu_session_ok, cpu_session = Session.begin(0)
eq(cpu_session_ok, true, "CPU navigation session starts")
eq(Session.select_node_action(cpu_session, "enter_system"), true,
    "system access opens link navigation for CPU path")
eq(Session.commit_node_selection(cpu_session), true,
    "confirm selects the first system access link")
eq(cpu_session.current_node, 4,
    "first system access link reaches the CPU")
eq(Session.select_node_action(cpu_session, "go_to_node"), true,
    "CPU Go To Node action opens whole-system navigation")
eq(cpu_session.phase, "goto", "CPU navigation has its own input phase")
eq(cpu_session.goto_cursor, 0,
    "Go To Node cursor starts at node zero, including when not at zero")
eq(Session.step_goto(cpu_session, true), true,
    "Go To Node accepts a right edge")
eq(cpu_session.goto_cursor, 1,
    "Go To Node cursor advances through the node catalog")
eq(Session.commit_node_selection(cpu_session), true,
    "Go To Node confirm commits the selected destination")
eq(cpu_session.current_node, 1,
    "Go To Node enters the selected node")
eq(cpu_session.previous_node, 4,
    "linked Go To Node destination preserves retreat")
eq(Session.cancel_node_selection(cpu_session), false,
    "node phase rejects a stale navigation cancel")

-- Exercise the actual deck-grid screen against a narrow fake host. This verifies
-- the Lua-owned palette cadence, shared child state, cursor, and selected mode.
local Cyberdeck = require("menu.screens.cyberdeck").Cyberdeck
local deck_screen, grid = Cyberdeck.new(), { mode = nil, redraw = nil }
local grid_ctx = {
    game = {
        set_deck_mode = function(mode) grid.mode = mode end,
        deck_record = function()
            return { ratings = {}, active = {}, load_io = 0, memory = 0 }
        end,
        deck_slots = function() return {} end,
    },
    redraw = function(region) grid.redraw = region end,
}
Cyberdeck.input(deck_screen, E.Pad.UP, grid_ctx)
eq(deck_screen.deck_palette_phase, 1,
   "deck palette advances on its first active frame")
eq(deck_screen.col, 0, "deck screen up keeps first column")
eq(deck_screen.row, 3, "deck screen up wraps to bottom row")
eq(grid.redraw, E.MenuRegion.FULL, "deck screen redraws changed footer art")
local child = Cyberdeck.input(deck_screen, E.Pad.A, grid_ctx)
eq(grid.mode, E.MatrixDeckMode.ATTACK, "deck screen submits attack mode")
eq(child.transition, E.MenuTransition.PUSH, "deck screen opens child route")
eq(child.screen, E.MenuScreen.DECK_CYBERINFO, "bottom-left cell opens deck info")
eq(child.args.deck_palette, deck_screen.deck_palette,
   "deck child shares Lua-owned palette state")
eq(child.args.matrix_state, deck_screen.matrix_state,
   "deck child shares the active Matrix session")

-- Exercise all eight FFDD22 edge bits from every grid cell, in both the
-- field deck and the online Matrix deck. ROM 0x1833C..0x18418 gives direction
-- priority Up/Down/Left/Right; 0x1841A, 0x18630, 0x18648 then give A/B/C
-- priority before the Start exit at 0x18660. The 0x1842E dispatch opens a
-- program child for rows 0..2, CyberInfo/Storage/Map for utility columns 0..2,
-- and gates Map/Pull Plug on the online marker.
do
    local pad = E.Pad
    local edge_bits = {
        pad.UP, pad.DOWN, pad.LEFT, pad.RIGHT,
        pad.A, pad.B, pad.C, pad.START,
    }
    local function run_deck_mask_matrix(
        online, matrix_state, return_to_node_action)
        local input = { mode = "unchanged", left_mode = nil }
        local context = {
            game = {
                set_deck_mode = function(mode) input.mode = mode end,
                deck_leave = function(mode) input.left_mode = mode end,
                deck_record = function()
                    return { ratings = {}, active = {}, load_io = 0,
                             memory = 0 }
                end,
                deck_slots = function() return {} end,
                matrix_node = function()
                    return { ic_condition = 0, ic_code = 0,
                             ic_rating = 0, ic_rating2 = 0 }
                end,
                alert = function() return MX.NONE end,
                world_effect_gate_negative = function() return false end,
                matrix_node_discovered = function() return false end,
                in_combat = function() return false end,
                matrix_online = function() return false end,
            },
            redraw = function() end,
        }

        for cell = 0, 15 do
            local col, row = cell % 4, cell // 4
            for mask = 0, 0xFF do
                local events = 0
                for bit = 1, #edge_bits do
                    if (mask & (1 << (bit - 1))) ~= 0 then
                        events = events | edge_bits[bit]
                    end
                end

                local screen = Cyberdeck.new({
                    matrix_session = online,
                    matrix_state = matrix_state,
                    return_to_node_action = return_to_node_action == true,
                })
                screen.col, screen.row = col, row
                screen.saved = E.MatrixModeResult.ATTACK
                input.mode, input.left_mode = "unchanged", nil
                if online then
                    matrix_state.mode = E.MatrixModeResult.BALANCED
                end

                local expected_col, expected_row = col, row
                local moved = false
                if (events & pad.UP) ~= 0 then
                    expected_row, moved = (row + 3) & 3, true
                elseif (events & pad.DOWN) ~= 0 then
                    expected_row, moved = (row + 1) & 3, true
                elseif (events & pad.LEFT) ~= 0 then
                    expected_col, moved = (col + 3) & 3, true
                elseif (events & pad.RIGHT) ~= 0 then
                    expected_col, moved = (col + 1) & 3, true
                end

                local button
                if (events & pad.A) ~= 0 then
                    button = pad.A
                elseif (events & pad.B) ~= 0 then
                    button = pad.B
                elseif (events & pad.C) ~= 0 then
                    button = pad.C
                end

                local label = ("Cyberdeck %s cell %02d mask %02X"):format(
                    online and "online" or "field", cell, mask)
                local result = Cyberdeck.input(screen, events, context)
                eq(screen.col, expected_col, label .. " cursor column")
                eq(screen.row, expected_row, label .. " cursor row")

                if moved then
                    eq(result, nil,
                       label .. " directions precede buttons and Start")
                    eq(input.mode, "unchanged",
                       label .. " movement preserves the selected deck mode")
                    if online then
                        eq(matrix_state.mode, E.MatrixModeResult.BALANCED,
                           label .. " movement preserves Matrix mode")
                    end
                elseif button ~= nil then
                    local deck_mode = button == pad.A and
                        E.MatrixDeckMode.ATTACK or button == pad.B and
                        E.MatrixDeckMode.BALANCED or E.MatrixDeckMode.DEFENSE
                    local matrix_mode = button == pad.A and
                        E.MatrixModeResult.ATTACK or button == pad.B and
                        E.MatrixModeResult.BALANCED or E.MatrixModeResult.DEFENSE
                    eq(input.mode, deck_mode, label .. " button mode")
                    if online then
                        eq(matrix_state.mode, matrix_mode,
                           label .. " published Matrix mode")
                    end

                    local expected_screen
                    if row < 3 then
                        expected_screen = E.MenuScreen.DECK_PROGRAM
                    elseif col == 0 then
                        expected_screen = E.MenuScreen.DECK_CYBERINFO
                    elseif col == 1 then
                        expected_screen = E.MenuScreen.DECK_STORAGE
                    elseif col == 2 and online then
                        expected_screen = E.MenuScreen.DECK_MAP
                    elseif col == 3 and online then
                        expected_screen = E.MenuScreen.PULL_PLUG
                    end
                    eq(result and result.transition,
                       expected_screen and E.MenuTransition.PUSH or nil,
                       label .. " child transition gate")
                    eq(result and result.screen, expected_screen,
                       label .. " child screen")
                    if expected_screen == E.MenuScreen.DECK_PROGRAM then
                        eq(result.args.col, col,
                           label .. " program child column")
                        eq(result.args.row, row,
                           label .. " program child row")
                        eq(result.args.deck_parent_col, col,
                           label .. " program return column")
                        eq(result.args.deck_parent_row, row,
                           label .. " program return row")
                    elseif expected_screen == E.MenuScreen.PULL_PLUG then
                        eq(result.args.black_ic, false,
                           label .. " Pull Plug black-IC state")
                    elseif expected_screen ~= nil then
                        eq(result.args.deck_parent_col, col,
                           label .. " utility return column")
                        eq(result.args.deck_parent_row, row,
                           label .. " utility return row")
                    end
                elseif (events & pad.START) ~= 0 then
                    eq(input.mode, "unchanged",
                       label .. " Start does not select an action mode")
                    eq(input.left_mode, E.MatrixModeResult.ATTACK,
                       label .. " Start restores the saved Matrix mode")
                    if online then
                        if return_to_node_action then
                            eq(result.transition, E.MenuTransition.POP,
                               label .. " nested Start returns to node actions")
                            eq(result.result.matrix_resumed, true,
                               label .. " nested Start resumes the parent")
                        else
                            eq(result.transition, E.MenuTransition.REPLACE,
                               label .. " entry Start returns to node actions")
                            eq(result.screen, E.MenuScreen.MATRIX_NODE_ACTION,
                               label .. " entry Start destination")
                        end
                        eq(matrix_state.mode, E.MatrixModeResult.ATTACK,
                           label .. " online Start restores Matrix mode")
                    else
                        eq(result.transition, E.MenuTransition.POP,
                           label .. " field Start exits the deck")
                    end
                else
                    eq(result, nil, label .. " idle stays on the grid")
                    eq(input.mode, "unchanged",
                       label .. " idle preserves the selected mode")
                    if online then
                        eq(matrix_state.mode, E.MatrixModeResult.BALANCED,
                           label .. " idle preserves Matrix mode")
                    end
                end
            end
        end
    end

    -- Earlier navigation fixtures leave their last Matrix singleton active;
    -- clear it so this first pass genuinely exercises the offline field deck.
    Session.finish(Session.current_state())
    run_deck_mask_matrix(false, nil, false)
    local session_started, mask_matrix_session = Session.begin(0)
    eq(session_started, true, "online Cyberdeck mask fixture starts")
    run_deck_mask_matrix(true, mask_matrix_session, true)
    run_deck_mask_matrix(true, mask_matrix_session, false)
    Session.finish(mask_matrix_session)
end

-- Audit every CyberInfo selector input combination at both ROM choices.
-- ROM 0x18ACC checks Up before Down, then A/B/C at 0x18B18..0x18B38,
-- and Start at 0x18B4A; Left/Right are ignored by this two-row screen.
do
    local CyberInfo = require("menu.screens.deck_screens").CyberInfo
    local pad = E.Pad
    local edge_bits = {
        pad.UP, pad.DOWN, pad.LEFT, pad.RIGHT,
        pad.A, pad.B, pad.C, pad.START,
    }
    local record = {
        model = 3, mpcp = 6, hardening = 3, memory = 128, storage = 1000,
        load_io = 10, response = 5, persona = { [0] = 2, [1] = 7,
                                                [2] = 4, [3] = 1 },
        ratings = {}, active = {},
    }
    local expected_values = { 6, 3, 128, 1000, 10, 5, 2, 7, 4, 1 }
    local host = {
        deck_record = function() return record end,
        deck_slots = function() return {} end,
        matrix_online = function() return false end,
        set_deck_mode = function() end,
    }
    local context = { game = host, redraw = function() end }
    Session.finish(Session.current_state())

    for initial_row = 0, 1 do
        for mask = 0, 0xFF do
            local events = 0
            for bit = 1, #edge_bits do
                if (mask & (1 << (bit - 1))) ~= 0 then
                    events = events | edge_bits[bit]
                end
            end
            local screen = CyberInfo.new({
                deck_parent_col = 0, deck_parent_row = 3,
            })
            screen.row = initial_row
            local vertical = (events & (pad.UP | pad.DOWN)) ~= 0
            local command = not vertical and
                            (events & (pad.A | pad.B | pad.C)) ~= 0
            local start = not vertical and not command and
                          (events & pad.START) ~= 0
            local expected_row = vertical and 1 - initial_row or initial_row
            local label = ("CyberInfo row %d mask %02X"):format(
                initial_row, mask)
            local result = CyberInfo.input(screen, events, context)

            eq(screen.row, expected_row, label .. " selector row")
            eq(screen.deck_parent_col, 0, label .. " parent column")
            eq(screen.deck_parent_row, 3, label .. " parent row")
            eq(screen.deck_palette_phase, 1,
               label .. " advances the shared ROM palette on first input")
            if vertical then
                eq(result, nil,
                   label .. " vertical cursor input precedes every action")
            elseif command then
                eq(result.transition, E.MenuTransition.POP,
                   label .. " A/B/C confirms the selected child")
                if initial_row == 0 then
                    local stats = result.result.matrix_deck_stats
                    eq(stats.manufacturer, "Fuchi",
                       label .. " Deck Stats manufacturer")
                    eq(stats.model, "Cyber-7", label .. " Deck Stats model")
                    for index, value in ipairs(expected_values) do
                        eq(stats.values[index], value,
                           label .. " Deck Stats live field " .. index)
                    end
                else
                    local run_info = result.result.matrix_run_info
                    eq(run_info.kind, "no_run",
                       label .. " Run Info follows the no-run RAM branch")
                    eq(run_info.message, "You are not on|a matrix run!!",
                       label .. " Run Info text")
                end
            elseif start then
                eq(result.transition, E.MenuTransition.POP,
                   label .. " Start returns to the parent deck")
                eq(result.result.matrix_deck_stats, nil,
                   label .. " Start does not publish Deck Stats")
                eq(result.result.matrix_run_info, nil,
                   label .. " Start does not publish Run Info")
            else
                eq(result, nil, label .. " unrelated or empty input stays")
            end
        end
    end

    -- 0x1AB100 is the default hardware label; 0x1AB118.. contains the five
    -- indexed maker/model pairs. The numeric values remain live record fields.
    local models = {
        { -1, "Allegiance", "Alpha" },
        { 0, "Cyber Shack", "PCD-500" },
        { 1, "Fuchi", "Cyber-5" },
        { 2, "SEGA", "CTY-360" },
        { 3, "Fuchi", "Cyber-7" },
        { 4, "Fairlight", "Excalibur" },
    }
    for _, model in ipairs(models) do
        record.model = model[1]
        local screen = CyberInfo.new({
            deck_parent_col = 0, deck_parent_row = 3,
        })
        local result = CyberInfo.input(screen, pad.A, context)
        local label = ("Deck Stats model %d"):format(model[1])
        local stats = result.result.matrix_deck_stats
        eq(stats.manufacturer, model[2], label .. " ROM manufacturer")
        eq(stats.model, model[3], label .. " ROM model")
        for index, value in ipairs(expected_values) do
            eq(stats.values[index], value,
               label .. " live numeric field " .. index)
        end
    end
end

-- IC visibility, layered types, and alert bonuses follow the ROM tables.
local node = { ic_condition = 10, ic_code = 7, ic_rating = 3, ic_rating2 = 5 }
eq(MX.ic_type(node, 0, false, false), MX.UNSEEN, "unseen node")
eq(MX.ic_type(node, 0, true, false), 0, "normal IC layer")
eq(MX.ic_type(node, 1, true, true), 6, "active IC layer")
node.ic_condition = 0
eq(MX.ic_type(node, 1, true, true), MX.NO_IC, "crashed IC")
node.ic_condition = 10
eq(MX.ic_rating(node, -1), 3, "no-alert rating")
eq(MX.ic_rating(node, 0), 4, "passive-alert rating")
eq(MX.ic_rating(node, 1), 7, "active second-layer rating")
local alert, overload = MX.raise_alert(-1, 1, false)
eq(alert, 0, "raise no-alert to passive")
eq(overload, false, "no overload below active")
alert, overload = MX.raise_alert(1, 1, true)
eq(alert, MX.ACTIVE, "active alert saturates")
eq(overload, true, "armed overload returned")
eq(MX.initial_alert(true), MX.ACTIVE,
   "field alarm initializes Matrix alert status")
eq(MX.initial_alert(false), MX.NONE,
   "quiet field initializes no Matrix alert")

local NodeActions = require("menu.rules.node_actions")
local attempts = { attempts = 3 }
local random_bound_seen
eq(NodeActions.random_alert(attempts, function(bound)
    random_bound_seen = bound
    return 2
end), true, "random alert fires below the accumulated attempt count")
eq(random_bound_seen, 10, "random alert always rolls against ten")
eq(attempts.attempts, 0, "triggered random alert clears its counter")
attempts.attempts = 0
eq(NodeActions.random_alert(attempts, function(bound)
    random_bound_seen = bound
    return 0
end), false, "zero attempts cannot raise an alert")
eq(random_bound_seen, 10,
   "zero attempts still consume the ROM's random draw")

-- A connected map region may contain a loop of connector cells. Reveal must
-- mark every reachable cell once and terminate while preserving the ROM's
-- directional connector checks.
local matrix_map = {}
for y = 0, MX.H - 1 do
    matrix_map[y] = {}
    for x = 0, MX.W - 1 do matrix_map[y][x] = 0xFF end
end
matrix_map[0][0] = 0
matrix_map[0][1] = 19
matrix_map[0][2] = 19
matrix_map[1][1] = 20
matrix_map[1][2] = 18
local visible = MX.reveal(matrix_map, {}, 0, 0)
eq(visible[0][0].revealed, true, "map reveals current node at its coordinate")
eq(visible[0][1].revealed, true, "map reveals first connector to the east")
eq(visible[0][2].revealed, true, "map follows connector east")
eq(visible[1][1].revealed, true, "map follows connector south")
eq(visible[1][2].revealed, true, "map follows connector loop")
local wrap_map = {}
for y = 0, MX.H - 1 do
    wrap_map[y] = {}
    for x = 0, MX.W - 1 do wrap_map[y][x] = 0 end
end
wrap_map[1][0] = 12
wrap_map[0][13] = 12
local wrapped = MX.reveal(wrap_map, {}, 0, 1)
eq(wrapped[0][13].revealed, true,
    "horizontal connector flow preserves the ROM's row-edge wrap")

-- Deck pool splits, target floors, and source word-sized memory math.
local deck = {
    mpcp = 8, response = 2, ratings = { [0] = 3, [1] = 4, [2] = 2,
        [3] = 1, [4] = 3, [5] = 1, [6] = 1, [7] = 1, [8] = 1,
        [9] = 1, [10] = 1, [11] = 1 }, evasion = 3, masking = 4,
    sensor = 5, bod = 6, hardening = 2, persona = 9, memory = 64,
    storage = 100, load_io = 3, active = { [0] = 0, [1] = -1, [2] = -1,
        [3] = -1, [4] = -1 }, files = {},
}
for i = 0, 4 do deck.files[i] = { id = -1, size = 0, value = 0 } end
local decker = { d81c = 3, computer = 6, physical = 10, int = 6 }
local st = { mode = -1, smoke = 0, mirrors = 0, alert = 0,
    karma_counter = 1, passive = 0, trace_on = false, trace_left = 0 }
eq(C.pool(deck, decker, -1, false), 9, "attack pool")
eq(C.pool(deck, decker, -1, true), 4, "attack mode defense pool")
eq(C.pool(deck, decker, 0, false), 6, "balanced attack pool")
eq(C.pool(deck, decker, 0, true), 7, "balanced defense rounds upward")
eq(C.pool(deck, decker, 1, true), 8, "defense pool clamps to MPCP")
eq(C.tn_decker(1, { smoke = 0 }), 2, "decker target minimum")
eq(C.tn_decker(3, { smoke = 2 }), 7, "smoke adds twice to target")
eq(C.tn_ic(1, { mirrors = 1, smoke = 1 }), 4, "IC target adds mirrors and smoke")
local roll_dice, roll_tn
local successes = C.decker_test(deck, decker,
    { colour = 2, security = 3 }, st, 0,
    function(dice, tn) roll_dice, roll_tn = dice, tn; return 2 end)
eq(roll_dice, 5, "decker dice include rating and colour penalty")
eq(roll_tn, 3, "decker test uses node security")
eq(successes, 2, "decker test returns successes")
eq(D.index(3, 2), 11, "program grid index")
eq(D.size(0, 3), 18, "program size")
eq(D.free_memory(deck), 46, "free memory from loaded program")
eq(D.fits(deck, 2), true, "program fits free memory")
eq(D.load_steps(13, 3), 5, "load duration rounds up")
eq(D.load_steps(100, 10), 6, "load duration cap")
eq(D.free_slot(deck), 1, "first empty program slot")
eq(D.loaded_slot(deck, 0), 0, "loaded program lookup")
local file_name_count = 0
for id = 0, 69 do
    if D.FILE_NAMES[id] ~= nil then file_name_count = file_name_count + 1 end
end
eq(file_name_count, 70, "deck contains all 70 ROM datafile names")
eq(D.FILE_NAMES[32], "Competitor Markting",
    "datafile names retain the source ROM spelling")
eq(D.FILE_NAMES[69], "Unsolved Cases",
    "datafile name table includes its final catalog entry")
eq(D.FILE_NAMES[70], nil, "text following the datafile table is not a file name")
local stored_files = { storage = 4096, files = {}, ratings = {} }
for i = 0, 4 do
    stored_files.files[i] = {
        id = -1, size = 0, data_value = 0, run_objective = false,
    }
end
for i = 0, 11 do stored_files.ratings[i] = 0 end
local objective_run = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.DOWNLOAD_FILE, "Security Files", 3, 30)
local objective_slot = Actions.store(stored_files, objective_run,
    { class = 4, colour = 2 },
    { run_objective = true, id = 3, size = 30 },
    function() return 0 end)
eq(objective_slot, 0, "objective file fills the first free deck slot")
eq(stored_files.files[0].id, 3,
    "objective download stores the requested data file")
eq(stored_files.files[0].run_objective, true,
    "stored objective uses a named marker")
eq(objective_run.status, E.MatrixRunStatus.COMPLETED,
    "storing the objective completes its run")
local ordinary_slot = Actions.store(stored_files, objective_run,
    { class = 4, colour = 3 }, { id = 21, size = 20 },
    function(bound) return bound == 7 and 1 or 0 end)
eq(ordinary_slot, 1, "ordinary data file uses the next free slot")
eq(stored_files.files[1].data_value, 2,
    "ordinary file retains its positive sale multiplier")
D.erase_file(stored_files, 0)
eq(stored_files.files[0].run_objective, false,
    "erasing a file clears its named objective marker")
eq(stored_files.files[0].data_value, 0,
    "erasing a file clears its ordinary data value")
-- [C] ROM 0x18C28..0x18CD0 distinguishes none, completed, CPU, and datastore
-- summaries; objective actions map to separate Run Info labels.
eq(D.run_info(nil).status, "none", "no active run summary")
eq(D.run_info(matrix_run(E.MatrixRunStatus.COMPLETED, "cpu",
    E.MatrixRunAction.CRASH_SYSTEM, nil, nil, 0)).status, "complete",
    "completed run summary")
local cpu_info = D.run_info(matrix_run(E.MatrixRunStatus.ACTIVE, "cpu",
    E.MatrixRunAction.CRASH_SYSTEM, nil, nil, 0))
eq(cpu_info.action, E.MatrixRunAction.CRASH_SYSTEM,
    "active CPU objective uses the Crash System action")
local delete_info = D.run_info(matrix_run(E.MatrixRunStatus.ACTIVE,
    "datastore", E.MatrixRunAction.DELETE_FILE, "Security Files", 0, 30))
eq(delete_info.action, E.MatrixRunAction.DELETE_FILE,
    "delete objective uses the Delete File action")
local upload_info = D.run_info(matrix_run(E.MatrixRunStatus.ACTIVE,
    "datastore", E.MatrixRunAction.UPLOAD_FILE, "Financial Data", nil, 30))
eq(upload_info.action, E.MatrixRunAction.UPLOAD_FILE,
    "upload objective uses the Upload File action")
local download_info = D.run_info(matrix_run(E.MatrixRunStatus.ACTIVE,
    "datastore", E.MatrixRunAction.DOWNLOAD_FILE, "Security Files", 20, 30))
eq(download_info.action, E.MatrixRunAction.DOWNLOAD_FILE,
    "download objective uses the Download File action")
eq(download_info.file_class, "Security Files",
    "Run Info retains the requested datastore category")
local slots = {
    [0] = { status = 6, phase = 0, timer = 0 },
    [1] = { status = 6, phase = 0, timer = 0 },
    [2] = { status = 6, phase = 0, timer = 0 },
    [3] = { status = 6, phase = 0, timer = 0 },
    [4] = { status = 6, phase = 0, timer = 0 },
}
D.load(deck, slots, 1, 2, false)
eq(deck.active[1], 2, "loading publishes selected program")
eq(slots[1].timer, 60, "loading timer is ten frames per step")
eq(slots[1].status, 0, "ready slot enters loading state")

-- Node movement and action outcomes preserve table ordering and story gates.
local links = { [0] = 5, [1] = -1, [2] = 7, [7] = 9 }
eq(Move.link_right(links, 0), 0, "right wraps at missing link")
eq(Move.link_left(links, 0), 7, "left scans down from last link")
local nodes = {
    [0] = { fn = 0, links = { [0] = 1 } },
    [1] = { fn = 1, links = { [0] = 0 } },
    [2] = { fn = -1, links = {} },
}
eq(Move.goto_right(nodes, 0, 0), 1, "CPU node navigation skips current")
eq(Move.goto_commit(nodes, 0, 1), 1, "linked jump preserves previous node")
eq(Move.goto_commit(nodes, 0, 0), 0, "unlinked jump has no retreat point")
local cancelled_alert_state = {
    alert = 0, world_effect_gate_negative = true,
}
local cancelled = Actions.cancel_alert(cancelled_alert_state)
eq(cancelled.text, Actions.TEXT.ALERT_CANCELLED, "cancel active alert text")
eq(cancelled_alert_state.alert, MX.NONE,
   "0x54AEA Cancel Alert writes FFF0A0 to No Alert")
eq(cancelled_alert_state.world_effect_gate_negative, true,
   "0x54AEA leaves the independent FFDF84 gate untouched")
local run = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.DELETE_FILE, "Security Files", 0, 30)
local run_node = { class = 5, file_class = "Security Files" }
-- [C] ROM 0x54C5E previews matching file objectives, uploads the explicit
-- upload case immediately, and 0x5559E completes only a matching delete.
local found = Actions.search(run, run_node, decker, deck, nil,
    function() return 0 end)
eq(found.file.run_objective, true,
   "objective search marks its preview as a run file")
eq(found.file.id, 0, "delete objective previews its selected data file")
eq(found.file.size, 30, "objective file uses run size")
local targeted_run = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.DELETE_FILE, "Security Files", 0, 30, 6)
local wrong_system_search = Actions.begin_search(
    targeted_run, run_node, 5)
eq(wrong_system_search, nil,
    "matching node classes do not expose a run objective in another system")
eq(targeted_run.status, E.MatrixRunStatus.ACTIVE,
    "searching a same-class node outside the target leaves the run active")
local wrong_system_erase = Actions.erase(
    targeted_run, run_node, 5)
eq(wrong_system_erase.text, Actions.TEXT.ERASED,
    "Erase cannot complete a Matrix objective in another system")
eq(targeted_run.status, E.MatrixRunStatus.ACTIVE,
    "wrong-system Erase leaves the objective unfinished")
local target_system_search = Actions.begin_search(
    targeted_run, run_node, 6)
eq(target_system_search.file.run_objective, true,
    "the assigned system exposes its matching datafile objective")
local upload_run = matrix_run(E.MatrixRunStatus.ACTIVE, "datastore",
    E.MatrixRunAction.UPLOAD_FILE, "Security Files", nil, 30)
local upload = Actions.begin_search(upload_run, run_node)
eq(upload.text, Actions.TEXT.UPLOAD_DONE,
    "upload objective completes immediately without a download prompt")
eq(upload_run.status, E.MatrixRunStatus.COMPLETED,
    "immediate objective upload completes the run")
local source_decker = { attributes = { [4] = 6 }, skills = { [7] = 6 } }
local source_deck = { persona = { [3] = 5 } }
local search_bounds = {}
local ordinary_search = Actions.finish_search(
    { class = 9 }, source_decker, source_deck, nil,
    function(bound) search_bounds[#search_bounds + 1] = bound; return 0 end)
eq(ordinary_search.file.id, 45,
    "ordinary search reads class and source RNG for file ID")
eq(ordinary_search.file.size, 20,
    "ordinary search reads the source size roll")
eq(table.concat(search_bounds, ","), "10,5,5",
    "ordinary search draws detection, file ID, and size in ROM order")
local missed_search = Actions.finish_search(
    { class = 9 }, { attributes = { [4] = 0 }, skills = { [7] = 0 } },
    { persona = { [3] = 0 } }, nil,
    function(bound) return bound - 1 end)
eq(missed_search.text, Actions.TEXT.NOTHING,
    "detection failure uses Int, Computer, and Sensor from typed records")
local special_facts = { system_id = 18,
    tips_page2_flags = 0, world_event_flags = 0 }
eq(SpecialDatastore.can_match(nil, 3), false,
    "special datastore handler rejects missing typed facts")
eq(SpecialDatastore.can_match(special_facts, 3), true,
    "system 18 class 3 needs its FFFC34 page-2 word")
eq(SpecialDatastore.can_match(
    { system_id = 21, tips_page3_flags = 0,
      world_event_flags = 0 }, 3), false,
    "system 21 class 3 waits for its group-item fact")
eq(SpecialDatastore.can_match(
    { system_id = 21, tips_page3_flags = 0,
      world_event_flags = 0, group_item_flags = 0 }, 3), true,
    "system 21 class 3 is enabled when its group-item fact exists")
eq(SpecialDatastore.can_match(
    { system_id = 19, tips_page3_flags = 0,
      world_event_flags = 0 }, 12), true,
    "system 19 class 12 uses the source FFFC1E and FFFC38 facts")
eq(SpecialDatastore.can_match(
    { system_id = 19, tips_page3_flags = 0,
      world_event_flags = 0 }, 2), false,
    "system 19 class 2 has no special-handler branch")
eq(SpecialDatastore.can_match(
    { system_id = 19, tips_page3_flags = 0,
      world_event_flags = 0 }, 0), false,
    "system 19 class 0 stays gated while its FFFC27 and FFFC26 facts are absent")
local system_19_johnson_fact = { system_id = 19, tips_page3_flags = 0,
    world_event_flags = 0, mr_johnsons_met_flags = 0x08 }
eq(SpecialDatastore.can_match(system_19_johnson_fact, 0), true,
    "system 19 class 0 uses the known FFFC27 bit-3 route")
eq(SpecialDatastore.matches(system_19_johnson_fact, 0), true,
    "system 19 class 0 returns its first-time story clue")
eq((system_19_johnson_fact.tips_page3_flags >> 14) & 1, 1,
    "system 19 class 0 commits its page-3 clue")
eq(SpecialDatastore.can_match(
    { system_id = 19, tips_page3_flags = 1 << 14,
      world_event_flags = 0, mr_johnsons_met_flags = 0x08 }, 0), false,
    "system 19 class 0 remains gated when it would need unknown FFFC26")
eq(SpecialDatastore.can_match(
    { system_id = 16, tips_page3_flags = 0,
      world_event_flags = 0 }, 1), false,
    "system 16 class 1 waits for the wallet balance it awards")
eq(SpecialDatastore.can_match(
    { system_id = 16, tips_page3_flags = 0,
      world_event_flags = 0, nuyen = 0 }, 1), true,
    "system 16 class 1 is enabled when the wallet is available")
eq(SpecialDatastore.can_match(
    { system_id = 23, tips_page3_flags = 0,
      world_event_flags = 0 }, 9), true,
    "system 23 class 9 uses the supplied FFFC38 and FFFC1E facts")
local system_23_facts = { system_id = 23, tips_page3_flags = 0,
    world_event_flags = 0 }
eq(SpecialDatastore.matches(system_23_facts, 9), true,
    "system 23 class 9 returns its special datafile")
eq(system_23_facts.tips_page3_flags, 1 << 11,
    "system 23 class 9 commits source Tips & Clues page 3 bit 11")
eq(SpecialDatastore.can_match(
    { system_id = 23, tips_page3_flags = 0,
      world_event_flags = 0 }, 13), false,
    "unavailable story facts leave system 23 class 13 unresolved")
local system_23_johnson_fact = { system_id = 23, tips_page3_flags = 0,
    world_event_flags = 0, mr_johnsons_met_flags = 0x08 }
eq(SpecialDatastore.can_match(system_23_johnson_fact, 12), true,
    "system 23 class 12 uses the known FFFC27 bit-3 route")
eq(SpecialDatastore.matches(system_23_johnson_fact, 12), true,
    "system 23 class 12 returns its first-time story clue")
eq((system_23_johnson_fact.tips_page3_flags >> 9) & 1, 1,
    "system 23 class 12 commits its page-3 clue")
local story_search = Actions.finish_search(
    { class = 3 }, source_decker, source_deck, special_facts,
    function() error("story datastores do not roll for random data") end)
eq(story_search.story, true,
    "source special datastore can return its notebook file before detection")
eq(story_search.text, Actions.TEXT.STORY_FILE,
    "special datastore selects the ROM notebook message")
eq((special_facts.tips_page2_flags >> 28) & 1, 1,
    "special datastore commits its source event bit in Lua state")
local virology_facts = { system_id = 16, tips_page3_flags = 0x7C000000,
    tips_page2_flags = 0, world_event_flags = 0, nuyen = 0xFFFFFFFF }
eq(SpecialDatastore.matches(virology_facts, 1), true,
    "system 16 class 1 completes when source flag bits 26 through 30 are set")
eq(virology_facts.tips_page3_flags, 0xFC000000,
    "system 16 class 1 sets source flag bit 31")
eq(virology_facts.nuyen, 49999,
    "system 16 class 1 applies the 32-bit ROM wallet addition")
local low_storage = { storage = 0, files = {}, ratings = {} }
for i = 0, 4 do low_storage.files[i] = { id = -1, size = 0 } end
for i = 0, 11 do low_storage.ratings[i] = 0 end
local _, low_storage_text = Actions.store(
    low_storage, nil, { class = 9, colour = 1 },
    { id = 45, size = 20 }, function() return 0 end)
eq(low_storage_text, Actions.TEXT.NO_STORAGE,
    "insufficient deck storage uses the ROM two-part message")
local full_file_slots = { storage = 100, files = {}, ratings = {} }
for i = 0, 4 do full_file_slots.files[i] = { id = i, size = 0 } end
for i = 0, 11 do full_file_slots.ratings[i] = 0 end
local _, full_file_text = Actions.store(
    full_file_slots, nil, { class = 9, colour = 1 },
    { id = 45, size = 20 }, function() return 0 end)
eq(full_file_text, Actions.TEXT.FIVE_FILES,
    "five occupied file slots use the ROM capacity message")
local result = Actions.erase(run, run_node)
eq(result.text, Actions.TEXT.ERASE_OBJECTIVE, "objective erase text")
eq(run.status, E.MatrixRunStatus.COMPLETED, "objective erase completes run")
local ordinary_erase_run = matrix_run(E.MatrixRunStatus.ACTIVE,
    "datastore", E.MatrixRunAction.DELETE_FILE, "Financial Data", 0, 30)
local ordinary_erase = Actions.erase(ordinary_erase_run, run_node)
eq(ordinary_erase.text, Actions.TEXT.ERASED,
    "non-objective Erase uses the ordinary ROM message")
eq(ordinary_erase_run.status, E.MatrixRunStatus.ACTIVE,
    "non-objective Erase leaves the active run incomplete")
local erase_nonzero_value_run = matrix_run(E.MatrixRunStatus.ACTIVE,
    "datastore", E.MatrixRunAction.DOWNLOAD_FILE, "Security Files", 1, 30)
local erase_nonzero_value = Actions.erase(erase_nonzero_value_run,
    run_node)
eq(erase_nonzero_value.text, Actions.TEXT.ERASED,
    "download objective does not complete through Erase")
eq(erase_nonzero_value_run.status, E.MatrixRunStatus.ACTIVE,
    "download objective remains active after Erase")
local no_run_erase = Actions.erase(nil, run_node)
eq(no_run_erase.text, Actions.TEXT.ERASED,
    "Erase without an imported active run remains a normal file erase")
local gate_state = { attempts = 0 }
eq(Actions.gate(3, 2, { colour = 1, security = 4 }, gate_state,
    function(dice, tn) eq(dice, 4, "node action pool uses minimum"); eq(tn, 4, "node action uses security"); return 1 end),
    true, "successful node action gate")
eq(gate_state.attempts, 1, "node action gate counts attempt")
local action_runner = {
    attributes = { [1] = 8, [4] = 4 },
    skills = { [7] = 7 },
}
local action_delay, action_computer = Actions.runner_action_inputs(
    action_runner, { response = 2 }, E.MatrixModeResult.ATTACK)
eq(action_delay, 12,
    "ROM action delay uses two-thirds mode and wrapped response inputs")
eq(action_computer, 7, "node action gate reads Computer at runner +0x88")
action_delay = Actions.runner_action_inputs(
    action_runner, { response = 2 }, E.MatrixDeckMode.BALANCED)
eq(action_delay, 8, "balanced action delay halves the wrapped total")
action_delay = Actions.runner_action_inputs(
    action_runner, { response = 2 }, E.MatrixDeckMode.DEFENSE)
eq(action_delay, 5, "defense action delay uses the one-third branch")
local runner_gate_state = { attempts = 0 }
eq(Actions.gate_runner_action(action_runner, { response = 2 },
    E.MatrixModeResult.ATTACK, { colour = 3, security = 4 }, runner_gate_state,
    function(dice, tn)
        eq(dice, 12, "ROM runner action dice use min plus Computer")
        eq(tn, 4, "runner action roll uses node security")
        return 1
    end), true, "runner source fields enter the node-action gate")
eq(runner_gate_state.attempts, 1,
    "runner action gate increments the alert-event counter")

-- Program effects and frame services return named outcomes for the presenter.
local healed = Programs.medic(deck, function(dice, tn)
    eq(dice, 3, "Medic uses program rating")
    eq(tn, 4, "Medic target follows persona band")
    return 2
end)
eq(healed.healed, 2, "Medic reports successes rolled")
eq(deck.persona, 10, "Medic clamps persona at ten")
local analyze_deck = {
    mpcp = 8, response = 2, sensor = 5,
    ratings = { [11] = 1 },
}
local analyze_node = { colour = 2, security = 3 }
local analyze_state = { mode = E.MatrixModeResult.ATTACK,
                        alert = 0, smoke = 0 }
local analyze_roll_count = 0
local analyzed = Programs.analyze(
    analyze_deck, { d81c = 3, computer = 6 }, analyze_node,
    analyze_state,
    function(dice, tn)
        analyze_roll_count = analyze_roll_count + 1
        if analyze_roll_count == 1 then
            eq(dice, 1, "Analyze decker dice include the color penalty")
            eq(tn, 3, "Analyze decker roll uses Matrix alert TN")
            return 2
        end
        eq(dice, 3, "Analyze opposes with node security")
        eq(tn, 5, "Analyze opposition uses deck Sensor")
        return 0
    end)
eq(analyzed.ok, true, "Analyze returns its successful outcome")
eq(analyzed.net, 2, "Analyze floors and reports net successes")
eq(analyzed.analysis_level, 2, "Analyze returns a semantic discovery level")
eq(analyzed.frames, 0x3C, "Analyze starts the ROM profile duration")
local analyze_failed = Programs.analyze(
    analyze_deck, { d81c = 3, computer = 6 }, analyze_node,
    analyze_state, function() return 0 end)
eq(analyze_failed.fail, true, "Analyze failure follows the unload path")
local analyze_transition = {}
eq(Session.begin_analyze_transition(analyze_transition, 1, 10), true,
   "Analyze transition accepts the active program slot")
for tick = 1, 18 do
    Session.advance_analyze_transition(analyze_transition)
end
eq(analyze_transition.analyze_transition.runner_phase, 1,
   "Analyze raises the runner after five source calls")
eq(analyze_transition.analyze_transition.ring_phase, 3,
   "Analyze selects the source ring phase from its signed timer")
for tick = 19, 60 do
    Session.advance_analyze_transition(analyze_transition)
end
eq(analyze_transition.analyze_transition.remaining, 0,
   "Analyze profile remains active for countdown zero")
eq(analyze_transition.analyze_transition.ring_phase, 0,
   "Analyze countdown zero selects phase zero")
local analyze_advanced, analyze_finished =
    Session.advance_analyze_transition(analyze_transition)
eq(analyze_advanced, true, "Analyze advances its final source call")
eq(analyze_finished.result_state, 10,
   "Analyze publishes the completed Resisted result")
eq(analyze_transition.analyze_transition, nil,
   "Analyze clears the transition after the signed countdown")
eq(analyze_transition.analyze_runner_idle, true,
   "Analyze restores the idle runner after completion")
local kind = C.ic_kind(7, 1)
eq(kind, "trace", "active layered IC attacks as trace")
eq(C.ic_kind(12, 0), "passive", "inactive layered IC remains passive")
eq(C.ic_interval({ ic_reaction = 5 }, -1), 440, "IC interval includes source alert bonus")
local value, timer, expired = C.tick_shroud(1, 0)
eq(value, 0, "shroud decrements on timeout")
eq(timer, C.SHROUD_PERIOD, "shroud reloads source period")
eq(expired, true, "zero shroud expires")
local x, y = Programs.dump_jitter(0x8185)
eq(x, -5, "dump jitter signed x")
eq(y, -1, "dump jitter signed y")
end

-- A type-six objective is projected from the Lua run registry into the
-- Matrix action transaction, then saved with that active offer.
local Registry = require("run_registry")
local run_probe = {
    prepare_offer = function() end,
    resolve_field_event = function() return { action = "no_action" } end,
}
local committed_world
game = {
    modules = { run_support = require("run_support") },
    runs = { registry = Registry, matrix_objective = run_probe },
    matrix_world_state = function()
        return { run = { state = 9 }, field = {}, actors = {} }
    end,
    commit_matrix_world_state = function(world)
        committed_world = world
    end,
}
-- 9d69a571 validates saved offers against the joined ROM offer row; use the
-- Big Rhino MATRIX_SIMPLE row (progress 2, base 425, scale 1).
Registry.active_offer = {
    id = "matrix_objective",
    category = "matrix",
    objective = "retrieve_matrix_data",
    completion_condition = "source_matrix_objective_satisfied",
    runner_progress_delta = 2,
    contact_id = E.RunContact.BIG_RHINO,
    offer_variant_id = "matrix_simple",
    payment = {
        base_amount = 425, negotiated_amount = 425, negotiation_skill = 0,
        negotiation_bound = 1, negotiation_roll = 0,
    },
    matrix_run_state = {
        status = E.MatrixRunStatus.ACTIVE,
        objective = {
            node = "datastore",
            action = E.MatrixRunAction.DELETE_FILE,
            file_class = "Security Files",
            file_id = 0,
        },
        file_size = 30,
        completion_marker_present = true,
        completion_recorded = false,
        target_system_id = 6,
    },
}
local world = Session.world_state(game)
eq(world.run.status, E.MatrixRunStatus.ACTIVE,
    "Matrix world projection uses the accepted Lua objective")
world.run.status = E.MatrixRunStatus.COMPLETED
world.run.completion_marker_present = false
world.run.completion_recorded = true
eq(Registry.active_matrix_run_state().status, E.MatrixRunStatus.ACTIVE,
    "Matrix projection is a copy until committed")
Session.commit_world_state(game, world)
eq(Registry.active_matrix_run_state().status, E.MatrixRunStatus.COMPLETED,
    "Matrix objective commit updates the Lua run registry")
eq(committed_world.run, nil,
    "native world commit does not retain a second Matrix run state")
local saved_runs = Registry.save_state()
local valid_runs = Registry.validate_saved_state(saved_runs)
eq(valid_runs, true, "saved Matrix objective state passes Lua validation")
Registry.active_offer = nil
Registry.restore_saved_state(saved_runs)
eq(Registry.active_matrix_run_state().target_system_id, 6,
    "save restore retains the Matrix objective's target system")
saved_runs.active_offer.matrix_run_state.target_system_id = 25
local invalid_runs = Registry.validate_saved_state(saved_runs)
eq(invalid_runs, false,
    "saved Matrix objective rejects a target outside the ROM system catalog")

-- All five ROM offer contexts contain eight six-byte records at F3EBC. The
-- native Lua model retains named variants, progress, payment, and negotiation
-- fields; the source initializer byte stays at the extraction boundary.
local OfferData = require("data.johnson_run_offers")
-- dialogue.lua asserts the authored JSON catalogs in game.content.
do
    local Json = require("json")
    local function load(path)
        return assert(Json.decode(__matrix_read_gameplay_file(
            "dialogue/" .. path .. ".json")))
    end
    game.content = {
        speakers = load("speakers"), menus = load("menus"),
        contacts = load("contacts"), conditions = load("conditions"),
        cinematics = load("cinematics"),
        conversations = {
            vid_phone_services = load("conversations/vid_phone_services"),
        },
    }
end
local Dialogue = require("dialogue")
local expected_context_rows = {
    [E.RunContact.JUMP_HOUSE] = {
        {"ghoul_bounty",1,10,3}, {"bodyguard_escort",1,45,1},
        {"package_delivery",1,35,1}, {"bodyguard_escort",1,50,1},
        {"package_delivery",1,40,1}, {"bodyguard_escort",1,55,1},
        {"package_delivery",1,45,1}, {"ghoul_bounty",1,20,2},
    },
    [E.RunContact.MATCHSTICK] = {
        {"matrix_moderate",3,2600,4}, {"bodyguard_escort",1,200,2},
        {"employee_recruitment",2,500,5}, {"package_recovery",2,600,5},
        {"employee_recruitment_two_site_pool",4,900,5},
        {"package_recovery_two_site_pool",4,1000,5},
        {"package_delivery",1,180,2}, {"gang_contract",2,160,3},
    },
    [E.RunContact.BIG_RHINO] = {
        {"matrix_simple",2,425,1}, {"gang_contract",2,190,2},
        {"package_delivery",1,180,1}, {"ghoul_bounty",1,40,1},
        {"package_recovery",2,550,4}, {"employee_recruitment",2,450,4},
        {"bodyguard_escort",1,200,1}, {"package_delivery",1,190,1},
    },
    [E.RunContact.WANDERER] = {
        {"gang_contract",1,230,1}, {"matrix_moderate",2,3000,2},
        {"matrix_expert",4,6350,5}, {"package_delivery",0,250,1},
        {"package_recovery_two_site_pool",3,1700,4},
        {"package_recovery_renraku",5,4500,10},
        {"employee_recruitment_renraku",5,4000,10},
        {"matrix_simple",1,500,1},
    },
    [E.RunContact.ICARUS_DESCENDING] = {
        {"matrix_moderate",3,2550,5}, {"matrix_expert",5,6000,10},
        {"employee_recruitment_two_site_pool",4,1200,5},
        {"package_recovery_two_site_pool",4,1400,5},
        {"employee_recruitment_renraku",6,3500,15},
        {"package_recovery_renraku",6,3850,15},
        {"matrix_expert",5,6100,10}, {"matrix_moderate",4,2750,5},
    },
}
local contact_ids = {
    E.RunContact.JUMP_HOUSE,
    E.RunContact.MATCHSTICK,
    E.RunContact.BIG_RHINO,
    E.RunContact.WANDERER,
    E.RunContact.ICARUS_DESCENDING,
}
for _, contact_id in ipairs(contact_ids) do
    local rows = OfferData.contacts[contact_id]
    eq(#rows, 8, "Johnson offer row count for " .. contact_id)
    for index = 1, 8 do
        local row = rows[index]
        local expected = expected_context_rows[contact_id][index]
        eq(row.variant_id, expected[1], "semantic offer variant row")
        eq(row.runner_progress_delta, expected[2], "progress row")
        eq(row.base_payment, expected[3], "base payment row")
        eq(row.negotiation_scale, expected[4], "negotiation scale row")
    end
end
eq(Dialogue.johnson_interaction_profiles["wanderer.vigore_jarl"].run_contact_id,
    E.RunContact.WANDERER, "Wanderer profile selects the Wanderer contact")

-- Wanderer context 3, slot 7 selects source initializer 6. Its simple
-- Matrix objective follows the ROM objective -> system -> size RNG sequence.
local Support = require("run_support")
local MatrixRun = require("run_matrix_objective")
local random_call = 0
local random_rolls = { 7 }
game = {
    modules = { run_support = Support },
    runs = {
        registry = Registry,
        matrix_objective = MatrixRun,
        matrix_objective_probe = run_probe,
    },
    services = {
        call = function(service, operation)
            if service == "runner" and operation == "negotiation_skill" then
                return "skill_0"
            end
            assert(service == "random", "unexpected test service " .. service)
            random_call = random_call + 1
            return "roll_" .. tostring(random_rolls[random_call] or 0)
        end,
    },
    matrix_world_state = function()
        return { run = { completion_marker_present = true } }
    end,
}
local selected_run = Registry.prepare_offer_for_contact(E.RunContact.WANDERER)
local matrix_offer = Registry.current_offer
eq(selected_run, "matrix_objective", "context selects Matrix Lua module")
eq(matrix_offer.offer_variant_id, "matrix_simple",
    "offer retains its semantic Matrix variant")
eq(matrix_offer.id, E.RunId.MATRIX_OBJECTIVE,
    "offer retains its semantic run identity")
eq(matrix_offer.contact_id, E.RunContact.WANDERER,
    "offer retains its semantic Johnson contact")
eq(matrix_offer.source_run_type, nil,
    "source handler selectors do not enter the Lua offer state")
eq(matrix_offer.source_offer_context, nil,
    "source context selectors do not enter the Lua offer state")
eq(matrix_offer.matrix_run_state.status, E.MatrixRunStatus.PENDING,
    "simple Matrix objective begins pending system selection")
eq(matrix_offer.matrix_run_state.objective.node, "datastore",
    "Matrix candidate targets a datastore")
eq(matrix_offer.matrix_run_state.objective.file_class, "Security Files",
    "Matrix candidate names its target datastore category")
eq(matrix_offer.matrix_run_state.objective.action,
    E.MatrixRunAction.DOWNLOAD_FILE,
    "Matrix candidate carries a named file operation")
eq(matrix_offer.matrix_run_state.objective.file_id, 20,
    "Matrix candidate names the requested file")
eq(matrix_offer.matrix_run_state.file_size, 10,
    "Matrix file size follows the source one-to-three roll")
eq(matrix_offer.matrix_run_state.completion_marker_present, true,
    "Matrix initializer keeps the host completion fact")
eq(matrix_offer.matrix_run_state.completion_recorded, false,
    "new objective begins without a recorded completion")
eq(matrix_offer.matrix_run_state.target_system_id, 2,
    "Matrix candidate uses an allowed simple-band system")
assert(matrix_offer.objective_text:find("A small R&D firm", 1, true),
    "Matrix offer prefixes its target system for source rows 0 through 16")
checks = checks + 1
assert(matrix_offer.objective_text:find("||", 1, true) and
       not matrix_offer.objective_text:find("\n", 1, true),
    "Matrix offer uses the shared dialogue paragraph separators")
checks = checks + 1
eq(Registry.resolve_current_offer("yes"), "accepted",
    "context-generated offer uses the shared decision path")
local generated_offer_save = Registry.save_state()
local generated_save_valid = Registry.validate_saved_state(generated_offer_save)
eq(generated_save_valid, true,
    "source context and Matrix state survive run-save validation")
Registry.active_offer = nil

game.runs.gang_contract = require("run_gang_contract")
game.runs.package_recovery = require("run_package_recovery")
game.runs.employee_recruitment = require("run_employee_recruitment")
local function prepare_contact_row(contact_id, rolls)
    random_call = 0
    random_rolls = rolls
    return Registry.prepare_offer_for_contact(contact_id), Registry.current_offer
end

local gang_id, gang_offer = prepare_contact_row(
    E.RunContact.MATCHSTICK, { 7, 0, 0, 0 })
eq(gang_id, "gang_contract", "gang source initializer loads its own module")
eq(gang_offer.offer_variant_id, "gang_contract",
    "gang offer retains semantic variant")
eq(gang_offer.route.target.location_id, "ork_gang_hideout",
    "gang target uses its semantic place identity")
eq(gang_offer.contract_variant_id, "building_vandalism",
    "gang copy stores a semantic contract variant")
assert(Support.gang_contract_variants[1].text:find("||", 1, true) and
       not Support.gang_contract_variants[1].text:find("\n", 1, true),
    "gang copy uses the shared dialogue paragraph separators")
checks = checks + 1
Registry.resolve_current_offer("no")

local recovery_id, recovery_offer = prepare_contact_row(
    E.RunContact.WANDERER, { 4, 0, 0, 0 })
eq(recovery_id, "package_recovery",
    "two-site package recovery loads its own module")
eq(recovery_offer.offer_variant_id, "package_recovery_two_site_pool",
    "recovery preserves its two-site target-pool choice")
eq(recovery_offer.route.target.location_id, "ares_corporate_offices",
    "recovery target uses the semantic place from the ROM two-site pool")
Registry.resolve_current_offer("no")

local employee_id, employee_offer = prepare_contact_row(
    E.RunContact.WANDERER, { 6, 0, 0 })
eq(employee_id, "employee_recruitment",
    "Renraku employee recovery loads its own module")
eq(employee_offer.offer_variant_id, "employee_recruitment_renraku",
    "employee offer preserves its fixed-site variant")
eq(employee_offer.route.target.location_id, "renraku_offices",
    "employee offer uses the semantic fixed Renraku place")
eq(employee_offer.employee_id, "mike_nana",
    "employee identity comes from the semantic client catalog")
Registry.resolve_current_offer("no")

local moderate_id, moderate_offer = prepare_contact_row(
    E.RunContact.WANDERER, { 1, 0, 14, 0, 0 })
eq(moderate_id, "matrix_objective",
    "moderate Matrix initializer reaches the Matrix module")
eq(moderate_offer.matrix_run_state.status, E.MatrixRunStatus.PENDING,
    "moderate Matrix objective begins pending system selection")
eq(moderate_offer.matrix_run_state.target_system_id, 10,
    "moderate candidate retries until its system is in the source band")
Registry.resolve_current_offer("no")

local expert_id, expert_offer = prepare_contact_row(
    E.RunContact.WANDERER, { 2, 0, 4, 0, 0 })
eq(expert_id, "matrix_objective",
    "expert Matrix initializer reaches the Matrix module")
eq(expert_offer.matrix_run_state.status, E.MatrixRunStatus.PENDING,
    "expert Matrix objective begins pending system selection")
eq(expert_offer.matrix_run_state.target_system_id, 7,
    "expert candidate retains an allowed source system")
Registry.resolve_current_offer("no")

-- Exercise the live shared Johnson script, not just the registry helper. The
-- Wanderer selects its profile context and has no invented generic work page.
local Johnson = require("johnson")
game.modules.dialogue = Dialogue
game.location = { text_profile = "wanderer.vigore_jarl" }
game.state = { wanderer_vigore_jarl_intro_seen = false }
random_call = 0
random_rolls = { 7, 0, 0, 0, 0 }
local johnson_menu_calls = 0
local johnson_intro_events = {}
local accepted_offer_context
game.ui = {
    say = function(event_id) johnson_intro_events[#johnson_intro_events + 1] = event_id end,
    say_text = function() end,
    menu = function()
        johnson_menu_calls = johnson_menu_calls + 1
        return johnson_menu_calls == 1 and "work" or "leave"
    end,
    prompt_text = function()
        accepted_offer_context = Registry.current_offer.contact_id
        return "no"
    end,
}
eq(Johnson.interaction(), "leave", "shared Johnson interaction returns to field")
eq(accepted_offer_context, E.RunContact.WANDERER,
    "work choice passes the live Wanderer context into offer selection")
eq(#johnson_intro_events, 2,
    "Wanderer first-visit introduction shows its two source pages")
eq(game.state.wanderer_vigore_jarl_intro_seen, true,
    "Wanderer introduction fact is set after its pages")
eq(johnson_intro_events[1], "first_impression",
    "first Wanderer page uses its location profile")
eq(johnson_intro_events[2], "first_meeting",
    "second Wanderer page uses its location profile")

-- The ROM gives Scene 14's post-Ito branch priority over first-visit and
-- run-return menus. It clears only context 3, then shows the empty-booth page
-- with the source's blank portrait profile and no choices.
local old_active_offer = Registry.active_offer
local old_pending_return = Registry._pending_johnson_return
Registry.active_offer = {
    id = "matrix_objective_probe",
    contact_id = E.RunContact.WANDERER,
}
Registry._pending_johnson_return = {
    run_id = "matrix_objective_probe",
    contact_id = E.RunContact.WANDERER,
}
game.location = { text_profile = "wanderer.vigore_jarl" }
game.state = {
    story_flag_bit_16 = true,
    wanderer_vigore_jarl_intro_seen = false,
}
local vacated_say
local vacated_menu_called = false
game.ui = {
    say = function(event_id, speaker_id, text_profile)
        vacated_say = { event_id, speaker_id, text_profile }
    end,
    say_text = function() end,
    menu = function()
        vacated_menu_called = true
        error("ROM booth-vacated descriptor has no choices")
    end,
}
eq(Johnson.interaction(), "booth_vacated",
    "post-Ito Scene 14 interaction ends on the empty-booth page")
eq(vacated_say[1], "booth_vacated",
    "post-Ito branch uses the empty-booth ROM text")
eq(vacated_say[2], "wanderer_booth_narrator",
    "post-Ito branch selects the source blank portrait profile")
eq(vacated_say[3], "wanderer.vigore_jarl",
    "post-Ito page retains the Wanderer text catalog")
eq(vacated_menu_called, false,
    "source sentinel prevents a booth-vacated choice menu")
eq(Registry.active_offer, nil,
    "0x1E68C clears the matching Wanderer offer")
eq(Registry._pending_johnson_return, nil,
    "0x1E68C clears the matching pending Wanderer return")
eq(game.state.wanderer_vigore_jarl_intro_seen, false,
    "post-Ito branch bypasses the first-visit intro flag")
Registry.active_offer = old_active_offer
Registry._pending_johnson_return = old_pending_return

-- A completed context-3 return uses the ROM's alternate-credit root.
local return_run = game.runs.matrix_objective_probe
local old_present_completion = return_run.present_completion
local old_take_return = Registry.take_pending_johnson_return
local return_presented = false
return_run.present_completion = function()
    return_presented = true
end
Registry.take_pending_johnson_return = function()
    return {
        run_id = "matrix_objective_probe",
        contact_id = E.RunContact.WANDERER,
    }
end
game.location = { text_profile = "wanderer.vigore_jarl" }
game.state = {}
local return_root_menu
game.ui = {
    say = function() end,
    say_text = function() end,
    menu = function(menu_id)
        return_root_menu = menu_id
        return "leave"
    end,
    prompt_text = function() return "no" end,
}
eq(Johnson.interaction(), "leave",
    "completed Wanderer run returns through the alternate root")
eq(return_presented, true,
    "completed Wanderer offer presents its Lua completion")
eq(return_root_menu, "conditional_opening",
    "FFFBEC/FFFC1C gate selects the conditional credit root")
Registry.take_pending_johnson_return = old_take_return
return_run.present_completion = old_present_completion

-- Mortimer's Big Rhino work choice uses source context 2 and its own opening
-- page. Information remains separate until its ROM question and contact-
-- purchase branches are connected.
local old_present_offer_for_contact = Registry.present_offer_for_contact
local old_take_pending_johnson_return = Registry.take_pending_johnson_return
local mortimer_offer_context
local mortimer_events = {}
local mortimer_menu_call = 0
Registry.present_offer_for_contact = function(contact_id)
    mortimer_offer_context = contact_id
    return "declined"
end
Registry.take_pending_johnson_return = function() return nil end
game.modules.dialogue = Dialogue
game.location = { text_profile = "big-rhino.mortimer_reed" }
game.state = {}
game.ui = {
    say = function(event_id)
        mortimer_events[#mortimer_events + 1] = event_id
    end,
    say_text = function() end,
    menu = function(menu_id)
        mortimer_menu_call = mortimer_menu_call + 1
        eq(menu_id, "opening", "Mortimer uses his source opening menu")
        return mortimer_menu_call == 1 and "work" or "leave"
    end,
    prompt_text = function() return "no" end,
}
eq(Johnson.interaction(), "leave",
    "Mortimer interaction returns to the field after leave")
eq(mortimer_offer_context, E.RunContact.BIG_RHINO,
    "Mortimer work selection uses ROM Johnson context two")
eq(mortimer_events[1], "booth_entry",
    "Mortimer first contact shows the ROM booth-entry dialogue")
eq(mortimer_events[2], "work_opening",
    "Mortimer work selection shows its ROM work opening")
game.ui.menu = function(menu_id)
    if menu_id == "repeat_opening" then return "information" end
    if menu_id == "information" then return "back" end
    if menu_id == "opening_after_information" then return "leave" end
    error("unexpected Mortimer menu " .. tostring(menu_id))
end
eq(Johnson.interaction(), "leave",
    "Mortimer information returns to its own opening menu")
eq(mortimer_events[5], "information_opening",
    "Mortimer information uses its own ROM introduction")
Registry.present_offer_for_contact = old_present_offer_for_contact
Registry.take_pending_johnson_return = old_take_pending_johnson_return

print("matrix Lua rules: " .. checks .. " checks passed")
)lua";

    if (luaL_dostring(state, kRules) != LUA_OK) {
        std::cerr << "Matrix Lua rules failed: "
                  << lua_tostring(state, -1) << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    lua_State* state = luaL_newstate();
    if (state == nullptr) {
        std::cerr << "Could not create Lua state for Matrix rule checks.\n";
        return 1;
    }
    g_script_root = std::filesystem::path(SHADOWRUN_GAMEPLAY_SCRIPT_ROOT);
    luaL_requiref(state, "_G", luaopen_base, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(state, 1);
    lua_pushcfunction(state, read_gameplay_file);
    lua_setglobal(state, "__matrix_read_gameplay_file");
    lua_newtable(state);
    lua_setglobal(state, "__matrix_test_modules");
    lua_pushcfunction(state, load_test_module);
    lua_setglobal(state, "require");

    const bool passed = run_rules(state);
    lua_close(state);
    return passed ? 0 : 1;
}
