#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

using NibbleLookup = std::array<std::uint8_t, 16>;

// ROM 0x3E86: expand each packed byte through the source's 16-entry nibble
// table, preserving the mapped high and low nibbles in one output byte.
std::vector<std::uint8_t> expand_packed_nibbles(
    const std::vector<std::uint8_t>& packed, const NibbleLookup& lookup);

} // namespace shadowrun::ghidra_conversion
