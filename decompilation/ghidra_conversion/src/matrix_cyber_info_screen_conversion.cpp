#include "shadowrun_ghidra/matrix_cyber_info_screen_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

bool page_activation_requested(const std::uint16_t input_events) {
    return (input_events & (activate_page_a | activate_page_b |
                            activate_page_c)) != 0U;
}

} // namespace

CyberInfoScreenResult run_cyber_info_screen(
    CyberInfoScreenState& state,
    const std::vector<std::uint16_t>& frame_inputs,
    const CyberInfoScreenCallbacks& callbacks) {
    // 0x18AA6..0x18AC6: clear the page cursor and lay down the CyberInfo
    // header before the first controller frame.
    state.selection = {};
    call_if_present(callbacks.write_panel_grid);
    call_if_present(callbacks.render_title);
    call_if_present(callbacks.write_page_cursor,
                    selected_cyber_info_page(state.selection));

    CyberInfoScreenResult result{};
    result.page = selected_cyber_info_page(state.selection);

    for (const auto input_events : frame_inputs) {
        ++result.frames_processed;
        call_if_present(callbacks.wait_for_vblank);
        call_if_present(callbacks.latch_controller_events);
        call_if_present(callbacks.sample_controller_input);
        call_if_present(callbacks.rotate_hud_history);

        // 0x18AE2..0x18B16: up has priority over down when both bits are
        // latched, and every movement performs the source cursor write.
        if ((input_events & move_page_up) != 0U) {
            update_cyber_info_selection(state.selection, -1);
            result.page = selected_cyber_info_page(state.selection);
            call_if_present(callbacks.write_page_cursor,
                            selected_cyber_info_page(state.selection));
            continue;
        }
        if ((input_events & move_page_down) != 0U) {
            update_cyber_info_selection(state.selection, 1);
            result.page = selected_cyber_info_page(state.selection);
            call_if_present(callbacks.write_page_cursor,
                            selected_cyber_info_page(state.selection));
            continue;
        }

        // 0x18B18..0x18B42: A/B/C all accept the currently selected page.
        // This test precedes the bit-15 return path in the source.
        if (page_activation_requested(input_events)) {
            result.page = selected_cyber_info_page(state.selection);
            if (result.page == CyberInfoPage::deck_stats) {
                call_if_present(callbacks.render_deck_stats);
            } else {
                call_if_present(callbacks.render_run_info);
            }
            result.exit = CyberInfoScreenExit::page_rendered;
            return result;
        }

        // 0x18B4A..0x18B5A: bit 15 redraws the selected Matrix panel and
        // returns without rendering a CyberInfo page.
        if ((input_events & return_to_matrix) != 0U) {
            call_if_present(callbacks.render_matrix_panel);
            result.exit = CyberInfoScreenExit::returned_to_matrix;
            return result;
        }
    }

    result.exit = CyberInfoScreenExit::frame_budget_exhausted;
    result.page = selected_cyber_info_page(state.selection);
    return result;
}

} // namespace shadowrun::ghidra_conversion
