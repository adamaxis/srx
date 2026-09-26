#include "shadowrun_ghidra/cyberspace/matrix_program_timing_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {

MatrixProgramLoadTiming calculate_matrix_program_load_steps(
    const std::uint32_t program_memory,
    const std::uint8_t load_io_divisor) {
    if (load_io_divisor == 0U) {
        return {};
    }

    // 0x195B4 copies D1.w into D0.w before DIVU.W.  Do not let a wider host
    // value change the source's selected-program word.
    const auto source_memory = static_cast<std::uint16_t>(program_memory);
    auto steps = static_cast<std::uint16_t>(
        source_memory / static_cast<std::uint16_t>(load_io_divisor));
    if ((source_memory % load_io_divisor) != 0U) {
        ++steps;
    }
    // The ROM uses BMI after CMP.W #7, so this is a wrapped sign-bit test of
    // (steps - 7), not an unsigned `steps >= 7` comparison.
    if ((static_cast<std::uint16_t>(steps - 7U) & 0x8000U) == 0U) {
        steps = 6U;
    }
    return {true, steps};
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
