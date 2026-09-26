#include "shadowrun_ghidra/cyberspace/matrix_interaction_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

bool vertical_movement(const std::uint16_t input_events) {
    return (input_events & (move_up | move_down)) != 0U;
}

bool horizontal_movement(const std::uint16_t input_events) {
    return (input_events & (move_left | move_right)) != 0U;
}

} // namespace

MatrixInteractionResult run_matrix_interaction_loop(
    MatrixInteractionState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const MatrixInteractionCallbacks& callbacks) {
    MatrixInteractionResult result{};

    for (const auto input_events : frame_inputs) {
        ++result.frames_processed;
        call_if_present(callbacks.wait_for_vblank);
        call_if_present(callbacks.latch_controller_events);
        call_if_present(callbacks.sample_controller_input);
        call_if_present(callbacks.rotate_hud_history);

        // 0x1833C..0x18404: vertical input is tested before horizontal input.
        // The source's vertical path writes the two row grids first; the
        // horizontal path only moves the column highlight.
        if (vertical_movement(input_events) ||
            horizontal_movement(input_events)) {
            if (vertical_movement(input_events)) {
                call_if_present(callbacks.write_two_hud_grids);
            }
            update_matrix_cursor(state.cursor, input_events);
            call_if_present(callbacks.write_cursor, state.cursor);
            call_if_present(callbacks.redraw_matrix_panel);
            continue;
        }

        MatrixActionState action{};
        apply_matrix_action_input(action, input_events);
        if (action.action_requested) {
            state.overlay_marker = action.overlay_marker;
            result.last_action_route = choose_matrix_action_route(
                state.cursor.row, state.cursor.column,
                state.matrix_run_active);

            switch (result.last_action_route) {
            case MatrixActionRoute::program_slot_menu:
                call_if_present(callbacks.run_program_slot_screen);
                break;
            case MatrixActionRoute::column_action_dispatch:
                if (callbacks.dispatch_column_action &&
                    callbacks.dispatch_column_action()) {
                    call_if_present(callbacks.leave_matrix);
                    result.exit = MatrixInteractionExit::returned_to_field;
                    return result;
                }
                break;
            case MatrixActionRoute::disconnect_confirmation:
                if (callbacks.run_disconnect_confirmation &&
                    callbacks.run_disconnect_confirmation()) {
                    call_if_present(callbacks.leave_matrix);
                    result.exit = MatrixInteractionExit::returned_to_field;
                    return result;
                }
                break;
            case MatrixActionRoute::no_action:
                break;
            }
            continue;
        }

        // 0x18630: bit 15 is only considered when no action mode won.
        if (action.exit_requested) {
            call_if_present(callbacks.leave_matrix);
            result.exit = MatrixInteractionExit::returned_to_field;
            return result;
        }
    }

    result.exit = MatrixInteractionExit::frame_budget_exhausted;
    return result;
}

} // namespace shadowrun::ghidra_conversion
