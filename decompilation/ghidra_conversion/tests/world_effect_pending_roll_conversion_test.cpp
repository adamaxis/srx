#include "shadowrun_ghidra/world_effect_pending_roll_conversion.hpp"

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

    WorldEffectPendingRollCallbacks unavailable{};
    if (advance_primary_world_effect_pending_roll(state, unavailable).action !=
        WorldEffectPendingRollAction::dependency_unavailable) {
        return 4;
    }
    return 0;
}
