#include "shadowrun_ghidra/field_coordinate_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t fold_field_coordinate(std::uint16_t coordinate) {
    auto folded = static_cast<std::uint16_t>(coordinate >> 4U);
    if (folded >= 8U) {
        folded = static_cast<std::uint16_t>(folded - 8U);
    }
    return folded;
}

std::int16_t offset_field_coordinate(std::uint16_t coordinate) {
    const auto folded = static_cast<std::uint16_t>(coordinate >> 4U);
    return static_cast<std::int16_t>(folded - 8U);
}

std::int16_t quantize_field_delta(std::int16_t target,
                                  std::uint16_t origin) {
    const auto delta = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(target) - origin);
    const auto quantized = static_cast<std::int32_t>(delta) >> 7;
    return static_cast<std::int16_t>(quantized << 1);
}

} // namespace shadowrun::ghidra_conversion
