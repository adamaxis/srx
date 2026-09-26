#include "shadowrun_ghidra/entities/entity_close_range_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool apply_close_range_entity_state(
    EntityCloseRangeState& state,
    const std::uint16_t distance) {
    if (distance >= 0x01E0U) {
        return false;
    }

    state.flags = static_cast<std::uint8_t>(state.flags & 0x80U);
    state.secondary_state = 0U;
    state.flags = static_cast<std::uint8_t>(state.flags | 0x04U);
    state.continuation = 0xB45CU;
    return true;
}

void apply_entity_close_range_state(
    const std::function<void()>& apply_state) {
    if (apply_state) {
        apply_state();
    }
}

} // namespace shadowrun::ghidra_conversion
