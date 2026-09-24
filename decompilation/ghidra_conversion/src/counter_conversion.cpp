#include "shadowrun_ghidra/counter_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t apply_byte_counter_delta_with_negative_floor(
    const std::uint8_t current,
    const std::int8_t delta) {
    const auto updated = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(current) +
        static_cast<std::int16_t>(delta));
    if (static_cast<std::int8_t>(updated) < 0) {
        return 0xFF;
    }
    return updated;
}

} // namespace shadowrun::ghidra_conversion
