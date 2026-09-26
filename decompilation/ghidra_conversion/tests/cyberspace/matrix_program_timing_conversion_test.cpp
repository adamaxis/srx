#include "shadowrun_ghidra/cyberspace/matrix_program_timing_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto exact = calculate_matrix_program_load_steps(12U, 3U);
    const auto rounded = calculate_matrix_program_load_steps(13U, 3U);
    const auto capped = calculate_matrix_program_load_steps(100U, 1U);
    const auto invalid = calculate_matrix_program_load_steps(5U, 0U);
    std::array<std::uint32_t, 12> program_memory{};
    program_memory[4] = 13U;
    const auto selected = calculate_selected_matrix_program_load_steps(
        4U, program_memory, 3U);
    const auto selected_invalid = calculate_selected_matrix_program_load_steps(
        12U, program_memory, 3U);
    if (!expect(exact.valid && exact.steps == 4U && rounded.valid &&
                rounded.steps == 5U && capped.valid && capped.steps == 6U &&
                !invalid.valid && selected.valid && selected.steps == 5U &&
                !selected_invalid.valid)) {
        return 1;
    }

    MatrixProgramSlotVisualState state{};
    state.program_codes.fill(-1);
    std::vector<int> events;
    if (!expect(refresh_matrix_program_slot(
                    state, 2U, 7U, 13U, 3U,
                    {[&] { events.push_back(1); },
                     [&] { events.push_back(2); }}) &&
                state.program_codes[2] == 7 &&
                state.pending_values[2] == 0U &&
                state.load_offsets[2] == 50 &&
                events == std::vector<int>{1, 2})) {
        return 2;
    }
    if (!expect(!refresh_matrix_program_slot(state, 5U, 1U, 1U, 1U, {}) &&
                !refresh_matrix_program_slot(state, 0U, 1U, 1U, 0U, {}))) {
        return 3;
    }
    return 0;
}
