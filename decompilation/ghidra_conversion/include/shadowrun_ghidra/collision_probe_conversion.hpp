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
};

// ROM 0xBC36: add three source-table offsets to an entity position and copy
// the current camera words. The source leaves the existing probe state alone
// while its collision-mode gate is active.
void build_collision_probe_points(
    CollisionProbeState& state,
    bool collision_mode_active,
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    const std::array<CollisionProbeOffset, 3>& offsets,
    std::uint16_t camera_x,
    std::uint16_t camera_y);

} // namespace shadowrun::ghidra_conversion
