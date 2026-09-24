#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x1610: decode the source byte stream whose big-endian header stores
// output_size - 1. The returned bytes are host data; no VDP destination is
// part of this conversion.
std::optional<std::vector<std::uint8_t>> decode_source_byte_stream(
    const std::vector<std::uint8_t>& encoded);

} // namespace shadowrun::ghidra_conversion
