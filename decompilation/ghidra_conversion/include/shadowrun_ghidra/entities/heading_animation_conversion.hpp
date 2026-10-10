#pragma once

#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct HeadingAnimationState {
    std::uint16_t heading = 0;
    MovementContinuation continuation =
        MovementContinuation::ReevaluateCollision;
    std::uint16_t direction_animation = 0;
};

using MovementDelayScheduler = std::function<void(std::uint16_t delay_mode)>;
using AnimationReset = std::function<void()>;

// ROM 0xB39A: install a normal movement heading, use the C042 directional
// animation table, and schedule the source delay mode supplied by the caller
// before resetting animation state. The embedded B3EE continuation is
// represented by the typed enum.
void set_entity_heading_with_field_animation(
    HeadingAnimationState& entity,
    std::uint16_t heading,
    std::uint16_t delay_mode,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation);

// ROM 0xB3FE: install the alternate C092 directional animation table. This
// path resets animation before scheduling its fixed zero delay mode.
void set_entity_heading_with_alternate_animation(
    HeadingAnimationState& entity,
    std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation);

} // namespace shadowrun::ghidra_conversion
