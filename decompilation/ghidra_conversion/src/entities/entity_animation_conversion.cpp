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

EntityMotionActionRowSelection select_entity_motion_action_row(
    const std::int8_t active_action_state,
    const std::uint8_t one_based_action_code) {
    const auto zero_based_code =
        static_cast<std::uint16_t>(one_based_action_code) - 1U;
    return EntityMotionActionRowSelection{
        active_action_state < 0 ? EntityMotionActionTable::MagicSpell
                                : EntityMotionActionTable::EquipmentItem,
        one_based_action_code,
        static_cast<std::uint16_t>(zero_based_code * 0x30U)};
}

} // namespace shadowrun::ghidra_conversion
