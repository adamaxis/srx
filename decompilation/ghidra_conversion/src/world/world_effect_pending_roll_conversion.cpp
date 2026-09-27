#include "shadowrun_ghidra/world/world_effect_pending_roll_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_subtract_byte_is_nonpositive(const std::uint8_t minuend,
                                         const std::uint8_t subtrahend,
                                         const std::uint8_t result) {
    // 0x56408 is SUB.B followed by BLE. Preserve the source N/V/Z flags;
    // a signed cast loses the overflow case.
    const bool negative = (result & 0x80U) != 0U;
    const bool zero = result == 0U;
    const bool overflow =
        (((minuend ^ subtrahend) & 0x80U) != 0U) &&
        (((minuend ^ result) & 0x80U) != 0U);
    return zero || (negative != overflow);
}

bool source_byte_is_negative(const std::uint8_t value) {
    return (value & 0x80U) != 0U;
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

    // 0x563FE moves FFF0DD and 0x56404 branches on zero before touching the
    // comparison byte.
    if (state.thresholded_rolls == 0U) {
        return {WorldEffectPendingRollAction::no_reduction, 0U};
    }

    const auto reduction = static_cast<std::uint8_t>(
        state.thresholded_rolls - state.comparison_amount);
    if (source_subtract_byte_is_nonpositive(
            state.thresholded_rolls, state.comparison_amount, reduction)) {
        return {WorldEffectPendingRollAction::no_reduction, 0U};
    }

    state.pending_amount = static_cast<std::uint8_t>(
        state.pending_amount - reduction);
    if (source_byte_is_negative(state.pending_amount)) {
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
