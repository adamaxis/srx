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
};

using CollisionProbeOffsetBank =
    std::array<std::array<CollisionProbeOffset, 3>, 4>;

// ROM 0xBC36: D4 is the first negative FFEF10..12 byte, ROL.B #3 and AND #3,
// written before TST.W FFDF40. A nonzero FFDF40 RTS with D4 set and leaves
// FFDF44/camera words. Zero copies three 0xBD34 bank offsets and FFDDC6/C8.
void build_collision_probe_points(
    CollisionProbeState& state,
    bool collision_mode_active,
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    const std::array<std::uint8_t, 3>& probe_bytes,
    const CollisionProbeOffsetBank& offset_banks,
    std::uint16_t camera_x,
    std::uint16_t camera_y);

} // namespace shadowrun::ghidra_conversion
