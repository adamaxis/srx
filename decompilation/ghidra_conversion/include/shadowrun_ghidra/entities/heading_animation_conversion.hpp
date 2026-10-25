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
using ScriptDelayScheduler = std::function<void()>;
using AnimationReset = std::function<void()>;

// ROM 0xB39A installs a normal movement heading, uses the C042 directional
// animation table, and schedules the caller's source delay mode before
// resetting animation state. [C]
void set_entity_heading_with_field_animation(
    HeadingAnimationState& entity,
    std::uint16_t heading,
    std::uint16_t delay_mode,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation);

// Lua-owned player cadence overload: the host supplies one script delay
// boundary instead of decoding or forwarding the source D0 mode. [C]
void set_entity_heading_with_field_animation(
    HeadingAnimationState& entity,
    std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations,
    const ScriptDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation);

// ROM 0xB3FE installs the alternate C092 directional animation table. This
// path resets animation before scheduling its fixed zero delay mode. [C]
void set_entity_heading_with_alternate_animation(
    HeadingAnimationState& entity,
    std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations,
    const MovementDelayScheduler& schedule_delay,
    const AnimationReset& reset_animation);

} // namespace shadowrun::ghidra_conversion
