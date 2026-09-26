#include "shadowrun_ghidra/cyberspace/matrix_program_timing_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {

MatrixProgramLoadTiming calculate_matrix_program_load_steps(
    const std::uint32_t program_memory,
    const std::uint8_t load_io_divisor) {
    if (load_io_divisor == 0U) {
        return {};
    }

    auto steps = program_memory / load_io_divisor;
    if ((program_memory % load_io_divisor) != 0U) {
        ++steps;
    }
    if (steps >= 7U) {
        steps = 6U;
    }
    return {true, static_cast<std::uint8_t>(steps)};
}

MatrixProgramLoadTiming calculate_selected_matrix_program_load_steps(
    const std::uint8_t program_index,
    const std::array<std::uint32_t, 12>& program_memory,
    const std::uint8_t load_io_divisor) {
    if (program_index >= program_memory.size()) {
        return {};
    }
    return calculate_matrix_program_load_steps(program_memory[program_index],
                                               load_io_divisor);
}

bool refresh_matrix_program_slot(
    MatrixProgramSlotVisualState& state, const std::size_t slot,
    const std::uint8_t program_index, const std::uint32_t program_memory,
    const std::uint8_t load_io_divisor,
    const MatrixProgramSlotRefreshCallbacks& callbacks) {
    if (slot >= state.program_codes.size()) {
        return false;
    }
    const auto timing = calculate_matrix_program_load_steps(
        program_memory, load_io_divisor);
    if (!timing.valid) {
        return false;
    }

    state.program_codes[slot] = static_cast<std::int8_t>(program_index);
    state.pending_values[slot] = 0U;
    state.load_offsets[slot] =
        static_cast<std::int16_t>(timing.steps * 10U);
    if (callbacks.write_grid) {
        callbacks.write_grid();
    }
    if (callbacks.render_rows) {
        callbacks.render_rows();
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion
