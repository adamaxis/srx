#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

using BoundedRandomRollBoundary =
    std::function<std::int16_t(std::uint16_t bound)>;

// ROM 0x0DBA: perform at least one six-sided roll, retry face 6 while adding
// six to the accumulated result, and count rolls meeting the signed threshold.
std::uint8_t count_thresholded_random_rolls(
    std::uint16_t trial_count,
    std::int16_t threshold,
    const BoundedRandomRollBoundary& roll_random);

} // namespace shadowrun::ghidra_conversion
