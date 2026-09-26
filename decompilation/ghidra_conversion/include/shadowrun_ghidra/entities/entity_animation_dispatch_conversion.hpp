#pragma once

#include "shadowrun_ghidra/entities/entity_target_motion_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationDispatchTarget {
    std::uint32_t identity = 0;
    std::uint16_t heading = 0; // linked record +0x2E
};

struct EntityAnimationDispatchState {
    std::uint8_t entity_flags = 0; // record +0x10
    std::uint8_t record_flags = 0; // record +0x11
    std::uint8_t phase = 0; // record +0x57
    std::int8_t frame = 0; // record +0x56
    std::int16_t transition_marker = 0; // record +0xDC
    std::int16_t collision_state = 0; // record +0x0C
    std::uint8_t entity_type = 0; // record +0x4E
    std::uint8_t action_counter = 0; // record +0x4F
    std::uint8_t route_flags = 0; // record +0xB0
    bool global_phase_active = false; // FFE186
    std::uint8_t lane = 0; // FFE190
    bool alternate_animation = false; // FFF0DD

    std::uint16_t animation_mode = 0; // FFDFE0
    std::uint16_t resource_window_marker = 0; // FFDFDE

    std::uint16_t current_x = 0; // record +0
    std::uint16_t current_y = 0; // record +4
    std::uint16_t target_x = 0; // record +0x96
    std::uint16_t target_y = 0; // record +0x98

    std::optional<EntityAnimationDispatchTarget> linked_target;
};

enum class EntityAnimationDispatchAction {
    no_linked_target,
    unlinked_fallback,
    held_special_phase,
    transition_blocked_without_collision,
    transition_resource_blocked,
    alternate_phase_resource_blocked,
    alternate_phase_gate_blocked,
    alternate_heading_prepared,
    waiting_for_spawn_window,
    spawn_heading_applied,
    spawn_cycle_cleanup,
    cleanup_resource_available,
    cleanup_resource_blocked,
    cleanup_noop,
    action_serviced,
    animation_reset_for_target_motion,
    target_motion_unresolved,
    target_motion_waiting,
    target_motion_finished,
    target_motion_spawn_synchronized,
    target_motion_transition_configured,
};

using EntityAnimationLinkRefresh =
    std::function<void(std::optional<EntityAnimationDispatchTarget>&)>;
using EntityAnimationFallback = std::function<void()>;

// For the source helpers at 0x14D8E and 0x15646, true means the source CCR
// carry was set.  Those calls are intentionally callbacks: their allocation,
// collision, and asset-side effects are separate conversion boundaries.
using EntityAnimationCarryBoundary = std::function<bool()>;
using EntityAnimationHeadingBoundary = std::function<void(std::uint16_t)>;
using EntityAnimationVoidBoundary = std::function<void()>;
using EntityAnimationLaneTargetSelector =
    std::function<void(std::optional<EntityAnimationDispatchTarget>&,
                       std::uint8_t lane)>;
using EntityAnimationTargetMotionResolver =
    std::function<EntityTargetMotionResult(EntityAnimationDispatchState&)>;

struct EntityAnimationDispatchCallbacks {
    EntityAnimationLinkRefresh refresh_link;
    EntityAnimationFallback fallback_unlinked;
    EntityAnimationCarryBoundary resource_gate;
    EntityAnimationCarryBoundary spawn_cycle;
    EntityAnimationCarryBoundary alternate_phase_gate;
    EntityAnimationLaneTargetSelector select_lane_target;
    EntityAnimationVoidBoundary update_target_heading;
    EntityAnimationVoidBoundary prepare_primary_animation;
    EntityAnimationVoidBoundary prepare_alternate_animation;
    EntityAnimationHeadingBoundary apply_field_heading;
    EntityAnimationHeadingBoundary apply_alternate_heading;
    EntityAnimationVoidBoundary service_action;
    EntityAnimationVoidBoundary update_animation_delay;
    EntityAnimationVoidBoundary upload_assets;
    EntityAnimationTargetMotionResolver resolve_target_motion;
    EntityAnimationVoidBoundary finish_animation;
    EntityAnimationVoidBoundary synchronize_behavior_spawn;
    EntityAnimationVoidBoundary configure_target_transition;
};

struct EntityAnimationDispatchResult {
    EntityAnimationDispatchAction action =
        EntityAnimationDispatchAction::target_motion_unresolved;
    bool link_refresh_called = false;
    bool source_failure_boundary = false;
    bool boundary_unresolved = false;
    bool lane_target_selected = false;
    bool target_destination_copied = false;
    std::optional<std::uint16_t> applied_heading;
};

// ROM 0x13FFE: dispatch one entity's animation/target state.  The source's
// 0x12345678 failure return is represented by source_failure_boundary rather
// than leaking a sentinel into native code; all allocation, collision, audio,
// and rendering work remains an explicit callback boundary.
EntityAnimationDispatchResult dispatch_entity_animation_state(
    EntityAnimationDispatchState& state,
    const EntityAnimationDispatchCallbacks& callbacks);

struct EntityAnimationActionServiceState {
    std::uint8_t pending_actions = 0; // record +0x4F
};

struct EntityAnimationActionServiceCallbacks {
    EntityAnimationVoidBoundary service_audio;
    EntityAnimationVoidBoundary update_animation_delay;
    EntityAnimationVoidBoundary advance_asset_meter;
};

// ROM 0x14226: service the action sound/delay/asset path and decrement the
// pending action counter once.
void service_entity_animation_action(
    EntityAnimationActionServiceState& state,
    const EntityAnimationActionServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
