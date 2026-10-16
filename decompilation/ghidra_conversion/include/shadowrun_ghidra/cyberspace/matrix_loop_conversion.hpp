#pragma once

#include <cstdint>
#include <functional>
#include <array>
#include <optional>

#include "shadowrun_ghidra/session/runner_action_delay_conversion.hpp"

namespace shadowrun::ghidra_conversion {

// The Matrix deck is a four-column by four-row program grid.  The source
// stores the selected row and column as bytes at FFF0D1 and FFF0D0.
struct MatrixCursorState {
    std::uint8_t column = 0;
    std::uint8_t row = 0;
    std::uint16_t row_marker = 0x0094;
    std::uint16_t column_marker = 0x008C;
    bool redraw_requested = false;
};

// ROM 0x18A92: clear D7, load FFF0D1 into its low byte, multiply the word by
// four, then add FFF0D0 to the low byte only.  The source returns the full D7
// register; the normal cursor domain (rows/columns 0..3) therefore produces
// 0..15, but the wider result preserves the source operation for audit and
// malformed-state cases too.
std::uint32_t matrix_program_index(std::uint8_t row, std::uint8_t column);

// ROM 0x1882E: program slots 0..11 consult FFFBCC, while the four extended
// slots (12..15) are not gated by that table.
bool matrix_program_is_available(
    std::uint8_t program_index,
    const std::array<std::uint8_t, 12>& availability_flags);

// ROM 0x1882E: only the first twelve slots enter the runner-detail/stat path
// before their description is rendered.
bool matrix_program_uses_runner_details(std::uint8_t program_index);

struct MatrixPanelRenderCallbacks {
    // 0x1882E always lays down the common Matrix panel header first.
    std::function<void()> write_panel_header;
    // A program in slots 0..11 with a zero FFBCC availability byte stops
    // after this message; no program resources are drawn in that case.
    std::function<void()> render_unavailable_message;
    std::function<void(std::uint8_t program_index)> render_program_icon;
    std::function<void(std::uint8_t program_index)> render_program_detail;
    // 0x1A16A renders the runner-detail grids and its program-group text.
    // The typed detail conversion below supplies the branch and row counts;
    // this callback is the host presentation boundary.
    std::function<void(std::uint8_t program_index)> render_runner_detail_panel;
    std::function<void(std::uint8_t program_index)> render_description_line;
};

// ROM 0x1882E: redraw the selected Matrix program panel. This expresses the
// source branch and ordering with host callbacks; it does not reproduce VDP
// grids or ROM-address arithmetic in the native runtime.
bool render_matrix_panel(
    std::uint8_t program_index,
    const std::array<std::uint8_t, 12>& availability_flags,
    const MatrixPanelRenderCallbacks& callbacks);

enum class MatrixRunnerDetailBranch {
    no_active_run,
    direct_status_program,
    special_program_group,
    delay_scaled_program,
};

struct MatrixRunnerDetailInput {
    std::uint8_t program_index = 0;        // result of ROM 0x18A92
    bool matrix_run_active = false;        // signed FFF0B0 is negative
    std::uint8_t program_status = 0;       // FFFBCC[index]
    std::uint8_t current_record_value = 0; // FF1100 + FFF118*0x14 + 0x05
    RunnerActionDelayInput delay_input{};
};

struct MatrixRunnerDetailPlan {
    MatrixRunnerDetailBranch branch = MatrixRunnerDetailBranch::no_active_run;
    std::uint8_t program_index = 0;
    std::uint16_t primary_value = 0;
    std::uint16_t final_value = 0;
    std::uint8_t primary_repeat_count = 0;
    std::uint8_t secondary_repeat_count = 0;
    std::uint8_t final_repeat_count = 0;
    // Source 0x1A27E computes the secondary row destination from the
    // primary loop counter restored by 0x11C0 (primary_repeat_count - 1),
    // then subtracts the compact-record +0x05 value.
    std::int16_t secondary_destination_delta = 0;
    bool eligibility_available = false;
    bool eligibility_positive = false;
};

struct MatrixRunnerDetailCallbacks {
    // ROM 0x1A16A writes the two common detail-panel grids before branching.
    std::function<void()> write_runner_header_grid;
    std::function<void()> render_no_active_run_text;
    std::function<void()> render_special_program_text;
    std::function<void(std::uint8_t)> write_primary_detail_rows;
    std::function<void(std::uint8_t, std::int16_t)> write_secondary_detail_rows;
    std::function<void(std::uint8_t)> write_final_detail_rows;
    // ROM 0x17C3C lookup used by the delay-scaled program group.
    std::function<std::optional<std::uint16_t>()> read_eligibility;
};

// ROM 0x1A16A: render the runner-specific Matrix detail panel. This keeps the
// source's program groups and row-count arithmetic as a typed plan while
// leaving actual host drawing to callbacks.
MatrixRunnerDetailPlan render_matrix_runner_details(
    const MatrixRunnerDetailInput& input,
    const MatrixRunnerDetailCallbacks& callbacks);

struct MatrixProgramSlotSelection {
    std::uint8_t slot = 0;
    bool slot_available = false;
    bool program_already_loaded = false;
};

// ROM 0x19238: scan the five FFFBD8 slot bytes for the selected program; if
// it is not loaded, select the first negative/free slot instead.
MatrixProgramSlotSelection select_matrix_program_slot(
    std::uint8_t program_index,
    const std::array<std::int8_t, 5>& slot_programs);

// These are the controller-event bits tested by 0x1822C after the common
// input latch/sample path has completed.
enum MatrixInput : std::uint16_t {
    move_up = 1U << 8,
    move_down = 1U << 9,
    move_left = 1U << 10,
    move_right = 1U << 11,
    set_overlay_marker_zero = 1U << 12,
    set_overlay_marker_one = 1U << 13,
    set_overlay_marker_special = 1U << 14,
    leave_matrix = 1U << 15,
};

struct MatrixActionState {
    // FFF0B7 is written with one of these three values before the source
    // action branch at 0x1842E is entered.
    std::uint8_t overlay_marker = 0;
    bool action_requested = false;
    bool exit_requested = false;
};

// ROM 0x18630..0x1866C: apply the action/exit bits in source priority. This
// does not name the physical Genesis buttons; it records their proven state
// effect instead.
void apply_matrix_action_input(MatrixActionState& state,
                               std::uint16_t input_events);

enum class MatrixActionRoute {
    no_action,
    program_slot_menu,
    column_action_dispatch,
    disconnect_confirmation,
};

// ROM 0x1842E..0x18628: choose the next source branch after an action is
// requested. `matrix_run_active` is the source FFF0B0 gate.
MatrixActionRoute choose_matrix_action_route(
    std::uint8_t row, std::uint8_t column, bool matrix_run_active);

// ROM 0x1822C writes the row highlight at 0x94 + row * 0x28 and the column
// highlight at 0x8C + column * 0x20.  These helpers expose the source result
// as ordinary host coordinates without carrying the original RAM addresses.
void update_matrix_cursor(MatrixCursorState& state,
                          std::uint16_t input_events);

struct MatrixLoopState {
    // Saved by 0x1822C on entry and restored by its return path.  They are
    // opaque mode state here; their source-specific consumers remain outside
    // this conversion slice.
    std::uint8_t saved_overlay_marker = 0;
    std::uint32_t saved_field_state = 0;
    std::uint8_t overlay_marker = 0;
    std::uint32_t field_state = 0;

    // FFF0D0/FFF0D1 and their derived highlight positions.
    MatrixCursorState cursor{};
    bool display_dirty = false;
};

struct MatrixLoopCallbacks {
    std::function<void()> write_transfer_header;
    std::function<void()> write_fixed_transfer_pair;
    std::function<void(std::uint16_t)> set_display_command;
    std::function<void()> clear_plane_a;
    std::function<void()> reset_upload_state;
    std::function<void(std::uint32_t source_address,
                       std::uint32_t destination_command)>
        decode_and_upload_source_stream;
    std::function<void()> refresh_mode_metadata;
    std::function<void()> render_hud_sprite_stream;
    std::function<void()> render_field_hud;
    std::function<void()> build_title_sprite_records;
    std::function<void()> write_hud_panel_grid;
    std::function<void()> render_hud_indicator_grids;
    std::function<void()> write_two_hud_grids;
    // 0x1882E redraws the selected Matrix program/panel.  Program execution
    // remains a separate boundary until its action branches are converted.
    std::function<void()> redraw_matrix_panel;
};

// Converts the deterministic setup at 0x1822C through its first redraw.  The
// source briefly draws the staging row 3, then clears FFF0D1 to row 0 before
// the first panel redraw. It deliberately does not execute a program or
// emulate Genesis VDP state.
void initialize_matrix_loop(MatrixLoopState& state,
                            const MatrixLoopCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
