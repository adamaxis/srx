#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct CollisionProbeOffset {
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct CollisionProbePoint {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct CollisionProbeState {
    std::array<CollisionProbePoint, 3> points{};
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::uint8_t selected_offset_bank = 0;
    bool initialized = false;
};

using CollisionProbeOffsetBank =
    std::array<std::array<CollisionProbeOffset, 3>, 4>;

// The gameplay layer expresses door contact with a named side, map position,
// and camera position. Lua applies the imported party formation; this state
// stores the resulting host positions for scene transitions.

} // namespace shadowrun::ghidra_conversion
