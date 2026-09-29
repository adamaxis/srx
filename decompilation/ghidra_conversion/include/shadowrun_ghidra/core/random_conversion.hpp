#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

using BoundedRandomRollBoundary =
    std::function<std::int16_t(std::uint16_t bound)>;

constexpr std::uint16_t kSourceRandomMultiplier = 0x20E5U;

// ROM 0x0D44: MULU #$20E5, ADDQ.W #1 into FFF9F8. Host holds the seed word.
std::uint16_t advance_source_random_seed(std::uint16_t& seed);

// ROM 0x0D3A: tick 0x0D44, MULU D1, SWAP, EXT.L. High word of seed*bound.
std::int16_t sample_source_random_bound(std::uint16_t& seed,
                                        std::uint16_t bound);

// ROM 0x0DBA: perform at least one six-sided roll, retry face 6 while adding
// six to the accumulated result, and count rolls meeting the signed threshold.
std::uint8_t count_thresholded_random_rolls(
    std::uint16_t trial_count,
    std::int16_t threshold,
    const BoundedRandomRollBoundary& roll_random);

} // namespace shadowrun::ghidra_conversion
