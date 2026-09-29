#include "shadowrun_ghidra/core/random_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t advance_source_random_seed(std::uint16_t& seed) {
    seed = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(seed * kSourceRandomMultiplier) + 1U);
    return seed;
}

std::int16_t sample_source_random_bound(std::uint16_t& seed,
                                        const std::uint16_t bound) {
    const auto raw = advance_source_random_seed(seed);
    const auto high = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(raw) * bound) >> 16U);
    return static_cast<std::int16_t>(high);
}

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
