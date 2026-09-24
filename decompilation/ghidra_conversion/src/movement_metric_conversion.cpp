#include "shadowrun_ghidra/movement_metric_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

std::uint32_t compute_diagonal_step_cost(
    const std::int32_t delta_x,
    const std::int32_t delta_y) {
    const auto x = static_cast<std::uint32_t>(
        delta_x < 0 ? -static_cast<std::int64_t>(delta_x) : delta_x);
    const auto y = static_cast<std::uint32_t>(
        delta_y < 0 ? -static_cast<std::int64_t>(delta_y) : delta_y);
    const auto major = std::max(x, y);
    const auto minor = std::min(x, y);
    return major + ((minor + 1U) / 2U);
}

} // namespace shadowrun::ghidra_conversion
