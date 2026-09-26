#include "shadowrun_ghidra/world_effect_pending_roll_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_signed_nonpositive(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) <= 0;
}

bool source_signed_negative(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) < 0;
}

} // namespace

WorldEffectPendingRollResult advance_primary_world_effect_pending_roll(
    WorldEffectPendingRollState& state,
    const WorldEffectPendingRollCallbacks& callbacks) {
    if (!callbacks.select_auxiliary_trial_count ||
        !callbacks.count_thresholded_random_rolls) {
        return {};
    }
    const auto trial_count = callbacks.select_auxiliary_trial_count();
    if (!trial_count.has_value()) {
        return {};
    }

    const auto difficulty =
        state.pending_amount < 2U ? 2U : state.pending_amount;
    const auto roll = callbacks.count_thresholded_random_rolls(
        *trial_count, difficulty);
    if (!roll.has_value()) {
        return {};
    }
    state.thresholded_rolls = *roll;

    const auto reduction = static_cast<std::uint8_t>(
        state.thresholded_rolls - state.comparison_amount);
    if (source_signed_nonpositive(reduction)) {
        return {WorldEffectPendingRollAction::no_reduction, 0U};
    }

    state.pending_amount = static_cast<std::uint8_t>(
        state.pending_amount - reduction);
    if (source_signed_negative(state.pending_amount)) {
        state.pending_amount = 0U;
        state.depletion_counter = 0U;
        if (callbacks.reset_title_transition) {
            callbacks.reset_title_transition();
        }
        return {WorldEffectPendingRollAction::depleted, reduction};
    }

    if (state.depletion_counter != 0U) {
        state.depletion_counter = static_cast<std::uint8_t>(
            state.depletion_counter - 1U);
    }
    if (callbacks.reset_title_transition) {
        callbacks.reset_title_transition();
    }
    return {WorldEffectPendingRollAction::reduced, reduction};
}

} // namespace shadowrun::ghidra_conversion
