#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

HotelCheckoutSelection move_checkout_selection(
    const HotelCheckoutSelection selection, const std::int8_t step) {
    return static_cast<HotelCheckoutSelection>(move_wrapped_menu_cursor(
        static_cast<std::uint8_t>(selection), step, 1U));
}

void service_hotel_options_frame(const HotelOptionsMenuCallbacks& callbacks,
                                 HotelOptionsInputFrame& input) {
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

void service_checkout_frame(const HotelCheckoutPromptCallbacks& callbacks,
                            HotelOptionsInputFrame& input) {
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

std::optional<HotelCheckoutResult> apply_hotel_checkout_input(
    HotelCheckoutPromptState& state,
    const HotelOptionsInputFrame& input) noexcept {
    if (input.move_up) {
        state.selection = move_checkout_selection(state.selection, -1);
    } else if (input.move_down) {
        state.selection = move_checkout_selection(state.selection, 1);
    }
    if (input.move_up || input.move_down) {
        // ROM 0xDAA8..0xDAEE redraws the cursor and jumps back to the wait;
        // the A/B/C checks at 0xDAF0..0xDB10 are skipped for this sample.
        return std::nullopt;
    }
    if (!input.confirm && !input.alternate_confirm && !input.action) {
        return std::nullopt;
    }
    return state.selection == HotelCheckoutSelection::yes
               ? HotelCheckoutResult::checked_out
               : HotelCheckoutResult::stay_at_hotel;
}

HotelOptionsMenuResult run_hotel_options_menu(
    HotelOptionsMenuState& state,
    const HotelOptionsMenuCallbacks& callbacks) {
    // ROM 0xCF38 clears the entity-status marker before the shared mode reset
    // and the one-time party-status service. Each displayed menu rebuild then
    // starts FFF0DC at the first Hotel option.
    state.entity_status_marker = 0U;
    state.menu_result = 0;
    state.selection = HotelOption::karma_screen;
    if (callbacks.reset_mode_display_buffers) {
        callbacks.reset_mode_display_buffers();
    }
    if (callbacks.prepare_hotel_options_display) {
        callbacks.prepare_hotel_options_display();
    }
    if (callbacks.service_hotel_entity_status) {
        callbacks.service_hotel_entity_status();
    }

    for (;;) {
        if (callbacks.prepare_hotel_options_display) {
            callbacks.prepare_hotel_options_display();
        }
        if (callbacks.write_hotel_options_grid) {
            callbacks.write_hotel_options_grid();
        }
        if (callbacks.render_hotel_options_title) {
            callbacks.render_hotel_options_title();
        }
        state.menu_result = 0;
        state.selection = HotelOption::karma_screen;
        if (callbacks.render_cursor) {
            callbacks.render_cursor(state.selection);
        }

        for (;;) {
            HotelOptionsInputFrame input{};
            service_hotel_options_frame(callbacks, input);

            const auto transition = callbacks.advance_options_input
                ? callbacks.advance_options_input(state.selection, input)
                : HotelOptionsInputTransition{state.selection};
            state.selection = transition.selection;
            if (transition.selection_moved) {
                if (callbacks.render_cursor) {
                    callbacks.render_cursor(state.selection);
                }
            }

            if (transition.confirmed) {
                HotelMenuHandlerOutcome outcome =
                    HotelMenuHandlerOutcome::redraw_options;
                switch (state.selection) {
                case HotelOption::karma_screen:
                    if (callbacks.run_hotel_karma_screen) {
                        outcome = callbacks.run_hotel_karma_screen();
                    }
                    break;
                case HotelOption::check_out:
                    if (callbacks.run_hotel_checkout_prompt) {
                        outcome = callbacks.run_hotel_checkout_prompt();
                    }
                    break;
                }
                if (outcome == HotelMenuHandlerOutcome::return_to_field) {
                    state.menu_result = -1;
                    return {HotelOptionsMenuExit::returned_to_field,
                            state.selection};
                }
                break;
            }

            // ROM 0xCFEA handles only the action input here; it does not
            // treat Start/cancel as a separate Hotel Options exit.
            if (transition.action) {
                if (callbacks.handle_mode_action_event) {
                    callbacks.handle_mode_action_event();
                }
            }
        }
    }
}

HotelCheckoutResult run_hotel_checkout_prompt(
    HotelCheckoutPromptState& state,
    const HotelCheckoutPromptCallbacks& callbacks) {
    state.selection = HotelCheckoutSelection::no;
    if (callbacks.write_checkout_prompt_grid) {
        callbacks.write_checkout_prompt_grid();
    }
    if (callbacks.render_checkout_question) {
        callbacks.render_checkout_question();
    }
    if (callbacks.render_checkout_choices) {
        callbacks.render_checkout_choices();
    }
    if (callbacks.render_cursor) {
        callbacks.render_cursor(state.selection);
    }

    for (;;) {
        HotelOptionsInputFrame input{};
        service_checkout_frame(callbacks, input);

        const auto previous_selection = state.selection;
        const auto result = apply_hotel_checkout_input(state, input);
        if (state.selection != previous_selection) {
            if (callbacks.render_cursor) {
                callbacks.render_cursor(state.selection);
            }
        }
        if (result.has_value()) {
            return *result;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
