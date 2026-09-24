#include "shadowrun_ghidra/movement_fallback_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t rotate_heading_to_direction_slot(
    const std::uint16_t heading) {
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        ++rotated;
    }
    return static_cast<std::uint16_t>((rotated & 0x07U) << 1U);
}

bool probe_failed(
    const MovementFallbackAttempt& attempt,
    const MovementResultPolicy& result_policy) {
    const auto result = apply_movement_result_policy(
        attempt.collision_code, result_policy);
    return result <= 0;
}

} // namespace

void resolve_entity_fallback_movement(
    MovementFallbackState& entity,
    const bool use_target_heading,
    const MovementResultPolicy& result_policy,
    const MovementFallbackProbe& probe,
    const std::uint32_t restored_continuation) {
    if (entity.movement_status == 0U) {
        entity.movement_status = 1U;
    }

    const auto source_heading =
        use_target_heading ? entity.target_heading : entity.heading;
    const auto first = probe(rotate_heading_to_direction_slot(source_heading));
    if (probe_failed(first, result_policy)) {
        entity.position_x = first.candidate_x;
        entity.position_y = first.candidate_y;
        entity.continuation = restored_continuation;
        return;
    }

    entity.movement_status = 2U;
    const auto second_heading =
        static_cast<std::uint16_t>(source_heading + 0x4000U);
    const auto second = probe(rotate_heading_to_direction_slot(second_heading));
    if (probe_failed(second, result_policy)) {
        entity.position_x = second.candidate_x;
        entity.position_y = second.candidate_y;
        entity.continuation = restored_continuation;
        return;
    }

    const auto third_heading =
        static_cast<std::uint16_t>(source_heading - 0x8000U);
    static_cast<void>(probe(rotate_heading_to_direction_slot(third_heading)));
    entity.continuation = restored_continuation;
}

} // namespace shadowrun::ghidra_conversion
