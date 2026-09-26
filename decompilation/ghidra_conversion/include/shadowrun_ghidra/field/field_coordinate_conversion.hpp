#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x41B6: fold the high nibble into the source's eight-value field band.
std::uint16_t fold_field_coordinate(std::uint16_t coordinate);

// ROM 0x4216: subtract eight from the high-nibble coordinate.
std::int16_t offset_field_coordinate(std::uint16_t coordinate);

// ROM 0x423C: quantize a wrapped 16-bit coordinate delta in 128-unit steps.
std::int16_t quantize_field_delta(std::int16_t target,
                                  std::uint16_t origin);

} // namespace shadowrun::ghidra_conversion
