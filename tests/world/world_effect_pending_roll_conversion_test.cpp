#include "shadowrun_ghidra/world/world_effect_pending_roll_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectPendingRollState state{};
    state.pending_amount = 6U;
    state.comparison_amount = 2U;
    state.depletion_counter = 3U;
    int reset_calls = 0;
    WorldEffectPendingRollCallbacks callbacks{};
    callbacks.select_auxiliary_trial_count = [] {
        return std::optional<std::uint16_t>(4U);
    };
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t trials,
           const std::uint16_t difficulty) -> std::optional<std::uint8_t> {
        return trials == 4U && difficulty == 6U
                   ? std::optional<std::uint8_t>(5U)
                   : std::nullopt;
    };
    callbacks.reset_title_transition = [&] { ++reset_calls; };

    const auto reduced =
        advance_primary_world_effect_pending_roll(state, callbacks);
    if (reduced.action != WorldEffectPendingRollAction::reduced ||
        reduced.reduction != 3U || state.pending_amount != 3U ||
        state.depletion_counter != 2U || state.thresholded_rolls != 5U ||
        reset_calls != 1) {
        return 1;
    }

    state.pending_amount = 1U;
    state.comparison_amount = 5U;
    state.depletion_counter = 4U;
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t difficulty) {
            return std::optional<std::uint8_t>(difficulty == 2U ? 7U : 0U);
        };
    const auto depleted =
        advance_primary_world_effect_pending_roll(state, callbacks);
    if (depleted.action != WorldEffectPendingRollAction::depleted ||
        depleted.reduction != 2U || state.pending_amount != 0U ||
        state.depletion_counter != 0U || reset_calls != 2) {
        return 2;
    }

    state.pending_amount = 5U;
    state.comparison_amount = 5U;
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(5U);
        };
    const auto unchanged =
        advance_primary_world_effect_pending_roll(state, callbacks);
    if (unchanged.action != WorldEffectPendingRollAction::no_reduction ||
        state.pending_amount != 5U || reset_calls != 2) {
        return 3;
    }

    // The source branches on FFF0DD == 0 before subtracting the comparison
    // byte; zero must not wrap into a reduction even when comparison is 0xff.
    state.pending_amount = 5U;
    state.comparison_amount = 0xffU;
    state.depletion_counter = 4U;
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(0U);
        };
    const auto zero_roll =
        advance_primary_world_effect_pending_roll(state, callbacks);
    if (zero_roll.action != WorldEffectPendingRollAction::no_reduction ||
        zero_roll.reduction != 0U || state.pending_amount != 5U ||
        state.depletion_counter != 4U || reset_calls != 2) {
        return 4;
    }

    // 0x80 - 0x01 wraps to 0x7f with V set, so SUB.B/BLE takes the no-roll
    // reduction path even though a signed cast would look positive.
    state.pending_amount = 5U;
    state.comparison_amount = 1U;
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(0x80U);
        };
    const auto overflow_roll =
        advance_primary_world_effect_pending_roll(state, callbacks);
    if (overflow_roll.action != WorldEffectPendingRollAction::no_reduction ||
        overflow_roll.reduction != 0U || state.pending_amount != 5U ||
        reset_calls != 2) {
        return 5;
    }

    WorldEffectPendingRollCallbacks unavailable{};
    if (advance_primary_world_effect_pending_roll(state, unavailable).action !=
        WorldEffectPendingRollAction::dependency_unavailable) {
        return 6;
    }
    return 0;
}
