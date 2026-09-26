#include "shadowrun_ghidra/cyberspace/matrix_program_slot_screen_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

bool prompt_accept_requested(const std::uint16_t input_events) {
    return (input_events & (accept_prompt_a | accept_prompt_b |
                            accept_prompt_c)) != 0U;
}

void finish_matrix_program_slot_screen(
    const MatrixProgramSlotScreenCallbacks& callbacks) {
    call_if_present(callbacks.write_hud_panel_grid);
    call_if_present(callbacks.render_matrix_panel);
}

} // namespace

MatrixProgramSlotScreenResult run_matrix_program_slot_screen(
    MatrixProgramSlotScreenState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const MatrixProgramSlotScreenCallbacks& callbacks) {
    call_if_present(callbacks.prepare_prompt);
    call_if_present(callbacks.write_prompt_cursor, state.prompt.selection);

    MatrixProgramSlotScreenResult result{};
    result.selection = state.prompt.selection;

    for (const auto input_events : frame_inputs) {
        ++result.frames_processed;
        call_if_present(callbacks.wait_for_vblank);
        call_if_present(callbacks.latch_controller_events);
        call_if_present(callbacks.sample_controller_input);
        call_if_present(callbacks.rotate_hud_history);

        // 0x193A0..0x193CE: the source gives up priority to up, then down,
        // and redraws the Yes/No cursor without leaving the prompt.
        if ((input_events & move_prompt_up) != 0U) {
            update_matrix_loaded_program_prompt(state.prompt, -1);
            result.selection = state.prompt.selection;
            call_if_present(callbacks.write_prompt_cursor,
                            state.prompt.selection);
            continue;
        }
        if ((input_events & move_prompt_down) != 0U) {
            update_matrix_loaded_program_prompt(state.prompt, 1);
            result.selection = state.prompt.selection;
            call_if_present(callbacks.write_prompt_cursor,
                            state.prompt.selection);
            continue;
        }

        // 0x193D0..0x1941C: activation has priority over bit 15. Yes is
        // selection zero; No returns to the Matrix panel without mutating a
        // slot. Yes either applies the ready command or refreshes an empty
        // slot when the source timer is clear.
        if (prompt_accept_requested(input_events)) {
            result.selection = state.prompt.selection;
            if (state.prompt.selection != 0U) {
                result.exit = MatrixProgramSlotScreenExit::cancelled_prompt;
            } else if (state.ready_timer_active) {
                call_if_present(callbacks.apply_menu_slot_command);
                result.exit = MatrixProgramSlotScreenExit::installed_program;
            } else {
                call_if_present(callbacks.clear_ready_menu_slot_timer);
                state.ready_timer_active = false;
                call_if_present(callbacks.refresh_menu_slot_visual);
                result.exit =
                    MatrixProgramSlotScreenExit::refreshed_existing_slot;
            }
            finish_matrix_program_slot_screen(callbacks);
            return result;
        }

        if ((input_events & return_to_matrix_from_prompt) != 0U) {
            result.selection = state.prompt.selection;
            result.exit = MatrixProgramSlotScreenExit::returned_to_matrix;
            finish_matrix_program_slot_screen(callbacks);
            return result;
        }
    }

    result.selection = state.prompt.selection;
    result.exit = MatrixProgramSlotScreenExit::frame_budget_exhausted;
    return result;
}

} // namespace shadowrun::ghidra_conversion
