#include "shadowrun_ghidra/core/packed_data_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::vector<std::uint8_t> expand_packed_nibbles(
    const std::vector<std::uint8_t>& packed, const NibbleLookup& lookup) {
    std::vector<std::uint8_t> expanded;
    expanded.reserve(packed.size());
    for (const auto value : packed) {
        const auto high = lookup[(value >> 4U) & 0x0FU];
        const auto low = lookup[value & 0x0FU];
        expanded.push_back(static_cast<std::uint8_t>((high << 4U) | low));
    }
    return expanded;
}

} // namespace shadowrun::ghidra_conversion
