#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void run_mode_specific_cleanup(
    const FieldModeExitTransitionCallbacks& callbacks,
    const ExitEffectSlotGroup group,
    FieldModeExitTransitionResult& result) {
    result.cleared_effect_group = group;
    result.mode_specific_cleanup_ran = true;
    if (callbacks.clear_high_effect_slots) {
        callbacks.clear_high_effect_slots(group);
    }
    if (callbacks.reset_world_effect_globals) {
        callbacks.reset_world_effect_globals();
    }
    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(
            FieldModeExitServicePhase::mode_specific_cleanup);
    }
}

} // namespace

FieldModeExitTransitionResult finalize_field_mode_exit_transition(
    FieldModeExitTransitionState& state,
    const FieldModeExitTransitionCallbacks& callbacks) {
    FieldModeExitTransitionResult result{};

    state.effect_transition_marker = 0U;
    state.effect_animation_marker = 0U;
    state.mode_prompt_pending = 0U;

    const bool transition_started = (state.transition_flags & 0x01U) != 0U;
    switch (state.exit_mode) {
    case 5U:
        // ROM 0xC66C rejects a mode-5 transition when the source effect flag
        // is clear or its high-effect bit is already set.
        if (transition_started &&
            (state.transition_flags & 0x80U) == 0U) {
            state.mode_five_effect_marker = 0U;
            run_mode_specific_cleanup(callbacks,
                                      ExitEffectSlotGroup::mode_five, result);
        }
        break;
    case 1U:
        if (transition_started) {
            run_mode_specific_cleanup(callbacks,
                                      ExitEffectSlotGroup::mode_one, result);
        }
        break;
    case 2U:
    case 4U:
        if (transition_started) {
            state.effect_service_flags &= ~(1UL << 9U);
            run_mode_specific_cleanup(callbacks,
                                      ExitEffectSlotGroup::mode_two_or_four,
                                      result);
        }
        break;
    default:
        break;
    }

    state.primary_runner_transition = 0U;
    state.secondary_runner_transition = 0U;
    if (callbacks.refresh_field_runner_state_and_assets) {
        callbacks.refresh_field_runner_state_and_assets();
    }

    // FF0110: clear bits 6 and 4, then assert the active bit. FF0111 clears
    // its bit 6 independently.
    state.runner_flags = static_cast<std::uint8_t>(
        (state.runner_flags & static_cast<std::uint8_t>(~0x50U)) | 0x01U);
    state.recruited_runner_flags = static_cast<std::uint8_t>(
        state.recruited_runner_flags & static_cast<std::uint8_t>(~0x40U));
    state.field_anchor = 0xB45CU;
    state.action_counter = std::max<std::uint8_t>(state.action_counter, 6U);
    state.resource_counter = std::max<std::uint8_t>(state.resource_counter, 8U);

    const auto resource_payment = std::min<std::uint32_t>(
        state.resource_pool >> 3U, 5000U);
    state.resource_pool -= resource_payment;

    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(
            FieldModeExitServicePhase::common_state_refresh);
    }
    if (callbacks.run_entity_animation_triplet) {
        callbacks.run_entity_animation_triplet();
    }
    if (callbacks.handle_mode_action_event) {
        callbacks.handle_mode_action_event();
    }

    if ((state.world_mode_flags & 0x01U) != 0U) {
        result.initialized_profile = FieldModeProfile::profile_a;
        if (callbacks.initialize_field_mode_profile_a) {
            callbacks.initialize_field_mode_profile_a();
        }
    } else {
        result.initialized_profile = FieldModeProfile::profile_b;
        if (callbacks.initialize_field_mode_profile_b) {
            callbacks.initialize_field_mode_profile_b();
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
