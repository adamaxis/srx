#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MovementOffset {
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct MovementProbeInput {
    std::uint16_t entity_x = 0;
    std::uint16_t entity_y = 0;
    std::uint8_t direction = 0;
    std::uint8_t entity_flags = 0;
};

struct MovementProbeResult {
    std::int16_t collision_code = 0;
    std::uint16_t candidate_x = 0;
    std::uint16_t candidate_y = 0;
    // The source stores twice the eight-way direction ordinal in DF66. This
    // is the byte offset used by its two-byte movement-vector table.
    std::uint8_t source_direction_slot = 0;
};

using CollisionResult = std::function<std::int16_t(
    std::uint16_t candidate_x, std::uint16_t candidate_y)>;

// ROM 0xB77E: form an eight-way movement candidate and query the collision
// system. A negative collision result becomes the ordinary blocked marker
// 0x7F unless entity flag bit 1 preserves the detailed negative result.
MovementProbeResult probe_entity_directional_collision(
    const MovementProbeInput& input,
    const std::array<MovementOffset, 8>& movement_offsets,
    const CollisionResult& collision_result);

struct MovementResultPolicy {
    bool field_transition_active = false;
    std::uint16_t scene_id = 0;
    std::uint16_t collision_mode = 0;
};

// ROM 0xB732: normalize a negative directional probe result according to the
// field-transition and scene-specific rejection rules. Non-negative results
// pass through unchanged.
std::int16_t apply_movement_result_policy(
    std::int16_t collision_code,
    const MovementResultPolicy& policy);

} // namespace shadowrun::ghidra_conversion
