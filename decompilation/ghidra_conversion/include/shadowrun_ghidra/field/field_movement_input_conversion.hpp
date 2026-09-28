#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct FieldMovementInputResult {
    bool valid = false;
    std::uint16_t target_heading = 0;
    std::uint8_t direction_ordinal = 0;
    // The movement-step conversion receives the source's doubled table-byte
    // offset; the directional probe then selects the corresponding pair.
    std::uint16_t direction_slot = 0;
};

// ROM 0x0BABC: rotate the current/target heading into the eight-way movement
// ordinal used by the fixed-point vector table at 0x0C002.
std::uint8_t field_movement_direction_from_heading(
    std::uint16_t heading);

// ROM 0x195C/0x19D2: use the normalized D-pad nibble to select a target
// heading, rejecting the source's 0xFFFF invalid/opposing combinations, then
// expose the ordinal and doubled source slot required by 0xBADE/0xB77E.
FieldMovementInputResult decode_field_movement_input(
    std::uint8_t dpad_nibble,
    const std::array<std::uint16_t, 16>& target_heading_table);

} // namespace shadowrun::ghidra_conversion
