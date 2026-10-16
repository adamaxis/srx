#include "shadowrun_ghidra/entities/movement_direction_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

DominantMovementDirections choose_dominant_axis_directions(
    const std::int32_t origin_x,
    const std::int32_t origin_y,
    const std::int32_t target_x,
    const std::int32_t target_y) {
    const auto delta_x = origin_x - target_x;
    const auto delta_y = origin_y - target_y;
    const auto horizontal = static_cast<std::int8_t>(delta_x < 0 ? 2 : 6);
    const auto vertical = static_cast<std::int8_t>(delta_y < 0 ? 4 : 0);
    const auto distance_x = static_cast<std::uint32_t>(
        delta_x < 0 ? -static_cast<std::int64_t>(delta_x) : delta_x);
    const auto distance_y = static_cast<std::uint32_t>(
        delta_y < 0 ? -static_cast<std::int64_t>(delta_y) : delta_y);

    if (distance_y > distance_x) {
        return DominantMovementDirections{vertical, horizontal};
    }
    return DominantMovementDirections{horizontal, vertical};
}

} // namespace shadowrun::ghidra_conversion
