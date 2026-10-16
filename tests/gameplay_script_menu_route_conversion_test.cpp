#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "shadowrun_ghidra/scripting/cyberdeck_menu_bindings.hpp"
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
#include "matrix_datastore_facts.hpp"
#include "matrix_map_presenter.hpp"

#include <QColor>
#include <QImage>
#include <QPainter>
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using shadowrun::ghidra_conversion::scripting::GameScriptRuntime;
using shadowrun::ghidra_conversion::scripting::ScriptNotebookBoundary;
using shadowrun::ghidra_conversion::scripting::ScriptNotebookFacts;
using shadowrun::ghidra_conversion::scripting::ScriptCyberdeckRecord;
using shadowrun::ghidra_conversion::scripting::ScriptMenuRouteGame;
using shadowrun::ghidra_conversion::scripting::ScriptMenuPartyMember;
using shadowrun::ghidra_conversion::scripting::ScriptMenuRouteBindings;
using shadowrun::ghidra_conversion::scripting::ScriptMatrixRunState;
using shadowrun::ghidra_conversion::scripting::ScriptMatrixDatastoreFacts;
using shadowrun::ghidra_conversion::scripting::ScriptMatrixWorldState;
using shadowrun::ghidra_conversion::scripting::CyberdeckMenuBinding;
using shadowrun::ghidra_conversion::scripting::dispatch_cyberdeck_menu_binding;
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
using shadowrun::ghidra_conversion::qt_runtime::
    apply_matrix_datastore_facts;
using shadowrun::ghidra_conversion::qt_runtime::
    make_matrix_datastore_facts;
using shadowrun::ghidra_conversion::qt_runtime::paint_matrix_system_map;
using shadowrun::ghidra_conversion::qt_runtime::paint_matrix_node_action_panel;
#endif

namespace {

constexpr std::uint16_t kCommand = 1U;
constexpr std::uint16_t kPosture = 2U;
constexpr std::uint16_t kAttributes = 5U;
constexpr std::uint16_t kCyberware = 6U;
constexpr std::uint16_t kPocketSecretary = 9U;
constexpr std::uint16_t kCyberdeck = 18U;
constexpr std::uint16_t kDebugMenu = 27U;
constexpr std::uint16_t kDebugPickSystem = 28U;
constexpr std::uint16_t kDeckMap = 34U;
constexpr std::uint16_t kMatrixNodeAction = 37U;
constexpr std::uint16_t kCommandPagePocketSecretary = 6U;
constexpr std::uint16_t kNotebook = 10U;
constexpr std::uint16_t kNotebookTips = 21U;
constexpr std::uint16_t kMrJohnsons = 20U;
constexpr std::uint16_t kGroupItems = 17U;
constexpr std::uint16_t kDismissRunner = 11U;
constexpr std::uint16_t kClips = 4U;
constexpr std::uint16_t kClipTrade = 22U;
constexpr std::uint16_t kReloadConfirm = 23U;
constexpr std::uint16_t kMessage = 19U;
constexpr int kNoCyberdeck = 17;
constexpr std::uint16_t kItemAction = 3U;
constexpr std::uint16_t kFirstAid = 12U;
constexpr std::uint16_t kInventoryTrade = 14U;
constexpr std::uint16_t kDiscardConfirm = 15U;
constexpr std::uint16_t kMoveSound = 0x25U;
constexpr std::uint16_t kSelectSound = 0x24U;
constexpr std::uint16_t kSidebarWindow = 0x9191U;
constexpr std::uint16_t kHiddenWindow = 0x9100U;

void check(const bool condition, const char* message, int& failures) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void verify_cyberdeck_file_value_bridge(int& failures) {
    using namespace shadowrun::ghidra_conversion::scripting;
    lua_State* const lua = luaL_newstate();
    if (lua == nullptr) {
        check(false, "create Lua state for Cyberdeck file-value bridge", failures);
        return;
    }

    ScriptMenuRouteGame game;
    ScriptCyberdeckRecord source;
    source.files[0] = {4, 20, -1};
    source.files[1] = {9, 30, 12};
    game.deck_record = [&source] { return source; };
    ScriptCyberdeckRecord committed;
    bool did_commit = false;
    game.deck_commit = [&committed, &did_commit](
                           const ScriptCyberdeckRecord& record,
                           const std::array<ScriptCyberdeckProgramSlot, 5>&) {
        committed = record;
        did_commit = true;
    };

    dispatch_cyberdeck_menu_binding(lua, game, CyberdeckMenuBinding::record);
    lua_getfield(lua, -1, "files");
    lua_rawgeti(lua, -1, 0);
    lua_getfield(lua, -1, "value");
    int is_integer = 0;
    const auto objective_value = lua_tointegerx(lua, -1, &is_integer);
    check(is_integer != 0 && objective_value == -1,
          "Lua receives the signed objective marker from the Cyberdeck record",
          failures);
    lua_settop(lua, 0);

    lua_createtable(lua, 0, 1); // record
    lua_createtable(lua, 1, 0); // files
    lua_createtable(lua, 0, 3); // file 0
    lua_pushinteger(lua, 12);
    lua_setfield(lua, -2, "id");
    lua_pushinteger(lua, 40);
    lua_setfield(lua, -2, "size");
    lua_pushinteger(lua, -7);
    lua_setfield(lua, -2, "value");
    lua_rawseti(lua, -2, 0);
    lua_setfield(lua, -2, "files");
    lua_createtable(lua, 5, 0); // program slots
    dispatch_cyberdeck_menu_binding(lua, game, CyberdeckMenuBinding::commit);
    check(did_commit && committed.files[0].id == 12 &&
              committed.files[0].size == 40 &&
              committed.files[0].value == -7,
          "Lua commit preserves the signed Cyberdeck file value", failures);
    lua_close(lua);
}

#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
void verify_matrix_datastore_field_projection(int& failures) {
    const std::array<std::uint32_t, 3> notebook_tips{
        0x01020304U, 0x11223344U, 0x55667788U};
    const auto facts = make_matrix_datastore_facts(
        notebook_tips, true, 0x81234567U,
        std::optional<std::uint32_t>{0x6023U},
        std::optional<std::uint32_t>{0x12345678U},
        std::optional<std::uint8_t>{0x08U});
    check(facts.tips_page2_flags == 0x11223344U &&
              facts.tips_page3_flags == 0x55667788U,
          "Matrix datastore receives FFFC34/FFFC38 from Notebook tips",
          failures);
    check(facts.world_event_flags == 0x81234567U &&
              facts.group_item_flags == 0x6023U &&
              facts.nuyen == 0x12345678U &&
              facts.mr_johnsons_met_flags == 0x08U,
          "Matrix datastore receives the live world, item, wallet, and Johnson facts",
          failures);

    const auto unresolved = make_matrix_datastore_facts(
        notebook_tips, false, 0U, std::nullopt, std::nullopt, std::nullopt);
    check(!unresolved.tips_page2_flags.has_value() &&
              !unresolved.tips_page3_flags.has_value(),
          "unrestored Notebook flags remain unknown to Matrix story dispatch",
          failures);

    auto committed_tips = std::array<std::uint32_t, 3>{9U, 8U, 7U};
    bool tips_resolved = false;
    std::uint32_t committed_world_flags = 0U;
    std::optional<std::uint32_t> committed_group_items;
    std::optional<std::uint32_t> committed_nuyen;
    std::optional<std::uint8_t> committed_johnsons;
    apply_matrix_datastore_facts(
        facts, committed_tips, tips_resolved, committed_world_flags,
        committed_group_items, committed_nuyen, committed_johnsons);
    check(tips_resolved && committed_tips[0] == 9U &&
              committed_tips[1] == 0x11223344U &&
              committed_tips[2] == 0x55667788U,
          "Matrix story updates write back only the two source tip pages",
          failures);
    check(committed_world_flags == 0x81234567U &&
              committed_group_items == 0x6023U &&
              committed_nuyen == 0x12345678U &&
              committed_johnsons == 0x08U,
          "Matrix story updates preserve the typed field facts",
          failures);
}
#endif

} // namespace

int main() {
#ifndef SHADOWRUN_GAMEPLAY_SCRIPT_ROOT
#error SHADOWRUN_GAMEPLAY_SCRIPT_ROOT must point at gameplay/scripts
#endif

    int failures = 0;
    verify_cyberdeck_file_value_bridge(failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    verify_matrix_datastore_field_projection(failures);
#endif
    GameScriptRuntime runtime;
    if (!runtime.load_package(
            std::filesystem::path(SHADOWRUN_GAMEPLAY_SCRIPT_ROOT))) {
        std::cerr << "could not load gameplay scripts: " << runtime.failure()
                  << '\n';
        return 1;
    }

    std::vector<std::uint16_t> sounds;
    std::vector<std::uint16_t> stopped_sounds;
    std::vector<std::uint16_t> windows;
    std::vector<std::uint16_t> legacy_screens;
    std::vector<std::uint16_t> blank_modes;
    int posture = 2;
    int reselect_count = 0;
    int closed_count = 0;
    int debug_unlock_consumptions = 0;
    int deck_enter_count = 0;
    int deck_leave_count = 0;
    int deck_tick_count = 0;
    int deck_mode = 0;
    ScriptMatrixWorldState matrix_world_state;
    matrix_world_state.run = ScriptMatrixRunState{-1, -1, 0, 0, 1, 0};
    matrix_world_state.datastore = ScriptMatrixDatastoreFacts{
        0U, 0xA2345678U, 0xCDEF1234U, 0x81234567U, 0x6023U,
        0x12345678U, 0x08U};
    matrix_world_state.field.alarm = true;
    matrix_world_state.field.security_off = false;
    matrix_world_state.field.maglocks_off = false;
    matrix_world_state.actors = {{"fixture-armed", 0x02U},
                                 {"fixture-other", 0x04U}};
    ScriptCyberdeckRecord matrix_deck;
    matrix_deck.mpcp = 6U;
    matrix_deck.storage = 1000U;
    matrix_deck.memory = 128U;
    matrix_deck.load_io = 2U;
    matrix_deck.response = 2U;
    matrix_deck.persona = {2U, 2U, 2U, 5U};
    int matrix_deck_commits = 0;
    bool pocket_extra_row = false;
    std::vector<int> random_bounds;
    bool random_high_branch = false;
    bool matrix_search_miss = false;
    int dismissed_slot = -1;
    bool legacy_active = false;
    std::vector<ScriptNotebookBoundary> notebook_boundaries;
    ScriptNotebookFacts notebook_facts;
    notebook_facts.current_run_available = false;
    notebook_facts.event_gates = std::array<bool, 3>{true, true, false};
    notebook_facts.tip_flags =
        std::array<std::uint32_t, 3>{0x5U, 0x1U, 0U};
    notebook_facts.referral_flags = 0x15U;
    std::array<ScriptMenuPartyMember, 3> party{{
        {1U, 3U, 20U, 0, 2U},
        {3U, 5U, 0U, 0, 2U},
        {0U, 0U, 0U, -1, 0U},
    }};
    std::vector<std::array<int, 3>> ammo_commits;
    ScriptMenuRouteBindings bindings;
    bindings.host.play_sound = [&sounds](const std::uint16_t sound) {
        sounds.push_back(sound);
    };
    bindings.host.stop_sound = [&stopped_sounds](const std::uint16_t sound) {
        stopped_sounds.push_back(sound);
    };
    bindings.host.redraw = [](std::uint32_t) {};
    bindings.host.blank = [&blank_modes](const std::uint16_t mode) {
        blank_modes.push_back(mode);
    };
    bindings.host.set_window = [&windows](const std::uint16_t window) {
        windows.push_back(window);
    };
    bindings.host.set_screen = [](std::uint16_t) {};
    bindings.host.menu_closed = [&closed_count] { ++closed_count; };
    bindings.host.legacy_open =
        [&legacy_screens, &legacy_active](
            const std::uint16_t screen,
            const shadowrun::ghidra_conversion::scripting::
                ScriptMenuRouteArgs&) {
            legacy_screens.push_back(screen);
            legacy_active = true;
        };
    bindings.host.legacy_advance =
        [&legacy_active](std::uint16_t, std::uint16_t) {
            return std::pair<bool, bool>{legacy_active, false};
        };
    bindings.host.notebook_boundary =
        [&notebook_boundaries](const ScriptNotebookBoundary boundary) {
            notebook_boundaries.push_back(boundary);
        };
    bindings.game.leader_incapacitated = [] { return false; };
    bindings.game.reselect_leader = [&reselect_count] { ++reselect_count; };
    bindings.game.leader_posture = [&posture] { return posture; };
    bindings.game.set_leader_posture = [&posture](const int value) {
        posture = value;
    };
    bindings.game.leader_archetype = [] { return 3; };
    bindings.game.inventory_item = [&party](const int column, const int row) {
        if (column < 0 || column > 1 || row < 0 || row > 3) return false;
        const auto slot = static_cast<std::size_t>(row * 2 + column);
        return party[0].inventory[slot] != 0U;
    };
    bindings.game.party = [&party] { return party; };
    bindings.game.deck_record = [&matrix_deck] { return matrix_deck; };
    bindings.game.deck_slots = [] {
        return std::array<shadowrun::ghidra_conversion::scripting::
                              ScriptCyberdeckProgramSlot,
                          5>{};
    };
    bindings.game.deck_commit = [&matrix_deck, &matrix_deck_commits](
                                    const ScriptCyberdeckRecord& record,
                                    const std::array<shadowrun::ghidra_conversion::
                                                         scripting::
                                                             ScriptCyberdeckProgramSlot,
                                                     5>&) {
        matrix_deck = record;
        ++matrix_deck_commits;
    };
    bindings.game.active_slot = [] { return 0; };
    bindings.game.magazine_capacity = [](int code) {
        return code == 2 ? 30 : (code == 3 ? 12 : 0);
    };
    bindings.game.set_runner_ammo =
        [&party, &ammo_commits](const int slot, const int clips,
                               const int rounds) {
            if (slot < 0 || slot >= static_cast<int>(party.size())) return;
            party[static_cast<std::size_t>(slot)].clips =
                static_cast<std::uint8_t>(clips);
            party[static_cast<std::size_t>(slot)].rounds =
                static_cast<std::uint8_t>(rounds);
            ammo_commits.push_back({slot, clips, rounds});
        };
    bindings.game.slot_active_set = [&party] {
        return std::array<bool, 3>{
            (party[0].status & 1U) != 0U,
            (party[1].status & 1U) != 0U,
            (party[2].status & 1U) != 0U};
    };
    int inventory_commits = 0;
    bindings.game.commit_party = [&party, &inventory_commits](
                                    const std::array<ScriptMenuPartyMember,
                                                     3>& next) {
        party = next;
        ++inventory_commits;
    };
    std::array<int, 2> first_aid_roll{};
    int forced_roll_successes = 2;
    std::vector<std::array<int, 2>> action_rolls;
    bindings.game.roll_successes = [&first_aid_roll, &forced_roll_successes,
                                    &action_rolls](const int count,
                                                   const int threshold) {
        first_aid_roll = {count, threshold};
        action_rolls.push_back({count, threshold});
        return forced_roll_successes;
    };
    bindings.game.pocket_extra_row = [&pocket_extra_row] {
        return pocket_extra_row;
    };
    bindings.game.source_random_bound = [&random_bounds,
                                         &random_high_branch,
                                         &matrix_search_miss](const int bound) {
        random_bounds.push_back(bound);
        if (matrix_search_miss && bound == 10) return bound - 1;
        if (random_high_branch) {
            return bound == 2 ? 1 : (bound == 4 ? 3 : 0);
        }
        return 0;
    };
    bindings.game.dismiss_runner = [&dismissed_slot](const int slot) {
        dismissed_slot = slot;
    };
    bindings.game.group_item_flags = [] {
        return std::optional<std::uint32_t>{(1U << 14U) | (1U << 1U) | 1U};
    };
    bindings.game.notebook_facts = [&notebook_facts] {
        return std::optional<ScriptNotebookFacts>{notebook_facts};
    };
    std::optional<bool> cyberdeck_available = false;
    bindings.game.cyberdeck_available = [&cyberdeck_available] {
        return cyberdeck_available;
    };
    bindings.game.consume_debug_unlock = [&debug_unlock_consumptions] {
        ++debug_unlock_consumptions;
    };
    bindings.game.deck_enter = [&deck_enter_count] {
        ++deck_enter_count;
        return 0;
    };
    bindings.game.deck_leave = [&deck_leave_count](int) {
        ++deck_leave_count;
    };
    bindings.game.deck_tick = [&deck_tick_count] { ++deck_tick_count; };
    bindings.game.set_deck_mode = [&deck_mode](const int value) {
        deck_mode = value;
    };
    bindings.game.matrix_world_state = [&matrix_world_state] {
        return matrix_world_state;
    };
    bindings.game.commit_matrix_world_state = [&matrix_world_state](
        const ScriptMatrixWorldState& update) {
        if (update.run.has_value()) {
            matrix_world_state.run = update.run;
        }
        if (update.field.alarm.has_value()) {
            matrix_world_state.field.alarm = update.field.alarm;
        }
        if (update.field.security_off.has_value()) {
            matrix_world_state.field.security_off = update.field.security_off;
        }
        if (update.field.maglocks_off.has_value()) {
            matrix_world_state.field.maglocks_off = update.field.maglocks_off;
        }
        if (update.datastore.has_value()) {
            matrix_world_state.datastore = update.datastore;
        }
        for (const auto& actor : update.actors) {
            const auto current = std::find_if(
                matrix_world_state.actors.begin(),
                matrix_world_state.actors.end(),
                [&actor](const auto& candidate) {
                    return candidate.actor_id == actor.actor_id;
                });
            if (current != matrix_world_state.actors.end()) {
                current->flags_9e = actor.flags_9e;
            }
        }
    };
    runtime.set_menu_route_bindings(std::move(bindings));

    const auto opened = runtime.open_menu_route(kCommand);
    if (!opened) {
        std::cerr << "menu route error: " << runtime.failure() << '\n';
    }
    check(opened, "open command route", failures);
    auto route = runtime.menu_route_snapshot();
    check(route.open && route.top_screen == kCommand,
          "command is active route root", failures);
    check(route.page == 6 && route.row == 0,
          "command initializes to Pocket Secretary and row zero", failures);
    check(sounds.size() == 1U && sounds.front() == kMoveSound,
          "opening the command screen plays move sound", failures);
    check(blank_modes.size() == 2U && blank_modes.front() == 0x8134U &&
              blank_modes.back() == 0x8174U,
          "full command entry brackets its draw with blank modes", failures);

    runtime.advance_menu_route(0x0100U); // Up: command -> posture.
    route = runtime.menu_route_snapshot();
    check(route.open && route.top_screen == kPosture,
          "Up pushes posture focus", failures);
    runtime.advance_menu_route(0x0800U); // Right: posture 2 -> 3.
    check(posture == 3 && sounds.back() == kMoveSound,
          "posture adjustment updates the live game service", failures);
    runtime.advance_menu_route(0x0800U);
    runtime.advance_menu_route(0x0800U);
    check(posture == 4, "posture clamps at full offense", failures);
    runtime.advance_menu_route(0x0200U | 0x0004U | 0x0400U);
    route = runtime.menu_route_snapshot();
    check(posture == 3 && route.top_screen == kPosture,
          "Down with held Left adjusts posture without closing", failures);
    const auto sounds_before_deselect = sounds.size();
    runtime.advance_menu_route(0x0200U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand,
          "Down returns from posture focus to command", failures);
    check(sounds.size() == sounds_before_deselect,
          "Down returns from posture focus without requesting a sound",
          failures);
    runtime.advance_menu_route(0x0100U | 0x0008U);
    check(runtime.menu_route_snapshot().top_screen == kCommand,
          "held Right blocks posture focus", failures);
    runtime.advance_menu_route(0x0100U);
    const auto sounds_before_start = sounds.size();
    runtime.advance_menu_route(0x8000U);
    route = runtime.menu_route_snapshot();
    check(!route.open && closed_count == 1,
          "Start in posture exits the complete menu route", failures);
    check(sounds.size() == sounds_before_start,
          "Start in posture exits without requesting a sound", failures);

    check(runtime.open_menu_route(kCommand), "reopen command route", failures);
    const auto legacy_count_before_secretary = legacy_screens.size();
    runtime.advance_menu_route(0x4000U); // A: Pocket Secretary child.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.page == 6 &&
              route.top_row == 0,
          "A opens the Lua Pocket Secretary root at row zero", failures);
    check(legacy_screens.size() == legacy_count_before_secretary,
          "Pocket Secretary root no longer opens through the legacy adapter",
          failures);
    const auto sounds_before_secretary = sounds.size();
    runtime.advance_menu_route(0x1000U); // B: no-op at 0xC2A6.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 0 &&
              sounds.size() == sounds_before_secretary,
          "Pocket Secretary ignores B without moving or playing a sound",
          failures);
    runtime.advance_menu_route(0x0200U); // Down: row zero -> Group Items.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 1 &&
              sounds.size() == sounds_before_secretary &&
              random_bounds == std::vector<int>{2, 8, 3, 2, 8, 3} &&
              route.top_grid_cells_present && route.top_grid_cells[0] == 0 &&
              route.top_grid_effect_frames == 2,
          "Pocket Secretary Lua owns silent row movement and its three-draw shared-frame effect",
          failures);
    runtime.advance_menu_route(0x4000U); // A dispatches Group Items.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kGroupItems &&
              route.top_visible_item_ids == std::vector<int>{14, 1, 0} &&
              legacy_screens.size() == legacy_count_before_secretary,
          "Lua Group Items derives the populated descending-bit rows without the legacy adapter",
          failures);
    runtime.advance_menu_route(0x1000U); // Any fresh button dismisses F312.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 1 &&
              route.top_grid_effect_frames == 4 &&
              random_bounds.size() == 12U,
          "Group Items returns to the Lua-selected row after its shared grid frame advances",
          failures);
    runtime.advance_menu_route(0x0200U); // Down to Cyberdeck.
    runtime.advance_menu_route(0x4000U); // A: no-deck branch is a Lua message.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage &&
              route.top_message == kNoCyberdeck &&
              route.top_grid_cells_present &&
              legacy_screens.size() == legacy_count_before_secretary,
          "Pocket Secretary routes the source no-cyberdeck response through Lua MESSAGE",
          failures);
    const auto no_deck_message_frames = route.top_grid_effect_frames;
    const auto no_deck_random_draws = random_bounds.size();
    runtime.advance_menu_route(0U); // The 0xC2A6 wait still advances E660.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage &&
              route.top_grid_effect_frames ==
                  no_deck_message_frames.value_or(0) + 1 &&
              random_bounds.size() == no_deck_random_draws + 3U,
          "no-cyberdeck wait advances the shared bezel animation each frame",
          failures);
    runtime.advance_menu_route(0x1000U); // B: dismiss the response.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 2,
          "no-cyberdeck message returns to the selected Pocket Secretary row",
          failures);
    cyberdeck_available.reset();
    runtime.advance_menu_route(0x4000U); // Unknown restored flag uses adapter.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCyberdeck &&
              legacy_screens.size() == legacy_count_before_secretary + 1U &&
              legacy_screens.back() == kCyberdeck,
          "unresolved saved cyberdeck state remains an explicit adapter boundary",
          failures);
    legacy_active = false;
    runtime.advance_menu_route(0U); // Host reports the unresolved child closed.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 2,
          "legacy cyberdeck boundary returns to its selected parent row",
          failures);
    const auto legacy_count_after_cyberdeck_boundary = legacy_screens.size();
    runtime.advance_menu_route(0x0100U); // Up to Group Items.
    runtime.advance_menu_route(0x0100U); // Up to Notebook.
    runtime.advance_menu_route(0x4000U); // Open Notebook through Lua.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kNotebook && route.top_row == 0 &&
              route.top_visible_option_ids == std::vector<int>{0, 1, 2, 3} &&
              legacy_screens.size() ==
                  legacy_count_after_cyberdeck_boundary,
          "Lua Notebook builds the ROM-order rows from typed story gates",
          failures);
    const auto notebook_draw_count = random_bounds.size();
    runtime.advance_menu_route(0U); // F236 services E660 once per frame.
    check(random_bounds.size() == notebook_draw_count + 3U,
          "Lua Notebook advances the shared bezel effect once per VBlank",
          failures);
    runtime.advance_menu_route(0x4000U); // Current Run, no run is available.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 16 &&
              route.top_grid_cells_present,
          "Notebook Current Run shows its source no-run response and keeps E660 active",
          failures);
    runtime.advance_menu_route(0x1000U); // B dismisses the no-run response.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kNotebook && route.top_row == 0,
          "Notebook no-run response returns to the selected row", failures);
    runtime.advance_menu_route(0x0200U); // Tips & Clues.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kNotebookTips && route.top_page == 0 &&
              route.top_visible_source_ids == std::vector<int>{0, 2, 32},
          "Tips & Clues filters the three source bitsets in LSB-first order",
          failures);
    runtime.advance_menu_route(0x0400U); // Left clamps at zero.
    runtime.advance_menu_route(0x0800U); // Right to second unlocked tip.
    runtime.advance_menu_route(0x0800U); // Right to third unlocked tip.
    runtime.advance_menu_route(0x0800U); // Right clamps at the last tip.
    check(runtime.menu_route_snapshot().top_page == 2,
          "Tips & Clues page selection clamps to its unlocked records",
          failures);
    runtime.advance_menu_route(0x4000U); // A returns to Notebook.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kNotebook && route.top_row == 1,
          "Tips & Clues returns to its selected Notebook row", failures);
    runtime.advance_menu_route(0x0200U); // Contacts.
    runtime.advance_menu_route(0x4000U); // Explicit unresolved page boundary.
    check(runtime.menu_route_snapshot().top_screen == kNotebook &&
              notebook_boundaries.back() ==
                  ScriptNotebookBoundary::contacts_flags_unresolved,
          "Contacts remains behind its unresolved source flag boundary",
          failures);
    runtime.advance_menu_route(0x0200U); // Mr. Johnsons.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMrJohnsons && route.top_row == 0 &&
              route.top_visible_source_ids == std::vector<int>{0, 2, 4},
          "Mr. Johnsons exposes only the five ROM-defined referral bits",
          failures);
    runtime.advance_menu_route(0x0100U); // Up wraps to the final visible row.
    runtime.advance_menu_route(0x1000U); // B opens the selected detail.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 19 &&
              route.top_detail_index == 4 && route.top_detail_row == 2 &&
              route.top_visible_source_ids == std::vector<int>{0, 2, 4},
          "Mr. Johnson detail carries its source record and selected row to the presenter",
          failures);
    runtime.advance_menu_route(0x1000U); // Dismiss detail, preserving selection.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMrJohnsons && route.top_row == 2,
          "Mr. Johnson detail wait returns to the same selected list row",
          failures);
    const auto notebook_reselects_before_johnsons = reselect_count;
    runtime.advance_menu_route(0x2000U); // C: mode action stays on the list.
    check(runtime.menu_route_snapshot().top_screen == kMrJohnsons &&
              reselect_count == notebook_reselects_before_johnsons + 1,
          "Mr. Johnsons C action reselects without leaving the page", failures);
    runtime.advance_menu_route(0x8000U); // Start returns to Notebook.
    check(runtime.menu_route_snapshot().top_screen == kNotebook &&
              runtime.menu_route_snapshot().top_row == 3,
          "Start returns from Mr. Johnsons to its Notebook selection", failures);
    runtime.advance_menu_route(0x8000U); // Start returns to Pocket Secretary.
    check(runtime.menu_route_snapshot().top_screen == kPocketSecretary &&
              runtime.menu_route_snapshot().top_row == 0,
          "Start returns from Notebook to the Pocket Secretary", failures);
    runtime.advance_menu_route(0x8000U); // Start returns to Command.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 6,
          "Start returns from the Lua Pocket Secretary to command page six",
          failures);

    runtime.advance_menu_route(0x4000U); // Open Pocket Secretary again.
    runtime.advance_menu_route(0x0200U); // Group Items.
    runtime.advance_menu_route(0x0200U); // Cyberdeck.
    runtime.advance_menu_route(0x0200U); // Dismiss Runner.
    runtime.advance_menu_route(0x4000U); // Open Lua Dismiss Runner.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDismissRunner && route.top_row == 0 &&
              route.top_grid_effect_frames == 4 &&
              route.top_grid_cells_present && route.top_grid_cells[0] == 0 &&
              legacy_screens.size() ==
                  legacy_count_after_cyberdeck_boundary,
          "Dismiss Runner keeps the parent's lower-bezel state on entry",
          failures);
    random_high_branch = true;
    runtime.advance_menu_route(0x0100U); // Select slot 1 below Abort.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDismissRunner && route.top_row == 1 &&
              sounds.back() == kMoveSound && route.top_grid_cells_present &&
              route.top_grid_cells[0] == 0 && route.top_grid_cells[11] == 6,
          "Lua Dismiss Runner selects a candidate and applies the alternate ROM grid branch",
          failures);
    runtime.advance_menu_route(0x4000U); // Dismiss slot 1.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.top_row == 3 &&
              route.top_grid_effect_frames == 6 &&
              route.top_grid_cells_present && route.top_grid_cells[0] == 0 &&
              route.top_grid_cells[11] == 6 && dismissed_slot == 1,
          "Dismiss Runner returns without clearing lower-bezel E660 writes",
          failures);
    runtime.close_menu_route();
    runtime.open_menu_route(kCommand);

    runtime.advance_menu_route(0x0400U); // page 6 -> 5
    runtime.advance_menu_route(0x0400U); // page 5 -> 4
    runtime.advance_menu_route(0x0400U); // page 4 -> 3
    check(runtime.menu_route_snapshot().page == 3,
          "command page navigation reaches Attributes/Skills", failures);
    const auto legacy_count_before_attributes = legacy_screens.size();
    runtime.advance_menu_route(0x1000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kAttributes && !windows.empty() &&
              windows.back() == kHiddenWindow &&
              route.top_column == 0 && route.top_row == 0,
          "Lua Attributes/Skills opens with its first cell focused and "
          "hides the sidebar window",
          failures);
    runtime.advance_menu_route(0x0200U); // Down moves its row.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kAttributes && route.top_column == 0 &&
              route.top_row == 1,
          "Lua Attributes/Skills owns its visible row selection", failures);
    runtime.advance_menu_route(0x0800U); // Right moves its column.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kAttributes && route.top_column == 1 &&
              route.top_row == 1,
          "Lua Attributes/Skills owns its visible column selection", failures);
    runtime.advance_menu_route(0x1000U); // View-only A leaves the page.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 6 &&
              windows.back() == kSidebarWindow && sounds.back() == kMoveSound &&
              legacy_screens.size() == legacy_count_before_attributes,
          "view-only Attributes return fully reinitializes page six without "
          "entering the C++ legacy adapter",
          failures);

    const auto command_reselect_count = reselect_count;
    runtime.advance_menu_route(0x2000U); // C: reselect and refresh.
    route = runtime.menu_route_snapshot();
    check(route.page == 6 && reselect_count == command_reselect_count + 1,
          "C reselects the leader and returns to page six", failures);
    check(std::find(sounds.begin(), sounds.end(), kSelectSound) !=
              sounds.end(),
          "command action uses the select sound", failures);

    runtime.advance_menu_route(0x0400U); // page 6 -> 5.
    runtime.advance_menu_route(0x0400U); // page 5 -> 4 (Cyberware).
    check(runtime.menu_route_snapshot().page == 4,
          "command navigation reaches Cyberware", failures);
    const auto legacy_count_before_cyberware = legacy_screens.size();
    runtime.advance_menu_route(0x1000U); // A opens the wait-only screen.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCyberware && route.page == 4 &&
              legacy_screens.size() == legacy_count_before_cyberware,
          "Cyberware is a Lua wait screen, not a legacy adapter", failures);
    runtime.advance_menu_route(0x0002U); // held input is not a new edge.
    check(runtime.menu_route_snapshot().top_screen == kCyberware,
          "Cyberware waits through held-only input", failures);
    runtime.advance_menu_route(0x0100U); // a fresh Up edge returns.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 4,
          "any fresh controller edge returns to the selected command page",
          failures);

    // Clips behavior is now routed through the live Lua screen. The host
    // exposes only typed party/ammo facts and persistence; selection, prompt,
    // trade, distribution, and acknowledgement state stay in Lua.
    runtime.advance_menu_route(0x0400U); // Cyberware -> Attributes.
    runtime.advance_menu_route(0x0400U); // Attributes -> Clips.
    route = runtime.menu_route_snapshot();
    check(route.page == 2,
          "command route reaches the Clips page", failures);
    runtime.advance_menu_route(0x4000U); // open Clips.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kClips && route.page == 2 &&
              route.top_selection == 2,
          "Clips selector opens with Reload selected", failures);
    runtime.advance_menu_route(0x4000U); // enter reload prompt.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kReloadConfirm && route.top_yes == false,
          "reload prompt defaults to No", failures);
    runtime.advance_menu_route(0x1000U); // B is ignored by prompt.
    check(runtime.menu_route_snapshot().top_screen == kReloadConfirm,
          "reload prompt ignores B", failures);
    runtime.advance_menu_route(0x4000U); // No exits silently.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 2 &&
              party[0].clips == 3U && party[0].rounds == 20U &&
              ammo_commits.empty(),
          "No returns to Command without changing live ammo", failures);

    runtime.advance_menu_route(0x4000U); // reopen Clips.
    party[0].rounds = 0U;
    runtime.advance_menu_route(0x4000U); // direct reload, then message.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 4 &&
              party[0].clips == 2U && party[0].rounds == 30U &&
              !ammo_commits.empty() && ammo_commits.back() ==
                  std::array<int, 3>{0, 2, 30},
          "direct reload commits one clip and clamps rounds to item capacity",
          failures);
    runtime.advance_menu_route(0x4000U); // acknowledge result.
    check(runtime.menu_route_snapshot().top_screen == kCommand &&
              runtime.menu_route_snapshot().page == 2,
          "reload result returns to the selected Command page", failures);

    // The ROM tests +0x4F as a signed byte. Values 0x80..0xFF take the same
    // empty-reserve path, clear the byte, and then show Function Aborted.
    party[0].clips = 0xFFU;
    const auto commits_before_signed_reserve = ammo_commits.size();
    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x4000U); // common signed-byte ammo gate.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 21 &&
              party[0].clips == 0U &&
              ammo_commits.size() == commits_before_signed_reserve + 1U &&
              ammo_commits.back() == std::array<int, 3>{0, 0, 30},
          "signed-negative reserve clears before Function Aborted", failures);
    runtime.advance_menu_route(0x4000U);

    // Distribute tests the high-byte +0x10 status bits, including the
    // remainder recipient, then verifies every modified record is committed.
    party[0].clips = 2U;
    party[1].clips = 5U;
    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x0800U); // Trade.
    runtime.advance_menu_route(0x0800U); // Distribute.
    check(runtime.menu_route_snapshot().top_selection == 4,
          "Clips selector reaches Distribute", failures);
    const auto commits_before_distribution = ammo_commits.size();
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 5 &&
              party[0].clips == 3U && party[1].clips == 4U &&
              party[2].clips == 3U &&
              ammo_commits.size() == commits_before_distribution + 3U,
          "Distribute writes the quotient to all records and adds remainder",
          failures);
    runtime.advance_menu_route(0x4000U);
    party[1].status = 1U; // clear remainder marker for Trade eligibility.
    party[2].status = 1U; // second eligible recipient for cursor cycling.
    party[2].clips = 3U;

    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x0800U); // Trade.
    runtime.advance_menu_route(0x4000U); // open filtered Clip Trade.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kClipTrade && route.top_command == 2 &&
              route.top_quantity == 3 && route.top_recipient == 1,
          "Clip Trade carries Lua-selected recipient and quantity to presenter",
          failures);
    runtime.advance_menu_route(0x0800U); // Decrease Clips.
    runtime.advance_menu_route(0x0800U); // Trade Clips.
    runtime.advance_menu_route(0x0800U); // Change Recipient.
    runtime.advance_menu_route(0x4000U); // Ricky -> third source record.
    check(runtime.menu_route_snapshot().top_recipient == 2,
          "Change Recipient advances through the filtered active roster",
          failures);
    runtime.advance_menu_route(0x4000U); // third record -> Ricky.
    check(runtime.menu_route_snapshot().top_recipient == 1,
          "Change Recipient wraps to the first eligible runner", failures);
    runtime.advance_menu_route(0x0400U); // Trade Clips.
    runtime.advance_menu_route(0x0400U); // Decrease Clips.
    runtime.advance_menu_route(0x4000U); // quantity 3 -> 2.
    runtime.advance_menu_route(0x0800U); // Trade Clips.
    runtime.advance_menu_route(0x4000U); // transfer two.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 5 &&
              party[0].clips == 1U && party[1].clips == 6U &&
              party[2].clips == 3U,
          "Clip Trade commits source and destination ammunition", failures);
    runtime.advance_menu_route(0x4000U);
    party[2].status = 1U;

    // The ROM accumulates the active reserve with byte addition. If the byte
    // wraps to zero, the division/write loop is skipped and the records stay
    // unchanged, even though the completion message is still displayed.
    party[0].clips = 100U;
    party[1].clips = 100U;
    party[2].clips = 56U;
    party[0].status = 1U;
    party[1].status = 1U;
    const auto commits_before_wrapped_distribution = ammo_commits.size();
    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x0800U); // Trade.
    runtime.advance_menu_route(0x0800U); // Distribute.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 5 &&
              party[0].clips == 100U && party[1].clips == 100U &&
              party[2].clips == 56U &&
              ammo_commits.size() == commits_before_wrapped_distribution,
          "Distribute preserves records when the source byte total wraps to zero",
          failures);
    runtime.advance_menu_route(0x4000U);

    // A lone active runner has no Trade recipient; zero reserve takes the
    // ROM common-gate abort route before the selected command is dispatched.
    party[2].status = 0U;
    party[1].status = 0U;
    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x0800U); // Trade.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 3,
          "Trade with no eligible teammate displays the source no-party result",
          failures);
    runtime.advance_menu_route(0x4000U);
    party[0].clips = 0U;
    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x4000U); // common ammo gate.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 21,
          "zero reserve follows the source Function Aborted exit", failures);
    runtime.advance_menu_route(0x4000U);
    party[0].clips = 3U;
    party[1].status = 1U;

    runtime.advance_menu_route(0x4000U); // open Clips.
    runtime.advance_menu_route(0x0400U); // wrap from Reload to Cancel.
    check(runtime.menu_route_snapshot().top_selection == 5,
          "Left wraps the Clips selector from Reload to Cancel", failures);
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 21,
          "Cancel displays the ROM Function Aborted message", failures);
    runtime.advance_menu_route(0x4000U);
    check(runtime.menu_route_snapshot().top_screen == kCommand &&
              runtime.menu_route_snapshot().page == 2,
          "message acknowledgement restores the Clips command page", failures);

    // Inventory actions now run through the Grok-authored Lua route. The
    // host exposes typed records and commits, while the script owns selector,
    // discard, trade, and First Aid input decisions.
    party[0].inventory[1] = 3U;
    party[0].phase[1] = 0U;
    party[0].weapon_slot = 0;
    party[0].weapon_code = 2U;
    party[0].rounds = 30U;
    runtime.advance_menu_route(0x0400U); // Clips -> inventory right.
    runtime.advance_menu_route(0x4000U); // Open selected item.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kItemAction && route.top_selection == 2 &&
              route.top_item_slot == 1 && route.top_item_code == 3,
          "Lua item-action route opens with the selected template item",
          failures);
    runtime.advance_menu_route(0x0C00U); // Simultaneous Left+Right: ROM checks Left first.
    check(runtime.menu_route_snapshot().top_selection == 5,
          "simultaneous item-action directions follow ROM Left priority",
          failures);
    runtime.advance_menu_route(0x0800U); // Right wraps Cancel to Equip/Use.
    runtime.advance_menu_route(0x4000U); // Equip the L36.
    route = runtime.menu_route_snapshot();
    if (!(route.top_screen == kCommand && route.page == 1 &&
          party[0].weapon_slot == 1 && party[0].weapon_code == 3U &&
          party[0].rounds == 12U && inventory_commits == 1)) {
        std::cerr << "inventory equipment debug: screen=" << route.top_screen
                  << " page=" << route.page.value_or(-1)
                  << " slot=" << party[0].weapon_slot
                  << " code=" << static_cast<int>(party[0].weapon_code)
                  << " rounds=" << static_cast<int>(party[0].rounds)
                  << " commits=" << inventory_commits << '\n';
    }
    check(route.top_screen == kCommand && route.page == 1 &&
              party[0].weapon_slot == 1 && party[0].weapon_code == 3U &&
              party[0].rounds == 12U && inventory_commits == 1,
          "Lua equipment action writes the selected weapon and its capacity",
          failures);

    runtime.advance_menu_route(0x4000U); // Open the L36 action row.
    runtime.advance_menu_route(0x0800U); // Discard.
    runtime.advance_menu_route(0x0800U); // Trade.
    runtime.advance_menu_route(0x4000U); // Open recipient prompt.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kInventoryTrade && route.top_recipient == 1 &&
              route.top_destination_slot == 0 &&
              route.top_recipient_full == false && route.top_yes == false,
          "Lua Trade starts with the first active recipient and No selected",
          failures);
    runtime.advance_menu_route(0x0800U); // Yes.
    runtime.advance_menu_route(0x4000U); // Transfer.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 1 &&
              party[0].inventory[1] == 0U && party[0].weapon_slot == -1 &&
              party[0].rounds == 12U && party[1].inventory[0] == 3U &&
              inventory_commits == 2,
          "Lua Trade clears source equipment but preserves loaded rounds",
          failures);

    party[0].inventory[2] = 2U;
    runtime.advance_menu_route(0x0400U); // Inventory left.
    runtime.advance_menu_route(0x0200U); // Row one.
    runtime.advance_menu_route(0x4000U); // Open item actions.
    runtime.advance_menu_route(0x0800U); // Discard.
    runtime.advance_menu_route(0x4000U); // Confirmation starts at No.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDiscardConfirm && route.top_yes == false &&
              route.top_item_slot == 2,
          "Lua discard confirmation starts at No for the selected cell",
          failures);
    runtime.advance_menu_route(0x0800U); // Yes.
    runtime.advance_menu_route(0x2000U); // C confirms discard (ROM 0x12750).
    check(runtime.menu_route_snapshot().top_screen == kCommand &&
              party[0].inventory[2] == 0U && inventory_commits == 3,
          "Lua discard commits the cell removal and returns to inventory",
          failures);

    party[0].inventory[4] = 0x10U;
    party[0].phase[4] = 1U;
    party[0].skills[8] = 4U;
    party[0].attributes[0] = 9U;
    runtime.advance_menu_route(0x0200U); // Row two.
    runtime.advance_menu_route(0x4000U); // Open actions.
    runtime.advance_menu_route(0x4000U); // First Aid.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kFirstAid && route.top_row == 0 &&
              route.top_targets == std::vector<int>{0, 1},
          "Lua First Aid builds the ROM-order active-party target list",
          failures);
    runtime.advance_menu_route(0x1000U); // B confirms; ROM shares A/B/C target path.
    route = runtime.menu_route_snapshot();
    if (!(route.top_screen == kMessage && route.top_message == 9 &&
          route.top_target == 0 && party[0].inventory[4] == 0U &&
          party[0].physical == 2U && party[0].stun == 2U &&
          party[0].state_function == 0xB45CU && inventory_commits == 4 &&
          first_aid_roll == std::array<int, 2>{4, 5})) {
        std::cerr << "inventory first aid debug: screen=" << route.top_screen
                  << " message=" << route.top_message.value_or(-1)
                  << " target=" << route.top_target.value_or(-1)
                  << " item=" << static_cast<int>(party[0].inventory[4])
                  << " physical=" << static_cast<int>(party[0].physical)
                  << " stun=" << static_cast<int>(party[0].stun)
                  << " continuation=" << party[0].state_function
                  << " commits=" << inventory_commits
                  << " roll=" << first_aid_roll[0] << ','
                  << first_aid_roll[1] << '\n';
    }
    check(route.top_screen == kMessage && route.top_message == 9 &&
              route.top_target == 0 && party[0].inventory[4] == 0U &&
              party[0].physical == 2U && party[0].stun == 2U &&
              party[0].state_function == 0xB45CU && inventory_commits == 4 &&
              first_aid_roll == std::array<int, 2>{4, 5},
          "B-confirmed Lua medkit commits meter, item, continuation, and ROM roll inputs",
          failures);
    runtime.advance_menu_route(0x4000U);
    check(runtime.menu_route_snapshot().top_screen == kCommand &&
              runtime.menu_route_snapshot().page == 0,
          "First Aid result returns to the inventory grid", failures);

    party[0].inventory[4] = 0x11U;
    party[0].phase[4] = 1U;
    runtime.advance_menu_route(0x4000U); // Reopen the selected cell.
    runtime.advance_menu_route(0x4000U); // Equip/Use opens First Aid.
    check(runtime.menu_route_snapshot().top_screen == kFirstAid,
          "slap patch reopens the Lua First Aid target screen", failures);
    runtime.advance_menu_route(0x2000U); // C confirms through ROM 0x1251C.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMessage && route.top_message == 10 &&
              route.top_target == 0 && party[0].inventory[4] == 0U &&
              party[0].stun == 4U && inventory_commits == 5,
          "C-confirmed slap patch applies the ROM target effect and consumes one item",
          failures);

    runtime.close_menu_route();
    check(!runtime.menu_route_snapshot().open && closed_count == 3,
          "host scene transition closes the complete route", failures);

    // Exercise the debug-only Matrix entry as a live route, not as an
    // isolated Lua function. The picker clamps 0..24, waits 20 frames, then
    // enters the Lua node-action loop. Start opens the deck loop and returns
    // here; the 3D Matrix actor/service loop remains a separate integration hill.
    party[0].attributes[1] = 12U;
    party[0].attributes[4] = 12U;
    party[0].skills[7] = 12U;
    action_rolls.clear();
    forced_roll_successes = 2;
    pocket_extra_row = true;
    check(runtime.open_menu_route(kCommand),
          "open command for Matrix debug route", failures);
    runtime.advance_menu_route(0x4000U); // Pocket Secretary.
    for (int row = 0; row < 5; ++row) {
        runtime.advance_menu_route(0x0200U);
    }
    runtime.advance_menu_route(0x4000U); // Debug menu.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDebugMenu,
          "development debug menu remains reachable", failures);
    for (int row = 0; row < 3; ++row) {
        runtime.advance_menu_route(0x0200U);
    }
    runtime.advance_menu_route(0x4000U); // Pick System.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDebugPickSystem && route.top_text == "00",
          "Pick System opens at ROM system zero", failures);
    for (int system = 0; system < 25; ++system) {
        runtime.advance_menu_route(0x0200U);
    }
    route = runtime.menu_route_snapshot();
    check(route.top_text == "24",
          "Pick System clamps at the 25th ROM system", failures);
    runtime.advance_menu_route(0x0200U);
    check(runtime.menu_route_snapshot().top_text == "24",
          "Pick System does not step past the last ROM system", failures);
    for (int system = 0; system < 24; ++system) {
        runtime.advance_menu_route(0x0100U);
    }
    check(runtime.menu_route_snapshot().top_text == "00",
          "Pick System returns to system zero", failures);
    runtime.advance_menu_route(0x4000U); // Jack in.
    check(runtime.menu_route_snapshot().top_screen == kDebugPickSystem &&
              sounds.back() == 0x1AU && debug_unlock_consumptions == 1,
          "Matrix entry plays the ROM jack-in effect and consumes the title unlock",
          failures);
    for (int frame = 0; frame < 19; ++frame) {
        runtime.advance_menu_route(0U);
    }
    check(runtime.menu_route_snapshot().top_screen == kDebugPickSystem,
          "Matrix entry keeps the source 20-frame transition", failures);
    runtime.advance_menu_route(0U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction &&
              route.top_option_labels == std::vector<std::string>{"Enter System"} &&
              route.top_matrix_state_present &&
              route.top_matrix_system_id == 0 && route.top_matrix_node == 0 &&
              deck_enter_count == 0,
          "jack-in opens the source node-action loop with the selected system",
          failures);
    runtime.advance_menu_route(0x4000U); // Select Enter System.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction && route.top_title == "Enter System",
          "Enter System selects links without moving immediately", failures);
    runtime.advance_menu_route(0x4000U); // Confirm first ROM link.
    route = runtime.menu_route_snapshot();
    check(route.top_matrix_node == 4 && route.top_title == "CPU" &&
              route.top_option_labels == std::vector<std::string>{
                  "Go To Node", "Cancel Alert", "Crash System"},
          "link confirmation enters the source CPU node and its action list",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    QImage node_actual(320, 224, QImage::Format_ARGB32);
    node_actual.fill(QColor(255, 0, 255));
    QPainter node_painter(&node_actual);
    const std::filesystem::path node_assets(SHADOWRUN_MATRIX_MAP_ASSET_ROOT);
    const QImage node_font = QImage(QString::fromStdString(
        (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
    const bool node_painted = paint_matrix_node_action_panel(
        node_painter, node_font, route);
    node_painter.end();
    check(node_painted,
          "live Lua node-action state reaches the Qt panel presenter", failures);
    bool node_stale_pixel = false;
    for (int y = 0; y < node_actual.height(); ++y) {
        for (int x = 0; x < node_actual.width(); ++x) {
            node_stale_pixel = node_stale_pixel ||
                node_actual.pixelColor(x, y) == QColor(255, 0, 255);
        }
    }
    check(!node_stale_pixel,
          "node-action redraw clears all graphics from the previous screen",
          failures);
    bool node_labels_have_separate_rows = true;
    constexpr std::array<int, 3> kNodeActionRows{85, 107, 129};
    for (const auto y0 : kNodeActionRows) {
        bool row_has_text = false;
        for (int y = y0; y < y0 + 10; ++y) {
            for (int x = 28; x < 180; ++x) {
                row_has_text = row_has_text ||
                    node_actual.pixelColor(x, y) == QColor(36, 219, 36);
            }
        }
        node_labels_have_separate_rows = node_labels_have_separate_rows &&
                                         row_has_text;
    }
    check(node_labels_have_separate_rows,
          "Matrix node actions render on three distinct rows", failures);
    const auto node_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) / "actual.bmp";
    std::filesystem::create_directories(node_artifact.parent_path());
    check(node_actual.save(QString::fromStdString(node_artifact.string()), "BMP"),
          "write live Matrix node-action actual.bmp artifact", failures);
#endif
    forced_roll_successes = 0;
    runtime.advance_menu_route(0x0200U); // Cancel Alert.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction &&
              route.top_title == "Matrix Message" &&
              route.top_text.find("You fail to") != std::string::npos &&
              route.top_matrix_alert == -1,
          "failed Matrix action shows ROM text without changing alert state",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    QImage action_failed_actual(320, 224, QImage::Format_ARGB32);
    action_failed_actual.fill(QColor(255, 0, 255));
    QPainter action_failed_painter(&action_failed_actual);
    const QImage action_failed_font = QImage(QString::fromStdString(
        (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
    const bool action_failed_painted = paint_matrix_node_action_panel(
        action_failed_painter, action_failed_font, route);
    action_failed_painter.end();
    check(action_failed_painted,
          "failed Matrix action message reaches the Qt panel presenter",
          failures);
    bool action_failed_rows_spaced = true;
    constexpr std::array<int, 3> kActionFailedRows{50, 62, 74};
    for (const auto y0 : kActionFailedRows) {
        bool row_has_text = false;
        for (int y = y0; y < y0 + 10; ++y) {
            for (int x = 24; x < 280; ++x) {
                row_has_text = row_has_text ||
                    action_failed_actual.pixelColor(x, y) ==
                        QColor(36, 219, 36);
            }
        }
        action_failed_rows_spaced = action_failed_rows_spaced && row_has_text;
    }
    bool action_row_leaked = false;
    for (int y = 107; y < 118; ++y) {
        for (int x = 24; x < 280; ++x) {
            action_row_leaked = action_row_leaked ||
                action_failed_actual.pixelColor(x, y) == QColor(36, 219, 36);
        }
    }
    check(action_failed_rows_spaced,
          "failed Matrix action message uses separate source text rows",
          failures);
    check(!action_row_leaked && route.top_option_labels.empty(),
          "failed Matrix action message clears the previous action row", failures);
    const auto action_failed_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) /
        "action-failed-message.bmp";
    std::filesystem::create_directories(action_failed_artifact.parent_path());
    check(action_failed_actual.save(
              QString::fromStdString(action_failed_artifact.string()), "BMP"),
          "write failed Matrix action message image artifact", failures);
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 any-edge wait.
    forced_roll_successes = 2;
    runtime.advance_menu_route(0x4000U); // Retry Cancel Alert successfully.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction &&
              route.top_title == "Matrix Message" &&
              route.top_text.find("NO ALERTS ARE") != std::string::npos &&
              route.top_matrix_alert == -1,
          "successful Cancel Alert preserves no-alert state and shows ROM text",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    QImage alert_message_actual(320, 224, QImage::Format_ARGB32);
    alert_message_actual.fill(QColor(255, 0, 255));
    QPainter alert_message_painter(&alert_message_actual);
    const QImage alert_message_font = QImage(QString::fromStdString(
        (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
    const bool alert_message_painted = paint_matrix_node_action_panel(
        alert_message_painter, alert_message_font, route);
    alert_message_painter.end();
    check(alert_message_painted,
          "Cancel Alert message reaches the Qt panel presenter", failures);
    bool alert_message_rows_spaced = true;
    constexpr std::array<int, 4> kAlertMessageRows{50, 74, 110, 134};
    for (const auto y0 : kAlertMessageRows) {
        bool row_has_text = false;
        for (int y = y0; y < y0 + 10; ++y) {
            for (int x = 24; x < 280; ++x) {
                row_has_text = row_has_text ||
                    alert_message_actual.pixelColor(x, y) ==
                        QColor(36, 219, 36);
            }
        }
        alert_message_rows_spaced = alert_message_rows_spaced && row_has_text;
    }
    bool old_action_row_visible = false;
    for (int y = 86; y < 94; ++y) {
        for (int x = 24; x < 280; ++x) {
            old_action_row_visible = old_action_row_visible ||
                alert_message_actual.pixelColor(x, y) == QColor(36, 219, 36);
        }
    }
    check(alert_message_rows_spaced,
          "Cancel Alert ROM message rows render at distinct positions", failures);
    check(!old_action_row_visible && route.top_option_labels.empty(),
          "Cancel Alert redraw removes the prior node action buttons", failures);
    const auto alert_message_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) / "cancel-alert-message.bmp";
    std::filesystem::create_directories(alert_message_artifact.parent_path());
    check(alert_message_actual.save(
              QString::fromStdString(alert_message_artifact.string()), "BMP"),
          "write Cancel Alert message image artifact", failures);
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 any-edge wait.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction && route.top_title == "CPU" &&
              route.top_option_labels.size() == 3U,
          "Cancel Alert message returns to the same node action list", failures);
    runtime.advance_menu_route(0x0100U); // Select Go To Node.
    runtime.advance_menu_route(0x4000U); // Go To Node, with shared action gate.
    check(runtime.menu_route_snapshot().top_title == "Go To Node",
          "CPU Go To Node enters destination selection", failures);
    for (int node = 0; node < 5; ++node) {
        runtime.advance_menu_route(0x0800U);
    }
    runtime.advance_menu_route(0x4000U); // I/O Port, node 6.
    route = runtime.menu_route_snapshot();
    check(route.top_matrix_node == 6 && route.top_title == "I/O Port" &&
              route.top_option_labels ==
                  std::vector<std::string>{"Leave Node", "Lockout"},
          "Go To Node reaches ROM I/O Port node and its Lockout action", failures);
    runtime.advance_menu_route(0x0200U); // Select Lockout.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction &&
              route.top_title == "Matrix Message" &&
              route.top_text.find("NODE ACCESS LOCKED") != std::string::npos &&
              route.top_text.find("DISABLED") != std::string::npos &&
              route.top_option_labels.empty(),
          "Lockout displays all ROM message lines and clears action buttons",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    QImage lockout_actual(320, 224, QImage::Format_ARGB32);
    lockout_actual.fill(QColor(255, 0, 255));
    QPainter lockout_painter(&lockout_actual);
    const QImage lockout_font = QImage(QString::fromStdString(
        (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
    const bool lockout_painted = paint_matrix_node_action_panel(
        lockout_painter, lockout_font, route);
    lockout_painter.end();
    check(lockout_painted,
          "Lockout message reaches the Qt panel presenter", failures);
    bool lockout_rows_spaced = true;
    constexpr std::array<int, 4> kLockoutRows{50, 86, 122, 146};
    for (const auto y0 : kLockoutRows) {
        bool row_has_text = false;
        for (int y = y0; y < y0 + 10; ++y) {
            for (int x = 24; x < 280; ++x) {
                row_has_text = row_has_text ||
                    lockout_actual.pixelColor(x, y) == QColor(36, 219, 36);
            }
        }
        lockout_rows_spaced = lockout_rows_spaced && row_has_text;
    }
    bool old_lockout_action_visible = false;
    for (int y = 107; y < 118; ++y) {
        for (int x = 24; x < 280; ++x) {
            old_lockout_action_visible = old_lockout_action_visible ||
                lockout_actual.pixelColor(x, y) == QColor(36, 219, 36);
        }
    }
    check(lockout_rows_spaced,
          "Lockout's four text groups render on separated rows", failures);
    check(!old_lockout_action_visible,
          "Lockout redraw does not leave its old action label underneath",
          failures);
    const auto lockout_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) / "lockout-message.bmp";
    std::filesystem::create_directories(lockout_artifact.parent_path());
    check(lockout_actual.save(QString::fromStdString(lockout_artifact.string()),
                              "BMP"),
          "write Lockout message image artifact", failures);
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 any-edge wait.
    runtime.advance_menu_route(0x0100U); // Leave Node from the I/O Port.
    runtime.advance_menu_route(0x4000U);
    runtime.advance_menu_route(0x4000U); // Follow the first link back to CPU.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "CPU" && route.top_matrix_node == 4,
          "I/O Port Leave Node follows its ROM link back to CPU", failures);
    runtime.advance_menu_route(0x4000U); // CPU Go To Node again.
    check(runtime.menu_route_snapshot().top_title == "Go To Node",
          "CPU action gate remains usable after Lockout returns", failures);
    check(action_rolls.size() == 7U,
          "all seven wired Matrix action attempts use the shared ROM gate",
          failures);
    runtime.advance_menu_route(0x0800U); // Next node.
    runtime.advance_menu_route(0x4000U); // Confirm destination.
    route = runtime.menu_route_snapshot();
    check(route.top_matrix_node == 1,
          "Go To Node commits the highlighted catalog node", failures);
    check(route.top_option_labels ==
              std::vector<std::string>{"Leave Node", "Turn Off Node"},
          "ROM node class 0x29 selects the Turn Off Node action row", failures);
    runtime.advance_menu_route(0x0200U); // Turn Off Node.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "NODE IS OFFLINE" &&
              route.top_option_labels.empty() &&
              matrix_world_state.field.maglocks_off ==
                  std::optional<bool>{true} && stopped_sounds.empty(),
          "Turn Off Node class 0x29 commits maglocks and displays ROM text",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    QImage offline_actual(320, 224, QImage::Format_ARGB32);
    offline_actual.fill(QColor(255, 0, 255));
    QPainter offline_painter(&offline_actual);
    const QImage offline_font = QImage(QString::fromStdString(
        (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
    const bool offline_painted = paint_matrix_node_action_panel(
        offline_painter, offline_font, route);
    offline_painter.end();
    bool old_node_action_pixels = false;
    for (const int row_y : {85, 107, 129}) {
        for (int y = row_y; y < row_y + 10; ++y) {
            for (int x = 24; x < 200; ++x) {
                old_node_action_pixels = old_node_action_pixels ||
                    offline_actual.pixelColor(x, y) == QColor(36, 219, 36);
            }
        }
    }
    check(offline_painted && !old_node_action_pixels,
          "Turn Off Node message redraw clears prior menu labels", failures);
    const auto offline_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) /
        "turn-off-node-message.bmp";
    std::filesystem::create_directories(offline_artifact.parent_path());
    check(offline_actual.save(QString::fromStdString(offline_artifact.string()),
                              "BMP"),
          "write Turn Off Node ROM message artifact", failures);
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 wait returns to node.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Slave Module" &&
              route.top_option_labels ==
                  std::vector<std::string>{"Leave Node", "Turn Off Node"},
          "Turn Off Node message returns to the same slave-module node",
          failures);

    runtime.advance_menu_route(0x0100U); // Select Leave Node.
    runtime.advance_menu_route(0x4000U); // Open its ROM links.
    runtime.advance_menu_route(0x4000U); // Enter the first linked datastore.
    route = runtime.menu_route_snapshot();
    check(route.top_matrix_node == 2 && route.top_title == "Datastore" &&
              route.top_option_labels == std::vector<std::string>{
                  "Leave Node", "Transfer Data", "Erase"},
          "ROM node navigation exposes Erase on a datastore",
          failures);

    // Transfer Data runs the source search cadence before presenting the ROM
    // file name and Yes/No choice. The typed runner/deck fields here are the
    // same +0x7B, +0x88, and deck +0x0B inputs read by 0x54D6A.
    party[0].attributes[1] = 6U;
    party[0].attributes[4] = 6U;
    party[0].skills[7] = 6U;
    forced_roll_successes = 2;
    runtime.advance_menu_route(0x0200U); // Select Transfer Data.
    const auto transfer_gate_count = action_rolls.size();
    const auto transfer_rng_count = random_bounds.size();
    runtime.advance_menu_route(0x4000U); // A passes the shared node gate.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "   Browsing the|datastore you find|" &&
              route.top_option_labels.empty() &&
              action_rolls.size() == transfer_gate_count + 1U &&
              random_bounds.size() == transfer_rng_count,
          "Transfer Data begins the ROM search without rolling before its first wait",
          failures);
    for (int frame = 0; frame < 19; ++frame) {
        runtime.advance_menu_route(0U);
    }
    check(runtime.menu_route_snapshot().top_text ==
              "   Browsing the|datastore you find|",
          "the first search dot waits a full 20 frames", failures);
    runtime.advance_menu_route(0U);
    route = runtime.menu_route_snapshot();
    check(route.top_text == "   Browsing the|datastore you find|.",
          "the first source dot appears on frame 20", failures);

#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    const auto save_transfer_frame = [&failures](
                                         const std::string& filename,
                                         const auto& snapshot) {
        QImage image(320, 224, QImage::Format_ARGB32);
        image.fill(QColor(255, 0, 255));
        QPainter painter(&image);
        const QImage font(QImage(QString::fromStdString(
            (std::filesystem::path(SHADOWRUN_MATRIX_MAP_ASSET_ROOT) /
             "cyberspace/matrix/system-map-font.bmp")
                .string())));
        const bool painted = paint_matrix_node_action_panel(
            painter, font, snapshot);
        painter.end();
        bool stale_pixels = false;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                stale_pixels = stale_pixels ||
                    image.pixelColor(x, y) == QColor(255, 0, 255);
            }
        }
        check(painted && !stale_pixels,
              "Transfer Data frame fully redraws without old-menu pixels",
              failures);
        const auto artifact = std::filesystem::path(
            SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) / filename;
        std::filesystem::create_directories(artifact.parent_path());
        check(image.save(QString::fromStdString(artifact.string()), "BMP"),
              "write Transfer Data route frame", failures);
    };
    const auto replaces_previous_frame = [&node_assets](
                                             const auto& previous,
                                             const auto& current) {
        const QImage font(QImage(QString::fromStdString(
            (node_assets / "cyberspace/matrix/system-map-font.bmp")
                .string())));
        QImage reused(320, 224, QImage::Format_ARGB32);
        reused.fill(QColor(255, 0, 255));
        {
            QPainter painter(&reused);
            const bool previous_painted =
                paint_matrix_node_action_panel(painter, font, previous);
            const bool current_painted =
                paint_matrix_node_action_panel(painter, font, current);
            if (!previous_painted || !current_painted) return false;
        }
        QImage fresh(320, 224, QImage::Format_ARGB32);
        fresh.fill(QColor(255, 0, 255));
        {
            QPainter painter(&fresh);
            if (!paint_matrix_node_action_panel(painter, font, current)) {
                return false;
            }
        }
        return reused == fresh;
    };
    save_transfer_frame("transfer-search-first-dot.bmp", route);
#endif

    for (int dot = 2; dot <= 4; ++dot) {
        for (int frame = 0; frame < 20; ++frame) {
            runtime.advance_menu_route(0U);
        }
        route = runtime.menu_route_snapshot();
        check(route.top_text ==
                  "   Browsing the|datastore you find|" +
                      std::string(static_cast<std::size_t>(dot), '.'),
              "search dots retain the ROM's 20-frame cadence", failures);
    }
    for (int frame = 0; frame < 9; ++frame) {
        runtime.advance_menu_route(0U);
    }
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Download File?" &&
              route.top_text ==
                  "Bank Acct. Numbers|     . . . . . . Mp 20" &&
              route.top_option_labels ==
                  std::vector<std::string>{"Yes", "No"} &&
              route.top_row == 0 &&
              random_bounds.size() == transfer_rng_count + 3U,
          "successful search previews the ROM file after the final 9-frame pause",
          failures);
    const auto transfer_file_prompt = route;
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("actual.bmp", route);
#endif
    runtime.advance_menu_route(0x0200U); // Down selects No.
    route = runtime.menu_route_snapshot();
    check(route.top_row == 1 && route.top_option_labels ==
              std::vector<std::string>{"Yes", "No"},
          "Transfer Data selection moves between Yes and No", failures);
    runtime.advance_menu_route(0x2000U); // B is ignored by the ROM prompt.
    check(runtime.menu_route_snapshot().top_title == "Download File?" &&
              matrix_deck_commits == 0,
          "download confirmation accepts A only", failures);
    runtime.advance_menu_route(0x4000U); // A cancels.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "Download Cancelled." &&
              matrix_deck_commits == 0,
          "No prints the ROM cancellation text without changing the deck",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    check(replaces_previous_frame(transfer_file_prompt, route),
          "Transfer Data message redraw removes the prior Yes/No panel",
          failures);
    save_transfer_frame("transfer-download-cancelled.bmp", route);
#endif
    runtime.advance_menu_route(0x2000U); // Any edge dismisses the message.
    check(runtime.menu_route_snapshot().top_title == "Datastore",
          "canceled download returns to the same datastore", failures);

    runtime.advance_menu_route(0x4000U); // Retry Transfer Data; Yes is default.
    for (int frame = 0; frame < 89; ++frame) {
        runtime.advance_menu_route(0U);
    }
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Download File?" && route.top_row == 0,
          "a new transfer resets the prompt to Yes", failures);
    runtime.advance_menu_route(0x4000U); // A downloads the file.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "File Download|  Complete." &&
              matrix_deck_commits == 1 && matrix_deck.files[0].id == 45 &&
              matrix_deck.files[0].size == 20 &&
              random_bounds.size() == transfer_rng_count + 8U,
          "Yes writes the discovered file through the typed deck commit",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-download-complete.bmp", route);
#endif
    runtime.advance_menu_route(0x2000U);

    matrix_world_state.run = ScriptMatrixRunState{0, 0, 0, 0, 1, 0};
    matrix_search_miss = true;
    runtime.advance_menu_route(0x4000U); // Search again with a forced miss.
    for (int frame = 0; frame < 89; ++frame) {
        runtime.advance_menu_route(0U);
    }
    matrix_search_miss = false;
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "nothing of interest." &&
              matrix_deck_commits == 1,
          "failed detection prints the ROM no-interest text without a write",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-no-result.bmp", route);
#endif
    runtime.advance_menu_route(0x2000U);

    matrix_deck.storage = 20U; // The first 20 Mp file consumes all storage.
    runtime.advance_menu_route(0x4000U);
    for (int frame = 0; frame < 89; ++frame) {
        runtime.advance_menu_route(0U);
    }
    runtime.advance_menu_route(0x4000U); // Yes reaches the storage check.
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text == "No available|storage!" &&
              matrix_deck_commits == 1,
          "insufficient storage uses the ROM two-line message without a write",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-no-storage.bmp", route);
#endif
    matrix_deck.storage = 1000U;
    runtime.advance_menu_route(0x2000U);
    runtime.advance_menu_route(0x0100U); // Restore the Leave Node row.
    check(runtime.menu_route_snapshot().top_row == 0,
          "returning from Transfer Data restores node action selection",
          failures);

    matrix_world_state.run = ScriptMatrixRunState{-1, 9, 12, 30, 1, 0};
    runtime.advance_menu_route(0x0200U);
    const auto objective_rng_count = random_bounds.size();
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Download File?" &&
              route.top_text ==
                  "Personnel Dossiers|     . . . . . . Mp 30" &&
              random_bounds.size() == objective_rng_count,
          "matching run objective skips search dots and uses its value as file ID",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-objective-preview.bmp", route);
#endif
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_text ==
              "  Shadowrun|Accomplished!||File Download|  Complete." &&
              matrix_deck_commits == 2 && matrix_deck.files[1].id == 12 &&
              matrix_deck.files[1].size == 30 &&
              matrix_deck.files[1].value == -1 &&
              matrix_world_state.run.has_value() &&
              matrix_world_state.run->state == 1,
          "objective download stores the ROM marker and completes the run",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-objective-complete.bmp", route);
#endif
    runtime.advance_menu_route(0x2000U);

    matrix_world_state.run = ScriptMatrixRunState{-1, 9, -1, 30, 1, 0};
    const auto upload_rng_count = random_bounds.size();
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text ==
                  "You upload the file| to the datastore.|| Run accomplished!" &&
              matrix_world_state.run->state == 1 &&
              matrix_deck_commits == 2 &&
              random_bounds.size() == upload_rng_count,
          "negative objective value uploads immediately without a datafile",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_transfer_frame("transfer-upload-complete.bmp", route);
#endif
    runtime.advance_menu_route(0x2000U);
    runtime.advance_menu_route(0x0100U); // Restore the Leave Node row.

    matrix_world_state.run = ScriptMatrixRunState{-1, 9, 0, 30, 1, 0};
    forced_roll_successes = 2;
    runtime.advance_menu_route(0x0200U); // Transfer Data.
    runtime.advance_menu_route(0x0200U); // Erase.
    const auto erase_gate_count = action_rolls.size();
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text ==
                  "  The recog-code| rapidly finds the|data and zaps it to| electron heaven!!|| Run accomplished!" &&
              route.top_option_labels.empty() &&
              matrix_world_state.run.has_value() &&
              matrix_world_state.run->state == 1 &&
              matrix_world_state.run->completion_flags == 0 &&
              matrix_world_state.run->run_flags == 0x80U &&
              action_rolls.size() == erase_gate_count + 1U,
          "objective Erase uses the shared gate, ROM class/value checks, and completes the run",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    const auto save_erase_message = [&](const std::string& filename) {
        QImage image(320, 224, QImage::Format_ARGB32);
        image.fill(QColor(255, 0, 255));
        QPainter painter(&image);
        const QImage font = QImage(QString::fromStdString(
            (node_assets / "cyberspace/matrix/system-map-font.bmp").string()));
        const bool painted = paint_matrix_node_action_panel(painter, font, route);
        painter.end();
        bool stale_pixels = false;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                stale_pixels = stale_pixels ||
                    image.pixelColor(x, y) == QColor(255, 0, 255);
            }
        }
        check(painted && !stale_pixels && route.top_option_labels.empty(),
              "Erase message replaces the prior datastore buttons cleanly",
              failures);
        const auto artifact = std::filesystem::path(
            SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) / filename;
        std::filesystem::create_directories(artifact.parent_path());
        check(image.save(QString::fromStdString(artifact.string()), "BMP"),
              "write Erase ROM message image artifact", failures);
    };
    save_erase_message("erase-objective-message.bmp");
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 wait returns to datastore.
    runtime.advance_menu_route(0x0100U); // Leave Node.
    runtime.advance_menu_route(0x0100U);
    runtime.advance_menu_route(0x4000U); // Open Leave Node links.
    runtime.advance_menu_route(0x0800U); // Choose the second link to Slave Module.
    runtime.advance_menu_route(0x4000U);
    check(runtime.menu_route_snapshot().top_matrix_node == 1,
          "datastore link navigation returns to the slave module",
          failures);

    runtime.advance_menu_route(0x4000U); // Leave Node from the slave module.
    runtime.advance_menu_route(0x4000U); // First link returns to datastore.
    route = runtime.menu_route_snapshot();
    matrix_world_state.run = ScriptMatrixRunState{-1, 5, 0, 30, 1, 0};
    forced_roll_successes = 2;
    runtime.advance_menu_route(0x0200U); // Transfer Data.
    runtime.advance_menu_route(0x0200U); // Erase.
    const auto ordinary_erase_gate_count = action_rolls.size();
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_title == "Matrix Message" &&
              route.top_text ==
                  "You find a file that| appears important| and obliterate it|  from memory!!!|| Score one for the|downtrodden hacker!" &&
              route.top_option_labels.empty() &&
              matrix_world_state.run.has_value() &&
              matrix_world_state.run->state == -1 &&
              action_rolls.size() == ordinary_erase_gate_count + 1U,
          "ordinary Erase uses the shared gate without completing another run",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    save_erase_message("erase-file-message.bmp");
#endif
    runtime.advance_menu_route(0x4000U); // ROM D94 wait returns to datastore.
    runtime.advance_menu_route(0x0100U); // Leave Node.
    runtime.advance_menu_route(0x0100U);
    runtime.advance_menu_route(0x4000U); // Open Leave Node links.
    runtime.advance_menu_route(0x0800U); // Choose Slave Module.
    runtime.advance_menu_route(0x4000U);
    check(runtime.menu_route_snapshot().top_matrix_node == 1,
          "ordinary Erase returns to the original slave module",
          failures);
    runtime.advance_menu_route(0x4000U); // Leave Node from the Slave Module.
    runtime.advance_menu_route(0x0800U); // Choose its CPU link.
    runtime.advance_menu_route(0x4000U);
    check(runtime.menu_route_snapshot().top_matrix_node == 4,
          "the erase route can return through the ROM CPU node", failures);
    runtime.advance_menu_route(0x4000U); // CPU Go To Node.
    runtime.advance_menu_route(0x0800U); // Select Slave Module.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.top_matrix_node == 1 &&
              route.top_matrix_previous_node == 4,
          "erase route restores the prior-node relationship used by Retreat",
          failures);
    matrix_world_state.run = ScriptMatrixRunState{-1, -1, 0, 0, 1, 0};
    runtime.advance_menu_route(0x8000U); // Start enters the deck loop.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCyberdeck &&
              route.top_option_labels.size() == 16U &&
              route.top_matrix_state_present && route.top_matrix_node == 1 &&
              deck_enter_count == 1,
          "Start enters the Cyberdeck loop while preserving the selected node",
          failures);
    runtime.advance_menu_route(0x0100U); // Up wraps to Map row 3.
    runtime.advance_menu_route(0x0800U);
    runtime.advance_menu_route(0x0800U); // Column 2: Map.
    runtime.advance_menu_route(0x4000U); // Map utility.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDeckMap && route.top_option_labels.size() == 2U &&
              route.top_matrix_state_present && route.top_matrix_system_id == 0,
          "online Matrix session opens the Map and Retreat utility",
          failures);
    check(deck_tick_count == 4,
          "online deck frame reaches Lua heartbeat callback", failures);
    check(deck_mode == -1,
          "A selects the signed host attack mode before opening the Map utility",
          failures);
    runtime.advance_menu_route(0x4000U); // Show map.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kDeckMap &&
              route.top_matrix_map_cells_present &&
              route.top_matrix_map_labels_present &&
              route.top_matrix_map_labels ==
                  std::array<std::string, 6>{"CPU", "SPU", "IOP", "SAN", "SM", "DS"} &&
              std::all_of(
                  route.top_matrix_map_cells.begin(),
                  route.top_matrix_map_cells.end(), [](const auto& cell) {
                      return std::all_of(
                          cell.tile_words.begin(), cell.tile_words.end(),
                          [](const auto word) {
                              const auto pattern = word & 0x07FFU;
                              const auto palette = word & 0x6000U;
                              return pattern >= 0x00BFU &&
                                     pattern <= 0x00D8U &&
                                     palette >= 0x2000U &&
                                     palette <= 0x6000U;
                          });
                  }),
          "Matrix map exports all 140 source tiles and six source labels", failures);

#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    const std::filesystem::path matrix_assets(SHADOWRUN_MATRIX_MAP_ASSET_ROOT);
    const auto load_matrix_image = [&matrix_assets](const std::string& relative) {
        return QImage(QString::fromStdString(
            (matrix_assets / relative).string()));
    };
    const auto map_background = load_matrix_image(
        "cyberspace/matrix/system-map-background.bmp");
    const auto map_font = load_matrix_image(
        "cyberspace/matrix/system-map-font.bmp");
    std::array<QImage, 4> map_tile_atlases{};
    for (std::size_t palette = 1U; palette <= 3U; ++palette) {
        map_tile_atlases[palette] = load_matrix_image(
            "cyberspace/matrix/map-tiles-palette-" +
            std::to_string(palette) + ".bmp");
    }
    QImage map_actual(320, 224, QImage::Format_ARGB32);
    map_actual.fill(QColor(255, 0, 255));
    QPainter map_painter(&map_actual);
    const bool map_painted = paint_matrix_system_map(
        map_painter, map_background, map_font, map_tile_atlases, route);
    map_painter.end();
    check(map_painted,
          "live Lua Matrix map snapshot reaches the Qt map presenter", failures);
    bool stale_menu_pixel = false;
    bool green_label_pixel = false;
    for (int y = 0; y < map_actual.height(); ++y) {
        for (int x = 0; x < map_actual.width(); ++x) {
            if (map_actual.pixelColor(x, y) == QColor(255, 0, 255)) {
                stale_menu_pixel = true;
            }
            if (y >= 32 && y < 40 && x >= 16 && x < 40) {
                const auto pixel = map_actual.pixelColor(x, y);
                green_label_pixel = green_label_pixel ||
                    (pixel.green() > pixel.red() &&
                     pixel.green() > pixel.blue());
            }
        }
    }
    check(!stale_menu_pixel,
          "Matrix map redraw clears previous-screen graphics", failures);
    check(green_label_pixel,
          "Matrix map uses its source text palette", failures);
    constexpr std::array<int, 6> kMapLabelRows{32, 64, 96, 128, 160, 192};
    bool source_palette_label_pixel = true;
    for (const auto row : kMapLabelRows) {
        bool label_at_source_row = false;
        for (int y = row; y < row + 8; ++y) {
            for (int x = 16; x < 48; ++x) {
                const auto pixel = map_actual.pixelColor(x, y);
                if (pixel == QColor(36, 219, 36) &&
                    pixel != map_background.pixelColor(x, y)) {
                    label_at_source_row = true;
                }
            }
        }
        source_palette_label_pixel =
            source_palette_label_pixel && label_at_source_row;
    }
    check(source_palette_label_pixel,
          "all six Matrix labels use the ROM 0x2000 palette and 32-pixel row spacing",
          failures);
    bool labels_stacked_in_top_rows = false;
    for (int y = 40; y < 64; ++y) {
        for (int x = 16; x < 48; ++x) {
            const auto pixel = map_actual.pixelColor(x, y);
            labels_stacked_in_top_rows = labels_stacked_in_top_rows ||
                (pixel == QColor(36, 219, 36) &&
                 pixel != map_background.pixelColor(x, y));
        }
    }
    check(!labels_stacked_in_top_rows,
          "Matrix labels do not stack in the top rows", failures);
    check(map_actual.pixelColor(16, 32) ==
              map_background.pixelColor(16, 32),
          "transparent map glyph pixels preserve the backdrop", failures);
    bool blank_cell_preserves_grid = false;
    for (std::size_t cell = 0U;
         cell < route.top_matrix_map_cells.size(); ++cell) {
        const auto& visual = route.top_matrix_map_cells[cell];
        const bool blank = std::all_of(
            visual.tile_words.begin(), visual.tile_words.end(),
            [](const auto word) { return (word & 0x07FFU) == 0x00BFU; });
        if (!blank) {
            continue;
        }
        const auto x = 64 + static_cast<int>(cell % 14U) * 16 + 3;
        const auto y = 24 + static_cast<int>(cell / 14U) * 16 + 3;
        blank_cell_preserves_grid = map_actual.pixelColor(x, y) ==
                                    map_background.pixelColor(x, y);
        break;
    }
    check(blank_cell_preserves_grid,
          "unknown map cells leave the plane-B grid visible", failures);
    const auto map_artifact = std::filesystem::path(
        SHADOWRUN_MATRIX_MAP_ARTIFACT_DIR) / "actual.bmp";
    std::filesystem::create_directories(map_artifact.parent_path());
    check(map_actual.save(QString::fromStdString(map_artifact.string()), "BMP"),
          "write live Matrix map actual.bmp artifact", failures);
#endif
    runtime.advance_menu_route(0x8000U); // Return to deck grid.
    check(runtime.menu_route_snapshot().top_screen == kCyberdeck,
          "Matrix map returns to the active deck screen", failures);
    runtime.advance_menu_route(0x8000U); // Return to node-action loop.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction && route.top_matrix_node == 1 &&
              deck_leave_count == 1,
          "deck return resumes the same Lua Matrix node", failures);
    runtime.advance_menu_route(0x8000U); // Re-enter the deck loop.
    runtime.advance_menu_route(0x0100U); // Up wraps to row 3.
    runtime.advance_menu_route(0x0800U);
    runtime.advance_menu_route(0x0800U); // Select Map.
    runtime.advance_menu_route(0x4000U); // Open Map/Retreat.
    runtime.advance_menu_route(0x0200U); // Select Retreat.
    runtime.advance_menu_route(0x4000U); // Confirm source retreat action.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kMatrixNodeAction && route.top_matrix_node == 4 &&
              route.top_matrix_previous_node == 1 && deck_leave_count == 2,
          "confirmed retreat returns to the prior node-action loop", failures);
    runtime.advance_menu_route(0x8000U); // Re-enter deck to pull the plug.
    for (int row = 0; row < 3; ++row) runtime.advance_menu_route(0x0200U);
    for (int column = 0; column < 3; ++column) runtime.advance_menu_route(0x0800U);
    runtime.advance_menu_route(0x4000U); // Pull Plug prompt.
    check(runtime.menu_route_snapshot().top_screen == 30U,
          "Matrix logout reaches the ROM Pull Plug confirmation", failures);
    runtime.advance_menu_route(0x0100U); // Change from No to Yes.
    runtime.advance_menu_route(0x4000U); // Confirm logout.
    route = runtime.menu_route_snapshot();
    check(route.open && route.top_screen == kCommand &&
              route.page == kCommandPagePocketSecretary &&
              deck_leave_count == 3 && closed_count == 3,
          "Matrix exit clears hidden children and rebuilds Command on page 6",
          failures);
    runtime.advance_menu_route(0x4000U); // Reopen Pocket Secretary.
    for (int row = 0; row < 5; ++row) {
        runtime.advance_menu_route(0x0200U);
    }
    runtime.advance_menu_route(0x4000U); // Debug row is still unlocked.
    check(runtime.menu_route_snapshot().top_screen == kDebugMenu,
          "debug menu remains unlocked after the Matrix return",
          failures);

    // Crash System is an automatic source countdown, not a dismissible
    // message: 3, 2, 1, 0 each remain for 60 VBlank frames before logout.
    for (int row = 0; row < 3; ++row) runtime.advance_menu_route(0x0200U);
    runtime.advance_menu_route(0x4000U); // Pick System.
    runtime.advance_menu_route(0x4000U); // Jack in to system zero.
    for (int frame = 0; frame < 20; ++frame) runtime.advance_menu_route(0U);
    runtime.advance_menu_route(0x4000U); // Enter System.
    runtime.advance_menu_route(0x4000U); // First link enters CPU.
    runtime.advance_menu_route(0x0200U);
    runtime.advance_menu_route(0x0200U); // Crash System row.
    runtime.advance_menu_route(0x4000U);
    route = runtime.menu_route_snapshot();
    check(route.open && route.top_screen == kMatrixNodeAction &&
              route.top_title == "Matrix Message" &&
              route.top_text ==
                  "  INITIATING||SYSTEM  SHUTDOWN|| IN - - SECONDS" &&
              route.top_matrix_countdown == 3 &&
              route.top_option_labels.empty(),
          "Crash System shows its ROM message and starts at three", failures);
    check(matrix_world_state.run.has_value() &&
              matrix_world_state.run->state == 1 &&
              matrix_world_state.run->completion_flags == 0 &&
              matrix_world_state.run->run_flags == 0x80U &&
              matrix_world_state.field.alarm == std::optional<bool>{false} &&
              matrix_world_state.field.security_off ==
                  std::optional<bool>{true} &&
              matrix_world_state.field.maglocks_off ==
                  std::optional<bool>{true},
          "Crash System commits its source run-completion and field flags through the host", failures);
    check(matrix_world_state.datastore.has_value() &&
              matrix_world_state.datastore->tips_page3_flags ==
                  0xA2345678U &&
              matrix_world_state.datastore->tips_page2_flags ==
                  0xCDEF1234U &&
              matrix_world_state.datastore->world_event_flags ==
                  0x81234567U &&
              matrix_world_state.datastore->group_item_flags ==
                  std::optional<std::uint32_t>{0x6023U} &&
              matrix_world_state.datastore->nuyen ==
                  std::optional<std::uint32_t>{0x12345678U} &&
              matrix_world_state.datastore->mr_johnsons_met_flags ==
                  std::optional<std::uint8_t>{0x08U},
          "Matrix datastore flag words survive the Lua-to-host state transaction",
          failures);
    check(matrix_world_state.actors.size() == 2U &&
              matrix_world_state.actors[0].flags_9e == 0x06U &&
              matrix_world_state.actors[1].flags_9e == 0x04U &&
              stopped_sounds == std::vector<std::uint16_t>{0x30U},
          "alarm-off applies the ROM actor flag change and stops sound 0x30",
          failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    const auto capture_shutdown_frame = [&](const int expected_countdown) {
        const auto frame = runtime.menu_route_snapshot();
        QImage image(320, 224, QImage::Format_ARGB32);
        image.fill(QColor(255, 0, 255));
        QPainter painter(&image);
        const bool painted = paint_matrix_node_action_panel(
            painter, node_font, frame);
        painter.end();
        bool rows_visible = true;
        for (const int row_y : {80, 96, 112}) {
            bool visible = false;
            for (int y = row_y; y < row_y + 8; ++y) {
                for (int x = 96; x < 240; ++x) {
                    visible = visible || image.pixelColor(x, y) ==
                                             QColor(36, 219, 36);
                }
            }
            rows_visible = rows_visible && visible;
        }
        bool digit_visible = false;
        bool stale_button_visible = false;
        bool selector_visible = false;
        for (int y = 112; y < 120; ++y) {
            for (int x = 136; x < 144; ++x) {
                digit_visible = digit_visible || image.pixelColor(x, y) ==
                                                    QColor(36, 219, 36);
            }
        }
        for (int y = 48; y < 160; ++y) {
            for (int x = 24; x < 88; ++x) {
                stale_button_visible = stale_button_visible ||
                    image.pixelColor(x, y) == QColor(36, 219, 36);
            }
        }
        for (int y = 40; y < 200; ++y) {
            for (int x = 20; x < 300; ++x) {
                selector_visible = selector_visible ||
                    image.pixelColor(x, y) == QColor(196, 190, 39);
            }
        }
        check(painted && frame.top_matrix_countdown == expected_countdown &&
                  rows_visible && digit_visible,
              "Crash System countdown frame has its ROM rows and digit",
              failures);
        check(!stale_button_visible && !selector_visible,
              "Crash System redraw removes old buttons and selector", failures);
        const auto artifact = std::filesystem::path(
            SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR) /
            ("shutdown-countdown-" + std::to_string(expected_countdown) +
             ".bmp");
        check(image.save(QString::fromStdString(artifact.string()), "BMP"),
              "write Crash System countdown image artifact", failures);
    };
    capture_shutdown_frame(3);
#endif
    for (int frame = 0; frame < 59; ++frame) runtime.advance_menu_route(0U);
    check(runtime.menu_route_snapshot().top_matrix_countdown == 3,
          "Crash System holds each count for 60 frames", failures);
    runtime.advance_menu_route(0U);
    check(runtime.menu_route_snapshot().top_matrix_countdown == 2,
          "Crash System changes from three to two after 60 frames", failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
    capture_shutdown_frame(2);
#endif
    for (const int expected : {2, 1, 0}) {
        for (int frame = 0; frame < 60; ++frame) {
            runtime.advance_menu_route(0U);
        }
        if (expected > 0) {
            check(runtime.menu_route_snapshot().top_matrix_countdown ==
                      expected - 1,
                  "Crash System follows its source countdown sequence", failures);
#ifdef SHADOWRUN_MATRIX_MAP_QT_TEST
            capture_shutdown_frame(expected - 1);
#endif
        }
    }
    check(!runtime.menu_route_snapshot().open && closed_count == 4,
          "Crash System logs out after the final 60-frame zero", failures);
    check(runtime.open_menu_route(kCommand),
          "reopen command after Matrix Crash System logout", failures);
    runtime.advance_menu_route(0x4000U); // Pocket Secretary.
    for (int row = 0; row < 5; ++row) runtime.advance_menu_route(0x0200U);
    runtime.advance_menu_route(0x4000U); // Debug menu remains unlocked.
    check(runtime.menu_route_snapshot().top_screen == kDebugMenu,
          "Crash System logout preserves the unlocked debug menu", failures);
    runtime.close_menu_route();
    check(!runtime.menu_route_snapshot().open && closed_count == 5,
          "close rebuilt command route after the Matrix QA path", failures);

    if (failures != 0) {
        std::cerr << "menu route failures: " << failures << '\n';
        return 1;
    }
    std::cout << "menu route checks passed\n";
    return 0;
}
