#include "shadowrun_ghidra/menus/hospital_options_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

HospitalOption move_hospital_option(const HospitalOption selection,
                                    const std::int8_t step) {
    return static_cast<HospitalOption>(move_wrapped_menu_cursor(
        static_cast<std::uint8_t>(selection), step, 2U));
}

void service_hospital_options_frame(
    const HospitalOptionsMenuCallbacks& callbacks,
    HospitalOptionsInputFrame& input) {
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.latch_controller_events) {
        callbacks.latch_controller_events();
    }
    if (callbacks.sample_controller_input) {
        input = callbacks.sample_controller_input();
    }
    if (callbacks.advance_random_grid_effect) {
        callbacks.advance_random_grid_effect();
    }
}

void service_leave_prompt_frame(const LeaveHospitalPromptCallbacks& callbacks,
                                HospitalOptionsInputFrame& input) {
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.latch_controller_events) {
        callbacks.latch_controller_events();
    }
    if (callbacks.sample_controller_input) {
        input = callbacks.sample_controller_input();
    }
    if (callbacks.advance_random_grid_effect) {
        callbacks.advance_random_grid_effect();
    }
}

} // namespace

HospitalOptionsMenuResult run_hospital_options_menu(
    HospitalOptionsMenuState& state,
    const HospitalOptionsMenuCallbacks& callbacks) {
    state.menu_result = 0;
    state.mode_result = 1U;
    if (callbacks.reset_mode_display_buffers) {
        callbacks.reset_mode_display_buffers();
    }

    for (;;) {
        if (callbacks.rebuild_hospital_options_screen) {
            callbacks.rebuild_hospital_options_screen();
        }
        state.menu_result = 0;
        if (callbacks.render_cursor) {
            callbacks.render_cursor(state.selection);
        }

        for (;;) {
            HospitalOptionsInputFrame input{};
            service_hospital_options_frame(callbacks, input);

            // ROM 0xC998 gives Up priority over Down and still evaluates the
            // confirm/action bits after moving the cursor in that frame.
            if (input.move_up) {
                state.selection = move_hospital_option(state.selection, -1);
                if (callbacks.render_cursor) {
                    callbacks.render_cursor(state.selection);
                }
            } else if (input.move_down) {
                state.selection = move_hospital_option(state.selection, 1);
                if (callbacks.render_cursor) {
                    callbacks.render_cursor(state.selection);
                }
            }

            if (input.confirm || input.alternate_confirm) {
                HospitalMenuHandlerOutcome outcome =
                    HospitalMenuHandlerOutcome::redraw_options;
                switch (state.selection) {
                case HospitalOption::medical_attention:
                    if (callbacks.run_medical_attention) {
                        outcome = callbacks.run_medical_attention();
                    }
                    break;
                case HospitalOption::cyberware:
                    // This is the direct 0xCA16 handler, not an invented
                    // generic menu callback.
                    state.mode_result = 1U;
                    if (callbacks.run_field_mode_service) {
                        callbacks.run_field_mode_service();
                    }
                    state.menu_result = -1;
                    outcome = HospitalMenuHandlerOutcome::return_to_mode_exit;
                    break;
                case HospitalOption::leave_hospital:
                    if (callbacks.run_leave_hospital_prompt) {
                        outcome = callbacks.run_leave_hospital_prompt();
                    }
                    break;
                }

                if (outcome == HospitalMenuHandlerOutcome::leave_hospital) {
                    return {HospitalOptionsMenuExit::left_hospital,
                            state.selection};
                }
                if (outcome ==
                    HospitalMenuHandlerOutcome::return_to_mode_exit) {
                    return {HospitalOptionsMenuExit::returned_to_mode_exit,
                            state.selection};
                }
                break;
            }

            // The source gives B/action priority over Start/cancel.
            if (input.action) {
                if (callbacks.handle_mode_action_event) {
                    callbacks.handle_mode_action_event();
                }
                continue;
            }
            if (input.cancel) {
                state.menu_result = 0;
                return {HospitalOptionsMenuExit::cancelled, state.selection};
            }
        }
    }
}

LeaveHospitalPromptResult run_leave_hospital_prompt(
    LeaveHospitalPromptState& state,
    const LeaveHospitalPromptCallbacks& callbacks) {
    state.selection = 1U;
    if (callbacks.render_prompt) {
        callbacks.render_prompt();
    }
    if (callbacks.render_cursor) {
        callbacks.render_cursor(state.selection);
    }

    for (;;) {
        HospitalOptionsInputFrame input{};
        service_leave_prompt_frame(callbacks, input);

        if (input.move_up) {
            state.selection = move_wrapped_menu_cursor(state.selection, -1, 1U);
            if (callbacks.render_cursor) {
                callbacks.render_cursor(state.selection);
            }
            continue;
        }
        if (input.move_down) {
            state.selection = move_wrapped_menu_cursor(state.selection, 1, 1U);
            if (callbacks.render_cursor) {
                callbacks.render_cursor(state.selection);
            }
            continue;
        }

        // ROM 0xCF02 accepts A, C, or B here; Start is not a separate exit
        // path in this prompt.
        if (input.confirm || input.alternate_confirm || input.action) {
            return state.selection == 0U
                       ? LeaveHospitalPromptResult::leave_hospital
                       : LeaveHospitalPromptResult::stay_in_hospital;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
