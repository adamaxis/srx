#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using shadowrun::ghidra_conversion::scripting::GameScriptRuntime;
using shadowrun::ghidra_conversion::scripting::ScriptMenuPartyMember;
using shadowrun::ghidra_conversion::scripting::ScriptMenuRouteBindings;

namespace {

constexpr std::uint16_t kCommand = 1U;
constexpr std::uint16_t kPosture = 2U;
constexpr std::uint16_t kAttributes = 5U;
constexpr std::uint16_t kCyberware = 6U;
constexpr std::uint16_t kPocketSecretary = 9U;
constexpr std::uint16_t kClips = 4U;
constexpr std::uint16_t kClipTrade = 22U;
constexpr std::uint16_t kReloadConfirm = 23U;
constexpr std::uint16_t kMessage = 19U;
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
    bindings.game.leader_incapacitated = [] { return false; };
    bindings.game.reselect_leader = [&reselect_count] { ++reselect_count; };
    bindings.game.leader_posture = [&posture] { return posture; };
    bindings.game.set_leader_posture = [&posture](const int value) {
        posture = value;
    };
    bindings.game.leader_archetype = [] { return 3; };
    bindings.game.inventory_item = [](int, int) { return false; };
    bindings.game.party = [&party] { return party; };
    bindings.game.active_slot = [] { return 0; };
    bindings.game.magazine_capacity = [](int code) {
        return code == 2 ? 30 : 0;
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
              legacy_screens.back() == kPocketSecretary,
          "view-only Attributes return fully reinitializes page six without "
          "entering the C++ legacy adapter",
          failures);

    runtime.advance_menu_route(0x2000U); // C: reselect and refresh.
    route = runtime.menu_route_snapshot();
    check(route.page == 6 && reselect_count == 1,
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
