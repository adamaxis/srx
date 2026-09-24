#include "shadowrun_ghidra/movement_probe_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t add_coordinate(const std::uint16_t base,
                             const std::int16_t offset) {
    return static_cast<std::uint16_t>(
        static_cast<std::int32_t>(base) + static_cast<std::int32_t>(offset));
}

} // namespace

MovementProbeResult probe_entity_directional_collision(
    const MovementProbeInput& input,
    const std::array<MovementOffset, 8>& movement_offsets,
    const CollisionResult& collision_result) {
    const auto direction = static_cast<std::size_t>(input.direction & 0x07U);
    const auto& offset = movement_offsets[direction];
    const auto candidate_x = add_coordinate(input.entity_x, offset.x);
    const auto candidate_y = add_coordinate(input.entity_y, offset.y);

    auto result = collision_result(candidate_x, candidate_y);
    if (result < 0 && (input.entity_flags & 0x02U) == 0U) {
        result = 0x7F;
    }

    return MovementProbeResult{
        result,
        candidate_x,
        candidate_y,
        static_cast<std::uint8_t>(direction * 2U)};
}

std::int16_t apply_movement_result_policy(
    const std::int16_t collision_code,
    const MovementResultPolicy& policy) {
    if (collision_code >= 0) {
        return collision_code;
    }

    const auto low_bits = static_cast<std::uint16_t>(collision_code) & 0x1FU;
    if (policy.field_transition_active || low_bits >= 0x18U ||
        ((policy.scene_id == 0x14U || policy.collision_mode == 2U) &&
         low_bits >= 2U && low_bits <= 5U)) {
        return 0x7F;
    }
    return collision_code;
}

} // namespace shadowrun::ghidra_conversion
