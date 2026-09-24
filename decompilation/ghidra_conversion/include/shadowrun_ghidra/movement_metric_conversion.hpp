#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x15CA4 and the record-based variant at 0x15CC0: measure two-axis
// movement with an orthogonal step cost of one and a diagonal contribution of
// one half, rounded up. This is the source's max-axis plus rounded-minimum
// metric, expressed directly in host coordinates rather than register pairs.
std::uint32_t compute_diagonal_step_cost(
    std::int32_t delta_x,
    std::int32_t delta_y);

} // namespace shadowrun::ghidra_conversion
