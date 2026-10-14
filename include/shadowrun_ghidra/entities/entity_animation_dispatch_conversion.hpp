#pragma once

#include "shadowrun_ghidra/entities/entity_animation_motion_conversion.hpp"
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

// The coordinator is the separately entered 0x13904 half of the animation
// state machine.  It shares the timer/global-frame gate with 0x138B8, but its
// record fields and branches are different: it handles type-0x0F delayed
// animation, route/effect flags, world-resource gating, and the hand-off to
// 0x13FFE/0x13D0C.  Keeping this state typed makes the source behavior
// inspectable without importing a 68000 record or a ROM-addressed runtime.
struct EntityAnimationCoordinatorAnchor {
    std::uint32_t identity = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t heading = 0;
};

struct EntityAnimationCoordinatorState {
    std::uint16_t timer = 0; // record +0xB6
    bool global_frame_lock = false; // FF E175
    std::uint16_t record_type = 0; // record +0x0E
    std::uint16_t animation_elapsed = 0; // record +0x16
    std::uint8_t route_flags = 0; // record +0xB0
    std::uint16_t transition_flags = 0; // record +0x9E
    std::int16_t collision_state = 0; // record +0x0C
    bool world_resource_active = false; // FF DF84
    bool world_phase_active = false; // FF E186
    std::uint8_t entity_flags = 0; // record +0x10
    std::uint8_t entity_kind = 0; // record +0x4E
    bool alternate_animation = false; // FF F0DD
    std::uint8_t special_tick = 0; // FF E188

    std::uint16_t current_x = 0; // record +0
    std::uint16_t current_y = 0; // record +4
    std::uint16_t target_x = 0; // record +0x96
    std::uint16_t target_y = 0; // record +0x98
    std::uint16_t relative_heading = 0; // 0x15D3C result
    std::uint16_t relative_distance = 0; // 0x15D3C result

    // A present anchor represents the record pointer at +0xAC.  A missing
    // anchor follows the source's DAT_FFFC04/player-reference path.
    std::optional<EntityAnimationCoordinatorAnchor> linked_anchor;
};

enum class EntityAnimationCoordinatorTargetRouteReason {
    active_world_idle,
    animation_dispatch_failure,
};

enum class EntityAnimationCoordinatorBoundary {
    special_link_selection,
    special_heading_update,
    special_animation_preparation,
    fallback_target,
    boundary_transition,
    effect_slot_cleanup,
    target_continuation,
    inactive_world_target_route,
    active_world_target_route,
    resource_gate,
    relative_heading_measurement,
    animation_dispatch,
    animation_motion,
};

enum class EntityAnimationCoordinatorAction {
    waiting_for_timer,
    suppressed_by_frame_lock,
    special_animation_window_closed,
    special_animation_continuation_scheduled,
    special_animation_primary_prepared,
    special_animation_alternate_prepared,
    fallback_target_prepared,
    transition_phase_held,
    boundary_transition_serviced,
    effect_slot_cleared,
    inactive_world_target_prepared,
    active_world_target_prepared,
    activity_timeout_scheduled,
    animation_dispatched_to_motion,
    linked_animation_dispatched_to_motion,
    animation_dispatch_failure_routed,
    boundary_unresolved,
};

using EntityAnimationCoordinatorVoidBoundary = std::function<void()>;
using EntityAnimationCoordinatorTargetRoute =
    std::function<void(EntityAnimationCoordinatorState&,
                       EntityAnimationCoordinatorTargetRouteReason)>;
using EntityAnimationCoordinatorDispatch =
    std::function<EntityAnimationDispatchResult(
        EntityAnimationCoordinatorState&)>;
using EntityAnimationCoordinatorMeasurement =
    std::function<std::optional<EntityAnimationHeadingDistance>(
        std::uint16_t current_x,
        std::uint16_t current_y,
        std::uint16_t linked_x,
        std::uint16_t linked_y)>;
using EntityAnimationCoordinatorMotion =
    std::function<void(EntityAnimationCoordinatorState&)>;

struct EntityAnimationCoordinatorCallbacks {
    EntityAnimationCoordinatorVoidBoundary prepare_fallback_target;
    EntityAnimationCoordinatorVoidBoundary service_boundary_transition;
    EntityAnimationCoordinatorVoidBoundary clear_effect_slot;
    EntityAnimationCoordinatorVoidBoundary schedule_target_continuation;
    EntityAnimationCoordinatorVoidBoundary schedule_world_activity_timeout;
    EntityAnimationCoordinatorVoidBoundary run_resource_gate;

    // These three callbacks are the 0x14492/0x0AE6E/0x0AF7C or 0x0AFAE
    // delayed-animation calls in the type-0x0F branch.
    std::function<void(std::optional<EntityAnimationCoordinatorAnchor>&)>
        select_special_link;
    EntityAnimationCoordinatorVoidBoundary update_special_heading;
    EntityAnimationCoordinatorVoidBoundary prepare_primary_animation;
    EntityAnimationCoordinatorVoidBoundary prepare_alternate_animation;

    // 0x15D3C runs only when +0xAC contains a linked record.  The result is
    // retained in typed fields before entering 0x13FFE.
    EntityAnimationCoordinatorMeasurement measure_relative_heading_distance;
    EntityAnimationCoordinatorDispatch dispatch_animation_state;
    EntityAnimationCoordinatorMotion run_animation_motion;
    EntityAnimationCoordinatorTargetRoute prepare_target_route;
    EntityAnimationCoordinatorVoidBoundary
        prepare_inactive_world_target_route;
};

struct EntityAnimationCoordinatorResult {
    EntityAnimationCoordinatorAction action =
        EntityAnimationCoordinatorAction::boundary_unresolved;
    bool timer_expired = false;
    bool timer_rearmed = false;
    bool relative_measurement_called = false;
    bool dispatch_called = false;
    bool motion_called = false;
    bool linked_anchor_copied = false;
    bool resource_gate_called = false;
    bool activity_timeout_called = false;
    bool source_dispatch_failure = false;
    std::optional<EntityAnimationDispatchAction> dispatch_action;
    std::optional<EntityAnimationCoordinatorBoundary> unresolved_boundary;
};

// ROM 0x13904: service the second animation-update entry.  This is a native
// control-flow contract, not a runtime emulator: callers provide the already
// converted 0x14508, 0x13FFE, 0x13D0C, 0x1450E, and 0x142AE boundaries.
EntityAnimationCoordinatorResult run_entity_animation_update_coordinator(
    EntityAnimationCoordinatorState& state,
    const EntityAnimationCoordinatorCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
