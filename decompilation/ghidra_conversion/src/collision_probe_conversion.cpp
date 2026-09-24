#include "shadowrun_ghidra/collision_probe_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::int32_t signed_word(const std::uint16_t value) {
    if (value < 0x8000U) {
        return static_cast<std::int32_t>(value);
    }
    return static_cast<std::int32_t>(value) - 0x10000;
}

std::uint16_t add_coordinate(const std::uint16_t base,
                             const std::int16_t offset) {
    return static_cast<std::uint16_t>(
        signed_word(base) + static_cast<std::int32_t>(offset));
}

} // namespace

void build_collision_probe_points(
    CollisionProbeState& state,
    const bool collision_mode_active,
    const std::uint16_t entity_x,
    const std::uint16_t entity_y,
    const std::array<CollisionProbeOffset, 3>& offsets,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y) {
    if (collision_mode_active) {
        return;
    }

    for (std::size_t index = 0; index < state.points.size(); ++index) {
        state.points[index] = CollisionProbePoint{
            add_coordinate(entity_x, offsets[index].x),
            add_coordinate(entity_y, offsets[index].y)};
    }
    state.camera_x = camera_x;
    state.camera_y = camera_y;
}

} // namespace shadowrun::ghidra_conversion
