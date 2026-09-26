#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The pending bytes consumed by ROM 0x563CC. Their source addresses are
// intentionally not part of the host type; these are the observed roles.
struct WorldEffectPendingRollState {
    std::uint8_t pending_amount = 0; // FFFBBE +0x00
    std::uint8_t comparison_amount = 0; // FFFBBE +0x01
    std::uint8_t depletion_counter = 0; // FFFBBE +0x07
    std::uint8_t thresholded_rolls = 0; // FFF0DD
};

struct WorldEffectPendingRollCallbacks {
    // ROM 0x19BF6: returns the D6 trial count after selecting the current or
    // adjacent compact record's auxiliary value and applying the mode variant.
    std::function<std::optional<std::uint16_t>()>
        select_auxiliary_trial_count;
    // ROM 0x0DBA: receives D6 as trials and D4 as the clamped difficulty.
    std::function<std::optional<std::uint8_t>(
        std::uint16_t trial_count, std::uint16_t difficulty)>
        count_thresholded_random_rolls;
    // ROM 0x1B1FC: reset the title transition after a successful reduction.
    std::function<void()> reset_title_transition;
};

enum class WorldEffectPendingRollAction {
    no_reduction,
    reduced,
    depleted,
    dependency_unavailable,
};

struct WorldEffectPendingRollResult {
    WorldEffectPendingRollAction action =
        WorldEffectPendingRollAction::dependency_unavailable;
    std::uint8_t reduction = 0;
};

// ROM 0x563CC: consume a thresholded roll from the pending amount and run
// the title-transition reset/depletion follow-up when the roll exceeds the
// comparison byte.
WorldEffectPendingRollResult advance_primary_world_effect_pending_roll(
    WorldEffectPendingRollState& state,
    const WorldEffectPendingRollCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
