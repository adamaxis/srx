#include "shadowrun_ghidra/field/field_mode_finalizer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

FieldModeFinalizerMessageVariant signed_message_variant(
    const std::int8_t alternate_result) {
    if (alternate_result > 0) {
        return FieldModeFinalizerMessageVariant::positive_result_prompt;
    }
    if (alternate_result < 0) {
        return FieldModeFinalizerMessageVariant::negative_result_prompt;
    }
    return FieldModeFinalizerMessageVariant::default_prompt;
}

bool matches(const FieldModeFinalizerState& state, const std::uint8_t scene,
             const std::uint8_t submode) {
    return state.scene_id == scene && state.submode == submode;
}

} // namespace

FieldModeFinalizerAction finalize_field_mode_exit(
    FieldModeFinalizerState& state,
    const FieldModeFinalizerCallbacks& callbacks) {
    FieldModeFinalizerSelection selection{};

    // These three routes have source-specific flag behavior and return before
    // the message/status cleanup shared by the table below.
    if (matches(state, 6U, 0U)) {
        state.mode_flags &= ~(1UL << 9U);
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_six_submode_zero;
        if (callbacks.run_mode_exit_services) {
            callbacks.run_mode_exit_services(selection, state);
        }
        return FieldModeFinalizerAction::handled;
    }
    if (matches(state, 7U, 0U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_seven_submode_zero;
        if ((state.world_flags & (1UL << 24U)) == 0U) {
            state.world_flags |= 1UL << 24U;
            selection.message_variant =
                FieldModeFinalizerMessageVariant::first_visit;
        } else if ((state.world_flags & (1UL << 25U)) == 0U) {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::pending_visit;
        } else {
            if (callbacks.reset_runner_status_triplet) {
                callbacks.reset_runner_status_triplet();
            }
            selection.message_variant =
                FieldModeFinalizerMessageVariant::completed_visit;
        }
        if (callbacks.run_mode_exit_services) {
            callbacks.run_mode_exit_services(selection, state);
        }
        return FieldModeFinalizerAction::handled;
    }
    if (matches(state, 9U, 0U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_nine_submode_zero;
        if ((state.world_flags & (1UL << 18U)) == 0U) {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::first_visit;
        } else if ((state.status_flags & (1UL << 5U)) != 0U) {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::fallback_visit;
        } else if ((state.world_flags & (1UL << 20U)) != 0U) {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::completed_visit;
        } else {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::pending_visit;
        }
        if (callbacks.run_mode_exit_services) {
            callbacks.run_mode_exit_services(selection, state);
        }
        return FieldModeFinalizerAction::handled;
    }

    bool matched = true;
    bool force_negative_status = false;

    if (matches(state, 0U, 1U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_one;
    } else if (matches(state, 0U, 2U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_two;
    } else if (matches(state, 0U, 4U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_four;
    } else if (matches(state, 0U, 5U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_five;
    } else if (matches(state, 0U, 6U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_six;
        force_negative_status = state.alternate_result == 0;
    } else if (matches(state, 0U, 7U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_seven;
    } else if (matches(state, 0U, 8U) || matches(state, 0U, 9U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_zero_submode_eight_or_nine;
        force_negative_status = true;
    } else if (matches(state, 4U, 1U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_four_submode_one;
        force_negative_status = state.alternate_result == 0;
    } else if (matches(state, 4U, 2U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_four_submode_two;
    } else if (matches(state, 4U, 5U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_four_submode_five;
    } else if (matches(state, 3U, 0U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_zero;
    } else if (matches(state, 3U, 2U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_two;
    } else if (matches(state, 3U, 3U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_three;
    } else if (matches(state, 3U, 4U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_four;
    } else if (matches(state, 3U, 5U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_five;
        force_negative_status = true;
    } else if (matches(state, 3U, 6U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_six;
    } else if (matches(state, 3U, 7U) || matches(state, 3U, 8U) ||
               matches(state, 3U, 9U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_three_submode_seven_or_eight_or_nine;
        force_negative_status = true;
    } else if (matches(state, 1U, 2U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_one_submode_two;
    } else if (matches(state, 1U, 0U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_one_submode_zero;
        if (state.alternate_result == 0) {
            const auto sample = callbacks.sample_source_random_bound
                                    ? callbacks.sample_source_random_bound(100U)
                                    : 0U;
            if (sample >= 75U) {
                selection.message_variant =
                    FieldModeFinalizerMessageVariant::default_effect_prompt;
                if (callbacks.forward_effect_sequence) {
                    callbacks.forward_effect_sequence();
                }
            } else {
                selection.message_variant =
                    FieldModeFinalizerMessageVariant::default_failure_prompt;
                force_negative_status = true;
            }
        }
    } else if (matches(state, 1U, 4U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_one_submode_four;
    } else if (matches(state, 1U, 9U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_one_submode_nine;
    } else if (matches(state, 1U, 8U) || matches(state, 1U, 10U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_one_submode_eight_or_ten;
        force_negative_status = true;
    } else if (matches(state, 2U, 0U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_zero;
        selection.message_variant =
            FieldModeFinalizerMessageVariant::default_prompt;
    } else if (matches(state, 2U, 3U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_three;
    } else if (matches(state, 2U, 4U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_four;
        force_negative_status = state.alternate_result == 0;
    } else if (matches(state, 2U, 5U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_five;
        force_negative_status = state.alternate_result == 0;
    } else if (matches(state, 2U, 7U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_seven;
        if (state.alternate_result > 0) {
            selection.message_variant =
                FieldModeFinalizerMessageVariant::alternate_profile_positive;
        }
    } else if (matches(state, 2U, 8U) || matches(state, 2U, 10U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_eight_or_ten;
        force_negative_status = true;
    } else if (matches(state, 2U, 9U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_two_submode_nine;
    } else if (matches(state, 5U, 1U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_five_submode_one;
    } else if (matches(state, 5U, 6U)) {
        selection.resource_profile =
            FieldModeFinalizerResourceProfile::scene_five_submode_six;
    } else {
        matched = false;
    }

    if (!matched) {
        return FieldModeFinalizerAction::no_matching_route;
    }

    if (selection.message_variant ==
        FieldModeFinalizerMessageVariant::default_prompt) {
        selection.message_variant = signed_message_variant(state.alternate_result);
    }
    if (force_negative_status) {
        state.service_status = -1;
    } else {
        state.service_status = 0;
    }
    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(selection, state);
    }
    state.alternate_result = 0;
    return FieldModeFinalizerAction::handled;
}

} // namespace shadowrun::ghidra_conversion
