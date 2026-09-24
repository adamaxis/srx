#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0xC8B0: add a signed byte delta to a counter selected by the caller.
// The source keeps the low byte of the result and replaces any negative
// signed result with 0xFF, its ready/expired sentinel.
std::uint8_t apply_byte_counter_delta_with_negative_floor(
    std::uint8_t current,
    std::int8_t delta);

} // namespace shadowrun::ghidra_conversion
