#include "shadowrun_ghidra/behavior_animation_conversion.hpp"

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

void select_animation(
    std::uint16_t& selected_animation,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::uint16_t heading) {
    selected_animation = direction_animations[direction_index(heading)];
}

} // namespace

void prepare_entity_delayed_animation_primary(
    DelayedBehaviorAnimationState& state,
    const EntityDelayAttributes& attributes,
    const std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations) {
    update_entity_animation_delay(
        state.delay, attributes, decode_entity_delay_scale(0U));
    state.continuation = EntityBehaviorContinuation::DelayedCollisionPrimary;
    state.heading = heading;
    select_animation(state.direction_animation, direction_animations, heading);
    reset_entity_animation_state(state.animation);
}

void prepare_entity_delayed_animation_alternate(
    DelayedBehaviorAnimationState& state,
    const EntityDelayAttributes& attributes,
    const std::uint16_t heading,
    const std::array<std::uint16_t, 8>& direction_animations) {
    update_entity_animation_delay(
        state.delay, attributes, decode_entity_delay_scale(0U));
    state.continuation = EntityBehaviorContinuation::DelayedCollisionAlternate;
    state.heading = heading;
    select_animation(state.direction_animation, direction_animations, heading);
    reset_entity_animation_state(state.animation);
}

void prepare_entity_auxiliary_behavior(
    EntityBehaviorState& state,
    const std::array<std::uint16_t, 8>& auxiliary_animations,
    const std::array<std::uint16_t, 8>& immediate_animations,
    const std::uint16_t heading,
    const EntityBehaviorEffect& controller_effect,
    const EntityBehaviorEffect& behavior_effect) {
    if ((state.flags & 0x10U) != 0U) {
        return;
    }

    state.heading = heading;
    if (state.auxiliary_state != 0U) {
        state.continuation = EntityBehaviorContinuation::AuxiliaryCounterExpiry;
        select_animation(state.direction_animation, auxiliary_animations, heading);
        reset_entity_animation_state(state.animation);
        state.countdown = 8U;
        return;
    }

    state.flags = static_cast<std::uint8_t>(state.flags | 0x40U);
    controller_effect();
    state.flags = static_cast<std::uint8_t>(state.flags & 0xBFU);
    behavior_effect();
    state.flags = static_cast<std::uint8_t>(state.flags | 0x10U);
    state.continuation = EntityBehaviorContinuation::ImmediateCollision;
    select_animation(state.direction_animation, immediate_animations, heading);
    reset_entity_animation_state(state.animation);
}

void enter_entity_behavior_transition(
    EntityBehaviorState& state,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::uint16_t heading,
    const std::uint16_t action_code,
    const EntityBehaviorEffect& controller_effect,
    const EntityBehaviorAction& behavior_action) {
    state.heading = heading;
    state.flags = static_cast<std::uint8_t>(state.flags | 0x40U);
    controller_effect();
    state.flags = static_cast<std::uint8_t>(state.flags & 0xBFU);
    behavior_action(action_code);
    state.flags = static_cast<std::uint8_t>(state.flags | 0x10U);
    state.continuation = EntityBehaviorContinuation::ImmediateCollision;
    select_animation(state.direction_animation, direction_animations, heading);
    reset_entity_animation_state(state.animation);
}

} // namespace shadowrun::ghidra_conversion
