#include "shadowrun_ghidra/entity_transition_conversion.hpp"

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

bool signature_changed(
    const EntityBehaviorSignature& before,
    const EntityBehaviorSignature& after) {
    return before.primary != after.primary ||
           before.secondary != after.secondary;
}

std::optional<SpawnedBehaviorEntity> spawn_related_child_if_needed(
    EntityTransitionState& entity,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child) {
    const auto before = related_signature;
    refresh_signature(related_signature);
    if (!signature_changed(before, related_signature)) {
        return std::nullopt;
    }

    prepare_related_behavior();
    const auto allocation = allocate_child();
    if (!allocation.has_value()) {
        return std::nullopt;
    }

    auto child = allocation->child;
    initialize_spawned_behavior_entity(
        child, allocation->resource_id, entity.position_x, entity.position_y);
    return child;
}

} // namespace

void initialize_spawned_behavior_entity(
    SpawnedBehaviorEntity& child,
    const std::uint16_t resource_id,
    const std::uint16_t parent_x,
    const std::uint16_t parent_y) {
    child.resource_id = resource_id;
    child.render_flags = static_cast<std::uint8_t>(child.render_flags | 0x01U);
    child.animation = 4U;
    child.behavior_type = 1U;
    child.continuation = EntityBehaviorContinuation::ImmediateCollision;
    child.lifecycle_flags = static_cast<std::uint8_t>(
        child.lifecycle_flags | 0x01U);
    child.position_x = parent_x;
    child.position_y = static_cast<std::uint16_t>(parent_y + 0x10U);
    child.vertical_offset = 0x2000U;
}

std::optional<SpawnedBehaviorEntity> synchronize_entity_behavior_spawn(
    EntityTransitionState& entity,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityDelayAttributes& delay_attributes,
    const std::array<std::uint16_t, 8>& direction_animations,
    const EntityBehaviorEffect& post_spawn_effect) {
    auto child = spawn_related_child_if_needed(
        entity, related_signature, refresh_signature, prepare_related_behavior,
        allocate_child);

    update_entity_animation_delay(
        entity.delay, delay_attributes, decode_entity_delay_scale(0U));
    post_spawn_effect();
    entity.type = 6U;
    entity.subtype = 0U;
    entity.continuation = EntityTransitionContinuation::SpawnCollisionCheck;
    select_animation(entity.direction_animation, direction_animations,
                     entity.heading);
    reset_entity_animation_state(entity.animation);
    return child;
}

std::optional<SpawnedBehaviorEntity> configure_entity_target_transition(
    EntityTransitionState& entity,
    const EntityDelayAttributes& delay_attributes,
    const bool has_linked_entity,
    const std::function<std::uint16_t()>& resolve_linked_heading,
    const EntityBehaviorAction& behavior_action,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::function<bool()>& transition_check,
    const bool alternate_follow_up_action,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityBehaviorEffect& post_transition_effect) {
    update_entity_animation_delay(
        entity.delay, delay_attributes, decode_entity_delay_scale(1U));
    const auto initial_action = entity.type == 0x0CU ? 0x55U : 0x4CU;
    behavior_action(initial_action);

    if (has_linked_entity) {
        entity.desired_heading = resolve_linked_heading();
    }
    entity.heading = entity.desired_heading;
    entity.continuation = EntityTransitionContinuation::TargetCollisionCheck;
    select_animation(entity.direction_animation, direction_animations,
                     entity.heading);
    reset_entity_animation_state(entity.animation);

    if (!transition_check()) {
        return std::nullopt;
    }

    post_transition_effect();
    behavior_action(alternate_follow_up_action ? 0x4EU : 0x4DU);
    return spawn_related_child_if_needed(
        entity, related_signature, refresh_signature, prepare_related_behavior,
        allocate_child);
}

} // namespace shadowrun::ghidra_conversion
