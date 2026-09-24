#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct RelativeHeadingMeasurement {
    std::uint16_t heading = 0;
    std::uint32_t distance = 0;
};

// ROM 0x15CA4 and the record-based variant at 0x15CC0: measure two-axis
// movement with an orthogonal step cost of one and a diagonal contribution of
// one half, rounded up. This is the source's max-axis plus rounded-minimum
// metric, expressed directly in host coordinates rather than register pairs.
std::uint32_t compute_diagonal_step_cost(
    std::int32_t delta_x,
    std::int32_t delta_y);

// ROM 0x15D4E: derive the clockwise fixed-point heading and the source's
// half-major-axis distance from a signed delta pair. The original's
// 1024-entry arctangent correction table is represented as a typed host
// constant rather than as a ROM lookup.
RelativeHeadingMeasurement measure_heading_and_distance(
    std::int32_t delta_x,
    std::int32_t delta_y);

// ROM 0x15D3C: subtract two world positions, then apply 0x15D4E.
RelativeHeadingMeasurement measure_relative_heading_and_distance(
    std::int32_t origin_x,
    std::int32_t origin_y,
    std::int32_t target_x,
    std::int32_t target_y);

} // namespace shadowrun::ghidra_conversion
