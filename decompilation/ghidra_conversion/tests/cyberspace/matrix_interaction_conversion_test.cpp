#include "shadowrun_ghidra/cyberspace/matrix_interaction_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixInteractionState state{};
    state.cursor.row = 1U;
    std::vector<std::string> events;
    const auto callbacks = MatrixInteractionCallbacks{
        [&] { events.emplace_back("vblank"); },
        [&] { events.emplace_back("latch"); },
        [&] { events.emplace_back("sample"); },
        [&] { events.emplace_back("history"); },
        [&] { events.emplace_back("two-grids"); },
        [&](const MatrixCursorState&) { events.emplace_back("cursor"); },
        [&] { events.emplace_back("redraw"); },
        [&] { events.emplace_back("program-slot"); },
        [&] {
            events.emplace_back("column-action");
            return false;
        },
        [&] {
            events.emplace_back("disconnect");
            return true;
        },
        [&] { events.emplace_back("leave"); },
    };

    // Up wins over a simultaneous horizontal event and writes the vertical
    // grid pair before the cursor redraw. The next action dispatches the
    // program-slot screen and returns to the loop.
    const auto program = run_matrix_interaction_loop(
        state, {move_up | move_right, set_overlay_marker_one}, callbacks);
    if (!expect(program.exit == MatrixInteractionExit::frame_budget_exhausted)) {
        return 11;
    }
    if (!expect(state.cursor.row == 0U && state.cursor.column == 0U &&
                state.overlay_marker == 1U)) {
        return 13;
    }
    if (!expect(program.last_action_route == MatrixActionRoute::program_slot_menu)) {
        return 12;
    }
    if (!expect(events == std::vector<std::string>{
                    "vblank", "latch", "sample", "history", "two-grids",
                    "cursor", "redraw", "vblank", "latch", "sample",
                    "history", "program-slot"})) {
        return 14;
    }

    // The row-3/column-2 action is gated off when no Matrix run is active;
    // the source stays in the loop and does not call the column boundary.
    state = {};
    state.cursor.row = 3U;
    state.cursor.column = 2U;
    events.clear();
    const auto gated = run_matrix_interaction_loop(
        state, {set_overlay_marker_zero, leave_matrix}, callbacks);
    if (!expect(gated.last_action_route == MatrixActionRoute::no_action &&
                gated.exit == MatrixInteractionExit::returned_to_field &&
                events == std::vector<std::string>{
                    "vblank", "latch", "sample", "history", "vblank",
                    "latch", "sample", "history", "leave"})) {
        return 2;
    }

    // An active run on row 3/column 3 enters the disconnect confirmation;
    // accepting that boundary follows the field-return cleanup path.
    state = {};
    state.cursor.row = 3U;
    state.cursor.column = 3U;
    state.matrix_run_active = true;
    events.clear();
    const auto disconnected = run_matrix_interaction_loop(
        state, {set_overlay_marker_special}, callbacks);
    if (!expect(disconnected.exit == MatrixInteractionExit::returned_to_field &&
                disconnected.last_action_route ==
                    MatrixActionRoute::disconnect_confirmation &&
                events == std::vector<std::string>{"vblank", "latch", "sample",
                                                   "history", "disconnect",
                                                   "leave"})) {
        return 3;
    }

    // The source ignores bit 15 when an action mode is selected.
    state = {};
    state.cursor.row = 3U;
    state.cursor.column = 1U;
    events.clear();
    const auto action_priority = run_matrix_interaction_loop(
        state, {set_overlay_marker_one | leave_matrix}, callbacks);
    if (!expect(action_priority.exit ==
                    MatrixInteractionExit::frame_budget_exhausted &&
                events == std::vector<std::string>{"vblank", "latch", "sample",
                                                   "history", "column-action"})) {
        return 4;
    }

    return 0;
}
