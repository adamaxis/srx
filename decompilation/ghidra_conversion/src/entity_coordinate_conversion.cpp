#include "shadowrun_ghidra/entity_coordinate_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void approach_entity_fixed_point_target(EntityCoordinateTarget& state) {
    if (state.current == state.target) {
        return;
    }

    const auto delta = static_cast<std::uint16_t>(state.current - state.target);
    if (static_cast<std::int16_t>(delta) < 0) {
        if (delta < 0xF800U) {
            state.current = static_cast<std::uint16_t>(state.current + 0x800U);
            return;
        }
    } else if (delta > 0x7FFU) {
        state.current = static_cast<std::uint16_t>(state.current - 0x800U);
        return;
    }

    state.current = state.target;
}

void apply_mode_five_boundary_motion(FieldBoundaryMotionState& state,
                                     const std::uint8_t mode) {
    if (mode != 5U || state.y <= 0x11FFU || state.y >= 0x1F80U ||
        state.x <= 0x25FFU) {
        return;
    }

    if (state.x < 0x2800U) {
        state.y = static_cast<std::uint16_t>(state.y - 8U);
        return;
    }
    if (state.x > 0x3BFFU && state.x < 0x3E00U) {
        state.y = static_cast<std::uint16_t>(state.y + 8U);
    }
}

EntityWorldProbeResult probe_entity_world_position(
    const EntityFixedPointPosition& position,
    const EntityWorldProbe& probe) {
    const auto world_x = position.world_x +
        (static_cast<std::int32_t>(position.tile_offset_x) * 0x100);
    const auto world_y = position.world_y +
        (static_cast<std::int32_t>(position.tile_offset_y) * 0x100);
    return EntityWorldProbeResult{world_x, world_y, probe(world_x, world_y)};
}

} // namespace shadowrun::ghidra_conversion
