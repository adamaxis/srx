#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

#include <utility>

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

std::int16_t signed_word(const std::uint16_t value) {
    return static_cast<std::int16_t>(value);
}

std::uint16_t add_word(const std::uint16_t value,
                       const std::int16_t delta) {
    return static_cast<std::uint16_t>(
        static_cast<std::int32_t>(value) + delta);
}

bool signature_changed(
    const EntityBehaviorSignature& before,
    const EntityBehaviorSignature& after) {
    return before.primary != after.primary ||
           before.secondary != after.secondary;
}

// ROM 0x15318 / 0x15336. 0x15238 submits the selected word to
// play_sfx_if_enabled; these are event IDs, not graphics resources.
constexpr std::array<std::uint16_t, 15> kRegularSpawnAudioEvents{
    0x001CU, 0x001CU, 0x001CU, 0x001CU, 0x0051U,
    0x0051U, 0x0051U, 0x001EU, 0x001EU, 0x001EU,
    0x001DU, 0x001DU, 0x001EU, 0x001EU, 0x001EU};
constexpr std::array<std::uint16_t, 15> kAlternateSpawnAudioEvents{
    0x003FU, 0x003FU, 0x003FU, 0x003FU, 0x003FU,
    0x003FU, 0x003FU, 0x004AU, 0x004AU, 0x004AU,
    0x003FU, 0x003FU, 0x004AU, 0x004AU, 0x004AU};

// ROM 0x62EC and 0x62FC. The source selects one of these eight-entry
// direction tables after cloning the primary record.
constexpr std::array<std::uint16_t, 8> kRegularSpawnAnimations{
    0x0000U, 0x0004U, 0x0008U, 0x000CU,
    0x0010U, 0x0014U, 0x0018U, 0x001CU};
constexpr std::array<std::uint16_t, 8> kSpecialSpawnAnimations{
    0x0020U, 0x0024U, 0x0028U, 0x002CU,
    0x0030U, 0x0034U, 0x0038U, 0x003CU};

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

std::optional<std::uint16_t> select_entity_spawn_audio_event(
    const std::uint8_t source_action_code,
    const std::uint8_t source_status_flags) {
    if (source_action_code == 0U ||
        source_action_code > kRegularSpawnAudioEvents.size()) {
        return std::nullopt;
    }
    const bool use_alternate_resource_table =
        (source_status_flags & 0x06U) != 0U;
    const auto& audio_events = use_alternate_resource_table
                                  ? kAlternateSpawnAudioEvents
                                  : kRegularSpawnAudioEvents;
    return audio_events[source_action_code - 1U];
}

void initialize_entity_spawn_record(
    SpawnEntityRecordState& child,
    const SpawnEntityParentState& parent,
    const std::uint16_t source_identifier,
    const bool collision_gate_active,
    std::optional<SpawnEntityLinkedState>& linked_target,
    const SpawnPreparationBoundary& prepare_spawn) {
    initialize_entity_behavior_record(child.behavior_defaults);
    if ((parent.flags & 0x08U) != 0U) {
        child.behavior_defaults.flags = static_cast<std::uint8_t>(
            child.behavior_defaults.flags | 0x08U);
    }

    if (!collision_gate_active && prepare_spawn) {
        prepare_spawn();
    }

    child.movement_scale = 0x4000U;
    child.position_x = parent.position_x;
    child.position_y = parent.position_y;
    child.heading = parent.heading;
    child.field_0x9d = parent.field_0x9d;
    child.field_0x57 = parent.field_0x57;
    child.field_0x56 = parent.field_0x56;
    child.source_identifier = source_identifier;
    child.behavior_code = 0x0014U;
    child.flags = child.behavior_defaults.flags;
    child.has_parent_link = true;

    if (linked_target.has_value() &&
        (linked_target->flags & 0x01U) != 0U) {
        if (!collision_gate_active) {
            // This is the source write to linked_target + 0xAC. The host
            // record retains that fact without storing a source pointer.
            linked_target->parent_backlink_set = true;
        }
        child.heading = measure_relative_heading_and_distance(
                            parent.position_x,
                            parent.position_y,
                            linked_target->position_x,
                            linked_target->position_y)
                            .heading;
    }

    const auto direction = direction_index(child.heading);
    static constexpr std::array<std::uint16_t, 16> kPositionOffsets{
        0x0000U, 0xFF50U, 0x007CU, 0xFF84U, 0x00B0U, 0x0000U,
        0x007CU, 0x007CU, 0x0000U, 0x00B0U, 0xFF84U, 0x007CU,
        0xFF50U, 0x0000U, 0xFF84U, 0xFF84U};
    static constexpr std::array<std::uint16_t, 16> kProbeOffsets{
        0x0040U, 0xFF00U, 0x0120U, 0xFF10U, 0x0180U, 0xFFC0U,
        0x00D0U, 0x0050U, 0x0000U, 0x0020U, 0xFE80U, 0x0040U,
        0xFE80U, 0xFFC0U, 0xFF00U, 0xFF40U};
    const auto offset = direction * 2U;
    child.directional_probe_x = add_word(
        child.position_x, signed_word(kProbeOffsets[offset]));
    child.directional_probe_y = add_word(
        child.position_y, signed_word(kProbeOffsets[offset + 1U]));
    child.position_x = add_word(
        child.position_x, signed_word(kPositionOffsets[offset]));
    child.position_y = add_word(
        child.position_y, signed_word(kPositionOffsets[offset + 1U]));

    const auto heading_offsets = derive_entity_heading_offsets(child.heading);
    child.heading_offset_x = heading_offsets.horizontal;
    child.heading_offset_y = heading_offsets.vertical;
}

void prepare_special_spawn_record(SpawnEntityRecordState& record) {
    record.lifecycle_state = 2U;
    // ROM 0x152FA seeds the two remaining rays after the initial shot.
    record.field_0x41 = 2;
    record.saved_position_x = record.position_x;
    record.saved_position_y = record.position_y;
    record.field_0x8b = 1U;
    record.field_0x89 = 0x20U;
}

EntitySpawnCycleResult process_entity_spawn_cycle(
    EntitySpawnCycleState& state,
    const EntitySpawnCycleGate& behavior_gate,
    const EntitySpawnCycleSlotAllocator& allocate_slot,
    const SpawnPreparationBoundary& prepare_spawn,
    const EntitySpawnCycleEffect& refresh_world,
    const EntitySpawnCycleAudioEffect& play_audio_event,
    const EntitySpawnCycleModeEffect& apply_mode_maintenance,
    const EntitySpawnCycleAmmoService& service_ammo) {
    EntitySpawnCycleResult result{};
    if (state.global_mode == 3U) {
        result.action = EntitySpawnCycleAction::global_mode_blocked;
        return result;
    }

    if ((state.animation_counter & 0x07FFU) > 0x003CU) {
        result.action = EntitySpawnCycleAction::animation_window_expired;
        result.overflow_latch_set = (state.entity_flags & 0x02U) != 0U;
        return result;
    }

    if (!behavior_gate || !behavior_gate()) {
        result.action = EntitySpawnCycleAction::behavior_gate_blocked;
        return result;
    }

    if (!service_ammo) {
        result.action = EntitySpawnCycleAction::ammo_service_unresolved;
        return result;
    }
    result.ammo_service_action =
        service_ammo(state.equipment, state.parent.field_0x57);
    if (result.ammo_service_action ==
        EntityAmmoServiceAction::weapon_service_unresolved) {
        result.action = EntitySpawnCycleAction::ammo_service_unresolved;
        return result;
    }
    if (result.ammo_service_action == EntityAmmoServiceAction::no_action) {
        result.action = EntitySpawnCycleAction::ammo_service_inactive;
        return result;
    }

    if (!allocate_slot) {
        result.action = EntitySpawnCycleAction::no_primary_slot;
        return result;
    }
    const auto primary_slot = allocate_slot();
    if (!primary_slot.has_value()) {
        result.action = EntitySpawnCycleAction::no_primary_slot;
        return result;
    }

    if (refresh_world) {
        refresh_world();
    }

    SpawnEntityRecordState primary{};
    initialize_entity_spawn_record(
        primary,
        state.parent,
        *primary_slot,
        state.collision_gate_active,
        state.linked_target,
        prepare_spawn);
    primary.slot_identifier = *primary_slot;
    primary.field_0x89 = 0x7FU;
    primary.field_0x8b = 0U;

    result.special_follow_up =
        state.parent.field_0x57 >= 8U && state.parent.field_0x57 <= 10U;
    if (result.special_follow_up) {
        prepare_special_spawn_record(primary);
    }

    const auto secondary_slot = allocate_slot();
    if (!secondary_slot.has_value()) {
        result.action = EntitySpawnCycleAction::no_secondary_slot;
        result.primary_slot = primary_slot;
        result.primary = std::move(primary);
        return result;
    }

    const auto source_type = state.parent.field_0x57;
    if (source_type == 0U || source_type > kRegularSpawnAudioEvents.size()) {
        result.action = EntitySpawnCycleAction::invalid_source_type;
        result.primary_slot = primary_slot;
        result.secondary_slot = secondary_slot;
        result.primary = std::move(primary);
        return result;
    }

    result.selected_audio_event = select_entity_spawn_audio_event(
        source_type, state.source_status_flags).value_or(0U);
    if (play_audio_event) {
        play_audio_event(result.selected_audio_event);
    }

    const auto source_marker =
        (state.source_status_flags & 0x06U) != 0U ? 0xFFU : 0U;
    if (state.global_mode == 2U && source_marker == 0U &&
        apply_mode_maintenance) {
        apply_mode_maintenance(*secondary_slot);
    }

    auto secondary = primary;
    secondary.position_x = primary.directional_probe_x;
    secondary.position_y = primary.directional_probe_y;
    secondary.slot_identifier = *secondary_slot;
    secondary.record_flags = static_cast<std::uint8_t>(
        secondary.record_flags | 0x01U);
    // 0x152A2 clears the cloned record's +0x0E animation/behavior type word;
    // the initialized +0x1C movement timer remains inherited from primary.
    secondary.behavior_defaults.animation_state = 0U;
    secondary.continuation_address = 0x0000AC4EU;
    secondary.source_marker = static_cast<std::uint8_t>(source_marker);
    select_animation(
        secondary.animation_resource,
        result.special_follow_up ? kSpecialSpawnAnimations
                                 : kRegularSpawnAnimations,
        secondary.heading);
    secondary.lifecycle_state = 0U;

    result.action = EntitySpawnCycleAction::spawned;
    result.primary_slot = primary_slot;
    result.secondary_slot = secondary_slot;
    result.primary = std::move(primary);
    result.secondary = std::move(secondary);
    return result;
}

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
