#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class MenuConfirmationInput : std::uint16_t {
    option_decrement = 0x0100U, // source controller bit 8
    option_increment = 0x0200U, // source controller bit 9
    confirm = 0x1000U,          // source controller bit 12
    cancel = 0x4000U,            // source controller bit 14
};

struct MenuConfirmationState {
    std::uint8_t selected_option = 1; // FFF0DC; 0 is the source accept choice
    bool accepted = false;
    bool closed = false;
};

enum class MenuConfirmationStep : std::uint8_t {
    no_change,
    option_moved,
    closed,
};

struct MenuConfirmationCallbacks {
    std::function<void()> draw_prompt_labels;
    std::function<void()> draw_prompt_choice;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> play_move_sound;
    std::function<void()> play_confirm_sound;
};

// ROM 0x57B8C.  Draws the two-choice prompt, toggles its bounded 0/1 cursor,
// and returns the selected option when either source exit button is pressed.
// Movement services the source audio command before the wrapped cursor-write
// boundary. The native conversion has no VDP or controller-port dependency.
MenuConfirmationState run_menu_confirmation_prompt(
    const MenuConfirmationCallbacks& callbacks);

// One non-blocking iteration of ROM 0x57B8C. The Qt menu keeps this typed
// state between frames while the source conversion above uses the same input
// transition inside its blocking callback loop.
MenuConfirmationStep advance_menu_confirmation_prompt(
    MenuConfirmationState& state, std::uint16_t controller_events) noexcept;

} // namespace shadowrun::ghidra_conversion
