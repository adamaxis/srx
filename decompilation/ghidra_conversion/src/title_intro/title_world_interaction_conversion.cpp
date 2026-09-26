#include "shadowrun_ghidra/title_intro/title_world_interaction_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleWorldInteractionAction advance_title_world_interaction_service(
    TitleWorldInteractionState& state,
    const TitleWorldInteractionCallbacks& callbacks) {
    if (state.scene_complete) {
        if (!callbacks.initialize_behavior_record ||
            !callbacks.install_behavior_pair) {
            return TitleWorldInteractionAction::dependency_unavailable;
        }
        callbacks.initialize_behavior_record();
        callbacks.install_behavior_pair(
            TitleWorldBehaviorPair{0x001C2A8U, 0x001C2AEU});
        return TitleWorldInteractionAction::completion_behavior_installed;
    }

    if ((state.title_service_flags & 0x01U) != 0U) {
        return TitleWorldInteractionAction::title_sweep_blocked;
    }

    // The primary descriptor is checked first. When it is in its terminal
    // stage (5), the source intentionally falls through to the input gate;
    // all other primary states return immediately.
    if (state.primary_behavior_descriptor == 0x001C36AU) {
        if (state.primary_stage == 3U && state.primary_phase == 3U) {
            if (!callbacks.advance_world_effect_activation) {
                return TitleWorldInteractionAction::dependency_unavailable;
            }
            callbacks.advance_world_effect_activation();
            return TitleWorldInteractionAction::world_effect_activation_advanced;
        }
        if (state.primary_stage != 5U) {
            return TitleWorldInteractionAction::primary_behavior_waiting;
        }
    } else if (state.secondary_behavior_descriptor == 0x001C350U) {
        if (state.secondary_stage == 3U && state.secondary_phase == 3U) {
            if (!callbacks.update_interaction_state) {
                return TitleWorldInteractionAction::dependency_unavailable;
            }
            callbacks.update_interaction_state();
            return TitleWorldInteractionAction::interaction_state_advanced;
        }
        if (state.secondary_stage != 5U) {
            return TitleWorldInteractionAction::secondary_behavior_waiting;
        }
    }

    const auto input_gate = static_cast<std::uint8_t>(
        state.controller_events & static_cast<std::uint8_t>(0x40U | 0x10U | 0x20U));
    if (state.world_effect_mode >= 3U || input_gate == 0U ||
        state.descriptor_transition_active) {
        return TitleWorldInteractionAction::no_action;
    }
    if (!callbacks.service_matrix_program_action) {
        return TitleWorldInteractionAction::dependency_unavailable;
    }
    callbacks.service_matrix_program_action();
    return TitleWorldInteractionAction::matrix_program_action_serviced;
}

} // namespace shadowrun::ghidra_conversion
