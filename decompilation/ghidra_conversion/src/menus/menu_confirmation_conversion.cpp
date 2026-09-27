#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint16_t bit(const MenuConfirmationInput input) {
    return static_cast<std::uint16_t>(input);
}

void move_binary_option(MenuConfirmationState& state, const int delta) {
    const auto candidate = static_cast<int>(state.selected_option) + delta;
    if (candidate < 0) {
        state.selected_option = 1U;
    }
    else if (candidate > 1) {
        state.selected_option = 0U;
    }
    else {
        state.selected_option = static_cast<std::uint8_t>(candidate);
    }
}

} // namespace

MenuConfirmationState run_menu_confirmation_prompt(
    const MenuConfirmationCallbacks& callbacks) {
    MenuConfirmationState state{};
    if (callbacks.draw_prompt_labels) {
        callbacks.draw_prompt_labels();
    }
    if (callbacks.draw_prompt_choice) {
        callbacks.draw_prompt_choice();
    }

    for (;;) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        const auto input = callbacks.sample_controller_input
                               ? callbacks.sample_controller_input()
                               : std::uint16_t{0};

        if ((input & bit(MenuConfirmationInput::option_decrement)) != 0U) {
            if (callbacks.play_move_sound) {
                callbacks.play_move_sound();
            }
            move_binary_option(state, -1);
            if (callbacks.draw_prompt_choice) {
                callbacks.draw_prompt_choice();
            }
            continue;
        }
        if ((input & bit(MenuConfirmationInput::option_increment)) != 0U) {
            if (callbacks.play_move_sound) {
                callbacks.play_move_sound();
            }
            move_binary_option(state, 1);
            if (callbacks.draw_prompt_choice) {
                callbacks.draw_prompt_choice();
            }
            continue;
        }
        if ((input & (bit(MenuConfirmationInput::confirm) |
                     bit(MenuConfirmationInput::cancel))) == 0U) {
            continue;
        }

        if (callbacks.play_confirm_sound) {
            callbacks.play_confirm_sound();
        }
        state.accepted = state.selected_option == 0U;
        state.closed = true;
        return state;
    }
}

} // namespace shadowrun::ghidra_conversion
