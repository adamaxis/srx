#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion::integration {

// The source compares the pending indirect handler against 0x5BAC4.  The
// host names that behavior instead of retaining a code pointer or a ROM
// address in runtime state.
enum class PendingFieldSceneHandlerResult {
    resumed_field,
    returned_to_title,
    unresolved,
};

enum class FieldRuntimeFrameResult {
    frame_waiting,
    field_transition_pending,
    world_activity_pending,
    field_transition_dispatched,
    world_activity_dispatched,
    action_event_dispatched,
    command_overview_dispatched,
    field_session_restart_pending,
    field_session_restart_requested,
    world_activity_scene_reentry_requested,
    field_scene_reentry_requested,
    returned_to_title,
    callback_unresolved,
};

// Typed state for the recurring portion of ROM 0x075E.  The source offsets
// remain provenance comments; the conversion does not expose a Genesis RAM
// image or a function-pointer address.
struct FieldRuntimeCoordinatorState {
    bool field_transition_requested = false; // FFDF3E
    bool world_activity_requested = false; // FFDF42
    bool field_activity_lock = false; // FFE18B
    std::uint16_t world_activity_countdown = 0; // FFE166
    std::uint16_t display_service_word = 0; // FFDFE8
    std::int16_t mode_entry_marker = 0; // FFE172
    std::int16_t scene_refresh_marker = 0; // FFDF8E
    std::uint16_t controller_word = 0; // FFDD22
    std::uint16_t controller_held = 0; // FFDD20
    std::uint16_t controller_heading = 0xFFFFU; // FFDD04
    std::int16_t confirm_latch = 0; // FFDDC0
    std::int16_t start_latch = 0; // FFDDC2
    bool pending_field_scene_handler = false; // FFE104
    bool joshua_archetype_selection_pending = false; // FFE108
    std::uint16_t scene_id = 0; // FFDDC4
    std::uint8_t field_mode = 0; // FFFBF8, active scene resource mode
    std::uint8_t mode_event_code = 0; // FFFBF9, mode-transition table column
    std::int16_t transition_context = 0; // FFFC10, mode-transition table row
    std::uint32_t world_activity_flags = 0; // FFFC1E
    std::uint8_t scene_runtime_state = 0; // FFE186
    std::uint16_t field_scene_gate = 0; // FFDF40
    std::uint16_t mode_transition_gate = 0; // FFFC0E
    std::uint8_t world_activity_transition_marker = 0; // FFE18F
    bool world_activity_scene_reset_bit = false; // FFFC1C bit 7
    std::uint8_t display_dirty_flags = 0; // FFEF14

    bool action_event_dispatched = false;
    bool command_overview_dispatched = false;
    bool pending_handler_dispatched = false;
};

struct FieldRuntimeCoordinatorCallbacks {
    // 0x0952: every pass begins at a VBlank boundary.
    std::function<void()> wait_for_vblank;

    // 0x0976..0x0A64: the FFDF3E branch is kept in source order here rather
    // than hidden behind one generic transition callback. A bool returned by
    // toggle_field_scene_variant is the source carry result. The exit
    // callback returns the caller-visible D6 != 0 outcome; the native name is
    // the behavior it controls (whether entity anchors must be refreshed).
    std::function<void()> prepare_mode_entry_service;
    std::function<bool(FieldRuntimeCoordinatorState&)>
        toggle_field_scene_variant;
    std::function<bool(FieldRuntimeCoordinatorState&)>
        run_field_exit_orchestrator;
    std::function<void(FieldRuntimeCoordinatorState&)>
        reset_field_entity_transition_words;
    std::function<void(FieldRuntimeCoordinatorState&)> snapshot_entity_anchors;
    std::function<bool(FieldRuntimeCoordinatorState&)>
        advance_field_mode_transition;
    std::function<void(FieldRuntimeCoordinatorState&)>
        initialize_world_activity_mode;
    std::function<void(FieldRuntimeCoordinatorState&)>
        begin_scene_37_world_activity;

    // 0x0A72..0x0AA4: world-activity request handling and scene-state
    // re-entry are distinct converted leaves, followed by the 0x08C0 warmup.
    std::function<void(FieldRuntimeCoordinatorState&)>
        handle_world_activity_request;
    std::function<void(FieldRuntimeCoordinatorState&)>
        initialize_field_scene_state;

    // 0x0AA8..0x0ACC: ordinary field-frame services, kept in source order.
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> service_entity_behavior_frames;
    std::function<void()> update_field_camera;
    std::function<void()> update_entity_render_pass;
    std::function<void()> advance_palette_fade;
    std::function<void()> advance_world_frame_logic;

    // 0x0B08: mode-action input is checked before Start/command input.
    std::function<void()> handle_mode_action_event;

    // 0x0B62..0x0B80: Start/command overview branch.
    std::function<void(std::uint16_t)> run_fixed_audio_transaction;
    std::function<void()> run_command_overview;
    std::function<void()> service_audio_tick;
    std::function<void()> select_next_entity;
    std::function<void()> restart_field_session;
    std::function<void()> advance_field_scene_transition;

    // 0x0B34: source's indirect field-scene handler boundary.
    std::function<PendingFieldSceneHandlerResult()>
        run_pending_field_scene_handler;
};

// ROM 0x0952..0x0B9C: advance exactly one VBlank-sized pass of the recurring
// field coordinator.  The caller cycles this routine; it does not contain an
// unbounded emulation loop.  The return value identifies the source branch
// that the host controller must continue from.
FieldRuntimeFrameResult advance_field_runtime_frame(
    FieldRuntimeCoordinatorState& state,
    const FieldRuntimeCoordinatorCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion::integration
