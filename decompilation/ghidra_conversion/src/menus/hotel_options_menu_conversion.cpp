#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

HotelOption move_hotel_option(const HotelOption selection,
                              const std::int8_t step) {
    return static_cast<HotelOption>(move_wrapped_menu_cursor(
        static_cast<std::uint8_t>(selection), step, 1U));
}

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

            // ROM 0xCF98 checks Up before Down and still proceeds to the
            // confirm/action checks in the same sampled frame.
            if (input.move_up) {
                state.selection = move_hotel_option(state.selection, -1);
                if (callbacks.render_cursor) {
                    callbacks.render_cursor(state.selection);
                }
            } else if (input.move_down) {
                state.selection = move_hotel_option(state.selection, 1);
                if (callbacks.render_cursor) {
                    callbacks.render_cursor(state.selection);
                }
            }

            if (input.confirm) {
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
            if (input.action) {
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

        if (input.move_up) {
            state.selection = move_checkout_selection(state.selection, -1);
            if (callbacks.render_cursor) {
                callbacks.render_cursor(state.selection);
            }
        } else if (input.move_down) {
            state.selection = move_checkout_selection(state.selection, 1);
            if (callbacks.render_cursor) {
                callbacks.render_cursor(state.selection);
            }
        }

        // ROM 0xDAF0/0xDAFC accepts A/C/B-style confirmation inputs. The
        // selected result is tested only after movement has been applied.
        if (input.confirm || input.alternate_confirm || input.action) {
            return state.selection == HotelCheckoutSelection::yes
                       ? HotelCheckoutResult::checked_out
                       : HotelCheckoutResult::stay_at_hotel;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
