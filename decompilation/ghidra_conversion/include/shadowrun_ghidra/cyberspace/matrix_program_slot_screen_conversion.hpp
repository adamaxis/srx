#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "shadowrun_ghidra/cyberspace/matrix_program_slot_conversion.hpp"

namespace shadowrun::ghidra_conversion {

// 0x19238 reuses the standard menu event positions while asking whether the
// selected Matrix program should be installed/erased.  These names describe
// the state effect rather than the original controller hardware labels.
enum MatrixProgramSlotScreenInput : std::uint16_t {
    move_prompt_up = 1U << 8,
    move_prompt_down = 1U << 9,
    accept_prompt_a = 1U << 12,
    accept_prompt_b = 1U << 13,
    accept_prompt_c = 1U << 14,
    return_to_matrix_from_prompt = 1U << 15,
};

enum class MatrixProgramSlotScreenExit {
    frame_budget_exhausted,
    installed_program,
    refreshed_existing_slot,
    cancelled_prompt,
    returned_to_matrix,
};

struct MatrixProgramSlotScreenState {
    // The source enters 0x19368 with FFF0DC already set to the No option
    // (one). Callers that enter an authored prompt directly can choose the
    // initial selection explicitly.
    MatrixLoadedProgramPromptState prompt{1U, false};
    // FFD50 is tested before the Yes path: nonzero applies the slot command;
    // zero clears the ready timer and refreshes the slot visual instead.
    bool ready_timer_active = false;
};

struct MatrixProgramSlotScreenResult {
    MatrixProgramSlotScreenExit exit =
        MatrixProgramSlotScreenExit::frame_budget_exhausted;
    std::uint8_t selection = 1U;
    std::size_t frames_processed = 0;
};

struct MatrixProgramSlotScreenCallbacks {
    // 0x19368..0x1937E prompt preparation.
    std::function<void()> prepare_prompt;

    // One source frame of the prompt loop.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<void()> rotate_hud_history;
    std::function<void(std::uint8_t selection)> write_prompt_cursor;

    // 0x19400..0x19418 Yes/No action boundary.
    std::function<void()> apply_menu_slot_command;
    std::function<void()> clear_ready_menu_slot_timer;
    std::function<void()> refresh_menu_slot_visual;

    // 0x19428 always redraws the Matrix panel before returning.
    std::function<void()> write_hud_panel_grid;
    std::function<void()> render_matrix_panel;
};

// ROM 0x19238 / 0x19368: drive the loaded-program Yes/No prompt after the
// slot/memory preparation branches. The Genesis loop is unbounded; the host
// conversion receives an explicit input sequence so tests and callers can
// bound it without introducing an implicit runtime loop.
MatrixProgramSlotScreenResult run_matrix_program_slot_screen(
    MatrixProgramSlotScreenState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const MatrixProgramSlotScreenCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
