#include "shadowrun_ghidra/collision_probe_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::int32_t signed_word(const std::uint16_t value) {
    if (value < 0x8000U) {
        return static_cast<std::int32_t>(value);
    }
    return static_cast<std::int32_t>(value) - 0x10000;
}

std::uint16_t add_coordinate(const std::uint16_t base,
                             const std::int16_t offset) {
    return static_cast<std::uint16_t>(
        signed_word(base) + static_cast<std::int32_t>(offset));
}

std::size_t select_offset_bank(
    const std::array<std::uint8_t, 3>& probe_bytes) {
    std::uint8_t selected = probe_bytes[2];
    for (const auto value : probe_bytes) {
        selected = value;
        if ((value & 0x80U) != 0U) {
            break;
        }
    }
    const auto rotated = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(selected << 3U) |
        static_cast<std::uint8_t>(selected >> 5U));
    return static_cast<std::size_t>(rotated & 0x03U);
}

} // namespace

void build_collision_probe_points(
    CollisionProbeState& state,
    const bool collision_mode_active,
    const std::uint16_t entity_x,
    const std::uint16_t entity_y,
    const std::array<std::uint8_t, 3>& probe_bytes,
    const CollisionProbeOffsetBank& offset_banks,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y) {
    if (collision_mode_active) {
        return;
    }

    const auto& offsets = offset_banks[select_offset_bank(probe_bytes)];
    for (std::size_t index = 0; index < state.points.size(); ++index) {
        state.points[index] = CollisionProbePoint{
            add_coordinate(entity_x, offsets[index].x),
            add_coordinate(entity_y, offsets[index].y)};
    }
    state.camera_x = camera_x;
    state.camera_y = camera_y;
}

} // namespace shadowrun::ghidra_conversion
