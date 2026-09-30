#include "shadowrun_ghidra/field/mode_action_loop_conversion.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::int32_t> auxiliary{1};
    fill_mode_auxiliary_sentinel(auxiliary);
    if (!expect(auxiliary.size() == 35U && auxiliary.front() == -1)) {
        return 1;
    }
    ModeDisplayStatusState display_status{};
    select_mode_display_status(display_status, 0);
    if (!expect(display_status.status_code == 0xCCU && display_status.dirty_flags == 1U)) {
        return 2;
    }
    select_mode_display_status(display_status, -1);
    if (!expect(display_status.status_code == 0xC0U && display_status.dirty_flags == 1U)) {
        return 3;
    }
    select_mode_display_status(display_status, 1);
    if (!expect(display_status.status_code == 0x0CU && display_status.dirty_flags == 1U)) {
        return 4;
    }
    int repeated_grid_calls = 0;
    write_mode_repeated_grid_service([&] { ++repeated_grid_calls; });
    if (!expect(repeated_grid_calls == 1)) {
        return 5;
    }
    std::vector<std::string> summary_order;
    render_mode_status_summary(
        [&] { summary_order.emplace_back("text"); },
        [&] { summary_order.emplace_back("decimal"); });
    if (!expect(summary_order ==
                std::vector<std::string>{"text", "text", "decimal"})) {
        return 6;
    }

    std::vector<std::string> order;
    std::vector<std::uint16_t> controls;
    std::vector<std::pair<std::int8_t, std::int8_t>> dispatched;
    std::vector<std::pair<std::int8_t, std::int8_t>> cursor_moves;
    std::vector<std::size_t> reset_slots;
    std::vector<std::uint16_t> inputs{0x0100U, 0x4000U, 0x8000U};
    std::size_t input_index = 0;
    int outer_redraws = 0;
    ModeActionState state{};

    const auto result = run_mode_action_loop(
        state,
        ModeActionRecord{3, 2, 0x08U, false, 2},
        {
            [&](std::uint16_t value) { controls.push_back(value); },
            [&] { order.emplace_back("reset"); },
            [&] { order.emplace_back("resource"); },
            [&] { order.emplace_back("clear"); },
            [&] { order.emplace_back("block"); },
            [&] { order.emplace_back("grid"); },
            [&] { order.emplace_back("status"); },
            [&] { order.emplace_back("metadata"); },
            [&](ModeActionState& current) {
                ++outer_redraws;
                current.display_result = 0;
                return false;
            },
            [&] { order.emplace_back("text"); },
            [&] { order.emplace_back("summary"); },
            [&](const std::int8_t direction, const std::int8_t cursor) {
                order.emplace_back("cursor");
                cursor_moves.emplace_back(direction, cursor);
            },
            [&] { order.emplace_back("vblank"); },
            [&] { order.emplace_back("latch"); },
            [&] { return inputs[input_index++]; },
            [&] { order.emplace_back("action"); },
            [&] { return ModeActionGateInputs{4, 2}; },
            [&] { order.emplace_back("repeat"); },
            [&] { order.emplace_back("wait"); },
            [&](std::int8_t kind, std::int8_t cursor) {
                dispatched.emplace_back(kind, cursor);
            },
            [&] { order.emplace_back("footer"); },
            [&] { order.emplace_back("upload"); },
            [&] {
                order.emplace_back("matrix");
                return true;
            },
            [&](std::size_t slot) { reset_slots.push_back(slot); },
        });

    if (!expect(result && state.display_result == 0 && state.hud_abort < 0 &&
                state.mode_flags == 0x48U && state.mode_random_count == 1U &&
                outer_redraws == 3 &&
                controls == std::vector<std::uint16_t>({0x8B00U, 0x8134U, 0x8174U,
                                                        0x8134U, 0x8174U, 0x8134U,
                                                        0x8174U, 0x8134U, 0x8174U}) &&
                dispatched == std::vector<std::pair<std::int8_t, std::int8_t>>{{3, 2}} &&
                cursor_moves ==
                    std::vector<std::pair<std::int8_t, std::int8_t>>{{0, 0}, {-1, 2},
                                                                       {0, 0}} &&
                order.size() == 67U &&
                reset_slots == std::vector<std::size_t>({0, 1, 2, 3, 4}))) {
        return 7;
    }

    ModeActionState action_state{};
    reset_slots.clear();
    std::vector<std::string> action_order;
    std::vector<std::uint16_t> action_inputs{0x2000U, 0x1000U, 0x8000U};
    std::size_t action_input_index = 0;
    const auto action_result = run_mode_action_loop(
        action_state,
        ModeActionRecord{7, 5, 0x02U, false, 4},
        {
            {}, {}, {}, {}, {}, {}, {}, {},
            [&](ModeActionState& current) {
                current.display_result = 0;
                return false;
            },
            {}, {}, {}, {}, {},
            [&] { return action_inputs[action_input_index++]; },
            [&] { action_order.emplace_back("action"); },
            [&] { return ModeActionGateInputs{9, 3}; },
            [&] { action_order.emplace_back("repeat"); },
            [&] { action_order.emplace_back("wait"); },
            [&](std::int8_t kind, std::int8_t cursor) {
                action_order.emplace_back("dispatch");
                if (kind != 7 || cursor != 0) {
                    action_order.emplace_back("bad-dispatch");
                }
            },
            [&] { action_order.emplace_back("footer"); },
            [&] { action_order.emplace_back("upload"); },
            [&] {
                action_order.emplace_back("matrix");
                return true;
            },
            [&](std::size_t slot) { reset_slots.push_back(slot); },
        });
    if (!expect(action_result && action_order ==
                std::vector<std::string>{"action", "dispatch", "action", "dispatch",
                                         "footer", "upload", "matrix"} &&
                reset_slots == std::vector<std::size_t>({0, 1, 2, 3, 4}))) {
        return 8;
    }

    if (!expect(static_cast<std::uint16_t>(ModeActionInput::confirm) == 0x4000U &&
                static_cast<std::uint16_t>(ModeActionInput::alternate_confirm) ==
                    0x1000U &&
                static_cast<std::uint16_t>(ModeActionInput::mode_action) ==
                    0x2000U)) {
        return 9;
    }

    ModeActionState random_state{};
    random_state.mode_random_count = 3;
    random_state.hud_abort = 0;
    std::vector<std::string> random_order;
    const bool random_fired = advance_random_mode_event(
        random_state,
        {
            [] { return std::int8_t{1}; },
            [&] { random_order.emplace_back("transition"); },
            [&] { random_order.emplace_back("status"); },
            [&] { random_order.emplace_back("text"); },
            [&] { random_order.emplace_back("vblank"); },
            [&] { random_order.emplace_back("wait"); },
            [&] { random_order.emplace_back("grid"); },
        });
    if (!expect(random_fired && random_state.mode_random_count == 0 &&
                random_order == std::vector<std::string>{"transition", "status", "text",
                                                         "vblank", "wait", "grid"})) {
        return 10;
    }

    return 0;
}
