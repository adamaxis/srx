#include "shadowrun_ghidra/matrix_program_action_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    MatrixProgramActionState state{};
    state.program_codes[0] = 1;
    state.visual_states[0] = 6U;
    state.selection_values[0] = 0U;
    state.selected_slot = 0U;
    state.overlay_stage = 0x0CU;
    state.input_flags = static_cast<std::uint16_t>(1U << 14U);
    std::vector<std::string> events;
    std::int8_t dispatched_mode = 0;
    const MatrixProgramActionCallbacks callbacks{
        [&](const std::int8_t code, const std::int8_t mode) {
            events.emplace_back("dispatch" + std::to_string(code));
            dispatched_mode = mode;
        },
        [&] { events.emplace_back("reset"); },
        [&](const std::size_t slot, const std::uint8_t value) {
            events.emplace_back("select" + std::to_string(slot) + ":" +
                                std::to_string(value));
        },
        [&] { events.emplace_back("text"); },
        [&] { events.emplace_back("rows"); },
        [&] { events.emplace_back("transition"); },
        [&](const std::uint8_t slot, const std::uint8_t mode) {
            events.emplace_back("seed" + std::to_string(slot) + ":" +
                                std::to_string(mode));
        }};
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::selection_published &&
                state.action_mode == -1 && dispatched_mode == -1 &&
                state.selection_values[0] == 3U &&
                events == std::vector<std::string>{
                                "dispatch1", "reset", "select0:3", "text",
                                "rows"})) {
        return 1;
    }

    state.selection_values[0] = 3U;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::ignored &&
                events == std::vector<std::string>{"rows"})) {
        return 2;
    }
    state.selection_values[0] = 0U;
    state.overlay_stage = 0U;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::ignored &&
                events == std::vector<std::string>{"rows"})) {
        return 3;
    }

    state.overlay_stage = 0x0CU;
    state.pending_action = true;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::pending_action_cleared &&
                !state.pending_action &&
                events == std::vector<std::string>{
                                "dispatch1", "reset", "rows"})) {
        return 4;
    }

    state.pending_action = false;
    state.selection_values[0] = 7U;
    state.runner_record_field = 0x10U;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::ignored &&
                events == std::vector<std::string>{
                                "dispatch1", "reset", "rows"})) {
        return 5;
    }

    state.selection_values[0] = 7U;
    state.program_codes[0] = 9;
    state.runner_record_field = 0x12U;
    state.availability_flags[9] = 1U;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::started_runner_action &&
                state.hud_mode_variant == 1 &&
                state.availability_flags[9] == 0U &&
                events == std::vector<std::string>{
                                "dispatch9", "reset", "transition",
                                "seed0:5"})) {
        return 6;
    }

    state.selection_values[0] = 0U;
    state.program_codes[0] = 11;
    state.runner_record_field = 0U;
    events.clear();
    if (!expect(service_matrix_program_action(state, callbacks) ==
                    MatrixProgramActionEvent::selection_published &&
                state.selection_values[0] == 2U &&
                events == std::vector<std::string>{
                                "dispatch11", "reset", "select0:2", "text",
                                "rows"})) {
        return 7;
    }
    return 0;
}
