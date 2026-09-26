#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x167C: expand the source coordinate stream into signed 16-bit values.
// The first big-endian word is the base coordinate and zero terminates the
// command stream.
std::optional<std::vector<std::int16_t>> decode_source_coordinate_stream(
    const std::vector<std::uint8_t>& encoded);

} // namespace shadowrun::ghidra_conversion
