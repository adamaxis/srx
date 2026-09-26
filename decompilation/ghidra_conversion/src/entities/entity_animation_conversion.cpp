#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"

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

EntityAnimationTableSelection select_entity_animation_table_row(
    const std::int8_t entity_variant,
    const std::uint8_t animation_id) {
    const auto zero_based_id = static_cast<std::uint16_t>(animation_id) - 1U;
    return EntityAnimationTableSelection{
        entity_variant < 0 ? EntityAnimationTableBank::Alternate
                           : EntityAnimationTableBank::Standard,
        static_cast<std::uint16_t>(zero_based_id * 0x30U)};
}

} // namespace shadowrun::ghidra_conversion
