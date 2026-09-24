#include "shadowrun_ghidra/entity_boundary_reposition_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool reposition_entity_at_boundary(
    EntityBoundaryRepositionState& state,
    const EntityBoundaryRepositionInput& input) {
    if ((state.flags & 0x40U) != 0U) {
        return false;
    }

    state.flags = static_cast<std::uint8_t>(state.flags | 0x40U);
    const auto horizontal_boundary = static_cast<std::uint16_t>(
        input.horizontal_origin + 0x0A00U);
    const auto vertical_boundary = static_cast<std::uint16_t>(
        input.vertical_origin + 0x0A00U);
    const auto horizontal_step = input.horizontal_midpoint / 2U <
                                         horizontal_boundary
                                     ? static_cast<std::int16_t>(-0x2580)
                                     : static_cast<std::int16_t>(0x2580);
    const auto vertical_step = input.vertical_midpoint / 2U <
                                       vertical_boundary
                                   ? static_cast<std::int16_t>(-0x2580)
                                   : static_cast<std::int16_t>(0x2580);
    state.x = static_cast<std::uint16_t>(horizontal_boundary + horizontal_step);
    state.y = static_cast<std::uint16_t>(vertical_boundary + vertical_step);
    return true;
}

} // namespace shadowrun::ghidra_conversion
