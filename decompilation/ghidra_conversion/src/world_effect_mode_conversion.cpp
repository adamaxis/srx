#include "shadowrun_ghidra/world_effect_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

// ROM 0x1AA10, the descriptor selected by every negative mode result:
// bytes 0C 06 and longword 0001AA28.
constexpr WorldEffectModeDescriptor kNegativeModeDescriptor{
    0x0CU, 0x06U, 0x0001AA28U};

} // namespace

WorldEffectDescriptorTransitionAction begin_world_effect_descriptor_transition(
    WorldEffectDescriptorTransitionState& state,
    const WorldEffectDescriptorTransitionCallbacks& callbacks) {
    if (state.completion_gate) {
        return WorldEffectDescriptorTransitionAction::blocked_by_completion_gate;
    }

    state.animation_mode = 0U;
    state.transition_active = true;

    if (!state.descriptor_stream_ready) {
        state.descriptor_loading = true;
        state.work.frame_counter = 6U;
        state.work.phase_or_descriptor_index = 0U;
        return WorldEffectDescriptorTransitionAction::loading_started;
    }

    state.descriptor_loading = false;
    const auto token = callbacks.read_behavior_token
                           ? callbacks.read_behavior_token(
                                 state.work.phase_or_descriptor_index)
                           : std::nullopt;
    if (!token.has_value()) {
        return WorldEffectDescriptorTransitionAction::descriptor_unresolved;
    }

    state.behavior_token = token;
    state.behavior_position = 0xD280U;
    state.work.frame_counter = 0U;
    state.work.descriptor_state = 7U;
    if (callbacks.attach_behavior_descriptor) {
        callbacks.attach_behavior_descriptor();
    }
    return WorldEffectDescriptorTransitionAction::descriptor_attached;
}

WorldEffectModeAction initialize_world_effect_mode(
    WorldEffectModeState& state,
    const WorldEffectModeCallbacks& callbacks) {
    state.animation_active = true;
    state.exit_requested = false;

    auto mode = state.requested_mode;
    if (!state.mode_request_pending && callbacks.select_run_hud_mode) {
        mode = callbacks.select_run_hud_mode();
    }
    state.selected_mode = mode;

    if (mode < 0) {
        state.work.descriptor_state = kNegativeModeDescriptor.descriptor_state;
        state.work.descriptor_phase = kNegativeModeDescriptor.descriptor_phase;
        state.work.descriptor_pointer =
            kNegativeModeDescriptor.descriptor_pointer;
    } else {
        if (mode == 0x45) {
            state.animation_active = false;
            state.exit_requested = true;
            return WorldEffectModeAction::exit_requested;
        }
        if (mode == 6 && !state.mode_six_started) {
            state.mode_six_started = true;
            state.mode_six_position = 0x47C0U;
        } else if (mode == 7 && !state.mode_seven_started) {
            state.mode_seven_started = true;
            state.mode_seven_position = 0x4800U;
        }

        const auto descriptor = callbacks.resolve_mode_descriptor
                                    ? callbacks.resolve_mode_descriptor(mode)
                                    : std::nullopt;
        if (!descriptor.has_value()) {
            return WorldEffectModeAction::descriptor_unresolved;
        }
        state.work.descriptor_state = descriptor->descriptor_state;
        state.work.descriptor_phase = descriptor->descriptor_phase;
        state.work.descriptor_pointer = descriptor->descriptor_pointer;
    }

    state.work.frame_counter = 0U;
    state.work.x = 0x0120U;
    state.work.y = 0x00D0U;
    return WorldEffectModeAction::initialized;
}

} // namespace shadowrun::ghidra_conversion
