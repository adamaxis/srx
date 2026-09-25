#include "shadowrun_ghidra/matrix_cyber_info_conversion.hpp"

#include "shadowrun_ghidra/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void update_cyber_info_selection(CyberInfoSelectionState& state,
                                 const std::int8_t step) {
    const auto next = move_wrapped_menu_cursor(state.page, step, 1U);
    if (next == state.page) {
        return;
    }
    state.page = next;
    state.redraw_requested = true;
}

CyberInfoPage selected_cyber_info_page(
    const CyberInfoSelectionState& state) {
    return state.page == 0U ? CyberInfoPage::deck_stats
                            : CyberInfoPage::run_info;
}

MatrixRunAvailability classify_matrix_run_availability(
    const std::int8_t run_state) {
    if (run_state < 0) {
        return MatrixRunAvailability::not_on_run;
    }
    if (run_state > 0) {
        return MatrixRunAvailability::complete;
    }
    return MatrixRunAvailability::active;
}

MatrixRunPresentation select_matrix_run_presentation(
    const std::int8_t run_state, const std::int8_t node_state,
    const std::int8_t action_state) {
    MatrixRunPresentation result{};
    result.availability = classify_matrix_run_availability(run_state);
    if (result.availability != MatrixRunAvailability::active) {
        return result;
    }

    if (node_state < 0) {
        result.node = MatrixRunNode::cpu;
        result.action = MatrixRunAction::crash_system;
        return result;
    }

    result.node = MatrixRunNode::datastore;
    if (action_state < 0) {
        result.action = MatrixRunAction::upload_file;
    } else if (action_state == 0) {
        result.action = MatrixRunAction::delete_file;
    } else {
        result.action = MatrixRunAction::download_file;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
