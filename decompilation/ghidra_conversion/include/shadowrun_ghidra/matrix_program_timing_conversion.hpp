#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MatrixProgramLoadTiming {
    bool valid = false;
    std::uint8_t steps = 0;
};

// ROM 0x195A6: divide the selected program footprint by the deck's load/IO
// divisor, round up when there is a remainder, and cap the result at six.
MatrixProgramLoadTiming calculate_matrix_program_load_steps(
    std::uint32_t program_memory, std::uint8_t load_io_divisor);

struct MatrixProgramSlotVisualState {
    std::array<std::int8_t, 5> program_codes{};
    std::array<std::uint8_t, 5> pending_values{};
    std::array<std::int16_t, 5> load_offsets{};
};

struct MatrixProgramSlotRefreshCallbacks {
    std::function<void()> write_grid;
    std::function<void()> render_rows;
};

// ROM 0x195D8: publish the selected program in its five-slot record, clear
// its pending value, store the ten-times load duration, and refresh the two
// presentation boundaries.
bool refresh_matrix_program_slot(
    MatrixProgramSlotVisualState& state,
    std::size_t slot,
    std::uint8_t program_index,
    std::uint32_t program_memory,
    std::uint8_t load_io_divisor,
    const MatrixProgramSlotRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
