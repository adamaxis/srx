#pragma once

#include <cstdint>
#include <functional>
#include <array>

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

// ROM 0x18A92: convert the two byte selections at FFF0D1/FFF0D0 into the
// linear Matrix program slot used by the panel/resource helpers.
std::uint8_t matrix_program_index(std::uint8_t row, std::uint8_t column);

// ROM 0x1882E: program slots 0..11 consult FFFBCC, while the four extended
// slots (12..15) are not gated by that table.
bool matrix_program_is_available(
    std::uint8_t program_index,
    const std::array<std::uint8_t, 12>& availability_flags);

// ROM 0x1882E: only the first twelve slots enter the runner-detail/stat path
// before their description is rendered.
bool matrix_program_uses_runner_details(std::uint8_t program_index);

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
