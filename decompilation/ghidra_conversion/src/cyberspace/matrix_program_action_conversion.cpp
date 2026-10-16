#include "shadowrun_ghidra/cyberspace/matrix_program_action_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

void render_rows(const MatrixProgramActionCallbacks& callbacks) {
    call_once(callbacks.render_rows);
}

} // namespace

MatrixProgramActionEvent service_matrix_program_action(
    MatrixProgramActionState& state,
    const MatrixProgramActionCallbacks& callbacks) {
    if (state.selected_slot >= state.program_codes.size() ||
        state.selected_slot >= state.visual_states.size() ||
        state.selected_slot >= state.selection_values.size() ||
        state.program_codes[state.selected_slot] < 0 ||
        state.visual_states[state.selected_slot] < 6U) {
        render_rows(callbacks);
        return MatrixProgramActionEvent::ignored;
    }

    auto& selection = state.selection_values[state.selected_slot];
    for (const auto blocked : {2U, 3U, 4U, 5U}) {
        if (selection == blocked) {
            render_rows(callbacks);
            return MatrixProgramActionEvent::ignored;
        }
    }
    if (state.overlay_stage != 0x0CU) {
        render_rows(callbacks);
        return MatrixProgramActionEvent::ignored;
    }

    if ((state.input_flags & (1U << 14U)) != 0U) {
        state.action_mode = -1;
    } else if ((state.input_flags & (1U << 12U)) != 0U) {
        state.action_mode = 0;
    } else {
        state.action_mode = 1;
    }

    const auto program_code = state.program_codes[state.selected_slot];
    if (callbacks.dispatch_program_action) {
        callbacks.dispatch_program_action(program_code, state.action_mode);
    }
    call_once(callbacks.reset_overlay);

    if (state.pending_action) {
        state.pending_action = false;
        render_rows(callbacks);
        return MatrixProgramActionEvent::pending_action_cleared;
    }

    if (selection == 7U) {
        if (state.runner_record_field < 0x11U) {
            render_rows(callbacks);
            return MatrixProgramActionEvent::ignored;
        }

        const auto transition_mode =
            (state.runner_record_field & 1U) != 0U ? 4U : 5U;
        if ((state.runner_record_field & 1U) == 0U &&
            static_cast<std::size_t>(program_code) <
                state.availability_flags.size()) {
            state.availability_flags[static_cast<std::size_t>(program_code)] =
                0U;
        }
        state.hud_mode_variant = 1;
        call_once(callbacks.update_hud_mode_transition);
        if (callbacks.initialize_world_effect) {
            callbacks.initialize_world_effect(state.selected_slot,
                                              transition_mode);
        }
        return MatrixProgramActionEvent::started_runner_action;
    }

    if (selection == 12U || selection == 11U) {
        render_rows(callbacks);
        return MatrixProgramActionEvent::ignored;
    }

    const auto code = static_cast<unsigned>(program_code);
    const std::uint8_t next_selection =
        code < 3U ? 3U : (code < 8U ? 5U : (code == 11U ? 2U : 4U));
    selection = next_selection;
    if (callbacks.publish_selection) {
        callbacks.publish_selection(state.selected_slot, next_selection);
    }
    call_once(callbacks.render_selection_text);
    render_rows(callbacks);
    return MatrixProgramActionEvent::selection_published;
}

} // namespace shadowrun::ghidra_conversion
