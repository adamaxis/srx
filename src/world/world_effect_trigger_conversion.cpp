#include "shadowrun_ghidra/world/world_effect_trigger_conversion.hpp"

namespace shadowrun::ghidra_conversion {

WorldEffectTriggerAction trigger_world_effect_sequence(
    const WorldEffectTriggerState& state,
    const WorldEffectTriggerCallbacks& callbacks) {
    if (state.trigger_delay_active || state.current_entity_protected ||
        state.activity_lock ||
        (!state.sequence_mode && state.world_effects_active) ||
        state.placement_pending) {
        return WorldEffectTriggerAction::blocked;
    }
    if ((state.frame_counter & 0x03FFU) != 10U) {
        return WorldEffectTriggerAction::not_due;
    }
    if (!callbacks.sample_random) {
        return WorldEffectTriggerAction::random_miss;
    }
    const auto [random_value, random_bound] = callbacks.sample_random();
    if (random_value >= random_bound) {
        return WorldEffectTriggerAction::random_miss;
    }

    if (state.sequence_mode) {
        if (callbacks.forward_effect_sequence) {
            callbacks.forward_effect_sequence();
        }
        return WorldEffectTriggerAction::forwarded;
    }

    const auto selected = callbacks.select_world_effect_sequence
                              ? callbacks.select_world_effect_sequence()
                              : 0;
    if (selected != 0) {
        if (callbacks.begin_world_activity_if_idle) {
            callbacks.begin_world_activity_if_idle();
        }
        return WorldEffectTriggerAction::activity_started;
    }
    if (callbacks.spawn_world_effect_sequence) {
        callbacks.spawn_world_effect_sequence();
    }
    return WorldEffectTriggerAction::sequence_spawned;
}

} // namespace shadowrun::ghidra_conversion
