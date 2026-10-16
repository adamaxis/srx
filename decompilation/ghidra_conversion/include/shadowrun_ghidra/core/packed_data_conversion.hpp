#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

using NibbleLookup = std::array<std::uint8_t, 16>;

// ROM 0x3EDC. 0x3E86 LEAs this table for the 0x2000, 0x4000, and fall-through
// +0x2C compares; all three paths use the same 16 bytes.
constexpr NibbleLookup kPackedNibbleTable{{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x04, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x03, 0x02}};

// ROM 0x3E86: map each packed byte's high then low nibble through 0x3EDC.
std::vector<std::uint8_t> expand_packed_nibbles(
    const std::vector<std::uint8_t>& packed);

} // namespace shadowrun::ghidra_conversion
