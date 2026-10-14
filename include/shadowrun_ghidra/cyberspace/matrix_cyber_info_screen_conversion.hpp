#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "shadowrun_ghidra/cyberspace/matrix_cyber_info_conversion.hpp"

namespace shadowrun::ghidra_conversion {

// These are the controller-event bits consumed by the CyberInfo handler at
// ROM 0x18AA6.  The source checks the page cursor first, then any of the
// three page-activation buttons, and only then the Matrix-return button.
enum CyberInfoScreenInput : std::uint16_t {
    move_page_up = 1U << 8,
    move_page_down = 1U << 9,
    activate_page_a = 1U << 12,
    activate_page_b = 1U << 13,
    activate_page_c = 1U << 14,
    return_to_matrix = 1U << 15,
};

enum class CyberInfoScreenExit {
    frame_budget_exhausted,
    page_rendered,
    returned_to_matrix,
};

struct CyberInfoScreenState {
    // FFF0DC is the source's two-entry CyberInfo page cursor.  The handler
    // clears it on entry, so a reused host state is reset by the routine.
    CyberInfoSelectionState selection{};
};

struct CyberInfoScreenResult {
    CyberInfoScreenExit exit = CyberInfoScreenExit::frame_budget_exhausted;
    CyberInfoPage page = CyberInfoPage::deck_stats;
    std::size_t frames_processed = 0;
};

struct CyberInfoScreenCallbacks {
    // 0x18AA6's one-time screen setup.
    std::function<void()> write_panel_grid;
    std::function<void()> render_title;

    // One source frame of the interactive handler.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<void()> rotate_hud_history;

    // C376's paired cursor writes remain a host presentation boundary.  It
    // is called once at setup and once for every source page movement.
    std::function<void(CyberInfoPage)> write_page_cursor;

    // The source renders exactly one of these panels before returning to the
    // Matrix loop.  Their field/number reads are separate conversions.
    std::function<void()> render_deck_stats;
    std::function<void()> render_run_info;

    // 0x1882E is the source's return-to-Matrix redraw boundary.
    std::function<void()> render_matrix_panel;
};

// ROM 0x18AA6: run the CyberInfo selector/handler.  `frame_inputs` is a host
// test/input boundary: the Genesis loop has no frame limit, while the native
// conversion uses the supplied sequence and returns frame_budget_exhausted if
// it ends before a source exit event occurs.
CyberInfoScreenResult run_cyber_info_screen(
    CyberInfoScreenState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const CyberInfoScreenCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
