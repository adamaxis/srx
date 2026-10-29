#pragma once

#include "shadowrun_ghidra/core/movement_direction.hpp"

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x195C/0x19D2 accepts the eight single/adjacent-direction combinations.
// Return their game meaning directly; opposing, empty, and three/four-way
// combinations produce no movement intent.
std::optional<MovementDirection> field_movement_direction_from_input(
    std::uint8_t dpad_input);

// Converts an established field heading into the octant used by movement
// rendering and collision probes.
std::uint8_t field_movement_direction_from_heading(std::uint16_t heading);

} // namespace shadowrun::ghidra_conversion
