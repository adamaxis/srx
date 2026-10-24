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

// ROM 0xBC36 door-contact math (row from FFEF10..12, 0xBD34 offsets, the
// FFDF40 gate) lives in Lua world_links.door_contact. CollisionProbeState is
// only the host mirror of the FFDF44..52 record it returns.

} // namespace shadowrun::ghidra_conversion
