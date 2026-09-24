#include "shadowrun_ghidra/random_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t count_thresholded_random_rolls(
    const std::uint16_t trial_count,
    const std::int16_t threshold,
    const BoundedRandomRollBoundary& roll_random) {
    std::uint8_t qualifying_rolls = 0;
    const auto trials = trial_count == 0U ? 1U : trial_count;

    for (std::uint16_t trial = 0; trial < trials; ++trial) {
        std::int32_t accumulated_roll = 0;
        while (true) {
            const auto face = static_cast<std::int32_t>(
                (roll_random ? roll_random(6U) : 0) + 1);
            if (face == 6) {
                accumulated_roll += face;
                continue;
            }
            if (accumulated_roll + face >= threshold) {
                qualifying_rolls = static_cast<std::uint8_t>(
                    qualifying_rolls + 1U);
            }
            break;
        }
    }
    return qualifying_rolls;
}

} // namespace shadowrun::ghidra_conversion
