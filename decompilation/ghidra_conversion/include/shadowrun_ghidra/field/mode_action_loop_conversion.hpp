#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// High-byte event bits sampled from FFDD22 by the mode-action loop at ROM
// 0x54662.  The source tests bit 14 first, then aliases bits 12 and 13 back
// to the same action preamble before testing the Matrix exit bit.
enum class ModeActionInput : std::uint16_t {
    move_previous = 0x0100U,
    move_next = 0x0200U,
    confirm = 0x4000U,
    alternate_confirm = 0x1000U,
    mode_action = 0x2000U,
    leave_to_matrix = 0x8000U,
};

struct ModeActionRecord {
    // The selected FF1100 + 0x00 byte chooses the indirect action family and
    // +0x05 is the action-gate boundary used by 0x54662.
    std::int8_t kind = 0;
    std::int8_t option_count = 0;
    std::uint8_t flags = 0;
    bool clears_auxiliary_menu_state = false;
    // The separate 0x1664D8 mode table + 0x04 byte bounds the visible cursor.
    std::int8_t cursor_limit = 0;
};

struct ModeActionState {
    std::uint8_t mode_flags = 0;       // FFFB30[selected scene]
    std::int8_t display_result = -1;   // FFF0B7
    std::int8_t hud_abort = 0;          // FFF0B1; 0x54662 does not initialize it
    std::uint8_t mode_random_count = 0; // FFF0A8
    std::int8_t cursor = 0;             // FFF0DC
    std::int8_t auxiliary_marker = -1;  // FFF0A6
    std::int8_t mode_work_marker = 0;   // FFF0D4
    bool matrix_requested = false;
};

struct ModeActionGateInputs {
    // The source uses (FFFC04 + 0x88) - 1, with zero normalized to one.
    std::uint8_t action_bound = 1;
    // Result of 0x0DBA after the action preamble.
    std::int8_t thresholded_random_rolls = 0;
};

struct ModeActionLoopCallbacks {
    std::function<void(std::uint16_t)> set_display_control;
    std::function<void()> reset_display_buffers;
    std::function<void()> refresh_resource_state;
    std::function<void()> clear_mode_auxiliary_state;
    std::function<void()> write_wrapped_resource_block;
    std::function<void()> write_grid;
    std::function<void()> select_display_status;
    std::function<void()> refresh_mode_metadata;

    // 0x54A8E: returns true when the random mode event fired and the caller
    // should redraw the outer loop.
    std::function<bool(ModeActionState&)> advance_random_mode_event;
    std::function<void()> render_encoded_text;
    std::function<void()> render_mode_status_summary;
    // 0xC376 receives D2=direction and publishes FFF0DC.  The conversion
    // updates the typed cursor first, then lets the host draw the new cursor.
    std::function<void(std::int8_t direction, std::int8_t cursor)>
        move_menu_cursor;

    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> run_action_preamble;
    std::function<ModeActionGateInputs()> read_action_gate_inputs;
    std::function<void()> write_repeated_grid;
    std::function<void()> wait_for_controller_event;

    // The source indexes a table of handlers by record kind and cursor.  The
    // targets have not been independently identified, so the boundary stays
    // typed and visible rather than becoming a fake generic dispatch.
    std::function<void(std::int8_t record_kind, std::int8_t cursor)>
        dispatch_mode_action;

    std::function<void()> render_menu_footer;
    std::function<void()> upload_menu_resource_pair;
    // The bool reports whether the Matrix boundary published the source's
    // negative FFF0B1 abort marker.  The ROM still resets five timers and
    // performs the next outer redraw before observing that marker.
    std::function<bool()> enter_matrix_mode;
    std::function<void(std::size_t slot)> reset_matrix_slot_timer;
};

// ROM 0x532A8: fill the 35-entry mode auxiliary buffer with the source
// sentinel.  It was previously mislabeled as an audio-menu operation.
void fill_mode_auxiliary_sentinel(std::vector<std::int32_t>& values);

// ROM 0x549E4: preserve the one-call presentation boundary used by the mode
// loop after an action or random event.
void write_mode_repeated_grid_service(const std::function<void()>& write_grid);

struct ModeDisplayStatusState {
    std::uint8_t status_code = 0;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x549F6: choose the source status tile/code for the current HUD mode
// and publish the source display/resource dirty bit.
void select_mode_display_status(ModeDisplayStatusState& state,
                                std::int8_t display_mode);

// ROM 0x54A28: render the two encoded status fields and grouped decimal data.
void render_mode_status_summary(const std::function<void()>& render_encoded_text,
                                const std::function<void()>& build_grouped_decimal_glyphs);

struct RandomModeEventCallbacks {
    std::function<std::int8_t()> sample_random_bound;
    std::function<void()> transition_hud_mode;
    std::function<void()> select_display_status;
    std::function<void()> render_encoded_text;
    std::function<void()> wait_for_vblank_frame;
    std::function<void()> wait_for_controller_event;
    std::function<void()> write_repeated_grid;
};

// ROM 0x54A8E: consume the mode's random-event counter and perform the
// source's optional HUD/wait/repeated-grid refresh when the event fires.
bool advance_random_mode_event(ModeActionState& state,
                               const RandomModeEventCallbacks& callbacks);

// ROM 0x54662: mode-entry action loop reached from the world transition at
// 0x1ADE6.  The host owns the presentation and input callbacks; this function
// preserves the source state machine and call order without VDP or ROM access.
bool run_mode_action_loop(ModeActionState& state,
                          const ModeActionRecord& record,
                          const ModeActionLoopCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
