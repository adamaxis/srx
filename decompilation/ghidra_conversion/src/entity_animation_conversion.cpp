#include "shadowrun_ghidra/entity_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void advance_entity_animation_accumulator(EntityAnimationState& state) {
    const auto accumulated = static_cast<std::int32_t>(state.accumulator) +
                             static_cast<std::int32_t>(state.increment);
    if (static_cast<std::int32_t>(state.period) <= accumulated) {
        state.frame = static_cast<std::uint16_t>(state.frame + 1U);
        state.redraw_requested = true;
        state.accumulator = static_cast<std::int16_t>(
            accumulated - static_cast<std::int32_t>(state.period));
        return;
    }

    state.accumulator = static_cast<std::int16_t>(accumulated);
}

} // namespace shadowrun::ghidra_conversion
