extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <filesystem>
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
local Session = require("menu.matrix_session")
local Cyberdeck = require("menu.screens.cyberdeck")
local MatrixText = require("menu.rules.matrix_text")

local checks = 0
local function eq(actual, expected, label)
    assert(actual == expected,
        (label .. ": expected %s, got %s"):format(tostring(expected), tostring(actual)))
    checks = checks + 1
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
local started, first_session = Session.begin(0)
eq(started, true, "Matrix session starts from system zero")
eq(first_session.current_node, 0, "Matrix session enters access node")
eq(first_session.mode, E.MatrixModeResult.ATTACK,
    "node action loop resets Matrix mode to attack")
eq(first_session.nodes[0].links[0], 4, "system zero access link")
eq(first_session.map[139] ~= nil, true, "Matrix session copies cell 139")
eq(first_session.map[140], nil, "Matrix session does not create cell 140")
eq(table.concat(Session.map_labels(), ","), "CPU,SPU,IOP,SAN,SM,DS",
    "Matrix map labels match ROM text table")
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
    deck_tick = function() end,
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
-- the menu calls its heartbeat, moves the visible cursor, and submits the
-- selected attack mode before opening the chosen child screen.
local Cyberdeck = require("menu.screens.cyberdeck").Cyberdeck
local deck_screen, grid = Cyberdeck.new(), { ticks = 0, mode = nil, redraw = nil }
local grid_ctx = {
    game = {
        deck_tick = function() grid.ticks = grid.ticks + 1 end,
        set_deck_mode = function(mode) grid.mode = mode end,
    },
    redraw = function(region) grid.redraw = region end,
}
Cyberdeck.input(deck_screen, E.Pad.UP, grid_ctx)
eq(grid.ticks, 1, "deck screen services heartbeat")
eq(deck_screen.col, 0, "deck screen up keeps first column")
eq(deck_screen.row, 3, "deck screen up wraps to bottom row")
eq(grid.redraw, E.MenuRegion.SUBMENU, "deck screen redraws moved grid")
local child = Cyberdeck.input(deck_screen, E.Pad.A, grid_ctx)
eq(grid.mode, E.MatrixDeckMode.ATTACK, "deck screen submits attack mode")
eq(child.transition, E.MenuTransition.PUSH, "deck screen opens child route")
eq(child.screen, E.MenuScreen.DECK_CYBERINFO, "bottom-left cell opens deck info")

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
local stored_files = { storage = 4096, files = {}, ratings = {} }
for i = 0, 4 do stored_files.files[i] = { id = -1, size = 0, value = 0 } end
for i = 0, 11 do stored_files.ratings[i] = 0 end
local objective_run = { state = -1, objective = 4, value = 3,
    fc28 = 1, fc1c = 0 }
local objective_slot = Actions.store(stored_files, objective_run,
    { class = 4, colour = 2 }, { fff0ae = 0xFF },
    { objective = true, size = 30 }, function() return 0 end)
eq(objective_slot, 0, "objective file fills the first free deck slot")
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
local slots = { status = { [0] = 6, [1] = 6, [2] = 6, [3] = 6, [4] = 6 },
    phase = { [0] = 0, [1] = 0, [2] = 0, [3] = 0, [4] = 0 },
    timer = { [0] = 0, [1] = 0, [2] = 0, [3] = 0, [4] = 0 } }
D.load(deck, slots, 1, 2, false)
eq(deck.active[1], 2, "loading publishes selected program")
eq(slots.timer[1], 60, "loading timer is ten frames per step")
eq(slots.status[1], 0, "ready slot enters loading state")

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
local cancelled = Actions.cancel_alert({ alert = 0 })
eq(cancelled.text, Actions.TEXT.ALERT_CANCELLED, "cancel active alert text")
local run = { state = -1, objective = 5, value = 0, size = 30,
    fc28 = 1, fc1c = 0 }
local found = Actions.search(run, { class = 5 }, {}, decker, deck, nil,
    function() return 0 end)
eq(found.file.objective, true, "objective search returns objective file")
eq(found.file.size, 30, "objective file uses run size")
local result = Actions.erase(run, { class = 5 })
eq(result.text, Actions.TEXT.ERASE_OBJECTIVE, "objective erase text")
eq(run.state, 1, "objective erase completes run")
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
    lua_newtable(state);
    lua_setglobal(state, "__matrix_test_modules");
    lua_pushcfunction(state, load_test_module);
    lua_setglobal(state, "require");

    const bool passed = run_rules(state);
    lua_close(state);
    return passed ? 0 : 1;
}
