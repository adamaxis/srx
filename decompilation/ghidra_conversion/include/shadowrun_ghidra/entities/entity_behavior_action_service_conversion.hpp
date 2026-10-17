#pragma once

#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityBehaviorActionWorldState {
    bool service_suppressed = false; // FFE18A
    bool movement_transition_active = false; // FFDF3A != 0
    bool activity_lock = false; // FFE18B
    std::uint8_t activity_countdown = 0; // FFE18C
    std::uint16_t controller_events = 0; // FFDD22
    std::int16_t controller_action = -1; // FFDD04
    std::int16_t action_override = 0; // FFDFE6
    std::uint8_t scene_state = 0; // FFE186; 0xAE6E RTS only when == 3
};

struct EntityEffectSpawnDispatchResult {
    // ROM 0x15646 leaves carry set when no effect record was created.
    bool source_carry = true;
    std::uint16_t spawned_heading = 0;
};

struct EntitySpawnCycleDispatchResult {
    // ROM 0x1514E leaves carry clear when the cycle produced a heading-bearing
    // record for the field-animation path.
    bool source_carry = true;
    std::uint16_t spawned_heading = 0;
};

struct EntityBehaviorActionCallbacks {
    std::function<void(EntityBehaviorFrameRecord&)>
        select_world_entity_service_target;
    std::function<void(EntityBehaviorFrameRecord&, std::uint16_t)>
        update_entity_animation_delay;
    std::function<void(EntityBehaviorFrameRecord&)> update_entity_target_heading;
    std::function<void(EntityBehaviorFrameRecord&)>
        prepare_delayed_animation_primary;
    std::function<void(EntityBehaviorFrameRecord&)>
        prepare_delayed_animation_alternate;
    std::function<EntityEffectSpawnDispatchResult(EntityBehaviorFrameRecord&)>
        spawn_entity_effect_record;
    std::function<void(EntityBehaviorFrameRecord&, std::uint16_t)>
        set_entity_heading_with_alternate_animation;
    std::function<EntitySpawnCycleDispatchResult(EntityBehaviorFrameRecord&)>
        process_entity_spawn_cycle;
    std::function<bool(EntityBehaviorFrameRecord&)>
        dispatch_player_action_if_ready;
    std::function<void(std::uint16_t)> play_sfx_if_enabled;
    std::function<void(EntityBehaviorFrameRecord&)>
        configure_entity_target_transition;
    std::function<void(EntityBehaviorFrameRecord&, std::uint16_t)>
        set_entity_heading_with_field_animation;
};

enum class EntityBehaviorActionResultKind {
    completed,
    callback_unresolved,
    service_suppressed,
    movement_transition_redirected,
    activity_countdown_waiting,
    activity_countdown_expired,
    animation_delay_pending,
    target_animation_prepared,
    effect_spawn_blocked,
    effect_heading_prepared,
    target_transition_prepared,
    spawn_cycle_heading_prepared,
    action_audio_delay_prepared,
    collision_recheck_prepared,
    controller_heading_prepared,
    no_action,
};

struct EntityBehaviorActionResult {
    EntityBehaviorActionResultKind kind =
        EntityBehaviorActionResultKind::completed;
    bool service_target_selected = false;
    bool action_override_set = false;
    bool action_dispatched = false;
    std::uint16_t audio_event = 0;
};

// ROM 0x0ACAE: route one entity's behavior action through the field-service
// gates and its type-specific target/spawn/animation branches. Override==0
// and FFDD22 bit 6 clear skip those branches and finish at 0xAE26.
EntityBehaviorActionResult advance_entity_behavior_action(
    EntityBehaviorFrameRecord& record,
    EntityBehaviorActionWorldState& world,
    const EntityBehaviorActionCallbacks& callbacks);

bool entity_behavior_action_callbacks_complete(
    const EntityBehaviorFrameRecord& record,
    const EntityBehaviorActionWorldState& world,
    const EntityBehaviorActionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
