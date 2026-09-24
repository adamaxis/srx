#pragma once

#include "shadowrun_ghidra/animation_reset_conversion.hpp"
#include "shadowrun_ghidra/entity_delay_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class EntityBehaviorContinuation {
    DelayedCollisionPrimary,
    DelayedCollisionAlternate,
    AuxiliaryCounterExpiry,
    ImmediateCollision,
};

struct DelayedBehaviorAnimationState {
    std::uint16_t heading = 0;
    std::uint16_t direction_animation = 0;
    EntityBehaviorContinuation continuation =
        EntityBehaviorContinuation::DelayedCollisionPrimary;
    EntityDelayState delay{};
    EntityAnimationResetState animation{};
};

// ROM 0xAF7C/0xAFAE: configure the delayed behavior animation using the C082
// direction table. The two entries differ only in their continuation state.
void prepare_entity_delayed_animation_primary(
    DelayedBehaviorAnimationState& state,
    const EntityDelayAttributes& attributes,
    std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations);

void prepare_entity_delayed_animation_alternate(
    DelayedBehaviorAnimationState& state,
    const EntityDelayAttributes& attributes,
    std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations);

struct EntityBehaviorState {
    std::uint8_t flags = 0;
    std::uint8_t auxiliary_state = 0;
    std::uint8_t countdown = 0;
    std::uint16_t heading = 0;
    std::uint16_t direction_animation = 0;
    EntityBehaviorContinuation continuation =
        EntityBehaviorContinuation::ImmediateCollision;
    EntityAnimationResetState animation{};
};

using EntityBehaviorEffect = std::function<void()>;
using EntityBehaviorAction = std::function<void(std::uint16_t action_code)>;

// ROM 0xAFF2: ignore an already-active behavior, otherwise choose the
// auxiliary countdown path or enter the immediate collision path. Controller
// and behavior effects remain explicit native service boundaries.
void prepare_entity_auxiliary_behavior(
    EntityBehaviorState& state,
    const std::array<std::uint16_t, 8>& auxiliary_animations,
    const std::array<std::uint16_t, 8>& immediate_animations,
    std::uint16_t heading,
    const EntityBehaviorEffect& controller_effect,
    const EntityBehaviorEffect& behavior_effect);

// ROM 0xB330: enter the immediate behavior path, dispatch its already-selected
// behavior action, and select the C0B2 directional animation entry.
void enter_entity_behavior_transition(
    EntityBehaviorState& state,
    const std::array<std::uint16_t, 8>& direction_animations,
    std::uint16_t heading,
    std::uint16_t action_code,
    const EntityBehaviorEffect& controller_effect,
    const EntityBehaviorAction& behavior_action);

// ROM 0x1562E: run the auxiliary preparation boundary before the already
// typed entity-behavior preparation boundary.
void service_entity_auxiliary_behavior(
    const EntityBehaviorEffect& prepare_auxiliary_assets,
    const EntityBehaviorEffect& prepare_behavior);

} // namespace shadowrun::ghidra_conversion
