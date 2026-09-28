#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t field_movement_direction_from_heading(
    const std::uint16_t heading) {
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        ++rotated;
    }
    return static_cast<std::uint8_t>(rotated & 0x07U);
}

FieldMovementInputResult decode_field_movement_input(
    const std::uint8_t dpad_nibble,
    const std::array<std::uint16_t, 16>& target_heading_table) {
    const auto target_heading = target_heading_table[dpad_nibble & 0x0FU];
    if (target_heading == 0xFFFFU) {
        return {};
    }

    const auto direction_ordinal =
        field_movement_direction_from_heading(target_heading);
    return FieldMovementInputResult{
        true,
        target_heading,
        direction_ordinal,
        static_cast<std::uint16_t>(direction_ordinal * 2U)};
}

} // namespace shadowrun::ghidra_conversion
