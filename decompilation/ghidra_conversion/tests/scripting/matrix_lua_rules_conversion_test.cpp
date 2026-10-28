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
    knowledge_ranks = {},
}
eq(Session.set_current_node_knowledge(knowledge_state, 4), true,
   "node entry replaces the prior knowledge nibble with four")
eq(Session.merge_current_node_knowledge(knowledge_state, 1), true,
   "Analyze ORs new knowledge into the existing source nibble")
eq(Session.current_node_knowledge(knowledge_state), 5,
   "Analyze retains the entry marker while adding its result bit")
eq(Session.merge_current_node_knowledge(knowledge_state, 2), true,
   "later Analyze results continue merging into the same node byte")
eq(Session.current_node_knowledge(knowledge_state), 7,
   "knowledge rank preserves the ROM bitwise-OR representation")

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
eq(routed_spu_status.knowledge_rank, 0,
    "unvisited Matrix node starts with no IC knowledge")
eq(routed_spu_status.file_class_label, "",
    "unrevealed node hides its file-class label")
eq(routed_spu_status.action_difficulty, -1,
    "unrevealed node hides its security rating")
eq(Session.traversal_view(routed_spu_state).palette_theme, 0,
    "ROM keeps an unrevealed world model on neutral palette row zero")
eq(Session.set_current_node_knowledge(routed_spu_state, 4), true,
    "node-action entry sets the source knowledge rank")
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
eq(Session.set_current_node_knowledge(active_spu_state, 0), true,
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
local remembered_cell = remembered_node.y * MX.W + remembered_node.x
local map_memory = {
    system_id = 15,
    cells = { [remembered_cell] = 0x42 },
    revealed = { [0] = 0xFF },
}
local memory_started, memory_state = Session.begin(
    15, false, 13, nil, false, nil, map_memory)
eq(memory_started, true, "same-system Matrix entry accepts FFFB30 memory")
eq(Session.current_node_knowledge(memory_state), 4,
    "direct entry restores the current node's FFFB30 knowledge nibble")
eq(memory_state.map_colours[remembered_cell], 2,
    "direct entry restores the FFFB30 visited-node color nibble")
eq(memory_state.known[0], true,
    "same-system direct entry restores the FFFAA4 revealed-cell bitmap")
eq(Session.begin_world_entry(memory_state), true,
    "restored map knowledge participates in source world-mode selection")
eq(Session.traversal_view(memory_state).world_effect_mode, 3,
    "remembered rank reveals the System 15/node 13 Blaster effect without FFDF84")
Session.set_current_node_knowledge(memory_state, 4)
eq(memory_state.matrix_map_memory.cells[remembered_cell], 0x42,
    "rank updates preserve the FFFB30 security-color nibble")
local reopened_started, reopened_state = Session.begin(
    15, false, 13, nil, false, nil, memory_state.matrix_map_memory)
eq(reopened_started, true, "reopening the same system reuses its RAM memory")
eq(Session.current_node_knowledge(reopened_state), 4,
    "same-system re-entry retains the analyzed node rank")
eq(reopened_state.map_colours[remembered_cell], 2,
    "same-system re-entry retains the visited map color")
local switched_started, switched_state = Session.begin(
    16, false, 0, nil, false, nil, reopened_state.matrix_map_memory)
eq(switched_started, true, "switching Matrix systems starts a valid session")
eq(switched_state.matrix_map_memory.cells[remembered_cell], nil,
    "a system change clears the previous FFFB30 map records")
eq(switched_state.matrix_map_memory.revealed[0], 0,
    "a system change clears the previous FFFAA4 reveal bitmap")
eq(Session.current_node_knowledge(switched_state), 0,
    "a different system does not inherit the previous node's knowledge")
Session.active = memory_previous_active
active_spu_state.phase = "link_select"
active_spu_state.link_cursor = 0
MatrixNodeAction.input(alert_screen, E.Pad.A, alert_context)
eq(active_spu_state.current_node, 0,
    "link confirmation selects the linked System Access node")
eq(alert_screen.world_entry_pending, true,
    "traversal link confirmation starts the next world-entry sequence")
eq(Session.current_node_knowledge(active_spu_state), 0,
    "the new node's knowledge rank stays unchanged during entry")
for _ = 1, 400 do
    if not alert_screen.world_entry_pending then break end
    MatrixNodeAction.input(alert_screen, 0, alert_context)
end
eq(alert_screen.world_entry_pending, false,
    "the second world-entry sequence reaches its node-action handoff")
eq(Session.current_node_knowledge(active_spu_state), 4,
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
Session.set_current_node_knowledge(paired_ice_state, 1)
paired_ice_view = Session.traversal_view(
    paired_ice_state, "Analyze-2", Session.program_status_label(10))
eq(paired_ice_view.ice_label, HUD.ic_types[0],
    "rank one reveals the IC type")
eq(paired_ice_view.ice_rating_label, "?",
    "rank one shows the IC type but hides its rating")
Session.set_current_node_knowledge(paired_ice_state, 4)
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
eq(first_session.map[139] ~= nil, true, "Matrix session copies cell 139")
eq(first_session.map[140], nil, "Matrix session does not create cell 140")
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
local targeted_entry = { state = -1, target_system_id = 6, fc1c = 0 }
eq(Session.activate_run_for_system(5, targeted_entry), true,
    "entering another system deactivates a Matrix objective")
eq(targeted_entry.state, 0,
    "an unfinished Matrix objective is inactive outside its target system")
eq(Session.activate_run_for_system(5, targeted_entry), false,
    "re-entering the same non-target system leaves its state stable")
eq(Session.activate_run_for_system(6, targeted_entry), true,
    "entering the objective system activates the Matrix run")
eq(targeted_entry.state, -1,
    "the ROM target system restores the active Matrix run state")
local completed_targeted_entry = {
    state = 1, target_system_id = 6, fc1c = 0x80,
}
eq(Session.activate_run_for_system(5, completed_targeted_entry), true,
    "fresh Matrix entry resets the prior completed-state display")
eq(completed_targeted_entry.state, 0,
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
local crash_run = { state = -1, objective = -1, value = 0, size = 0,
    fc28 = 1, fc1c = 0 }
local crash_field = { alarm = true, security_off = false, maglocks_off = false }
local alarm_actors = { { flags9e = 0x02 }, { flags9e = 0x04 } }
local crash_result = Actions.crash_system(crash_run, crash_field, alarm_actors)
eq(crash_run.state, 1, "Crash System completes a negative state and objective")
eq(crash_run.fc28, 0, "Crash System clears the completion bit")
eq(crash_run.fc1c, 0x80, "Crash System sets the run-complete flag")
eq(crash_field.alarm, false, "Crash System turns the alarm off")
eq(crash_field.security_off, true, "Crash System disables security")
eq(crash_field.maglocks_off, true, "Crash System disengages maglocks")
eq(alarm_actors[1].flags9e, 0x06, "alarm-off arms actors whose +0x9E bit 1 was set")
eq(alarm_actors[2].flags9e, 0x04, "alarm-off preserves unrelated actor flags")
eq(crash_result.step, 61, "Crash System wait uses D5=$3C plus the DBF pass")
eq(crash_result.stop_sfx, 0x30, "Crash System stops the source alarm effect")
local partial_run = { state = -1, objective = 0, fc28 = 1, fc1c = 0 }
Actions.crash_system(partial_run, {})
eq(partial_run.state, -1, "Crash System leaves a nonnegative objective incomplete")
eq(partial_run.fc28, 1, "incomplete Crash System preserves the completion bit")
local absent_run_result = Actions.crash_system(nil, {})
eq(absent_run_result.logout, true, "Crash System handles an unknown run without inventing one")

local class_27_field = { alarm = true }
local class_27_actors = { { flags9e = 0x02 } }
local class_27_result = Actions.turn_off_node(
    { class = 0x27 }, class_27_field, class_27_actors)
eq(class_27_field.alarm, false, "class 0x27 turns off the alarm")
eq(class_27_field.security_off, true, "class 0x27 disables building security")
eq(class_27_actors[1].flags9e, 0x06, "class 0x27 applies alarm actor flags")
eq(class_27_result.stop_sfx, 0x30, "class 0x27 stops the alarm sound")
eq(class_27_result.text, Actions.TEXT.OFFLINE, "class 0x27 uses ROM offline text")
local class_28_field = { alarm = true }
local class_28_result = Actions.turn_off_node({ class = 0x28 }, class_28_field)
eq(class_28_field.security_off, true, "class 0x28 disables security")
eq(class_28_field.alarm, true, "class 0x28 leaves the alarm unchanged")
eq(class_28_result.stop_sfx, nil, "class 0x28 does not stop alarm audio")
local class_29_field = { alarm = true }
Actions.turn_off_node({ class = 0x29 }, class_29_field)
eq(class_29_field.maglocks_off, true, "class 0x29 disengages maglocks")
eq(class_29_field.alarm, true, "class 0x29 leaves the alarm unchanged")
local class_2a_field = { alarm = true }
local class_2a_result = Actions.turn_off_node({ class = 0x2A }, class_2a_field)
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

-- IC visibility, layered types, and alert bonuses follow the ROM tables.
local node = { ic_condition = 10, ic_code = 7, ic_rating = 3, ic_rating2 = 5 }
eq(MX.ic_type(node, 0, false, 0), MX.UNSEEN, "unseen node")
eq(MX.ic_type(node, 0, true, 0), 0, "normal IC layer")
eq(MX.ic_type(node, 1, true, 1), 6, "active IC layer")
node.ic_condition = 0
eq(MX.ic_type(node, 1, true, 1), MX.NO_IC, "crashed IC")
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
for cell = 0, MX.LAST do matrix_map[cell] = 0xFF end
matrix_map[0] = 0
matrix_map[1] = 19
matrix_map[2] = 19
matrix_map[15] = 20
matrix_map[16] = 18
local visible = MX.reveal(matrix_map, {}, 0, 0)
eq(visible[0], true, "map reveals current node")
eq(visible[1], true, "map reveals first connector")
eq(visible[2], true, "map follows connector east")
eq(visible[15], true, "map follows connector south")
eq(visible[16], true, "map follows connector loop")

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
for i = 0, 4 do stored_files.files[i] = { id = -1, size = 0, value = 0 } end
for i = 0, 11 do stored_files.ratings[i] = 0 end
local objective_run = { state = -1, objective = 4, value = 3,
    fc28 = 1, fc1c = 0 }
local objective_slot = Actions.store(stored_files, objective_run,
    { class = 4, colour = 2 }, { fff0ae = 0xFF },
    { objective = true, id = 3, value = 3, size = 30 },
    function() return 0 end)
eq(objective_slot, 0, "objective file fills the first free deck slot")
eq(stored_files.files[0].id, 3,
    "objective download uses the ROM run value as its datafile ID")
eq(stored_files.files[0].value, -1,
    "objective marker is represented as the signed ROM byte")
eq(objective_run.state, 1, "storing the objective completes its run")
local ordinary_slot = Actions.store(stored_files, objective_run,
    { class = 4, colour = 3 }, {}, { id = 21, size = 20 },
    function(bound) return bound == 7 and 1 or 0 end)
eq(ordinary_slot, 1, "ordinary data file uses the next free slot")
eq(stored_files.files[1].value, 2,
    "ordinary file retains its positive sale multiplier")
D.erase_file(stored_files, 0)
eq(stored_files.files[0].value, 0,
    "erasing a file clears its value byte with the size word")
eq(D.run_info(0, 0, 0), "NONE", "no active run summary")
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
local run = { state = -1, objective = 5, value = 0, size = 30,
    fc28 = 1, fc1c = 0 }
local found = Actions.search(run, { class = 5 }, {}, decker, deck, nil,
    function() return 0 end)
eq(found.file.objective, true, "objective search returns objective file")
eq(found.file.id, 0, "objective preview uses the ROM run value as file ID")
eq(found.file.size, 30, "objective file uses run size")
local targeted_run = { state = -1, objective = 5, value = 0, size = 30,
    fc28 = 1, fc1c = 0, target_system_id = 6 }
local wrong_system_search = Actions.begin_search(
    targeted_run, { class = 5 }, {}, 5)
eq(wrong_system_search, nil,
    "matching node classes do not expose a run objective in another system")
eq(targeted_run.state, -1,
    "searching a same-class node outside the target leaves the run active")
local wrong_system_erase = Actions.erase(
    targeted_run, { class = 5 }, 5)
eq(wrong_system_erase.text, Actions.TEXT.ERASED,
    "Erase cannot complete a Matrix objective in another system")
eq(targeted_run.state, -1,
    "wrong-system Erase leaves the objective unfinished")
local target_system_search = Actions.begin_search(
    targeted_run, { class = 5 }, {}, 6)
eq(target_system_search.file.objective, true,
    "the assigned system exposes its matching datafile objective")
local upload_run = { state = -1, objective = 5, value = -1, size = 30,
    fc28 = 1, fc1c = 0 }
local upload = Actions.begin_search(upload_run, { class = 5 }, {})
eq(upload.text, Actions.TEXT.UPLOAD_DONE,
    "negative objective value uploads immediately without a download prompt")
eq(upload_run.state, 1, "immediate objective upload completes the run")
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
for i = 0, 4 do low_storage.files[i] = { id = -1, size = 0, value = 0 } end
for i = 0, 11 do low_storage.ratings[i] = 0 end
local _, low_storage_text = Actions.store(
    low_storage, nil, { class = 9, colour = 1 }, {},
    { id = 45, size = 20 }, function() return 0 end)
eq(low_storage_text, Actions.TEXT.NO_STORAGE,
    "insufficient deck storage uses the ROM two-part message")
local full_file_slots = { storage = 100, files = {}, ratings = {} }
for i = 0, 4 do full_file_slots.files[i] = { id = i, size = 0, value = 0 } end
for i = 0, 11 do full_file_slots.ratings[i] = 0 end
local _, full_file_text = Actions.store(
    full_file_slots, nil, { class = 9, colour = 1 }, {},
    { id = 45, size = 20 }, function() return 0 end)
eq(full_file_text, Actions.TEXT.FIVE_FILES,
    "five occupied file slots use the ROM capacity message")
local result = Actions.erase(run, { class = 5 })
eq(result.text, Actions.TEXT.ERASE_OBJECTIVE, "objective erase text")
eq(run.state, 1, "objective erase completes run")
local ordinary_erase_run = { state = -1, objective = 9, value = 0,
    size = 30, fc28 = 1, fc1c = 0 }
local ordinary_erase = Actions.erase(ordinary_erase_run, { class = 5 })
eq(ordinary_erase.text, Actions.TEXT.ERASED,
    "non-objective Erase uses the ordinary ROM message")
eq(ordinary_erase_run.state, -1,
    "non-objective Erase leaves the active run incomplete")
local erase_nonzero_value_run = { state = -1, objective = 5, value = 1,
    size = 30, fc28 = 1, fc1c = 0 }
local erase_nonzero_value = Actions.erase(erase_nonzero_value_run,
    { class = 5 })
eq(erase_nonzero_value.text, Actions.TEXT.ERASED,
    "objective-class Erase requires a zero objective value")
eq(erase_nonzero_value_run.state, -1,
    "nonzero objective value does not complete the run")
local no_run_erase = Actions.erase(nil, { class = 5 })
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
local known_nodes = { [14] = 0 }
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
    end, known_nodes, 14)
eq(analyzed.ok, true, "Analyze returns its successful outcome")
eq(analyzed.net, 2, "Analyze floors and reports net successes")
eq(analyzed.level, 2, "Analyze returns the FFFB30 knowledge rank")
eq(analyzed.frames, 0x3C, "Analyze starts the ROM profile duration")
eq(known_nodes[14], 0x20, "Analyze stores its knowledge nibble")
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
    context = E.RunOfferContext.BIG_RHINO,
    source_offer_context = E.RunOfferContext.BIG_RHINO,
    source_initializer = E.RunOfferInitializer.MATRIX_SIMPLE,
    payment = {
        base_amount = 425, negotiated_amount = 425, negotiation_skill = 0,
        negotiation_bound = 1, negotiation_roll = 0,
    },
    source_run_type = E.RunType.MATRIX_OBJECTIVE,
    matrix_run_state = {
        state = -1, objective = 5, value = 0, size = 30,
        fc28 = 1, fc1c = 0, target_system_id = 6,
    },
}
local world = Session.world_state(game)
eq(world.run.state, -1,
    "Matrix world projection uses the accepted Lua objective")
world.run.state = 1
eq(Registry.active_matrix_run_state().state, -1,
    "Matrix projection is a copy until committed")
Session.commit_world_state(game, world)
eq(Registry.active_matrix_run_state().state, 1,
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

-- All five ROM offer contexts contain eight records at F3EBC. The typed Lua
-- table retains initializer, progress, payment, and negotiation fields.
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
    [0] = {
        {0,1,10,3}, {1,1,45,1}, {2,1,35,1}, {1,1,50,1},
        {2,1,40,1}, {1,1,55,1}, {2,1,45,1}, {0,1,20,2},
    },
    [1] = {
        {7,3,2600,4}, {1,1,200,2}, {5,2,500,5}, {4,2,600,5},
        {11,4,900,5}, {9,4,1000,5}, {2,1,180,2}, {3,2,160,3},
    },
    [2] = {
        {6,2,425,1}, {3,2,190,2}, {2,1,180,1}, {0,1,40,1},
        {4,2,550,4}, {5,2,450,4}, {1,1,200,1}, {2,1,190,1},
    },
    [3] = {
        {3,1,230,1}, {7,2,3000,2}, {8,4,6350,5}, {2,0,250,1},
        {9,3,1700,4}, {10,5,4500,10}, {12,5,4000,10}, {6,1,500,1},
    },
    [4] = {
        {7,3,2550,5}, {8,5,6000,10}, {11,4,1200,5}, {9,4,1400,5},
        {12,6,3500,15}, {10,6,3850,15}, {8,5,6100,10}, {7,4,2750,5},
    },
}
for context = 0, 4 do
    local rows = OfferData.contexts[context]
    eq(#rows, 8, "ROM offer row count for context " .. context)
    for index = 1, 8 do
        local row = rows[index]
        local expected = expected_context_rows[context][index]
        eq(row.source_initializer, expected[1], "source initializer row")
        eq(row.runner_progress_delta, expected[2], "progress row")
        eq(row.base_payment, expected[3], "base payment row")
        eq(row.negotiation_scale, expected[4], "negotiation scale row")
    end
end
eq(Dialogue.johnson_interaction_profiles["wanderer.vigore_jarl"].run_offer_context,
    E.RunOfferContext.WANDERER, "Wanderer routes work to context three")

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
        return { run = { fc28 = 5 } }
    end,
}
local selected_run = Registry.prepare_offer_for_context(E.RunOfferContext.WANDERER)
local matrix_offer = Registry.current_offer
eq(selected_run, "matrix_objective", "context selects Matrix Lua module")
eq(matrix_offer.source_offer_type, E.RunOfferInitializer.MATRIX_SIMPLE,
    "offer retains its source initializer variant")
eq(matrix_offer.source_run_type, E.RunType.MATRIX_OBJECTIVE,
    "offer retains the canonical run type")
eq(matrix_offer.source_offer_context, E.RunOfferContext.WANDERER,
    "offer retains its ROM Johnson context")
eq(matrix_offer.matrix_run_state.state, -1,
    "simple Matrix initializer selects the negative band")
eq(matrix_offer.matrix_run_state.objective, 4,
    "Matrix objective class comes from the source candidate")
eq(matrix_offer.matrix_run_state.value, 20,
    "Matrix objective value comes from the source candidate")
eq(matrix_offer.matrix_run_state.size, 10,
    "Matrix file size follows the source one-to-three roll")
eq(matrix_offer.matrix_run_state.fc28, 5,
    "Matrix initializer preserves the typed completion flags")
eq(matrix_offer.matrix_run_state.fc1c, 0,
    "common source initializer clears the run flags")
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
local function prepare_context_row(context, rolls)
    random_call = 0
    random_rolls = rolls
    return Registry.prepare_offer_for_context(context), Registry.current_offer
end

local gang_id, gang_offer = prepare_context_row(1, { 7, 0, 0, 0 })
eq(gang_id, "gang_contract", "gang source initializer loads its own module")
eq(gang_offer.source_offer_type, E.RunOfferInitializer.GANG_CONTRACT,
    "gang offer retains source initializer")
eq(gang_offer.route.target.area_id, 1,
    "gang target is selected from the ROM three-site list")
eq(gang_offer.route.target.location_id, 9,
    "gang target retains its source event ID")
eq(gang_offer.variant, 0, "gang copy uses the source four-variant roll")
assert(Support.gang_contract_texts[1]:find("||", 1, true) and
       not Support.gang_contract_texts[1]:find("\n", 1, true),
    "gang copy uses the shared dialogue paragraph separators")
checks = checks + 1
Registry.resolve_current_offer("no")

local recovery_id, recovery_offer = prepare_context_row(3, { 4, 0, 0, 0 })
eq(recovery_id, "package_recovery",
    "two-site package recovery loads its own module")
eq(recovery_offer.source_offer_type,
    E.RunOfferInitializer.PACKAGE_RECOVERY_TWO_SITE_POOL,
    "recovery preserves its two-site initializer variant")
eq(recovery_offer.route.target.area_id, 1,
    "recovery target uses the ROM two-site pool")
eq(recovery_offer.route.target.location_id, 2,
    "recovery target retains the selected event ID")
Registry.resolve_current_offer("no")

local employee_id, employee_offer = prepare_context_row(3, { 6, 0, 0 })
eq(employee_id, "employee_recruitment",
    "Renraku employee recovery loads its own module")
eq(employee_offer.source_offer_type,
    E.RunOfferInitializer.EMPLOYEE_RECRUITMENT_RENRAKU,
    "employee offer preserves its fixed-site initializer")
eq(employee_offer.route.target.area_id, 5,
    "employee offer uses the ROM fixed Renraku area")
eq(employee_offer.route.target.location_id, 6,
    "employee offer uses the ROM fixed Renraku event")
eq(employee_offer.employee_name, "Mike Nana",
    "employee name comes from the twelve-name source table")
Registry.resolve_current_offer("no")

local moderate_id, moderate_offer = prepare_context_row(3, { 1, 0, 14, 0, 0 })
eq(moderate_id, "matrix_objective",
    "moderate Matrix initializer reaches the Matrix module")
eq(moderate_offer.matrix_run_state.state, 0,
    "moderate Matrix initializer selects the zero band")
eq(moderate_offer.matrix_run_state.target_system_id, 10,
    "moderate candidate retries until its system is in the source band")
Registry.resolve_current_offer("no")

local expert_id, expert_offer = prepare_context_row(3, { 2, 0, 4, 0, 0 })
eq(expert_id, "matrix_objective",
    "expert Matrix initializer reaches the Matrix module")
eq(expert_offer.matrix_run_state.state, 1,
    "expert Matrix initializer selects the positive band")
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
        accepted_offer_context = Registry.current_offer.source_offer_context
        return "no"
    end,
}
eq(Johnson.interaction(), "leave", "shared Johnson interaction returns to field")
eq(accepted_offer_context, E.RunOfferContext.WANDERER,
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
    source_offer_context = E.RunOfferContext.WANDERER,
}
Registry._pending_johnson_return = {
    run_id = "matrix_objective_probe",
    context = E.RunOfferContext.WANDERER,
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
        context = E.RunOfferContext.WANDERER,
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
local old_present_offer_for_context = Registry.present_offer_for_context
local old_take_pending_johnson_return = Registry.take_pending_johnson_return
local mortimer_offer_context
local mortimer_events = {}
local mortimer_menu_call = 0
Registry.present_offer_for_context = function(context)
    mortimer_offer_context = context
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
eq(mortimer_offer_context, E.RunOfferContext.BIG_RHINO,
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
Registry.present_offer_for_context = old_present_offer_for_context
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
