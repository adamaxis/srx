#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using shadowrun::ghidra_conversion::scripting::GameScriptRuntime;
using shadowrun::ghidra_conversion::scripting::ScriptMenuRouteBindings;

namespace {

constexpr std::uint16_t kCommand = 1U;
constexpr std::uint16_t kPosture = 2U;
constexpr std::uint16_t kAttributes = 5U;
constexpr std::uint16_t kPocketSecretary = 9U;
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
    bool legacy_active = false;
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
    bindings.game.leader_incapacitated = [] { return false; };
    bindings.game.reselect_leader = [&reselect_count] { ++reselect_count; };
    bindings.game.leader_posture = [&posture] { return posture; };
    bindings.game.set_leader_posture = [&posture](const int value) {
        posture = value;
    };
    bindings.game.leader_archetype = [] { return 3; };
    bindings.game.inventory_item = [](int, int) { return false; };
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
    runtime.advance_menu_route(0x1000U); // A: Pocket Secretary child.
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kPocketSecretary && route.page == 6,
          "A pushes Pocket Secretary while preserving command selection",
          failures);
    check(!legacy_screens.empty() &&
              legacy_screens.back() == kPocketSecretary && legacy_active,
          "legacy child opens through the host adapter", failures);
    legacy_active = false;
    runtime.advance_menu_route(0U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 6,
          "legacy return rebuilds its parent command page", failures);

    runtime.advance_menu_route(0x0400U); // page 6 -> 5
    runtime.advance_menu_route(0x0400U); // page 5 -> 4
    runtime.advance_menu_route(0x0400U); // page 4 -> 3
    check(runtime.menu_route_snapshot().page == 3,
          "command page navigation reaches Attributes/Skills", failures);
    runtime.advance_menu_route(0x1000U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kAttributes && !windows.empty() &&
              windows.back() == kHiddenWindow,
          "Attributes/Skills adapter hides the sidebar window", failures);
    legacy_active = false;
    runtime.advance_menu_route(0U);
    route = runtime.menu_route_snapshot();
    check(route.top_screen == kCommand && route.page == 6 &&
              windows.back() == kSidebarWindow,
          "Attributes/Skills return fully reinitializes page six", failures);

    runtime.advance_menu_route(0x2000U); // C: reselect and refresh.
    route = runtime.menu_route_snapshot();
    check(route.page == 6 && reselect_count == 1,
          "C reselects the leader and returns to page six", failures);
    check(std::find(sounds.begin(), sounds.end(), kSelectSound) !=
              sounds.end(),
          "command action uses the select sound", failures);
    runtime.close_menu_route();
    check(!runtime.menu_route_snapshot().open && closed_count == 2,
          "host scene transition closes the complete route", failures);

    if (failures != 0) {
        std::cerr << "menu route failures: " << failures << '\n';
        return 1;
    }
    std::cout << "menu route checks passed\n";
    return 0;
}
