#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

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
using shadowrun::ghidra_conversion::scripting::ScriptMenuPartyMember;
using shadowrun::ghidra_conversion::scripting::ScriptMenuRouteBindings;

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

} // namespace

int main() {
#ifndef SHADOWRUN_GAMEPLAY_SCRIPT_ROOT
#error SHADOWRUN_GAMEPLAY_SCRIPT_ROOT must point at gameplay/scripts
#endif

    int failures = 0;
    GameScriptRuntime runtime;
    if (!runtime.load_package(
            std::filesystem::path(SHADOWRUN_GAMEPLAY_SCRIPT_ROOT))) {
        std::cerr << "could not load gameplay scripts: " << runtime.failure()
                  << '\n';
        return 1;
    }

    std::vector<std::uint16_t> sounds;
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
    bool pocket_extra_row = false;
    std::vector<int> random_bounds;
    bool random_high_branch = false;
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
    bindings.game.roll_successes = [&first_aid_roll](const int count,
                                                     const int threshold) {
        first_aid_roll = {count, threshold};
        return 2;
    };
    bindings.game.pocket_extra_row = [&pocket_extra_row] {
        return pocket_extra_row;
    };
    bindings.game.source_random_bound = [&random_bounds,
                                         &random_high_branch](const int bound) {
        random_bounds.push_back(bound);
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
    // enters the online Cyberdeck route. The selected system stays attached
    // through Map, and Start rebuilds Command. The 3D Matrix loop is still a
    // separate unfinished integration hill.
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
    check(route.top_screen == kCyberdeck &&
              route.top_option_labels.size() == 16U &&
              route.top_matrix_state_present &&
              route.top_matrix_system_id == 0 && deck_enter_count == 1,
          "jack-in opens the online deck route with the selected system",
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
              std::count(route.top_text.begin(), route.top_text.end(), '|') == 9 &&
              route.top_text.find('@') != std::string::npos,
          "Matrix map shows the source 14x10 grid and current node", failures);
    runtime.advance_menu_route(0x8000U); // Return to Matrix grid.
    check(runtime.menu_route_snapshot().top_screen == kCyberdeck,
          "Matrix map returns to the active deck screen", failures);
    runtime.advance_menu_route(0x8000U); // Leave the Matrix session.
    route = runtime.menu_route_snapshot();
    check(route.open && route.top_screen == kCommand &&
              route.page == kCommandPagePocketSecretary &&
              deck_leave_count == 1 && closed_count == 3,
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
    runtime.close_menu_route();
    check(!runtime.menu_route_snapshot().open && closed_count == 4,
          "close rebuilt command route after the Matrix QA path", failures);

    if (failures != 0) {
        std::cerr << "menu route failures: " << failures << '\n';
        return 1;
    }
    std::cout << "menu route checks passed\n";
    return 0;
}
