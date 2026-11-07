#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "hotel_menu_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::scripting;
using namespace shadowrun::ghidra_conversion::qt_runtime;

std::optional<std::vector<std::int64_t>> advance_options(
    GameScriptRuntime& scripts, const std::uint16_t events) {
    return scripts.call_integers("hotel", "advance_options",
                                 {static_cast<std::int64_t>(events)});
}

std::optional<std::vector<std::int64_t>> advance_checkout(
    GameScriptRuntime& scripts, const std::uint16_t events) {
    return scripts.call_integers("hotel", "advance_checkout",
                                 {static_cast<std::int64_t>(events)});
}

bool reset_options(GameScriptRuntime& scripts) {
    const auto reset = scripts.call_integers("hotel", "reset_options", {});
    return reset.has_value() && reset->size() == 1U && reset->front() == 0;
}

std::uint16_t event_word(const std::uint16_t mask) {
    std::uint16_t events = 0U;
    if ((mask & 0x01U) != 0U) events |= 0x0100U;
    if ((mask & 0x02U) != 0U) events |= 0x0200U;
    if ((mask & 0x04U) != 0U) events |= 0x0400U;
    if ((mask & 0x08U) != 0U) events |= 0x0800U;
    if ((mask & 0x10U) != 0U) events |= 0x4000U;
    if ((mask & 0x20U) != 0U) events |= 0x1000U;
    if ((mask & 0x40U) != 0U) events |= 0x2000U;
    if ((mask & 0x80U) != 0U) events |= 0x8000U;
    return events;
}

bool check_masks(GameScriptRuntime& scripts) {
    // ROM 0xCF98..0xCFC6 checks Up before Down, then A. C is checked after
    // confirmation at 0xCFEA. B, Left, Right, and Start have no branch.
    for (std::uint8_t initial = 0U; initial < 2U; ++initial) {
        for (std::uint16_t mask = 0U; mask < 256U; ++mask) {
            const bool up = (mask & 0x01U) != 0U;
            const bool down = (mask & 0x02U) != 0U;
            const bool a = (mask & 0x10U) != 0U;
            const bool c = (mask & 0x40U) != 0U;

            if (!reset_options(scripts)) {
                std::cerr << "Hotel Options Lua reset failed\n";
                return false;
            }
            if (initial != 0U) {
                const auto moved = advance_options(scripts, 0x0200U);
                if (!moved.has_value() || moved->size() != 3U ||
                    moved->at(0U) != 1 || moved->at(1U) != 1 ||
                    moved->at(2U) != 1) {
                    std::cerr << "Hotel Options Lua initial row failed\n";
                    return false;
                }
            }
            const auto decision = advance_options(
                scripts, event_word(mask));
            if (!decision.has_value() || decision->size() != 3U) {
                std::cerr << "Hotel Options Lua input call failed: "
                          << scripts.failure() << '\n';
                return false;
            }

            const auto expected_selection = (up || down)
                ? static_cast<std::int64_t>(1U - initial)
                : static_cast<std::int64_t>(initial);
            const auto expected_event = a
                ? (expected_selection == 0 ? 3 : 2)
                : c ? 4 : (up || down) ? 1 : 0;
            if (decision->at(1U) != expected_selection ||
                decision->at(0U) != expected_event ||
                decision->at(2U) != (up || down ? 1 : 0)) {
                std::cerr << "Hotel Options mask failed: row="
                          << static_cast<unsigned>(initial)
                          << " mask=" << static_cast<unsigned>(mask)
                          << '\n';
                return false;
            }
        }
    }
    return true;
}

bool check_status_acknowledgement_masks() {
    const std::array<std::string, 3> names{"Joshua", "David", "Jack"};
    for (std::uint16_t mask = 0U; mask < 256U; ++mask) {
        HotelMenuRuntime runtime;
        const std::array<bool, kHotelPartySlotCount> medical_attention{};
        runtime.open(names, medical_attention);
        if (runtime.phase() != HotelMenuPhase::entity_status) {
            std::cerr << "Hotel status mask setup failed\n";
            return false;
        }

        // ROM 0xF312 acknowledges status from any FFDD22 high-byte edge.
        const auto events = static_cast<std::uint16_t>(mask << 8U);
        const auto result = runtime.advance_controller_events(events);
        const auto expected = mask == 0U ? HotelMenuEvent::waiting
                                         : HotelMenuEvent::status_acknowledged;
        const auto expected_phase = mask == 0U
            ? HotelMenuPhase::entity_status : HotelMenuPhase::options;
        const bool wrong_initial_selection =
            mask != 0U &&
            runtime.option_selection() != HotelOption::karma_screen;
        if (result != expected || runtime.phase() != expected_phase ||
            wrong_initial_selection) {
            std::cerr << "Hotel status acknowledgement mask failed: "
                      << mask << '\n';
            return false;
        }
    }

    HotelMenuRuntime held_runtime;
    const std::array<bool, kHotelPartySlotCount> held_medical_attention{};
    held_runtime.open(names, held_medical_attention);
    if (held_runtime.advance_controller_events(0x00FFU) !=
            HotelMenuEvent::waiting ||
        held_runtime.phase() != HotelMenuPhase::entity_status) {
        std::cerr << "Hotel status treated held buttons as a fresh edge\n";
        return false;
    }
    return true;
}

bool check_checkout_masks(GameScriptRuntime& scripts) {
    // ROM 0xDA6A starts on No. Direction branches at 0xDABE..0xDAEE move the
    // cursor and loop before A/B/C confirmation at 0xDAF0..0xDB10.
    // Left, Right, and Start have no branch.
    for (std::uint8_t initial = 0U; initial < 2U; ++initial) {
        for (std::uint16_t mask = 0U; mask < 256U; ++mask) {
            const bool up = (mask & 0x01U) != 0U;
            const bool down = (mask & 0x02U) != 0U;
            const bool left = (mask & 0x04U) != 0U;
            const bool right = (mask & 0x08U) != 0U;
            const bool a = (mask & 0x10U) != 0U;
            const bool b = (mask & 0x20U) != 0U;
            const bool c = (mask & 0x40U) != 0U;
            const bool start = (mask & 0x80U) != 0U;

            if (!reset_options(scripts)) {
                std::cerr << "Hotel Check Out Lua reset failed\n";
                return false;
            }
            if (initial == 0U) {
                const auto moved = advance_checkout(scripts, 0x0100U);
                if (!moved.has_value() || moved->size() != 3U ||
                    moved->at(0U) != 1 || moved->at(1U) != 0 ||
                    moved->at(2U) != 1) {
                    std::cerr << "Hotel Check Out Lua initial Yes row failed\n";
                    return false;
                }
            }
            const auto expected_selection = (up || down)
                ? static_cast<HotelCheckoutSelection>(1U - initial)
                : static_cast<HotelCheckoutSelection>(initial);
            const bool direction_consumes_sample = up || down;
            const std::int64_t expected_event =
                direction_consumes_sample ? 1
                : (a || b || c)
                    ? expected_selection == HotelCheckoutSelection::yes ? 3 : 2
                    : 0;
            const auto decision = advance_checkout(
                scripts, event_word(mask));
            if (!decision.has_value() || decision->size() != 3U) {
                std::cerr << "Hotel Check Out Lua input call failed: "
                          << scripts.failure() << '\n';
                return false;
            }
            const auto expected_selection_value =
                static_cast<std::int64_t>(expected_selection);
            if (decision->at(0U) != expected_event ||
                decision->at(1U) != expected_selection_value ||
                decision->at(2U) != (direction_consumes_sample ? 1 : 0)) {
                std::cerr << "Hotel Check Out mask failed: row="
                          << static_cast<unsigned>(initial)
                          << " mask=" << static_cast<unsigned>(mask)
                          << " Left=" << left << " Right=" << right
                          << " Start=" << start << '\n';
                return false;
            }

            const std::optional<HotelCheckoutResult> expected_result =
                (!direction_consumes_sample && (a || b || c))
                    ? std::optional<HotelCheckoutResult>{
                          expected_selection == HotelCheckoutSelection::yes
                              ? HotelCheckoutResult::checked_out
                              : HotelCheckoutResult::stay_at_hotel}
                    : std::nullopt;
            if ((expected_event == 0 || expected_event == 1) !=
                    !expected_result.has_value() ||
                (expected_event == 2 &&
                 expected_result != HotelCheckoutResult::stay_at_hotel) ||
                (expected_event == 3 &&
                 expected_result != HotelCheckoutResult::checked_out)) {
                std::cerr << "Hotel Check Out expected event mapping failed\n";
                return false;
            }
        }
    }
    return true;
}

bool check_checkout_edge_latch_sequence(GameScriptRuntime& scripts) {
    HotelMenuRuntime runtime;
    const std::array<std::string, 3> names{"Joshua", "David", "Jack"};
    const std::array<bool, kHotelPartySlotCount> medical_attention{};
    runtime.open(names, medical_attention);
    if (!reset_options(scripts)) {
        std::cerr << "Hotel Lua Options reset failed before checkout path\n";
        return false;
    }
    if (runtime.advance_controller_events(0x0100U) !=
            HotelMenuEvent::status_acknowledged) {
        std::cerr << "Hotel checkout edge-latch status setup failed\n";
        return false;
    }
    const auto down = advance_options(scripts, 0x0200U);
    if (!down.has_value() || down->size() != 3U || down->at(0U) != 1 ||
        down->at(1U) != 1 || down->at(2U) != 1 ||
        runtime.apply_lua_options_result(HotelMenuEvent::selection_moved,
                                         HotelOption::check_out) !=
            HotelMenuEvent::selection_moved) {
        std::cerr << "Hotel Options Lua did not select checkout\n";
        return false;
    }
    const auto confirm = advance_options(scripts, 0x4000U);
    if (!confirm.has_value() || confirm->size() != 3U ||
        confirm->at(0U) != 2 || confirm->at(1U) != 1 ||
        confirm->at(2U) != 0 ||
        runtime.apply_lua_options_result(HotelMenuEvent::checkout_opened,
                                         HotelOption::check_out) !=
            HotelMenuEvent::checkout_opened ||
        runtime.checkout_selection() != HotelCheckoutSelection::no) {
        std::cerr << "Hotel checkout edge-latch setup failed\n";
        return false;
    }

    const auto direction_with_confirm = advance_checkout(scripts, 0x4100U);
    if (!direction_with_confirm.has_value() ||
        direction_with_confirm->size() != 3U ||
        direction_with_confirm->at(0U) != 1 ||
        direction_with_confirm->at(1U) != 0 ||
        direction_with_confirm->at(2U) != 1 ||
        runtime.apply_lua_checkout_result(
            HotelMenuEvent::selection_moved, HotelCheckoutSelection::yes) !=
            HotelMenuEvent::selection_moved ||
        runtime.checkout_selection() != HotelCheckoutSelection::yes ||
        runtime.phase() != HotelMenuPhase::checkout) {
        std::cerr << "Hotel checkout direction+confirm did not defer\n";
        return false;
    }
    const auto held = advance_checkout(scripts, 0x0041U);
    if (!held.has_value() || held->size() != 3U || held->at(0U) != 0 ||
        held->at(1U) != 0 || held->at(2U) != 0 ||
        runtime.apply_lua_checkout_result(HotelMenuEvent::waiting,
                                          HotelCheckoutSelection::yes) !=
            HotelMenuEvent::waiting ||
        runtime.checkout_selection() != HotelCheckoutSelection::yes ||
        runtime.phase() != HotelMenuPhase::checkout) {
        std::cerr << "Hotel checkout treated held buttons as fresh edges\n";
        return false;
    }
    const auto checkout_confirm = advance_checkout(scripts, 0x4000U);
    if (!checkout_confirm.has_value() || checkout_confirm->size() != 3U ||
        checkout_confirm->at(0U) != 3 || checkout_confirm->at(1U) != 0 ||
        checkout_confirm->at(2U) != 0 ||
        runtime.apply_lua_checkout_result(
            HotelMenuEvent::checked_out, HotelCheckoutSelection::yes) !=
            HotelMenuEvent::checked_out ||
        runtime.phase() != HotelMenuPhase::closed) {
        std::cerr << "Hotel checkout did not accept a fresh direction-free A\n";
        return false;
    }

    HotelMenuRuntime stayed_runtime;
    const std::array<bool, kHotelPartySlotCount> stayed_medical_attention{};
    stayed_runtime.open(names, stayed_medical_attention);
    if (stayed_runtime.advance_controller_events(0x0100U) !=
            HotelMenuEvent::status_acknowledged ||
        !reset_options(scripts)) {
        std::cerr << "Hotel checkout decline setup failed\n";
        return false;
    }
    const auto down_to_checkout = advance_options(scripts, 0x0200U);
    const auto open_checkout = advance_options(scripts, 0x4000U);
    if (!down_to_checkout.has_value() || down_to_checkout->size() != 3U ||
        down_to_checkout->at(0U) != 1 ||
        !open_checkout.has_value() || open_checkout->size() != 3U ||
        open_checkout->at(0U) != 2 ||
        stayed_runtime.apply_lua_options_result(
            HotelMenuEvent::checkout_opened, HotelOption::check_out) !=
            HotelMenuEvent::checkout_opened) {
        std::cerr << "Hotel checkout decline did not enter the prompt\n";
        return false;
    }
    const auto default_no = advance_checkout(scripts, 0x4000U);
    if (!default_no.has_value() || default_no->size() != 3U ||
        default_no->at(0U) != 2 || default_no->at(1U) != 1 ||
        stayed_runtime.apply_lua_checkout_result(
            HotelMenuEvent::stayed_at_hotel, HotelCheckoutSelection::no) !=
            HotelMenuEvent::stayed_at_hotel ||
        stayed_runtime.phase() != HotelMenuPhase::options ||
        stayed_runtime.option_selection() != HotelOption::karma_screen) {
        std::cerr << "Hotel Check Out default-No did not return to Options\n";
        return false;
    }
    return true;
}

bool check_dispatch_order(GameScriptRuntime& scripts) {
    // First, B is ignored. Next, Down+C moves to Check Out and invokes the
    // action service. Finally Up+Down+A+C moves once (Up has source priority)
    // and A dispatches the newly selected Karma row before C can run.
    std::deque<HotelOptionsInputFrame> frames{
        {false, false, false, true, false, true, true, true},
        {false, true, false, false, true},
        {true, true, true, true, true, true, true, true},
    };
    std::vector<HotelOption> rendered_cursors;
    std::size_t action_calls = 0U;
    std::size_t karma_calls = 0U;
    std::size_t checkout_calls = 0U;

    HotelOptionsMenuState state{};
    HotelOptionsMenuCallbacks callbacks{};
    if (!reset_options(scripts)) {
        return false;
    }
    callbacks.advance_options_input = [&scripts](
        const HotelOption, const HotelOptionsInputFrame& input) {
        std::uint16_t mask = 0U;
        if (input.move_up) mask |= 0x01U;
        if (input.move_down) mask |= 0x02U;
        if (input.move_left) mask |= 0x04U;
        if (input.move_right) mask |= 0x08U;
        if (input.confirm) mask |= 0x10U;
        if (input.alternate_confirm) mask |= 0x20U;
        if (input.action) mask |= 0x40U;
        if (input.start) mask |= 0x80U;
        const auto decision = advance_options(scripts, event_word(mask));
        if (!decision.has_value() || decision->size() != 3U ||
            decision->at(0U) < 0 || decision->at(0U) > 4 ||
            decision->at(1U) < 0 || decision->at(1U) > 1) {
            return HotelOptionsInputTransition{};
        }
        const auto code = decision->at(0U);
        return HotelOptionsInputTransition{
            static_cast<HotelOption>(decision->at(1U)),
            decision->at(2U) == 1,
            code == 2 || code == 3,
            code == 4};
    };
    callbacks.render_cursor = [&](const HotelOption selection) {
        rendered_cursors.push_back(selection);
    };
    callbacks.sample_controller_input = [&] {
        if (frames.empty()) {
            return HotelOptionsInputFrame{};
        }
        const auto input = frames.front();
        frames.pop_front();
        return input;
    };
    callbacks.handle_mode_action_event = [&] { ++action_calls; };
    callbacks.run_hotel_karma_screen = [&] {
        ++karma_calls;
        return HotelMenuHandlerOutcome::return_to_field;
    };
    callbacks.run_hotel_checkout_prompt = [&] {
        ++checkout_calls;
        return HotelMenuHandlerOutcome::return_to_field;
    };

    const auto result = run_hotel_options_menu(state, callbacks);
    if (result.final_selection != HotelOption::karma_screen ||
        state.menu_result != -1 || action_calls != 1U || karma_calls != 1U ||
        checkout_calls != 0U || !frames.empty() ||
        rendered_cursors != std::vector<HotelOption>{
                                HotelOption::karma_screen,
                                HotelOption::check_out,
                                HotelOption::karma_screen}) {
        std::cerr << "Hotel Options A/C dispatch order failed\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    GameScriptRuntime scripts;
    const auto script_root = std::filesystem::path(
        SHADOWRUN_GAMEPLAY_SCRIPT_ROOT);
    const auto dialogue_text = std::filesystem::path(
        SHADOWRUN_GHIDRA_ASSET_ROOT) / "dialogue" / "script-text.json";
    if (!scripts.load_package(script_root, dialogue_text)) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    if (!check_masks(scripts) || !check_status_acknowledgement_masks() ||
        !check_checkout_masks(scripts) ||
        !check_checkout_edge_latch_sequence(scripts) ||
        !check_dispatch_order(scripts)) {
        return 1;
    }
    return 0;
}
