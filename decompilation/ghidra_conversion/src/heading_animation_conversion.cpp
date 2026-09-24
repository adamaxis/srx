#include "shadowrun_ghidra/heading_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::size_t direction_index(const std::uint16_t heading) {
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        ++rotated;
    }
    return static_cast<std::size_t>(rotated & 0x07U);
}

std::uint16_t field_delay_mode(const std::uint8_t entity_behavior) {
    const auto reduced = static_cast<std::uint8_t>(entity_behavior - 1U);
    return (reduced == 0x0AU || reduced == 0x0BU) ? 0U : 1U;
}

} // namespace

void set_entity_heading_with_field_animation(
    HeadingAnimationState& entity,
    const std::uint16_t heading,
    const std::uint8_t entity_behavior,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation) {
    entity.heading = heading;
    entity.continuation = MovementContinuation::ReevaluateCollision;
    entity.direction_animation = direction_animations[direction_index(heading)];
    schedule_delay(field_delay_mode(entity_behavior));
    reset_animation();
}

void set_entity_heading_with_alternate_animation(
    HeadingAnimationState& entity,
    const std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation) {
    entity.heading = heading;
    entity.continuation = MovementContinuation::ReevaluateCollision;
    entity.direction_animation = direction_animations[direction_index(heading)];
    reset_animation();
    schedule_delay(0U);
}

} // namespace shadowrun::ghidra_conversion
