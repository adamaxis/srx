#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "shadowrun_ghidra/matrix_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {

enum class MatrixInteractionExit {
    frame_budget_exhausted,
    returned_to_field,
};

struct MatrixInteractionState {
    MatrixCursorState cursor{};
    std::uint8_t overlay_marker = 0;
    bool matrix_run_active = false;
};

struct MatrixInteractionResult {
    MatrixInteractionExit exit = MatrixInteractionExit::frame_budget_exhausted;
    MatrixActionRoute last_action_route = MatrixActionRoute::no_action;
    std::size_t frames_processed = 0;
};

struct MatrixInteractionCallbacks {
    // The source performs this frame service sequence before reading the
    // latched event word.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<void()> rotate_hud_history;

    // Vertical movement rewrites the two highlight grids; horizontal
    // movement updates only the derived cursor position.
    std::function<void()> write_two_hud_grids;
    std::function<void(const MatrixCursorState&)> write_cursor;
    std::function<void()> redraw_matrix_panel;

    // These are explicit native behavior boundaries for the source's
    // indirect/action handlers. A true return means the handler selected the
    // source's field-return cleanup path.
    std::function<void()> run_program_slot_screen;
    std::function<bool()> dispatch_column_action;
    std::function<bool()> run_disconnect_confirmation;
    std::function<void()> leave_matrix;
};

// ROM 0x1822C: service one Matrix frame at a time, preserving movement/action
// priority and the row/column route gates. `frame_inputs` bounds the host
// conversion for tests; the source loop itself waits indefinitely for input.
MatrixInteractionResult run_matrix_interaction_loop(
    MatrixInteractionState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const MatrixInteractionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
