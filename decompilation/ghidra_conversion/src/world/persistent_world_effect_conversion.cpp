#include "shadowrun_ghidra/world/persistent_world_effect_conversion.hpp"

namespace shadowrun::ghidra_conversion {

PersistentWorldEffectAction advance_persistent_world_effect(
    PersistentWorldEffectState& state,
    const PersistentWorldEffectCallbacks& callbacks) {
    if (state.record.active == 0U) {
        return PersistentWorldEffectAction::inactive;
    }

    WorldEffectRecordCallbacks record_callbacks{};
    record_callbacks.resolve_easing = callbacks.resolve_easing;
    if (advance_world_effect_record(state.record, record_callbacks) ==
        WorldEffectRecordAction::easing_unresolved) {
        return PersistentWorldEffectAction::record_unresolved;
    }

    const auto sprite_data = callbacks.resolve_sprite_source
                                 ? callbacks.resolve_sprite_source(
                                       state.record.source_variant,
                                       state.record.animation_phase)
                                 : resolve_world_effect_sprite_data(
                                       state.record.source_variant,
                                       state.record.animation_phase);
    if (!sprite_data.has_value()) {
        return PersistentWorldEffectAction::sprite_source_unresolved;
    }

    if ((state.frame_counter & 0x0003U) == 0U) {
        state.record.ring_index = static_cast<std::uint8_t>(
            state.record.ring_index + 1U);
        if (state.record.ring_index >= 6U) {
            state.record.ring_index = 0U;
        }
    }
    auto& slot = state.slots[state.record.ring_index];
    slot.sprite_data = sprite_data;
    slot.x = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.record.position_x) >> 4U);
    slot.y = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.record.position_y) >> 4U);

    if (callbacks.build_sprite) {
        for (std::uint8_t draw = 0; draw < 7U; ++draw) {
            const auto index = static_cast<std::uint8_t>(
                (state.record.ring_index + 1U + draw) % 6U);
            const auto& draw_slot = state.slots[index];
            if (draw_slot.sprite_data.has_value()) {
                callbacks.build_sprite(*draw_slot.sprite_data, draw_slot.x,
                                       draw_slot.y, 0x6000U);
            }
        }
    }

    if (state.record.active == 0U) {
        state.transition_state = 0U;
        if (callbacks.run_menu_selection_variant) {
            callbacks.run_menu_selection_variant();
        }
        return PersistentWorldEffectAction::finalized;
    }
    return PersistentWorldEffectAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
