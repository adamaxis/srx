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

std::optional<MovementDirection> field_movement_direction_from_input(
    const std::uint8_t dpad_input) {
    // Named input combinations recovered from the ROM's 0x19D2 map. This is
    // an explicit host mapping, so the runtime does not load or index a ROM
    // word table.
    switch (dpad_input & 0x0FU) {
    case 0x01U: return MovementDirection::north;
    case 0x09U: return MovementDirection::north_east;
    case 0x08U: return MovementDirection::east;
    case 0x0AU: return MovementDirection::south_east;
    case 0x02U: return MovementDirection::south;
    case 0x05U: return MovementDirection::north_west; // 0x19D2[5] = 0xE000 [C]
    case 0x04U: return MovementDirection::west;
    case 0x06U: return MovementDirection::south_west; // 0x19D2[6] = 0xA000 [C]
    default: return std::nullopt;
    }
}

} // namespace shadowrun::ghidra_conversion
