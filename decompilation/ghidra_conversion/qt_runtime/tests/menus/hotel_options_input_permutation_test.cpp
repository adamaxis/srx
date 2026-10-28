#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"
#include "hotel_menu_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

bool check_masks() {
    // ROM 0xCF98..0xCFC6 checks Up before Down, then bit 0x0E (A).
    // The B bit (0x0C) is not checked in this loop; C (0x0D) is checked only
    // afterward at 0xCFEA as the mode-action event. Left (0x0A), Right
    // (0x0B), and Start (0x0F) have no branch in this menu loop.
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

            HotelOptionsMenuState state{};
            state.selection = static_cast<HotelOption>(initial);
            const auto transition = apply_hotel_options_input(
                state, HotelOptionsInputFrame{up, down, a, b, c,
                                              left, right, start});

            const auto expected_selection = (up || down)
                ? static_cast<HotelOption>(1U - initial)
                : static_cast<HotelOption>(initial);
            if (state.selection != expected_selection ||
                transition.selection_moved != (up || down) ||
                transition.confirmed != a || transition.action != c) {
                std::cerr << "Hotel Options mask failed: row="
                          << static_cast<unsigned>(initial)
                          << " mask=" << static_cast<unsigned>(mask)
                          << " B=" << b << " Left=" << left
                          << " Right=" << right << " Start=" << start
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
        std::array<RunnerRecordFields, 3> records{};
        runtime.open(records, names, 0U);
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
    std::array<RunnerRecordFields, 3> held_records{};
    held_runtime.open(held_records, names, 0U);
    if (held_runtime.advance_controller_events(0x00FFU) !=
            HotelMenuEvent::waiting ||
        held_runtime.phase() != HotelMenuPhase::entity_status) {
        std::cerr << "Hotel status treated held buttons as a fresh edge\n";
        return false;
    }
    return true;
}

bool check_checkout_masks() {
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

            HotelCheckoutPromptState state{};
            state.selection = static_cast<HotelCheckoutSelection>(initial);
            const auto expected_selection = (up || down)
                ? static_cast<HotelCheckoutSelection>(1U - initial)
                : static_cast<HotelCheckoutSelection>(initial);
            const bool direction_consumes_sample = up || down;
            const std::optional<HotelCheckoutResult> expected_result =
                (!direction_consumes_sample && (a || b || c))
                    ? std::optional<HotelCheckoutResult>{
                          expected_selection == HotelCheckoutSelection::yes
                              ? HotelCheckoutResult::checked_out
                              : HotelCheckoutResult::stay_at_hotel}
                    : std::nullopt;

            const auto result = apply_hotel_checkout_input(
                state, HotelOptionsInputFrame{up, down, a, b, c,
                                              left, right, start});
            if (state.selection != expected_selection ||
                result != expected_result) {
                std::cerr << "Hotel Check Out mask failed: row="
                          << static_cast<unsigned>(initial)
                          << " mask=" << static_cast<unsigned>(mask)
                          << " Left=" << left << " Right=" << right
                          << " Start=" << start << '\n';
                return false;
            }
        }
    }
    return true;
}

bool check_checkout_edge_latch_sequence() {
    HotelMenuRuntime runtime;
    std::array<RunnerRecordFields, 3> records{};
    const std::array<std::string, 3> names{"Joshua", "David", "Jack"};
    runtime.open(records, names, 0U);
    if (runtime.advance_controller_events(0x0100U) !=
            HotelMenuEvent::status_acknowledged ||
        runtime.advance_controller_events(0x0200U) !=
            HotelMenuEvent::selection_moved ||
        runtime.advance_controller_events(0x4000U) !=
            HotelMenuEvent::checkout_opened ||
        runtime.checkout_selection() != HotelCheckoutSelection::no) {
        std::cerr << "Hotel checkout edge-latch setup failed\n";
        return false;
    }

    if (runtime.advance_controller_events(0x4100U) !=
            HotelMenuEvent::selection_moved ||
        runtime.checkout_selection() != HotelCheckoutSelection::yes ||
        runtime.phase() != HotelMenuPhase::checkout) {
        std::cerr << "Hotel checkout direction+confirm did not defer\n";
        return false;
    }
    if (runtime.advance_controller_events(0x0041U) != HotelMenuEvent::waiting ||
        runtime.checkout_selection() != HotelCheckoutSelection::yes ||
        runtime.phase() != HotelMenuPhase::checkout) {
        std::cerr << "Hotel checkout treated held buttons as fresh edges\n";
        return false;
    }
    if (runtime.advance_controller_events(0x4000U) !=
            HotelMenuEvent::checked_out ||
        runtime.phase() != HotelMenuPhase::closed) {
        std::cerr << "Hotel checkout did not accept a fresh direction-free A\n";
        return false;
    }
    return true;
}

bool check_dispatch_order() {
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
    if (!check_masks() || !check_status_acknowledgement_masks() ||
        !check_checkout_masks() || !check_checkout_edge_latch_sequence() ||
        !check_dispatch_order()) {
        return 1;
    }
    return 0;
}
