#pragma once

#include "shadowrun_ghidra/behavior_animation_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityBehaviorSignature {
    std::uint8_t primary = 0;
    std::uint8_t secondary = 0;
};

struct SpawnedBehaviorEntity {
    std::uint16_t resource_id = 0;
    std::uint8_t render_flags = 0;
    std::uint8_t lifecycle_flags = 0;
    std::uint16_t animation = 0;
    std::uint8_t behavior_type = 0;
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t vertical_offset = 0;
    EntityBehaviorContinuation continuation =
        EntityBehaviorContinuation::ImmediateCollision;
};

struct ChildAllocation {
    std::uint16_t resource_id = 0;
    SpawnedBehaviorEntity child{};
};

using SignatureRefresh = std::function<void(EntityBehaviorSignature&)>;
using ChildAllocator = std::function<std::optional<ChildAllocation>()>;

void initialize_spawned_behavior_entity(
    SpawnedBehaviorEntity& child,
    std::uint16_t resource_id,
    std::uint16_t parent_x,
    std::uint16_t parent_y);

enum class EntityTransitionContinuation {
    SpawnCollisionCheck,
    TargetCollisionCheck,
};

struct EntityTransitionState {
    std::uint8_t type = 0;
    std::uint8_t subtype = 0;
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t desired_heading = 0;
    std::uint16_t direction_animation = 0;
    EntityTransitionContinuation continuation =
        EntityTransitionContinuation::TargetCollisionCheck;
    EntityDelayState delay{};
    EntityAnimationResetState animation{};
};

// ROM 0xB11C: refresh a related entity signature, optionally prepare its
// behavior and allocate a child, then move the current entity into the C042
// spawn-collision transition.
std::optional<SpawnedBehaviorEntity> synchronize_entity_behavior_spawn(
    EntityTransitionState& entity,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityDelayAttributes& delay_attributes,
    const std::array<std::uint16_t, 8>& direction_animations,
    const EntityBehaviorEffect& post_spawn_effect);

// ROM 0xB1DC: apply the initial behavior action, resolve a linked heading when
// present, enter the C0A2 target-collision transition, then optionally perform
// the follow-up action and related-entity spawn path.
std::optional<SpawnedBehaviorEntity> configure_entity_target_transition(
    EntityTransitionState& entity,
    const EntityDelayAttributes& delay_attributes,
    bool has_linked_entity,
    const std::function<std::uint16_t()>& resolve_linked_heading,
    const EntityBehaviorAction& behavior_action,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::function<bool()>& transition_check,
    bool alternate_follow_up_action,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityBehaviorEffect& post_transition_effect);

} // namespace shadowrun::ghidra_conversion
