#include "shadowrun_ghidra/world_effect_activation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

WorldEffectActivationAction advance_world_effect_activation(
    WorldEffectActivationState& state,
    const WorldEffectActivationCallbacks& callbacks) {
    if ((state.control_flags & 0x40U) != 0U) {
        if (state.secondary.active) {
            return WorldEffectActivationAction::idle_secondary_record;
        }
        state.secondary.active = true;
        state.secondary.phase_delay = 0U;
        state.secondary.animation_phase = 0U;
        state.secondary.source_variant = 0U;
        state.secondary.frame_delay = 0x14U;
        state.secondary.lifetime = 0x3CU;
        state.secondary.position_x = 0x1400;
        state.secondary.position_y = 0x1070;
        state.secondary.velocity_x = -8;
        state.secondary.velocity_y = -14;
        return WorldEffectActivationAction::secondary_record_started;
    }

    if (!state.transition_work_active) {
        if (state.setup_marker < 0) {
            state.special.position_x = 0x0134U;
            state.special.behavior_token = 0x0001C27CU;
            state.special.behavior_stage = 0U;
            state.special.position_y = 0x0102U;
            state.special.completion_marker = -1;
            state.special.motion_phase = 0U;
            state.special.frame_countdown = 3U;
            if (callbacks.play_audio) {
                callbacks.play_audio(0x16U);
            }
            return WorldEffectActivationAction::special_effect_started;
        }
        if (state.setup_marker != 0) {
            if (callbacks.initialize_world_effect_motion) {
                callbacks.initialize_world_effect_motion();
            }
            return WorldEffectActivationAction::motion_initialization_requested;
        }
    }

    if (callbacks.spawn_effect_record) {
        callbacks.spawn_effect_record(0x20, -0x28, 0);
    }
    if (state.transition_pending && callbacks.adjust_spawn_record_y) {
        callbacks.adjust_spawn_record_y(-6);
    }
    return WorldEffectActivationAction::spawn_requested;
}

} // namespace shadowrun::ghidra_conversion
